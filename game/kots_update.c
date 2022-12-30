#include <pthread.h>
#include <stdio.h>
#include <curl/curl.h>
#include "kots_update.h"
#include "kots_utils.h"
#include "kots_server.h"

#ifdef KOTS_OFFICIAL

#include "kots_official.h"

#else

#define KOTS_FTP_USER       "user:pass"
#define KOTS_URL_BASE       "ftp://localhost/"
#define KOTS_URL_VERSION    "version.txt"

#endif


#ifdef WIN32

#define KOTS_LOCAL_LIBRARY  "gamex86.dll"
#define KOTS_LOCAL_DEBUG    "kots2007.pdb"
#define KOTS_URL_LIBRARY    "latest-win32.dll"
#define KOTS_URL_DEBUG      "latest-win32.pdb"

#else

#define KOTS_LOCAL_LIBRARY  "gamei386.so"
#define KOTS_LOCAL_DEBUG    ""
#define KOTS_URL_LIBRARY    "latest-linux.so"
#define KOTS_URL_DEBUG      ""

#endif


//global variables
static pthread_cond_t   condition;
static pthread_mutex_t  update_lock;
static pthread_t        update_thread;
static qboolean         is_updating = false;
static char             game_dir[256];
static char             game_ver[64];
static char             update_url[512];
static int              update_wait;
static int              update_client_id;
static edict_t          *update_ent;

//function prototypes
void *Kots_RunUpdate(void *args);
void *Kots_RunRevert(void *args);

void Kots_UpdateInit()
{
    pthread_mutex_init(&update_lock, NULL);
    pthread_cond_init(&condition, NULL);
}

void Kots_UpdateFree()
{
    //wait for the update to finish
    pthread_mutex_lock(&update_lock);
    if (is_updating)
        pthread_cond_wait(&condition, &update_lock);
    pthread_mutex_unlock(&update_lock);

    //destory the lock and condition
    pthread_mutex_destroy(&update_lock);
    pthread_cond_destroy(&condition);
}

/*
size_t Kots_WriteFile( void *ptr, size_t size, size_t nmemb, void *stream)
{
    FILE *fHandle = (FILE *)stream;
    return fwrite(ptr,
}
*/

void Kots_DownloadFile(char *url, char *filepath)
{
    FILE *fHandle = NULL;

    //remove the existing file first if it exists
    remove(filepath);

    fHandle = fopen(filepath, "wb");
    if (fHandle)
    {
        CURL *curl = curl_easy_init();

        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_USERPWD, KOTS_FTP_USER);
            curl_easy_setopt(curl, CURLOPT_WRITEHEADER, 0);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
            curl_easy_setopt(curl, CURLOPT_FILE, fHandle);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
            curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 0);
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }

        fclose(fHandle);
    }
}

qboolean Kots_GetVersion(char *version, int len, char *filename)
{
    qboolean result = false;
    FILE *fVersion = NULL;

    fVersion = fopen(filename, "r");
    if (fVersion != NULL)
    {
        //reset version to an empty string
        *version = '\0';

        //read the version information and if successful set result to true
        fread(version, len, 1, fVersion);

        //if version is no longer an empty string
        //then we successfully read it
        if (*version != '\0')
            result = true;

        fclose(fVersion);
    }

    return result;
}

qboolean Kots_FileExists(char *filename)
{
    FILE *file =  fopen(filename, "r");
    if (file != NULL)
    {
        fclose(file);
        return true;
    }

    return false;
}

qboolean Kots_RenameAll(const int total, ...)
{
    int i;
    va_list list;
    char **files = malloc(sizeof(char *) * total);
    qboolean result = true;

    va_start(list, total);

    for (i = 0; i < total; i += 2)
    {
        files[i] = va_arg(list, char *);
        files[i + 1] = va_arg(list, char *);

        if (Kots_FileExists(files[i + 1]))
            remove(files[i + 1]);

        if (rename(files[i], files[i + 1]))
        {
            //in windows it may return failure but still work
            if (!Kots_FileExists(files[i + 1]))
            {
                //rename failed so set our result and break out
                result = false;
                break;
            }
        }
    }

    //if we failed rename the files back in reverse order
    if (!result)
    {
        for (; i >= 0; i -= 2)
        {
            rename(files[i + 1], files[i]);
        }
    }

    va_end(list);
    return result;
}

void Kots_WaitIncrement(int *remaining)
{
    int sleep_amount;

    //determine the interval at which to wait based on the total amount remaining
    if (*remaining <= 5)
        sleep_amount = *remaining;
    else if (*remaining <= 10)
        sleep_amount = 5 - *remaining % 5;
    else if (*remaining <= 30)
        sleep_amount = 20 - *remaining % 30;
    else if (*remaining <= 60)
        sleep_amount = 30 - *remaining % 60;
    else
        sleep_amount = 60 - *remaining % 60;

    *remaining -= sleep_amount;
    Sleep(sleep_amount * 1000);
}


void Kots_UpdateStartDefault(edict_t *ent)
{
    Kots_UpdateStart(ent, KOTS_URL_BASE, 60);
}

void Kots_UpdateStart(edict_t *ent, char *url, int wait)
{
    pthread_mutex_lock(&update_lock);

    if (is_updating)
    {
        gi.cprintf(ent, PRINT_HIGH, "ERROR: Unable to perform update. Another is currently in progress.\n");
    }
    else
    {
        //copy the game path and current version
        Kots_strncpy(game_dir, gamedir->string, sizeof(game_dir));
        Kots_strncpy(game_ver, gamever->string, sizeof(game_ver));

        //flag that an update is in progress
        is_updating = true;
        update_ent = ent;

        if (ent)
            update_client_id = ent->client->pers.kots_persist.client_id;
        else
            update_client_id = 0;

        //save the values for the update
        Kots_strncpy(update_url, url, sizeof(update_url));
        update_wait = wait;

        //create the thread
        pthread_create(&update_thread, NULL, Kots_RunUpdate, NULL);

        //print out an initial message to indicate it started successfully
        gi.cprintf(ent, PRINT_HIGH, "Update initiated. Checking for latest version...\n");
    }

    pthread_mutex_unlock(&update_lock);
}

void Kots_RevertStartDefault(edict_t *ent)
{
    Kots_RevertStart(ent, 60);
}

void Kots_RevertStart(edict_t *ent, int wait)
{
    char file1[1024];
    char file2[1024];

    pthread_mutex_lock(&update_lock);

    if (is_updating)
    {
        gi.cprintf(ent, PRINT_HIGH, "ERROR: Unable to perform revert. Another is currently in progress.\n");
    }
    else
    {
        qboolean hasDebug = (strlen(KOTS_URL_DEBUG) > 0 ? true : false);

        //copy the game path and current version
        Kots_strncpy(game_dir, gamedir->string, sizeof(game_dir));
        Kots_strncpy(game_ver, gamever->string, sizeof(game_ver));

        //verify required files are there
        Kots_snprintf(file1, sizeof(file1), "%s/%s.old", game_dir, KOTS_LOCAL_LIBRARY);
        Kots_snprintf(file2, sizeof(file2), "%s.old", KOTS_LOCAL_DEBUG);
        if (Kots_FileExists(file1) && (!hasDebug || Kots_FileExists(file2)))
        {

            //flag that an update is in progress
            is_updating = true;
            update_ent = ent;

            if (ent)
                update_client_id = ent->client->pers.kots_persist.client_id;
            else
                update_client_id = 0;

            //save the values for the update
            update_wait = wait;

            //create the thread
            pthread_create(&update_thread, NULL, Kots_RunRevert, NULL);

            //print out an initial message to indicate it started successfully
            gi.cprintf(ent, PRINT_HIGH, "Revert initiated.\n");
        }
        else
        {
            gi.cprintf(ent, PRINT_HIGH, "ERROR: Cannot initiate revert because the required files are missing.\n");
        }
    }

    pthread_mutex_unlock(&update_lock);
}

#define VERSION_URL_INDEX   0
#define VERSION_LOCAL_INDEX 1
#define GAME_URL_INDEX      2
#define GAME_LOCAL_INDEX    3
#define GAME_NEW_INDEX      4
#define GAME_OLD_INDEX      5
#define DEBUG_URL_INDEX     6
#define DEBUG_LOCAL_INDEX   7
#define DEBUG_NEW_INDEX     8
#define DEBUG_OLD_INDEX     9

void *Kots_RunUpdate(void *args)
{
    char files[DEBUG_OLD_INDEX + 1][1024];
    char message[128];
    char new_ver[64];

    //build all the filenames/urls we will be using
    Kots_snprintf(files[VERSION_URL_INDEX], sizeof(files[0]), "%s/%s", update_url, KOTS_URL_VERSION);
    Kots_snprintf(files[VERSION_LOCAL_INDEX], sizeof(files[0]), "%s/%s", game_dir, KOTS_URL_VERSION);
    Kots_snprintf(files[GAME_URL_INDEX], sizeof(files[0]), "%s/%s", update_url, KOTS_URL_LIBRARY);
    Kots_snprintf(files[GAME_LOCAL_INDEX], sizeof(files[0]), "%s/%s", game_dir, KOTS_LOCAL_LIBRARY);
    Kots_snprintf(files[GAME_NEW_INDEX], sizeof(files[0]), "%s/%s.new", game_dir, KOTS_LOCAL_LIBRARY);
    Kots_snprintf(files[GAME_OLD_INDEX], sizeof(files[0]), "%s/%s.old", game_dir, KOTS_LOCAL_LIBRARY);
    Kots_snprintf(files[DEBUG_URL_INDEX], sizeof(files[0]), "%s/%s", update_url, KOTS_URL_DEBUG);
    Kots_snprintf(files[DEBUG_LOCAL_INDEX], sizeof(files[0]), "%s", KOTS_LOCAL_DEBUG);
    Kots_snprintf(files[DEBUG_NEW_INDEX], sizeof(files[0]), "%s.new", KOTS_LOCAL_DEBUG);
    Kots_snprintf(files[DEBUG_OLD_INDEX], sizeof(files[0]), "%s.old", KOTS_LOCAL_DEBUG);


    //first download the version
    Kots_DownloadFile(files[VERSION_URL_INDEX], files[VERSION_LOCAL_INDEX]);

    //if the version is different than our current version
    if (Kots_GetVersion(new_ver, sizeof(new_ver), files[VERSION_LOCAL_INDEX]) && Q_stricmp(new_ver, game_ver) != 0)
    {
        qboolean hasDebug = (strlen(KOTS_URL_DEBUG) > 0 ? true : false);

        Kots_DownloadFile(files[GAME_URL_INDEX], files[GAME_NEW_INDEX]);

        if (hasDebug)
        {
            Kots_DownloadFile(files[DEBUG_URL_INDEX], files[DEBUG_NEW_INDEX]);
        }

        //if we successfully downloaded the necessary files
        if (Kots_FileExists(files[GAME_NEW_INDEX]) && (!hasDebug || Kots_FileExists(files[DEBUG_NEW_INDEX])))
        {
            qboolean success;

            if (hasDebug)
            {
                //has debug information so rename game libraries and debug files
                success = Kots_RenameAll(8, files[GAME_LOCAL_INDEX], files[GAME_OLD_INDEX], files[DEBUG_LOCAL_INDEX], files[DEBUG_OLD_INDEX],
                    files[GAME_NEW_INDEX], files[GAME_LOCAL_INDEX], files[DEBUG_NEW_INDEX], files[DEBUG_LOCAL_INDEX]);
            }
            else
            {
                //no debug information so just rename the game library files
                success = Kots_RenameAll(4, files[GAME_LOCAL_INDEX], files[GAME_OLD_INDEX],
                    files[GAME_NEW_INDEX], files[GAME_LOCAL_INDEX]);
            }

            if (success)
            {
                    int sleeptime = update_wait;

                    //send out success message
                    Kots_snprintf(message, sizeof(message), "The server has been updated to %s and will restart in %i seconds.\n", new_ver, sleeptime);
                    Kots_ServerAddServerMessage(message);

                    //wait the specified amount of time
                    while (sleeptime > 0)
                    {
                        //wait for a moment before we print the next message
                        Kots_WaitIncrement(&sleeptime);

                        //send out messages that the server is going down
                        Kots_snprintf(message, sizeof(message), "The server is going restart to finish an update in %i seconds.\n", sleeptime);
                        Kots_ServerAddServerMessage(message);
                    }

                    //force the server to reload the game library
                    Kots_ServerAddServerCommand("sv_allow_map 1; map q2dm1; sv_allow_map 0\n");
            }

            if (!success)
            {
                Kots_ServerAddClientMessage(update_ent, update_client_id, "ERROR: Update failed. Unable to rename one or more files.\n");
            }
        }
        else
        {
            Kots_ServerAddClientMessage(update_ent, update_client_id, "ERROR: Update failed. Unable to download one or more files.\n");
        }
    }
    else
    {
        Kots_ServerAddClientMessage(update_ent, update_client_id, "No new updates available.\n");
    }

    //signal that the update is complete
    pthread_mutex_lock(&update_lock);

    is_updating = false;

    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&update_lock);

    return NULL;
}

void *Kots_RunRevert(void *args)
{
    char files[DEBUG_OLD_INDEX + 1][1024];
    char message[128];
    qboolean success;
    qboolean hasDebug = (strlen(KOTS_URL_DEBUG) > 0 ? true : false);

    //build all the filenames/urls we will be using
    Kots_snprintf(files[VERSION_URL_INDEX], sizeof(files[0]), "%s/%s", update_url, KOTS_URL_VERSION);
    Kots_snprintf(files[VERSION_LOCAL_INDEX], sizeof(files[0]), "%s/%s", game_dir, KOTS_URL_VERSION);
    Kots_snprintf(files[GAME_URL_INDEX], sizeof(files[0]), "%s/%s", update_url, KOTS_URL_LIBRARY);
    Kots_snprintf(files[GAME_LOCAL_INDEX], sizeof(files[0]), "%s/%s", game_dir, KOTS_LOCAL_LIBRARY);
    Kots_snprintf(files[GAME_NEW_INDEX], sizeof(files[0]), "%s/%s.new", game_dir, KOTS_LOCAL_LIBRARY);
    Kots_snprintf(files[GAME_OLD_INDEX], sizeof(files[0]), "%s/%s.old", game_dir, KOTS_LOCAL_LIBRARY);
    Kots_snprintf(files[DEBUG_URL_INDEX], sizeof(files[0]), "%s/%s", update_url, KOTS_URL_DEBUG);
    Kots_snprintf(files[DEBUG_LOCAL_INDEX], sizeof(files[0]), "%s", KOTS_LOCAL_DEBUG);
    Kots_snprintf(files[DEBUG_NEW_INDEX], sizeof(files[0]), "%s.new", KOTS_LOCAL_DEBUG);
    Kots_snprintf(files[DEBUG_OLD_INDEX], sizeof(files[0]), "%s.old", KOTS_LOCAL_DEBUG);

    if (hasDebug)
    {
        //has debug information so rename game libraries and debug files
        success = Kots_RenameAll(8, files[GAME_LOCAL_INDEX], files[GAME_NEW_INDEX], files[DEBUG_LOCAL_INDEX], files[DEBUG_NEW_INDEX],
            files[GAME_OLD_INDEX], files[GAME_LOCAL_INDEX], files[DEBUG_OLD_INDEX], files[DEBUG_LOCAL_INDEX]);
    }
    else
    {
        //no debug information so just rename the game library files
        success = Kots_RenameAll(4, files[GAME_LOCAL_INDEX], files[GAME_NEW_INDEX],
            files[GAME_OLD_INDEX], files[GAME_LOCAL_INDEX]);
    }
    //if we were successful at renaming the new library files
    if (success)
    {
        int sleeptime = update_wait;

        //send out success message
        Kots_snprintf(message, sizeof(message), "The server has been reverted to a previous version and will restart in %i seconds.\n", sleeptime);
        Kots_ServerAddServerMessage(message);

        //wait the specified amount of time
        while (sleeptime > 0)
        {
            //wait for a moment before we print the next message
            Kots_WaitIncrement(&sleeptime);

            //send out messages that the server is going down
            Kots_snprintf(message, sizeof(message), "The server is going restart to finish an revert in %i seconds.\n", sleeptime);
            Kots_ServerAddServerMessage(message);
        }

        //force the server to reload the game library
        Kots_ServerAddServerCommand("sv_allow_map 1; map q2dm1; sv_allow_map 0\n");
    }
    else
    {
        Kots_ServerAddClientMessage(update_ent, update_client_id, "ERROR: Revert failed. Unable to revert one or more files.\n");
    }

    //signal that the update is complete
    pthread_mutex_lock(&update_lock);

    is_updating = false;

    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&update_lock);

    return NULL;
}


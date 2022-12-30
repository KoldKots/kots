#include <time.h>
#include "kots_server.h"
#include "kots_logging.h"
#include "kots_mute.h"
#include "kots_commands.h"
#include "kots_runes.h"
#include "kots_update.h"
#include "kots_utils.h"
#include "kots_menu.h"
#include "kots_admin.h"

#define KOTS_SERVER_UPDATE_INTERVAL     120 /* Interval in seconds */

array_t *kots_ipbans = NULL;
array_t *kots_servers = NULL;
time_t next_server_update = 0;
qboolean allow_login = true;
qboolean server_login = true;

//static function prototypes
static void LoginCharacterInternal(edict_t *ent, char *name, char *pass, char *ip_address, int client_id);
static void MysqlCreateCharacter(dbthread_t *thread, jobitem_t *args);
void MysqlSaveCharacter(dbthread_t *thread, jobitem_t *args);
static void MysqlLoginCharacter(dbthread_t *thread, jobitem_t *args);
static void MysqlLoadCharacter(dbthread_t *thread, jobitem_t *args);
static void MysqlUpdateServer(dbthread_t *thread, jobitem_t *args);
static void MysqlUpdateIpbans(dbthread_t *thread, jobitem_t *args);
static void MysqlBanIp(dbthread_t *thread, jobitem_t *args);
static void MysqlAdminLogin(dbthread_t *thread, jobitem_t *job);
static void MysqlLoadRunes(dbthread_t *thread, jobitem_t *job);
static void MysqlLogInfo(dbthread_t *thread, jobitem_t *job);
static void MysqlLoadMuteList(dbthread_t *thread, jobitem_t *job);
static void MysqlMutePlayer(dbthread_t *thread, jobitem_t *job);
static void MysqlUnmutePlayer(dbthread_t *thread, jobitem_t *job);

//global reference to the dbthreads
dbthread_t *threads[KOTS_MAX_DBTHREADS];

void Kots_InitializeServer()
{
    int i;

    //first verify commands are all fine
    Kots_CheckDuplicateCommands();

    //initialize other things
    Kots_UpdateInit();
    Kots_MuteInit();

    //do initial creating of queues, etc
    Kots_InitDbThreads();

    //create the actual threads
    for (i = 0; i < KOTS_MAX_DBTHREADS; i++)
        threads[i] = Kots_CreateDbThread();

    kots_servers = Array_Create(NULL, 32, 0);
    kots_ipbans = Array_Create(NULL, 32, 0);
    next_server_update = time(NULL) + 5; //give the server a minute to initialize
}

void Kots_FreeServer()
{

    int i;
    
    //free the other services
    Kots_UpdateFree();

    //wait for all input jobs to be completed
    Kots_WaitForDbThreads();

    // Kold -   This is broken in the official release due to deadlock. Temporary disablement,
    //          since it prevents automatic server shutdown.
    /*
    for (i = 0; i < KOTS_MAX_DBTHREADS; i++)
    {
        gi.dprintf("Stopping thread %i.\n", i);
        Kots_StopDbThread(threads[i]);
        Kots_FreeDbThread(threads[i]);
        threads[i] = NULL;
    }
    */

    gi.dprintf("Cleaning up the queues.\n");
    //cleanup the input/output queues etc
    Kots_FreeDbThreads();

    gi.dprintf("Cleaning up server arrays.\n");
    Array_Delete(kots_servers);
    Array_Delete(kots_ipbans);
    kots_servers = NULL;
    kots_ipbans = NULL;

    //free the other services
    gi.dprintf("Cleaning up mute list.\n");
    Kots_MuteFree();

}

void Kots_ServerProcessOutput()
{
    listitem_t *item;
    jobitem_t *job;
    time_t now = time(NULL);

    //process the first item in each output queue
    //we don't want to do too much in one frame
    while ((item = Kots_GetNextDbOutput()) != NULL)
    {
        job = (jobitem_t *)item->data;

        //call the callback completion method for the job
        if (job->complete_callback)
            job->complete_callback(job);

        //check for any error messages
        if (job->last_error)
            gi.dprintf("DB: ERROR - %s\n", job->last_error);

        Kots_FreeJobItem(item);
    }

    //next server update will be 0 until the previous one finishes
    if (next_server_update != 0 && now >= next_server_update)
    {
        next_server_update = 0;
        Kots_AddServerUpdateJob();
    }
}

qboolean Kots_ServerIsClientValid(edict_t *ent, int client_id)
{
    if (ent && ent->client && ent->client->pers.kots_persist.client_id == client_id)
        return true;
    else
        return false;
}

void Kots_ServerProcessAdminLogin(jobitem_t *job)
{
    adminlogin_info_t *info = (adminlogin_info_t *)job->args;

    //Print debug information about what results will be shown
    gi.dprintf("DB: Result of admin login for '%s': ", info->name);
    
    //if the client is no longer connected
    //we can't do anything
    if (info->ent && info->ent->inuse && info->ent->client && info->client_id == info->ent->client->pers.kots_persist.client_id)
    {

        //process result
        switch (job->result)
        {
        case KOTS_SERVER_SUCCESS:
            if (Q_stricmp(info->ip_address, info->ent->client->pers.kots_persist.ip_address) == 0)
            {
                gi.cprintf(info->ent, PRINT_HIGH, "You are now logged in with admin priviledges.\n");
                info->ent->client->pers.kots_persist.is_admin = true;
                gi.dprintf("Admin login successful.\n");
            }
            break;

        case KOTS_SERVER_NOTADMIN:
            gi.cprintf(info->ent, PRINT_HIGH, "The specified character is not an admin.\n");
            gi.dprintf("Character is not an admin.\n");
            break;
        case KOTS_SERVER_INVALIDCHAR:
            gi.cprintf(info->ent, PRINT_HIGH, "That character doesn't exist.\n");
            gi.dprintf("Character does not exist.\n");
            break;
        case KOTS_SERVER_INVALIDPASS:
            gi.cprintf(info->ent, PRINT_HIGH, "The password specified is not correct.\n");
            gi.dprintf("The password is not correct.\n");
            break;
        case KOTS_SERVER_CONNECTIONDOWN:
            gi.cprintf(info->ent, PRINT_HIGH, "The character server may be down.\n");
            gi.dprintf("The character server may be down.\n");
            break;
        case KOTS_SERVER_UNKNOWNERROR:
            gi.cprintf(info->ent, PRINT_HIGH, "An unknown error occurred with the character server.\n");
            gi.dprintf("An unknown error occurred.\n");
            break;
        }
    }
    else
    {
        if (job->result == KOTS_SERVER_SUCCESS)
            gi.dprintf("Successful and client disconnected.\n");
        else
            gi.dprintf("Error occurred and client disconnected.\n", info->name);
    }
}

void Kots_ServerProcessCharacterInfoOutput(jobitem_t *job)
{
    character_info_t *info = (character_info_t *)job->args;
    int logout = 0;

    switch (info->type)
    {
        case KOTS_CHARINFO_TYPE_LOGIN:
            gi.dprintf("DB: Result of login for '%s': ", info->name);
            break;
        case KOTS_CHARINFO_TYPE_KOTSINFO:
            gi.dprintf("DB: Result of kots_info for '%s': ", info->name);
            break;
        case KOTS_CHARINFO_TYPE_POINTLIST:
            gi.dprintf("DB: Result of kots_pointlist for '%s': ", info->name);
            break;
        case KOTS_CHARINFO_TYPE_SAVE:
            gi.dprintf("DB: Result of saving character '%s': ", info->name);

    }
    
    //if the client is no longer connected
    //we can't do anything
    //or if a different client replaced him/her
    if (info->ent && info->ent->inuse && info->ent->client && info->ent->character && info->client_id == info->ent->client->pers.kots_persist.client_id)
    {
        //if successfull
        if (job->result == KOTS_SERVER_SUCCESS)
        {
            switch (info->type)
            {
            case KOTS_CHARINFO_TYPE_LOGIN:
                if (info->ent->character->is_loggedin)
                    gi.dprintf("Character already logged in, duplicate attempt.\n");
                else if (!allow_login)
                {
                    gi.cprintf(info->ent, PRINT_HIGH, "Login has temporarily been disabled. Please try again shortly.\n");
                    gi.dprintf("Can't login because login disabled.\n");
                    logout = 1;
                }
                else
                {
                    //don't allow cursed characters to login
                    if (info->character.is_cursed)
                    {
                        gi.centerprintf(info->ent, "Your character has been cursed!");
                        gi.dprintf("Can't login because character is cursed.\n");
                    }
                    else
                    {
                        (*info->ent->character) = info->character;
                        Kots_strncpy(info->ent->character->name, info->name, 50);
                        Kots_strncpy(info->ent->character->title, info->character.title, 51);
                        Kots_CharacterLogin(info->ent);
                        Kots_CharacterLoadPersist(info->ent, &info->persist);
                        gi.dprintf("Login successful.\n");
                    }
                }
                break;
            case KOTS_CHARINFO_TYPE_KOTSINFO:
                Kots_MenuShowOtherCharacterInfo(info->ent, NULL, &info->character);
                gi.dprintf("Successful.\n");
                break;
            case KOTS_CHARINFO_TYPE_POINTLIST:
                Kots_MenuShowOtherPointlist(info->ent, &info->character);
                gi.dprintf("Successful.\n");
                break;
            case KOTS_CHARINFO_TYPE_SAVE:
                gi.dprintf("Successful.\n");
                break;
            }
        }

        //error messages
        switch (job->result)
        {
        case KOTS_SERVER_ALREADYEXISTS:
            gi.cprintf(info->ent, PRINT_HIGH, "The specified character already exists. Try joining the game instead.\n");
            gi.dprintf("Character already exists.\n");
            break;
        case KOTS_SERVER_ALREADYLOGGEDIN:
            if (!info->ent->character->is_loggedin)
            {
                gi.cprintf(info->ent, PRINT_HIGH, "This character is already logged in. Please wait a while and try again.\n");
                gi.dprintf("Already logged in.\n");
            }
            else
                gi.dprintf("Duplicate login attempt.\n");

            break;
        case KOTS_SERVER_INVALIDCHAR:
            gi.cprintf(info->ent, PRINT_HIGH, "That character doesn't exist.\n");
            gi.dprintf("Character doesn't exist.\n");
            break;
        case KOTS_SERVER_INVALIDPASS:
            gi.cprintf(info->ent, PRINT_HIGH, "The password specified is not correct.\n");
            gi.dprintf("Password is not correct.\n");
            break;
        case KOTS_SERVER_LOGIN_DISABLED:
            gi.cprintf(info->ent, PRINT_HIGH, "Login has been administratively disabled.\n");
            gi.dprintf("Login is disabled.\n");
            break;
        case KOTS_SERVER_CONNECTIONDOWN:
            gi.cprintf(info->ent, PRINT_HIGH, "The character server may be down.\n");
            gi.dprintf("The connection is down.\n");
            break;
        case KOTS_SERVER_UNKNOWNERROR:
            gi.cprintf(info->ent, PRINT_HIGH, "An unknown error occurred with the character server.\n");
            gi.dprintf("An unknown error occurred.\n");
            break;
        }
    }
    else
    {
        if (job->result == KOTS_SERVER_SUCCESS)
            gi.dprintf("Successful and client disconnected.\n");
        else
            gi.dprintf("Error occurred and client disconnected.\n", info->name);

        //If we successfully logged in to a character then make sure we logout since they're no longer here
        //Otherwise the next server they go to they will be unable to login for a while
        if (info->type == KOTS_CHARINFO_TYPE_LOGIN && job->result == KOTS_SERVER_SUCCESS)
            logout = 1;
    }

    //If we successfully logged in but had to logout for some reason then ensure we get logged out correctly.
    if (logout)
    {
        jobitem_t *job2 = Kots_CreateJobItem("Logout after failed login");
        character_info_t *info2 = malloc(sizeof(*info));
        info2->ent = NULL;
        info2->client_id = 0;
        info2->character = info->character;
        info2->persist = info->persist;
        Kots_strncpy(info2->name, info->name, 16);
        info2->character.is_loggedin = false;
        info2->type = KOTS_CHARINFO_TYPE_SAVE;

        job2->args = info2;
        job2->wait_check = Kots_WaitForSave;
        job2->function = MysqlSaveCharacter;
        job2->complete_callback = Kots_ServerProcessCharacterInfoOutput;

        gi.dprintf("DB: Logging out of '%s' after failed login.\n", info2->name);
        Kots_AddDbThreadJob(job2);
    }
}

int Kots_ServerSort(void *a, void *b)
{
    server_info_t *s1 = (server_info_t *)a;
    server_info_t *s2 = (server_info_t *)b;
    return Q_stricmp(s1->name, s2->name);
}

void Kots_ServerProcessIpban(jobitem_t *job)
{
    ipban_info_t *info = (ipban_info_t *)job->args;

    if (job->result == KOTS_SERVER_SUCCESS)
    {
        char *ip = strdup(info->ip_address);

        //put the ip address into the list of bans
        Array_PushBack(kots_ipbans, ip);

        //print out the message
        gi.bprintf(PRINT_HIGH, "%s\n", info->description);
        gi.dprintf("DB: Successfully saved ip ban for %s by %s.\n", info->ip_address, info->admin);

        //kick the user from the server
        if (info->banned->inuse)
            Kots_Admin_KickUser(info->admin, info->banned);
    }
    else
    {
        //an error occurred so notify the admin
        gi.cprintf(info->admin, PRINT_HIGH, "An error occurred trying to ban %s. Please try again.\n", info->banned->character->name);
        gi.dprintf("DB: Error - Failed to save ip ban for %s by %s.\n", info->ip_address, info->admin);
    }
}

void Kots_ServerProcessIpbanOutput(jobitem_t *job)
{
    array_t *list = (array_t *)job->args;

    if (list != NULL)
    {
        //delete all current ip ban entries
        Array_DeleteAll(kots_ipbans);

        //copy the new list of bans
        Array_CopyValues(list, kots_ipbans);

        //cleanup, clear the list first so we don't free items we just copied and delete the array
        Array_ClearAll(list);
        Array_Delete(list);

        //clear the pointer
        job->args = list = NULL;

        gi.dprintf("DB: Successfully obtained list of ip bans from the server...\n");

        if (kots_ipbans->length > 0)
        {
            int i;
            edict_t *ent = g_edicts + 1;

            for (i = 0; i < game.maxclients; i++, ent++)
            {
                if (!ent->inuse)
                    continue;

                if (Kots_CheckIpBan(ent))
                {
                    gi.bprintf(PRINT_HIGH, "%s is IP banned.\n", ent->client->pers.netname);
                    Kots_Admin_KickUser(NULL, ent);
                }
            }
        }
    }
    else
    {
        gi.dprintf("DB: ERROR - Failed to load new ip ban list from server!\n");
    }
}

void Kots_ServerProcessServerInfoOutput(jobitem_t *job)
{
    if (job->result == KOTS_SERVER_SUCCESS)
    {
        server_args_t *args = (server_args_t *)job->args;

        if (args->servers)
        {

            //delete all the current servers because we're going to replace them with the new list
            Array_DeleteAll(kots_servers);
            Array_CopyValues(args->servers, kots_servers);

            //sort the list of servers
            Array_Sort(kots_servers, Kots_ServerSort);

            //now cleanup the server args before continuing
            Array_ClearAll(args->servers);
            Array_Delete(args->servers);
            args->servers = NULL;

            gi.dprintf("DB: Successfully updated server list.\n");

            //if there are servers refresh all clients servers page
            if (kots_servers->length > 0)
            {
                int i;
                edict_t *player = g_edicts + 1;

                for (i = 0; i < game.maxclients; i++, player++)
                {
                    if (!player->inuse)
                        continue;

                    if (Kots_MenuIsOpen(player, "ServerMenu"))
                        Kots_MenuCreateServers(player);
                }
            }
        }
        else
        {
            gi.dprintf("DB: ERROR - Unknown error occurred loading server list!\n");
        }
    }
    else
        gi.dprintf("DB: ERROR - Failed to download server list!\n");

    //set the next server update time
    next_server_update = time(NULL) + KOTS_SERVER_UPDATE_INTERVAL;
}

void Kots_AddIpbanUpdateJob()
{
    jobitem_t *job = Kots_CreateJobItem("Update IP Bans");
    job->complete_callback = Kots_ServerProcessIpbanOutput;
    job->function = MysqlUpdateIpbans;
    Kots_AddDbThreadJob(job);
}

void Kots_AddMuteListUpdateJob()
{
    jobitem_t *job = Kots_CreateJobItem("Update Mute List");
    job->complete_callback = Kots_UpdateMuteList;
    job->function = MysqlLoadMuteList;
    Kots_AddDbThreadJob(job);
}

void Kots_AddMuteJob(mute_info_t *info)
{
    jobitem_t *job = Kots_CreateJobItem("Mute");
    job->complete_callback = Kots_MuteComplete;
    job->function = MysqlMutePlayer;
    job->args = info;
    Kots_AddDbThreadJob(job);
}

void Kots_AddUnmuteJob(mute_info_t *info)
{
    jobitem_t *job = Kots_CreateJobItem("Unmute");
    job->complete_callback = Kots_UnmuteComplete;
    job->function = MysqlUnmutePlayer;
    job->args = info;
    Kots_AddDbThreadJob(job);
}

void Kots_ServerBanIp(edict_t *admin, edict_t *banned)
{
    jobitem_t *job = Kots_CreateJobItem("IP Ban");
    ipban_info_t *args = malloc(sizeof(*args));
    job->args = args;

    //set the job arguments info
    Kots_strncpy(args->ip_address, banned->client->pers.kots_persist.ip_address, 16);
    Kots_snprintf(args->description, 512, "%s was banned by %s", banned->client->pers.netname, admin->character->name);
    args->admin = admin;
    args->banned = banned;

    job->complete_callback = Kots_ServerProcessIpban;
    job->wait_check = NULL;
    job->function = MysqlBanIp;
    Kots_AddDbThreadJob(job);
}

qboolean Kots_CheckIpBan(edict_t *ent)
{
    ULONG i;

    for (i = 0; i < kots_ipbans->length; i++)
    {
        char *ip = (char *)Array_GetValueAt(kots_ipbans, i);

        if (strcmp(ip, ent->client->pers.kots_persist.ip_address) == 0)
            return true;
    }

    return false;
}

void Kots_AddServerUpdateJob()
{
    int i;
    edict_t *ent;
    jobitem_t *job = Kots_CreateJobItem("Update Server List");
    server_args_t *args = malloc(sizeof(*args));
    memset(args, 0, sizeof(*args));
    Kots_strncpy(args->info.name, hostname->string, 128);
    Kots_strncpy(args->info.map, mapname->string, 64);
    args->info.port = (int)port->value;
    args->info.max_players = game.maxclients;
    args->info.is_public = (kots_public->value != 0 ? 1 : 0);
    args->servers = NULL;

    //count the current number of players
    ent = g_edicts + 1;
    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (ent->inuse)
            args->info.cur_players++;
    }

    job->args = args;
    job->function = MysqlUpdateServer;
    job->complete_callback = Kots_ServerProcessServerInfoOutput;
    Kots_AddDbThreadJob(job);
}

void Kots_AddRuneUpdateJob(void (*complete_callback)(struct jobitem_s *job))
{
    jobitem_t *job = Kots_CreateJobItem("Update Rune List");
    
    //Rune updates don't require any arguments and they only return back using args
    job->function = MysqlLoadRunes;
    job->complete_callback = complete_callback;
    
    //add the job to the queue
    Kots_AddDbThreadJob(job);
}

void Kots_ServerLogInfo(char *reason)
{
    jobitem_t *job = Kots_CreateJobItem("Add Server Log Entry");
    
    //set up the job arguments and callbacks
    job->args = Kots_ServerBuildLogInfo(reason);
    job->function = MysqlLogInfo;
    
    //add the job to the queue
    Kots_AddDbThreadJob(job);
}

void Kots_ServerProcessDebugMessage(jobitem_t *job)
{
    gi.dprintf((char *)job->args);
}

void Kots_ServerAddDebugMessage(char *message)
{
    if (message)
    {
        jobitem_t *job = Kots_CreateJobItem("Debug Message");
        job->args = strdup(message);
        job->complete_callback = Kots_ServerProcessDebugMessage;
        job->result = KOTS_SERVER_SUCCESS;
        Kots_AddDbOutputJob(job);
    }
}

void Kots_ServerProcessOutputMessage(jobitem_t *job)
{
    gi.bprintf(PRINT_HIGH, (char *)job->args);
}

void Kots_ServerAddServerMessage(char *message)
{
    if (message)
    {
        jobitem_t *job = Kots_CreateJobItem("Server Message");
        job->args = strdup(message);
        job->complete_callback = Kots_ServerProcessOutputMessage;
        job->result = KOTS_SERVER_SUCCESS;
        Kots_AddDbOutputJob(job);
    }
}

void Kots_ServerProcessOutputCommand(jobitem_t *job)
{
    gi.AddCommandString((char *)job->args);
}

void Kots_ServerAddServerCommand(char *command)
{
    if (command)
    {
        jobitem_t *job = Kots_CreateJobItem("Server Command");
        job->args = strdup(command);
        job->complete_callback = Kots_ServerProcessOutputCommand;
        job->result = KOTS_SERVER_SUCCESS;
        Kots_AddDbOutputJob(job);
    }
}

void Kots_ServerProcessOutputClientMessage(jobitem_t *job)
{
    client_message_t *msg = (client_message_t *)job->args;

    //verify the client is still valid
    if (!msg->ent || (msg->ent->inuse && msg->ent->client->pers.kots_persist.client_id == msg->client_id))
    {
        gi.cprintf(msg->ent, PRINT_HIGH, msg->message);
    }

    //cleanup the message
    free(msg->message);
    msg->message = NULL;
}

void Kots_ServerAddClientMessage(edict_t *ent, int client_id, char *text)
{
    jobitem_t *job = Kots_CreateJobItem("Client Message");
    client_message_t *msg = malloc(sizeof(*msg));

    //create message
    msg->ent = ent;
    msg->client_id = client_id;
    msg->message = strdup(text);

    //create job
    job->args = msg;
    job->complete_callback = Kots_ServerProcessOutputClientMessage;
    job->result = KOTS_SERVER_SUCCESS;

    //add job to the queue
    Kots_AddDbOutputJob(job);
}

void Kots_ServerCreateCharacter(edict_t *ent)
{
    jobitem_t *job = Kots_CreateJobItem("Create Character");
    character_info_t *info = malloc(sizeof(*info));
    memset(&info->character, 0, sizeof(character_t));
    Kots_strncpy(info->name, ent->client->pers.netname, 16);
    Kots_strncpy(info->pass, Info_ValueForKey(ent->client->pers.userinfo, "kots_pass"), 46);
    Kots_strncpy(info->ip_address, ent->client->pers.kots_persist.ip_address, 16);
    info->ent = ent;
    info->client_id = ent->client->pers.kots_persist.client_id;
    info->type = KOTS_CHARINFO_TYPE_SAVE;

    job->args = info;
    job->wait_check = Kots_WaitForSave; //mainly to prevent dups
    job->function = MysqlCreateCharacter;
    job->complete_callback = Kots_ServerProcessCharacterInfoOutput;

    gi.dprintf("DB: Creating character '%s'\n", info->name);
    Kots_AddDbThreadJob(job);
}

void Kots_ServerSaveCharacter(edict_t *ent)
{
    jobitem_t *job = Kots_CreateJobItem("Save Character");
    character_info_t *info = malloc(sizeof(*info));
    info->ent = ent;
    info->client_id = ent->client->pers.kots_persist.client_id;
    info->character = *(ent->character);
    Kots_CharacterPersist(ent, &info->character, &info->persist);
    Kots_strncpy(info->name, ent->character->name, 16);

    job->args = info;
    info->type = KOTS_CHARINFO_TYPE_SAVE;
    job->wait_check = Kots_WaitForSave;
    job->skip_check = Kots_CheckForNewerSaves;
    job->function = MysqlSaveCharacter;
    job->complete_callback = Kots_ServerProcessCharacterInfoOutput;

    gi.dprintf("DB: Saving character '%s'\n", info->name);
    Kots_AddDbThreadJob(job);
}

void Kots_ServerLoginCharacter(edict_t *ent)
{
    LoginCharacterInternal(ent, ent->client->pers.netname, Info_ValueForKey(ent->client->pers.userinfo, "kots_pass"), ent->client->pers.kots_persist.ip_address, ent->client->pers.kots_persist.client_id);
    gi.dprintf("DB: Logging into character '%s'.\n", ent->client->pers.netname);
}


void Kots_ServerLoadKotsInfo(edict_t *ent, char *name)
{
    jobitem_t *job = Kots_CreateJobItem("Load kots_info");
    character_info_t *info = malloc(sizeof(*info));
    memset(&info->character, 0, sizeof(character_t));
    Kots_strncpy(info->character.name, name, 16);
    Kots_strncpy(info->name, name, 16);
    info->ent = ent;
    info->client_id = ent->client->pers.kots_persist.client_id;
    info->type = KOTS_CHARINFO_TYPE_KOTSINFO;

    job->args = info;
    job->function = MysqlLoadCharacter;
    job->complete_callback = Kots_ServerProcessCharacterInfoOutput;

    gi.dprintf("DB: Loading character '%s' kotsinfo for client '%s'.\n", info->name, ent->client->pers.netname);
    Kots_AddDbThreadJob(job);
}

void Kots_ServerLoadPointlist(edict_t *ent, char *name)
{
    jobitem_t *job = Kots_CreateJobItem("Load kots_pointlist");
    character_info_t *info = malloc(sizeof(*info));
    memset(&info->character, 0, sizeof(character_t));
    Kots_strncpy(info->character.name, name, 16);
    Kots_strncpy(info->name, name, 16);
    info->ent = ent;
    info->client_id = ent->client->pers.kots_persist.client_id;
    info->type = KOTS_CHARINFO_TYPE_POINTLIST;

    job->args = info;
    job->function = MysqlLoadCharacter;
    job->complete_callback = Kots_ServerProcessCharacterInfoOutput;

    gi.dprintf("DB: Loading character '%s' pointlist for client '%s'.\n", info->name, ent->client->pers.netname);
    Kots_AddDbThreadJob(job);
}

void Kots_ServerAdminLogin(edict_t *ent)
{
    jobitem_t *job = Kots_CreateJobItem("Admin Login");
    adminlogin_info_t *info = malloc(sizeof(*info));
    Kots_strncpy(info->name, ent->client->pers.netname, 16);
    Kots_strncpy(info->pass, Info_ValueForKey(ent->client->pers.userinfo, kots_pass->name), 46);
    Kots_strncpy(info->ip_address, ent->client->pers.kots_persist.ip_address, 16);
    info->ent = ent;
    info->client_id = ent->client->pers.kots_persist.client_id;

    job->args = info;
    job->function = MysqlAdminLogin;
    job->complete_callback = Kots_ServerProcessAdminLogin;

    gi.dprintf("DB: Logging into admin character '%s' (%s).\n", info->name, info->ip_address);
    Kots_AddDbThreadJob(job);
}

void Kots_ServerAllowLogin(jobitem_t *job)
{
    //allow login again
    allow_login = true;
}

void Kots_ServerDbNameChanged()
{
    int i;
    jobitem_t *job = Kots_CreateJobItem("DB Change");
    edict_t *ent = g_edicts + 1;

    job->function = Kots_CloseDbConnections;
    job->complete_callback = Kots_ServerAllowLogin;

    //print a message on the server console
    gi.dprintf("DB: Processing queue then closing all database connections...\n");

    //Log information before logging out clients
    Kots_LogOnDisconnect();

    //temporarily disable login
    allow_login = false;

    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (!ent->inuse)
            continue;

        if (!ent->character || !ent->character->is_loggedin)
            continue;

        //logout and alert all characters in game so we don't lose any data
        gi.centerprintf(ent, "Please wait...\nYou are being logged out by the server.\n");
        Kots_CharacterLogout(ent, true, false);
    }

    //add the job to the queue last so all of our saving is done first
    Kots_AddDbThreadJob(job);
}

void Kots_ServerSetAllowLogin(edict_t *admin, qboolean allow)
{
    server_login = allow;
#ifndef KOTS_TEST // Useful for logging in as admin, then disabling login
    //if login has been disabled by the server then log everyone out
    if (!server_login)
    {
        int i;
        edict_t *ent = g_edicts + 1;

        //print a message on the server console
        gi.cprintf(admin, PRINT_HIGH, "Login has been disabled.\n");
        gi.cprintf(admin, PRINT_HIGH, "Forcing all characters to logout. Please give this a minute...\n");

        //Log information before logging clients out
        Kots_LogOnDisconnect();

        for (i = 0; i < game.maxclients; i++, ent++)
        {
            if (!ent->inuse)
                continue;

            if (!ent->character || !ent->character->is_loggedin)
                continue;

            //logout and alert all characters in game so we don't lose any data
            gi.centerprintf(ent, "Please wait...\nYou are being logged out by the server.\n");
            Kots_CharacterLogout(ent, true, false);
        }
    }
#endif
    if (server_login)
    {
        //print a message on the server console
        gi.cprintf(admin, PRINT_HIGH, "Login has been enabled.\n");
    }
}

qboolean Kots_ServerCanLogin()
{
    return server_login;
}

static void LoginCharacterInternal(edict_t *ent, char *name, char *pass, char *ip_address, int client_id)
{
    jobitem_t *job = Kots_CreateJobItem("Login After Create");
    character_info_t *info = malloc(sizeof(*info));
    memset(&info->character, 0, sizeof(character_t));
    Kots_strncpy(info->name, name, 16);
    Kots_strncpy(info->pass, pass, 46);
    Kots_strncpy(info->ip_address, ip_address, 16);
    info->ent = ent;
    info->client_id = client_id;
    info->type = KOTS_CHARINFO_TYPE_LOGIN;

    job->args = info;
    job->wait_check = Kots_WaitForSave;
    job->function = MysqlLoginCharacter;
    job->complete_callback = Kots_ServerProcessCharacterInfoOutput;

    Kots_AddDbThreadJob(job);
}

static int MysqlExecQuery(dbthread_t *thread, jobitem_t *job, int querylength)
{
    int result, i;
    time_t start = time(NULL);
    time_t ellapsed;

    //if we got here and we're not connected then we already tried once and failed
    if (!thread->is_connected)
        return KOTS_SERVER_CONNECTIONDOWN;

    //try a few times to ensure the query is done
    for (i = 0 ; i < 3; i++)
    {
        result = mysql_real_query(thread->mysql, thread->query_buffer, querylength);

        //if successful then break early
        if (!result)
            break;
        else
        {
            //get the current time to determine how long it took
            ellapsed = time(NULL) - start;

            //ensure that we haven't been trying for too long
            if (ellapsed > 120)
                break;
        }
    }

    if (!result)
        return KOTS_SERVER_SUCCESS;
    else
    {
        //set the last error message
        const char *error = mysql_error(thread->mysql);

        if (error && *error)
            job->last_error = strdup(error);

        return KOTS_SERVER_CONNECTIONDOWN;
    }
}

static void MysqlLogInfo(dbthread_t *thread, jobitem_t *job)
{
    int querylength;
    char escaped_info[4192];
    loginfo_t *info = (loginfo_t *)job->args;
    MYSQL_ITERATOR *iterator;

    mysql_real_escape_string(thread->mysql, escaped_info, info->info, strlen(info->info));

    //perform query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `LogInfo`('%s');\n",
        escaped_info);

    //get iterator to work with results
    job->result = MysqlExecQuery(thread, job, querylength);
    iterator = mysql_iterator_create(thread->mysql);

    //free up all results
    mysql_iterator_free(iterator);
}

static void MysqlUpdateIpbans(dbthread_t *thread, jobitem_t *job)
{
    int querylength;
    MYSQL_ITERATOR *iterator;
    
    //perform query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `LoadIpBans`();\n");

    //get iterator to work with results
    job->result = MysqlExecQuery(thread, job, querylength);
    iterator = mysql_iterator_create(thread->mysql);
    if (job->result == KOTS_SERVER_SUCCESS)
    {
        if (iterator->result)
        {
            //ensure that the array is large enough for all runes and clear all current runes if any
            array_t *list = Array_Create(NULL, iterator->numrows, 0);
            job->args = list;

            //keep adding runes until there are no more
            while (iterator->currentrow)
            {
                char *ip = mysql_iterator_getvalue(iterator, "ip_address");

                if (ip != NULL)
                {
                    //create a copy of the ip to store in the array
                    ip = strdup(ip);

                    //add the ip to the array
                    Array_PushBack(list, ip);
                }

                //move on to the next row
                mysql_iterator_nextrow(iterator);
            }

            //set return value to success
            job->result = KOTS_SERVER_SUCCESS;
        }
        else
            job->result = KOTS_SERVER_UNKNOWNERROR;
    }
    
    //free the iterator
    mysql_iterator_free(iterator);
}

static void MysqlBanIp(dbthread_t *thread, jobitem_t *job)
{
    int querylength;
    char escaped_desc[1024];
    ipban_info_t *info = (ipban_info_t *)job->args;
    MYSQL_ITERATOR *iterator;

    mysql_real_escape_string(thread->mysql, escaped_desc, info->description, strlen(info->description));

    //perform query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `BanIp`('%s', '%s');\n",
        info->ip_address, escaped_desc);

    //get iterator to work with results
    job->result = MysqlExecQuery(thread, job, querylength);
    iterator = mysql_iterator_create(thread->mysql);

    //free up all results
    mysql_iterator_free(iterator);
}


static void MysqlCreateCharacter(dbthread_t *thread, jobitem_t *job)
{
    character_info_t *info = (character_info_t *)job->args;
    MYSQL_ITERATOR *iterator = NULL;
    char escaped_name[32];
    char escaped_pass[92];
    int name_length = strlen(info->name);
    int pass_length = strlen(info->pass);
    int querylength;

    //escape invalid chars in username and pass
    mysql_real_escape_string(thread->mysql, escaped_name, info->name, name_length);
    mysql_real_escape_string(thread->mysql, escaped_pass, info->pass, pass_length);

    //perform query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `CreateCharacter2`('%s', '%s', @return_val);\n"
        "select @return_val;",
        escaped_name, escaped_pass);

    //Execute the query and get an iterator
    job->result = MysqlExecQuery(thread, job, querylength);
    iterator = mysql_iterator_create(thread->mysql);

    if (job->result == KOTS_SERVER_SUCCESS)
    {
        //get iterator to work with results
        if (iterator->result)
        {
            //determine the return value
            job->result = mysql_iterator_getint_col(iterator, 0);
        }
        else
            job->result = KOTS_SERVER_UNKNOWNERROR;
    }
    
    //free the iterator
    mysql_iterator_free(iterator);

    //if the character creation was successful lets load it
    if (job->result == KOTS_SERVER_SUCCESS)
    {
        //TODO: Make it so we can go straight from here to login
        //      instead of adding to the queue again
        //MysqlLoginCharacter(thread, args)
        LoginCharacterInternal(info->ent, info->name, info->pass, info->ip_address, info->client_id);
    }
}

void MysqlSaveCharacter(dbthread_t *thread, jobitem_t *job)
{
    character_info_t *info = (character_info_t *)job->args;
    character_t *character = &info->character;
    character_persist_t *persist = &info->persist;
    MYSQL_ITERATOR *iterator = NULL;
    int querylength;

    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `SaveCharacter3`(%i, %i, %i, '%c', %i, %i, %i, %i, %i, %i, %i, %i, %i);\n"
        "call `SaveWeapon3`(%i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i);\n"
        "call `SavePlayer2`(%i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i);\n"
        "call `SavePower2`(%i, %i, %i, %i, %i, %i, %i, %i);\n"
        "call `SavePersist`(%i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i);\n"
        "call `SaveStats2`(%i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i);\n"
        "call `SaveSettings2`(%i, %i, %i, %i, %i, %i);",

        //save character parameters
        character->id, character->level, character->exp, character->gender,
        character->resist, character->cubes, character->credits, 
        character->respawn_weapon, character->respawns,
        character->is_loggedin, character->is_cursed, character->is_admin, character->respec_points,
        
        //save weapon parameters
        character->id, character->sabre, character->shotgun, character->supershotgun, 
        character->machinegun, character->chaingun, character->grenade, 
        character->grenadelauncher, character->rocketlauncher, character->hyperblaster,
        character->railgun, character->bfg, character->wantiweapon, character->weaponsbought, character->weaponpoints,

        //save player parameters
        character->id, character->dexterity, character->strength, character->karma, 
        character->wisdom, character->technical, character->spirit, 
        character->rage, character->vithealth, character->vitarmor,
        character->munition, character->playersbought, character->playerpoints,

        //save power parameters
        character->id, character->expack, character->spiral, character->bide, 
        character->kotsthrow, character->antiweapon, character->powersbought, character->powerpoints,

        //save persist parameters
        character->id, persist->health, persist->armor, persist->weapon, 
        persist->persist, persist->shotgun, persist->supershotgun,
        persist->machinegun, persist->chaingun, persist->grenadelauncher,
        persist->rocketlauncher, persist->hyperblaster, persist->railgun,
        persist->bfg, persist->shells, persist->bullets, persist->grenades,
        persist->rockets, persist->cells, persist->slugs,

        //save stats parameters
        character->id, character->kills, character->killed,
        character->telefrags, character->twofers, character->threefers,
        character->highestfer, character->sprees, character->spreewars,
        character->spreesbroken, character->spreewarsbroken, character->longestspree,
        character->suicides, character->teleports, character->timeplayed,
        character->total_credits, character->total_packs,

        //save settings parameters
        character->id, character->using_highjump, character->using_spiritswim, character->using_pconvert, character->laserhook_color, character->using_cgconvert
        );

    job->result = MysqlExecQuery(thread, job, querylength);

    //get iterator to work with results
    iterator = mysql_iterator_create(thread->mysql);

    //free up all results
    mysql_iterator_clearresults(iterator);
    mysql_iterator_free(iterator);
}

static void MysqlLoginCharacter(dbthread_t *thread, jobitem_t *job)
{
    character_info_t *info = (character_info_t *)job->args;
    character_t *character = &info->character;
    character_persist_t *persist = &info->persist;
    MYSQL_ITERATOR *iterator = NULL;
    char escaped_name[32];
    char escaped_pass[92];
    int name_length = strlen(info->name);
    int pass_length = strlen(info->pass);
    int querylength;

    //escape invalid chars in username and pass
    mysql_real_escape_string(thread->mysql, escaped_name, info->name, name_length);
    mysql_real_escape_string(thread->mysql, escaped_pass, info->pass, pass_length);

    //perform query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `Login3`('%s', '%s', '%s', @return_val);\n"
        "select @return_val;",
        escaped_name, escaped_pass, info->ip_address);

    //perform the query and create the iterator
    job->result = MysqlExecQuery(thread, job, querylength);
    iterator = mysql_iterator_create(thread->mysql);

    if (job->result == KOTS_SERVER_SUCCESS)
    {
        if (iterator->result)
        {
            if (mysql_iterator_hascolumn(iterator, "@return_val"))
                job->result = mysql_iterator_getint(iterator, "@return_val");
            
            else
            {
                //get character information
                character->id = mysql_iterator_getint(iterator, "id");
                character->level = mysql_iterator_getint(iterator, "level");
                character->exp = mysql_iterator_getint(iterator, "exp");
                character->gender = mysql_iterator_getchar(iterator, "gender");
                character->is_cursed = mysql_iterator_getint(iterator, "cursed");
                character->is_admin = mysql_iterator_getint(iterator, "isadmin");
                Kots_strncpy(character->title, mysql_iterator_getvalue(iterator, "title"), 51);
                character->resist = mysql_iterator_getint(iterator, "resist");
                character->cubes = mysql_iterator_getint(iterator, "cubes");
                character->credits = mysql_iterator_getint(iterator, "credits");
                character->rune_id = mysql_iterator_getint(iterator, "rune_id");
                character->respawn_weapon = mysql_iterator_getint(iterator, "respawn_weapon");
                character->respawns = mysql_iterator_getint(iterator, "respawns");
                character->sabre = mysql_iterator_getint(iterator, "sabre");
                character->shotgun = mysql_iterator_getint(iterator, "shotgun");
                character->supershotgun = mysql_iterator_getint(iterator, "supershotgun");
                character->machinegun = mysql_iterator_getint(iterator, "machinegun");
                character->chaingun = mysql_iterator_getint(iterator, "chaingun");
                character->grenade = mysql_iterator_getint(iterator, "grenade");
                character->grenadelauncher = mysql_iterator_getint(iterator, "grenadelauncher");
                character->rocketlauncher = mysql_iterator_getint(iterator, "rocketlauncher");
                character->hyperblaster = mysql_iterator_getint(iterator, "hyperblaster");
                character->railgun = mysql_iterator_getint(iterator, "railgun");
                character->bfg = mysql_iterator_getint(iterator, "bfg");
                character->wantiweapon = mysql_iterator_getint(iterator, "wantiweapon");
                character->weaponsbought = mysql_iterator_getint(iterator, "weaponsbought");
                character->weaponpoints = mysql_iterator_getint(iterator, "weaponpoints");
                character->dexterity = mysql_iterator_getint(iterator, "dexterity");
                character->strength = mysql_iterator_getint(iterator, "strength");
                character->karma = mysql_iterator_getint(iterator, "karma");
                character->wisdom = mysql_iterator_getint(iterator, "wisdom");
                character->technical = mysql_iterator_getint(iterator, "technical");
                character->spirit = mysql_iterator_getint(iterator, "spirit");
                character->rage = mysql_iterator_getint(iterator, "rage");
                character->vithealth = mysql_iterator_getint(iterator, "vithealth");
                character->vitarmor = mysql_iterator_getint(iterator, "vitarmor");
                character->munition = mysql_iterator_getint(iterator, "munition");
                character->playersbought = mysql_iterator_getint(iterator, "playersbought");
                character->playerpoints = mysql_iterator_getint(iterator, "playerpoints");
                character->expack = mysql_iterator_getint(iterator, "expack");
                character->spiral = mysql_iterator_getint(iterator, "spiral");
                character->bide = mysql_iterator_getint(iterator, "bide");
                character->kotsthrow = mysql_iterator_getint(iterator, "kotsthrow");
                character->antiweapon = mysql_iterator_getint(iterator, "antiweapon");
                character->powersbought = mysql_iterator_getint(iterator, "powersbought");
                character->powerpoints = mysql_iterator_getint(iterator, "powerpoints");
                character->respec_points = mysql_iterator_getint(iterator, "respec_points");

                //get persist info
                persist->health = mysql_iterator_getint(iterator, "health");
                persist->armor = mysql_iterator_getint(iterator, "armor");
                persist->weapon = mysql_iterator_getint(iterator, "weapon");
                persist->persist = mysql_iterator_getint(iterator, "persist");
                persist->shotgun = mysql_iterator_getint(iterator, "persist_shotgun");
                persist->supershotgun = mysql_iterator_getint(iterator, "persist_supershotgun");
                persist->machinegun = mysql_iterator_getint(iterator, "persist_machinegun");
                persist->chaingun = mysql_iterator_getint(iterator, "persist_chaingun");
                persist->grenadelauncher = mysql_iterator_getint(iterator, "persist_grenadelauncher");
                persist->rocketlauncher = mysql_iterator_getint(iterator, "persist_rocketlauncher");
                persist->hyperblaster = mysql_iterator_getint(iterator, "persist_hyperblaster");
                persist->railgun = mysql_iterator_getint(iterator, "persist_railgun");
                persist->bfg = mysql_iterator_getint(iterator, "persist_bfg");
                persist->shells = mysql_iterator_getint(iterator, "shells");
                persist->bullets = mysql_iterator_getint(iterator, "bullets");
                persist->grenades = mysql_iterator_getint(iterator, "grenades");
                persist->rockets = mysql_iterator_getint(iterator, "rockets");
                persist->cells = mysql_iterator_getint(iterator, "cells");
                persist->slugs = mysql_iterator_getint(iterator, "slugs");

                //get stats
                character->kills = mysql_iterator_getint(iterator, "kills");
                character->killed = mysql_iterator_getint(iterator, "killed");
                character->telefrags = mysql_iterator_getint(iterator, "telefrags");
                character->twofers = mysql_iterator_getint(iterator, "twofers");
                character->threefers = mysql_iterator_getint(iterator, "threefers");
                character->highestfer = mysql_iterator_getint(iterator, "highestfer");
                character->sprees = mysql_iterator_getint(iterator, "sprees");
                character->spreewars = mysql_iterator_getint(iterator, "spreewars");
                character->spreesbroken = mysql_iterator_getint(iterator, "spreesbroken");
                character->spreewarsbroken = mysql_iterator_getint(iterator, "spreewarsbroken");
                character->longestspree = mysql_iterator_getint(iterator, "longestspree");
                character->suicides = mysql_iterator_getint(iterator, "suicides");
                character->teleports = mysql_iterator_getint(iterator, "teleports");
                character->timeplayed = mysql_iterator_getint(iterator, "timeplayed");
                character->total_credits = mysql_iterator_getint(iterator, "total_credits");
                character->total_packs = mysql_iterator_getint(iterator, "total_packs");

                //get settings
                character->using_highjump = mysql_iterator_getint(iterator, "highjump");
                character->using_spiritswim = mysql_iterator_getint(iterator, "spiritswim");
                character->using_pconvert = mysql_iterator_getint(iterator, "pconvert");
                character->laserhook_color = mysql_iterator_getint(iterator, "laserhook_color");
                character->using_cgconvert = mysql_iterator_getint(iterator, "cgconvert");

                //set return value to success
                job->result = KOTS_SERVER_SUCCESS;
            }
        }
        else
            job->result = KOTS_SERVER_UNKNOWNERROR;
    }

    //free up all remaining results
    mysql_iterator_free(iterator);
}


static void MysqlLoadCharacter(dbthread_t *thread, jobitem_t *job)
{
    character_info_t *info = (character_info_t *)job->args;
    character_t *character = &info->character;
    MYSQL_ITERATOR *iterator = NULL;
    char escaped_name[32];
    int name_length = strlen(info->name);
    int querylength;

    //escape invalid chars in username
    mysql_real_escape_string(thread->mysql, escaped_name, info->name, name_length);

    //perform query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `Load2`('%s', @return_val);\n"
        "select @return_val;",
        escaped_name);

    //get iterator to work with results
    job->result = MysqlExecQuery(thread, job, querylength);
    iterator = mysql_iterator_create(thread->mysql);

    if (job->result == KOTS_SERVER_SUCCESS)
    {
        if (iterator->result)
        {
            if (mysql_iterator_hascolumn(iterator, "@return_val"))
                job->result = mysql_iterator_getint(iterator, "@return_val");
            else
            {
                //get character information
                //character->id = mysql_iterator_getint(iterator, "id");
                character->level = mysql_iterator_getint(iterator, "level");
                character->exp = mysql_iterator_getint(iterator, "exp");
                character->gender = mysql_iterator_getchar(iterator, "gender");
                character->is_cursed = mysql_iterator_getint(iterator, "cursed");
                character->is_admin = mysql_iterator_getint(iterator, "isadmin");
                Kots_strncpy(character->title, mysql_iterator_getvalue(iterator, "title"), 51);
                character->resist = mysql_iterator_getint(iterator, "resist");
                character->cubes = mysql_iterator_getint(iterator, "cubes");
                character->credits = mysql_iterator_getint(iterator, "credits");
                character->rune_id = mysql_iterator_getint(iterator, "rune_id");
                character->respawn_weapon = mysql_iterator_getint(iterator, "respawn_weapon");
                character->respawns = mysql_iterator_getint(iterator, "respawns");
                character->sabre = mysql_iterator_getint(iterator, "sabre");
                character->shotgun = mysql_iterator_getint(iterator, "shotgun");
                character->supershotgun = mysql_iterator_getint(iterator, "supershotgun");
                character->machinegun = mysql_iterator_getint(iterator, "machinegun");
                character->chaingun = mysql_iterator_getint(iterator, "chaingun");
                character->grenade = mysql_iterator_getint(iterator, "grenade");
                character->grenadelauncher = mysql_iterator_getint(iterator, "grenadelauncher");
                character->rocketlauncher = mysql_iterator_getint(iterator, "rocketlauncher");
                character->hyperblaster = mysql_iterator_getint(iterator, "hyperblaster");
                character->railgun = mysql_iterator_getint(iterator, "railgun");
                character->bfg = mysql_iterator_getint(iterator, "bfg");
                character->wantiweapon = mysql_iterator_getint(iterator, "wantiweapon");
                character->weaponpoints = mysql_iterator_getint(iterator, "weaponpoints");
                character->dexterity = mysql_iterator_getint(iterator, "dexterity");
                character->strength = mysql_iterator_getint(iterator, "strength");
                character->karma = mysql_iterator_getint(iterator, "karma");
                character->wisdom = mysql_iterator_getint(iterator, "wisdom");
                character->technical = mysql_iterator_getint(iterator, "technical");
                character->spirit = mysql_iterator_getint(iterator, "spirit");
                character->rage = mysql_iterator_getint(iterator, "rage");
                character->vithealth = mysql_iterator_getint(iterator, "vithealth");
                character->vitarmor = mysql_iterator_getint(iterator, "vitarmor");
                character->munition = mysql_iterator_getint(iterator, "munition");
                character->playerpoints = mysql_iterator_getint(iterator, "playerpoints");
                character->expack = mysql_iterator_getint(iterator, "expack");
                character->spiral = mysql_iterator_getint(iterator, "spiral");
                character->bide = mysql_iterator_getint(iterator, "bide");
                character->kotsthrow = mysql_iterator_getint(iterator, "kotsthrow");
                character->antiweapon = mysql_iterator_getint(iterator, "antiweapon");
                character->powerpoints = mysql_iterator_getint(iterator, "powerpoints");
                //character->respec_points = mysql_iterator_getint(iterator, "respec_points");

                //get stats
                character->kills = mysql_iterator_getint(iterator, "kills");
                character->killed = mysql_iterator_getint(iterator, "killed");
                character->telefrags = mysql_iterator_getint(iterator, "telefrags");
                character->twofers = mysql_iterator_getint(iterator, "twofers");
                character->threefers = mysql_iterator_getint(iterator, "threefers");
                character->highestfer = mysql_iterator_getint(iterator, "highestfer");
                character->sprees = mysql_iterator_getint(iterator, "sprees");
                character->spreewars = mysql_iterator_getint(iterator, "spreewars");
                character->spreesbroken = mysql_iterator_getint(iterator, "spreesbroken");
                character->spreewarsbroken = mysql_iterator_getint(iterator, "spreewarsbroken");
                character->longestspree = mysql_iterator_getint(iterator, "longestspree");
                character->suicides = mysql_iterator_getint(iterator, "suicides");
                character->teleports = mysql_iterator_getint(iterator, "teleports");
                character->timeplayed = mysql_iterator_getint(iterator, "timeplayed");

                //get settings
                character->using_highjump = mysql_iterator_getint(iterator, "highjump");
                character->using_spiritswim = mysql_iterator_getint(iterator, "spiritswim");
                character->using_pconvert = mysql_iterator_getint(iterator, "pconvert");
                character->laserhook_color = mysql_iterator_getint(iterator, "laserhook_color");
                character->using_cgconvert = mysql_iterator_getint(iterator, "cgconvert");

                //set return value to success
                job->result = KOTS_SERVER_SUCCESS;
            }
        }
        else
            job->result = KOTS_SERVER_UNKNOWNERROR;
    }

    //free up all remaining results
    mysql_iterator_free(iterator);
}

static void MysqlUpdateServer(dbthread_t *thread, jobitem_t *job)
{
    server_args_t *args = (server_args_t *)job->args;
    MYSQL_ITERATOR *iter = NULL;
    char escaped_name[256];
    char escaped_map[128];
    int name_length = strlen(args->info.name);
    int map_length = strlen(args->info.map);
    int querylength;

    //escape invalid chars in username and pass
    mysql_real_escape_string(thread->mysql, escaped_name, args->info.name, name_length);
    mysql_real_escape_string(thread->mysql, escaped_map, args->info.map, map_length);

    //perform query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `UpdateServer3`('%s', %i, %i, %i, '%s', %i);\n"
        "call `LoadServers`();",
        escaped_name, args->info.port, args->info.max_players, args->info.cur_players, escaped_map, args->info.is_public);

    //get iterator to work with results
    job->result = MysqlExecQuery(thread, job, querylength);
    iter = mysql_iterator_create(thread->mysql);

    if (job->result == KOTS_SERVER_SUCCESS)
    {
        if (iter->result)
        {
            //ensure that the array is large enough for all runes and clear all current runes if any
            args->servers = Array_Create(NULL, iter->numrows, 0);

            //keep adding runes until there are no more
            while (iter->currentrow)
            {
                server_info_t *info = malloc(sizeof(*info));
                Kots_strncpy(info->name, mysql_iterator_getvalue(iter, "name"), 128);
                Kots_strncpy(info->map, mysql_iterator_getvalue(iter, "map"), 64);
                Kots_strncpy(info->host, mysql_iterator_getvalue(iter, "hostname"), 128);
                info->port = mysql_iterator_getint(iter, "port");
                info->max_players = mysql_iterator_getint(iter, "max_players");
                info->cur_players = mysql_iterator_getint(iter, "cur_players");
                info->status = mysql_iterator_getint(iter, "status");

                //add the rune to the array and continue on
                Array_PushBack(args->servers, info);
                mysql_iterator_nextrow(iter);
            }

            //set return value to success
            job->result = KOTS_SERVER_SUCCESS;
        }
        else
            job->result = KOTS_SERVER_UNKNOWNERROR;
    }

    //free up all remaining results
    mysql_iterator_free(iter);
}

static void MysqlAdminLogin(dbthread_t *thread, jobitem_t *job)
{
    adminlogin_info_t *info = (adminlogin_info_t *)job->args;
    MYSQL_ITERATOR *iterator = NULL;
    char escaped_name[32];
    char escaped_pass[92];
    int name_length = strlen(info->name);
    int pass_length = strlen(info->pass);
    int querylength;

    //escape invalid chars in username and pass
    mysql_real_escape_string(thread->mysql, escaped_name, info->name, name_length);
    mysql_real_escape_string(thread->mysql, escaped_pass, info->pass, pass_length);

    //perform query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call `AdminLogin`('%s', '%s', '%s', @return_val);\n"
        "select @return_val;",
        escaped_name, escaped_pass, info->ip_address);

    //get iterator to work with results
    job->result = MysqlExecQuery(thread, job, querylength);
    iterator = mysql_iterator_create(thread->mysql);

    if (job->result == KOTS_SERVER_SUCCESS)
    {
        if (iterator->result)
        {
            //save the return value
            job->result = mysql_iterator_getint_col(iterator, 0);
        }
        else
        {
            job->result = KOTS_SERVER_UNKNOWNERROR;
        }
    }

    //free the iterator
    mysql_iterator_free(iterator);
}

static void MysqlLoadRunes(dbthread_t *thread, jobitem_t *job)
{
    MYSQL_ITERATOR *iter = NULL;
    array_t *runes;
    int querylength;

    //build the query
    Kots_strncpy(thread->query_buffer, "call LoadRunes()", KOTS_QUERYBUFFER_SIZE);
    querylength = strlen(thread->query_buffer);

    //rune the query and create an iterator
    job->result = MysqlExecQuery(thread, job, querylength);
    iter = mysql_iterator_create(thread->mysql);

    if (job->result == KOTS_SERVER_SUCCESS)
    {
        //if we don't have a result then some unknown error happened
        if (!iter->result)
            job->result = KOTS_SERVER_UNKNOWNERROR;
        else
        {
            //all is good so we'll return success
            job->result = KOTS_SERVER_SUCCESS;

            //ensure that the array is large enough for all runes
            job->args = Array_Create(NULL, iter->numrows, iter->numrows);
            runes = (array_t *)job->args;

            //keep adding runes until there are no more
            while (iter->currentrow)
            {
                rune_t *rune = malloc(sizeof(*rune));
                rune->id = mysql_iterator_getint(iter, "id");
                Kots_strncpy(rune->name, mysql_iterator_getvalue(iter, "name"), 51);
                Kots_strncpy(rune->pickup_text, mysql_iterator_getvalue(iter, "pickup_text"), 257);
                Kots_strncpy(rune->model_name, mysql_iterator_getvalue(iter, "model_name"), MAX_QPATH);
                Kots_strncpy(rune->image_name, mysql_iterator_getvalue(iter, "image_name"), MAX_QPATH);
                Kots_strncpy(rune->sound_name, mysql_iterator_getvalue(iter, "sound_name"), MAX_QPATH);
                Kots_strtov(mysql_iterator_getvalue(iter, "mins"), &rune->mins);
                Kots_strtov(mysql_iterator_getvalue(iter, "maxs"), &rune->maxs);
                rune->effects = mysql_iterator_getint(iter, "effects");
                rune->renderfx = mysql_iterator_getint(iter, "renderfx");
                rune->rarity = atof(mysql_iterator_getvalue(iter, "rarity"));

                rune->sabre = mysql_iterator_getint(iter, "sabre");
                rune->shotgun = mysql_iterator_getint(iter, "shotgun");
                rune->supershotgun = mysql_iterator_getint(iter, "supershotgun");
                rune->machinegun = mysql_iterator_getint(iter, "machinegun");
                rune->chaingun = mysql_iterator_getint(iter, "chaingun");
                rune->grenade = mysql_iterator_getint(iter, "grenade");
                rune->grenadelauncher = mysql_iterator_getint(iter, "grenadelauncher");
                rune->rocketlauncher = mysql_iterator_getint(iter, "rocketlauncher");
                rune->hyperblaster = mysql_iterator_getint(iter, "hyperblaster");
                rune->railgun = mysql_iterator_getint(iter, "railgun");
                rune->bfg = mysql_iterator_getint(iter, "bfg");
                rune->dexterity = mysql_iterator_getint(iter, "dexterity");
                rune->strength = mysql_iterator_getint(iter, "strength");
                rune->karma = mysql_iterator_getint(iter, "karma");
                rune->wisdom = mysql_iterator_getint(iter, "wisdom");
                rune->technical = mysql_iterator_getint(iter, "technical");
                rune->spirit = mysql_iterator_getint(iter, "spirit");
                rune->rage = mysql_iterator_getint(iter, "rage");
                rune->vithealth = mysql_iterator_getint(iter, "vithealth");
                rune->vitarmor = mysql_iterator_getint(iter, "vitarmor");
                rune->munition = mysql_iterator_getint(iter, "munition");
                rune->expack = mysql_iterator_getint(iter, "expack");
                rune->spiral = mysql_iterator_getint(iter, "spiral");
                rune->bide = mysql_iterator_getint(iter, "bide");
                rune->kotsthrow = mysql_iterator_getint(iter, "throw");
                rune->antiweapon = mysql_iterator_getint(iter, "antiweapon");
                rune->tballs = mysql_iterator_getint(iter, "tballs");
                rune->tball_regen = mysql_iterator_getint(iter, "tball_regen");
                rune->tball_speed = mysql_iterator_getint(iter, "tball_speed");
                rune->normal_resist = mysql_iterator_getint(iter, "normal_resist");
                rune->energy_resist = mysql_iterator_getint(iter, "energy_resist");

                //add the rune to the array and continue on
                Array_PushBack(runes, rune);
                mysql_iterator_nextrow(iter);
            }
        }
    }

    //free up all remaining results
    mysql_iterator_free(iter);
}

static void MysqlLoadMuteList(dbthread_t *thread, jobitem_t *job)
{
    MYSQL_ITERATOR *iter = NULL;
    array_t *list;
    int querylength;

    //build the query
    Kots_strncpy(thread->query_buffer, "call `LoadMuteList`();", KOTS_QUERYBUFFER_SIZE);
    querylength = strlen(thread->query_buffer);

    //rune the query and create an iterator
    job->result = MysqlExecQuery(thread, job, querylength);
    iter = mysql_iterator_create(thread->mysql);

    if (job->result == KOTS_SERVER_SUCCESS)
    {
        //if we don't have a result then some unknown error happened
        if (!iter->result)
            job->result = KOTS_SERVER_UNKNOWNERROR;
        else
        {
            //ensure that the array is large enough for all results
            job->args = Array_Create(NULL, iter->numrows, iter->numrows);
            list = (array_t *)job->args;

            //keep adding mutes until there are no more
            while (iter->currentrow)
            {
                mute_entry_t *entry = malloc(sizeof(*entry));
                entry->character_id = mysql_iterator_getint(iter, "character_id");
                Kots_strncpy(entry->name, mysql_iterator_getvalue(iter, "name"), sizeof(entry->name));
                Kots_strncpy(entry->ip_address, mysql_iterator_getvalue(iter, "ip_address"), sizeof(entry->ip_address));

                //add the rune to the array and continue on
                Array_PushBack(list, entry);
                mysql_iterator_nextrow(iter);
            }

            //all is good so we'll return success
            job->result = KOTS_SERVER_SUCCESS;
        }
    }

    //free up all remaining results
    mysql_iterator_free(iter);
}

static void MysqlMutePlayer(dbthread_t *thread, jobitem_t *job)
{
    MYSQL_ITERATOR *iter = NULL;
    mute_info_t *info = (mute_info_t *)job->args;
    char escaped_name[32];
    char escaped_ip[32];
    int name_length = strlen(info->name);
    int ip_length = strlen(info->ip_address);
    int querylength;

    //escape invalid chars in username and ip
    mysql_real_escape_string(thread->mysql, escaped_name, info->name, name_length);
    mysql_real_escape_string(thread->mysql, escaped_ip, info->ip_address, ip_length);

    //build the query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call MutePlayer('%s', '%s', '%i')",
        escaped_ip, escaped_name, info->character_id);

    //rune the query and create an iterator
    job->result = MysqlExecQuery(thread, job, querylength);
    iter = mysql_iterator_create(thread->mysql);

    //free up all remaining results
    mysql_iterator_free(iter);
}

static void MysqlUnmutePlayer(dbthread_t *thread, jobitem_t *job)
{
    MYSQL_ITERATOR *iter = NULL;
    mute_info_t *info = (mute_info_t *)job->args;
    char escaped_ip[32];
    int ip_length = strlen(info->ip_address);
    int querylength;

    //escape invalid chars in ip
    mysql_real_escape_string(thread->mysql, escaped_ip, info->ip_address, ip_length);

    //build the query
    querylength = Kots_snprintf(thread->query_buffer, KOTS_QUERYBUFFER_SIZE,
        "call UnmutePlayer('%s')", escaped_ip);

    //rune the query and create an iterator
    job->result = MysqlExecQuery(thread, job, querylength);
    iter = mysql_iterator_create(thread->mysql);

    //free up all remaining results
    mysql_iterator_free(iter);
}




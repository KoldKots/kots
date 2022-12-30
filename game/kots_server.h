#ifndef KOTS_SERVER_H
#define KOTS_SERVER_H

#ifdef KOTS_OFFICIAL

#include "kots_official.h"

#else

#define MYSQL_HOST      "localhost"
#define MYSQL_USER      "user"
#define MYSQL_PASS      "pass"
#define MYSQL_DB        "kots" //live server
#define MYSQL_PORT      3306

#endif


#define KOTS_SERVER_SUCCESS             0
#define KOTS_SERVER_CONNECTIONDOWN      1
#define KOTS_SERVER_INVALIDCHAR         2
#define KOTS_SERVER_INVALIDPASS         3
#define KOTS_SERVER_ALREADYLOGGEDIN     4
#define KOTS_SERVER_ALREADYEXISTS       5
#define KOTS_SERVER_UNKNOWNERROR        6
#define KOTS_SERVER_NOTADMIN            7
#define KOTS_SERVER_LOGIN_DISABLED      8

#define KOTS_CHARINFO_TYPE_LOGIN        1
#define KOTS_CHARINFO_TYPE_SAVE         2
#define KOTS_CHARINFO_TYPE_KOTSINFO     3
#define KOTS_CHARINFO_TYPE_POINTLIST    4

#include "kots_dbthread.h"
#include "kots_character.h"
#include "kots_mysql_helper.h"
#include "kots_array.h"
#include "kots_mute.h"

typedef struct loginfo_s
{
    char info[2048];
} loginfo_t;

typedef struct
{
    edict_t *ent;
    int client_id;
    char *message;
} client_message_t;

typedef struct
{
    char name[16];
    char pass[46];
    char ip_address[16];
    int client_id;
    edict_t *ent; //do not modify contents unless in main game thread
    character_t character;
    character_persist_t persist;
    int type;
} character_info_t;

typedef struct
{
    char name[16];
    char pass[46];
    char ip_address[16];
    int client_id;
    edict_t *ent; //do not modify contents unless in main game thread
} adminlogin_info_t;

typedef struct
{
    char name[128];
    char map[64];
    char host[256];
    int port;
    int max_players;
    int cur_players;
    int status;
    int is_public;
} server_info_t;

typedef struct
{
    edict_t *admin;
    edict_t *banned;
    char ip_address[16];
    char description[512];
} ipban_info_t;

typedef struct
{
    server_info_t info;
    array_t *servers;
} server_args_t;

//declare this variable for outside use
extern array_t *kots_servers;

void Kots_InitializeServer();
void Kots_FreeServer();

void Kots_AddRuneUpdateJob(void (*complete_callback)(struct jobitem_s *job));
void Kots_AddIpbanUpdateJob();
void Kots_ServerBanIp(edict_t *admin, edict_t *banned);
qboolean Kots_CheckIpBan(edict_t *ent);
qboolean Kots_ServerIsClientValid(edict_t *ent, int client_id);
void Kots_ServerCreateCharacter(edict_t *ent);
void Kots_ServerSaveCharacter(edict_t *ent);
void Kots_ServerLoginCharacter(edict_t *ent);
void Kots_ServerLoadKotsInfo(edict_t *ent, char *name);
void Kots_ServerLoadPointlist(edict_t *ent, char *name);
void Kots_ServerAdminLogin(edict_t *ent);
void Kots_ServerAddClientMessage(edict_t *ent, int client_id, char *message);
void Kots_ServerAddServerMessage(char *message);
void Kots_ServerAddServerCommand(char *message);
void Kots_ServerDbNameChanged();
void Kots_ServerProcessOutput();
void Kots_ServerSetAllowLogin(edict_t *admin, qboolean allow);
qboolean Kots_ServerCanLogin();
void Kots_ServerLogInfo(char *reason);
void Kots_DebugDbInfo(edict_t *ent);
void Kots_AddMuteListUpdateJob();
void Kots_AddMuteJob(mute_info_t *args);
void Kots_AddUnmuteJob(mute_info_t *args);
void Kots_AddServerUpdateJob();
void Kots_ServerAddDebugMessage(char *message);


#endif

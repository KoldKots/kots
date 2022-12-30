#ifndef KOTS_MAPLIST_H
#define KOTS_MAPLIST_H

#include "g_local.h"
#include "kots_array.h"

#define MAPLIST_CAPACITY        256
#define MAPLIST_GROWSIZE        128
#define MAPLIST_MINMAPS         3
#define MAPLIST_MAX_FILENAME    257

#define MAPVOTE_NONE            0
#define MAPVOTE_YES             1
#define MAPVOTE_NO              2

typedef enum
{
    ML_ROTATE_SEQ,
    ML_ROTATE_RANDOM
} maplist_rotation_t;

typedef struct
{
    int min_players;                        //minimum number of players on server for this map to be picked
    int max_players;                        //maximum number of players on server for this map to be picked
    int priority;                           //priority over other maps
    char mapname[MAPLIST_MAX_FILENAME];     //name of the map
    int index;
} mapentry_t;
       
typedef struct
{
    char filename[MAPLIST_MAX_FILENAME];    //filename on server
    array_t *maps;                      //array of mapentry_t's in the maplist
    maplist_rotation_t rotationflag;        //type of rotation to follow
    int  currentmap;                        //index of current map
} maplist_t;

typedef struct
{
    int mapindex;           //index of map
    int fails;              //number of failed attempts to vote for the map
} votefail_t;

extern maplist_t maplist;

void Kots_Maplist_Init();
void Kots_Maplist_Free();
int Kots_Maplist_Load(char *filename);
int Kots_Maplist_GetRandomMap();
int Kots_Maplist_GetNextMap();
char *Kots_Maplist_GetName(int index);
void Kots_Maplist_ClearMapList();
void Kots_Maplist_AddPrevMap();
void Kots_Maplist_Show(edict_t *ent);
void Kots_Maplist_ServerCommand();
void Kots_Maplist_ServerDisplayUsage(edict_t *ent);
void Kots_Maplist_Goto(edict_t *ent, char *mapname);
void Kots_Maplist_ServerChangeRotation();
void Kots_Maplist_Vote(edict_t *ent, int vote);
void Kots_Maplist_VoteCommand(edict_t *ent);
void Kots_Maplist_ClearVoteInfo();
void Kots_Maplist_CheckVoteTimeout();
void Kots_Maplist_UpdateMapIndex();
void Kots_Maplist_RemoveMap(ULONG mapindex);
void Kots_Maplist_PrintVoteInfo(edict_t *ent);

#endif

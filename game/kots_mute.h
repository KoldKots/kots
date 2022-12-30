#include "kots_character.h"
#include "kots_dbthread.h"

#ifndef KOTS_MUTE_H
#define KOTS_MUTE_H

typedef struct
{
    char ip_address[16];
    char name[16];
    int character_id;
} mute_entry_t;

typedef struct
{
    edict_t *admin;
    int admin_id;
    edict_t *player;
    int player_id;
    char ip_address[16];
    char name[16];
    int character_id;
} mute_info_t;

void Kots_MuteInit();
void Kots_MuteFree();
void Kots_MuteListAdd(mute_info_t *entry);
void Kots_MuteListRemove(mute_info_t *entry);
void Kots_MuteCheckAll();
void Kots_MuteCheck(edict_t *ent);
void Kots_UpdateMuteList(jobitem_t *job);
void Kots_MutePlayer(edict_t *admin, char *name);
void Kots_MuteIP(edict_t *admin, char *ip_address);
void Kots_UnmutePlayer(edict_t *admin, char *name);
void Kots_UnmuteIP(edict_t *admin, char *ip_address);
void Kots_MuteList(edict_t *ent);
void Kots_MuteComplete(jobitem_t *job);
void Kots_UnmuteComplete(jobitem_t *job);

#endif

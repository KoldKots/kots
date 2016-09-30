#ifndef KOTS_ADMIN_H
#define KOTS_ADMIN_H

#include "kots_character.h"

edict_t *Kots_AdminFindPlayer(char *name, edict_t *ignore);
edict_t *Kots_AdminFindPlayerByIp(char *ip_address);
void Kots_Admin_CreateRune(edict_t *ent, char *args);
void Kots_Admin_SpawnMonster(edict_t *ent, char *args);
void Kots_Admin_ListIP (edict_t *ent);
void Kots_Admin_KickUser (edict_t *ent, edict_t *kick);
void Kots_Admin_Curse(edict_t *ent, edict_t *other);
void Kots_Admin_KickByName(edict_t *admin, char *name);
void Kots_Admin_CurseByName(edict_t *admin, char *name);
void Kots_Admin_IpBan(edict_t *admin, char *name);

void Kots_AdminSilence(edict_t *ent);
void Kots_AdminUnsilence(edict_t *ent);

#endif

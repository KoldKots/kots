#ifndef KOTS_MENU_H
#define KOTS_MENU_H

#include "kots_character.h"

qboolean Kots_MenuCloseIfOpen(edict_t *ent, char *name);
qboolean Kots_MenuIsOpen(edict_t *ent, char *name);

// Aldarn
void Kots_MenuClose(edict_t *ent, pmenuhnd_t *hnd);
void Kots_VoteMonOff(edict_t *ent, pmenuhnd_t *hnd);
void Kots_VoteMonMixed(edict_t *ent, pmenuhnd_t *hnd);
void Kots_VoteMonOnly(edict_t *ent, pmenuhnd_t *hnd);
void Kots_VoteMenuShow(edict_t *ent, pmenuhnd_t *hnd);

void Kots_MenuShow(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuShowCharacterInfo(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuShowOtherCharacterInfo(edict_t *ent, edict_t *other, character_t *character);
void Kots_MenuShowPointlist(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuShowOtherPointlist(edict_t *ent, character_t *character);
void Kots_MenuShowPlayer(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuShowWeapon(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuShowPower(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuShowResist(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuShowRespawn(edict_t *ent, pmenuhnd_t *hnd);

void Kots_MenuPlayerAdd(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuWeaponAdd(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuPowerAdd(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuResistSet(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuRespawnSet(edict_t *ent, pmenuhnd_t *hnd);

void Kots_MenuShowBuy(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuBuySelectSpawns(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuBuySpawns(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuBuyPlayer(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuBuyWeapon(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuBuyPower(edict_t *ent, pmenuhnd_t *hnd);

void Kots_MenuServers(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuCreateServers(edict_t *ent);
void Kots_MenuServersConnect(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuServersPrevPage(edict_t *ent, pmenuhnd_t *hnd);
void Kots_MenuServersNextPage(edict_t *ent, pmenuhnd_t *hnd);

#endif

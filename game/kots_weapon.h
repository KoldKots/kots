#ifndef KOTS_WEAPON_H
#define KOTS_WEAPON_H

#include "kots_character.h"

typedef struct
{
    char *name;
    int weapon;
    void (*add)(edict_t *ent);
    void (*resist)(edict_t *ent);
} weaponpoint_t;

weaponpoint_t *Kots_GetWeaponTree(char *name);
int Kots_CharacterGetCurrentWeaponLevel(edict_t *ent, int weapon);
qboolean Kots_CharacterCanAddWeapon(edict_t *ent, int cost);
void Kots_CharacterAddSabre(edict_t *ent);
void Kots_CharacterAddShotgun(edict_t *ent);
void Kots_CharacterAddMachinegun(edict_t *ent);
void Kots_CharacterAddChaingun(edict_t *ent);
void Kots_CharacterAddSuperShotgun(edict_t *ent);
void Kots_CharacterAddGrenade(edict_t *ent);
void Kots_CharacterAddGrenadeLauncher(edict_t *ent);
void Kots_CharacterAddRocketLauncher(edict_t *ent);
void Kots_CharacterAddHyperblaster(edict_t *ent);
void Kots_CharacterAddRailgun(edict_t *ent);
void Kots_CharacterAddBfg(edict_t *ent);
void Kots_CharacterAddWAntiweapon(edict_t *ent);
void Kots_CharacterAddWeapon(edict_t *ent, char *weapon);
void Kots_CharacterSetWeapon(edict_t *ent);

#endif

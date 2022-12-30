#ifndef KOTS_WEAPONDAMAGE_H
#define KOTS_WEAPONDAMAGE_H

#include "kots_character.h"

int Kots_CharacterSabreDamage(edict_t *ent, int damage);
int Kots_CharacterShotgunDamage(edict_t *ent, int damage);
int Kots_CharacterSuperShotgunDamage(edict_t *ent, int damage);
int Kots_CharacterMachinegunDamage(edict_t *ent, int damage);
int Kots_CharacterChaingunDamage(edict_t *ent, int damage);
int Kots_CharacterGrenadeDamage(edict_t *ent, int damage);
int Kots_CharacterGrenadeLauncherDamage(edict_t *ent, int damage);
int Kots_CharacterRocketLauncherDamage(edict_t *ent, int damage);
int Kots_CharacterHyperblasterDamage(edict_t *ent, int damage);
int Kots_CharacterRailgunDamage(edict_t *ent, int damage);
int Kots_CharacterBfgDamage(edict_t *ent, int damage);
int Kots_CharacterMunitionDamage(edict_t *ent, int damage);
int Kots_CharacterMunitionDamagePlus(edict_t *ent, float damage, int weapon_level);

#endif

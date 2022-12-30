#ifndef KOTS_RESIST_H
#define KOTS_RESIST_H

#include "kots_character.h"

qboolean Kots_CharacterCanResist(edict_t *ent);

//Power resist methods
void Kots_CharacterResistExpack(edict_t *ent);
void Kots_CharacterResistSpiral(edict_t *ent);
void Kots_CharacterResistBide(edict_t *ent);
void Kots_CharacterResistThrow(edict_t *ent);

//Weapon resist methods
void Kots_CharacterResistSabre(edict_t *ent);
void Kots_CharacterResistShotgun(edict_t *ent);
void Kots_CharacterResistMachinegun(edict_t *ent);
void Kots_CharacterResistChaingun(edict_t *ent);
void Kots_CharacterResistSuperShotgun(edict_t *ent);
void Kots_CharacterResistGrenade(edict_t *ent);
void Kots_CharacterResistGrenadeLauncher(edict_t *ent);
void Kots_CharacterResistRocketLauncher(edict_t *ent);
void Kots_CharacterResistHyperblaster(edict_t *ent);
void Kots_CharacterResistRailgun(edict_t *ent);
void Kots_CharacterResistBfg(edict_t *ent);

#endif

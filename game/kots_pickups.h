#ifndef KOTS_PICKUPS_H
#define KOTS_PICKUPS_H

#include "kots_character.h"

int Kots_CharacterGetMaxAmmoBonus(edict_t *ent);
int Kots_CharacterGetMaxAmmo(edict_t *ent, int basemax);
void Kots_CharacterPickupAmmo(edict_t *ent, gitem_t *ammo, int amount);
int Kots_CharacterGetAmmoPickup(edict_t *ent, int baseammo);
bool Kots_CharacterPickupArmor(edict_t *ent, edict_t *other);
bool Kots_CharacterPickupHealth(edict_t *ent, edict_t *other);
void Kots_CharacterCheckAmmoRot(edict_t *ent);

#endif

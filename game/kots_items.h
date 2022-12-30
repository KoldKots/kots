#ifndef KOTS_ITEMS_H
#define KOTS_ITEMS_H

#include "kots_character.h"

typedef struct pack_s
{
    //longer to account for monster full name including level
    char owner_name[100];

    //has weapons?
    qboolean shotgun;
    qboolean supershotgun;
    qboolean machinegun;
    qboolean chaingun;
    qboolean grenadelauncher;
    qboolean rocketlauncher;
    qboolean hyperblaster;
    qboolean railgun;
    qboolean bfg;

    //ammo
    int shells;
    int bullets;
    int grenades;
    int rockets;
    int cells;
    int slugs;

    //credits and cubes
    int credits;
    int cubes;

} pack_t;

void Kots_GiveWeapon(edict_t *ent, int index, qboolean give);
edict_t *Kots_PackDrop(edict_t *targ, edict_t *attacker);
qboolean Kots_PackPickup(edict_t *pack, edict_t *ent);
void Kots_PackMakeTouchable(edict_t *ent);
qboolean Kots_FakeHealthPickup(edict_t *item, edict_t *ent);
void Kots_CharacterDropFakeHealth(edict_t *ent, char *pickup_name);
qboolean Kots_MinePickup(edict_t *item, edict_t *ent);
void Kots_MineMakeTouchable(edict_t *ent);
void Kots_CharacterDropStims(edict_t *ent);
void Kots_CharacterDropShards(edict_t *ent);
qboolean Kots_FakeShardPickup(edict_t *item, edict_t *ent);
void Kots_CharacterDropFakeShard(edict_t *ent);
void Kots_TelefragItem(edict_t *ent);
void Kots_FreeItem(edict_t *ent);

#endif

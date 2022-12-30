#ifndef KOTS_TBALLS_H
#define KOTS_TBALLS_H

#include "kots_character.h"

//Quake 2 function prototypes
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
qboolean KillBox( edict_t *ent );
edict_t *SelectDeathmatchSpawnPoint(edict_t *ent);

//kots function prototypes
qboolean Kots_Teleport(edict_t *other);
void Kots_RadiusTeleport(edict_t *ent);
void Kots_Use_T_Ball(edict_t *ent, char *args);

#endif

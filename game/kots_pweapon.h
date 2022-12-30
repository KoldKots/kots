#include "g_local.h"

#ifndef KOTS_PWEAPON_H
#define KOTS_PWEAPON_H

//Variables moved to this header in order to allow separation
//of normal q2 weapons from kots weapons

extern qboolean is_quad;
extern byte     is_silenced;

//function prototypes needed by kots weapons
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void VectorCreateVelocity (vec3_t start, vec3_t end, vec3_t velocity);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void fire_blaster_ex (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int renderfx, qboolean stun, int mod);
edict_t *CreateLaser(edict_t *owner, vec3_t start, vec3_t end, int effects, int color, int diameter, int final_diameter, float time);

#endif

#ifndef KOTS_HOOK_H
#define KOTS_HOOK_H

#include "kots_character.h"

void CTFPlayerResetGrapple(edict_t *ent);
void CTFResetGrapple(edict_t *self);
void CTFGrappleTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void CTFGrappleDrawCable(edict_t *self);
void CTFGrapplePull(edict_t *self);
void CTFGrappleFire(edict_t *ent, vec3_t g_offset, int damage, int effect);
void CTFWeapon_Grapple(edict_t *ent);

#endif


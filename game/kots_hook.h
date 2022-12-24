#ifndef KOTS_HOOK_H
#define KOTS_HOOK_H

#include "kots_character.h"

typedef enum
{
    LASERHOOK_COLOR_RED = 0xf0f0f0f0,
    LASERHOOK_COLOR_BLUE = 0xf1f1f1f1,
    LASERHOOK_COLOR_GREEN = 0xd0d1d2d3,
    LASERHOOK_COLOR_YELLOW = 0xdcdddedf,
    LASERHOOK_COLOR_ORANGE = 0xe0e1e2e3,
} laserhook_color_e;

void CTFPlayerResetGrapple(edict_t *ent);
void CTFResetGrapple(edict_t *self);
void CTFGrappleTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void CTFGrapplePull(edict_t *self);
void CTFGrappleFire(edict_t *ent, vec3_t g_offset, int damage, int effect);
void CTFWeapon_Grapple(edict_t *ent);
void CTFGrapple_Think(edict_t *self);

#endif


#include "g_local.h"

#ifndef KOTS_SABRE_H
#define KOTS_SABRE_H

//function definitions for sabre
void fire_sabre (edict_t *self, vec3_t start, vec3_t dir, int damage, int length, int color);
void weapon_sabre_fire (edict_t *ent);
void Weapon_Sabre (edict_t *ent);
void create_laser (edict_t *owner, vec3_t start, vec3_t end, int color, int diameter, float time);

#endif

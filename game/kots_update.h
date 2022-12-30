#ifndef KOTS_UPDATE_H
#define KOTS_UPDATE_H

#include "kots_character.h"

void Kots_UpdateInit();
void Kots_UpdateFree();
void Kots_UpdateStartDefault(edict_t *ent);
void Kots_UpdateStart(edict_t *ent, char *url, int wait);
void Kots_RevertStartDefault(edict_t *ent);
void Kots_RevertStart(edict_t *ent, int wait);

#endif

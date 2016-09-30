#ifndef KOTS_HUD_H
#define KOTS_HUD_H

#define KOTS_INTERMISSION_DELAY			5.0 /* delay before moving to the next screen */

#define KOTS_INTERMISSION_SCORE			0
#define KOTS_INTERMISSION_ACCURACY		1
#define KOTS_INTERMISSION_KILLRATIO		2
#define KOTS_INTERMISSION_DAMAGE		3
#define KOTS_INTERMISSION_EXP			4
#define KOTS_INTERMISSION_PACKS			5

#include "kots_character.h"

void Kots_HudShowStats(edict_t *ent, char *args);
void Kots_HudScoreboard(edict_t *ent);
void Kots_HudIntermissionCheckScreen();
void Kots_HudResetIntermissionClicked(edict_t *ent);
qboolean Kots_HudHasIntermissionClicked(edict_t *ent);
void Kots_HudIntermissionClick(edict_t *ent);

#endif

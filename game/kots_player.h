#ifndef KOTS_PLAYER_H
#define KOTS_PLAYER_H

#include "kots_character.h"

typedef struct
{
	char *name;
	int player;
	void (*add)(edict_t *ent);
} playerpoint_t;

playerpoint_t *Kots_GetPlayerTree(char *name);
qboolean Kots_CharacterCanAddPlayer(edict_t *ent, int level, int max, int cost);
int Kots_CharacterGetNextPlayerLevelCost(edict_t *ent, int player, int level);
void Kots_CharacterAddDexterity(edict_t *ent);
void Kots_CharacterAddStrength(edict_t *ent);
void Kots_CharacterAddKarma(edict_t *ent);
void Kots_CharacterAddWisdom(edict_t *ent);
void Kots_CharacterAddTechnical(edict_t *ent);
void Kots_CharacterAddSpirit(edict_t *ent);
void Kots_CharacterAddRage(edict_t *ent);
void Kots_CharacterAddVitHealth(edict_t *ent);
void Kots_CharacterAddVitArmor(edict_t *ent);
void Kots_CharacterAddMunition(edict_t *ent);
void Kots_CharacterAddPlayer(edict_t *ent, char *player);
void Kots_CharacterSetPlayer(edict_t *ent);
qboolean Kots_CharacterCheckLevelCap(edict_t *ent, int player, int total);
void Kots_CharacterRespecPlayer(edict_t *ent, char *player);

#endif

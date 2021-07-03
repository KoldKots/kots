#ifndef KOTS_POWER_H
#define KOTS_POWER_H

#include "kots_character.h"

#define KOTS_BIDE_MAX_RADIUS	768
#define KOTS_BIDE_MIN_RADIUS	125
#define KOTS_BIDE_START_CAP		500

#define KOTS_SPIRAL_DAMAGE_CAP	5000.0
#define KOTS_SPIRAL_DAMAGE		100.0
#define KOTS_SPIRAL_TIME		10.0	/*Spiral time in seconds*/
#define KOTS_SPIRAL_MAX_RADIUS	64		/*Spiral maximum radius size*/
#define KOTS_SPIRAL_MIN_RADIUS	30		/*Spiral minimum radius size*/
#define KOTS_SPIRAL_FORWARD		128		/*Distance the spiral should go forward*/
#define KOTS_SPIRAL_BEAMSIZE	30		/*Size of each beam*/
#define KOTS_SPIRAL_SPEED		8		/*Speed of rotation in degrees per frame*/

#define KOTS_POWER_TRAINING_LIMIT_LEVEL_CAP	30

typedef struct
{
	char *name;
	int power;
	void (*add)(edict_t *ent);
	void (*resist)(edict_t *ent);
} powerpoint_t;

powerpoint_t *Kots_GetPowerTree(char *name);
qboolean Kots_CharacterCanAddPower(edict_t *ent);
void Kots_CharacterAddExpack(edict_t *ent);
void Kots_CharacterAddSpiral(edict_t *ent);
void Kots_CharacterAddBide(edict_t *ent);
void Kots_CharacterAddThrow(edict_t *ent);
void Kots_CharacterAddAntiweapon(edict_t *ent);
void Kots_ExpackExplode(edict_t *ent);
qboolean Kots_ExpackTouch(edict_t *ent, edict_t *other);
void Kots_ExpackDrop(edict_t *ent);
void Kots_CharacterThrow(edict_t *ent, char *args);
void Kots_CharacterBide(edict_t *ent);
void Kots_CharacterBideCheckTimer(edict_t *ent);
void Kots_CharacterBideExplode(edict_t *ent);
void Kots_CharacterThrowOther(edict_t *ent);
void Kots_CharacterThrowSelf(edict_t *ent);
void Kots_CharacterThrowUp(edict_t *ent);
void Kots_SpiralThink(edict_t *ent);
void Kots_SpiralBeamThink(edict_t *ent);
void Kots_CharacterSpiral(edict_t *ent, char *args);
void Kots_CharacterCreateSpiral(edict_t *ent, qboolean forward);
void Kots_CharacterAddPower(edict_t *ent, char *power);
void Kots_CharacterSetPower(edict_t *ent);
qboolean Kots_PowerCharacterCheckLevelCap(edict_t *ent, int power, int total);

#endif

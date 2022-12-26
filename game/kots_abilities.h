#ifndef KOTS_ABILITIES_H
#define KOTS_ABILITIES_H

#include "kots_character.h"

//Quake 2 Prototypes
void Use_Weapon(edict_t *ent, gitem_t *item);
void Drop_Weapon(edict_t *ent, gitem_t *item);


void Kots_CheckFastWeaponSwitch(edict_t *ent);
void Kots_CharacterDoDamageBasedSkills(edict_t *targ, edict_t *attacker, int health, int total, int realtotal);
void Kots_CharacterFly(edict_t *ent);
void Kots_CharacterLand(edict_t *ent);
void Kots_CharacterHighJumpToggle(edict_t *ent, char *value);
void Kots_CharacterCheckDoubleJump(edict_t *ent, pmove_t *pm);
void Kots_CharacterCheckFly(edict_t *ent, pmove_t *pm);
void Kots_CharacterFlyUseCubes(edict_t *ent);
void Kots_CharacterCheckHealthRegen(edict_t *ent);
void Kots_CharacterCheckArmorRegen(edict_t *ent);
void Kots_CharacterCheckTballRegen(edict_t *ent);
void Kots_CharacterCheckAmmoRegen(edict_t *ent);
void Kots_CharacterLiftItem(edict_t *ent);
void Kots_CharacterTossItem(edict_t *ent);
void Kots_CharacterToggleCloaking(edict_t *ent, char *value);
void Kots_CharacterCloak(edict_t *ent);
void Kots_CharacterUncloak(edict_t *ent);
void Kots_CharacterCloakUseCubes(edict_t *ent);
void Kots_CharacterCheckCloak(edict_t *ent);
void Kots_CharacterToggleFlashlight(edict_t *ent, char *value);
void Kots_CharacterFlashlightOn(edict_t *ent);
void Kots_CharacterFlashlightOff(edict_t *ent);
void Kots_CharacterFlashlightThink(edict_t *self);
void Kots_CharacterHook(edict_t *ent);
void Kots_CharacterUnhook(edict_t *ent);
void Kots_CharacterKnockWeapon(edict_t *ent);
void Kots_CharacterCheckKnock(edict_t *targ, edict_t *attacker, int damage);
void Kots_CharacterCheckKnockCounter(edict_t *ent);
void Kots_CharacterToggleKnock(edict_t *ent, char *value);
void Kots_CharacterCheckVampire(edict_t *targ, edict_t *attacker, int damage);
void Kots_CharacterCheckAmmoSteal(edict_t *targ, edict_t *attacker, int damage);
void Kots_CharacterPlayerId(edict_t *ent);
void Kots_CharacterCreateHealth(edict_t *ent);
void Kots_CharacterCreateFakeStim(edict_t *ent);
void Kots_CharacterCreateFakeMega(edict_t *ent);
void Kots_CharacterDropFakeHealth(edict_t *ent, char *pickup_name);
void Kots_CharacterToggleSpiritSwim(edict_t *ent, char *value);
void Kots_CharacterCheckSpiritSwim(edict_t *ent, pmove_t *pm);
void Kots_CharacterEmpathyOff(edict_t *ent);
void Kots_CharacterEmpathyOn(edict_t *ent);
void Kots_CharacterEmpathyToggle(edict_t *ent, char *value);
void Kots_CharacterCheckEmpathyDamage(edict_t *targ, edict_t *attacker, int damage);
void Kots_CharacterEmpathyUseCubes(edict_t *ent);
void Kots_CharacterKotsArmor(edict_t *ent);
void Kots_CharacterCheckKotsArmor(edict_t *ent);
void Kots_CharacterConvertAmmoToggle(edict_t *ent, char *value);
void Kots_CharacterCheckConvertAmmo(edict_t *ent);
void Kots_CharacterConvertCellsToggle(edict_t *ent, char *value);
void Kots_CharacterConvertCells(edict_t *ent);
qboolean Kots_CharacterCheckRailDeflect(edict_t *targ, edict_t *attacker, vec3_t dir, int damage);
void Kots_CharacterToggleDeflect(edict_t *ent, char *value);
void Kots_CharacterCheckCubeRegen(edict_t *ent);
void Kots_CharacterCheckConvertCells(edict_t *ent);
void Kots_CharacterSpite(edict_t *ent);
void Kots_CharacterCheckQuadRage(edict_t *targ, edict_t *attacker, int dmg);
int Kots_CharacterGetQuadRageTime(edict_t *ent);
void Kots_CharacterCheckPoison(edict_t *targ, edict_t *attacker, int dmg);
void Kots_CharacterCheckPoisonDamage(edict_t *ent);
void Kots_CharacterDropMine(edict_t *ent);
void Kots_CheckArmorKnock(edict_t *targ, edict_t *attacker, int dmg);
void Kots_CharacterFlail(edict_t *ent);
void Kots_CharacterConflagration(edict_t *ent);
void Kots_CharacterCheckConflagration(edict_t *ent);
qboolean Kots_IsHeadShot (edict_t *attacker, edict_t *targ, vec3_t dir, vec3_t point, int damage);
void Kots_CharacterCheckDizzy(edict_t *ent, pmove_t *pm);
void Kots_CharacterCheckStun(edict_t *ent, pmove_t *pm);
void Kots_CharacterTogglePoison(edict_t *ent, char *value);
void Kots_CharacterToggleQuadRage(edict_t *ent, char *value);
void Kots_CharacterToggleLaser(edict_t *ent, char *value);
void Kots_CharacterToggleLaserBall(edict_t *ent, char *value);
void Kots_CharacterCreateLaser(edict_t *ent);
void Kots_CharacterCreateLaserBall(edict_t *ent);
void Kots_CharacterLaserThink(edict_t *self);
void Kots_CharacterHaste(edict_t *ent);
void Kots_CharacterCheckHaste(edict_t *ent, pmove_t *pm);
void Kots_CharacterDetonate(edict_t *ent);
void Kots_CharacterFlyCheckCharge(edict_t *ent);
void Kots_CharacterCheckHook(edict_t *ent);
void Kots_CharacterHookColor(edict_t *ent, char *value);

#endif

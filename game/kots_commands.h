#ifndef KOTS_COMMANDS_H
#define KOTS_COMMANDS_H

#include "kots_character.h"

typedef struct
{
    char *name;
    int weapon;
} weaponlookup_t;

typedef struct
{
    char *name;
    char *desc;
    void (*action)(edict_t *ent, char *args);
    qboolean is_admin;
} command_t;

//Quake 2 Prototypes used
void Use_Weapon(edict_t *ent, gitem_t *item);


void stuffcmd(edict_t *ent, char *s);
void Kots_CheckDuplicateCommands();
void Kots_StuffCommands(edict_t *ent);
void Kots_Help(edict_t *ent, char *args);
void Kots_SetPassword(edict_t *ent, char *pass);
void Kots_StartGame(edict_t *ent, pmenuhnd_t *hnd);
void Kots_Create(edict_t *ent, pmenuhnd_t *hnd);
void Kots_Observer(edict_t *ent, pmenuhnd_t *hnd);
void Kots_StartGame1(edict_t *ent, char *hnd);
void Kots_Create1(edict_t *ent, char *hnd);
void Kots_Observer1(edict_t *ent, char *hnd);
void Kots_ServerList(edict_t *ent, char *args);
void Kots_Info(edict_t *ent, char *args);
void Kots_Pointlist(edict_t *ent, char *args);
void Kots_Player(edict_t *ent, char *player);
void Kots_Power(edict_t *ent, char *power);
void Kots_Weapon(edict_t *ent, char *weapon);
void Kots_SetPlayer(edict_t *ent, char *player);
void Kots_SetPower(edict_t *ent, char *power);
void Kots_SetWeapon(edict_t *ent, char *weapon);
void Kots_Detonate(edict_t *ent, char *args);
void Kots_Haste(edict_t *ent, char*args);
void Kots_Resist(edict_t *ent, char *resist);
void Kots_Respawn(edict_t *ent, char *respawn);
void Kots_Buy(edict_t *ent, char *args);
void Kots_Tball(edict_t *ent, char *args);
void Kots_Lift(edict_t *ent, char *args);
void Kots_Toss(edict_t *ent, char *args);
void Kots_Fly(edict_t *ent, char *args);
void Kots_Land(edict_t *ent, char *args);
void Kots_Cloak(edict_t *ent, char *args);
void Kots_Expack(edict_t *ent, char *args);
void Kots_Hook(edict_t *ent, char *args);
void Kots_Unhook(edict_t *ent, char *args);
void Kots_Jump(edict_t *ent, char *args);
void Kots_Health(edict_t *ent, char *args);
void Kots_Empathy(edict_t *ent, char *args);
void Kots_FakeMega(edict_t *ent, char *args);
void Kots_FakeStim(edict_t *ent, char *args);
void Kots_FakeShard(edict_t *ent, char *args);
void Kots_Flashlight(edict_t *ent, char *args);
void Kots_SpiritSwim(edict_t *ent, char *args);
void Kots_Armor(edict_t *ent, char *args);
void Kots_AmmoConvert(edict_t *ent, char *args);
void Kots_PowerConvert(edict_t *ent, char *args);
void Kots_Deflect(edict_t *ent, char *args);
void Kots_Knock(edict_t *ent, char *args);
void Kots_Poison(edict_t *ent, char *args);
void Kots_QuadRage(edict_t *ent, char *args);
void Kots_Laser(edict_t *ent, char *args);
void Kots_Spite(edict_t *ent, char *args);
void Kots_Mine(edict_t *ent, char *args);
void Kots_Conflag(edict_t *ent, char *args);
void Kots_Flail(edict_t *ent, char *args);
void Kots_Throw(edict_t *ent, char *args);
void Kots_Bide(edict_t *ent, char *args);
void Kots_Spiral(edict_t *ent, char *args);
void Kots_Flail(edict_t *ent, char *args);
void Kots_Maplist(edict_t *ent, char *args);
void Kots_Vote(edict_t *ent, char *args);
void Kots_ListIP(edict_t *ent, char *args);
void Kots_KickUser(edict_t *ent, char *args);
void Kots_CurseUser(edict_t *ent, char *args);
void Kots_IpBanUser(edict_t *ent, char *args);
void Kots_SpawnMonster(edict_t *ent, char *args);
void Kots_DropRune(edict_t *ent, char *args);
void Kots_CreateRune(edict_t *ent, char *args);
void Kots_ListRunes(edict_t *ent, char *args);
void Kots_ShowRunes(edict_t *ent, char *args);
void Kots_DestroyRune(edict_t *ent, char *args);
void Kots_PlayVoice(edict_t *ent, char *args);
void Kots_Gender(edict_t *ent, char *args);
void Kots_Silence(edict_t *ent, char *args);
void Kots_Unsilence(edict_t *ent, char *args);
void Kots_MuteCommand(edict_t *ent, char *args);
void Kots_UnmuteCommand(edict_t *ent, char *args);
void Kots_UnmuteIPCommand(edict_t *ent, char *args);
void Kots_MuteListCommand(edict_t *ent, char *args);
void Kots_AdminLogin(edict_t *ent, char *args);
void Kots_AdminLogout(edict_t *ent, char *args);
void Kots_AllowLogin(edict_t *ent, char *args);
void Kots_NoLogin(edict_t *ent, char *args);
void Kots_Map(edict_t *ent, char *args);
void Kots_HgTimer(edict_t *ent, char *args);
void Kots_HgSpeed(edict_t *ent, char *args);
void Kots_HgFuse(edict_t *ent, char *args);
void Kots_Update(edict_t *ent, char *args);
void Kots_Revert(edict_t *ent, char *args);
void Kots_RCon(edict_t *ent, char *args);
void Kots_Stats(edict_t *ent, char *args);
void Kots_Respec(edict_t *ent, char *player);
void Kots_HookColor(edict_t *ent, char *args);
qboolean Kots_Command(edict_t *ent, char *cmd);

#endif

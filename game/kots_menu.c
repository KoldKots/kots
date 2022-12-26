#include "kots_menu.h"
#include "kots_commands.h"
#include "kots_monster.h"
#include "kots_server.h"
#include "kots_utils.h"

void Kots_MenuPrint(pmenu_t *menu, const char *format, ...);
char *Kots_GetStatString(int base, int current);

#define KotsMenuCount 18
pmenu_t kotsmenu[KotsMenuCount] = {
	{ "*-= KOTS2007 Menu =-",	PMENU_ALIGN_CENTER, NULL },
	{ "Join game",				PMENU_ALIGN_LEFT, Kots_StartGame },
	{ "Create character",		PMENU_ALIGN_LEFT, Kots_Create },
	{ "Switch to observer",		PMENU_ALIGN_LEFT, Kots_Observer },
	{ "Server Menu",			PMENU_ALIGN_LEFT, Kots_MenuServers },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ "Character Info",			PMENU_ALIGN_LEFT, Kots_MenuShowCharacterInfo },
	{ "Character Point List",	PMENU_ALIGN_LEFT, Kots_MenuShowPointlist },
	{ "Player Points",			PMENU_ALIGN_LEFT, Kots_MenuShowPlayer },
	{ "Weapon Points",			PMENU_ALIGN_LEFT, Kots_MenuShowWeapon },
	{ "Power Points",			PMENU_ALIGN_LEFT, Kots_MenuShowPower },
	{ "Set Respawn",			PMENU_ALIGN_LEFT, Kots_MenuShowRespawn },
	{ "Set Resist",				PMENU_ALIGN_LEFT, Kots_MenuShowResist },
	{ "Buy Menu",				PMENU_ALIGN_LEFT, Kots_MenuShowBuy },
#ifndef KOTS_PVP_ONLY
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ "Monster Voting",			PMENU_ALIGN_LEFT, Kots_VoteMenuShow },
#endif
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_RIGHT, NULL }
};

pmenu_t characterinfo_menu[KotsMenuCount] = {
	{ "*-= Character Info =-",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ "Back",					PMENU_ALIGN_LEFT, Kots_MenuShow }
};

pmenu_t pointlist_menu[KotsMenuCount] = {
	{ "*-= Point List =-",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Back",				PMENU_ALIGN_LEFT, Kots_MenuShow }
};

char *player_menu_args[] =
	{"dexterity", "strength", "karma", "wisdom", "spirit",
	"technical", "rage", "vithealth", "vitarmor", "munition"};

pmenu_t player_menu[KotsMenuCount] = {
	{ "*-= Player Points =-",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuPlayerAdd },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ "Back",					PMENU_ALIGN_LEFT, Kots_MenuShow }
};

char *weapon_menu_args[] =
	{"sabre", "sg", "ssg", "mg", "cg",
	"grenade", "gl", "rl", "hb", "rg", "bfg", "anti"};

pmenu_t weapon_menu[KotsMenuCount] = {
	{ "*-= Weapon Points =-",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, Kots_MenuWeaponAdd },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ "Back",					PMENU_ALIGN_LEFT, Kots_MenuShow }
};


char *power_menu_args[] =
	{"expack", "spiral", "bide", "throw", "antiweapon"};

pmenu_t power_menu[KotsMenuCount] = {
	{ "*-= Power Points =-",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuPowerAdd },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuPowerAdd },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuPowerAdd },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuPowerAdd },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuPowerAdd },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Back",				PMENU_ALIGN_LEFT, Kots_MenuShow }
};

pmenu_t respawn_menu[KotsMenuCount] = {
	{ "*-= Set Respawn =-",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Sabre",				PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Shotgun",			PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Super Shotgun",		PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Machinegun",			PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Chaingun",			PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Hand Grenades",		PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Grenade Launcher",	PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Rocket Launcher",	PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Hyperblaster",		PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "Railgun",			PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ "BFG10K",				PMENU_ALIGN_LEFT, Kots_MenuRespawnSet },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Back",				PMENU_ALIGN_LEFT, Kots_MenuShow }
};


pmenu_t resist_menu[KotsMenuCount] = {
	{ "*-= Set Resist =-",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Sabre",				PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Shotgun",			PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Super Shotgun",		PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Machinegun",			PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Chaingun",			PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Hand Grenades",		PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Grenade Launcher",	PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Rocket Launcher",	PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Hyperblaster",		PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Railgun",			PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "BFG10K",				PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Expack",				PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Spiral",				PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Bide",				PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Throw",				PMENU_ALIGN_LEFT, Kots_MenuResistSet },
	{ "Back",				PMENU_ALIGN_LEFT, Kots_MenuShow }
};

#define SpawnIntervalCount	6
int spawn_intervals[SpawnIntervalCount] = {10, 25, 50, 100, 200, 500};
pmenu_t buy_menu[KotsMenuCount] = {
	{ "*-= Buy Menu =-",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuBuySelectSpawns },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuBuySpawns },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuBuyPlayer },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuBuyWeapon },
	{ NULL,					PMENU_ALIGN_LEFT, Kots_MenuBuyPower },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Back",				PMENU_ALIGN_LEFT, Kots_MenuShow }
};

// Aldarn - vote menu
// Vote menu
pmenu_t vote_menu[KotsMenuCount] = {
	{ "*-= Monster Voting =-",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Monsters Off",				PMENU_ALIGN_LEFT, Kots_VoteMonOff },
	{ "Monsters Mixed",				PMENU_ALIGN_LEFT, Kots_VoteMonMixed },
	{ "Monsters Only",				PMENU_ALIGN_LEFT, Kots_VoteMonOnly },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Back",						PMENU_ALIGN_LEFT, Kots_MenuShow }
};

pmenu_t servers_menu[KotsMenuCount] = {
	{ "*-= KOTS2007 Servers =-",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Back",						PMENU_ALIGN_LEFT, Kots_MenuShow }
};

qboolean Kots_MenuIsOpen(edict_t *ent, char *name)
{
	if (level.intermissiontime || !level.time)
		return false;

	//if the specified menu is open close it and return true
	if (ent->client->menu && Q_stricmp(ent->client->menu->name, name) == 0)
		return true;
	else
		return false;
}

qboolean Kots_MenuCloseIfOpen(edict_t *ent, char *name)
{
	if (level.intermissiontime || !level.time)
		return false;

	//if the specified menu is open close it and return true
	if (ent->client->menu && Q_stricmp(ent->client->menu->name, name) == 0)
	{
		PMenu_Close(ent);
		return true;
	}

	return false;
}

// Aldarn - vote menu functions //
void Kots_VoteMenuShow(edict_t *ent, pmenuhnd_t *hnd)
{
	if (Kots_MenuCloseIfOpen(ent, "VoteMenu"))
		return;

	//close any other menus and show the new one
	PMenu_Close(ent);

	//show the current vote status
	Kots_MenuPrint(&vote_menu[2], "Currently: %14s",
		(KOTS_MONSTER_VOTE == KOTS_MONSTERS_OFF ? "Off" :
		 KOTS_MONSTER_VOTE == KOTS_MONSTERS_ONLY ? "Only" :
		 "Mixed"));
	Kots_MenuPrint(&vote_menu[3], "Your Vote: %14s",
		(ent->client->pers.kots_persist.monster_vote == KOTS_MONSTERS_OFF ? "Off" :
		 ent->client->pers.kots_persist.monster_vote == KOTS_MONSTERS_ONLY ? "Only" :
		 "Mixed"));

	PMenu_Open(ent, vote_menu, "VoteMenu", 1, KotsMenuCount, NULL);
}

void Kots_MenuClose(edict_t *ent, pmenuhnd_t *hnd)
{
	PMenu_Close(ent);
}

void Kots_VoteMonOff(edict_t *ent, pmenuhnd_t *hnd)
{
	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must join the game to vote.\n");
		return;
	}

	PMenu_Close(ent);

	Kots_MonsterVote(ent, KOTS_MONSTERS_OFF);
}

void Kots_VoteMonMixed(edict_t *ent, pmenuhnd_t *hnd)
{
	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must join the game to vote.\n");
		return;
	}

	PMenu_Close(ent);

	Kots_MonsterVote(ent, KOTS_MONSTERS_MIXED);
}

void Kots_VoteMonOnly(edict_t *ent, pmenuhnd_t *hnd)
{
	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must join the game to vote.\n");
		return;
	}

	PMenu_Close(ent);

	Kots_MonsterVote(ent, KOTS_MONSTERS_ONLY);
}
// End //

void Kots_MenuShow(edict_t *ent, pmenuhnd_t *hnd)
{
	if (level.intermissiontime)
		return;

	if (Kots_MenuCloseIfOpen(ent, "MainMenu"))
		return;

	if (!kotsmenu[KotsMenuCount - 1].text)
		Kots_MenuPrint(&kotsmenu[KotsMenuCount - 1], "*%s", gamever->string);

	//close any other menus and show the new one
	PMenu_Close(ent);
	PMenu_Open(ent, kotsmenu, "MainMenu", 1, KotsMenuCount, NULL);
}

void Kots_MenuShowCharacterInfo(edict_t *ent, pmenuhnd_t *hnd)
{
	if (!ent->character->is_loggedin)
	{
		//character may not exist or be logged in if monster died and was freed
		if (ent->client->chase_target && ent->client->chase_target->character && ent->client->chase_target->character->is_loggedin)
		{
			Kots_MenuShowOtherCharacterInfo(ent, ent->client->chase_target, ent->client->chase_target->character);
			return;
		}

		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to open this menu.\n");
		return;
	}

	Kots_MenuShowOtherCharacterInfo(ent, ent, ent->character);
}

void Kots_MenuShowOtherCharacterInfo(edict_t *ent, edict_t *other, character_t *character)
{
	//build the player menu
	Kots_MenuPrint(&characterinfo_menu[2], "Name: %19s", character->name);
	Kots_MenuPrint(&characterinfo_menu[3], "Level: %18u", character->level);
	Kots_MenuPrint(&characterinfo_menu[4], "Exp: %20u", character->exp);
	Kots_MenuPrint(&characterinfo_menu[5], "Next Level: %13u", Kots_CharacterGetNextLevelExp(character->level));
	Kots_MenuPrint(&characterinfo_menu[6], "Credits: %16u", character->credits);
	Kots_MenuPrint(&characterinfo_menu[7], "Frags: %18u", (other && other->client ? other->client->ps.stats[STAT_FRAGS] : character->kills));
	Kots_MenuPrint(&characterinfo_menu[8], "Shots: %18u", character->shots);
	Kots_MenuPrint(&characterinfo_menu[9], "Hits: %19u", character->hits);
	Kots_MenuPrint(&characterinfo_menu[10], "Accuracy:           %03.2f%%", (character->shots ? (float)character->hits / character->shots * 100 : 0));
	Kots_MenuPrint(&characterinfo_menu[11], "Spree: %3u Respawns: %4u", character->spree, character->respawns);
	Kots_MenuPrint(&characterinfo_menu[12], "Avail Player Pts: %7u", character->playerpoints);
	Kots_MenuPrint(&characterinfo_menu[13], "Avail Weapon Pts: %7u", character->weaponpoints);
	Kots_MenuPrint(&characterinfo_menu[14], "Avail Power Pts: %8u", character->powerpoints);
	Kots_MenuPrint(&characterinfo_menu[15], "Resist: %17s", Kots_CharacterGetResistName(character->resist));
	Kots_MenuPrint(&characterinfo_menu[16], "Respawn: %16s", Kots_CharacterGetRespawnName(character->respawn_weapon));

	//close existing menu and open new one
	PMenu_Close(ent);
	PMenu_Open(ent, characterinfo_menu, "CharacterInfo", 0, KotsMenuCount, NULL);
}

void Kots_MenuShowPointlist(edict_t *ent, pmenuhnd_t *hnd)
{
	if (!ent->character->is_loggedin)
	{
		//character may not exist or be logged in if monster died and was freed
		if (ent->client->chase_target && ent->client->chase_target->character && ent->client->chase_target->character->is_loggedin)
		{
			Kots_MenuShowOtherPointlist(ent, ent->client->chase_target->character);
			return;
		}

		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to open this menu.\n");
		return;
	}

	Kots_MenuShowOtherPointlist(ent, ent->character);
}

void Kots_MenuShowOtherPointlist(edict_t *ent, character_t *character)
{
	//build the player menu
	Kots_MenuPrint(&pointlist_menu[1], "Name: %19s", character->name);

	if (character->is_loggedin)
	{
		Kots_MenuPrint(&pointlist_menu[2], "Dex: %3s Str: %3s Kar: %2s",
			Kots_GetStatString(character->dexterity, character->cur_dexterity),
			Kots_GetStatString(character->strength, character->cur_strength),
			Kots_GetStatString(character->karma, character->cur_karma));
		Kots_MenuPrint(&pointlist_menu[3], "Spi: %3s Wis: %3s Tec: %2s",
			Kots_GetStatString(character->spirit, character->cur_spirit),
			Kots_GetStatString(character->wisdom, character->cur_wisdom),
			Kots_GetStatString(character->technical, character->cur_technical));
		Kots_MenuPrint(&pointlist_menu[4], "Rag: %3s Mun: %3s",
			Kots_GetStatString(character->rage, character->cur_rage),
			Kots_GetStatString(character->munition, character->cur_munition));
		Kots_MenuPrint(&pointlist_menu[5], "ViA: %3s ViH: %3s",
			Kots_GetStatString(character->vitarmor, character->cur_vitarmor),
			Kots_GetStatString(character->vithealth, character->cur_vithealth));
		Kots_MenuPrint(&pointlist_menu[7], "Sabre: %5s SG:    %5s",
			Kots_GetStatString(character->sabre, character->cur_sabre),
			Kots_GetStatString(character->shotgun, character->cur_shotgun));
		Kots_MenuPrint(&pointlist_menu[8], "SSG:   %5s MG:    %5s",
			Kots_GetStatString(character->supershotgun, character->cur_supershotgun),
			Kots_GetStatString(character->machinegun, character->cur_machinegun));
		Kots_MenuPrint(&pointlist_menu[9], "CG:    %5s HG:    %5s",
			Kots_GetStatString(character->chaingun, character->cur_chaingun),
			Kots_GetStatString(character->grenade, character->cur_grenade));
		Kots_MenuPrint(&pointlist_menu[10], "GL:    %5s RL:    %5s",
			Kots_GetStatString(character->grenadelauncher, character->cur_grenadelauncher),
			Kots_GetStatString(character->rocketlauncher, character->cur_rocketlauncher));
		Kots_MenuPrint(&pointlist_menu[11], "HB:    %5s RG:    %5s",
			Kots_GetStatString(character->hyperblaster, character->cur_hyperblaster),
			Kots_GetStatString(character->railgun, character->cur_railgun));
		Kots_MenuPrint(&pointlist_menu[12], "BFG:   %5s Anti:  %5s",
			Kots_GetStatString(character->bfg, character->cur_bfg),
			Kots_GetStatString(character->wantiweapon, character->cur_wantiweapon));
		Kots_MenuPrint(&pointlist_menu[14], "Expack: %4s Spiral: %4s",
			Kots_GetStatString(character->expack, character->cur_expack),
			Kots_GetStatString(character->spiral, character->cur_spiral));
		Kots_MenuPrint(&pointlist_menu[15], "Bide:   %4s Throw:  %4s",
			Kots_GetStatString(character->bide, character->cur_bide),
			Kots_GetStatString(character->kotsthrow, character->cur_kotsthrow));
		Kots_MenuPrint(&pointlist_menu[16], "Anti:   %4s", Kots_GetStatString(character->antiweapon, character->cur_antiweapon));
	}
	else
	{
		Kots_MenuPrint(&pointlist_menu[2], "Dex: %3u Str: %3u Kar: %2u", character->dexterity, character->strength, character->karma);
		Kots_MenuPrint(&pointlist_menu[3], "Spi: %3u Wis: %3u Tec: %2u", character->spirit, character->wisdom, character->technical);
		Kots_MenuPrint(&pointlist_menu[4], "Rag: %3u Mun: %3u", character->rage, character->munition);
		Kots_MenuPrint(&pointlist_menu[5], "ViA: %3u ViH: %3u", character->vitarmor, character->vithealth);
		Kots_MenuPrint(&pointlist_menu[7], "Sabre: %5u SG:    %5u", character->sabre, character->shotgun);
		Kots_MenuPrint(&pointlist_menu[8], "SSG:   %5u MG:    %5u", character->supershotgun, character->machinegun);
		Kots_MenuPrint(&pointlist_menu[9], "CG:    %5u HG:    %5u", character->chaingun, character->grenade);
		Kots_MenuPrint(&pointlist_menu[10], "GL:    %5u RL:    %5u", character->grenadelauncher, character->rocketlauncher);
		Kots_MenuPrint(&pointlist_menu[11], "HB:    %5u RG:    %5u", character->hyperblaster, character->railgun);
		Kots_MenuPrint(&pointlist_menu[12], "BFG:   %5u", character->bfg);
		Kots_MenuPrint(&pointlist_menu[14], "Expack: %4u Spiral: %4u", character->expack, character->spiral);
		Kots_MenuPrint(&pointlist_menu[15], "Bide:   %4u Throw:  %4u", character->bide, character->kotsthrow);
		Kots_MenuPrint(&pointlist_menu[16], "Anti:   %4u", character->antiweapon);
	}

	//close existing menu and open new one
	PMenu_Close(ent);
	PMenu_Open(ent, pointlist_menu, "Pointlist", 0, KotsMenuCount, NULL);
}

void Kots_MenuShowPlayer(edict_t *ent, pmenuhnd_t *hnd)
{
	int i;

	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to open this menu.\n");
		return;
	}

	//build the player menu
	Kots_MenuPrint(&player_menu[2], "Available: %12u", ent->character->playerpoints);
	Kots_MenuPrint(&player_menu[3], "Dexterity: %12u", ent->character->dexterity);
	Kots_MenuPrint(&player_menu[4], "Strength: %13u", ent->character->strength);
	Kots_MenuPrint(&player_menu[5], "Karma: %16u", ent->character->karma);
	Kots_MenuPrint(&player_menu[6], "Wisdom: %15u", ent->character->wisdom);
	Kots_MenuPrint(&player_menu[7], "Spirit: %15u", ent->character->spirit);
	Kots_MenuPrint(&player_menu[8], "Technical: %12u", ent->character->technical);
	Kots_MenuPrint(&player_menu[9], "Rage: %17u", ent->character->rage);
	Kots_MenuPrint(&player_menu[10], "VitHealth: %12u", ent->character->vithealth);
	Kots_MenuPrint(&player_menu[11], "VitArmor: %13u", ent->character->vitarmor);
	Kots_MenuPrint(&player_menu[12], "Munition: %13u", ent->character->munition);

	if (hnd && hnd->num == 0)
		i = hnd->cur;
	else
		i = 0;

	//close existing menu and open new one
	PMenu_Close(ent);
	PMenu_Open(ent, player_menu, "PlayerMenu", i, KotsMenuCount, NULL);
}

void Kots_MenuPlayerAdd(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	int available = ent->character->playerpoints;
	Kots_Player(ent, player_menu_args[hnd->cur - 3]);
	
	//if changed refresh the menu
	if (available != ent->character->playerpoints)
		Kots_MenuShowPlayer(ent, &temphnd);
}

void Kots_MenuShowWeapon(edict_t *ent, pmenuhnd_t *hnd)
{
	int i;

	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to open this menu.\n");
		return;
	}

	//build the weapon menu
	Kots_MenuPrint(&weapon_menu[2], "Available: %12u", ent->character->weaponpoints);
	Kots_MenuPrint(&weapon_menu[3], "Sabre: %16u", ent->character->sabre);
	Kots_MenuPrint(&weapon_menu[4], "Shotgun: %14u", ent->character->shotgun);
	Kots_MenuPrint(&weapon_menu[5], "Super Shotgun: %8u", ent->character->supershotgun);
	Kots_MenuPrint(&weapon_menu[6], "Machinegun: %11u", ent->character->machinegun);
	Kots_MenuPrint(&weapon_menu[7], "Chaingun: %13u", ent->character->chaingun);
	Kots_MenuPrint(&weapon_menu[8], "Grenades: %13u", ent->character->grenade);
	Kots_MenuPrint(&weapon_menu[9], "Grenade Launcher: %5u", ent->character->grenadelauncher);
	Kots_MenuPrint(&weapon_menu[10], "Rocket Launcher: %6u", ent->character->rocketlauncher);
	Kots_MenuPrint(&weapon_menu[11], "Hyperblaster: %9u", ent->character->hyperblaster);
	Kots_MenuPrint(&weapon_menu[12], "Railgun: %14u", ent->character->railgun);
	Kots_MenuPrint(&weapon_menu[13], "BFG: %18u", ent->character->bfg);
	Kots_MenuPrint(&weapon_menu[14], "Antiweapon: %11u", ent->character->wantiweapon);

	if (hnd && hnd->num == 0)
		i = hnd->cur;
	else
		i = 0;

	//close existing menu and open new one
	PMenu_Close(ent);
	PMenu_Open(ent, weapon_menu, "WeaponMenu", i, KotsMenuCount, NULL);
}

void Kots_MenuWeaponAdd(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	int available = ent->character->weaponpoints;
	Kots_Weapon(ent, weapon_menu_args[hnd->cur - 3]);

	//if changed refresh the menu
	if (available != ent->character->weaponpoints)
		Kots_MenuShowWeapon(ent, &temphnd);
}


void Kots_MenuShowPower(edict_t *ent, pmenuhnd_t *hnd)
{
	int i;

	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to open this menu.\n");
		return;
	}

	//build the power menu
	Kots_MenuPrint(&power_menu[2], "Available: %12u", ent->character->powerpoints);
	Kots_MenuPrint(&power_menu[3], "Expack: %15u", ent->character->expack);
	Kots_MenuPrint(&power_menu[4], "Spiral: %15u", ent->character->spiral);
	Kots_MenuPrint(&power_menu[5], "Bide: %17u", ent->character->bide);
	Kots_MenuPrint(&power_menu[6], "Throw: %16u", ent->character->kotsthrow);
	Kots_MenuPrint(&power_menu[7], "Antiweapon: %11u", ent->character->antiweapon);

	if (hnd && hnd->num == 0)
		i = hnd->cur;
	else
		i = 0;

	//close existing menu and open new one
	PMenu_Close(ent);
	PMenu_Open(ent, power_menu, "PowerMenu", i, KotsMenuCount, NULL);
}

void Kots_MenuPowerAdd(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	int available = ent->character->powerpoints;
	Kots_Power(ent, power_menu_args[hnd->cur - 3]);
	
	//if changed refresh the menu
	if (available != ent->character->powerpoints)
		Kots_MenuShowPower(ent, &temphnd);
}

void Kots_MenuShowRespawn(edict_t *ent, pmenuhnd_t *hnd)
{
	int i;

	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to open this menu.\n");
		return;
	}

	//build the weapon menu
	Kots_MenuPrint(&respawn_menu[2], "Current: %16s", Kots_CharacterGetRespawnName(ent->character->respawn_weapon));

	if (hnd && hnd->num == 0)
		i = hnd->cur;
	else
		i = 0;

	//close existing menu and open new one
	PMenu_Close(ent);
	PMenu_Open(ent, respawn_menu, "RespawnMenu", i, KotsMenuCount, NULL);
}

void Kots_MenuRespawnSet(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	Kots_Respawn(ent, weapon_menu_args[hnd->cur - 3]);
	Kots_MenuShowRespawn(ent, &temphnd);
}



void Kots_MenuShowResist(edict_t *ent, pmenuhnd_t *hnd)
{
	int i;

	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to open this menu.\n");
		return;
	}

	//build the weapon menu
	Kots_MenuPrint(&resist_menu[1], "Current: %16s", Kots_CharacterGetResistName(ent->character->resist));

	if (hnd && hnd->num == 0)
		i = hnd->cur;
	else
		i = 0;

	//close existing menu and open new one
	PMenu_Close(ent);
	PMenu_Open(ent, resist_menu, "ResistMenu", i, KotsMenuCount, NULL);
}

void Kots_MenuResistSet(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	int resist = ent->character->resist;
	if (hnd->cur >= 13)
		Kots_Resist(ent, power_menu_args[hnd->cur - 13]);
	else
		Kots_Resist(ent, weapon_menu_args[hnd->cur - 2]);

	if (ent->character->resist != resist)
		Kots_MenuShowResist(ent, &temphnd);
}


void Kots_MenuShowBuy(edict_t *ent, pmenuhnd_t *hnd)
{
	int i;

	if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to open this menu.\n");
		return;
	}

	Kots_MenuPrint(&buy_menu[2], "Your Credits %10ic", ent->character->credits);
	Kots_MenuPrint(&buy_menu[4], "Spawns to Buy %9i", spawn_intervals[ent->character->menu_spawns]);
	Kots_MenuPrint(&buy_menu[5], "Buy Spawns %12ic", Kots_CharacterGetSpawnCost(spawn_intervals[ent->character->menu_spawns]));
	Kots_MenuPrint(&buy_menu[7], "Buy Player %12ic", Kots_CharacterGetNextPlayerPointCost(ent));
	Kots_MenuPrint(&buy_menu[8], "Buy Weapon %12ic", Kots_CharacterGetNextWeaponPointCost(ent));
	Kots_MenuPrint(&buy_menu[9], "Buy Power %13ic", Kots_CharacterGetNextPowerPointCost(ent));

	if (hnd && hnd->num == 0)
		i = hnd->cur;
	else
		i = 0;

	//close existing menu and open new one
	PMenu_Close(ent);
	PMenu_Open(ent, buy_menu, "BuyMenu", i, KotsMenuCount, NULL);
}

void Kots_MenuBuySelectSpawns(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};

	//increment the spawns to buy or start at the beginning
	if (++ent->character->menu_spawns >= SpawnIntervalCount)
		ent->character->menu_spawns = 0;

	Kots_MenuShowBuy(ent, &temphnd);
}

void Kots_MenuBuySpawns(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	int respawns = ent->character->respawns;

	//attempt to buy the spawns
	Kots_CharacterBuySpawns(ent, spawn_intervals[ent->character->menu_spawns]);

	//if spawns were purchased
	if (respawns != ent->character->respawns)
		Kots_MenuShowBuy(ent, &temphnd);
}

void Kots_MenuBuyPlayer(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	int points = ent->character->playerpoints;

	//attempt to buy the point
	Kots_CharacterBuyPlayerPoint(ent);

	//if a point was purchased
	if (points != ent->character->playerpoints)
		Kots_MenuShowBuy(ent, &temphnd);
}

void Kots_MenuBuyWeapon(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	int points = ent->character->weaponpoints;

	//attempt to buy the point
	Kots_CharacterBuyWeaponPoint(ent);

	//if a point was purchased
	if (points != ent->character->weaponpoints)
		Kots_MenuShowBuy(ent, &temphnd);
}

void Kots_MenuBuyPower(edict_t *ent, pmenuhnd_t *hnd)
{
	pmenuhnd_t temphnd = {NULL, hnd->cur, 0, NULL};
	int points = ent->character->powerpoints;

	//attempt to buy the point
	Kots_CharacterBuyPowerPoint(ent);

	//if a point was purchased
	if (points != ent->character->powerpoints)
		Kots_MenuShowBuy(ent, &temphnd);
}

int servers_per_page = 8;
void Kots_MenuCreateServers(edict_t *ent)
{
	unsigned long i;
	int j, count;
	char name[19];

	j = 3;
	count = 0;
	if (kots_servers->length == 0) 
	{
		Kots_MenuPrint(&servers_menu[j], "No servers found.");
		servers_menu[j].SelectFunc = NULL;
		j++;
	}
	else
	{
		for (i = servers_per_page * ent->client->menu_page; i < kots_servers->length && count <= servers_per_page; i++, j++, count++)
		{
			server_info_t *info = Array_GetValueAt(kots_servers, i);
			Kots_strncpy(name, info->name, sizeof(name));
			Kots_MenuPrint(&servers_menu[j], "%-18s - %02i/%02i", name, info->cur_players, info->max_players);
			servers_menu[j].SelectFunc = Kots_MenuServersConnect;
		}
	}

	for ( ; j < KotsMenuCount - 1; j++)
	{
		Kots_MenuPrint(&servers_menu[j], "");
		servers_menu[j].SelectFunc = NULL;
	}

	if (kots_servers->length > servers_per_page)
	{
		j = KotsMenuCount - 4;
		Kots_MenuPrint(&servers_menu[j], "Previous Page");
		servers_menu[j].SelectFunc = Kots_MenuServersPrevPage;

		j++;
		Kots_MenuPrint(&servers_menu[j], "Next Page");
		servers_menu[j].SelectFunc = Kots_MenuServersNextPage;
	}

	//close existing menu and open new one
	if (Kots_MenuIsOpen(ent, "ServerMenu"))
		j = ent->client->menu->cur;
	else
		j = 0;

	PMenu_Close(ent);
	PMenu_Open(ent, servers_menu, "ServerMenu", j, KotsMenuCount, NULL);
}


void Kots_MenuServers(edict_t *ent, pmenuhnd_t *hnd)
{
	ent->client->menu_page = 0;
	Kots_MenuCreateServers(ent);
}

void Kots_MenuServersConnect(edict_t *ent, pmenuhnd_t *hnd)
{
	int start = 3;
	int index = (ent->client->menu_page * servers_per_page) + (hnd->cur - start);

	if (index < kots_servers->length)
	{
		server_info_t *info = Array_GetValueAt(kots_servers, (unsigned long)index);
		gi.bprintf(PRINT_HIGH, "%s decided to hang out at %s\n", ent->client->pers.netname, info->name);
		stuffcmd(ent, va("connect %s:%i\n", info->host, info->port));
	}
}

void Kots_MenuServersPrevPage(edict_t *ent, pmenuhnd_t *hnd)
{
	ent->client->menu_page--;

	if (ent->client->menu_page < 0)
		ent->client->menu_page = (int)ceil(kots_servers->length / (float)servers_per_page);

	Kots_MenuCreateServers(ent);
}

void Kots_MenuServersNextPage(edict_t *ent, pmenuhnd_t *hnd)
{
	int pages = (int)ceil(kots_servers->length / (float)servers_per_page);
	ent->client->menu_page++;

	if (ent->client->menu_page >= pages)
		ent->client->menu_page = 0;

	Kots_MenuCreateServers(ent);
}


void Kots_MenuPrint(pmenu_t *menu, const char *format, ...)
{
	va_list args;

	if (!menu->text)
		menu->text = gi.TagMalloc(sizeof(char) * 48, TAG_GAME);

	//format the message
	va_start(args, format);
	Kots_vsnprintf(menu->text, 48, format, args);
	va_end(args);
}

char *Kots_GetStatString(int base, int current)
{
	//We need to keep multiple arrays of strings because they will get used multiple times at once
	static char strings[10][20];
	static int i = 0;
	char *string = strings[i];

	//create the formatted string
	if (base == current)
		Kots_snprintf(string, 20, "%u", current);
	else if (current > base)
		Kots_snprintf(string, 20, "%u+%u", base, current - base);
	else
		Kots_snprintf(string, 20, "%u-%u", base, base - current);

	//increment i to the next string to use
	i = (i + 1) % 10;

	//return the formatted string
	return string;
}

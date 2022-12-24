#include "kots_commands.h"
#include "kots_mute.h"
#include "kots_weapon.h"
#include "kots_power.h"
#include "kots_maplist.h"
#include "kots_utils.h"
#include "kots_server.h"
#include "kots_menu.h"
#include "kots_player.h"
#include "kots_power.h"
#include "kots_weapon.h"
#include "kots_abilities.h"
#include "kots_tballs.h"
#include "kots_hook.h"
#include "kots_items.h"
#include "kots_runes.h"
#include "kots_monster.h"
#include "kots_admin.h"
#include "kots_hud.h"
#include "kots_update.h"

//Respawn weapons
weaponlookup_t WeaponLookup[] =
{
	"sabre", WEAP_SABRE,
	"machinegun", WEAP_MACHINEGUN,
	"mg", WEAP_MACHINEGUN,
	"shotgun", WEAP_SHOTGUN,
	"sg", WEAP_SHOTGUN,
	"chaingun", WEAP_CHAINGUN,
	"cg", WEAP_CHAINGUN,
	"supershotgun", WEAP_SUPERSHOTGUN,
	"ssg", WEAP_SUPERSHOTGUN,
	"hg", WEAP_GRENADES,
	"grenade", WEAP_GRENADES,
	"grenadelauncher", WEAP_GRENADELAUNCHER,
	"gl", WEAP_GRENADELAUNCHER,
	"rocketlauncher", WEAP_ROCKETLAUNCHER,
	"rl", WEAP_ROCKETLAUNCHER,
	"hyperblaster", WEAP_HYPERBLASTER,
	"hb", WEAP_HYPERBLASTER,
	"railgun", WEAP_RAILGUN,
	"rail", WEAP_RAILGUN,
	"rg", WEAP_RAILGUN,
	"bfg", WEAP_BFG,
	"bfg10k", WEAP_BFG
};
#define TotalWeapons (sizeof(WeaponLookup) / sizeof(weaponlookup_t))

command_t kots_commands[] =
{
	"kots_help", "Show a list of all KOTS commands", Kots_Help, false,
	"kots_pass", "Set your characters password", NULL, false,
	"kots_create", "Creates a character and joins the game", Kots_Create1, false,
	"kots_join", "Start playing", Kots_StartGame1, false,
	"kots_observer", "Stop playing (enter observer mode)", Kots_Observer1, false,
	"kots_servers", "Show the list of servers", Kots_ServerList, false,
	"kots_player", "Add a player point", Kots_Player, false,
	"kots_power", "Add a power point", Kots_Power, false,
	"kots_weapon", "Add a weapon point", Kots_Weapon, false,
	"kots_respawn", "Sets the weapon you respawn with", Kots_Respawn, false,
	"kots_buy", "Buy respawns, player points, etc with credits", Kots_Buy, false,
	"kots_resist", "Resist a weapon or power", Kots_Resist, false,
	"kots_lift", "Lifts packs and runes towards you", Kots_Lift, false,
	"kots_toss", "Tosses packs and runes away from you", Kots_Toss, false,
	"kots_fly", "Start flying until kots_land", Kots_Fly, false,
	"kots_land", "Stop flying and fall to the ground", Kots_Land, false,
	"kots_cloak", "Toggles cloaking on or off", Kots_Cloak, false,
	"kots_expack", "Drops an a fake player pack that explodes", Kots_Expack, false,
	"kots_hook", "Shoots the grappling hook out", Kots_Hook, false,
	"kots_unhook", "Stops the grappling hook", Kots_Unhook, false,
	"kots_health", "Drops a mega health that doesn't rot", Kots_Health, false,
	"kots_fakestim", "Creates a fake stim pack that causes damage", Kots_FakeStim, false,
	"kots_fakemega", "Creates a fake mega health that causes damage", Kots_FakeMega, false,
	"kots_flashlight", "Enables/disables flashlight", Kots_Flashlight, false,
	"kots_swim", "Enables/disables fast swimming", Kots_SpiritSwim, false,
	"kots_empathy", "Enables/disables empathy shield", Kots_Empathy, false,
	"kots_armor", "Uses cubes to create armor", Kots_Armor, false,
	"kots_pconvert", "Converts power cells into cubes", Kots_PowerConvert, false,
	"kots_deflect", "Enables/disables railgun deflection", Kots_Deflect, false,
	"kots_jump", "Enables/disables strength jump", Kots_Jump, false,
	"kots_knock", "Enables/disables weapon knock", Kots_Knock, false,
	"kots_detonate", "Detonates the last hand grenade thrown", Kots_Detonate, false,
	"kots_spite", "Uses health in order to get more ammo", Kots_Spite, false,
	"kots_mine", "Uses rockets and grenades to create a mine", Kots_Mine, false,
	"kots_conflag", "Kamikaze attack to kill enemies in all directions", Kots_Conflag, false,
	"kots_flail", "Flail about injuring all nearby enemies", Kots_Flail, false,
	"kots_throw", "Throws yourself or an enemy far away", Kots_Throw, false,
	"kots_bide", "Charge up damage received and then dish it out", Kots_Bide, false,
	"kots_spiral", "Send a spiraling tornado of death", Kots_Spiral, false,
	"kots_info", "Open up the character info menu", Kots_Info, false,
	"kots_pointlist", "Open up the character pointlist menu", Kots_Pointlist, false,
	"kots_stats", "Open up the stats scoreboards", Kots_Stats, false,
	"kots_tball", "Teleport you and/or surrounding enemies away", Kots_Tball, false,
	"kots_listip", "List all players IP addresses", Kots_ListIP, true,
	"kots_kick", "Kick a player", Kots_KickUser, true,
	"kots_curse", "Curses a player", Kots_CurseUser, true,
	"kots_ipban", "Bans a player by ip address", Kots_IpBanUser, true,
	"kots_silence", "Silences all players in game for the rest of the level", Kots_Silence, true,
	"kots_unsilence", "Unsilences all players in game allowing them to talk again", Kots_Unsilence, true,
	"kots_mute", "Mutes a single player disallowing him from talking at all", Kots_MuteCommand, true,
	"kots_unmute", "Unmutes a single player allowing him to talk again", Kots_UnmuteCommand, true,
	"kots_unmuteip", "Unmutes a single player by ip allowing him to talk again", Kots_UnmuteIPCommand, true,
	"kots_mutelist", "Shows a list of muted players and ip addresses", Kots_MuteListCommand, true,
	"kots_admin", "Grants admin priviledges without joining the game", Kots_AdminLogin, true,
	"kots_noadmin", "Disables your admin priviledges", Kots_AdminLogout, true,
	"kots_allowlogin", "Allows characters to login again after using kots_nologin", Kots_AllowLogin, true,
	"kots_nologin", "Forces all characters to logout and prevents them from logging in", Kots_NoLogin, true,
	"kots_map", "Changes to the specified map", Kots_Map, true,
	"kots_monster", "Attempt to spawn the specified monster", Kots_SpawnMonster, true,
	"kots_createrune", "Creates the specified rune and drops it in front of you", Kots_CreateRune, true,
	"kots_listrunes", "Shows a list of all runes", Kots_ListRunes, true,
	"kots_showrunes", "Shows a list of all runes on the current map", Kots_ShowRunes, true,
	"kots_destroyrune", "Destroyes the first rune found with this name", Kots_DestroyRune, true,
	"kots_update", "Updates to the latest version of KOTS2007", Kots_Update, true,
	"kots_revert", "Reverts to the last saved version of KOTS2007", Kots_Revert, true,
	"kots_rcon", "Provides rcon-like functionality for KOTS2007 admins", Kots_RCon, true,
	"kots_poison", "Enables/disables rage poison ability", Kots_Poison, false,
	"kots_quadrage", "Enables/disables quad rage ability", Kots_QuadRage, false,
	"kots_laser", "Enables laser sight or laser sight ball", Kots_Laser, false,
	"kots_fakeshard", "Creates a fake armor shard that causes damage", Kots_FakeShard, false,
	"kots_setplayer", "Admin command to set the level of the specified player tree for a character", Kots_SetPlayer, true,
	"kots_setweapon", "Admin command to set the level of the specified weapon for a character", Kots_SetWeapon, true,
	"kots_setpower", "Admin command to set the level of the specified power for a character", Kots_SetPower, true,
	"kots_haste", "Testing for possible new haste ability", Kots_Haste, false,
	"kots_gender", "Sets your characters gender for titles", Kots_Gender, false,
	"kots_hgtimer", "Enables/disables resetting of HG timer", Kots_HgTimer, false,
	"kots_hgspeed", "Enables/disables fixed HG speed", Kots_HgSpeed, false,
	"kots_hgfuse", "Enables/disables shorter HG fuse", Kots_HgFuse, false,
	"kots_respec", "Allows you to respec (reset) one or more player points", Kots_Respec, false,
	"kots_hookcolor", "Allows you to set the color of the laserhook (red, blue, green, orange, yellow)", Kots_HookColor, false,
	"droprune", "Drops the rune you are currently holding", Kots_DropRune, false,
	"maplist", "View the maplist", Kots_Maplist, false,
	"play_voice", "Plays the specified sound originating from your player", Kots_PlayVoice, false,
	"vote", "Vote for monsters or map", Kots_Vote, false
};
#define CommandCount (sizeof(kots_commands) / sizeof(command_t))

void Kots_CheckDuplicateCommands()
{
	int i, j = CommandCount;

	for (i = 0; i < CommandCount; i++)
	{
		for (j = i + 1; j < CommandCount; j++)
		{
			if (Q_stricmp(kots_commands[i].name, kots_commands[j].name) == 0)
			{
				assert(false);
				return;
			}
		}
	}
}

void stuffcmd(edict_t *ent, char *s)
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}

void Kots_Help(edict_t *ent, char *args)
{
	int i;

	Kots_SlowPrint(ent, "\n---------=== KOTS Commands ===---------\n");
	for (i = 0; i < CommandCount; i++)
		if (!kots_commands[i].is_admin || ent->client->pers.kots_persist.is_admin)
			Kots_SlowPrint(ent, "%-15s - %s\n", kots_commands[i].name, kots_commands[i].desc);
}

void Kots_StuffCommands(edict_t *ent)
{
	//TODO: Figure out a way to make something like this work
	//		The current problem is commands that require args
	//		ignore the args and only do "cmd cmdname"
	/*
	int i;
	char cmd[100];

	for (i = 0; i < CommandCount; i++)
	{
		Kots_snprintf(cmd, 100, "alias %s cmd %s\n", kots_commands[i].name, kots_commands[i].name);
		stuffcmd(ent, cmd);
	}
	*/
	char *s = Info_ValueForKey(ent->client->pers.userinfo, kots_pass->name);
	if (!s || !(*s))
		stuffcmd(ent, va("set %s \"\" u\n", kots_pass->name));

	s = Info_ValueForKey(ent->client->pers.userinfo, kots_nas->name);
	if (!s || !(*s))
		stuffcmd(ent, va("set %s 0 u\n", kots_nas->name));

	stuffcmd(ent, "alias +fly kots_fly\n");
	stuffcmd(ent, "alias -fly kots_land\n");
	stuffcmd(ent, "alias +hook kots_hook\n");
	stuffcmd(ent, "alias -hook kots_unhook\n");
	stuffcmd(ent, va("exec %s.cfg\n", ent->client->pers.netname));
}

void Kots_StartGame(edict_t *ent, pmenuhnd_t *hnd)
{
	if (level.intermissiontime)
		return;
	else if (ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You are already in the game.\n");
	else if (!Kots_ServerCanLogin())
		gi.cprintf(ent, PRINT_HIGH, "Login has been temporarily disabled on this server.\n");
	else
	{
		char *pass = Info_ValueForKey(ent->client->pers.userinfo, "kots_pass");
		int len = strlen(pass);

		if (len > 45)
			gi.cprintf(ent, PRINT_HIGH, "Your password can only be 45 characters or less.\n");
		else if (len == 0)
			gi.cprintf(ent, PRINT_HIGH, "No password set. Please use kots_pass \"password\" to set a password.\n");
		else
		{
			Kots_ServerLoginCharacter(ent);
			
			gi.centerprintf(ent, "Loading character, please wait...\n");
			gi.centerprintf(ent, "Visit www.kingoftheserver.org to see the\n"
									"current rankings for this server!\n"
									"https://discord.gg/uZdBxkbCzP #kots\n"
									"Welcome to KOTS2020!\n"
									"LET'S FRAG LIKE THE OLD DAYS!\n"
									"Weekly timeslots:\n"
									"Tuesday 8-10PM EST, Saturdays 6-8PM EST\n");
			PMenu_Close(ent);
		}
	}
}

qboolean Kots_ValidateName(edict_t *ent)
{
	static char *valid_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 -=[]',./!@#%^&*()_+{}|:<>?";
	size_t len = strlen(ent->client->pers.netname);
	
	if (len == 0)
		gi.cprintf(ent, PRINT_HIGH, "Name cannot be blank.\n");
	else if (ent->client->pers.netname[0] == ' ')
		gi.cprintf(ent, PRINT_HIGH, "Names cannot start with a space.\n");
	else if (ent->client->pers.netname[len - 1] == ' ')
		gi.cprintf(ent, PRINT_HIGH, "Names cannot end with a space.\n");
	else if (Q_stricmp(ent->client->pers.netname, "console") == 0)
		gi.cprintf(ent, PRINT_HIGH, "Console is a reserved name.\n");
	else if (!Kots_ValidateString(ent->client->pers.netname, valid_chars))
		gi.cprintf(ent, PRINT_HIGH, "Your name contains one or more invalid characters.\n");
	else // all checks passed so return true
		return true;

	//one of the checks failed so return false
	return false;
}

void Kots_Create(edict_t *ent, pmenuhnd_t *hnd)
{
	char *pass = Info_ValueForKey(ent->client->pers.userinfo, "kots_pass");

	if (level.intermissiontime)
		return;
	else if (ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must first logout to create a new character.\n");
	else if (!pass || *pass == '\0')
		gi.cprintf(ent, PRINT_HIGH, "Please enter a password using kots_pass first.\n");
	else
	{
		//check that name passed validation
		if (!Kots_ValidateName(ent))
			return;

		Kots_ServerCreateCharacter(ent);

		gi.centerprintf(ent, "Creating character, please wait...\n");
		PMenu_Close(ent);
	}
}

void Kots_Observer(edict_t *ent, pmenuhnd_t *hnd)
{
	if (level.intermissiontime)
		return;
	else if (ent->character->is_loggedin)
		Kots_CharacterLogout(ent, true, true);
	else
		gi.cprintf(ent, PRINT_HIGH, "You are already in observer mode.\n");
}

void Kots_StartGame1(edict_t *ent, char *args)
{
	Kots_StartGame(ent, NULL);
}

void Kots_Create1(edict_t *ent, char *args)
{
	Kots_Create(ent, NULL);
}

void Kots_Observer1(edict_t *ent, char *args)
{
	Kots_Observer(ent, NULL);
}

void Kots_ServerList(edict_t *ent, char *args)
{
	if (Kots_MenuCloseIfOpen(ent, "ServerMenu"))
		return;

	Kots_MenuServers(ent, NULL);
}

void Kots_Info(edict_t *ent, char *args)
{
	if (Kots_MenuCloseIfOpen(ent, "CharacterInfo") && (!args || !*args))
		return;

	if (level.intermissiontime)
		return;

	//if we're logged in and want to see our own info
	else if (ent->character->is_loggedin && (!(*args) || strcmp(args, ent->character->name) == 0))
		Kots_MenuShowCharacterInfo(ent, NULL);
	else if (*args != 0) //otherwise load the other players info
	{
		int i;
		edict_t *other;

		//try to find the player in the game
		for (i = 1; i <= maxclients->value; i++)
		{
			other = g_edicts + i;
			if (!other->inuse)
				continue;

			//if this is the character we're looking for and he's logged in
			if (other->character->is_loggedin &&
				Q_stricmp(args, other->character->name) == 0)
			{
				//pull their info directly from the game
				Kots_MenuShowOtherCharacterInfo(ent, other, other->character);
				return;
			}
		}

		//player not found in the game so let's load him from the db
		Kots_ServerLoadKotsInfo(ent, args);
	}
	else if (ent->client->chase_target)
	{
		Kots_MenuShowOtherCharacterInfo(ent, ent->client->chase_target, ent->client->chase_target->character);
	}
}

void Kots_Pointlist(edict_t *ent, char *args)
{
	if (Kots_MenuCloseIfOpen(ent, "Pointlist") && (!args || !*args))
		return;

	if (level.intermissiontime)
		return;

	//if we're logged in and want to see our own pointlist
	else if (ent->character->is_loggedin && (!(*args) || strcmp(args, ent->character->name) == 0))
		Kots_MenuShowPointlist(ent, NULL);
	else if (*args != 0) //otherwise load the other players pointlist
	{
		int i;
		edict_t *other;

		//try to find the player in the game
		for (i = 1; i <= maxclients->value; i++)
		{
			other = g_edicts + i;
			if (!other->inuse)
				continue;

			//if this is the character we're looking for and he's logged in
			if (other->character->is_loggedin &&
				Q_stricmp(args, other->character->name) == 0)
			{
				//pull their info directly from the game
				Kots_MenuShowOtherPointlist(ent, other->character);
				return;
			}
		}

		Kots_ServerLoadPointlist(ent, args);
	}
	else if (ent->client->chase_target)
	{
		Kots_MenuShowOtherPointlist(ent, ent->client->chase_target->character);
	}
}

void Kots_Player(edict_t *ent, char *player)
{
	if (level.intermissiontime)
		return;
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else
		Kots_CharacterAddPlayer(ent, player);
}

void Kots_Power(edict_t *ent, char *power)
{
	if (level.intermissiontime)
		return;
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else
		Kots_CharacterAddPower(ent, power);
}

void Kots_Weapon(edict_t *ent, char *weapon)
{
	if (level.intermissiontime)
		return;
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else
		Kots_CharacterAddWeapon(ent, weapon);
}

void Kots_SetPlayer(edict_t *ent, char *player)
{
	if (level.intermissiontime)
		return;
	else if (ent->client->pers.kots_persist.is_admin)
		Kots_CharacterSetPlayer(ent);
}

void Kots_SetPower(edict_t *ent, char *power)
{
	if (level.intermissiontime)
		return;
	else if (ent->client->pers.kots_persist.is_admin)
		Kots_CharacterSetPower(ent);
}

void Kots_SetWeapon(edict_t *ent, char *weapon)
{
	if (level.intermissiontime)
		return;
	else if (ent->client->pers.kots_persist.is_admin)
		Kots_CharacterSetWeapon(ent);
}

void Kots_Detonate(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else
		Kots_CharacterDetonate(ent);
}

void Kots_Haste(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else if (ent->health <= 0)
		return;
	else
		Kots_CharacterHaste(ent);
}

void Kots_Resist(edict_t *ent, char *resist)
{
	if (level.intermissiontime)
		return;
	else if (!resist || resist[0] == '\0')
		gi.cprintf(ent, PRINT_HIGH, "Usage: %s power or weapon\n", gi.argv(0));
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else
	{
		weaponpoint_t *weapon = Kots_GetWeaponTree(resist);

		if (weapon && weapon->resist)
			weapon->resist(ent);
		else
		{
			powerpoint_t *power = Kots_GetPowerTree(resist);

			if (power && power->resist)
				power->resist(ent);
			else
				gi.cprintf(ent, PRINT_HIGH, "%s is not a valid resistance.\n", resist);
		}
	}
}

void Kots_Respawn(edict_t *ent, char *respawn)
{
	int i;
	int len = strlen(respawn);

	if (level.intermissiontime)
		return;
	else if (!respawn || !len)
		gi.cprintf(ent, PRINT_HIGH, "Usage: %s weapon\n", gi.argv(0));
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else
	{
		for (i = 0; i < TotalWeapons; i++)
		{
			if (Q_stricmp(respawn, WeaponLookup[i].name) == 0)
			{
				ent->character->respawn_weapon = WeaponLookup[i].weapon;
				gi.cprintf(ent, PRINT_HIGH, "Respawn set to %s.\n", respawn);
				return;
			}
		}

		gi.cprintf(ent, PRINT_HIGH, "%s is not a valid respawn.\n", respawn);
	}
}

void Kots_Buy(edict_t *ent, char *args)
{
	int argc = gi.argc();
	if (!ent->character->is_loggedin)
		return;

	if (level.intermissiontime)
		return;
	else if (Q_stricmp(gi.argv(1), "spawns") == 0 || Q_stricmp(gi.argv(1), "respawns") == 0)
	{
		if (gi.argc() < 3)
			Kots_CharacterBuySpawns(ent, 0);
		else
			Kots_CharacterBuySpawns(ent, atoi(gi.argv(2)));
	}
	else if (Q_stricmp(args, "player") == 0)
		Kots_CharacterBuyPlayerPoint(ent);
	else if (Q_stricmp(args, "weapon") == 0)
		Kots_CharacterBuyWeaponPoint(ent);
	else if (Q_stricmp(args, "power") == 0)
		Kots_CharacterBuyPowerPoint(ent);
	else if (argc >= 2)
		gi.cprintf(ent, PRINT_HIGH, "%s is not recognized as something that can be bought.\n", gi.argv(1));
	else
		gi.cprintf(ent, PRINT_HIGH, "%s is not recognized as something that can be bought.\n", args);
}

void Kots_Tball(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_Use_T_Ball(ent, args);
}

void Kots_Fly(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterFly(ent);
}

void Kots_Land(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterLand(ent);
}

void Kots_Cloak(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterToggleCloaking(ent, args);
}

void Kots_Expack(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_ExpackDrop(ent);
}

void Kots_Hook(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterHook(ent);
}

void Kots_Unhook(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterUnhook(ent);
}

void Kots_Jump(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || level.intermissiontime)
		return;

	Kots_CharacterHighJumpToggle(ent, args);
}

void Kots_Health(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterCreateHealth(ent);
}

void Kots_FakeStim(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterCreateFakeStim(ent);
}

void Kots_FakeMega(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterCreateFakeMega(ent);
}

void Kots_FakeShard(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterDropFakeShard(ent);
}

void Kots_Flashlight(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterToggleFlashlight(ent, args);
}

void Kots_Empathy(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterEmpathyToggle(ent, args);
}

void Kots_SpiritSwim(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || level.intermissiontime)
		return;

	Kots_CharacterToggleSpiritSwim(ent, args);
}

void Kots_Lift(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterLiftItem(ent);
}

void Kots_Toss(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterTossItem(ent);
}

void Kots_Armor(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterKotsArmor(ent);
}

void Kots_PowerConvert(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || level.intermissiontime)
		return;

	if (!(*args))
		Kots_CharacterConvertCells(ent);
	else
		Kots_CharacterConvertCellsToggle(ent, args);
}

void Kots_Deflect(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || level.intermissiontime)
		return;

	Kots_CharacterToggleDeflect(ent, args);
}

void Kots_Knock(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || level.intermissiontime)
		return;

	Kots_CharacterToggleKnock(ent, args);
}

void Kots_Poison(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || level.intermissiontime)
		return;

	Kots_CharacterTogglePoison(ent, args);
}

void Kots_QuadRage(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || level.intermissiontime)
		return;

	Kots_CharacterToggleQuadRage(ent, args);
}

void Kots_Laser(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || level.intermissiontime)
		return;
	else
	{
		char *value = gi.argv(1);

		if (gi.argc() == 1 || Q_stricmp(value, "on") == 0 || Q_stricmp(value, "off") == 0)
			Kots_CharacterToggleLaser(ent, value);
		else if (Q_stricmp(value, "ball") == 0)
			Kots_CharacterToggleLaserBall(ent, gi.argv(2));
		else
			gi.cprintf(ent, PRINT_HIGH, "Usage: kots_laser [on/off] or kots_laser ball [on/off]\n");
	}
}

void Kots_Spite(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterSpite(ent);
}

void Kots_Mine(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterDropMine(ent);
}

void Kots_Conflag(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterConflagration(ent);
}

void Kots_Flail(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterFlail(ent);
}

void Kots_Throw(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterThrow(ent, args);
}

void Kots_Bide(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterBide(ent);
}

void Kots_Spiral(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_CharacterSpiral(ent, args);
}

void Kots_DropRune(edict_t *ent, char *args)
{
	if (!ent->character->is_loggedin || ent->health <= 0 || level.intermissiontime)
		return;

	Kots_RuneDrop(ent);
}

void Kots_Maplist(edict_t *ent, char *args)
{
	Kots_Maplist_Show(ent);
}

void Kots_Vote(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;

	else if (gi.argc() == 1)
		gi.cprintf(ent, PRINT_HIGH, "Usage: vote (info/monsters/map/yes/no)\n");
	else
	{
		char *votetype = gi.argv(1);

		if (Q_stricmp(votetype, "info") == 0)
		{
			Kots_MonsterPrintVoteInfo(ent);
			Kots_SlowPrint(ent, "\n"); //separate the two info types
			Kots_Maplist_PrintVoteInfo(ent);
		}
		else if (!ent->character->is_loggedin)
			gi.cprintf(ent, PRINT_HIGH, "You must join the game to vote.\n");
		else if (Q_stricmp(votetype, "monsters") == 0)
			Kots_MonsterVoteCommand(ent);
		else if (Q_stricmp(votetype, "map") == 0)
			Kots_Maplist_VoteCommand(ent);
		else if (Q_stricmp(votetype, "yes") == 0)
			Kots_Maplist_Vote(ent, MAPVOTE_YES);
		else if (Q_stricmp(votetype, "no") == 0)
			Kots_Maplist_Vote(ent, MAPVOTE_NO);
		else
			gi.cprintf(ent, PRINT_HIGH, "Unrecognized vote type. Valid vote types are: monsters, map, yes, no'\n");
	}
}

void Kots_SpawnMonster(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_Admin_SpawnMonster(ent, args);
}

void Kots_CreateRune(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_Admin_CreateRune(ent, args);
}

void Kots_ListIP(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_Admin_ListIP(ent);
}

void Kots_KickUser(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_Admin_KickByName(ent, args);
}

void Kots_CurseUser(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_Admin_CurseByName(ent, args);
}

void Kots_IpBanUser(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_Admin_IpBan(ent, args);
}

void Kots_ListRunes(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_RunesList(ent);
}

void Kots_ShowRunes(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_RunesShow(ent);
}

void Kots_DestroyRune(edict_t *ent, char *args)
{
	if(!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_RuneDestroy(ent, args);
}

void Kots_PlayVoice(edict_t *ent, char *args)
{
	gi.sound(ent, CHAN_VOICE, gi.soundindex(args), 1, ATTN_NORM, 0);
}

void Kots_Silence(edict_t *ent, char *args)
{
	if (!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_AdminSilence(ent);
}

void Kots_Unsilence(edict_t *ent, char *args)
{
	if (!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_AdminUnsilence(ent);
}

void Kots_MuteCommand(edict_t *ent, char *args)
{
	if (ent->client->pers.kots_persist.is_admin)
		Kots_MutePlayer(ent, args);
}

void Kots_UnmuteCommand(edict_t *ent, char *args)
{
	if (ent->client->pers.kots_persist.is_admin)
		Kots_UnmutePlayer(ent, args);
}

void Kots_UnmuteIPCommand(edict_t *ent, char *args)
{
	if (ent->client->pers.kots_persist.is_admin)
		Kots_UnmuteIP(ent, args);
}

void Kots_MuteListCommand(edict_t *ent, char *args)
{
	if (!level.intermissiontime && ent->client->pers.kots_persist.is_admin)
		Kots_MuteList(ent);
}

void Kots_AdminLogin(edict_t *ent, char *args)
{
	if (!level.intermissiontime)
		Kots_ServerAdminLogin(ent);
}

void Kots_AdminLogout(edict_t *ent, char *args)
{
	if (ent->client->pers.kots_persist.is_admin)
	{
		ent->client->pers.kots_persist.is_admin = false;
		gi.cprintf(ent, PRINT_HIGH, "Your admin priviledges have been disabled.\n");
	}
}

void Kots_AllowLogin(edict_t *ent, char *args)
{
	if (ent->client->pers.kots_persist.is_admin)
		Kots_ServerSetAllowLogin(ent, true);
}

void Kots_NoLogin(edict_t *ent, char *args)
{
	if (ent->client->pers.kots_persist.is_admin)
		Kots_ServerSetAllowLogin(ent, false);
}

void Kots_Map(edict_t *ent, char *args)
{
	if (ent->client->pers.kots_persist.is_admin)
		Kots_Maplist_Goto(ent, args);
}

void Kots_Stats(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;

	Kots_HudShowStats(ent, args);
}

qboolean Kots_ToggleValue(char *value, qboolean current)
{
	if (Q_stricmp(value, "on") == 0)
		return true;
	else if (Q_stricmp(value, "off") == 0)
		return false;
	else if (!current)
		return true;
	else
		return false;
}

void Kots_HgTimer(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else
	{
		ent->client->pers.kots_persist.hg_resettimer = Kots_ToggleValue(args, ent->client->pers.kots_persist.hg_resettimer);

		if (ent->client->pers.kots_persist.hg_resettimer)
			gi.cprintf(ent, PRINT_HIGH, "HG timer reset has been enabled.\n");
		else
			gi.cprintf(ent, PRINT_HIGH, "HG timer reset has been disabled..\n");
	}
}

void Kots_HgSpeed(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else
	{
		ent->client->pers.kots_persist.hg_fixedspeed = Kots_ToggleValue(args, ent->client->pers.kots_persist.hg_fixedspeed);

		if (ent->client->pers.kots_persist.hg_fixedspeed)
			gi.cprintf(ent, PRINT_HIGH, "HG fixed speed has been enabled.\n");
		else
			gi.cprintf(ent, PRINT_HIGH, "HG fixed speed has been disabled.\n");
	}
}

void Kots_HgFuse(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else
	{
		ent->client->pers.kots_persist.hg_shortfuse = Kots_ToggleValue(args, ent->client->pers.kots_persist.hg_shortfuse);

		if (ent->client->pers.kots_persist.hg_shortfuse)
			gi.cprintf(ent, PRINT_HIGH, "HG shorter fuse has been enabled.\n");
		else
			gi.cprintf(ent, PRINT_HIGH, "HG shorter fuse has been disabled.\n");
	}
}

void Kots_Update(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else if (ent->client->pers.kots_persist.is_admin)
	{
		Kots_UpdateStartDefault(ent);
	}
}

void Kots_Revert(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else if (ent->client->pers.kots_persist.is_admin)
	{
		Kots_RevertStartDefault(ent);
	}
}

void Kots_RCon(edict_t *ent, char *args)
{
	if (ent->client->pers.kots_persist.is_admin)
	{
		char command[1024];
		Kots_snprintf(command, sizeof(command), "%s\n", args);
		gi.AddCommandString(command);
	}
}

void Kots_Gender(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else if (!ent->character->is_loggedin)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in to set your gender.\n");
		return;
	}
	else if (gi.argc() <= 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "Your current gender is %s.\n", (ent->character->gender != KOTS_GENDER_FEMALE ? "Male" : "Female"));
	}
	else
	{
		if (Q_stricmp(args, "m") == 0 || Q_stricmp(args, "male") == 0)
		{
			ent->character->gender = KOTS_GENDER_MALE;
			gi.cprintf(ent, PRINT_HIGH, "Gender set to male.\n");
			return;
		}
		else if (Q_stricmp(args, "f") == 0 || Q_stricmp(args, "female") == 0)
		{
			ent->character->gender = KOTS_GENDER_FEMALE;
			gi.cprintf(ent, PRINT_HIGH, "Gender set to female.\n");
			return;
		}
	}

	//print usage if fell through to here
	gi.cprintf(ent, PRINT_HIGH, "Usage: %s (m|f|male|female)\n", gi.argv(0));
}

void Kots_Respec(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else
		Kots_CharacterRespecPlayer(ent, (gi.argc() > 1 ? gi.argv(1) : ""));
}

void Kots_HookColor(edict_t *ent, char *args)
{
	if (level.intermissiontime)
		return;
	else if (!ent->character->is_loggedin)
		gi.cprintf(ent, PRINT_HIGH, "You must be logged in first.\n");
	else
		Kots_CharacterHookColor(ent, args);
}

qboolean Kots_Command(edict_t *ent, char *cmd)
{
	int i;
	
	if (Q_stricmp(cmd, "use") == 0 && Q_stricmp(gi.args(), "Blaster") == 0)
	{
		Use_Weapon(ent, FindItem("Sabre"));
		return true;
	}

	if (Q_stricmp(cmd, "kots_pass") == 0)
	{
		stuffcmd(ent, va("set kots_pass \"%s\" u\n", cmd));
	}

	for (i = 0; i < CommandCount; i++)
	{
		if (Q_stricmp(cmd, kots_commands[i].name) == 0)
		{
			if (kots_commands[i].action)
				kots_commands[i].action(ent, gi.args());

			return true;
		}
	}

	if (Q_strncasecmp(cmd, "kots_", 5) == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Unrecognized command '%s'.\n", cmd);
		return true;
	}

	return false;
}

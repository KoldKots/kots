#ifndef KOTS_CHARACTER_H
#define KOTS_CHARACTER_H

#include "g_local.h"

#ifndef DEG2RAD
#define DEG2RAD( a ) ( a * M_PI ) / 180.0F
#endif

#define KOTS_VERSION				"KOTS2007 1.1.0"

#ifndef HEALTH_IGNORE_MAX
#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2
#endif

//define death mods
#define MOD_GRAPPLE					34
#define MOD_SABRE					40
#define MOD_EXPACK					41
#define MOD_SPIRAL					42
#define MOD_BIDE					43
#define	MOD_EMPATHY					44
#define	MOD_FAKEHEALTH				45
#define MOD_SMITE					46
#define MOD_MINE					47
#define MOD_POISON					48
#define MOD_CONFLAG					49
#define MOD_FLAIL					50
#define	MOD_BIDERADIUS				51
#define MOD_SPREEQUIT				52
#define MOD_BOSSEXPLODE				53
#define	MOD_FAKESHARD				54


#if !defined(WIN32) && !defined(Sleep)

#define Sleep(msecs) \
{ \
	struct timespec ts; \
	ts.tv_sec = msecs / 1000; \
	ts.tv_nsec = (msecs - ts.tv_sec * 1000) * 1000000; \
	nanosleep(&ts, NULL); \
}

#endif


//define some new weapons
//a none weapon isn't defined so we'll make our own (used for resist)
#define WEAP_NONE				0
#define WEAP_SABRE				1 /*So we keep the same weapon model as blaster*/
#define WEAP_GRAPPLE			13

//ctf grappling hook constants
//speeds moved to kots_hook.c for convenience
//#define CTF_GRAPPLE_SPEED						1200	/* speed of grapple in flight */
//#define CTF_GRAPPLE_PULL_SPEED				1000	/* speed player is pulled at */

typedef enum {
	CTF_GRAPPLE_STATE_FLY,
	CTF_GRAPPLE_STATE_PULL,
	CTF_GRAPPLE_STATE_HANG
} ctfgrapplestate_t;

//define experience constants
#define KOTS_EXP_BASE				15
#define KOTS_MONSTER_EXP_BASE		3
#define KOTS_MONSTER_EXP_MIN		1
#define KOTS_MONSTER_EXP_MAX		6
#define KOTS_EXP_MIN				5
#define KOTS_EXP_MAX				500
#define KOTS_EXP_BREAK_BONUS		2.5
#define KOTS_EXP_BREAK_MIN			30
#define KOTS_EXP_WARBREAK_BONUS		4.0
#define KOTS_EXP_SPREE_MIN			15
#define KOTS_EXP_SPREE_BONUS		2.0
#define KOTS_EXP_SPREE2_BONUS		3.0
#define KOTS_EXP_SPREEWAR_BONUS		5.0
#define KOTS_EXP_SPREEWAR_MIN		40
#define KOTS_EXP_SPREEWAR_BREAK		80
#define KOTS_EXP_2FER_BONUS			2.0
#define KOTS_EXP_3FER_BONUS			3.0
#define KOTS_EXP_2FER_MIN			20
#define KOTS_EXP_3FER_MIN			40

#define KOTS_ENABLE_MAX_WEAPON_SKILL	1

// define fly constants
#define KOTS_FLY_MAX_CHARGE			3.0
#define KOTS_FLY_COOLDOWN			5.0
#define KOTS_FLY_CHARGE_COOLDOWN	0.8

// define hook constants
#define KOTS_HOOK_MAX_TIME			5.0
#define KOTS_HOOK_COOLDOWN			0.5

//define genders
#define KOTS_GENDER_MALE			'M'
#define KOTS_GENDER_FEMALE			'F'

//define various base values
#define KOTS_BASE_MAX_CUBES			200

//define some constant time values
#define KOTS_SPREE_TIMER			60.0
#define KOTS_NFER_TIMER				1.0
#define KOTS_THANKSTO_TIMER			2.0

//define constant flashlight values
#define KOTS_FLASHLIGHT_NO			0 /*flashlight not on character*/
#define KOTS_FLASHLIGHT_NORM		1 /*normal flashlight on character*/
#define KOTS_FLASHLIGHT_MASTER		2 /*mastered flashlight on character*/

//definite constant lasersight values
#define KOTS_LASERSIGHT_OFF			0
#define KOTS_LASERSIGHT_ON			1
#define KOTS_LASERSIGHT_BALL		2

//define some more constants for items
#define KOTS_ITEM_LIFTABLE					0x0100
#define KOTS_ITEM_MONSTERTOUCHABLE			0x0200
#define KOTS_ITEM_SHARD						0x0400
#define KOTS_ITEM_IDABLE					0x0800
#define KOTS_ITEM_REMOVE					0x1000
#define KOTS_ITEM_TELEFRAG					0x2000
#define KOTS_ITEM_LIFT_PREFERRED			0x4000 /* lift should give preference to these items */

//define armor constants
#define ARMOR_INDEX			ITEM_INDEX(FindItem("Body Armor"))
#define SABRE_INDEX			ITEM_INDEX(FindItem("Sabre"))
#define BLASTER_INDEX		ITEM_INDEX(FindItem("Blaster"))

//define spectator constants
#define KOTS_SPECTATOR_MODE_FREE			0
#define KOTS_SPECTATOR_MODE_CHASE			1
#define KOTS_SPECTATOR_MODE_EYES			2

//define vitarmor bonuses
#define VITA_BONUS		75

//define vithealth bonuses
#define VITH_BONUS		35

//define munition bonuses
#define MUNITION_BONUS	0.0475 /*4.75% per level bonus*/

//define wisdom bonuses
#define WIS_TOUGHNESS	2
#define WIS_LEVEL_BONUS	0.03
#define WIS_MASTERY		(WIS_LEVEL_BONUS * 1.5)

//define technical armor resistance
#define TECH_RESIST_NORMAL	1.4
#define TECH_RESIST_HIGH	1.75
#define TECH_RESIST_SUPER	2.25

//define antiweapon enhancements
#define ANTIWEAPON_BONUS		30
#define ANTIWEAPON_REGEN_BONUS	10

//define character level bonuses
#define CHAR_ARMOR_REGEN_BONUS	5

//start at 20 to leave room for weapon expansion
#define POW_EXPACK		20
#define POW_SPIRAL		21
#define POW_BIDE		22
#define POW_THROW		23
#define POW_ANTIWEAPON	24

//define player abilities
#define KOTS_PLAYER_AVAILABLE	0
#define KOTS_PLAYER_DEXTERITY	1
#define KOTS_PLAYER_STRENGTH	2
#define KOTS_PLAYER_KARMA		3
#define KOTS_PLAYER_WISDOM		4
#define KOTS_PLAYER_TECHNICAL	5
#define KOTS_PLAYER_SPIRIT		6
#define KOTS_PLAYER_RAGE		7
#define KOTS_PLAYER_VITH		8
#define KOTS_PLAYER_VITA		9
#define KOTS_PLAYER_MUNITION	10

//define power abilities
#define KOTS_POWER_AVAILABLE	0
#define KOTS_POWER_EXPACK		1
#define KOTS_POWER_SPIRAL		2
#define KOTS_POWER_BIDE			3
#define KOTS_POWER_THROW		4
#define KOTS_POWER_ANTIWEAPON	5

//define some bonuses
#define KOTS_BONUS_WEAPON		0.025 /*2.5% per level bonus after level 6*/

typedef struct character_s
{
	//basic character info
	int id;
	char name[51];
	char title[51];
	int level;
	int exp;
	char gender;
	qboolean is_cursed;
	qboolean is_admin;

	//player points
	int playerpoints;
	int playersbought;
	int dexterity;
	int strength;
	int karma;
	int wisdom;
	int technical;
	int spirit;
	int rage;
	int vitarmor;
	int vithealth;
	int munition;

	//power points
	int powerpoints;
	int powersbought;
	int expack;
	int spiral;
	int bide;
	int kotsthrow;
	int antiweapon;

	//weapon points
	int weaponpoints;
	int weaponsbought;
	int sabre;
	int shotgun;
	int machinegun;
	int chaingun;
	int supershotgun;
	int grenade;
	int grenadelauncher;
	int rocketlauncher;
	int hyperblaster;
	int railgun;
	int bfg;
	int wantiweapon;

	//resist
	int resist;

	//other info
	int cubes;
	int credits;
	int rune_id;
	int respawn_weapon;
	int respawns;
	int tballs;
	int menu_spawns;
	int respec_points;

	//in-game stats
	int score;
	int shots;
	int hits;
	//int level_kills; //not needed because this is tracked in our normal frags stat
	int player_kills;
	int level_killed;
	int level_dealt;
	int level_taken;
	int level_packs;
	int level_credits;
	int damage_exp;

	//stats
	int kills;
	int killed;
	int telefrags;
	int twofers;
	int threefers;
	int highestfer;
	int sprees;
	int spreewars;
	int spreesbroken;
	int spreewarsbroken;
	int longestspree;
	int suicides;
	int teleports;
	int timeplayed;
	int total_packs;
	int total_credits;

	int spree;			//keeps track of # of kills without dying
	float spreetimer;	//level time for when spree expires

	int nfer;			//# of kills in a short period
	float nfertimer;	//level time when nfer expires

	qboolean canDoubleJump; //player has not yet jumped his extra mid-air jump

	//monster specific stuff
	struct kots_monster_s *monster_info;
	struct monster_build_s *monster_build;
	int spiral_hits; // number of times hit by spiral, 20 = teleport

	//represents current values (includes rune upgrades etc)
	int cur_dexterity;
	int cur_strength;
	int cur_karma;
	int cur_wisdom;
	int cur_technical;
	int cur_spirit;
	int cur_rage;
	int cur_vitarmor;
	int cur_vithealth;
	int cur_munition;
	int cur_expack;
	int cur_spiral;
	int cur_bide;
	int cur_kotsthrow;
	int cur_antiweapon;
	int cur_sabre;
	int cur_shotgun;
	int cur_machinegun;
	int cur_chaingun;
	int cur_supershotgun;
	int cur_grenade;
	int cur_grenadelauncher;
	int cur_rocketlauncher;
	int cur_hyperblaster;
	int cur_railgun;
	int cur_bfg;
	int cur_wantiweapon;

	//timers and counters, etc
	int knockdamage;
	int fakehealths;
	int fakeshards;
	int quadragedamage;
	int armorknock_damage;
	int kots_armor;
	int expacksleft;
	int expacksout;
	int minesout;
	int throwsleft;
	// Aldarn - Bide vars //
	int			bidesleft; // charges
	int			bidedmg; // current damage accumulation
	qboolean	bideon; // a bide on or off?
	float		bidestart; // when the bide was started
	// End //
	int spiralsleft;
	float next_candamage;
	float next_cloak;
	float next_unflashlight;
	float next_spiral;
	float next_regen;
	float next_knock;
	float next_karma_id;
	float next_karma_hud;
	float next_fakehealth;
	float next_fakeshard;
	float next_deflect;
	float next_spite;
	float next_mine;
	float next_flail;
	float next_armorknock;
	float next_quadrage;
	float next_conflag;
	float next_throw;
	float next_save;
	float next_headshot;
	float next_fast_regen; // Aldarn - every 0.2 secs
	float next_empdone;
	float next_dizzydone;
	float next_stundone;
	float next_haste;
	float next_hastecomplete;
	float last_damaged;
	float last_hookrelease;
	float next_hookrelease;

	// fly variables
	float fly_charge;
	float next_flytime;
	float next_flycharge;

	//character state values
	int is_flashlighted;
	qboolean is_poisoned;
	qboolean is_stealth;
	qboolean is_loggedin;
	qboolean is_flying;
	qboolean is_cloaked;
	qboolean using_highjump;
	qboolean using_empathy;
	qboolean using_spiritswim;
	qboolean using_pconvert;

	//pointers
	edict_t *karma_id_ent;
	edict_t *poisonedby;
	edict_t *last_damagedby;
	edict_t	*flashlight;
	edict_t	*lasersight;
	struct rune_s *rune;
} character_t;

typedef struct character_persist_s
{
	int health;
	int armor;
	int weapon;

	//determine whether or not to persist
	//we should persist if dead and possibly other situations
	qboolean persist; 

	//has weapons?
	qboolean shotgun;
	qboolean supershotgun;
	qboolean machinegun;
	qboolean chaingun;
	qboolean grenadelauncher;
	qboolean rocketlauncher;
	qboolean hyperblaster;
	qboolean railgun;
	qboolean bfg;

	//ammo
	int shells;
	int bullets;
	int grenades;
	int rockets;
	int cells;
	int slugs;

} character_persist_t;

// Aldarn - hold all dmg + who did what here
typedef struct
{
	edict_t *warent;
	int		dmgs[MAX_CLIENTS];
} spreewar_t;

spreewar_t spreewar;

//Added Includes to .cpp files now, no need for this

/*
//Lazy way to get all KOTS header files included
//Everything includes kots_character.h including g_local.h
#include "kots_runes.h"
#include "kots_player.h"
#include "kots_weapon.h"
#include "kots_resist.h"
#include "kots_power.h"
#include "kots_abilities.h"
#include "kots_pickups.h"
#include "kots_utils.h"
#include "kots_server.h"
#include "kots_pweapon.h"
#include "kots_bfg.h"
#include "kots_sabre.h"
#include "kots_monster.h"
#include "kots_weapondamage.h"
#include "kots_commands.h"
#include "kots_menu.h"
#include "kots_items.h"
#include "kots_hook.h"
#include "kots_hud.h"
#include "kots_tballs.h"
#include "kots_admin.h"
#include "kots_configstrings.h"
#include "kots_maplist.h"
#include "kots_motd.h"
#include "kots_svcmds.h"
#include "kots_conpersist.h"
#include "kots_update.h"
*/

//Quake 2 prototypes
void spectator_respawn(edict_t *ent);
qboolean IsNeutral(edict_t *ent);
qboolean IsFemale(edict_t *ent);
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void Use_Quad(edict_t *ent, gitem_t *item);
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage);


//Kots prototypes

int Kots_CharacterGetNextLevelExp(int level);
int Kots_CharacterGetDamageExp(edict_t *killer, edict_t *killed, int damage);
int Kots_CharacterGetKillExp(edict_t *killer, edict_t *killed);
void Kots_CharacterAdjustExpLimits(edict_t *target, int *exp);
int Kots_CharacterGetStartHealth(edict_t *ent);
int Kots_CharacterGetMaxRegenHealth(edict_t *ent);
int Kots_CharacterGetStartArmor(edict_t *ent);
int Kots_CharacterGetMaxArmor(edict_t *ent);
int Kots_CharacterGetMaxArmorRegen(edict_t *ent);
void Kots_CharacterCheckLevelUp(edict_t *ent);
void Kots_CharacterAddDamageExp(edict_t *damager, edict_t *damaged, int damage);
void Kots_CharacterAddKillExp(edict_t *killer, edict_t *killed);
void Kots_CharacterLevelUp(edict_t *ent);
void Kots_CharacterInit(edict_t *ent);
void Kots_CharacterFree(edict_t *ent);
void Kots_CharacterClearEdicts(edict_t *ent);
void Kots_CharacterLoad(edict_t *ent);
void Kots_CharacterLogin(edict_t *ent);
void Kots_CharacterLogout(edict_t *ent, qboolean force_respawn, qboolean log_activity);
void Kots_CharacterRespawn(edict_t *ent);
void Kots_CharacterSelectRespawn(edict_t *ent);
void Kots_CharacterCheckResistances(edict_t *targ, edict_t *attacker, int *take, int mod, int dflags);
void Kots_CharacterCheckReductions(edict_t *ent, int *take, int mod);
void Kots_CharacterCheckArmor(edict_t *ent, int *save, int *armor);
void Kots_CharacterPreThink(edict_t *ent, pmove_t *pm);
void Kots_CharacterThink(edict_t *ent, pmove_t *pm);
void Kots_CharacterRunFrame(edict_t *ent);
edict_t *Kots_CharacterGetFakeItemAssist(edict_t *inflictor, int mod);
void Kots_CharacterKilled(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t point, int mod);
void Kots_CharacterAfterKilled(edict_t *targ);
void Kots_CharacterInitAfterSpawn(edict_t *ent);
void Kots_CharacterResetStats(edict_t *ent);
void Kots_CharacterResetTimers(edict_t *ent);
void Kots_CharacterPersist(edict_t *ent, character_t *character, character_persist_t *persist);
void Kots_CharacterLoadPersist(edict_t *ent, character_persist_t *persist);
gitem_t *Kots_GetWeaponById(int weaponid);
char *Kots_CharacterGetResistName(int resist);
char *Kots_CharacterGetRespawnName(int respawn_weapon);
char *Kots_CharacterGetName(edict_t *ent);
char *Kots_CharacterGetFullName(edict_t *ent);
void Kots_CharacterCheckSpree(edict_t *attacker, edict_t *targ, int *exp);
void Kots_CharacterCheckNfer(edict_t *ent, int *exp);
void Kots_CharacterCheckSpreeBreak(edict_t *attacker, edict_t *targ, int *exp);
void Kots_CharacterClientEffects(edict_t *ent);
void Kots_CharacterBeginServerFrame(edict_t *ent);
void Kots_CharacterEndServerFrame(edict_t *ent);
int Kots_CharacterGetMaxCubes(edict_t *ent);
int Kots_CharacterGetRespawnCubes(edict_t *ent);
void Kots_CharacterPickupCubes(edict_t *ent, int cubes);
float Kots_CharacterGetPoisonReduction(edict_t *ent);
void Kots_CharacterPerformDamageCalculations(edict_t *targ, edict_t *attacker, int *take, int *asave, int mod, vec3_t dir, vec3_t point, vec3_t normal, int te_sparks, int dflags);
int Kots_CharacterGetArmor(edict_t *ent);
void Kots_CharacterBuySpawns(edict_t *ent, int buy);
void Kots_CharacterWriteStats(edict_t *ent);
void Kots_CharacterUpdateKarmaIdStats(edict_t *ent);
void Kots_CharacterClientBegin(edict_t *ent);
void Kots_CharacterCheckCubeRot(edict_t *ent);
void Kots_CharacterCheckArmorRot(edict_t *ent);
void Kots_CharacterCheckHealthRot(edict_t *ent);
int Kots_CharacterGetTotalPlayerPoints(edict_t *ent);
int Kots_CharacterGetPurchasedPlayerPoints(edict_t *ent);
int Kots_CharacterGetNextPlayerPointCost(edict_t *ent);
int Kots_CharacterGetTotalWeaponPoints(edict_t *ent);
int Kots_CharacterGetPurchasedWeaponPoints(edict_t *ent);
int Kots_CharacterGetNextWeaponPointCost(edict_t *ent);
int Kots_CharacterGetTotalPowerPoints(edict_t *ent);
int Kots_CharacterGetPurchasedPowerPoints(edict_t *ent);
int Kots_CharacterGetNextPowerPointCost(edict_t *ent);
int Kots_CharacterGetSpawnCost(int spawns);
void Kots_CharacterBuyPlayerPoint(edict_t *ent);
void Kots_CharacterBuyWeaponPoint(edict_t *ent);
void Kots_CharacterBuyPowerPoint(edict_t *ent);
edict_t *Kots_CharacterGetThanksTo(edict_t *targ);
void Kots_CharacterEndSpree(edict_t *ent, qboolean allow_forfeit);
void Kots_CharacterDivideWarExp(edict_t *warent);

#endif

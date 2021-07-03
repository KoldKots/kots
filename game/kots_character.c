#include "kots_character.h"
#include "kots_logging.h"
#include "kots_monster.h"
#include "kots_items.h"
#include "kots_maplist.h"
#include "kots_runes.h"
#include "kots_utils.h"
#include "kots_server.h"
#include "kots_abilities.h"
#include "kots_power.h"
#include "kots_pickups.h"
#include "kots_commands.h"
#include "kots_conpersist.h"
#include "kots_menu.h"

//prototypes used in this file
void Kots_Maplist_CheckVote(edict_t *ent);
static int CheckArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags);

int Kots_CharacterGetNextLevelExp(int level)
{
	switch (level)
	{
	case 0:
		return 500;
	case 1:
		return 1000;
	case 2:
		return 2000;
	case 3:
		return 4000;
	case 4:
		return 8000;
	case 5:
		return 16000;
	case 6:
		return 32000;
	case 7:
		return 60000;
	case 8:
		return 100000;
	case 9:
		return 150000;
	case 10:
		return 200000;
	case 11:
		return 275000;
	case 12:
		return 350000;
	case 13:
		return 425000;
	case 14:
		return 500000;
	default:
		return 100000 * (level - 9);
	}
}

int Kots_CharacterGetDamageExp(edict_t *attacker, edict_t *target, int damage)
{
	//get maximum health to compare
	int max = target->max_health;
	
	//determine the total kill exp for the character
	int exp = Kots_CharacterGetKillExp(attacker, target);

	//damage exp for client is based on 2x max health or the REAL max health
	if (target->client)
		max *= 2;

	Kots_CharacterAdjustExpLimits(target, &exp);

	//players only give half exp from damage
	//monsters still give full exp
	if (target->client)
		exp = (int)ceil(exp / 2.0);

	//then take a percentage of that based on the damage dealt
	return Kots_RandMultiply(exp, (float)damage / max);
}

int Kots_CharacterGetKillExp(edict_t *killer, edict_t *killed)
{
	//if we're calculating kill exp for killing a monster
	if ((killed->svflags & SVF_MONSTER))
	{
		int diff = killer->character->level - killed->character->level;

		//if killed monster is a lower level
		if (diff >= 0)
			return (KOTS_MONSTER_EXP_BASE - diff) + killed->character->monster_info->bonus_exp;
		else
			return (KOTS_MONSTER_EXP_BASE - diff) + killed->character->monster_info->bonus_exp;
	}
	else //otherwise do player exp calculations
	{
		//if they're the same level then just give the base exp
		if (killer->character->level == killed->character->level)
			return KOTS_EXP_BASE;

		//calculate characters 5 and under differently
		else if (killer->character->level <= 5)
		{
			//if we're a higher level than the target
			//calculated differently to avoid steep cost from low level ratios
			if (killer->character->level > killed->character->level)
				return KOTS_EXP_BASE - ((killer->character->level - killed->character->level) * 2);
			
			//otherwise for higher level enemies simply find the difference
			//this gives lower levels more experience at first until the switch to the new calculations
			//for every 5 levels above there's an extra boost as well
			else
				return KOTS_EXP_BASE * (killed->character->level - killer->character->level + ((killed->character->level - killer->character->level) / 5 + 1));
		}
		
		//if we're a higher level than the target then simply take a ratio of the base exp
		else if (killer->character->level > killed->character->level)
			return Kots_Round(KOTS_EXP_BASE * (killed->character->level / (float)killer->character->level));
		
		else //base system on maximum exp, in order to get maximum exp target must be 4x higher level
			return Kots_Round(KOTS_EXP_BASE + ((KOTS_EXP_MAX - KOTS_EXP_BASE) / 3.0) * ((killed->character->level - killer->character->level) / (float)killer->character->level));
	}
}

void Kots_CharacterAdjustExpLimits(edict_t *target, int *exp)
{
	if (!(target->svflags & SVF_MONSTER))
	{
		if (*exp > KOTS_EXP_MAX)
			*exp = KOTS_EXP_MAX;
		else if (*exp < KOTS_EXP_MIN)
			*exp = KOTS_EXP_MIN;
	}
	else
	{
		if (*exp > (KOTS_MONSTER_EXP_MAX + target->character->monster_info->bonus_exp))
			*exp = KOTS_MONSTER_EXP_MAX + target->character->monster_info->bonus_exp;
		else if (*exp < KOTS_MONSTER_EXP_MIN)
			*exp = KOTS_MONSTER_EXP_MIN;
	}
}

int Kots_CharacterGetStartHealth(edict_t *ent)
{
	return 100 + (ent->character->level * 5) + (ent->character->cur_vithealth * VITH_BONUS);
}

int Kots_CharacterGetMaxRegenHealth(edict_t *ent)
{
	return ent->max_health * 2;
}

int Kots_CharacterGetStartArmor(edict_t *ent)
{
	int start = (10 * ent->character->cur_wantiweapon) + (ANTIWEAPON_BONUS * ent->character->cur_antiweapon) + (ent->character->cur_vitarmor * 10);
	return start;
}

int Kots_CharacterGetMaxArmor(edict_t *ent)
{
	return 200 + (ent->character->level * 5) + (ent->character->cur_vitarmor * VITA_BONUS);
}

int Kots_CharacterGetMaxArmorRegen(edict_t *ent)
{
	return (ent->character->cur_antiweapon * 30) + (ent->character->cur_wantiweapon * ANTIWEAPON_REGEN_BONUS) + 
		(ent->character->level * CHAR_ARMOR_REGEN_BONUS) + 50;
}

void Kots_CharacterCheckLevelUp(edict_t *ent)
{
	int leveledup = 0, startHealth;
	while (ent->character->exp >= Kots_CharacterGetNextLevelExp(ent->character->level))
	{
		Kots_CharacterLevelUp(ent);
		++leveledup;
	}

	if (leveledup)
	{
		//if char has less health than started with
		//be nice and give him more
		startHealth = Kots_CharacterGetStartHealth(ent);
		if (ent->health < startHealth)
			ent->health = startHealth;

		ent->max_health = startHealth;

		//don't save monsters
		if (ent->client)
		{
			gi.bprintf(PRINT_HIGH, "%s is now level %i.\n", ent->character->name, ent->character->level);
			if (ent->character->respec_points)
				gi.centerprintf(ent, "You have gained a level!\nYou should use the menu to add your new skills.\nYou also have %d respec_points you may use.",
					ent->character->respec_points);
			else
				gi.centerprintf(ent, "You have gained a level!\nYou should use the menu to add your new skills.\n");
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/keyuse.wav"), 1, ATTN_NORM, 0);
			Kots_ServerSaveCharacter(ent);

			//since we saved now we can update the timer
			ent->character->next_save = level.time + 300 + (rand() % 180);
		}
		else
		{
			gi.bprintf(PRINT_HIGH, "%s is now level %i.\n", ent->character->name, ent->character->level);

			//monsters need to add stats
			Kots_MonsterCreateBuild(ent);
			Kots_MonsterAddBonuses(ent);
		}
	}
}

void Kots_CharacterAddDamageExp(edict_t *damager, edict_t *damaged, int damage)
{
	int exp;

	if (damager == damaged)
		return;

	if (damaged->health < damage)
		damage = damaged->health;

	if (damage <= 0)
		return;

	exp = Kots_CharacterGetDamageExp(damager, damaged, damage);

	damager->character->exp += exp * KOTS_EXP_MULTIPLY;
	damager->character->score += exp * KOTS_EXP_MULTIPLY;
	damager->character->damage_exp += exp * KOTS_EXP_MULTIPLY;
}

void Kots_CharacterAddKillExp(edict_t *killer, edict_t *killed)
{
	int exp;

	if (!killed->character)
		return;

	//on suicide check if we were killed thanks to another player
	if (killer == killed || !killer->character)
	{
		//determine if the kill was assisted by someone else
		killer = Kots_CharacterGetThanksTo(killed);

		if (!killer)
			return;
	}

	++killer->character->kills;

	//don't give kill exp for monsters
	if (!killed->client)
		return;

	exp = Kots_CharacterGetKillExp(killer, killed);

	//monsters can kill other monsters and still get bonuses
	if (killed->client || !killer->client)
	{
		//add nfer and spree bonus exp
		Kots_CharacterCheckNfer(killer, &exp);
		Kots_CharacterCheckSpree(killer, killed, &exp);
		Kots_CharacterCheckSpreeBreak(killer, killed, &exp);
	}

	//enforce min and max exp
	Kots_CharacterAdjustExpLimits(killed, &exp);

	killer->character->exp += exp * KOTS_EXP_MULTIPLY;
	killer->character->score += exp * KOTS_EXP_MULTIPLY;

	//increment number of player kills (mainly for logging algorithms)
	killer->character->player_kills++;
}

void Kots_CharacterLevelUp(edict_t *ent)
{
	ent->character->level++;
	ent->character->playerpoints++;
	ent->character->weaponpoints++;

	if (ent->character->level % 3 == 0)
		ent->character->powerpoints++;

	if (ent->character->level == 10)
		ent->character->respec_points += 5;
}

void Kots_CharacterInit(edict_t *ent)
{
	if (!ent->character)
		ent->character = gi.TagMalloc(sizeof(character_t), TAG_LEVEL);

	memset(ent->character, 0, sizeof(character_t));
}

void Kots_CharacterFree(edict_t *ent)
{
	if (ent->character)
	{
		gi.TagFree(ent->character);
		ent->character = NULL;
	}
}

void Kots_CharacterClearEdicts(edict_t *ent)
{
	int i;
	qboolean remove_packs = true;
	edict_t *other = NULL;

	//Don't bother removing packs for monsters
	if (!ent->client)
		remove_packs = false;
	else
	{
		//check for other players with a different ip
		//if there are none then we're going to remove their packs or reset the credits/cubes to 1
		//this is to prevent people from reconnecting with a different name to pick up their own packs
		other = g_edicts + 1;
		for (i = 1; i < game.maxclients; i++, other++)
		{
			if (other->inuse)
			{
				if (Q_stricmp(other->client->pers.kots_persist.ip_address, ent->client->pers.kots_persist.ip_address) != 0)
				{
					remove_packs = false;
					break;
				}
			}
		}
	}

	other = g_edicts + game.maxclients;
	for (i = game.maxclients; i < (int)maxentities->value; i++, other++)
	{
		if (other->owner == ent && other != ent)
		{
			if (other->item)
			{
				//if we should remove the item
				if (other->item->tag & KOTS_ITEM_REMOVE)
					G_FreeEdict(other);
				else if (other->rune)
					continue;
				else if (remove_packs && other->pack)
				{
					other->pack->credits = 1;
					other->pack->cubes = 1;
				}
				else //otherwise just clear the owner
					other->owner = NULL;
			}
			else if (other->character)
			{
				if (other->character->karma_id_ent == ent)
					other->character->karma_id_ent = NULL;

				if (other->character->last_damagedby == ent)
					other->character->last_damagedby = NULL;

				if (other->character->poisonedby == ent)
				{
					other->character->is_poisoned = false;
					other->character->poisonedby = NULL;
				}
			}
			else
				G_FreeEdict(other);
		}
	}

	UpdateAllChaseCams(ent);
}

void Kots_CharacterLogin(edict_t *ent)
{
	ent->client->pers.kots_persist.is_loggedin = true;
	ent->character->is_loggedin = true;
	ent->character->next_save = level.time + 300 + (rand() % 180);

	//if the character is an admin give admin priviledges from now on
	//we don't want to set this to false unless they disconnect
	if (ent->character->is_admin)
	{
		gi.centerprintf(ent, "You have logged in as an admin.");
		ent->client->pers.kots_persist.is_admin = true;
	}

	//reset war damage
	spreewar.dmgs[ent - g_edicts - 1] = 0;

	spectator_respawn(ent);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("misc/bigtele.wav"), 1, ATTN_NORM, 0);
	
	//kick off the initial server frame to prepare everything
	Kots_CharacterInitAfterSpawn(ent);

	//if character had a rune from a previous map attempt to give it to him
	if (ent->client->pers.kots_persist.rune_id)
		Kots_RuneGive(ent, ent->client->pers.kots_persist.rune_id);
			
	Kots_RuneCheckAssociated(ent);

	if (ent->client && ent->client->pers.kots_persist.using_lasersight && ent->character->cur_technical >= 7)
		Kots_CharacterToggleLaser(ent, "on");
	if (ent->client && ent->client->pers.kots_persist.using_laserball && ent->character->cur_technical >= 7)
		Kots_CharacterToggleLaserBall(ent, "on");

	Kots_CharacterBeginServerFrame(ent);

	//persist flashlight state between maps
	if (ent->character->cur_spirit >= 1 && ent->client->pers.kots_persist.using_flashlight)
		Kots_CharacterFlashlightOn(ent);

	ent->character->expacksout = 0;
	ent->character->minesout = 0;
	ent->character->tballs = 2;
	ent->character->throwsleft = ent->character->cur_kotsthrow * 2;
	ent->character->expacksleft = ent->character->cur_expack * 2;
	ent->character->bidesleft = ent->character->cur_bide;
	ent->character->spiralsleft = ent->character->cur_spiral * 2;

	//give invuln when we first start
	ent->character->next_candamage = level.time + 2;
	ent->client->invincible_framenum = ent->character->next_candamage / FRAMETIME;

	if (ent->character->is_cursed)
		gi.centerprintf(ent, "Your character is cursed!\n");

	//now recreate all monsters to match the new character level average
	Kots_MonsterRecreateAll();

	//check vote with this new player logged in
	Kots_Maplist_CheckVote(ent);

	//TODO: This can be removed at a later date, but to fix a bug it needs to remain for a while
	//Give players that trained powers they resisted or vice versa their points back
	if (ent->character->expack > 0 && ent->character->resist == POW_EXPACK)
	{
		gi.centerprintf(ent, "You cannot train powers you resist.\nPlease re-add your points.");
		ent->character->powerpoints += ent->character->expack;
		ent->character->expack = 0;
		ent->character->resist = WEAP_NONE;
	}
	if (ent->character->spiral > 0 && ent->character->resist == POW_SPIRAL)
	{
		gi.centerprintf(ent, "You cannot train powers you resist.\nPlease re-add your points.");
		ent->character->powerpoints += ent->character->spiral;
		ent->character->spiral = 0;
		ent->character->resist = WEAP_NONE;
	}
	if (ent->character->bide > 0 && ent->character->resist == POW_BIDE)
	{
		gi.centerprintf(ent, "You cannot train powers you resist.\nPlease re-add your points.");
		ent->character->powerpoints += ent->character->bide;
		ent->character->bide = 0;
		ent->character->resist = WEAP_NONE;
	}
	if (ent->character->kotsthrow > 0 && ent->character->resist == POW_THROW)
	{
		gi.centerprintf(ent, "You cannot train powers you resist.\nPlease re-add your points.");
		ent->character->powerpoints += ent->character->kotsthrow;
		ent->character->kotsthrow = 0;
		ent->character->resist = WEAP_NONE;
	}
}

void Kots_CharacterLogout(edict_t *ent, qboolean force_respawn, qboolean log_activity)
{
	//Log suspicious activity
	//We may not always want to log it if we're going to be logging people out in bulk
	//In those situations we should log and then log everyone out
	if (log_activity)
	{
		Kots_LogOnDisconnect();
	}

	Kots_CharacterEndSpree(ent, true);
	ent->client->pers.kots_persist.is_loggedin = false;
	ent->character->is_loggedin = false;
	Kots_ServerSaveCharacter(ent);

	//if still alive drop whatever rune you may have
	if (ent->health > 0 && ent->character->rune)
	{
		edict_t *rune = Kots_RuneDrop(ent);

		//remove the owner so it doesn't get removed when we clear edicts
		if (rune)
			rune->owner = NULL;
	}

	//check the new vote average now that we're logged out
	//Monster votes are now checked at certain intervals
	//Kots_MonsterCheckVote(ent);
	Kots_Maplist_CheckVote(ent);

	//reset war damage
	spreewar.dmgs[ent - g_edicts - 1] = 0;

	if (force_respawn)
	{
		spectator_respawn(ent);
		Kots_CharacterClearEdicts(ent);
		Kots_CharacterInit(ent);

		//reset the rune id since they shouldn't keep it when going back into game
		ent->client->pers.kots_persist.rune_id = 0;
	}

	//clear some stats for the HUD
	ent->character->using_empathy = false;
	ent->character->bideon = false;
	ent->character->bidestart = 0;
	ent->character->karma_id_ent = NULL;

	//now recreate all monsters to match the new character level average
	Kots_MonsterRecreateAll();
}

void Kots_CharacterRespawn(edict_t *ent)
{
	//do calculations now since they're not done if you're dead
	Kots_CharacterInitAfterSpawn(ent);
	Kots_CharacterBeginServerFrame(ent);
	
	ent->health = ent->max_health;

	if (ent->character->tballs < 2)
		ent->character->tballs = 2;

	ent->character->throwsleft = ent->character->cur_kotsthrow * 2;
	ent->character->expacksleft = ent->character->cur_expack * 2;
	ent->character->bidesleft = ent->character->cur_bide;
	ent->character->spiralsleft = ent->character->cur_spiral * 2;
	
	//give invuln when we first respawn
	if (ent->client)
	{
		ent->character->next_candamage = level.time + 2;
		ent->client->invincible_framenum = ent->character->next_candamage / FRAMETIME;
	}
	else //monsters only get 1 second of invuln on spawn
		ent->character->next_candamage = level.time + 1;


	Kots_CharacterPickupCubes(ent, Kots_CharacterGetRespawnCubes(ent));

	if (ent->client)
	{
		ent->client->pers.inventory[ARMOR_INDEX] = Kots_CharacterGetStartArmor(ent);
		ent->client->pers.inventory[SABRE_INDEX] = 1;

		Kots_CharacterSelectRespawn(ent);

		//randomly play mocking sounds
		if (Kots_RandRound(0.2))
		{
			float sound = random();
			int sound_index = 0;

			if (sound >= 0.8)
				sound_index = gi.soundindex("makron/roar1.wav");
			else if (sound >= 0.6)
				sound_index = gi.soundindex("makron/voice.wav");
			else if (sound >= 0.4)
				sound_index = gi.soundindex("makron/voice1.wav");
			else if (sound >= 0.2)
				sound_index = gi.soundindex("makron/voice3.wav");
			else
				sound_index = gi.soundindex("makron/voice4.wav");

			Kots_UnicastSound(ent, sound_index, 1.0);
		}
	}

}

void Kots_CharacterSelectRespawn(edict_t *ent)
{
	//level 1 and lower get an automatic rocket launcher and 10 rockets
	//regardless of whether or not they have a respawn weapon set
	if (ent->character->level <= 1)
	{
		ent->client->newweapon = FindItem("Rocket Launcher");
		ent->client->pers.inventory[ITEM_INDEX(ent->client->newweapon)] = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->newweapon->ammo))] = 10;
	}

	if (ent->character->respawns > 0 && ent->character->respawn_weapon != WEAP_SABRE)
	{
		gitem_t *ammo;
		int index;
		int ammo_bonus = (ent->character->level < 5 ? 1 : 2); //2x ammo at 5
		
		//decrement number of respawns
		--ent->character->respawns;

		//set our new weapon to spawn with
		ent->client->newweapon = Kots_GetWeaponById(ent->character->respawn_weapon);

		//if we're not respawning with grenades then give the weapon and set the ammo type
		if (ent->character->respawn_weapon != WEAP_GRENADES)
		{
			ent->client->pers.inventory[ITEM_INDEX(ent->client->newweapon)] = 1;
			ammo = FindItem(ent->client->newweapon->ammo);
		}
		else
			ammo = ent->client->newweapon;

		//get the inventory index of the ammo type
		index = ITEM_INDEX(ammo);

		//double respawn amount if spirit master
		if (ent->character->cur_spirit >= 7)
			ent->client->pers.inventory[index] += ammo->quantity * 2 * ammo_bonus;
		else
			ent->client->pers.inventory[index] += ammo->quantity * ammo_bonus;

		//Double respawn amount for grenades since your getting screwed for not having a weapon
		if (ent->character->respawn_weapon == WEAP_GRENADES)
			ent->client->pers.inventory[index] *= 2;

		switch (ent->character->respawns)
		{
		case 10:
		case 5:
			gi.centerprintf(ent, "You only have %i respawns remaining!", ent->character->respawns);
			break;
		case 1:
			gi.centerprintf(ent, "You only have 1 respawn remaining!");
			break;
		case 0:
			gi.centerprintf(ent, "You are out of respawns!");
			break;
		default:
			break;
		}
	}
	else
		ent->client->newweapon = Kots_GetWeaponById(WEAP_SABRE);

	ChangeWeapon(ent);
}

void Kots_CharacterCheckResistances(edict_t *targ, edict_t *attacker, int *take, int mod, int dflags)
{
	switch (mod)
	{
	case MOD_HG_SPLASH:
	case MOD_G_SPLASH:
		if (targ->character->cur_strength >= 4)
			*take -= Kots_RandMultiply(*take, 0.25);

		break;

	case MOD_R_SPLASH:
		if (targ->character->cur_wisdom >= 2)
			*take -= Kots_RandMultiply(*take, 0.25);

		break;

	case MOD_SLIME:
		if (targ->character->cur_technical >= 1)
			*take = 0;

		break;

	case MOD_LAVA:
		if (targ->character->cur_technical >= 4)
			*take = 0;
		else if (targ->character->cur_technical >= 2)
			*take -= Kots_RandMultiply(*take, 0.25);

		break;

	case MOD_FAKEHEALTH:
		if (targ->character->cur_spirit >= 6)
			*take -= Kots_RandMultiply(*take, 0.5);
		break;
		
	case MOD_BIDE:
	case MOD_BIDERADIUS:
		if (attacker->character->resist == POW_BIDE)
			*take -= Kots_RandMultiply(*take, 0.5);
		if (targ->character->resist == POW_BIDE)
			*take -= Kots_RandMultiply(*take, 0.25);
		break;
		
	case MOD_SPIRAL:
		if (attacker->character->resist == POW_SPIRAL )
			*take -= Kots_RandMultiply(*take, 0.5);
		if (targ->character->resist == POW_SPIRAL)
			*take -= Kots_RandMultiply(*take, 0.25);
		break;

	case MOD_EXPACK:
		if (targ->character->cur_dexterity >= 7)
			*take -= Kots_RandMultiply(*take, 0.25);

		if (attacker->character->resist == POW_EXPACK)
			*take -= Kots_RandMultiply(*take, 0.5);
		if (targ->character->resist == POW_EXPACK)
			*take -= Kots_RandMultiply(*take, 0.25);
		break;

	default:
		break;
	}

	//monster damages cannot be resisted
	if (!(attacker->svflags & SVF_MONSTER))
	{
		switch (mod)
		{
		//check target and attacker for resistances
		case MOD_SABRE:
			if (attacker->character && attacker->character->resist == WEAP_SABRE)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_SABRE)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_SHOTGUN:
			if (attacker->character && attacker->character->resist == WEAP_SHOTGUN)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_SHOTGUN)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_SSHOTGUN:
			if (attacker->character && attacker->character->resist == WEAP_SUPERSHOTGUN)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_SUPERSHOTGUN)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_MACHINEGUN:
			if (attacker->character && attacker->character->resist == WEAP_MACHINEGUN)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_MACHINEGUN)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_CHAINGUN:
			if (attacker->character && attacker->character->resist == WEAP_CHAINGUN)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_CHAINGUN)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_HG_SPLASH:
		case MOD_HANDGRENADE:
			if (attacker->character && attacker->character->resist == WEAP_GRENADES)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_GRENADES)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_G_SPLASH:
		case MOD_GRENADE:
			if (attacker->character && attacker->character->resist == WEAP_GRENADELAUNCHER)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_GRENADELAUNCHER)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_R_SPLASH:
		case MOD_ROCKET:
			if (attacker->character && attacker->character->resist == WEAP_ROCKETLAUNCHER)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_ROCKETLAUNCHER)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_HYPERBLASTER:
			if (attacker->character && attacker->character->resist == WEAP_HYPERBLASTER)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_HYPERBLASTER)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_RAILGUN:
			if (attacker->character && attacker->character->resist == WEAP_RAILGUN)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_RAILGUN)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		case MOD_BFG_BLAST:
		case MOD_BFG_LASER:
			if (attacker->character && attacker->character->resist == WEAP_BFG)
				*take -= Kots_RandMultiply(*take, 0.5);
			if (targ->character->resist == WEAP_BFG)
				*take -= Kots_RandMultiply(*take, 0.25);
			break;

		default:
			break;
		}
	}
	

	switch (mod)
	{
		//damage types that ignore wisdom resistance
	case MOD_EMPATHY:
	case MOD_POISON:
		break;

	default:
		//if we're not taking any damage then don't bother with this
		//was causing damage to be taken when we have full immunity to lava, etc
		if (targ->character->cur_wisdom >= 4 && *take > 0)
		{
			*take -= WIS_TOUGHNESS;

			//don't allow granting of full immunity to damages
			if (*take <= 0)
				*take = 1;
		}
		break;
	}

	//check for resistance runes
	if (dflags & DAMAGE_ENERGY)
	{
		if (targ->character->rune && targ->character->rune->energy_resist)
			*take -= Kots_RandMultiply(*take, 0.25);
	}
	else
	{
		if (targ->character->rune && targ->character->rune->normal_resist)
			*take -= Kots_RandMultiply(*take, 0.25);
	}

	if (*take < 0)
		*take = 0;
}

void Kots_CharacterCheckReductions(edict_t *ent, int *take, int mod)
{
	switch (mod)
	{
	case MOD_FALLING:
	case MOD_CRUSH:
	case MOD_WATER:
		if (ent->character->cur_dexterity >= 4)
			*take -= Kots_RandMultiply(*take, 0.75);

		break;

	case MOD_EMPATHY:
		if (ent->character->cur_strength >= 10)
			*take -= Kots_RandMultiply(*take, 0.8);
		else if (ent->character->cur_strength >= 7)
			*take -= Kots_RandMultiply(*take, 0.6);
		else if (ent->character->cur_strength >= 6)
			*take -= Kots_RandMultiply(*take, 0.4);
		else if (ent->character->cur_strength >= 3)
			*take -= Kots_RandMultiply(*take, 0.2);

		break;

	case MOD_FLAIL:
		if (ent->character->cur_dexterity >= 10)
			*take -= Kots_RandMultiply(*take, 0.9);
		else if (ent->character->cur_dexterity >= 6)
			*take -= Kots_RandMultiply(*take, 0.5);
		else if (ent->character->cur_dexterity >= 4)
			*take -= Kots_RandMultiply(*take, 0.25);
		break;

	default:
		break;
	}

	if (ent->character->cur_wisdom >= 7)
		*take -= floor((*take) * WIS_MASTERY * ent->character->cur_wisdom);
	else if (ent->character->cur_wisdom > 0)
		*take -= floor((*take) * WIS_LEVEL_BONUS * ent->character->cur_wisdom);

	//just make sure we're not giving health
	if (*take < 0)
		*take = 0;
}

void Kots_CharacterCheckArmor(edict_t *ent, int *save, int *armor)
{
	int total_armor = *armor;

	if (ent->character->cur_technical >= 5)
		total_armor = Kots_RandMultiply(total_armor, TECH_RESIST_HIGH);
	else if (ent->character->cur_technical >= 3)
		total_armor = Kots_RandMultiply(total_armor, TECH_RESIST_NORMAL);

	//if we have enough armor to take the full shot
	if (total_armor > *save)
	{
		int take = *save;

		if (ent->character->cur_technical >= 5)
			take = Kots_Round(take / TECH_RESIST_HIGH);
		else if (ent->character->cur_technical >= 3)
			take = Kots_Round(take / TECH_RESIST_NORMAL);
		
		//ensure we don't take more armor than we have
		if (take > *armor)
			*armor = 0;
		else
			*armor -= take;
	}
	else
	{
		//not enough armor so take it all and only save what we had
		*armor = 0;
		*save = total_armor;
	}
}

void Kots_CharacterPreThink(edict_t *ent, pmove_t *pm)
{
	//only perform these actions when
	//the character is logged in
	if (ent->character->is_loggedin)
	{
		//only perform these when the character is not dead 
		if (ent->health > 0)
		{
			Kots_CharacterCheckFly(ent, pm);
			Kots_CharacterCheckStun(ent, pm);
			Kots_CharacterCheckDizzy(ent, pm);
		}
	}
}

void Kots_CharacterThink(edict_t *ent, pmove_t *pm)
{
	//only perform these actions when
	//the character is logged in
	if (ent->character->is_loggedin)
	{
		//only perform these when the character is not dead
		if (ent->health > 0)
		{ 
			Kots_CharacterCheckDoubleJump(ent, pm);
			Kots_CharacterCheckSpiritSwim(ent, pm);
			Kots_CharacterCheckHaste(ent, pm);
		}
	}
}

void Kots_CharacterRunFrame(edict_t *ent)
{
	//only perform these actions when
	//the character is logged in
	if (ent->character->is_loggedin && !level.intermissiontime) 
	{
		//only perform these when the character is not dead
		if (ent->health > 0)
		{
			Kots_CharacterPlayerId(ent);
			Kots_CharacterBideCheckTimer(ent);

			//TODO: Perhaps we should put each of these timed events
			//		on their own timer?

			//only perform these actions every second
			if (level.time >= ent->character->next_regen)
			{
				//TODO: Do we only want the time spent alive?
				++ent->character->timeplayed;

				ent->character->next_regen = level.time + 1;
				Kots_CharacterCheckTballRegen(ent);
				Kots_CharacterCheckHealthRegen(ent);
				Kots_CharacterCheckArmorRegen(ent);
				Kots_CharacterCheckAmmoRegen(ent);
				Kots_CharacterCheckKnockCounter(ent);
				Kots_CharacterCheckCubeRegen(ent);
				Kots_CharacterCheckConvertCells(ent);
				Kots_CharacterCheckCubeRot(ent);
				Kots_CharacterCheckArmorRot(ent);
				Kots_CharacterCheckHealthRot(ent);
				Kots_CharacterCheckPoisonDamage(ent);

				//ensure that the ent hasn't been freed
				//this will happen if monsters die from the poison
				if (!ent->character)
					return;
			}

			//SWB - It's possible that fast regen and normal
			//regen could happen in the same frame so we don't want else
			if(level.time >= ent->character->next_fast_regen) // Aldarn - This is every 0.2 seconds
			{
				ent->character->next_fast_regen = level.time + 0.2;

				Kots_CharacterFlyUseCubes(ent); // Aldarn - modified so cubes removed faster for fly
				Kots_CharacterCheckKotsArmor(ent); // Aldarn - armor generates quicker
				Kots_CharacterCheckAmmoRot(ent);
				Kots_CharacterEmpathyUseCubes(ent);
				Kots_CharacterCloakUseCubes(ent);
			}
			
			Kots_CharacterCheckHook(ent);
			Kots_CharacterFlyCheckCharge(ent);
			Kots_CharacterCheckConflagration(ent);
		}

		//check level up here rather than when we do damage or kills
		//so we don't see level ups spamming the screen and stuff
		//and so we don't have to do this multiple times in one loop
		//for weapons that have more than one shot
		Kots_CharacterCheckLevelUp(ent);


		//TODO: Add messages and sounds when spree
		//times out and when close to timing out
		if (spreewar.warent == ent && level.time == ent->character->spreetimer)
		{
			Kots_CenterPrintAll("%s's spree war has ended due to inactivity.", ent->character->name);

			ent->character->spree = 0;
			ent->character->spreetimer = 0;
			if (spreewar.warent == ent)
				spreewar.warent = NULL;
		}

		if (ent->character->spree >= 6 && ent->character->spreetimer && level.time >= ent->character->spreetimer)
		{
			int index = gi.soundindex("tank/thud.wav");

			//reset spreetimer so we don't play the sounds again
			ent->character->spreetimer = 0;

			//play three thumps as a reminder
			Kots_QueueSound(ent, index, 1.0, level.time);
			Kots_QueueSound(ent, index, 1.0, level.time + 1.0);
			Kots_QueueSound(ent, index, 1.0, level.time + 2.0);
		}


		if (level.time >= ent->character->nfertimer)
			ent->character->nfer = 0;

		if (level.time >= ent->character->next_save && ent->client)
		{
			ent->character->next_save = level.time + 300 + (rand() % 180);
			Kots_ServerSaveCharacter(ent);
		}
	}
}

edict_t *Kots_CharacterGetThanksTo(edict_t *targ)
{
	if (targ->character->last_damaged && targ->character->last_damaged >= (level.time - KOTS_THANKSTO_TIMER))
	{
		if (targ->character->last_damagedby && targ->character->last_damagedby->character && targ->character->last_damagedby->character->is_loggedin)
			return targ->character->last_damagedby;
	}

	return NULL;
}

edict_t *Kots_CharacterGetFakeItemAssist(edict_t *inflictor, int mod)
{
	//remove friendly fire, we don't care about it here
	switch (mod & ~MOD_FRIENDLY_FIRE)
	{
	case MOD_EXPACK:
	case MOD_FAKEHEALTH:
	case MOD_FAKESHARD:
	case MOD_MINE:
		if (inflictor->target_ent != NULL && inflictor->target_ent->character && inflictor->target_ent->character->is_loggedin)
		{
			if (inflictor->wait >= (level.time - KOTS_THANKSTO_TIMER))
			{
				return inflictor->target_ent;
			}
		}
		break;

	default:
		break;
	}

	return NULL;
}

void Kots_CharacterKilled(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t point, int mod)
{
	edict_t *thanksto = NULL;
	edict_t *assisted = NULL;
	int deadflag = targ->deadflag;

	//treat the target as dead until we're done in here
	//this will prevent the target from "dying" multiple times
	targ->deadflag = DEAD_DEAD;

	//determine if the kill was assisted by a player
	if (targ == attacker || !attacker || !attacker->character)
		thanksto = Kots_CharacterGetThanksTo(targ);

	//determine if the kill was a fake item assist
	assisted = Kots_CharacterGetFakeItemAssist(inflictor, mod);

	//we can't assist our own kills
	if (assisted == targ || assisted == attacker)
		assisted = NULL;

	//if this was our own attack convert the assist to a thanks to kill
	if (targ == attacker)
	{
		//if there's already a thanks to then that takes precidence and you get nothing
		if (thanksto == NULL)
		{
			thanksto = assisted;
			targ->character->last_damagedby = assisted;
			targ->character->last_damaged = level.time;
		}

		assisted = NULL;
	}

	//if the kill was fake item assisted
	if (assisted != NULL)
	{
		char assist_name[128];
		char targ_name[128];
		char attacker_name[128];

		//give the asister kill experience and add to their score
		Kots_CharacterAddKillExp(assisted, targ);
		assisted->client->resp.score++;

		//copy names
		Kots_strncpy(assist_name, Kots_CharacterGetFullName(assisted), sizeof(assist_name));
		Kots_strncpy(targ_name, Kots_CharacterGetFullName(targ), sizeof(targ_name));
		Kots_strncpy(attacker_name, Kots_CharacterGetFullName(attacker), sizeof(attacker_name));

		//print out a message to indicate that this happened
		switch (mod & ~MOD_FRIENDLY_FIRE)
		{
		case MOD_EXPACK:
			gi.bprintf(PRINT_HIGH, "%s repositioned %s's expack in front of %s.\n", assist_name, attacker_name, targ_name);
			break;
		case MOD_FAKEHEALTH:
			gi.bprintf(PRINT_HIGH, "%s repositioned %s's fake health in front of %s.\n", assist_name, attacker_name, targ_name);
			break;
		case MOD_FAKESHARD:
			gi.bprintf(PRINT_HIGH, "%s repositioned %s's fake shard in front of %s.\n", assist_name, attacker_name, targ_name);
			break;
		case MOD_MINE:
			gi.bprintf(PRINT_HIGH, "%s repositioned %s's mine in front of %s.\n", assist_name, attacker_name, targ_name);
			break;
		default:
			break;
		}
	}

	Kots_CharacterBideExplode(targ);

	//drop the pack to the assisted killer if there is one
	if (thanksto)
		Kots_PackDrop(targ, thanksto);
	else
		Kots_PackDrop(targ, attacker);

	Kots_CharacterDropStims(targ);
	Kots_CharacterDropShards(targ);
	Kots_RuneDeathDrop(targ);

	//don't lose exp if the kill was thanks to another player
	// or wasn't a suicide
	if (!thanksto && (!attacker || targ == attacker || !attacker->character))
	{
		int explost = Kots_CharacterGetKillExp(targ, targ);
		
		//determine how much extra exp we lost
		if (spreewar.warent == targ)
			explost += KOTS_EXP_SPREEWAR_MIN;
		else if (targ->character->spree >= 6)
			explost += (KOTS_EXP_SPREEWAR_MIN / 2);

		++targ->character->suicides;

		if (targ->character->exp >= explost)
		{
			targ->character->exp -= explost * KOTS_EXP_MULTIPLY;
			targ->character->score -= explost * KOTS_EXP_MULTIPLY;
		}
		else
		{
			targ->character->score -= targ->character->exp * KOTS_EXP_MULTIPLY;
			targ->character->exp = 0;
		}

		if (targ->character->spree >= 6)
		{
			char *sex;

			if (IsNeutral(targ))
				sex = "its";
			else if (IsFemale(targ))
				sex = "her";
			else
				sex = "his";

			if (spreewar.warent == targ)
			{
				spreewar.warent = NULL;
				Kots_CenterPrintAll("%s broke %s own spree war! Everyone point and laugh!", Kots_CharacterGetName(targ), sex);
				Kots_CharacterDivideWarExp(targ);
			}
			else
				gi.bprintf(PRINT_HIGH, "%s broke %s own spree!\n", Kots_CharacterGetName(targ), sex);
		}
	}
	else //wasn't a suicide
	{
		//keep track of when the last player was killed for monster spawning
		if (targ->client && attacker->client)
			level.last_playerkilled = level.time;
	}
	
	//reset some spree values
	targ->character->spree = 0;
	targ->character->spreetimer = 0;
	++targ->character->killed;
	++targ->character->level_killed;

	//show map reminder if necessary
	if (level.mapvotestarted && targ->client && targ->client->pers.kots_persist.map_vote == MAPVOTE_NONE)
	{
		gi.centerprintf(targ, "There is a map vote in progress and you have not voted yet.");
	}

	//restore the previous deadflag
	targ->deadflag = deadflag;
}

void Kots_CharacterAfterKilled(edict_t *targ)
{
	//reset the timers when killed
	Kots_CharacterResetTimers(targ);
}

void Kots_CharacterInitAfterSpawn(edict_t *ent)
{
	//initialize regen timers
	ent->character->next_regen = level.time + 1;
	ent->character->next_fast_regen = level.time + 0.2;

	//initialize damage timers
	ent->character->last_damaged = 0;
	ent->character->last_damagedby = NULL;

	//reset stats for next spawn
	Kots_CharacterResetStats(ent);
}

void Kots_CharacterResetStats(edict_t *ent)
{
	//only calculate the values if not cursed
	//cursed characters permanently have level 0 everything
	if (!ent->character->is_cursed)
	{
		//calculate values and other stuff that
		//will remain true until the next frame
		ent->character->cur_dexterity = ent->character->dexterity;
		ent->character->cur_strength = ent->character->strength;
		ent->character->cur_karma = ent->character->karma;
		ent->character->cur_wisdom = ent->character->wisdom;
		ent->character->cur_technical = ent->character->technical;
		ent->character->cur_spirit = ent->character->spirit;
		ent->character->cur_rage = ent->character->rage;
		ent->character->cur_vitarmor = ent->character->vitarmor;
		ent->character->cur_vithealth = ent->character->vithealth;
		ent->character->cur_munition = ent->character->munition;
		ent->character->cur_expack = ent->character->expack;
		ent->character->cur_spiral = ent->character->spiral;
		ent->character->cur_bide = ent->character->bide;
		ent->character->cur_kotsthrow = ent->character->kotsthrow;
		ent->character->cur_antiweapon = ent->character->antiweapon;
		ent->character->cur_sabre = ent->character->sabre;
		ent->character->cur_shotgun = ent->character->shotgun;
		ent->character->cur_machinegun = ent->character->machinegun;
		ent->character->cur_chaingun = ent->character->chaingun;
		ent->character->cur_supershotgun = ent->character->supershotgun;
		ent->character->cur_grenade = ent->character->grenade;
		ent->character->cur_grenadelauncher = ent->character->grenadelauncher;
		ent->character->cur_rocketlauncher = ent->character->rocketlauncher;
		ent->character->cur_hyperblaster = ent->character->hyperblaster;
		ent->character->cur_railgun = ent->character->railgun;
		ent->character->cur_bfg = ent->character->bfg;
		ent->character->cur_wantiweapon = ent->character->wantiweapon;
	}
	else
	{
		ent->character->cur_dexterity = 0;
		ent->character->cur_strength = 0;
		ent->character->cur_karma = 0;
		ent->character->cur_wisdom = 0;
		ent->character->cur_technical = 0;
		ent->character->cur_spirit = 0;
		ent->character->cur_rage = 0;
		ent->character->cur_vitarmor = 0;
		ent->character->cur_vithealth = 0;
		ent->character->cur_munition = 0;
		ent->character->cur_expack = 0;
		ent->character->cur_spiral = 0;
		ent->character->cur_bide = 0;
		ent->character->cur_kotsthrow = 0;
		ent->character->cur_antiweapon = 0;
		ent->character->cur_sabre = 0;
		ent->character->cur_shotgun = 0;
		ent->character->cur_machinegun = 0;
		ent->character->cur_chaingun = 0;
		ent->character->cur_supershotgun = 0;
		ent->character->cur_grenade = 0;
		ent->character->cur_grenadelauncher = 0;
		ent->character->cur_rocketlauncher = 0;
		ent->character->cur_hyperblaster = 0;
		ent->character->cur_railgun = 0;
		ent->character->cur_bfg = 0;
		ent->character->cur_wantiweapon = 0;
	}
}

void Kots_CharacterResetTimers(edict_t *ent)
{
	//TODO: Character class should probably be broken up
	//		There should be a timers structure with all these in it
	//		Then we can just use memset to reset everything and make our lives easier

	//reset timers and stuff
	ent->character->knockdamage = 0;
	ent->character->armorknock_damage = 0;

	if (ent->character->quadragedamage > 200)
		ent->character->quadragedamage -= 100;
	else
		ent->character->quadragedamage /= 2;

	ent->character->next_cloak = 0;
	ent->character->next_unflashlight = 0;
	ent->character->next_regen = 0;
	ent->character->next_knock = 0;
	ent->character->next_karma_id = 0;
	ent->character->next_karma_hud = 0;
	ent->character->next_deflect = 0;
	ent->character->next_spite = 0;
	ent->character->next_mine = 0;
	ent->character->next_flail = 0;
	ent->character->next_armorknock = 0;
	ent->character->next_quadrage = 0;
	ent->character->next_conflag = 0;
	ent->character->next_throw = 0;
	ent->character->next_fakehealth = 0;
	ent->character->next_fakeshard = 0;
	ent->character->next_empdone = 0;
	ent->character->next_dizzydone = 0;
	ent->character->next_stundone = 0;
	ent->character->next_haste = 0;
	ent->character->next_hastecomplete = 0;
	ent->character->is_poisoned = false;
	ent->character->is_flying = false;
	ent->character->is_cloaked = false;
	ent->character->using_empathy = false;
	ent->character->karma_id_ent = NULL;
	ent->character->poisonedby = NULL;
	ent->character->bidestart = 0;
	ent->character->bidedmg = 0;
	ent->character->bideon = false;
	ent->character->last_hookrelease = 0;
	ent->character->next_hookrelease = 0;
	ent->character->fly_charge = KOTS_FLY_MAX_CHARGE;
	ent->character->next_flycharge = 0;
	ent->character->next_flytime = 0;
}

void Kots_CharacterEndSpree(edict_t *ent, qboolean allow_forfeit)
{
	//don't do any of this stuff if at intermission
	if (!level.intermissiontime && allow_forfeit)
	{
		//if the character was on a spree
		if (ent->character->spree >= 6)
		{
			int clients = Kots_MonsterGetClientsLoggedIn();
			float lastdmg = KOTS_SPREE_TIMER - (ent->character->spreetimer - level.time);

			if ((lastdmg <= 20.0 || spreewar.warent == ent) && clients > 1)
			{
				//do spree quit damage from last damaged by if exists
				if (ent->character->last_damagedby)
					T_Damage(ent, ent->character->last_damagedby, ent->character->last_damagedby, ent->velocity, ent->s.origin, NULL, ent->health + 1000, 0, DAMAGE_NO_BONUS | DAMAGE_NO_RESIST | DAMAGE_NO_PROTECTION | DAMAGE_NO_ARMOR, MOD_SPREEQUIT);
				else
					T_Damage(ent, ent, ent, ent->velocity, ent->s.origin, NULL, ent->health + 1000, 0, DAMAGE_NO_BONUS | DAMAGE_NO_RESIST | DAMAGE_NO_PROTECTION | DAMAGE_NO_ARMOR, MOD_SPREEQUIT);
			}
		}

		//ensure that the war is ended
		if (ent == spreewar.warent)
		{
			Kots_CenterPrintAll("%s's war ended with a wimper!\n", ent->character->name);
			memset(&spreewar, 0, sizeof(spreewar));
		}
		
		//reset spree values just to be thorough
		ent->character->spree = 0;
		ent->character->spreetimer = 0;
	}
	else
	{
		//ensure that the war is ended
		if (ent == spreewar.warent)
		{
			Kots_CenterPrintAll("%s is unstoppable!\n", ent->character->name);
			Kots_CharacterDivideWarExp(ent); //divide the war exp up
			memset(&spreewar, 0, sizeof(spreewar));
		}
	}
}

void Kots_CharacterCheckSpreeBreak(edict_t *attacker, edict_t *targ, int *exp)
{
	if (targ->character->spree >= 25)
	{
		spreewar.warent = NULL;
		++attacker->character->spreewarsbroken;
		Kots_CenterPrintAll("%s's spree war was broken by %s!", Kots_CharacterGetName(targ), Kots_CharacterGetName(attacker));
		Kots_CharacterDivideWarExp(targ);

		*exp *= KOTS_EXP_WARBREAK_BONUS;

		if ((*exp) < KOTS_EXP_SPREEWAR_BREAK)
			*exp = KOTS_EXP_SPREEWAR_BREAK;
	}
	else if (targ->character->spree >= 6)
	{
		++attacker->character->spreesbroken;
		gi.bprintf(PRINT_HIGH, "%s's %i frag spree was broken by %s.\n", Kots_CharacterGetName(targ), targ->character->spree, Kots_CharacterGetName(attacker));
		*exp *= KOTS_EXP_BREAK_BONUS;
		
		if ((*exp) < KOTS_EXP_BREAK_MIN)
			*exp = KOTS_EXP_BREAK_MIN;
	}
}

void Kots_CharacterDivideWarExp(edict_t *warent)
{
	edict_t	*cl_ent,*highest_user;
	int		i,j,totaldmg=0,bonusexp=0,highest_dmg=0;
	float	percent,highest_percent;

	// Aldarn - divide out war exp
	for (j=0;j<game.maxclients;j++)
		totaldmg += spreewar.dmgs[j];

	if (totaldmg > 0)
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;
			if (!cl_ent->character->is_loggedin)
				continue;
			if(spreewar.dmgs[i] > 0)
			{
				percent = (float)spreewar.dmgs[i] / totaldmg;
				bonusexp = (int)(KOTS_EXP_MAX * percent); // total bonus exp is max
				cl_ent->character->exp += bonusexp * KOTS_EXP_MULTIPLY;
				cl_ent->character->score += bonusexp * KOTS_EXP_MULTIPLY;
				gi.cprintf(cl_ent,PRINT_HIGH,"You got %i bonus exp for doing %i damage (%03.2f%%) on %s's SpreeWar!\n",bonusexp,spreewar.dmgs[i],percent * 100,Kots_CharacterGetName(warent));
				
				if(spreewar.dmgs[i] > highest_dmg)
				{
					highest_dmg = spreewar.dmgs[i];
					highest_percent = percent;
					highest_user = cl_ent;
				}
				
				spreewar.dmgs[i] = 0;
			}	
		}

		gi.bprintf(PRINT_HIGH,"%s did the highest damage of %i (%03.2f%%) on %s's spree!\n",Kots_CharacterGetName(highest_user),highest_dmg,highest_percent * 100,Kots_CharacterGetName(warent));
	}
}

void Kots_CharacterCheckSpree(edict_t *attacker, edict_t *targ, int *exp)
{
	//only count kills while alive towards sprees
	if (attacker->health > 0)
	{
		//determine the minimum level of the target to count towards your spree
		int minlevel = attacker->character->level / 4;

		//anyone level 10 or lower can kill anyone and it counts toward sprees
		if (attacker->character->level <= 10)
			minlevel = 0;

		//between 11 and 13 subtracting 10 levels is lower than dividing by 4 so do that
		else if (attacker->character->level <= 13)
			minlevel = attacker->character->level - 10;

		else //anything else should be 1/4 of the attackers level
			minlevel = attacker->character->level / 4;

		//if the target was below the minimum level requirement then don't count toward spree
		if (targ->character->level < minlevel)
			return;

		++attacker->character->spree;

		switch (attacker->character->spree)
		{
		case 50:
			gi.sound(attacker, CHAN_AUTO, gi.soundindex("makron/laf4.wav"), 1.0, ATTN_NONE, 0);
			break;
		case 25:
		case 16:
		case 6:
			attacker->character->tballs += 2;
			gi.sound(attacker, CHAN_AUTO, gi.soundindex("player/lava1.wav"), 1.0, ATTN_NONE, 0);
			break;
		default:
			break;
		}

		//don't end monster spreewars
		if (attacker->client && attacker->character->spree >= 50)
		{
			Kots_CenterPrintAll("%s reached the maximum spree war limit!", attacker->character->name);
			Kots_CharacterDivideWarExp(attacker); //divide up war exp

			//reset the spree count to 0
			attacker->character->longestspree = attacker->character->spree + 1;
			attacker->character->spree = 0;
			spreewar.warent = NULL;
		}
		else if (attacker->character->spree == 25)
		{
			Kots_CenterPrintAll("%s is on a spree war!\n", Kots_CharacterGetName(attacker), attacker->character->spree);
			++attacker->character->spreewars;
			spreewar.warent = attacker;

			if (attacker->client)
			{
				//if a client warred kill all monsters
				Kots_MonsterKillAll();

				//check for suspicious wars
				Kots_LogSpreeWar();
			}
		}
		else if (attacker->character->spree == 6)
		{
			++attacker->character->sprees;
			attacker->character->spreetimer = level.time + KOTS_SPREE_TIMER;
		}

		if (attacker->character->spree > 25)
		{
			gi.bprintf(PRINT_HIGH, "%s is on a %i frag spreewar.\n", Kots_CharacterGetName(attacker), attacker->character->spree);
			*exp *= KOTS_EXP_SPREEWAR_BONUS;

			if ((*exp) < KOTS_EXP_SPREEWAR_MIN)
				*exp = KOTS_EXP_SPREEWAR_MIN;
			
			if (attacker->character->spree > attacker->character->longestspree)
				attacker->character->longestspree = attacker->character->spree;
		}
		else if (attacker->character->spree >= 6)
		{
			//don't print the message at a 25 spree because there's already one
			if (attacker->character->spree != 25)
				gi.bprintf(PRINT_HIGH, "%s is on a %i frag spree.\n", Kots_CharacterGetName(attacker), attacker->character->spree);

			if (attacker->character->spree < 16)
				*exp *= KOTS_EXP_SPREE_BONUS;
			else
				*exp *= KOTS_EXP_SPREE2_BONUS;

			if ((*exp) < KOTS_EXP_SPREE_MIN)
				*exp = KOTS_EXP_SPREE_MIN;

			if (attacker->character->spree > attacker->character->longestspree)
				attacker->character->longestspree = attacker->character->spree;
		}

		//on frag sprees of 16 or higher force the character to drop rune
		if (attacker->character->spree >= 16 && attacker->character->rune)
			Kots_RuneDrop(attacker);
	}
}

void Kots_CharacterCheckNfer(edict_t *ent, int *exp)
{
	++ent->character->nfer;
	ent->character->nfertimer = level.time + KOTS_NFER_TIMER;
	
	if (ent->character->nfer > 1)
	{
		gi.bprintf(PRINT_HIGH, "%s got a %ifer.\n", Kots_CharacterGetName(ent), ent->character->nfer);

		if (ent->character->nfer > ent->character->highestfer)
			ent->character->highestfer = ent->character->nfer;

		if (ent->character->nfer == 2)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex("world/brkglas.wav"), 1.0, ATTN_NONE, 0);
			++ent->character->twofers;

			*exp *= KOTS_EXP_2FER_BONUS;

			if ((*exp) < KOTS_EXP_2FER_MIN)
				*exp = KOTS_EXP_2FER_MIN;
		}
		else
		{
			if (ent->character->nfer == 3)
				++ent->character->threefers;

			*exp *= KOTS_EXP_3FER_BONUS;

			if ((*exp) < KOTS_EXP_3FER_MIN)
				*exp = KOTS_EXP_3FER_MIN;

			if (ent->client && ent->health > 0)
			{
				//give 5 seconds of quad
				if (ent->client->quad_framenum > level.framenum)
					ent->client->quad_framenum += 5 * 10;
				else
					ent->client->quad_framenum = level.framenum + 5 * 10;

				//play the quad sound
				gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
			}
		}
	}
}

void Kots_CharacterPersist(edict_t *ent, character_t *character, character_persist_t *persist)
{
	//even though cubes isn't a "persist" value it sort of makes sense to limit it here because it is "persisted"
	int max_cubes;
	
	//ensure that we remove all rune abilities to prevent them from affecting max armor and health 
	if (ent->character->rune)
		Kots_RuneRemoveAbilities(ent, ent->character->rune);

	max_cubes = Kots_CharacterGetMaxCubes(ent);
	
	if (character->cubes > max_cubes)
		character->cubes = max_cubes;

	if (ent->health > 0)
	{
		int max_health = Kots_CharacterGetStartHealth(ent);
		int max_armor = Kots_CharacterGetMaxArmor(ent);

		persist->health = (ent->health <= max_health ? ent->health : max_health);
		persist->armor = ent->client->pers.inventory[ARMOR_INDEX];

		//limit persisted armor to maximum armor
		if (persist->armor > max_armor)
			persist->armor = max_armor;

		if (ent->client->pers.weapon)
			persist->weapon = ent->client->pers.weapon->weapmodel;
		else //why don't we have a wepaon here?
			persist->weapon = WEAP_SABRE;

		persist->shotgun = (ent->client->pers.inventory[ITEM_INDEX(FindItem("Shotgun"))] ? 1 : 0);
		persist->supershotgun = (ent->client->pers.inventory[ITEM_INDEX(FindItem("Super Shotgun"))] ? 1 : 0);
		persist->machinegun = (ent->client->pers.inventory[ITEM_INDEX(FindItem("Machinegun"))] ? 1 : 0);
		persist->chaingun = (ent->client->pers.inventory[ITEM_INDEX(FindItem("Chaingun"))] ? 1 : 0);
		persist->grenadelauncher = (ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenade Launcher"))] ? 1 : 0);
		persist->rocketlauncher = (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] ? 1 : 0);
		persist->hyperblaster = (ent->client->pers.inventory[ITEM_INDEX(FindItem("Hyperblaster"))] ? 1 : 0);
		persist->railgun = (ent->client->pers.inventory[ITEM_INDEX(FindItem("Railgun"))] ? 1 : 0);
		persist->bfg = (ent->client->pers.inventory[ITEM_INDEX(FindItem("BFG10K"))] ? 1 : 0);
		persist->shells = ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))];
		persist->bullets = ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))];
		persist->grenades = ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))];
		persist->rockets = ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))];
		persist->cells = ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))];
		persist->slugs = ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))];
		persist->persist = 1;
	}
	else
	{
		memset(persist, 0, sizeof(*persist));
		persist->persist = 0;
	}

	//if we removed rune abilities add the back
	if (ent->character->rune)
		Kots_RuneAddAbilities(ent, ent->character->rune);
}

void Kots_CharacterLoadPersist(edict_t *ent, character_persist_t *persist)
{
	if (persist->persist)
	{
		ent->health = persist->health;
		ent->client->pers.inventory[ARMOR_INDEX] = persist->armor;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Shotgun"))] = (int)persist->shotgun;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Super Shotgun"))] = (int)persist->supershotgun;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Machinegun"))] = (int)persist->machinegun;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Chaingun"))] = (int)persist->chaingun;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenade Launcher"))] = (int)persist->grenadelauncher;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] = (int)persist->rocketlauncher;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Hyperblaster"))] = (int)persist->hyperblaster;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Railgun"))] = (int)persist->railgun;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("BFG10K"))] = (int)persist->bfg;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] = persist->shells;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] = persist->bullets;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] = persist->grenades;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] = persist->rockets;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] = persist->cells;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] = persist->slugs;
		ent->client->newweapon = Kots_GetWeaponById(persist->weapon);
		ChangeWeapon(ent);
	}
	else //our last save was while we were dead so start over from respawn
		Kots_CharacterRespawn(ent);
}

gitem_t *Kots_GetWeaponById(int weaponid)
{
	switch (weaponid)
	{
	case WEAP_SHOTGUN:
		return FindItem("Shotgun");
	case WEAP_SUPERSHOTGUN:
		return FindItem("Super Shotgun");
	case WEAP_MACHINEGUN:
		return FindItem("Machinegun");
	case WEAP_CHAINGUN:
		return FindItem("Chaingun");
	case WEAP_GRENADES:
		return FindItem("Grenades");
	case WEAP_GRENADELAUNCHER:
		return FindItem("Grenade Launcher");
	case WEAP_ROCKETLAUNCHER:
		return FindItem("Rocket Launcher");
	case WEAP_HYPERBLASTER:
		return FindItem("Hyperblaster");
	case WEAP_RAILGUN:
		return FindItem("Railgun");
	case WEAP_BFG:
		return FindItem("BFG10K");
	case WEAP_SABRE:
	default:
		return FindItem("Sabre");
	}
}

char *Kots_CharacterGetResistName(int resist)
{
	gitem_t *weapon;
	
	switch (resist)
	{
	case WEAP_NONE:
		return "None";
	case POW_EXPACK:
		return "Expack";
	case POW_SPIRAL:
		return "Spiral";
	case POW_BIDE:
		return "Bide";
	case POW_THROW:
		return "Throw";
	default:
		weapon = Kots_GetWeaponById(resist);
		return weapon->pickup_name;
	}
}

char *Kots_CharacterGetRespawnName(int respawn_weapon)
{
	gitem_t *weapon = Kots_GetWeaponById(respawn_weapon);
	return weapon->pickup_name;
}

char *Kots_CharacterGetName(edict_t *ent)
{
	if (ent->character && ent->character->name)
		return ent->character->name;
	else
		return ent->classname;
}

char *Kots_CharacterGetFullName(edict_t *ent)
{
	if (ent->svflags & SVF_MONSTER)
	{
		static char fullname[100];
		Kots_snprintf(fullname, 100, "Level %i %s", ent->character->level, Kots_CharacterGetName(ent));
		return &fullname[0];
	}
	else
		return ent->character->name;
}

void Kots_CharacterClientEffects(edict_t *ent)
{
	if (!ent->character || !ent->character->is_loggedin)
		return;

	if (ent->health > 0)
	{
		//check if war is in progress
		if (spreewar.warent != NULL)
		{
			ent->s.effects |= EF_COLOR_SHELL | EF_HYPERBLASTER;

			//warrer is white, others red
			if (spreewar.warent == ent)
				ent->s.renderfx |= RF_SHELL_BLUE | RF_SHELL_GREEN | RF_SHELL_RED;
			else
				ent->s.renderfx |= RF_SHELL_RED;

			ent->s.renderfx |= RF_GLOW;
		}

		//check for spree
		else if (ent->character->spree >= 6)
		{
			ent->s.effects |= EF_COLOR_SHELL | EF_HYPERBLASTER;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}

		//add rocket trail behind players using haste
		if (ent->character->next_hastecomplete > level.time)
			ent->s.effects |= EF_ROCKET;

		if (ent->character->bideon && ent->character->bidestart > level.time - 10)
			ent->s.effects |= EF_FLAG1;

		//check character cloaking state
		Kots_CharacterCheckCloak(ent);
	}
}

void Kots_CharacterBeginServerFrame(edict_t *ent)
{
	if (!ent->character->is_loggedin || ent->health <= 0) 
		return;

	//monsters have their own max health so don't recalculate
	if (ent->client)
	{
		ent->max_health = Kots_CharacterGetStartHealth(ent);
	}

	//enable/disable powers that may or may not be available now
	ent->character->is_stealth = (ent->character->cur_dexterity >= 2 ? true : false);
	if (ent->client && ent->client->pers.kots_persist.using_flashlight && ent->character->cur_spirit <= 0)
		Kots_CharacterFlashlightOff(ent);
	if (ent->character->using_empathy && ent->character->cur_spirit < 3)
		Kots_CharacterEmpathyOff(ent);
	//no need to toggle it off because they won't convert anymore if they don't have enough spirit
	//if (ent->client && ent->character->using_pconvert && ent->character->cur_spirit < 5)
		//Kots_CharacterConvertCellsToggle(ent, "off");
	if (ent->client && ent->client->pers.kots_persist.using_lasersight && ent->character->cur_technical < 7)
		Kots_CharacterToggleLaser(ent, "off");
	if (ent->client && ent->client->pers.kots_persist.using_laserball && ent->character->cur_technical < 7)
		Kots_CharacterToggleLaserBall(ent, "off");
	if (ent->character->is_flying && ent->character->cur_wisdom < 3)
		ent->character->is_flying = false;
	if (ent->character->is_cloaked && ent->character->cur_wisdom < 5)
		Kots_CharacterUncloak(ent);
}

void Kots_CharacterEndServerFrame(edict_t *ent)
{
	if (!ent->character || !ent->character->is_loggedin)
		return;

	if (!ent->client)
		UpdateAllChaseCams(ent);
	
	//monsters get client effects set here
	if (ent->svflags & SVF_MONSTER)
	{
		Kots_CharacterClientEffects(ent);

		if (ent->character->next_candamage > level.time)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_RED;
		}
	}

	if (ent->health <= 0)
		return;

	//silence footsteps and flying
	if (ent->character->cur_dexterity >= 2)
	{
		if (ent->s.event == EV_FOOTSTEP)
			ent->s.event = EV_NONE;

		/* don't remember why I had this here
		if (!ent->client || ent->s.sound != ent->client->weapon_sound)
			ent->s.sound = 0;
		*/
	}
	else if (ent->character->is_flying)
		ent->s.sound = gi.soundindex ("weapons/rockfly.wav");

	//check for empathy shield
	//ensure we don't add the power screen effect on cloaked players (even though I don't know how it could show up)
	if (ent->character->using_empathy && !(ent->svflags & SVF_NOCLIENT))
		ent->s.effects |= EF_POWERSCREEN;

	//check for emp shockwave hit 
	//if (ent->character->next_empdone > level.time)
		//ent->s.effects |= EF_GRENADE;
	//else
		//ent->s.effects &= ~EF_GRENADE;

	if (level.time >= ent->character->next_unflashlight)
	{
		//reset some stuff for the next game loop
		ent->character->is_flashlighted = KOTS_FLASHLIGHT_NO;
	}
}

int Kots_CharacterGetMaxCubes(edict_t *ent)
{
	return (KOTS_BASE_MAX_CUBES * Kots_CharacterGetMaxAmmoBonus(ent));
}

int Kots_CharacterGetRespawnCubes(edict_t *ent)
{
	if (ent->character->cur_dexterity < 7)
		return 1 + ent->character->cur_dexterity * 2;
	else
		return 1 + 10 + ent->character->cur_dexterity * 2;
}

float Kots_CharacterGetPoisonReduction(edict_t *ent)
{
	return ent->character->cur_dexterity * 0.08;
}

void Kots_CharacterPickupCubes(edict_t *ent, int cubes)
{
	if (cubes != 0)
	{
		int max = Kots_CharacterGetMaxCubes(ent);

		if (ent->character->cubes < max)
		{
			ent->character->cubes += cubes;

			if (ent->character->cubes > max)
				ent->character->cubes = max;
		}
	}
}

int Kots_CharacterGetArmor(edict_t *ent)
{
	if (ent->client)
		return ent->client->pers.inventory[ARMOR_INDEX];
	else
		return 0;
}

void Kots_CharacterPerformDamageCalculations(edict_t *targ, edict_t *attacker, int *take, int *asave, int mod, vec3_t dir, vec3_t point, vec3_t normal, int te_sparks, int dflags)
{
	int total, realtotal;
	qboolean friendly_fire = (mod & MOD_FRIENDLY_FIRE ? true : false);
	mod = (mod & ~MOD_FRIENDLY_FIRE);

	//no need to do anything for dead things or non-characters
	if (targ->health <= 0 || !targ->character || targ->deadflag)
	{
		*asave = 0;
		return;
	}

	//we want to damage ourselves and take damage from the environment no matter what
	if (targ != attacker && attacker->character)
	{
		//don't take damage in monsters only
		//or if a spree war is going and neither are the warrer
		//or if we're just starting
		if ((KOTS_MONSTER_VOTE == KOTS_MONSTERS_ONLY && targ->client && attacker->client)
			|| (spreewar.warent && spreewar.warent != targ && spreewar.warent != attacker))
		{

			if (!(dflags & DAMAGE_NO_PROTECTION))
			{
				//play invincibility sound
				if (targ->pain_debounce_time < level.time)
				{
					gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);
					targ->pain_debounce_time = level.time + 2;
				}

				//take no damage
				*asave = 0;
				*take = 0;
				return;
			}
		}
	}
	
	// check for invuln
	if (((targ->client && targ->client->invincible_framenum > level.framenum) && !(dflags & DAMAGE_NO_PROTECTION))
		|| (targ->character->next_candamage > level.time))
	{

		if (!(dflags & DAMAGE_NO_PROTECTION))
		{
			if (targ->pain_debounce_time < level.time)
			{
				gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
				targ->pain_debounce_time = level.time + 2;
			}

			//save 75% of the damage
			*asave = Kots_RandMultiply(*take, 0.75);
			*take -= *asave;
		}
	}

	//add monster attack bonuses only against non-monsters
	if (!attacker->client && !(targ->svflags & SVF_MONSTER))
	{
		//don't give damage bonus on boss explosions
		if (mod != MOD_BOSSEXPLODE)
			Kots_MonsterAddDamageBonus(attacker, take);
	}

	if (targ->character && mod == MOD_RAILGUN)
	{
		//emp shockwave, ignores deflect
		if (dflags & DAMAGE_EMP && targ->character->cur_strength < 10)
		{
			//set the time when emp expires
			if (targ->character->cur_strength >= 7)
			{
				targ->character->next_empdone = level.time + 3.0;
			}
			else
			{
				targ->character->next_empdone = level.time + 6.0;
			}

			//set new regen times so we don't regen for a while
			targ->character->next_regen = targ->character->next_empdone;
			targ->character->next_fast_regen = targ->character->next_empdone;

			//disable abilities
			Kots_CharacterEmpathyOff(targ);
			Kots_CharacterUncloak(targ);
			Kots_CharacterLand(targ);

			//reactivate their current weapon causing them to stop firing
			if (targ->client)
			{
				targ->client->newweapon = targ->client->pers.weapon;
				ChangeWeapon(targ);
			}

			gi.sound(targ, CHAN_BODY, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
		}
		else if (Kots_CharacterCheckRailDeflect(targ, attacker, dir, *take))
		{
			*asave = 0;
			*take = 0;
			return;
		}
	}

	if (dflags & DAMAGE_STUN && targ->character->cur_strength < 10)
	{
		if (targ->character->cur_strength < 7) // Only cause stun 50% of the time
			targ->character->next_stundone = level.time + (Kots_RandRound(0.5) * 0.1);

		VectorClear(targ->velocity);
	}

	if (dflags & DAMAGE_DIZZY && targ->character->cur_strength < 10)
	{
		// Reduced dizzyness from 5 seconds to 3 seconds and 1.5 seconds for strength masters
		if (targ->character->cur_strength >= 7)
			targ->character->next_dizzydone = level.time + 1.5;
		else
			targ->character->next_dizzydone = level.time + 3.0;
	}

	//ensure that the damage dealt and received updates the spree timers
	//this is done regardless of rail deflect
	if (targ != attacker && targ->character && attacker->character)
	{
		if (targ->character->spreetimer)
			targ->character->spreetimer = level.time + KOTS_SPREE_TIMER;

		if (attacker->character->spreetimer)
			attacker->character->spreetimer = level.time + KOTS_SPREE_TIMER;
		
		targ->character->last_damaged = level.time;
		targ->character->last_damagedby = attacker;

		// spree war dmg
		if(spreewar.warent && attacker->client && spreewar.warent == targ)
			spreewar.dmgs[attacker - g_edicts - 1] += *take;
	}

	//take resistances into account before anything else
	if (targ->character && !(dflags & DAMAGE_NO_RESIST))
		Kots_CharacterCheckResistances(targ, attacker, take, mod, dflags);

	//check damage that goes to armor
	total = *take;
	*asave = CheckArmor(targ, point, normal, *take, te_sparks, dflags);
	*take -= *asave;

	//calculate the real amount of damage taken (not counting damage past 0 health)
	if (total > targ->health)
		realtotal = targ->health + *asave;
	else
		realtotal = total;

	//damage type all armor only does have damage to health after taking into account armor
	if (dflags & DAMAGE_ALL_ARMOR)
		*take *= 0.75;

	//check damage based skills
	if (!(dflags & DAMAGE_NO_BONUS) && targ->character && attacker->character && targ != attacker)
	{
		//don't do damage based stuff on dead stuff or friendly fire
		if (targ->health > 0 && !friendly_fire)
			Kots_CharacterDoDamageBasedSkills(targ, attacker, *take, total, realtotal);
	}

	//calculate reductions
	if (targ->health > 0 && *take > 0)
		Kots_CharacterCheckReductions(targ, take, mod);

	//require a minimum of 1 damage despite all resistances, unless it's of type all armor damage
	if (*take <= 0 && attacker->character && !(dflags & DAMAGE_ALL_ARMOR))
		*take = 1;

	//add damage based experience
	if (targ->character && attacker->character)
		Kots_CharacterAddDamageExp(attacker, targ, *take);

	//don't count damage towards ourselves
	if (targ != attacker && attacker->character)
	{
		//if we did more damage than the targets health only count up the health and armor
		if (*take > targ->health)
		{
			targ->character->level_taken += targ->health + *asave;
			attacker->character->level_dealt += targ->health + *asave;
		}
		else
		{
			targ->character->level_taken += *take + *asave;
			attacker->character->level_dealt += *take + *asave;
		}
	}
}

//Copied from g_combat.c
static int CheckArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
	gclient_t	*client;
	int			save;
	int			index;
	gitem_t		*armor;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	index = ArmorIndex (ent);
	if (!index)
		return 0;

	armor = GetItemByIndex (index);

	if (dflags & DAMAGE_ENERGY)
		save = ceil(((gitem_armor_t *)armor->info)->energy_protection*damage);
	else if (dflags & DAMAGE_ALL_ARMOR)
		save = damage;
	else
		save = ceil(((gitem_armor_t *)armor->info)->normal_protection*damage);
	
	//if this is not an all armor type attack
	if (!(dflags & DAMAGE_ALL_ARMOR))
		Kots_CharacterCheckArmor(ent, &save, &client->pers.inventory[index]);
	else
	{
		if (save > client->pers.inventory[index])
			save = client->pers.inventory[index];

		//remove armor directly for all armor attacks
		client->pers.inventory[index] -= save;
	}

	if (!save)
		return 0;

	SpawnDamage (te_sparks, point, normal, save);
	return save;
}

void Kots_CharacterBuySpawns(edict_t *ent, int buy)
{
	if (buy < 1 || buy > 500)
		gi.cprintf(ent, PRINT_HIGH, "The number of spawns must be between 1 and 500.\n");
	else if (ent->character->respawns >= 500)
		gi.cprintf(ent, PRINT_HIGH, "You already have the maximum number of spawns.\n");
	else
	{
		int cost = Kots_CharacterGetSpawnCost(buy);
		if (ent->character->credits < cost)
			gi.cprintf(ent, PRINT_HIGH, "You do not have enough credits.\nCredit Cost: %i\n", cost);
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "You have purchased %u respawns.\n", buy);
			ent->character->respawns += buy;
			ent->character->credits -= cost;
		}
	}
}

void Kots_CharacterWriteStats(edict_t *ent)
{
	if (!ent->client->pers.kots_persist.is_loggedin)
	{
		ent->client->ps.stats[STAT_KOTS_SCORE] = 0;
		ent->client->ps.stats[STAT_KOTS_SPREE] = 0;
		ent->client->ps.stats[STAT_KOTS_CUBES] = 0;
		ent->client->ps.stats[STAT_KOTS_EXPACK] = 0;
		ent->client->ps.stats[STAT_KOTS_THROW] = 0;
		ent->client->ps.stats[STAT_KOTS_SPIRAL] = 0;
		ent->client->ps.stats[STAT_KOTS_BIDE] = 0;
		ent->client->ps.stats[STAT_KOTS_HASEXPACK] = 0;
		ent->client->ps.stats[STAT_KOTS_HASTHROW] = 0;
		ent->client->ps.stats[STAT_KOTS_HASSPIRAL] = 0;
		ent->client->ps.stats[STAT_KOTS_HASBIDE] = 0;
		ent->client->ps.stats[STAT_KOTS_EMPATHY] = 0;
		ent->client->ps.stats[STAT_KOTS_KARMAID] = 0;
		ent->client->ps.stats[STAT_KOTS_RUNE] = 0;
		ent->client->ps.stats[STAT_CHASE] = 0;
		ent->client->ps.stats[STAT_KOTS_TBALLS] = 0;
		return;
	}


	//set the stats
	ent->client->ps.stats[STAT_KOTS_SCORE] = ent->character->score;
	ent->client->ps.stats[STAT_KOTS_SPREE] = ent->character->spree;
	ent->client->ps.stats[STAT_KOTS_CUBES] = ent->character->cubes;
	ent->client->ps.stats[STAT_KOTS_EXPACK] = (ent->character->expacksleft >= 0 ? ent->character->expacksleft : 0);
	ent->client->ps.stats[STAT_KOTS_THROW] = (ent->character->throwsleft >= 0 ? ent->character->throwsleft : 0);
	ent->client->ps.stats[STAT_KOTS_SPIRAL] = (ent->character->spiralsleft >= 0 ? ent->character->spiralsleft : 0);
	ent->client->ps.stats[STAT_KOTS_BIDE] = (ent->character->bidesleft >= 0 ? ent->character->bidesleft : 0);
	ent->client->ps.stats[STAT_KOTS_HASEXPACK] = ent->character->cur_expack;
	ent->client->ps.stats[STAT_KOTS_HASTHROW] = ent->character->cur_kotsthrow;
	ent->client->ps.stats[STAT_KOTS_HASSPIRAL] = ent->character->cur_spiral;
	ent->client->ps.stats[STAT_KOTS_HASBIDE] = ent->character->cur_bide;
	ent->client->ps.stats[STAT_KOTS_EMPATHY] = ent->character->using_empathy;
	ent->client->ps.stats[STAT_KOTS_TBALLS] = (ent->character->tballs > 0 ? ent->character->tballs : 0);

	//update karma id stats
	Kots_CharacterUpdateKarmaIdStats(ent);

	if (!ent->character->rune)
		ent->client->ps.stats[STAT_KOTS_RUNE] = 0;
	else
	{
		int index = (CS_GENERAL + (ent - g_edicts) - 1 + MAX_CLIENTS);
		ent->client->ps.stats[STAT_KOTS_RUNE] = index;
		gi.configstring(index, ent->character->rune->name);
	}
}

void Kots_CharacterUpdateKarmaIdStats(edict_t *ent)
{
	if (!ent->character->karma_id_ent)
		ent->client->ps.stats[STAT_KOTS_KARMAID] = 0;
	else
	{
		int index = (CS_GENERAL + (ent - g_edicts) - 1);
		ent->client->ps.stats[STAT_KOTS_KARMAID] = index;

		//if it's time to update and the edict hasn't been freed
		if (level.time >= ent->character->next_karma_hud && ent->character->karma_id_ent->inuse)
		{
			char message[MAX_QPATH];

			ent->character->next_karma_hud = level.time + 0.1;
			
			//create the karma message
			if (ent->character->karma_id_ent->character && ent->character->karma_id_ent->character->is_loggedin)
			{
				Kots_snprintf(message, MAX_QPATH * 2, "[%s Level %i] (%i/%i)",
					Kots_CharacterGetName(ent->character->karma_id_ent),
					ent->character->karma_id_ent->character->level,
					ent->character->karma_id_ent->health,
					Kots_CharacterGetArmor(ent->character->karma_id_ent));
			}
			else
			{
				//don't know how to handle this to id it
				ent->client->ps.stats[STAT_KOTS_KARMAID] = 0;
				return;
			}

			//we need to multicast this out so that spectators get it
			gi.configstring(index, message);
			gi.WriteChar(13 /*svc_configstring*/);
			gi.WriteShort(index);
			gi.WriteString(message);
			gi.multicast(ent->s.origin, MULTICAST_PVS_R);
		}
	}
}

void Kots_CharacterClientBegin(edict_t *ent)
{
	//Stuff all the kots commands to the user
	Kots_StuffCommands(ent);

	//SWB
	//ensure that if we're not already logged in, but we should be then do so now
	if (!ent->character->is_loggedin && ent->client->pers.kots_persist.is_loggedin)
	{
		//clear connection persistant data
		Kots_ClearConPersistData(ent);

		//regardless of success or fail mark the user as not logged in
		//so we don't mess up anything that relies on it
		ent->client->pers.kots_persist.is_loggedin = false;

		//reset the persistant value 
		Kots_ServerLoginCharacter(ent);
	}
	else //no need to show the menu if we're logging in
	{
		//Show the Kots menu when someone connects
		Kots_MenuShow(ent, NULL);
	}
}

void Kots_CharacterCheckCubeRot(edict_t *ent)
{
	int max = Kots_CharacterGetMaxCubes(ent);
	int overmax = ent->character->cubes - max;

	if (overmax <= 0)
		return;
	
	//rot depending on how far over max we are
	if (overmax <= 50)
		ent->character->cubes -= 1;
	else if (overmax <= 100)
		ent->character->cubes -= 2;
	else
		ent->character->cubes -= 3;

	//make sure we don't rot too many cubes
	if (ent->character->cubes < max)
		ent->character->cubes = max;
		
}

void Kots_CharacterCheckArmorRot(edict_t *ent)
{
	if (ent->character->cur_technical < 7)
	{
		if (ent->client)
		{
			int index = ARMOR_INDEX;
			int max = Kots_CharacterGetMaxArmor(ent);
			int overmax = ent->client->pers.inventory[index] - max;

			if (overmax <= 0)
				return;
			
			//rot depending on how far over max we are
			if (overmax <= 50)
				ent->client->pers.inventory[index] -= 1;
			else if (overmax <= 100)
				ent->client->pers.inventory[index] -= 2;
			else
				ent->client->pers.inventory[index] -= 3;

			//make sure we don't rot too much armor
			if (ent->client->pers.inventory[index] < max)
				ent->client->pers.inventory[index] = max;
		}
	}
}

void Kots_CharacterCheckHealthRot(edict_t *ent)
{
	if (ent->character->cur_karma < 7)
	{
		int max = Kots_CharacterGetMaxRegenHealth(ent);
		int overmax = ent->health - max;

		if (overmax <= 0)
			return;
		
		//rot depending on how far over max we are
		if (overmax <= 50)
			ent->health -= 1;
		else if (overmax <= 100)
			ent->health -= 2;
		else
			ent->health -= 3;

		//make sure we don't rot too much health
		if (ent->health < max)
			ent->health = max;
	}
}

int Kots_CharacterGetTotalPlayerPoints(edict_t *ent)
{
	return ent->character->playerpoints + ent->character->dexterity
		+ ent->character->strength + ent->character->karma
		+ ent->character->wisdom + ent->character->technical
		+ ent->character->spirit + ent->character->rage
		+ ent->character->vitarmor + ent->character->vithealth
		+ ent->character->munition;
}

int Kots_CharacterGetPurchasedPlayerPoints(edict_t *ent)
{
	return ent->character->playersbought;
}

int Kots_CharacterGetNextPlayerPointCost(edict_t *ent)
{
	return (8000 + Kots_CharacterGetPurchasedPlayerPoints(ent) * 1000);
}

int Kots_CharacterGetTotalWeaponPoints(edict_t *ent)
{
	return ent->character->weaponpoints + ent->character->sabre
		+ ent->character->shotgun + ent->character->machinegun
		+ ent->character->chaingun + ent->character->supershotgun
		+ ent->character->grenade + ent->character->grenadelauncher
		+ ent->character->rocketlauncher + ent->character->hyperblaster
		+ ent->character->railgun + ent->character->bfg + ent->character->wantiweapon;
}

int Kots_CharacterGetPurchasedWeaponPoints(edict_t *ent)
{
	return ent->character->weaponsbought;
}

int Kots_CharacterGetNextWeaponPointCost(edict_t *ent)
{
	return (4000 + Kots_CharacterGetPurchasedWeaponPoints(ent) * 500);
}

int Kots_CharacterGetTotalPowerPoints(edict_t *ent)
{
	return ent->character->powerpoints + ent->character->expack
		+ ent->character->spiral + ent->character->bide
		+ ent->character->kotsthrow + ent->character->antiweapon;
}

int Kots_CharacterGetPurchasedPowerPoints(edict_t *ent)
{
	return ent->character->powersbought;
}

int Kots_CharacterGetNextPowerPointCost(edict_t *ent)
{
	return (12000 + Kots_CharacterGetPurchasedPowerPoints(ent) * 1500);
}

int Kots_CharacterGetSpawnCost(int spawns)
{
	if (spawns < 200)
		return spawns;
	else
		return (int)ceil(spawns * 0.75);

}

void Kots_CharacterBuyPlayerPoint(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPlayerPointCost(ent);
	if (ent->character->credits < cost)
		gi.cprintf(ent, PRINT_HIGH, "You do not have enough credits.\nCredit Cost: %i\n", cost);
	else
	{
		++ent->character->playerpoints;
		++ent->character->playersbought;
		ent->character->credits -= cost;

		gi.cprintf(ent, PRINT_HIGH, "You have purchased a player point.\n");
	}
}

void Kots_CharacterBuyWeaponPoint(edict_t *ent)
{
	int cost = Kots_CharacterGetNextWeaponPointCost(ent);
	if (ent->character->credits < cost)
		gi.cprintf(ent, PRINT_HIGH, "You do not have enough credits.\nCredit Cost: %i\n", cost);
	else
	{
		++ent->character->weaponsbought;
		++ent->character->weaponpoints;
		ent->character->credits -= cost;
		gi.cprintf(ent, PRINT_HIGH, "You have purchased a weapon point.\n");
	}
}

void Kots_CharacterBuyPowerPoint(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPowerPointCost(ent);
	if (ent->character->credits < cost)
		gi.cprintf(ent, PRINT_HIGH, "You do not have enough credits.\nCredit Cost: %i\n", cost);
	else
	{
		++ent->character->powersbought;
		++ent->character->powerpoints;
		ent->character->credits -= cost;
		gi.cprintf(ent, PRINT_HIGH, "You have purchased a power point.\n");
	}
}

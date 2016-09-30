#include "kots_player.h"
#include "kots_runes.h"

//Player point names and functions to add them
#define TotalPlayerPoints 17
playerpoint_t PlayerPoints[TotalPlayerPoints] =
{
	"dexterity", KOTS_PLAYER_DEXTERITY, Kots_CharacterAddDexterity,
	"dex", KOTS_PLAYER_DEXTERITY, Kots_CharacterAddDexterity,
	"strength", KOTS_PLAYER_STRENGTH, Kots_CharacterAddStrength,
	"str", KOTS_PLAYER_STRENGTH, Kots_CharacterAddStrength,
	"karma", KOTS_PLAYER_KARMA, Kots_CharacterAddKarma,
	"wisdom", KOTS_PLAYER_WISDOM, Kots_CharacterAddWisdom,
	"wis", KOTS_PLAYER_WISDOM, Kots_CharacterAddWisdom,
	"technical", KOTS_PLAYER_TECHNICAL, Kots_CharacterAddTechnical,
	"tech", KOTS_PLAYER_TECHNICAL, Kots_CharacterAddTechnical,
	"spirit", KOTS_PLAYER_SPIRIT, Kots_CharacterAddSpirit,
	"rage", KOTS_PLAYER_RAGE, Kots_CharacterAddRage,
	"vith", KOTS_PLAYER_VITH, Kots_CharacterAddVitHealth,
	"vithealth", KOTS_PLAYER_VITH, Kots_CharacterAddVitHealth,
	"vita", KOTS_PLAYER_VITA, Kots_CharacterAddVitArmor,
	"vitarmor", KOTS_PLAYER_VITA, Kots_CharacterAddVitArmor,
	"munition", KOTS_PLAYER_MUNITION, Kots_CharacterAddMunition,
	"mun", KOTS_PLAYER_MUNITION, Kots_CharacterAddMunition
};

#define Kots_CharacterAddStat(stat, val, max)	{stat += val; if (stat > max) stat = max;}
#define Kots_CharacterSetStat(oldval, curval, newval, max)	{ curval += newval - oldval; oldval = newval; if (curval > max) curval = max; }

playerpoint_t *Kots_GetPlayerTree(char *name)
{
	int i;

	for (i = 0; i < TotalPlayerPoints; i++)
	{
		if (Q_stricmp(name, PlayerPoints[i].name) == 0)
			return &PlayerPoints[i];
	}

	return NULL;
}

qboolean Kots_CharacterCanAddPlayer(edict_t *ent, int level, int max, int cost)
{
	if (max > 0 && level >= max)
	{
		gi.cprintf(ent, PRINT_HIGH, "You have already reached the maximum level.\n");
		return false;
	}
	else if (cost == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "You have mastered the maximum number of trees.\n");
		return false;
	}
	if (ent->character->playerpoints >= cost)
		return true;
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough player points available.\n");
		return false;
	}
}

int *Kots_CharacterGetPlayerLevel(edict_t *ent, int player)
{
	switch (player)
	{
	case KOTS_PLAYER_DEXTERITY:
		return &ent->character->dexterity;
	case KOTS_PLAYER_STRENGTH:
		return &ent->character->strength;
	case KOTS_PLAYER_KARMA:
		return &ent->character->karma;
	case KOTS_PLAYER_WISDOM:
		return &ent->character->wisdom;
	case KOTS_PLAYER_SPIRIT:
		return &ent->character->spirit;
	case KOTS_PLAYER_TECHNICAL:
		return &ent->character->technical;
	case KOTS_PLAYER_RAGE:
		return &ent->character->rage;
	case KOTS_PLAYER_MUNITION:
		return &ent->character->munition;
	case KOTS_PLAYER_VITA:
		return &ent->character->vitarmor;
	case KOTS_PLAYER_VITH:
		return &ent->character->vithealth;
	case KOTS_PLAYER_AVAILABLE:
	default:
		return &ent->character->playerpoints;
	}
}

int Kots_GetMaxPlayerLevel(int player)
{
	switch (player)
	{
	case KOTS_PLAYER_MUNITION:
	case KOTS_PLAYER_VITA:
	case KOTS_PLAYER_VITH:
		return 0;
	default:
		return 7;
	}
}

int Kots_CharacterGetNextPlayerLevelCost(edict_t *ent, int player, int level)
{
	int total;
	int mastered;

	switch (player)
	{
	case KOTS_PLAYER_MUNITION:
	case KOTS_PLAYER_VITA:
	case KOTS_PLAYER_VITH:
		return 1;
	default:

		//everything is 1 point until level 5
		if (level < 5)
			return 1;

		//determine how many skills are mastered or partially mastered
		mastered = 0;
		total = 0;
		if (ent->character->dexterity > 5)
		{
			mastered++;
			total += ent->character->dexterity - 5;
		}
		if (ent->character->strength > 5)
		{
			mastered++;
			total += ent->character->strength - 5;
		}
		if (ent->character->karma > 5)
		{
			mastered++;
			total += ent->character->karma - 5;
		}
		if (ent->character->wisdom > 5)
		{
			mastered++;
			total += ent->character->wisdom - 5;
		}
		if (ent->character->technical > 5)
		{
			mastered++;
			total += ent->character->technical - 5;
		}
		if (ent->character->spirit > 5)
		{
			mastered++;
			total += ent->character->spirit - 5;
		}
		if (ent->character->rage > 5)
		{
			mastered++;
			total += ent->character->rage - 5;
		}

		//we can only master a max of 3 trees
		//if the current level is 5 that means
		//we're trying to master something new
		if (mastered >= 3 && level == 5)
			return 0;

		//first level 6 and 7 (or 2 level 6's) are 1 point
		else if (total < 2)
			return 1;

		//next level 6 and 7 (or multiple 6's) are 2 points
		else
			return (total - 1) * 2;
	}
}

//Adding player points
void Kots_CharacterAddDexterity(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPlayerLevelCost(ent, KOTS_PLAYER_DEXTERITY, ent->character->dexterity);
	if (Kots_CharacterCanAddPlayer(ent, ent->character->dexterity, 7, cost))
	{
		ent->character->dexterity++;
		Kots_CharacterAddStat(ent->character->cur_dexterity, 1, 10);
		ent->character->playerpoints -= cost;
		gi.cprintf(ent, PRINT_HIGH, "Dexterity is now %i.\n", ent->character->dexterity);
	}
}
void Kots_CharacterAddStrength(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPlayerLevelCost(ent, KOTS_PLAYER_STRENGTH, ent->character->strength);
	if (Kots_CharacterCanAddPlayer(ent, ent->character->strength, 7, cost))
	{
		ent->character->strength++;
		Kots_CharacterAddStat(ent->character->cur_strength, 1, 10);
		ent->character->playerpoints -= cost;
		gi.cprintf(ent, PRINT_HIGH, "Strength is now %i.\n", ent->character->strength);
	}
}
void Kots_CharacterAddKarma(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPlayerLevelCost(ent, KOTS_PLAYER_KARMA, ent->character->karma);
	if (Kots_CharacterCanAddPlayer(ent, ent->character->karma, 7, cost))
	{
		ent->character->karma++;
		Kots_CharacterAddStat(ent->character->cur_karma, 1, 10);
		ent->character->playerpoints -= cost;
		gi.cprintf(ent, PRINT_HIGH, "Karma is now %i.\n", ent->character->karma);
	}
}
void Kots_CharacterAddWisdom(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPlayerLevelCost(ent, KOTS_PLAYER_WISDOM, ent->character->wisdom);
	if (Kots_CharacterCanAddPlayer(ent, ent->character->wisdom, 7, cost))
	{
		ent->character->wisdom++;
		Kots_CharacterAddStat(ent->character->cur_wisdom, 1, 10);
		ent->character->playerpoints -= cost;
		gi.cprintf(ent, PRINT_HIGH, "Wisdom is now %i.\n", ent->character->wisdom);
	}
}
void Kots_CharacterAddTechnical(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPlayerLevelCost(ent, KOTS_PLAYER_TECHNICAL, ent->character->technical);
	if (Kots_CharacterCanAddPlayer(ent, ent->character->technical, 7, cost))
	{
		ent->character->technical++;
		Kots_CharacterAddStat(ent->character->cur_technical, 1, 10);
		ent->character->playerpoints -= cost;
		gi.cprintf(ent, PRINT_HIGH, "Technical is now %i.\n", ent->character->technical);
	}
}
void Kots_CharacterAddSpirit(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPlayerLevelCost(ent, KOTS_PLAYER_SPIRIT, ent->character->spirit);
	if (Kots_CharacterCanAddPlayer(ent, ent->character->spirit, 7, cost))
	{
		ent->character->spirit++;
		Kots_CharacterAddStat(ent->character->cur_spirit, 1, 10);
		ent->character->playerpoints -= cost;
		gi.cprintf(ent, PRINT_HIGH, "Spirit is now %i.\n", ent->character->spirit);
	}
}
void Kots_CharacterAddRage(edict_t *ent)
{
	int cost = Kots_CharacterGetNextPlayerLevelCost(ent, KOTS_PLAYER_RAGE, ent->character->rage);
	if (Kots_CharacterCanAddPlayer(ent, ent->character->rage, 7, cost))
	{
		ent->character->rage++;
		Kots_CharacterAddStat(ent->character->cur_rage, 1, 10);
		ent->character->playerpoints -= cost;
		gi.cprintf(ent, PRINT_HIGH, "Rage is now %i.\n", ent->character->rage);
	}
}
void Kots_CharacterAddVitHealth(edict_t *ent)
{
	if (Kots_CharacterCanAddPlayer(ent, 0, 0, 1) &&
		Kots_CharacterCheckLevelCap(ent, KOTS_PLAYER_VITH, ent->character->vithealth + 1))
	{
		ent->character->vithealth++;
		ent->character->cur_vithealth++;
		ent->character->playerpoints--;
		ent->max_health = Kots_CharacterGetStartHealth(ent);
		gi.cprintf(ent, PRINT_HIGH, "VitHealth is now %i.\n", ent->character->vithealth);
	}
}
void Kots_CharacterAddVitArmor(edict_t *ent)
{
	if (Kots_CharacterCanAddPlayer(ent, 0, 0, 1) &&
		Kots_CharacterCheckLevelCap(ent, KOTS_PLAYER_VITA, ent->character->vitarmor + 1))
	{
		ent->character->vitarmor++;
		ent->character->cur_vitarmor++;
		ent->character->playerpoints--;
		gi.cprintf(ent, PRINT_HIGH, "VitArmor is now %i.\n", ent->character->vitarmor);
	}
}
void Kots_CharacterAddMunition(edict_t *ent)
{
	if (Kots_CharacterCanAddPlayer(ent, 0, 0, 1) &&
		Kots_CharacterCheckLevelCap(ent, KOTS_PLAYER_MUNITION, ent->character->munition + 1))
	{
		ent->character->munition++;
		ent->character->cur_munition++;
		ent->character->playerpoints--;
		gi.cprintf(ent, PRINT_HIGH, "Munition is now %i.\n", ent->character->munition);
	}
}

void Kots_CharacterAddPlayer(edict_t *ent, char *player)
{
	if (!player || player[0] == '\0')
		gi.cprintf(ent, PRINT_HIGH, "Usage: %s PlayerTree\n", gi.argv(0));
	else
	{
		playerpoint_t *player_tree = Kots_GetPlayerTree(player);

		if (!player_tree)
			gi.cprintf(ent, PRINT_HIGH, "%s is not a valid player ability.\n", player);
		else
			player_tree->add(ent);
	}
}

void Kots_CharacterRespecPlayer(edict_t *ent, char *player)
{
	// Make sure they entered a player tree, if not print the usage and the points they currently have
	if (!player || player[0] == '\0')
	{
		gi.cprintf(ent, PRINT_HIGH, "You currently have %d respec points\n", ent->character->respec_points);
		gi.cprintf(ent, PRINT_HIGH, "Usage: %s PlayerTree [count]\n", gi.argv(0));
	}
	else if (ent->character->respec_points <= 0) // Make sure we have points to respec with first
		gi.cprintf(ent, PRINT_HIGH, "You do not have any points available to respec.\n", gi.argv(0));
	else
	{
		// Make sure the player tree is valid
		playerpoint_t *player_tree = Kots_GetPlayerTree(player);
		if (!player_tree)
			gi.cprintf(ent, PRINT_HIGH, "%s is not a valid player ability.\n", player);
		else
		{
			// Make sure there are points in the player tree
			int *value = Kots_CharacterGetPlayerLevel(ent, player_tree->player);
			if (*value == 0)
			{
				gi.cprintf(ent, PRINT_HIGH, "You do not have any points in %s to respec.\n", player);
			}
			else
			{
				int count = 1;

				// Check to see if they entered the optional number of points to respec
				if (gi.argc() >= 3)
				{
					count = atoi(gi.argv(2));
					if (count <= 0)
					{
						gi.cprintf(ent, PRINT_HIGH, "The number you entered to respec (%s) is not valid.\n", gi.argv(2));
						return;
					}
				}

				// Make sure the user didn't try to use to many points
				if (count > ent->character->respec_points)
				{
					gi.cprintf(ent, PRINT_HIGH, "You only have %d points you can use to respec with.\n", ent->character->respec_points);
				}
				else if (count > *value) // Ensure we have enough points to reset
				{
					// Not enough points to reset
					gi.cprintf(ent, PRINT_HIGH, "You only have %d points in %s to respec.\n", *value, player);
				}
				else
				{
					(*value) -= count;
					ent->character->respec_points -= count;
					ent->character->playerpoints += count;
					gi.cprintf(ent, PRINT_HIGH, "You now have %d points in %s and %d points available and %d respec points.\n",
						*value, player, ent->character->playerpoints, ent->character->respec_points);
				}
			}
		}
	}
}

void Kots_CharacterSetPlayer(edict_t *ent)
{
	if (gi.argc() != 4)
		gi.cprintf(ent, PRINT_HIGH, "Usage: %s PlayerName PlayerTree Level\n", gi.argv(0));
	else
	{
		int i;
		edict_t *other = g_edicts + 1;
		char *name = gi.argv(1);
		char *player = gi.argv(2);
		int level = atoi(gi.argv(3));

		for (i = 0; i < game.maxclients; i++, other++)
		{
			if (!other->inuse || !other->character || !other->character->is_loggedin)
				continue;

			if (Q_stricmp(other->client->pers.netname, name) == 0)
			{
				playerpoint_t *player_tree = Kots_GetPlayerTree(player);
				
				if (!player_tree)
				{
					gi.cprintf(ent, PRINT_HIGH, "Unrecognized player tree %s.\n", player);
					return;
				}
				else
				{
					int *points = Kots_CharacterGetPlayerLevel(other, player_tree->player);
					int max = Kots_GetMaxPlayerLevel(player_tree->player);

					if (level < 0 || (max != 0 && level > max))
					{
						gi.cprintf(ent, PRINT_HIGH, "Invalid player tree level specified.\n");
						return;
					}
					else
					{
						if (other->character->rune)
							Kots_RuneRemoveAbilities(other, other->character->rune);

						*points = level;
						Kots_CharacterResetStats(other);

						if (other->character->rune)
							Kots_RuneAddAbilities(other, other->character->rune);

						gi.cprintf(other, PRINT_HIGH, "Your %s player tree is now level %i thanks to %s.\n", player_tree->name, *points, ent->character->name);
						gi.cprintf(ent, PRINT_HIGH, "You changed %s's %s level to %i.\n", other->character->name, player_tree->name, *points);
						return;
					}
				}
			}
		}

		gi.cprintf(ent, PRINT_HIGH, "Unable to find a player logged in with that name.\n");
	}
}

qboolean Kots_CharacterCheckLevelCap(edict_t *ent, int player, int total)
{
	// The level cap only applies when you're below level 10
	if (ent->character->level < 10)
	{
		switch (player)
		{
		case KOTS_PLAYER_MUNITION:
		case KOTS_PLAYER_VITA:
		case KOTS_PLAYER_VITH:

			// Only allow 1 point for every 3 levels
			if (total > ((ent->character->level / 3) + 1))
			{
				int next_level = (total - 1) * 3;
				if (next_level > 10)
					next_level = 10;

				gi.cprintf(ent, PRINT_HIGH, "You cannot train this player tree again until level %d.\n", next_level);
				return false;
			}

			break;
		default:
			break;
		}
	}

	return true;
}

#include "kots_hud.h"
#include "kots_array.h"

#define SCOREBOARD_MAX_PLAYERS		12

//Quake 2 functions used
void ExitLevel();

static int intermission_screen = KOTS_INTERMISSION_SCORE;

void Kots_HudShowStats(edict_t *ent, char *args)
{
	//ensure that inventory and help are always hidden
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	//no args wil show/hide the stats
	if (!args || !*args)
	{
		ent->client->showscores = !ent->client->showscores;
		return;
	}

	//don't give any not found errors for help just print the help
	if (Q_stricmp(args, "help") != 0)
	{
		int choice = atoi(args);

		switch (choice - 1)
		{
		case KOTS_INTERMISSION_SCORE:
		case KOTS_INTERMISSION_ACCURACY:
		case KOTS_INTERMISSION_KILLRATIO:
		case KOTS_INTERMISSION_DAMAGE:
		case KOTS_INTERMISSION_EXP:
		case KOTS_INTERMISSION_PACKS:

			//set the intermission screen to show and show the scoreboard
			ent->client->pers.kots_persist.intermission_screen = choice - 1;
			ent->client->showscores = true;
			Kots_HudScoreboard(ent);
			gi.unicast(ent, false);
			return;

		default:
			gi.cprintf(ent, PRINT_HIGH, "Unrecognized stat. Please use valid options below.");
			break;
		}
	}

	//print out help message
	gi.cprintf(ent, PRINT_HIGH, "%i - Show score stats\n", KOTS_INTERMISSION_SCORE + 1);
	gi.cprintf(ent, PRINT_HIGH, "%i - Show accuracy stats\n", KOTS_INTERMISSION_ACCURACY + 1);
	gi.cprintf(ent, PRINT_HIGH, "%i - Show kill ratio stats\n", KOTS_INTERMISSION_KILLRATIO + 1);
	gi.cprintf(ent, PRINT_HIGH, "%i - Show damage stats\n", KOTS_INTERMISSION_DAMAGE + 1);
	gi.cprintf(ent, PRINT_HIGH, "%i - Show exp ratio stats\n", KOTS_INTERMISSION_EXP + 1);
	gi.cprintf(ent, PRINT_HIGH, "%i - Show pack stats\n", KOTS_INTERMISSION_PACKS + 1);
}

int Kots_HudSort(void *a, void *b)
{
	edict_t *ent1 = (edict_t *)a;
	edict_t *ent2 = (edict_t *)b;

	if (ent1->client->pers.kots_persist.is_loggedin && ent2->client->pers.kots_persist.is_loggedin)
	{
		//these are all sorted descending
		switch (intermission_screen)
		{
		case KOTS_INTERMISSION_SCORE:
			if (ent1->character->score > ent2->character->score)
				return -1;
			else if (ent1->character->score < ent2->character->score)
				return 1;
			else
				return 0;

		case KOTS_INTERMISSION_ACCURACY:
			{
			float acc1 = ent1->character->hits / (float)ent1->character->shots;
			float acc2 = ent2->character->hits / (float)ent2->character->shots;

			if (acc1 > acc2)
				return -1;
			else if (acc1 < acc2)
				return 1;
			else
				return 0;
			}

		case KOTS_INTERMISSION_KILLRATIO:
			{
			float ratio1 = 0;
			float ratio2 = 0;

			if (ent1->client->resp.score + ent1->character->level_killed > 0)
				ratio1 = ent1->client->resp.score / (float)(ent1->client->resp.score + ent1->character->level_killed);
			
			if (ent2->client->resp.score + ent2->character->level_killed > 0)
				ratio2 = ent2->client->resp.score / (float)(ent2->client->resp.score + ent2->character->level_killed);

			if (ratio1 > ratio2)
				return -1;
			else if (ratio1 < ratio2)
				return 1;
			else
				return 0;
			}

		case KOTS_INTERMISSION_DAMAGE:
			{
			float ratio1 = 0;
			float ratio2 = 0;

			if (ent1->character->level_dealt + ent1->character->level_taken > 0)
				ratio1 = ent1->character->level_dealt / (float)(ent1->character->level_dealt + ent1->character->level_taken);
			
			if (ent2->character->level_dealt + ent2->character->level_taken > 0)
				ratio2 = ent2->character->level_dealt / (float)(ent2->character->level_dealt + ent2->character->level_taken);

			if (ratio1 > ratio2)
				return -1;
			else if (ratio1 < ratio2)
				return 1;
			else
				return 0;
			}

		case KOTS_INTERMISSION_EXP:
			if (ent1->character->damage_exp > ent2->character->damage_exp)
				return -1;
			else if (ent1->character->damage_exp < ent2->character->damage_exp)
				return 1;
			else
				return 0;

		case KOTS_INTERMISSION_PACKS:
			if (ent1->character->level_credits > ent2->character->level_credits)
				return -1;
			else if (ent1->character->level_credits < ent2->character->level_credits)
				return 1;
			else
				return 0;

		default:
			return 0;
		}
	}
	else if (!ent1->client->pers.kots_persist.is_loggedin && !ent2->client->pers.kots_persist.is_loggedin)
		return Q_stricmp(ent1->client->pers.netname, ent2->client->pers.netname); //sort by names if both logged out
	else if (!ent1->client->pers.kots_persist.is_loggedin)
		return 1; //player 1 should be after player 2
	else
		return -1; //player 2 should be after player 1
}

array_t *Kots_HudGetSortedArray()
{
	int i;
	edict_t *ent;
	array_t *players = Array_Create(NULL, (unsigned long)maxclients->value, 0);
	players->FreeItem = NULL;

	ent = g_edicts + 1;
	for (i = 0 ; i < game.maxclients; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		Array_PushBack(players, ent);
	}

	Array_Sort(players, Kots_HudSort);
	return players;
}


int Kots_HudScoreboardMainTitle(char *entry, int size)
{
	int time_remaining = (int)(timelimit->value - level.time);

	//print the title
	Com_sprintf (entry, size,
		"xv 0 yv 0 string2 \"  KOTS2007              Time: %02i:%02i\" ",
		(time_remaining > 0 ? time_remaining / 60 : 0),
		(time_remaining > 0 ? time_remaining % 60 : 0));

	return strlen(entry);
}
int Kots_HudScoreboardWriteTitle(edict_t *ent, int screen, char *entry, int size)
{
	//create the title layout based on intermission screen
	switch (screen)
	{
	case KOTS_INTERMISSION_ACCURACY:
		Com_sprintf (entry, size,
			"yv 10 string2 \"  Player          Shots  Hits Accuracy\" ");
		break;

	case KOTS_INTERMISSION_KILLRATIO:
		Com_sprintf (entry, size,
			"yv 10 string2 \"  Player          Death  Frag Ratio\" ");
		break;

	case KOTS_INTERMISSION_DAMAGE:
		Com_sprintf (entry, size,
			"yv 10 string2 \"  Player            Dealt    Took Ratio\" ");
		break;

	case KOTS_INTERMISSION_EXP:
		Com_sprintf (entry, size,
			"yv 10 string2 \"  Player           Dmg Exp    Kill Ratio\" ");
		break;

	case KOTS_INTERMISSION_PACKS:
		Com_sprintf (entry, size,
			"yv 10 string2 \"  Player            Packs    Credits  Avg\" ");
		break;
		
	case KOTS_INTERMISSION_SCORE:
	default:
		Com_sprintf (entry, size,
			"yv 10 string2 \"  Player          Score Lvl Frg Png\" ");
		break;
	}

	return strlen(entry);
}

int Kots_HudScoreboardWriteEntry(edict_t *ent, edict_t *current, int screen, char *entry, int size)
{
	static char *blank = " ";
	static char *clicked = "\x0d";
	static char *spec = "s";
	static char *dead = "d";
	char *icon = blank;

	if (current->client->intermission_clicked)
		icon = clicked;
	else if (!current->client->pers.kots_persist.is_loggedin)
		icon = spec;
	else if (current->health <= 0)
		icon = dead;

	//create the entry layout based on intermission screen
	switch (screen)
	{
	case KOTS_INTERMISSION_ACCURACY:
		{
		int hits = 0;
		int shots = 0;
		float ratio = 0.0;

		if (current->client->pers.kots_persist.is_loggedin)
		{
			hits = current->character->hits;
			shots = current->character->shots;

			if (shots > 0)
				ratio = (hits / (float)shots) * 100;
		}

		Com_sprintf (entry, size,
			"%s \"%s %-15s %5i %5i %7.1f%%\" ",
			(current == ent ? "string2" : "string"),
			icon, current->client->pers.netname,
			shots, hits, ratio);
		break;
		}

	case KOTS_INTERMISSION_KILLRATIO:
		{
		int kills = 0;
		int killed = 0;
		float ratio = 0.0;

		if (current->client->pers.kots_persist.is_loggedin)
		{
			kills = current->client->resp.score;
			killed = current->character->level_killed;

			if (kills + killed > 0)
				ratio = (kills / (float)(kills + killed)) * 100;
		}

		Com_sprintf (entry, size,
			"%s \"%s %-15s %5i %5i %4.0f%%\" ",
			(current == ent ? "string2" : "string"),
			icon, current->client->pers.netname,
			killed, kills, ratio);
		break;
		}

	case KOTS_INTERMISSION_DAMAGE:
		{
		int dealt = 0;
		int taken = 0;
		float ratio = 0.0;

		if (current->client->pers.kots_persist.is_loggedin)
		{
			dealt = current->character->level_dealt;
			taken = current->character->level_taken;

			if (dealt + taken > 0)
				ratio = (dealt / (float)(dealt + taken)) * 100;
		}

		Com_sprintf (entry, size,
			"%s \"%s %-15s %7i %7i %4.0f%%\" ",
			(current == ent ? "string2" : "string"),
			icon, current->client->pers.netname,
			dealt, taken, ratio);
		break;
		}

	case KOTS_INTERMISSION_EXP:
		{
		int damage = 0;
		int total = 0;
		float ratio = 0.0;

		if (current->client->pers.kots_persist.is_loggedin)
		{
			damage = current->character->damage_exp;
			total = current->character->score;

			if (current->character->score > 0)
				ratio = (damage / (float)total) * 100;
		}

		Com_sprintf (entry, size,
			"%s \"%s %-15s %8i %7i %4.0f%%\" ",
			(current == ent ? "string2" : "string"),
			icon, current->client->pers.netname,
			damage, total - damage, ratio);
		break;
		}

	case KOTS_INTERMISSION_PACKS:
		{
		int packs = 0;
		int credits = 0;
		float avg = 0.0;

		if (current->client->pers.kots_persist.is_loggedin)
		{
			packs = current->character->level_packs;
			credits = current->character->level_credits;

			if (credits > 0)
				avg = (packs / (float)credits) * 100;
		}

		Com_sprintf (entry, size,
			"%s \"%s %-15s %7i %10i %2.2f\" ",
			(current == ent ? "string2" : "string"),
			icon, current->client->pers.netname,
			packs, credits, avg);
		break;
		}
		
	case KOTS_INTERMISSION_SCORE:
	default:
		Com_sprintf (entry, size,
			"%s \"%s %-15s %5i %3i %3i %3i\" ",
			(current == ent ? "string2" : "string"),
			icon, current->client->pers.netname,
			(current->client->pers.kots_persist.is_loggedin ? current->character->score : 0),
			current->character->level, current->client->resp.score, current->client->ping);
		break;
	}

	return strlen(entry);
}

void Kots_HudScoreboard(edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	unsigned long	i;
	int		j, y, len, size;
	int		screen = ent->client->pers.kots_persist.intermission_screen;
	edict_t	*current;
	array_t *players = Kots_HudGetSortedArray();
	
	//ensure we show correct screen in intermission
	if (level.intermissiontime)
		screen = level.intermission_screen;

	//update the variable used for the sorting function
	intermission_screen = screen;

	//intialize both strings
	string[0] = entry[0] = '\0';

	size = sizeof(entry);
	len = Kots_HudScoreboardMainTitle(string, size);
	len += Kots_HudScoreboardWriteTitle(ent, screen, string + len, size - len);

	y = 14;
	for (i = 0; i < players->length && i < SCOREBOARD_MAX_PLAYERS; i++)
	{
		current = Array_GetValueAt(players, i);

		//start with the new y value
		y += 10;
		Com_sprintf (entry, size, "yv %i ", y);
		j = strlen(entry);

		//now print the rest of the entry and add the length to j
		j += Kots_HudScoreboardWriteEntry(ent, current, screen, entry + j, size - j);
		
		//ensure we won't go over the max string length
		if (len + j > size)
			break;

		strcpy(string + len, entry);
		len += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

	Array_Delete(players);
}

void Kots_HudUpdateScoreboards()
{
	int i;
	edict_t *ent;

	ent = g_edicts + 1;
	for (i = 0; i < game.maxclients; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		Kots_HudScoreboard(ent);
		gi.unicast(ent, true);
	}
}

void Kots_HudResetClicks()
{
	int i;
	edict_t *ent;

	ent = g_edicts + 1;
	for (i = 0; i < game.maxclients; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		ent->client->intermission_clicked = false;
	}
}

void Kots_HudIntermissionCheckScreen()
{
	if (!level.intermissiontime)
		return;

	if (level.intermission_dirty)
	{
		level.intermission_dirty = false;
		Kots_HudUpdateScoreboards();
		return;
	}

	//if the time hasn't ellapsed yet to move to the next screen
	//determine if we should move on by all players clicking
	if (level.time < level.intermission_nextscreen)
	{
		int i, count = 0;
		edict_t *ent;

		ent = g_edicts + 1;
		for (i = 0; i < game.maxclients; i++, ent++)
		{
			if (!ent->inuse)
				continue;
	
			//if any are false then 
			if (!ent->client->intermission_clicked)
				return;

			count++; //keep a count of clicked players
		}

		//if no players in the game then wait for ellapsed time
		if (!count)
			return;
	}

	//if this is the last screen
	if (level.intermission_screen == KOTS_INTERMISSION_EXP)
	{
		//reset screen, end intermission and go to next level
		level.intermission_screen = KOTS_INTERMISSION_SCORE;
		ExitLevel();
	}
	else
	{
		//change the screen and update all scoreboards
		level.intermission_screen++;
		level.intermission_nextscreen = level.time + KOTS_INTERMISSION_DELAY;
		level.intermission_dirty = false;
		Kots_HudResetClicks();
		Kots_HudUpdateScoreboards();
	}
}

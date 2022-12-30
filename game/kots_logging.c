#include "kots_logging.h"
#include "kots_server.h"
#include "kots_utils.h"

int FindIP(void *a, void *b)
{
    return Q_stricmp((char *)a, (char *)b);
}

void Kots_GetPlayers(int *players, int *specs)
{
    int i;
    edict_t *ent = g_edicts + 1;

    *players = 0;
    *specs = 0;
    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (ent->inuse)
        {
            if (ent->client->pers.kots_persist.is_loggedin)
                (*players)++;
            else
                (*specs)++;
        }
    }
}

void Kots_GetUniquePlayers(int *players, int *specs, int *unique)
{
    int i;
    edict_t *ent = g_edicts + 1;
    array_t *ip_list = Array_Create(NULL, game.maxclients, game.maxclients);

    *players = 0;
    *specs = 0;
    *unique = 0;
    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (ent->inuse)
        {
            if (ent->client->pers.kots_persist.is_loggedin)
            {
                (*players)++;

                if (!Array_Find(ip_list, ent->client->pers.kots_persist.ip_address, FindIP))
                {
                    Array_PushBack(ip_list, ent->client->pers.kots_persist.ip_address);
                    (*unique)++;
                }
            }
            else
                (*specs)++;
        }
    }

    //clear the list and delete the array
    Array_ClearAll(ip_list);
    Array_Delete(ip_list);
}

void Kots_LogHighScores()
{
    int i;
    qboolean found = false;
    edict_t *ent = g_edicts + 1;

    //search for unusually high scores
    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (ent->inuse && ent->character)
        {
            if (ent->character->score >= 3000)
            {
                found = true;
                break;
            }
        }
    }

    if (found)
    {
        int players, specs, unique;

        Kots_GetUniquePlayers(&players, &specs, &unique);

        //Log high scores only if there are less than 4 unique players
        if (unique < 4)
        {
            Kots_ServerLogInfo("High Score");
        }
    }
}

void Kots_LogLowScores()
{
    int i;
    qboolean found = false;
    edict_t *ent = g_edicts + 1;

    //search for unusually low scores
    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (ent->inuse && ent->character)
        {
            if (ent->character->score <= -300)
            {
                found = true;
                break;
            }
        }
    }

    if (found)
    {
        int players, specs, unique;

        Kots_GetUniquePlayers(&players, &specs, &unique);

        //Log low scores only if there are less than 4 unique players
        if (unique < 4)
        {
            Kots_ServerLogInfo("Low Score / Pack Farming");
        }
    }
}

void Kots_LogHighRatio()
{
    int i;
    qboolean found = false;
    edict_t *ent = g_edicts + 1;

    //search for unusually low scores
    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (ent->inuse && ent->character)
        {
            //only check ratio if they have 10 or more kills
            if (ent->character->player_kills)
            {
                float ratio = (float)ent->character->level_dealt / (ent->character->level_dealt + ent->character->level_taken);
                if (ratio >= 0.90)
                {
                    found = true;
                    break;
                }
            }
        }
    }

    if (found)
    {
        int players, specs, unique;

        Kots_GetUniquePlayers(&players, &specs, &unique);

        //Log low scores only if there are less than 4 unique players
        if (unique < 4 && players > 1)
        {
            Kots_ServerLogInfo("High Damage Ratio");
        }
    }
}

void Kots_LogSpreeWar()
{
    int players, specs, unique;
    Kots_GetUniquePlayers(&players, &specs, &unique);

    //Log low scores only if there are less than 4 unique players
    if (unique <= 2)
    {
        Kots_ServerLogInfo("Spreewar with few people");
    }
}

void Kots_LogOnDisconnect()
{
    Kots_LogHighRatio();
    Kots_LogHighScores();
    Kots_LogLowScores();
}

struct loginfo_s *Kots_ServerBuildLogInfo(char *reason)
{
    int players, spectators, pos, i;
    size_t size;
    loginfo_t *info = malloc(sizeof(*info));
    edict_t *ent = g_edicts + 1;

    //get the number of players in game
    Kots_GetPlayers(&players, &spectators);

    //print out the server info
    size = sizeof(info->info);
    pos = Kots_snprintf(info->info, size, "Log Reason: %s\nServer Info:\nName = %s\nMap = %s\nPlayers = %i\nSpectators = %i\n\n",
        reason, hostname->string, level.mapname, players, spectators);

    //print out the title and info for each of the clients
    pos += Kots_snprintf(info->info + pos, size - pos, "Game Info:\n%-20s%-18s%6s%6s%6s%7s%8s%8s%6s%6s%6s\n",
        "  Name", "IPAddress", "Level", "Score", "Kills", "Killed", "Dealt", "Taken", "Packs", "Creds", "Spree");
    ent = g_edicts + 1;
    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (ent->inuse)
        {
            if (ent->client->pers.kots_persist.is_loggedin)
                pos += Kots_snprintf(info->info + pos, size - pos, "  %-18s%-18s%6i%6i%6i%7i%8i%8i%6i%6i%6i\n",
                    ent->character->name, ent->client->pers.kots_persist.ip_address, ent->character->level, ent->character->score,
                    ent->client->resp.score, ent->character->level_killed, ent->character->level_dealt, ent->character->level_taken,
                    ent->character->level_packs, ent->character->level_credits, ent->character->spree);
            else
                pos += Kots_snprintf(info->info + pos, size - pos, "s %-18s%-18s%6i%6i%6i%7i%8i%8i%6i%6i%6i\n",
                    ent->client->pers.netname, ent->client->pers.kots_persist.ip_address, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        }
    }

    //return the logininfo structure
    return info;
}

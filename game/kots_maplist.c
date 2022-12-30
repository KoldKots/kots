#include "kots_maplist.h"
#include "kots_array.h"
#include "kots_utils.h"

#define KOTS_MAPVOTE_TIMEOUT 30

maplist_t maplist;

//keep track of previously visited maps
array_t *prevmaps;
array_t *failed_votes;

void Kots_Maplist_FreeEntry(void *item)
{
    gi.TagFree(item);
}

void Kots_Maplist_Init()
{
    prevmaps = Array_Create(NULL, MAPLIST_CAPACITY, MAPLIST_GROWSIZE);
    maplist.maps = Array_Create(NULL, MAPLIST_CAPACITY, MAPLIST_GROWSIZE);
    failed_votes = Array_Create(NULL, MAPLIST_CAPACITY, MAPLIST_GROWSIZE);

    //we don't want to free previous maps we only want to remove them
    prevmaps->FreeItem = NULL;
    maplist.maps->FreeItem = Kots_Maplist_FreeEntry;
}

void Kots_Maplist_Free()
{
    Array_ClearAll(prevmaps);
    Array_Delete(maplist.maps);
    Array_Delete(prevmaps);
    Array_Delete(failed_votes);

    prevmaps = NULL;
    maplist.maps = NULL;
}

votefail_t *GetVoteFail(int mapindex)
{
    //find the vote fail
    ULONG i;
    votefail_t *vote = NULL;

    //loop through each item to find it
    for (i = 0; i < failed_votes->length; i++)
    {
        vote = (votefail_t *)failed_votes->values[i];

        if (vote->mapindex == mapindex)
            return vote;
    }

    return NULL; //index not found
}

void AddVoteFail(int mapindex)
{
    //find the vote fail
    votefail_t *vote = GetVoteFail(mapindex);

    //if the vote fail was found increment the number of failures
    if (vote)
        vote->fails++;
    else
    {
        //create a new failed vote attempt
        vote = malloc(sizeof(*vote));
        vote->mapindex = mapindex;
        vote->fails = 1;

        //add the new failed vote to the list
        Array_PushBack(failed_votes, vote);
    }
}

qboolean CanVoteMap(int mapindex)
{
    //find the vote fail
    votefail_t *vote = GetVoteFail(mapindex);

    //allow voting if we haven't failed 3 or more times
    return (!vote || vote->fails < 3);
}

ULONG GetPreviousMapIndex(char *mapname)
{
    //find the index of the specified map
    ULONG i;

    //loop through each item to find it's index
    for (i = 0; i < prevmaps->length; i++)
        if (Q_stricmp(((mapentry_t *)prevmaps->values[i])->mapname, mapname) == 0)
            return i;

    return -1; //index not found
}

ULONG GetMaplistIndex(char *mapname)
{
    //find the index of the specified map
    ULONG i;

    //loop through each item to find it's index
    for (i = 0; i < maplist.maps->length; i++)
        if (Q_stricmp(((mapentry_t *)maplist.maps->values[i])->mapname, mapname) == 0)
            return i;

    return -1; //index not found
}

void Kots_Maplist_UpdateMapIndex()
{
    maplist.currentmap = GetMaplistIndex(level.mapname);
}

void Kots_Maplist_RemoveMap(ULONG mapindex)
{
    if (maplist.maps->length <= mapindex)
    {
        gi.dprintf("ERROR: Tried to remove map at index %u. Out of array bounds.\n", mapindex);
        return;
    }

    Array_DeleteAt(maplist.maps, mapindex);

    //loop through each item to update it's index
    for ( ; mapindex < maplist.maps->length; mapindex++)
    {
        ((mapentry_t *)maplist.maps->values[mapindex])->index = mapindex;
    }
}

FILE *Kots_Maplist_OpenFile(char *filename)
{
    FILE *fp = NULL;
    char path[MAPLIST_MAX_FILENAME];

    //create path to the file using gamedir
    Kots_snprintf(path, MAPLIST_MAX_FILENAME, "%s/%s", gamedir->string, filename);

    //try to open the file
    if ((fp = fopen(path, "r")) == NULL)
    {
        // file did not load
        gi.dprintf ("Could not open file \"%s\".\n", filename);
        return NULL;
    }

    return fp;
}

void Kots_Maplist_CloseFile(FILE *fp)
{
    //if we have a valid file pointer
    if (fp)
        fclose(fp);
    else //no file is opened
        gi.dprintf ("ERROR: Attempted to close invalid file handle.\n");
}

int Kots_Maplist_SortCompare(void *item1, void *item2)
{
    mapentry_t *a = (mapentry_t *)item1;
    mapentry_t *b = (mapentry_t *)item2;

    return Q_stricmp(a->mapname, b->mapname);
}

int Kots_Maplist_Load(char *filename)
{
    FILE *fp;
    char line[1024];
    mapentry_t *entry;
    int result;

    //if we try to load a null or empty string just clear the maplist and return
    if (!filename || !filename[0])
    {
        Kots_Maplist_ClearMapList();
        return 1;
    }

    //open the specified file
    fp = Kots_Maplist_OpenFile(filename);

    //if we got a file poniter
    if (fp)
    {
        //if there's already a maplist loaded then clear it
        if (maplist.maps->length > 0)
            Kots_Maplist_ClearMapList();


        while (fgets(line, sizeof(line), fp) != NULL)
        {
            //ignore lines starting with # because they are comments
            if (line[0] == '#')
                continue;

            //allocate a new mapentry to read in
            entry = gi.TagMalloc(sizeof(*entry), TAG_GAME);

            //read in the map information
            result = sscanf(line, "%d %d %d %256s", &entry->min_players, &entry->max_players, &entry->priority, entry->mapname);

            //check if the line was successfully formatted
            if (result == 4)
            {
                if (entry->min_players < 0)
                    entry->min_players = 0;
                else if (entry->min_players > 99)
                    entry->min_players = 99;

                if (entry->max_players > 99)
                    entry->max_players = 99;
                else if (entry->max_players < entry->min_players)
                    entry->max_players = entry->min_players;

                if (entry->priority < 0)
                    entry->priority = 0;
                else if (entry->priority > 9)
                    entry->priority = 9;

                entry->index = maplist.maps->length;
                Array_PushBack(maplist.maps, entry);
            }
            else
            {
                gi.TagFree(entry);
                entry = NULL;
            }
        }

        Kots_Maplist_CloseFile(fp);

        if (!maplist.maps->length)
        {
            gi.dprintf("No maps found in map list %s.\n", filename);
            return 0;  // abnormal exit -- no maps in file
        }
        else
        {
            Kots_strncpy(maplist.filename, filename, MAPLIST_MAX_FILENAME);
            gi.cvar_forceset(kots_maplist->name, maplist.filename);
            gi.dprintf ("%d map(s) loaded from maplist %s.\n", maplist.maps->length, filename);
            maplist.currentmap = GetMaplistIndex(level.mapname);
            return 1; // normal exit
        }
    }

    return 0;  // abnormal exit -- couldn't open file
}

int Kots_Maplist_GetPlayerCount()
{
    int i;
    int count = 0;

    for (i = 1; i <= maxclients->value; i++)
        if (g_edicts[i].inuse)
            count++;

    return count;
}

int Kots_Maplist_GetNextMap()
{
    ULONG i;
    ULONG start = maplist.currentmap + 1;
    mapentry_t *entry;
    int players = Kots_Maplist_GetPlayerCount();

    if (start >= maplist.maps->length)
        start = 0; //start back at the beginning

    i = start;

    //try to obey all the rules in the first pass
    do
    {
        entry = (mapentry_t *)maplist.maps->values[i];

        if (entry->priority != 0 && players >= entry->min_players && players <= entry->max_players)
            return i;

        i = (i + 1) % maplist.maps->length;

    } while (i != start);

    //only obey priority now
    do
    {
        entry = (mapentry_t *)maplist.maps->values[i];

        if (entry->priority != 0)
            return i;

        i = (i + 1) % maplist.maps->length;

    } while (i != start);

    return start;
}

int Kots_Maplist_GetRandomMap()
{
    ULONG i;
    int players, priority, rand_index;
    int total = 0;
    mapentry_t *entry;
    array_t *valid = Array_Create(NULL, maplist.maps->length, MAPLIST_GROWSIZE);
    valid->FreeItem = NULL;

    //get the number of players in the game
    players = Kots_Maplist_GetPlayerCount();

    //first get all qualified maps
    for (i = 0; i < maplist.maps->length; i++)
    {
        entry = (mapentry_t *)maplist.maps->values[i];

        //no matter what don't allow re-entry into the current map
        if (entry->index == maplist.currentmap)
            continue;

        //check normal requirements first
        if (entry->min_players > players)
            continue;
        if (entry->max_players < players)
            continue;
        if (entry->priority == 0)
            continue;

        //try not to use the previous maps if possible
        if (Array_IndexOf(prevmaps, entry) != -1)
            continue;

        //map appears to be valid
        Array_PushBack(valid, entry);
        total += entry->priority; //tally up priority
    }

    //if we don't have enough maps then just pick any map
    if (valid->length < MAPLIST_MINMAPS)
    {
        Array_ClearAll(valid);
        total = 0;

        //first get all qualified maps
        for (i = 0; i < maplist.maps->length; i++)
        {
            entry = (mapentry_t *)maplist.maps->values[i];

            //no matter what don't allow re-entry into the current map
            if (entry->index == maplist.currentmap)
                continue;

            //only check priority now
            if (entry->priority == 0)
                continue;

            //try not to use the previous maps if possible
            if (Array_IndexOf(prevmaps, entry) != -1)
                continue;

            //map appears to be valid
            Array_PushBack(valid, entry);
            total += entry->priority; //tally up priority
        }
    }

    //if we still don't have enough maps try dipping into previous maps
    if (valid->length < MAPLIST_MINMAPS)
    {
        //loop through previous maps for valid maps or until we hit the minimum
        for (i = 0; i < prevmaps->length && valid->length < MAPLIST_MINMAPS; i++)
        {
            entry = (mapentry_t *)prevmaps->values[i];

            //no matter what don't allow re-entry into the current map
            if (entry->index == maplist.currentmap)
                continue;

            //only check priority now
            if (entry->priority == 0)
                continue;

            //map appears to be valid
            Array_PushBack(valid, entry);
            total += entry->priority; //tally up priority
        }
    }

    //if for some reason we still don't have any choices just blindly pick one
    if (!valid->length)
    {
        //randomly pick a new map, but make sure it's not the current one
        do
        {
            rand_index = rand() % maplist.maps->length;
        } while (rand_index != maplist.currentmap && maplist.maps->length > 1);
    }
    else
    {
        rand_index = rand() % total; //pick a number between 0 and total priority
        priority = 0;

        //find the map we picked
        for (i = 0; i < valid->length; i++)
        {
            entry = (mapentry_t *)valid->values[i];
            priority += entry->priority;

            if (priority > rand_index)
            {
                rand_index = entry->index;
                break;
            }
        }
    }

    //delete the array
    Array_Delete(valid);

    return rand_index; //return index of map to use
}

void Kots_Maplist_AddPrevMap()
{
    if (maplist.maps->length > 0 && maplist.currentmap != -1)
    {
        int prev = kots_prevmaps->value;

        //enforce at least 1 previous map to be saved
        if (prev <= 0)
        {
            gi.cvar_forceset(kots_prevmaps->name, "1");
            prev = 1;
        }

        //if we've hit the limit pop the first map off the list
        if (prevmaps->length >= kots_prevmaps->value || prevmaps->length >= maplist.maps->length)
            Array_PopFront(prevmaps);

        Array_PushBack(prevmaps, maplist.maps->values[maplist.currentmap]);
    }
}

char *Kots_Maplist_GetName(int index)
{
    return ((mapentry_t *)maplist.maps->values[index])->mapname;
}

void Kots_Maplist_ClearMapList()
{
    if (!maplist.maps->length)
        gi.cprintf(NULL, PRINT_HIGH, "No maplist loaded.\n");
    {
        if (level.mapvotestarted)
        {
            level.mapvote = -1;
            level.mapvotestarted = 0;
            gi.bprintf(PRINT_HIGH, "Map vote was cancelled because the server cleared the maplist.\n");
        }

        maplist.currentmap = -1;
        maplist.filename[0] = '\0';
        gi.cvar_forceset(kots_maplist->name, "");
        Array_ClearAll(prevmaps);
        Array_DeleteAll(maplist.maps);
        gi.cprintf(NULL, PRINT_HIGH, "Maplist cleared.\n");
    }
}

void Kots_Maplist_Show(edict_t *ent)
{
    if (!maplist.maps->length)
    {
        gi.cprintf(ent, PRINT_HIGH, "No maplist currently active.\nServers should use 'sv maplist' to set one.\n");
    }
    else
    {
        //static char *modes[] = {NULL, "Deathmatch", "Defend The Base", "Capture The Flag", NULL};
        static char *rotations[] = {"Sequential", "Random"};
        ULONG i;
        mapentry_t *entry;

        if (ent == NULL) // only show filename to server
            Kots_SlowPrint(ent, "Maplist File: %s\n", maplist.filename);

        for (i = 0; i < maplist.maps->length; i++)
        {
            entry = (mapentry_t *)maplist.maps->values[i];
            Kots_SlowPrint(ent, "#%4d. %s\n", i + 1, entry->mapname);
        }

        Kots_SlowPrint(ent, "%d map(s) in list.\n", maplist.maps->length);
        Kots_SlowPrint(ent, "Rotation Mode: %s\n", rotations[maplist.rotationflag]);

        if (maplist.currentmap == -1)
            Kots_SlowPrint(ent, "Current map: %s (not in maplist)\n");
        else
            Kots_SlowPrint(ent, "Current map: #%d %s\n", maplist.currentmap + 1 , Kots_Maplist_GetName(maplist.currentmap));

    }
}

void Kots_Maplist_ServerCommand()
{
    char *command = gi.argv(2);

    switch (gi.argc())
    {
        case 3:

            //determine which command was used
            if (Q_stricmp(command, "next") == 0)
            {
                Kots_Maplist_Goto(NULL, "next");
                break;
            }
            else if (Q_stricmp(command, "clear") == 0)
            {
                Kots_Maplist_ClearMapList();
                break;
            }
            else if (Q_stricmp(command, "show") == 0)
            {
                Kots_Maplist_Show(NULL);
                break;
            }
            else if (Q_stricmp(command, "reload") == 0)
            {
                Kots_Maplist_Load(maplist.filename);
                break;
            }

        //case 3 falls into case 4 when setting a maplist
        case 4:

            //if we specified to go to a specific map
            if (Q_stricmp(command, "goto") == 0)
                Kots_Maplist_Goto(NULL, gi.argv(3));

            //if we're just changing the rotation flag stop here
            else if (Q_stricmp(command, "rotation") == 0)
                Kots_Maplist_ServerChangeRotation();

            else //assume we're trying to set a new maplist
            {
                // load new maplist
                if (Kots_Maplist_Load(command))
                {
                    maplist.rotationflag = atoi(gi.argv(3));
                    gi.cvar_forceset(kots_maprotate->name, va("%i", maplist.rotationflag));
                    gi.cprintf(NULL, PRINT_HIGH, "Maplist loaded.\n");
                }
            }
            break;

        default:
            Kots_Maplist_ServerDisplayUsage(NULL);
    }
}

void Kots_Maplist_ServerDisplayUsage(edict_t *ent)
{
    gi.cprintf(ent, PRINT_HIGH, "Usage:\n");
    gi.cprintf(ent, PRINT_HIGH, "sv maplist <filename> [<rotation>]\n");
    gi.cprintf(ent, PRINT_HIGH, "           <filename>             - server ini file\n");
    gi.cprintf(ent, PRINT_HIGH, "                       <rotation> - 0 = sequential (default)\n");
    gi.cprintf(ent, PRINT_HIGH, "                                    1 = random\n\n");
    gi.cprintf(ent, PRINT_HIGH, "sv maplist rotation <rotation>    - change the rotation\n");
    gi.cprintf(ent, PRINT_HIGH, "sv maplist next                   - go to the next map\n");
    gi.cprintf(ent, PRINT_HIGH, "sv maplist goto <mapname>         - go to the specified map\n");
    gi.cprintf(ent, PRINT_HIGH, "sv maplist clear                  - clear the current maplist\n");
    gi.cprintf(ent, PRINT_HIGH, "sv maplist show                   - show the maplist\n");
    gi.cprintf(ent, PRINT_HIGH, "sv maplist reload                 - reloads the maplist\n");
}

edict_t *CreateTargetChangeLevel(char *map);
void Kots_Maplist_Goto(edict_t *ent, char *mapname)
{
    //if a maplist exists end the current level and go to the next
    if (maplist.maps->length > 0)
    {
        if (Q_stricmp(mapname, "next") == 0)
        {
            //end the level to force next map
            EndDMLevel();
        }
        else
        {
            //attempt to find the map by name
            int index = GetMaplistIndex(mapname);

            if (index >= 0)
            {
                BeginIntermission(CreateTargetChangeLevel(mapname));
                maplist.currentmap = index;
            }
            else
                gi.cprintf(ent, PRINT_HIGH, "Specified map was not found in the maplist.\n");
        }
    }
    else
        gi.cprintf(ent, PRINT_HIGH, "No maplist loaded.\n");
}

void Kots_Maplist_ServerChangeRotation()
{
    //if a maplist exists end the current level and go to the next
    if (maplist.maps->length > 0)
    {
        int rotation = atoi(gi.argv(3));
        if (rotation < ML_ROTATE_SEQ || rotation > ML_ROTATE_RANDOM)
            gi.cprintf(NULL, PRINT_HIGH, "Invalid rotation setting. Should be 0 (sequential) or 1 (random).\n");
        else
        {
            gi.cvar_forceset(kots_maprotate->name, va("%i", rotation));
            maplist.rotationflag = rotation;
        }
    }
    else
        gi.cprintf(NULL, PRINT_HIGH, "No maplist loaded.\n");
}

void Kots_Maplist_CheckVote(edict_t *ent)
{
    int i;
    int players = 0;
    int yes = 0;
    int no = 0;
    int needed = 0;
    edict_t *cl_ent;

    //don't check the vote when none in progress or at intermission
    if (!level.mapvotestarted || level.intermissiontime)
        return;

    cl_ent = g_edicts + 1;
    for (i = 0; i<game.maxclients; i++, cl_ent++)
    {
        if (!cl_ent->inuse || !cl_ent->character)
            continue;
        if(!cl_ent->character->is_loggedin)
            continue;

        players++;

        if(cl_ent->client->pers.kots_persist.map_vote == MAPVOTE_NO)
            no++;
        else if(cl_ent->client->pers.kots_persist.map_vote == MAPVOTE_YES)
            yes++;
    }

    needed = (players / 2) + 1;

    if (ent) //no entity is passed when a player disconnects so we don't print this message
        gi.bprintf(PRINT_MEDIUM, "Vote for map %s: %i YES, %i NO, NEEDED: %i\n", Kots_Maplist_GetName(level.mapvote), yes, no, needed);

    if(yes >= needed) // passed
    {
        char *name = Kots_Maplist_GetName(level.mapvote);
        gi.bprintf(PRINT_MEDIUM, "Vote for map %s PASSED!\n", name);
        Kots_snprintf(level.nextmap, 64, "%s", name); //SWB - we need to set the map in order to go to it
        level.passed = true;
        EndDMLevel();

        //reset mapvote state
        level.mapvote = -1;
        level.mapvotestarted = 0;
    }
    else if(players <= (yes + no)) // everyone voted
    {
        gi.bprintf(PRINT_MEDIUM, "Vote for map %s FAILED!\n", Kots_Maplist_GetName(level.mapvote));

        //reset mapvote state
        AddVoteFail(level.mapvote);
        level.mapvote = -1;
        level.mapvotestarted = 0;

        cl_ent = g_edicts + 1;
        for (i=0 ; i<game.maxclients ; i++, cl_ent++) // reset votes
        {
            if (!cl_ent->inuse || !cl_ent->character)
                continue;
            if(!cl_ent->character->is_loggedin) // specs dont count
                continue;

            cl_ent->client->pers.kots_persist.map_vote = MAPVOTE_NONE;
        }
    }
}

void Kots_Maplist_CheckVoteTimeout()
{
    int     i;
    edict_t *cl_ent;

    if (level.intermissiontime) //already in intermission
        return;
    if (!level.mapvotestarted) // no vote
        return;
    else if (level.mapvotestarted > level.time - KOTS_MAPVOTE_TIMEOUT) // timeout
        return;

    for (i=0 ; i<game.maxclients ; i++) // reset votes
    {
        cl_ent = g_edicts + 1 + i;
        if (!cl_ent->inuse)
            continue;
        if(!cl_ent->character->is_loggedin)
            continue;

        cl_ent->client->pers.kots_persist.map_vote = MAPVOTE_NONE;
    }

    // Send a message
    gi.bprintf(PRINT_MEDIUM, "Vote for map %s FAILED (time out)\n", Kots_Maplist_GetName(level.mapvote));

    // Ok expire the map vote
    AddVoteFail(level.mapvote);
    level.mapvote = 0;
    level.mapvotestarted = 0;

    gi.sound (NULL, CHAN_VOICE, gi.soundindex("world/fuseout.wav"), 1, ATTN_NONE, 0); // play a sound
}

void Kots_Maplist_Vote(edict_t *ent, int vote)
{
    //ensure vote is in progress
    if(!level.mapvotestarted)
    {
        gi.cprintf(ent, PRINT_HIGH, "There is no vote currently in progress.\n");
        return;
    }

    //ensure that we haven't already voted
    if(ent->client->pers.kots_persist.map_vote != MAPVOTE_NONE)
    {
        gi.cprintf(ent, PRINT_HIGH, "You have already voted.\n");
        return;
    }

    ent->client->pers.kots_persist.map_vote = vote;
    Kots_Maplist_CheckVote(ent);
}

void Kots_Maplist_VoteCommand(edict_t *ent)
{
    char        *mapname;
    int         index;

    if (gi.argc() < 3)
    {
        gi.cprintf(ent, PRINT_HIGH, "Usage: vote map mapname\n");
        return;
    }
    if(level.mapvotestarted) // theres a vote going on
    {
        gi.cprintf(ent, PRINT_HIGH, "There is already a vote in progress.\n");
        return;
    }
    if(level.time < 20) // give people a chance to join
    {
        gi.cprintf(ent, PRINT_HIGH, "This map just started. Please wait %i more seconds.\n", (int)ceil(20.0 - level.time));
        return;
    }

    mapname = gi.argv(2);

    if (Q_stricmp(mapname, "next") == 0)
        index = Kots_Maplist_GetNextMap();
    else
        index = GetMaplistIndex(mapname);

    if(index >= 0)
    {
        if(index == maplist.currentmap)
        {
            gi.cprintf(ent, PRINT_HIGH, "Cannot vote for current map!\n");
            return;
        }
        else
        {
            //determine if it was recently played
            int prev_index = GetPreviousMapIndex(mapname);

            //don't allow voting for maps that were recently played
            if (prev_index >= 0)
            {
                gi.cprintf(ent, PRINT_HIGH, "This map was recently played and cannot be voted again for a while.\n");
                return;
            }

            //check if this map hasn't failed a vote too many times
            if (!CanVoteMap(index))
            {
                gi.cprintf(ent, PRINT_HIGH, "Voting has failed too many times for this map.\n");
                return;
            }
        }

        level.mapvote = index;
        level.mapvotestarted = level.time;

        gi.bprintf(PRINT_MEDIUM, "%s started a map vote for %s. Type 'vote yes' or 'vote no' to vote.\n", ent->client->pers.netname, mapname);

        gi.sound (ent, CHAN_VOICE, gi.soundindex("world/x_light.wav"), 1, ATTN_NONE, 0); // play a sound

        Kots_Maplist_Vote(ent, MAPVOTE_YES); // this guy votes yes
    }
    else
        gi.cprintf(ent, PRINT_HIGH, "Could not find map '%s'. Type 'maplist' to see all map names.\n", mapname);
}

void Kots_Maplist_ClearVoteInfo()
{
    //clear the voting information
    level.mapvote = -1;
    level.mapvotestarted = 0.0;
    level.passed = false;

    //remove all the failed vote attempts
    Array_DeleteAll(failed_votes);
}

void Kots_Maplist_PrintVoteInfo(edict_t *ent)
{
    static char *vote_names[] = {"Not voted", "Yes", "No"};

    Kots_SlowPrint(ent, "Map Vote Info:\n");

    if (!level.mapvotestarted)
    {
        Kots_SlowPrint(ent, "No vote in progress.\n");
    }
    else
    {
        edict_t *player = NULL;
        int i;

        Kots_SlowPrint(ent, "Voting for map: %s\n", Kots_Maplist_GetName(level.mapvote));
        Kots_SlowPrint(ent, "Time remaining: %2.2f second(s)\n", (level.mapvotestarted + KOTS_MAPVOTE_TIMEOUT) - level.time);
        Kots_SlowPrint(ent, "\nPlayer Votes:\n");
        Kots_SlowPrint(ent, "%-20s %-10s\n", "Name", "Vote");

        player = g_edicts + 1;
        for (i = 0; i < game.maxclients; i++, player++)
        {
            if (player->inuse && player->character && player->character->is_loggedin)
            {
                Kots_SlowPrint(ent, "%-20s %-10s\n", player->client->pers.netname, vote_names[player->client->pers.kots_persist.map_vote]);
            }
        }
    }
}


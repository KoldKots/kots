/*
============================
kots_admin.c
Admin Functions

- List User IP's
- Kick User
- Give/Take <value> Points
- Make Admin
- Ban User
- Spawn Monster
============================
*/

#include "kots_admin.h"
#include "kots_runes.h"
#include "kots_maplist.h"
#include "kots_monster.h"
#include "kots_server.h"
#include "kots_utils.h"

edict_t *Kots_AdminFindPlayerByIp(char *ip_address)
{
    int i;
    edict_t *user = g_edicts + 1;

    for (i = 0; i < game.maxclients; i++, user++)
    {
        if (!user->inuse)
            continue;

        //if the ip address matches
        if(Q_stricmp(user->client->pers.kots_persist.ip_address, ip_address) == 0)
            return user; //return this edict
    }

    return NULL; //return not found
}

edict_t *Kots_AdminFindPlayer(char *name, edict_t *ignore)
{
    int i;
    edict_t *user = g_edicts + 1;

    for (i = 0; i < game.maxclients; i++, user++)
    {
        if (!user->inuse)
            continue;

        if (user == ignore)
            continue;

        //if the name matches
        if(Q_stricmp(user->client->pers.netname, name) == 0)
            return user; //return this edict
    }

    return NULL; //return not found
}

void Kots_Admin_CreateRune(edict_t *ent, char *args)
{
    if (level.intermissiontime)
        gi.cprintf(ent, PRINT_HIGH, "Can't create runes during intermission.\n");
    else if (Kots_RunesOnMap() >= KOTS_RUNES_MAX_DROPPED * 2) //admins can only spawn up to twice the max
        gi.cprintf(ent, PRINT_HIGH, "Can't create any more runes. Too many on map.\n");
    else
    {
        rune_t *rune = Kots_RuneFindByName(args);

        if (!rune)
            gi.cprintf(ent, PRINT_HIGH, "Rune '%s' not found.\n", args);
        else
        {
            gi.cprintf(ent, PRINT_HIGH, "Rune '%s' has been dropped.\n", args);
            Kots_RuneDropAt(ent, rune);
        }
    }
}

void Kots_Admin_SpawnMonster(edict_t *ent, char *args)
{
    if (level.intermissiontime)
        gi.cprintf(ent, PRINT_HIGH, "Can't spawn monsters during intermission.\n");
    else if (spreewar.warent)
        gi.cprintf(ent, PRINT_HIGH, "Can't spawn monsters during spree war.\n");
    else if (level.total_monsters >= KOTS_MONSTERS_MAX)
        gi.cprintf(ent, PRINT_HIGH, "Maximum monsters reached. Can't spawn any more monsters.\n");
    else if (KOTS_MONSTER_VOTE == KOTS_MONSTERS_OFF)
        gi.cprintf(ent, PRINT_HIGH, "You cannot spawn a monster while monsters are voted off.\n");
    else if (!args)
        gi.cprintf(ent, PRINT_HIGH, "USAGE: kots_monster MonsterName");
    else
    {
        kots_monster_t *monster = Kots_MonsterFind(args);

        if (!monster)
            gi.cprintf(ent, PRINT_HIGH, "Unable to find monster %s.\n", args);
        else if (!Kots_MonsterSpawnNew(monster))
            gi.cprintf(ent, PRINT_HIGH, "Unable to spawn monster. No suitable spawn found.\n");
        else
            gi.cprintf(ent, PRINT_HIGH, "Monster %s has been spawned.\n", args);
    }
}

void Kots_Admin_ListIP(edict_t *ent)
{
    int i;
    edict_t *cl_ent;

    // Loop through all users
    for (i=0;i<game.maxclients;i++)
    {
        cl_ent = g_edicts + 1 + i;
        if (!cl_ent->inuse)
            continue;
        
        gi.cprintf(ent,PRINT_HIGH,"%-16s %s\n", cl_ent->client->pers.netname, cl_ent->client->pers.kots_persist.ip_address);
    }
}

void Kots_Admin_KickUser(edict_t *ent, edict_t *kick)
{
    if (ent && ent->inuse)
        gi.bprintf(PRINT_MEDIUM, "%s was kicked by %s.\n", kick->client->pers.netname, ent->client->pers.netname);
    else
        gi.bprintf(PRINT_MEDIUM, "%s was kicked by the server.\n", kick->client->pers.netname);

    gi.AddCommandString(va("kick %d", kick - g_edicts - 1));
}

void Kots_Admin_Curse(edict_t *ent, edict_t *other)
{
    if (!other->character->is_loggedin)
    {
        gi.cprintf(ent, PRINT_MEDIUM, "%s must be logged in.\n", other->client->pers.netname);
        return;
    }

    if(other->client->pers.kots_persist.is_admin)
    {
        gi.cprintf(ent, PRINT_MEDIUM, "%s is an admin and cannot be banned.\n", other->client->pers.netname);
        return;
    }

    gi.bprintf(PRINT_MEDIUM, "%s was cursed by %s!\n", ent->client->pers.netname, other->client->pers.netname); 
    other->character->is_cursed = true; 
    Kots_Admin_KickUser(ent, other);
}

void Kots_Admin_KickByName(edict_t *admin, char *name)
{
    edict_t *ent = NULL;

    //if the user exists
    if ((ent = Kots_AdminFindPlayer(name, admin)) != NULL)
    {
        //kick him
        Kots_Admin_KickUser(admin, ent);
    }
    else
        gi.cprintf(admin, PRINT_HIGH, "Could not find player %s.\n", name);
}

void Kots_Admin_CurseByName(edict_t *admin, char *name)
{
    edict_t *ent = NULL;

    //if the user exists
    if ((ent = Kots_AdminFindPlayer(name, admin)) != NULL)
    {
        //kick him
        Kots_Admin_Curse(admin, ent);
    }
    else
        gi.cprintf(admin, PRINT_HIGH, "Could not find player %s.\n", name);
}

void Kots_Admin_IpBan(edict_t *admin, char *name)
{
    edict_t *ent = NULL;

    //if the user exists
    if ((ent = Kots_AdminFindPlayer(name, admin)) != NULL)
    {
        //kick him
        Kots_ServerBanIp(admin, ent);
    }
    else
        gi.cprintf(admin, PRINT_HIGH, "Could not find player %s.\n", name);
}

void Kots_AdminSilence(edict_t *ent)
{
    if (level.is_silenced)
    {
        gi.cprintf(ent, PRINT_HIGH, "The level is already silenced.\n");
        return;
    }

    level.is_silenced = true;
    Kots_CenterPrintAll("This level has been silenced until further notice by %s.", ent->client->pers.netname);
}

void Kots_AdminUnsilence(edict_t *ent)
{
    if (!level.is_silenced)
    {
        gi.cprintf(ent, PRINT_HIGH, "The level is not silenced.\n");
        return;
    }

    level.is_silenced = false;
    Kots_CenterPrintAll("This level has been unsilenced by %s.", ent->client->pers.netname);
}

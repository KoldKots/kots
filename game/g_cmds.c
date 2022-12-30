/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"

//SWB - KOTS Includes
#include "kots_runes.h"
#include "kots_menu.h"
#include "kots_utils.h"
#include "kots_commands.h"


char *ClientTeam (edict_t *ent)
{
    char        *p;
    static char value[512];

    value[0] = 0;

    if (!ent->client)
        return value;

    strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
    p = strchr(value, '/');
    if (!p)
        return value;

    if ((int)(dmflags->value) & DF_MODELTEAMS)
    {
        *p = 0;
        return value;
    }

    // if ((int)(dmflags->value) & DF_SKINTEAMS)
    return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
    //if both players are either logged in or both are logged out consider them the same team
    if (!ent1->client->pers.kots_persist.is_loggedin && !ent2->client->pers.kots_persist.is_loggedin)
        return true;
    else
        return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->client;

    //SWB - PMenu Changes
    if (cl->menu) {
        PMenu_Next(ent);
        return;
    }

    else if (cl->chase_target)
    {
        ChaseNext(ent);
        return;
    }

    // scan  for the next valid one
    for (i=1 ; i<=MAX_ITEMS ; i++)
    {
        index = (cl->pers.selected_item + i)%MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->pers.selected_item = index;
        return;
    }

    cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->client;

    //SWB - PMenu Changes
    if (cl->menu) {
        PMenu_Prev(ent);
        return;
    }

    else if (cl->chase_target) {
        ChasePrev(ent);
        return;
    }

    // scan  for the next valid one
    for (i=1 ; i<=MAX_ITEMS ; i++)
    {
        index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->pers.selected_item = index;
        return;
    }

    cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
    gclient_t   *cl;

    cl = ent->client;

    if (cl->pers.inventory[cl->pers.selected_item])
        return;     // valid

    SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
    char        *name;
    gitem_t     *it;
    int         index;
    int         i;
    qboolean    give_all;
    edict_t     *it_ent;

    if (deathmatch->value && !sv_cheats->value)
    {
        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    name = gi.args();

    if (Q_stricmp(name, "all") == 0)
        give_all = true;
    else
        give_all = false;

    if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
    {
        if (gi.argc() == 3)
            ent->health = atoi(gi.argv(2));
        else
            ent->health = ent->max_health;
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "weapons") == 0)
    {
        for (i=0 ; i<game.num_items ; i++)
        {
            it = itemlist + i;
            if (!it->pickup)
                continue;
            if (!(it->flags & IT_WEAPON))
                continue;
            ent->client->pers.inventory[i] += 1;
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "ammo") == 0)
    {
        for (i=0 ; i<game.num_items ; i++)
        {
            it = itemlist + i;
            if (!it->pickup)
                continue;
            if (!(it->flags & IT_AMMO))
                continue;
            Add_Ammo (ent, it, 1000);
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "armor") == 0)
    {
        gitem_armor_t   *info;

        it = FindItem("Jacket Armor");
        ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

        it = FindItem("Combat Armor");
        ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

        it = FindItem("Body Armor");
        info = (gitem_armor_t *)it->info;
        ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "Power Shield") == 0)
    {
        it = FindItem("Power Shield");
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem (it_ent, it);
        Touch_Item (it_ent, ent, NULL, NULL);
        if (it_ent->inuse)
            G_FreeEdict(it_ent);

        if (!give_all)
            return;
    }

    if (give_all)
    {
        for (i=0 ; i<game.num_items ; i++)
        {
            it = itemlist + i;
            if (!it->pickup)
                continue;
            if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
                continue;
            ent->client->pers.inventory[i] = 1;
        }
        return;
    }

    it = FindItem (name);
    if (!it)
    {
        name = gi.argv(1);
        it = FindItem (name);
        if (!it)
        {
            gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
            return;
        }
    }

    if (!it->pickup)
    {
        gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
        return;
    }

    index = ITEM_INDEX(it);

    if (it->flags & IT_AMMO)
    {
        if (gi.argc() == 3)
            ent->client->pers.inventory[index] = atoi(gi.argv(2));
        else
            ent->client->pers.inventory[index] += it->quantity;
    }
    else
    {
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem (it_ent, it);
        Touch_Item (it_ent, ent, NULL, NULL);
        if (it_ent->inuse)
            G_FreeEdict(it_ent);
    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
    char    *msg;

    if (deathmatch->value && !sv_cheats->value)
    {
        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->flags ^= FL_GODMODE;
    if (!(ent->flags & FL_GODMODE) )
        msg = "godmode OFF\n";
    else
        msg = "godmode ON\n";

    gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
    char    *msg;

    if (deathmatch->value && !sv_cheats->value)
    {
        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->flags ^= FL_NOTARGET;
    if (!(ent->flags & FL_NOTARGET) )
        msg = "notarget OFF\n";
    else
        msg = "notarget ON\n";

    gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
    char    *msg;

    if (deathmatch->value && !sv_cheats->value)
    {
        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    if (ent->movetype == MOVETYPE_NOCLIP)
    {
        ent->movetype = MOVETYPE_WALK;
        msg = "noclip OFF\n";
    }
    else
    {
        ent->movetype = MOVETYPE_NOCLIP;
        msg = "noclip ON\n";
    }

    gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
    int         index;
    gitem_t     *it;
    char        *s;

    s = gi.args();
    it = FindItem (s);
    if (!it)
    {
        gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
        return;
    }
    if (!it->use)
    {
        gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    index = ITEM_INDEX(it);
    if (!ent->client->pers.inventory[index])
    {
        gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
        return;
    }

    it->use (ent, it);
}


//power cube pickup defiend in g_items.c
void Drop_PowerCubes(edict_t *ent, gitem_t *item);

/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
    int         index;
    gitem_t     *it;
    char        *s;

    //SWB - don't allow dropping items during intermission or death
    if (level.intermissiontime || ent->health <= 0)
        return;

    s = gi.args();
    it = FindItem (s);
    if (!it)
    {
        gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
        return;
    }
    if (!it->drop)
    {
        gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    index = ITEM_INDEX(it);

    //SWB - power cubes will be handled separately as KOTS cubes
    if (it->drop != Drop_PowerCubes)
    {
        if (!ent->client->pers.inventory[index])
        {
            gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
            return;
        }
    }

    //SWB - Weapons not dropable in kots at least not by commands
    //TODO: May want to make weapons and ammo droppable in team modes
    if (it->weapmodel && it->weapmodel != WEAP_GRENADES && it->weapmodel != WEAP_BLASTER)
        return;

    it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
    int         i;
    gclient_t   *cl;

    cl = ent->client;

    //SWB - PMenu Changes
    if (ent->client->menu) {
        PMenu_Close(ent);
        ent->client->update_chase = true;
        return;
    }
    else
    {
        Kots_MenuShow(ent, NULL);
        return;
    }

    cl->showscores = false;
    cl->showhelp = false;

    if (cl->showinventory)
    {
        cl->showinventory = false;
        return;
    }

    cl->showinventory = true;

    gi.WriteByte (svc_inventory);
    for (i=0 ; i<MAX_ITEMS ; i++)
    {
        gi.WriteShort (cl->pers.inventory[i]);
    }
    gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
    gitem_t     *it;

    if (ent->client->menu) {
        PMenu_Select(ent);
        return;
    }

    ValidateSelectedItem (ent);

    if (ent->client->pers.selected_item == -1)
    {
        gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
        return;
    }

    it = &itemlist[ent->client->pers.selected_item];
    if (!it->use)
    {
        gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    //SWB - PMenu Changes
    if (cl->menu) {
        PMenu_Prev(ent);
        return;
    }

    if (!cl->pers.weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->pers.weapon);

    // scan  for the next valid one
    for (i=1 ; i<=MAX_ITEMS ; i++)
    {
        index = (selected_weapon + i)%MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (! (it->flags & IT_WEAPON) )
            continue;
        it->use (ent, it);
        if (cl->pers.weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    //SWB - PMenu Changes
    if (cl->menu) {
        PMenu_Next(ent);
        return;
    }

    if (!cl->pers.weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->pers.weapon);

    // scan  for the next valid one
    for (i=1 ; i<=MAX_ITEMS ; i++)
    {
        index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (! (it->flags & IT_WEAPON) )
            continue;
        it->use (ent, it);
        if (cl->pers.weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
    gclient_t   *cl;
    int         index;
    gitem_t     *it;

    cl = ent->client;

    if (!cl->pers.weapon || !cl->pers.lastweapon)
        return;

    index = ITEM_INDEX(cl->pers.lastweapon);
    if (!cl->pers.inventory[index])
        return;
    it = &itemlist[index];
    if (!it->use)
        return;
    if (! (it->flags & IT_WEAPON) )
        return;
    it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
    gitem_t     *it;

    //SWB - Items not dropable in kots
    //TODO: May want to make weapons and ammo droppable in team play modes
    return;

    ValidateSelectedItem (ent);

    if (ent->client->pers.selected_item == -1)
    {
        gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
        return;
    }

    it = &itemlist[ent->client->pers.selected_item];
    if (!it->drop)
    {
        gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
    //SWB - command only allowed for admins in kots
    if (!ent->client->pers.kots_persist.is_admin)
        return;

    if((level.time - ent->client->respawn_time) < 5)
        return;
    ent->flags &= ~FL_GODMODE;
    ent->health = 0;
    meansOfDeath = MOD_SUICIDE;

    //player_die (ent, ent, ent, 100000, vec3_origin);

    //SWB - character killed
    //Kots_CharacterKilled(ent, ent, ent->s.origin);
    T_Damage(ent, ent, ent, ent->velocity, ent->s.origin, vec3_origin, ent->health + 1000, 0, DAMAGE_NO_PROTECTION | DAMAGE_NO_RESIST, MOD_SUICIDE);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
    ent->client->showscores = false;
    ent->client->showhelp = false;
    ent->client->showinventory = false;
}


int PlayerSort (void const *a, void const *b)
{
    int     anum, bnum;

    anum = *(int *)a;
    bnum = *(int *)b;

    anum = game.clients[anum].ps.stats[STAT_FRAGS];
    bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

    if (anum < bnum)
        return -1;
    if (anum > bnum)
        return 1;
    return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
    int     i;
    int     count;
    char    small[64];
    char    large[1280];
    int     index[256];

    count = 0;
    for (i = 0 ; i < maxclients->value ; i++)
        if (game.clients[i].pers.connected)
        {
            index[count] = i;
            count++;
        }

    // sort by frags
    qsort (index, count, sizeof(index[0]), PlayerSort);

    // print information
    large[0] = 0;

    for (i = 0 ; i < count ; i++)
    {
        Com_sprintf (small, sizeof(small), "%3i %s\n",
            game.clients[index[i]].ps.stats[STAT_FRAGS],
            game.clients[index[i]].pers.netname);
        if (strlen (small) + strlen(large) > sizeof(large) - 100 )
        {   // can't print all of them in one packet
            strcat (large, "...\n");
            break;
        }
        strcat (large, small);
    }

    gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
    int     i;

    i = atoi (gi.argv(1));

    // can't wave when ducked
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        return;

    if (ent->client->anim_priority > ANIM_WAVE)
        return;

    ent->client->anim_priority = ANIM_WAVE;

    switch (i)
    {
    case 0:
        gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
        ent->s.frame = FRAME_flip01-1;
        ent->client->anim_end = FRAME_flip12;
        break;
    case 1:
        gi.cprintf (ent, PRINT_HIGH, "salute\n");
        ent->s.frame = FRAME_salute01-1;
        ent->client->anim_end = FRAME_salute11;
        break;
    case 2:
        gi.cprintf (ent, PRINT_HIGH, "taunt\n");
        ent->s.frame = FRAME_taunt01-1;
        ent->client->anim_end = FRAME_taunt17;
        break;
    case 3:
        gi.cprintf (ent, PRINT_HIGH, "wave\n");
        ent->s.frame = FRAME_wave01-1;
        ent->client->anim_end = FRAME_wave11;
        break;
    case 4:
    default:
        gi.cprintf (ent, PRINT_HIGH, "point\n");
        ent->s.frame = FRAME_point01-1;
        ent->client->anim_end = FRAME_point12;
        break;
    }
}


void Kots_ReplaceText(char *source, char *start, char *replaceWith, int len)
{
    static char out[2048];
    *start = '\0'; //null terminate at starting character to create the first part

    //combine the start, new value, and end all together
    Kots_snprintf(out, sizeof(out), "%s%s%s", source, replaceWith, start + len);
    Kots_strncpy(source, out, 2048);
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
    int     i, j;
    edict_t *other;
    char    *p;
    char    text[2048];
    gclient_t *cl;

    if (gi.argc () < 2 && !arg0)
        return;

    //SWB - always allow admins to talk
    if (!ent->client->pers.kots_persist.is_admin)
    {
        //SWB - on silenced levels only allow talking between teammates
        if (level.is_silenced)
            team = true;

        //SWB - if team chat is trying to be used
        if (level.is_silenced && ent->client->pers.kots_persist.is_loggedin)
        {
            gi.centerprintf(ent, "This level has been silenced.\n");
            return;
        }

        //SWB - check for individual muting
        if (ent->client->pers.kots_persist.is_muted)
        {
            gi.centerprintf(ent, "You cannot talk because you are muted.\n");
            return;
        }
    }

    text[0] = 0;
    if (arg0)
    {
        Kots_snprintf(text, sizeof(text), "%s %s", gi.argv(0), gi.args());
    }
    else
    {
        p = gi.args();

        if (*p == '"')
        {
            p++;
            p[strlen(p)-1] = 0;
        }

        Kots_strncpy(text, p, sizeof(text));
    }

    //SWB - format the message for special identifiers
    for (j = 0, p = &text[0]; *p != '\0' && j < sizeof(text); j++, p++)
    {
        if (*p == '%')
        {
            switch (*(++p))
            {
            case 'a':
            case 'A':
                Kots_ReplaceText(text, --p, va("%i", ent->client->pers.inventory[ARMOR_INDEX]), 2);
                break;

            case 'h':
            case 'H':
                Kots_ReplaceText(text, --p, va("%i", ent->health), 2);
                break;

            case 'w':
            case 'W':
                if (ent->client->pers.weapon)
                    Kots_ReplaceText(text, --p, ent->client->pers.weapon->pickup_name, 2);
                else
                    Kots_ReplaceText(text, --p, "No Weapon", 2);

                break;

            case 'm':
            case 'M':
                {
                    int ammo = 0;
                    if (ent->client->pers.kots_persist.is_loggedin && ent->client->pers.weapon)
                    {
                        //if the weapon is grenades then get the inventory amount for the weapon
                        if (ent->client->pers.weapon->weapmodel == WEAP_GRENADES)
                            ammo = ent->client->pers.inventory[ITEM_INDEX(ent->client->pers.weapon)];

                        //otherwise if it's not a sabre get the amount for the ammo of the weapon
                        else if (ent->client->pers.weapon->weapmodel != WEAP_SABRE)
                            ammo = ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo))];
                    }

                    Kots_ReplaceText(text, --p, va("%i", ammo), 2);
                }
                break;

            case 'r':
            case 'R':
                if (ent->client->pers.kots_persist.is_loggedin && ent->character->rune)
                    Kots_ReplaceText(text, --p, ent->character->rune->name, 2);
                else
                    Kots_ReplaceText(text, --p, "No Rune", 2);

                break;

            case 'f':
            case 'F':
                if (ent->client->pers.kots_persist.is_loggedin)
                    Kots_ReplaceText(text, --p, va("%i", ent->client->ps.stats[STAT_FRAGS]), 2);
                else
                    Kots_ReplaceText(text, --p, "0", 2);

                break;

            case 's':
            case 'S':
                if (ent->client->pers.kots_persist.is_loggedin)
                    Kots_ReplaceText(text, --p, va("%i", ent->client->ps.stats[STAT_KOTS_SCORE]), 2);
                else
                    Kots_ReplaceText(text, --p, "0", 2);

                break;

            case 'p':
            case 'P':
                {
                    edict_t *targ = ent;
                    edict_t *best = NULL;
                    int count = 0;
                    float bd = 0, d = 0;
                    vec3_t  dist;

                    while ((targ = findradius(targ, ent->s.origin, 1024)) != NULL)
                    {
                        if (++count > MAX_EDICTS)
                        {
                            targ = NULL;
                            break;
                        }

                        if (!targ->inuse)
                            continue;
                        if (!targ->item)
                            continue;

                        //only print location for permanent items
                        if (targ->spawnflags & DROPPED_PLAYER_ITEM || targ->spawnflags & DROPPED_ITEM)
                            continue;

                        VectorSubtract(ent->s.origin, targ->s.origin, dist);
                        d = VectorLength(dist);

                        if (d < bd || bd == 0)
                        {
                            bd = d;
                            best = targ;
                        }
                    }


                    if (!best)
                        Kots_ReplaceText(text, --p, "No Nearby Items", 2);
                    else
                        Kots_ReplaceText(text, --p, best->item->pickup_name, 2);

                }

                break;

            case '%': //escape double percent signs
                Kots_ReplaceText(text, p--, "", 1);
                break;

            default:
                p--;
                break;
            }
        }
    }

    //SWB - since we modify the text before appending the name now
    if (team)
        Kots_strncpy(text, va("(%s): %s", ent->client->pers.netname, text), sizeof(text));
    else
        Kots_strncpy(text, va("%s: %s", ent->client->pers.netname, text), sizeof(text));

    // don't let text be too long for malicious reasons
    if (strlen(text) > 150)
        text[150] = 0;

    strcat(text, "\n");

    if (flood_msgs->value) {
        cl = ent->client;

        if (level.time < cl->flood_locktill) {
            gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
                (int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
        if (cl->flood_when[i] &&
            level.time - cl->flood_when[i] < flood_persecond->value) {
            cl->flood_locktill = level.time + flood_waitdelay->value;
            gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
                (int)flood_waitdelay->value);
            return;
        }
        cl->flood_whenhead = (cl->flood_whenhead + 1) %
            (sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
        cl->flood_when[cl->flood_whenhead] = level.time;
    }

    if (dedicated->value)
    {
        gi.cprintf(NULL, PRINT_CHAT, "%s", text);
    }

    for (j = 1; j <= game.maxclients; j++)
    {
        other = &g_edicts[j];
        if (!other->inuse)
            continue;
        if (!other->client)
            continue;

        //SWB - admins hear all team conversations
        if (team && !other->client->pers.kots_persist.is_admin)
        {
            //SWB - we're always on our own team
            if (!OnSameTeam(ent, other) && ent != other)
                continue;
        }

        //SWB - print title or spectator in front
        if (ent->character && ent->client->pers.kots_persist.is_loggedin)
        {
            if (ent->character->title[0] != '\0')
                gi.cprintf(other, PRINT_HIGH, "%s ", ent->character->title);
        }
        else
        {
            gi.cprintf(other, PRINT_HIGH, "(spectator) ");
        }

        gi.cprintf(other, PRINT_CHAT, "%s", text);
    }
}

void Cmd_PlayerList_f(edict_t *ent)
{
    int i;
    char st[80];
    char text[1400];
    edict_t *e2;

    // connect time, ping, score, name
    *text = 0;
    for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
        if (!e2->inuse)
            continue;

        Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
            (level.framenum - e2->client->resp.enterframe) / 600,
            ((level.framenum - e2->client->resp.enterframe) % 600)/10,
            e2->client->ping,
            e2->client->resp.score,
            e2->client->pers.netname,
            e2->client->resp.spectator ? " (spectator)" : "");
        if (strlen(text) + strlen(st) > sizeof(text) - 50) {
            sprintf(text+strlen(text), "And more...\n");
            gi.cprintf(ent, PRINT_HIGH, "%s", text);
            return;
        }
        strcat(text, st);
    }
    gi.cprintf(ent, PRINT_HIGH, "%s", text);
}


/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
    char    *cmd;

    if (!ent->client)
        return;     // not fully in game yet

    cmd = gi.argv(0);

    //SWB
    //check if command is part of kots
    if (Kots_Command(ent, cmd))
        return;

    if (Q_stricmp (cmd, "players") == 0)
    {
        Cmd_Players_f (ent);
        return;
    }
    if (Q_stricmp (cmd, "say") == 0)
    {
        Cmd_Say_f (ent, false, false);
        return;
    }
    if (Q_stricmp (cmd, "say_team") == 0)
    {
        Cmd_Say_f (ent, true, false);
        return;
    }
    if (Q_stricmp (cmd, "score") == 0)
    {
        Cmd_Score_f (ent);
        return;
    }
    if (Q_stricmp (cmd, "help") == 0)
    {
        Cmd_Help_f (ent);
        return;
    }

    if (level.intermissiontime)
        return;

    if (Q_stricmp (cmd, "use") == 0)
        Cmd_Use_f (ent);
    else if (Q_stricmp (cmd, "drop") == 0)
        Cmd_Drop_f (ent);
    else if (Q_stricmp (cmd, "give") == 0)
        Cmd_Give_f (ent);
    else if (Q_stricmp (cmd, "god") == 0)
        Cmd_God_f (ent);
    else if (Q_stricmp (cmd, "notarget") == 0)
        Cmd_Notarget_f (ent);
    else if (Q_stricmp (cmd, "noclip") == 0)
        Cmd_Noclip_f (ent);
    else if (Q_stricmp (cmd, "inven") == 0)
        Cmd_Inven_f (ent);
    else if (Q_stricmp (cmd, "invnext") == 0)
        SelectNextItem (ent, -1);
    else if (Q_stricmp (cmd, "invprev") == 0)
        SelectPrevItem (ent, -1);
    else if (Q_stricmp (cmd, "invnextw") == 0)
        SelectNextItem (ent, IT_WEAPON);
    else if (Q_stricmp (cmd, "invprevw") == 0)
        SelectPrevItem (ent, IT_WEAPON);
    else if (Q_stricmp (cmd, "invnextp") == 0)
        SelectNextItem (ent, IT_POWERUP);
    else if (Q_stricmp (cmd, "invprevp") == 0)
        SelectPrevItem (ent, IT_POWERUP);
    else if (Q_stricmp (cmd, "invuse") == 0)
        Cmd_InvUse_f (ent);
    else if (Q_stricmp (cmd, "invdrop") == 0)
        Cmd_InvDrop_f (ent);
    else if (Q_stricmp (cmd, "weapprev") == 0)
        Cmd_WeapPrev_f (ent);
    else if (Q_stricmp (cmd, "weapnext") == 0)
        Cmd_WeapNext_f (ent);
    else if (Q_stricmp (cmd, "weaplast") == 0)
        Cmd_WeapLast_f (ent);
    else if (Q_stricmp (cmd, "kill") == 0)
        Cmd_Kill_f (ent);
    else if (Q_stricmp (cmd, "putaway") == 0)
        Cmd_PutAway_f (ent);
    else if (Q_stricmp (cmd, "wave") == 0)
        Cmd_Wave_f (ent);
    else if (Q_stricmp(cmd, "playerlist") == 0)
        Cmd_PlayerList_f(ent);
    else    // anything that doesn't match a command will be a chat
        Cmd_Say_f (ent, false, true);
}

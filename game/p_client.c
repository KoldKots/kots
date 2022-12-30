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
#include "kots_logging.h"
#include "kots_mute.h"
#include "kots_monster.h"
#include "kots_maplist.h"
#include "kots_hud.h"
#include "kots_runes.h"
#include "kots_utils.h"
#include "kots_conpersist.h"
#include "kots_server.h"
#include "kots_admin.h"
#include "kots_motd.h"
#include "kots_hook.h"
#include "kots_commands.h"


void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void SP_misc_teleporter_dest (edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
    edict_t *spot;
    vec3_t  d;

    spot = NULL;

    while(1)
    {
        spot = G_Find(spot, FOFS(classname), "info_player_start");
        if (!spot)
            return;
        if (!spot->targetname)
            continue;
        VectorSubtract(self->s.origin, spot->s.origin, d);
        if (VectorLength(d) < 384)
        {
            if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
            {
//              gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
                self->targetname = spot->targetname;
            }
            return;
        }
    }
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
    edict_t *spot;

    if(Q_stricmp(level.mapname, "security") == 0)
    {
        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->s.origin[0] = 188 - 64;
        spot->s.origin[1] = -164;
        spot->s.origin[2] = 80;
        spot->targetname = "jail3";
        spot->s.angles[1] = 90;

        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->s.origin[0] = 188 + 64;
        spot->s.origin[1] = -164;
        spot->s.origin[2] = 80;
        spot->targetname = "jail3";
        spot->s.angles[1] = 90;

        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->s.origin[0] = 188 + 128;
        spot->s.origin[1] = -164;
        spot->s.origin[2] = 80;
        spot->targetname = "jail3";
        spot->s.angles[1] = 90;

        return;
    }
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
    if (!coop->value)
        return;
    if(Q_stricmp(level.mapname, "security") == 0)
    {
        // invoke one of our gross, ugly, disgusting hacks
        self->think = SP_CreateCoopSpots;
        self->nextthink = level.time + FRAMETIME;
    }
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
    if (!deathmatch->value)
    {
        G_FreeEdict (self);
        return;
    }
    SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
    if (!coop->value)
    {
        G_FreeEdict (self);
        return;
    }

    if((Q_stricmp(level.mapname, "jail2") == 0)   ||
       (Q_stricmp(level.mapname, "jail4") == 0)   ||
       (Q_stricmp(level.mapname, "mine1") == 0)   ||
       (Q_stricmp(level.mapname, "mine2") == 0)   ||
       (Q_stricmp(level.mapname, "mine3") == 0)   ||
       (Q_stricmp(level.mapname, "mine4") == 0)   ||
       (Q_stricmp(level.mapname, "lab") == 0)     ||
       (Q_stricmp(level.mapname, "boss1") == 0)   ||
       (Q_stricmp(level.mapname, "fact3") == 0)   ||
       (Q_stricmp(level.mapname, "biggun") == 0)  ||
       (Q_stricmp(level.mapname, "space") == 0)   ||
       (Q_stricmp(level.mapname, "command") == 0) ||
       (Q_stricmp(level.mapname, "power2") == 0) ||
       (Q_stricmp(level.mapname, "strike") == 0))
    {
        // invoke one of our gross, ugly, disgusting hacks
        self->think = SP_FixCoopSpots;
        self->nextthink = level.time + FRAMETIME;
    }
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
    // player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
    char        *info;

    if (!ent->client)
        return false;

    info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
    if (info[0] == 'f' || info[0] == 'F')
        return true;
    return false;
}

qboolean IsNeutral (edict_t *ent)
{
    char        *info;

    //SWB
    //assume neutral if no client
    if (!ent->client)
        return true;

    info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
    if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
        return true;
    return false;
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
    int         mod;
    char        *message;
    char        *message2;
    char        targ_name[100];
    qboolean    ff;
    qboolean    quad=false;
    qboolean    headshot=false;

    //SWB - get targ name needs to be done separately in case there's also an attacker
    Kots_strncpy(targ_name, Kots_CharacterGetFullName(self), 100);

    if (coop->value && attacker->client)
        meansOfDeath |= MOD_FRIENDLY_FIRE;

    if (deathmatch->value || coop->value)
    {
        ff = meansOfDeath & MOD_FRIENDLY_FIRE;
        mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
        message = NULL;
        message2 = "";

        switch (mod)
        {
        case MOD_SUICIDE:
            message = "suicides";
            break;
        case MOD_FALLING:
            message = "cratered";
            break;
        case MOD_CRUSH:
            message = "was squished";
            break;
        case MOD_WATER:
            message = "sank like a rock";
            break;
        case MOD_SLIME:
            message = "melted";
            break;
        case MOD_LAVA:
            message = "did a back flip into the lava";
            break;
        case MOD_EXPLOSIVE:
        case MOD_BARREL:
            message = "blew up";
            break;
        case MOD_EXIT:
            message = "found a way out";
            break;
        case MOD_TARGET_LASER:
            message = "saw the light";
            break;
        case MOD_TARGET_BLASTER:
            message = "got blasted";
            break;
        case MOD_BOMB:
        case MOD_SPLASH:
        case MOD_TRIGGER_HURT:
            message = "was in the wrong place";
            break;
        }
        if (attacker == self)
        {
            switch (mod)
            {
            case MOD_HELD_GRENADE:
                message = "tried to put the pin back in";
                break;
            case MOD_HG_SPLASH:
            case MOD_G_SPLASH:
                if (IsNeutral(self))
                    message = "tripped on its own grenade";
                else if (IsFemale(self))
                    message = "tripped on her own grenade";
                else
                    message = "tripped on his own grenade";
                break;
            case MOD_R_SPLASH:
                if (IsNeutral(self))
                    message = "blew itself up";
                else if (IsFemale(self))
                    message = "blew herself up";
                else
                    message = "blew himself up";
                break;
            case MOD_BFG_BLAST:
                message = "should have used a smaller gun";
                break;
                
            //kots suicides
            case MOD_SMITE:
                if (IsNeutral(self))
                    message = "smited itself to death";
                else if (IsFemale(self))
                    message = "smited herself to death";
                else
                    message = "smited himself to death";
                break;
            case MOD_EXPACK:
                if (IsNeutral(self))
                    message = "found its own expack";
                else if (IsFemale(self))
                    message = "found her own expack";
                else
                    message = "found his own expack";
                break;
            case MOD_CONFLAG:
                if (IsNeutral(self))
                    message = "blew itself up";
                else if (IsFemale(self))
                    message = "blew herself up";
                else
                    message = "blew himself up";
                break;
            case MOD_FAKEHEALTH:
                if (IsNeutral(self))
                    message = "found its own fake health";
                else if (IsFemale(self))
                    message = "found her own fake health";
                else
                    message = "found his own fake health";
                break;
            case MOD_FAKESHARD:
                if (IsNeutral(self))
                    message = "found its own fake shard";
                else if (IsFemale(self))
                    message = "found her own fake shard";
                else
                    message = "found his own fake shard";
                break;
            case MOD_FLAIL:
                message = "flailed to death";
                break;
            case MOD_MINE:
                if (IsNeutral(self))
                    message = "found its own mine";
                else if (IsFemale(self))
                    message = "found her own mine";
                else
                    message = "found his own mine";
                break;
            case MOD_SPIRAL:
                if (IsNeutral(self))
                    message = "got caught in its own spiral tornado";
                else if (IsFemale(self))
                    message = "got caught in her own spiral tornado";
                else
                    message = "got caught in his own spiral tornado";
                break;
            case MOD_BIDERADIUS:
                if (IsNeutral(self))
                    message = "unleashed its fury upon itself";
                else if (IsFemale(self))
                    message = "unleashed her fury upon herself";
                else
                    message = "unleashed his fury upon himself";
                break;
            case MOD_SPREEQUIT:
                if (IsNeutral(self))
                {
                    if (spreewar.warent == self)
                        message = "forfeited its spreewar";
                    else
                        message = "forfeited its spree";
                }
                else if (IsFemale(self))
                {
                    if (spreewar.warent == self)
                        message = "forfeited her spreewar";
                    else
                        message = "forfeited its spree";
                }
                else
                {
                    if (spreewar.warent == self)
                        message = "forfeited his spreewar";
                    else
                        message = "forfeited her spree";
                }
                break;
            default:
                if (IsNeutral(self))
                    message = "killed itself";
                else if (IsFemale(self))
                    message = "killed herself";
                else
                    message = "killed himself";
                break;
            }
        }
        if (message)
        {
            //SWB - added thanks to kills
            edict_t *thanksto = Kots_CharacterGetThanksTo(self);

            if (thanksto)
                gi.bprintf (PRINT_MEDIUM, "%s %s thanks to %s.\n", targ_name, message, Kots_CharacterGetFullName(thanksto));
            else
                gi.bprintf (PRINT_MEDIUM, "%s %s.\n", targ_name, message);

            //if (deathmatch->value && self->client)
                //self->client->resp.score--;
            self->enemy = NULL;
            return;
        }

        self->enemy = attacker;

        //SWB
        if (attacker && attacker->character)
        {
            if(attacker->client)
            {
                if(attacker->client->quad_framenum > level.framenum) // Aldarn - quad messages
                    quad = true;
                if(self->client)
                {
                    if(self->client->pers.headshot_death) // Aldarn - headshot messages
                        headshot = true;
                }
            }
            if(quad)
            {
                switch (mod)
                {
                case MOD_BLASTER:
                    message = "was launched into the abyss by";
                    message2 = "'s quad blaster cannon";
                    break;
                case MOD_SHOTGUN:
                    message = "was dissipated around the arena by";
                    message2 = "'s quad shotgun shell";
                    break;
                case MOD_SSHOTGUN:
                    message = "is now in a thousand different evidence bags due to";
                    message2 = "'s quad super shotgun";
                    break;
                case MOD_MACHINEGUN:
                    message = "was torn to pieces by";
                    message2 = "'s quad machinegun";
                    break;
                case MOD_CHAINGUN:
                    message = "was cut in quarters by";
                    message2 = "'s quad chaingun";
                    break;
                case MOD_GRENADE:
                    message = "is now a large crater as a result of";
                    message2 = "'s quad grenade launcher";
                    break;
                case MOD_G_SPLASH:
                    message = "face could do with some serious reconstruction from";
                    message2 = "'s quad shrapnel";
                    break;
                case MOD_ROCKET:
                    message = "imploded from";
                    message2 = "'s quad rocket";
                    break;
                case MOD_R_SPLASH:
                    message = "didn't stand a change of dodging";
                    message2 = "'s quad rocket";
                    break;
                case MOD_HYPERBLASTER:
                    message = "can now be written as a chemical formulae because of";
                    message2 = "'s quad hyperblaster";
                    break;
                case MOD_RAILGUN:
                    message = "was derailed by";
                    message2 = "'s quad slugathon";
                    break;
                case MOD_BFG_LASER:
                    message = "saw the pretty lights from";
                    message2 = "'s quad BFG, and nothing else after that!";
                    break;
                case MOD_BFG_BLAST:
                    message = "was annihilated by";
                    message2 = "'s quad BFG blast";
                    break;
                case MOD_BFG_EFFECT:
                    message = "couldn't build a lead bunker in time to hide from";
                    message2 = "'s quad BFG";
                    break;
                case MOD_HANDGRENADE:
                    message = "was knocked out by";
                    message2 = "'s quad handgrenade";
                    break;
                case MOD_HG_SPLASH:
                    message = "saw";
                    message2 = "'s quad handgrenade, but it was too late";
                    break;
                case MOD_HELD_GRENADE:
                    message = "feels";
                    message2 = "'s pain 4 times over";
                    break;
                case MOD_TELEFRAG:
                    message = "tried to invade";
                    message2 = "'s personal space (quad? lol)";
                    break;

                //SWB
                //Kots Obituaries
                case MOD_POISON:
                    message = "was poisoned to death by";
                    break;
                case MOD_EXPACK:
                    message = "was sent packing by";
                    break;
                case MOD_EMPATHY:
                    message = "felt";
                    message2 = "'s pain";
                    break;
                case MOD_BIDE:
                    message = "took the full wrath of";
                    message2 = "'s bided fury";
                    break;
                case MOD_BIDERADIUS:
                    message = "got a little too close to";
                    message2 = "'s bided fury";
                    break;
                case MOD_CONFLAG:
                    message = "was reduced to non-existence by";
                    message2 = "'s quad conflagration";
                    break;
                case MOD_FAKEHEALTH:
                    message = "was killed by";
                    message2 = "'s fake health";
                    break;
                case MOD_FAKESHARD:
                    message = "was killed by";
                    message2 = "'s fake shard";
                    break;
                case MOD_FLAIL:
                    message = "was mutilated by";
                    message2 = "'s quad flailing about";
                    break;
                case MOD_GRAPPLE:
                    message = "was gutted by";
                    message2 = "'s quad grappling hook";
                    break;
                case MOD_MINE:
                    message = "didn't see";
                    message2 = "'s mine";
                    break;
                case MOD_SPIRAL:
                    message = "got caught in";
                    message2 = "'s spiral tornado";
                    break;
                case MOD_SABRE:
                    message = "was disemboweled by";
                    message2 = "'s quad sabre";
                    break;
                case MOD_BOSSEXPLODE:
                    message = "was taken down along with";
                    break;
                case MOD_SPREEQUIT:
                    if (IsNeutral(self))
                    {
                        if (spreewar.warent == self)
                            message = "forfeited its spreewar to";
                        else
                            message = "forfeited its spree to";
                    }
                    else if (IsFemale(self))
                    {
                        if (spreewar.warent == self)
                            message = "forfeited her spreewar to";
                        else
                            message = "forfeited her spree to";
                    }
                    else
                    {
                        if (spreewar.warent == self)
                            message = "forfeited his spreewar to";
                        else
                            message = "forfeited his spree to";
                    }
                    break;
                }
            }
            else if(headshot)
            {
                switch (mod)
                {
                case MOD_BLASTER:
                    message = "face melted from";
                    message2 = "'s blaster";
                    break;
                case MOD_SHOTGUN:
                    message = "chewed on";
                    message2 = "'s shells";
                    break;
                case MOD_SSHOTGUN:
                    message = "put two barrels of";
                    message2 = "'s super shotgun in their mouth";
                    break;
                case MOD_MACHINEGUN:
                    message = "bit";
                    message2 = "'s bullet";
                    break;
                case MOD_CHAINGUN:
                    message = "has a slight headache from";
                    message2 = "'s chaingun";
                    break;
                case MOD_GRENADE:
                    message = "was corked up by";
                    message2 = "'s grenade";
                    break;
                case MOD_G_SPLASH:
                    message = "what the fuck";
                    message2 = "'s what the fuck";
                    break;
                case MOD_ROCKET:
                    message = "digested";
                    message2 = "'s rocket";
                    break;
                case MOD_R_SPLASH:
                    message = "what da shit";
                    message2 = "'s what da shit";
                    break;
                case MOD_HYPERBLASTER:
                    message = "facial structure now resembles";
                    message2 = "'s hyperblaster";
                    break;
                case MOD_RAILGUN:
                    message = "saw a spiral of blue light from";
                    message2 = "'s railgun";
                    break;
                case MOD_BFG_LASER:
                    message = "took laser eye surgery from";
                    message2 = "'s BFG";
                    break;
                case MOD_BFG_BLAST:
                    message = "what the crap";
                    message2 = "'s what the crap";
                    break;
                case MOD_BFG_EFFECT:
                    message = "i have no idea how";
                    message2 = " did that";
                    break;
                case MOD_HANDGRENADE:
                    message = "choked on";
                    message2 = "'s handgrenade";
                    break;
                case MOD_HG_SPLASH:
                    message = "dunno";
                    message2 = "'s dunno";
                    break;
                case MOD_HELD_GRENADE:
                    message = "huh";
                    message2 = "'s huh";
                    break;
                case MOD_TELEFRAG:
                    message = "lol";
                    message2 = "'s lol";
                    break;

                //SWB
                //Kots Obituaries
                case MOD_POISON:
                    message = "was poisoned to death by";
                    break;
                case MOD_EXPACK:
                    message = "was sent packing by";
                    break;
                case MOD_EMPATHY:
                    message = "felt";
                    message2 = "'s pain";
                    break;
                case MOD_BIDE:
                    message = "took the full wrath of";
                    message2 = "'s bided fury TO THEIR FACE";
                    break;
                case MOD_BIDERADIUS:
                    message = "got a little too close to";
                    message2 = "'s bided fury";
                    break;
                case MOD_CONFLAG:
                    message = "'s face was reduced to non-existence by";
                    message2 = "'s quad conflagration";
                    break;
                case MOD_FAKEHEALTH:
                    message = "was killed by";
                    message2 = "'s fake health";
                    break;
                case MOD_FLAIL:
                    message = "was mutilated by";
                    message2 = "'s quad flailing about";
                    break;
                case MOD_GRAPPLE:
                    message = "has a hole in their head from";
                    message2 = "'s grappling hook";
                    break;
                case MOD_MINE:
                    message = "didn't see";
                    message2 = "'s mine";
                    break;
                case MOD_SPIRAL:
                    message = "got caught in";
                    message2 = "'s spiral tornado";
                    break;
                case MOD_SABRE:
                    message = "had their head cleaved in two by";
                    message2 = "'s sabre";
                    break;
                case MOD_BOSSEXPLODE:
                    message = "was taken down along with";
                    break;
                case MOD_SPREEQUIT:
                    if (IsNeutral(self))
                    {
                        if (spreewar.warent == self)
                            message = "forfeited its spreewar to";
                        else
                            message = "forfeited its spree to";
                    }
                    else if (IsFemale(self))
                    {
                        if (spreewar.warent == self)
                            message = "forfeited her spreewar to";
                        else
                            message = "forfeited her spree to";
                    }
                    else
                    {
                        if (spreewar.warent == self)
                            message = "forfeited his spreewar to";
                        else
                            message = "forfeited his spree to";
                    }
                    break;
                }
            }
            else
            {
                switch (mod)
                {
                case MOD_BLASTER:
                    message = "was blasted by";
                    break;
                case MOD_SHOTGUN:
                    message = "was gunned down by";
                    break;
                case MOD_SSHOTGUN:
                    message = "was blown away by";
                    message2 = "'s super shotgun";
                    break;
                case MOD_MACHINEGUN:
                    message = "was machinegunned by";
                    break;
                case MOD_CHAINGUN:
                    message = "was cut in half by";
                    message2 = "'s chaingun";
                    break;
                case MOD_GRENADE:
                    message = "was popped by";
                    message2 = "'s grenade";
                    break;
                case MOD_G_SPLASH:
                    message = "was shredded by";
                    message2 = "'s shrapnel";
                    break;
                case MOD_ROCKET:
                    message = "ate";
                    message2 = "'s rocket";
                    break;
                case MOD_R_SPLASH:
                    message = "almost dodged";
                    message2 = "'s rocket";
                    break;
                case MOD_HYPERBLASTER:
                    message = "was melted by";
                    message2 = "'s hyperblaster";
                    break;
                case MOD_RAILGUN:
                    message = "was railed by";
                    break;
                case MOD_BFG_LASER:
                    message = "saw the pretty lights from";
                    message2 = "'s BFG";
                    break;
                case MOD_BFG_BLAST:
                    message = "was disintegrated by";
                    message2 = "'s BFG blast";
                    break;
                case MOD_BFG_EFFECT:
                    message = "couldn't hide from";
                    message2 = "'s BFG";
                    break;
                case MOD_HANDGRENADE:
                    message = "caught";
                    message2 = "'s handgrenade";
                    break;
                case MOD_HG_SPLASH:
                    message = "didn't see";
                    message2 = "'s handgrenade";
                    break;
                case MOD_HELD_GRENADE:
                    message = "feels";
                    message2 = "'s pain";
                    break;
                case MOD_TELEFRAG:
                    message = "tried to invade";
                    message2 = "'s personal space";
                    break;

                //SWB
                //Kots Obituaries
                case MOD_POISON:
                    message = "was poisoned to death by";
                    break;
                case MOD_EXPACK:
                    message = "was sent packing by";
                    break;
                case MOD_EMPATHY:
                    message = "felt";
                    message2 = "'s pain";
                    break;
                case MOD_BIDE:
                    message = "took the full wrath of";
                    message2 = "'s bided fury";
                    break;
                case MOD_BIDERADIUS:
                    message = "got a little too close to";
                    message2 = "'s bided fury";
                    break;
                case MOD_CONFLAG:
                    message = "blown to hell by";
                    message2 = "'s conflagration";
                    break;
                case MOD_FAKEHEALTH:
                    message = "was killed by";
                    message2 = "'s fake health";
                    break;
                case MOD_FAKESHARD:
                    message = "was killed by";
                    message2 = "'s fake shard";
                    break;
                case MOD_FLAIL:
                    message = "was killed by";
                    message2 = "'s flailing about";
                    break;
                case MOD_GRAPPLE:
                    message = "was impaled by";
                    message2 = "'s grappling hook";
                    break;
                case MOD_MINE:
                    message = "didn't see";
                    message2 = "'s mine";
                    break;
                case MOD_SPIRAL:
                    message = "got caught in";
                    message2 = "'s spiral tornado";
                    break;
                case MOD_SABRE:
                    message = "was disemboweled by";
                    message2 = "'s sabre";
                    break;
                case MOD_BOSSEXPLODE:
                    message = "was taken down along with";
                    break;
                case MOD_SPREEQUIT:
                    if (IsNeutral(self))
                    {
                        if (spreewar.warent == self)
                            message = "forfeited its spreewar to";
                        else
                            message = "forfeited its spree to";
                    }
                    else if (IsFemale(self))
                    {
                        if (spreewar.warent == self)
                            message = "forfeited her spreewar to";
                        else
                            message = "forfeited her spree to";
                    }
                    else
                    {
                        if (spreewar.warent == self)
                            message = "forfeited his spreewar to";
                        else
                            message = "forfeited his spree to";
                    }
                    break;
                }
            }

            if (!message && attacker->character && self->character)
                message = "was killed by";

            if (message)
            {
                //SWB
                gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", targ_name, message, Kots_CharacterGetFullName(attacker), message2);

                if (deathmatch->value && attacker->client)
                {
                    if (!ff)
                        attacker->client->resp.score++;
                    //else
                        //attacker->client->resp.score--;
                }
                return;
            }
        }
    }

    {
        //SWB - added thanks to kills
        edict_t *thanksto = Kots_CharacterGetThanksTo(self);

        if (thanksto)
            gi.bprintf (PRINT_MEDIUM, "%s died thanks to %s.\n", targ_name, Kots_CharacterGetFullName(thanksto));
        else
            gi.bprintf (PRINT_MEDIUM,"%s died.\n", targ_name);

        //if (deathmatch->value && self->client)
            //self->client->resp.score--;
    }
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
    //gitem_t       *item;
    edict_t     *drop;
    //qboolean  quad;
    //float     spread;

    if (!deathmatch->value)
        return;

    //SWB - force quad drop and don't drop item
    /*
    item = self->client->pers.weapon;
    if (! self->client->pers.inventory[self->client->ammo_index] )
        item = NULL;
    if (item && (strcmp (item->pickup_name, "Blaster") == 0))
        item = NULL;

    if (!((int)(dmflags->value) & DF_QUAD_DROP))
        quad = false;
    else
        quad = (self->client->quad_framenum > (level.framenum + 10));

    if (item && quad)
        spread = 22.5;
    else
        spread = 0.0;

    if (item)
    {
        self->client->v_angle[YAW] -= spread;
        drop = Drop_Item (self, item);
        self->client->v_angle[YAW] += spread;
        drop->spawnflags = DROPPED_PLAYER_ITEM;
    }

    */
    if (self->client->quad_framenum > (level.framenum + 10))
    {
        //SWB - don't drop quad rage quad damages
        float quadtime = (self->client->quad_framenum - level.framenum) * FRAMETIME;

        //if we have both then don't count the time from rage
        if (self->character->next_quadrage > level.time)
        {
            float ragetime = (self->character->next_quadrage - level.time);

            if (ragetime > quadtime)
                quadtime = 0;

            else
            {
                quadtime -= ragetime;

                //ensure there is at least a second of quad in addition to normal quad damage
                //in order to prevent a quad rage quad from dropping
                if (quadtime < 1.0)
                    quadtime = 0;
            }
        }

        if (quadtime >= 1.0)
        {
            self->client->v_angle[YAW] -= 22.5;
            drop = Drop_Item (self, FindItemByClassname ("item_quad"));
            self->client->v_angle[YAW] += 22.5;
            drop->spawnflags |= DROPPED_PLAYER_ITEM;

            drop->touch = Touch_Item;
            drop->nextthink = level.time + quadtime;
            drop->think = G_FreeEdict;
        }
    }

    //SWB - In KOTS2007 you can die with invuln so we need to be able to drop it
    //if the player died with more than a second of invuln
    if (self->client->invincible_framenum > (level.framenum + 10))
    {
        //SWB - calculate remaining invuln time
        float invulntime = (self->client->invincible_framenum - level.framenum) * FRAMETIME;

        //SWB - remove spawn invulnerability from the time
        if (self->character->next_candamage && self->character->next_candamage > level.time)
            invulntime -= (self->character->next_candamage - level.time);

        if (invulntime >= 1.0)
        {
            self->client->v_angle[YAW] += 22.5;
            drop = Drop_Item(self, FindItemByClassname("item_invulnerability"));
            self->client->v_angle[YAW] -= 22.5;
            drop->spawnflags |= DROPPED_PLAYER_ITEM;

            drop->touch = Touch_Item;
            drop->nextthink = level.time + invulntime;
            drop->think = G_FreeEdict;
        }
    }
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
    vec3_t      dir;

    if (attacker && attacker != world && attacker != self)
    {
        VectorSubtract (attacker->s.origin, self->s.origin, dir);
    }
    else if (inflictor && inflictor != world && inflictor != self)
    {
        VectorSubtract (inflictor->s.origin, self->s.origin, dir);
    }
    else
    {
        self->client->killer_yaw = self->s.angles[YAW];
        return;
    }

    if (dir[0])
        self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
    else {
        self->client->killer_yaw = 0;
        if (dir[1] > 0)
            self->client->killer_yaw = 90;
        else if (dir[1] < 0)
            self->client->killer_yaw = -90;
    }
    if (self->client->killer_yaw < 0)
        self->client->killer_yaw += 360;
    

}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    int     n;

    VectorClear (self->avelocity);

    self->takedamage = DAMAGE_YES;
    self->movetype = MOVETYPE_TOSS;

    self->s.modelindex2 = 0;    // remove linked weapon model

    self->s.angles[0] = 0;
    self->s.angles[2] = 0;

    self->s.sound = 0;
    self->client->weapon_sound = 0;

    self->maxs[2] = -8;

//  self->solid = SOLID_NOT;
    self->svflags |= SVF_DEADMONSTER;

    if (!self->deadflag)
    {
        self->client->respawn_time = level.time + 1.0;
        LookAtKiller (self, inflictor, attacker);
        self->client->ps.pmove.pm_type = PM_DEAD;
        ClientObituary (self, inflictor, attacker);

        TossClientWeapon (self);
        if (deathmatch->value && !self->client->menu)
            Cmd_Help_f (self);      // show scores

        // clear inventory
        // this is kind of ugly, but it's how we want to handle keys in coop
        for (n = 0; n < game.num_items; n++)
        {
            if (coop->value && itemlist[n].flags & IT_KEY)
                self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
            self->client->pers.inventory[n] = 0;
        }
    }

    // remove powerups
    self->client->quad_framenum = 0;
    self->client->invincible_framenum = 0;
    self->client->breather_framenum = 0;
    self->client->enviro_framenum = 0;
    self->flags &= ~FL_POWER_ARMOR;

    if (self->health < -40)
    {   // gib
        gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n= 0; n < 4; n++)
            ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowClientHead (self, damage);

        self->takedamage = DAMAGE_NO;
    }
    else
    {   // normal death
        if (!self->deadflag)
        {
            static int i;

            i = (i+1)%3;
            // start a death animation
            self->client->anim_priority = ANIM_DEATH;
            if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
            {
                self->s.frame = FRAME_crdeath1-1;
                self->client->anim_end = FRAME_crdeath5;
            }
            else switch (i)
            {
            case 0:
                self->s.frame = FRAME_death101-1;
                self->client->anim_end = FRAME_death106;
                break;
            case 1:
                self->s.frame = FRAME_death201-1;
                self->client->anim_end = FRAME_death206;
                break;
            case 2:
                self->s.frame = FRAME_death301-1;
                self->client->anim_end = FRAME_death308;
                break;
            }
            gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
        }
    }

    self->deadflag = DEAD_DEAD;

    gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
    gitem_t     *item;
    kots_persist_t persist = client->pers.kots_persist;

    memset (&client->pers, 0, sizeof(client->pers));

    //SWB
    //Default to sabre now
    item = FindItem("Sabre");
    client->pers.selected_item = ITEM_INDEX(item);
    client->pers.inventory[client->pers.selected_item] = 1;

    client->pers.weapon = item;

    client->pers.health         = 100;
    client->pers.max_health     = 100;

    client->pers.max_bullets    = 200;
    client->pers.max_shells     = 100;
    client->pers.max_rockets    = 50;
    client->pers.max_grenades   = 50;
    client->pers.max_cells      = 200;
    client->pers.max_slugs      = 50;

    client->pers.connected = true;
    client->pers.kots_persist = persist;
}


void InitClientResp (gclient_t *client)
{
    memset (&client->resp, 0, sizeof(client->resp));
    client->resp.enterframe = level.framenum;
    client->resp.coop_respawn = client->pers;
}


qboolean IsClientIdInUse(int client_id)
{
    int i;
    edict_t *player = g_edicts + 1;

    for (i = 0; i < game.maxclients; i++, player++)
    {
        if (player->inuse && player->client->pers.kots_persist.client_id == client_id)
            return true;
    }

    return false;
}

/*
==============
InitKotsPersist

This is only called only once when a client initially connects and before InitClientPersistant
==============
*/
void InitKotsPersist (gclient_t *client)
{
    static int client_id = 0;


    client_id++; //increment client id every time we init a new client

    //0 is a reserved client id
    if (client_id == 0)
        client_id++;

    //ensure that the client id isn't already in use (EXTREMELY RARE)
    while (IsClientIdInUse(client_id))
        client_id++;

    client->pers.kots_persist.connect_time = time(NULL);
    client->pers.kots_persist.client_id = client_id;
    client->pers.kots_persist.is_loggedin = false;
    client->pers.kots_persist.is_admin = false;
    client->pers.kots_persist.is_muted = false;
    client->pers.kots_persist.monster_vote = MAPVOTE_NONE;
    client->pers.kots_persist.monster_vote = KOTS_MONSTERS_MIXED;
    client->pers.kots_persist.rune_id = 0;
    client->pers.kots_persist.ip_address[0] = '\0';
    client->pers.kots_persist.intermission_screen = KOTS_INTERMISSION_SCORE;

    //initialize default settings
    client->pers.kots_persist.using_knock = true;
    client->pers.kots_persist.using_deflect = true;
    client->pers.kots_persist.using_poison = true;
    client->pers.kots_persist.using_quadrage = true;
    client->pers.kots_persist.using_lasersight = false;
    client->pers.kots_persist.using_laserball = false;
    client->pers.kots_persist.hg_resettimer = true;
    client->pers.kots_persist.hg_fixedspeed = true;
    client->pers.kots_persist.hg_shortfuse = true;
    
    //not technically a persistant value but only needs to be set once at connection
    client->chase_mode = KOTS_SPECTATOR_MODE_FREE;
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
    int     i;
    edict_t *ent;
    qboolean need_persist = false;

    //SWB - end any spreewars going on BEFORE we log characters out
    if (spreewar.warent != NULL)
        Kots_CharacterEndSpree(spreewar.warent, true);

    //SWB - Log information before logging clients out
    Kots_LogOnDisconnect();

    for (i=0 ; i<game.maxclients ; i++)
    {
        ent = &g_edicts[1+i];
        if (!ent->inuse)
            continue;

        //SWB - this can happen if gamemap is used to change maps
        if (ent->character->is_loggedin)
        {
            //save the rune for the next map
            ent->client->pers.kots_persist.rune_id = (ent->character->rune ? ent->character->rune->id : 0);
            ent->character->rune = NULL;

            Kots_CharacterLogout(ent, false, false);
            Kots_CharacterClearEdicts(ent);
 
            //we want to stay logged in for the next map so reset this to true
            game.clients[i].pers.kots_persist.is_loggedin = true;
            need_persist = true; //signal that we should persist info after this a gamemap probably occurred
        }


        //SWB - reset character info
        Kots_CharacterInit(ent);

        game.clients[i].pers.health = ent->health;
        game.clients[i].pers.health = 999;
        game.clients[i].pers.max_health = ent->max_health;
        game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
        if (coop->value)
            game.clients[i].pers.score = ent->client->resp.score;
    }

    //clear all previous persistant data and save all current data
    if (need_persist)
    {
        Kots_ClearAllConPersistData();
        Kots_SaveAllConPersistData();
    }
}

void FetchClientEntData (edict_t *ent)
{
    ent->health = ent->client->pers.health;
    ent->max_health = ent->client->pers.max_health;
    ent->flags |= ent->client->pers.savedFlags;
    if (coop->value)
        ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float   PlayersRangeFromSpot (edict_t *spot)
{
    edict_t *player;
    float   bestplayerdistance = 0;
    vec3_t  v;
    int     n;
    float   playerdistance;
    qboolean found = false;


    bestplayerdistance = 9999999;

    for (n = 1; n <= maxclients->value; n++)
    {
        player = &g_edicts[n];

        if (!player->inuse)
            continue;

        if (player->health <= 0)
            continue;

        found = true;
        VectorSubtract (spot->s.origin, player->s.origin, v);
        playerdistance = VectorLength (v);

        if (playerdistance < bestplayerdistance)
            bestplayerdistance = playerdistance;
    }

    //SWB - if not found we return 0
    //otherwise the first spawn is always going to be used in spawn farthest
    if (found)
        return bestplayerdistance;
    else
        return 0;
}

/*
================
SelectClosestDeathmatchSpawnPoint

return the closest spawn point to an entity
================
*/
edict_t *SelectClosestDeathmatchSpawnPoint(edict_t *ent)
{
    float   playerdistance;
    vec3_t  v;
    float   bestdistance = 0;
    edict_t *bestspot = NULL;
    edict_t *spot = NULL;

    while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
    {
        VectorSubtract(spot->s.origin, ent->s.origin, v);
        playerdistance = VectorLength(v);

        if (bestspot == NULL || playerdistance < bestdistance)
        {
            bestspot = spot;
            bestdistance = playerdistance;
        }
    }

    return bestspot;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
//SWB - Modified to account for monster distances
//      Modified to take an entity as a spot to ignore
edict_t *SelectRandomDeathmatchSpawnPoint (edict_t *ignore)
{
    edict_t *spot, *spot1, *spot2;
    int     count = 0;
    int     selection;
    float   range, range1, range2;

    spot = NULL;
    range1 = range2 = 99999;
    spot1 = spot2 = NULL;

    while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
    {
        //SWB - Ignore the specified spot and don't add it to the count
        if (spot == ignore)
            continue;

        count++;
        range = PlayersRangeFromSpot(spot);
        if (range < range1)
        {
            range1 = range;
            spot1 = spot;
        }
        else if (range < range2)
        {
            range2 = range;
            spot2 = spot;
        }
    }

    if (!count)
        return NULL;

    if (count <= 2)
    {
        spot1 = spot2 = NULL;
    }
    else
        count -= 2;

    selection = rand() % count;

    spot = NULL;
    do
    {
        spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");

        //SWB - Ignore the specified spot
        if (spot == ignore)
            continue;

        if (spot == spot1 || spot == spot2)
            selection++;
    } while(selection--);

    return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/

//SWB - Modified to account for monster distances
//      Modified to take an entity as a spot to ignore
edict_t *SelectFarthestDeathmatchSpawnPoint (edict_t *ignore)
{
    edict_t *bestspot;
    float   bestdistance, playerdistance, monsterdistance;
    edict_t *spot;


    spot = NULL;
    bestspot = NULL;
    bestdistance = 0;
    while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
    {
        //SWB - Ignore spot specified
        if (spot == ignore)
            continue;

        playerdistance = PlayersRangeFromSpot (spot);
        monsterdistance = Kots_MonstersRangeFromSpot(spot);

        //if monster distance is closer then use it to compare
        if (monsterdistance && monsterdistance < playerdistance)
            playerdistance = monsterdistance;

        if (playerdistance > bestdistance)
        {
            bestspot = spot;
            bestdistance = playerdistance;
        }
    }

    if (bestspot)
    {
        return bestspot;
    }

    // if there is a player just spawned on each and every start spot
    // we have no choice to turn one into a telefrag meltdown
    return SelectRandomDeathmatchSpawnPoint(ignore);
}

//SWB - Modified to take an entity
//      If an entity is passed then the closest spawn point to them is ignored
edict_t *SelectDeathmatchSpawnPoint (edict_t *ent)
{
    edict_t *ignore = NULL;

    //SWB - if an entity was passed in find the closest spot and ignore it
    if (ent)
        ignore = SelectClosestDeathmatchSpawnPoint(ent);

    if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
        return SelectFarthestDeathmatchSpawnPoint (ignore);
    else
        return SelectRandomDeathmatchSpawnPoint (ignore);
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
    int     index;
    edict_t *spot = NULL;
    char    *target;

    index = ent->client - game.clients;

    // player 0 starts in normal player spawn point
    if (!index)
        return NULL;

    spot = NULL;

    // assume there are four coop spots at each spawnpoint
    while (1)
    {
        spot = G_Find (spot, FOFS(classname), "info_player_coop");
        if (!spot)
            return NULL;    // we didn't have enough...

        target = spot->targetname;
        if (!target)
            target = "";
        if ( Q_stricmp(game.spawnpoint, target) == 0 )
        {   // this is a coop spawn point for one of the clients here
            index--;
            if (!index)
                return spot;        // this is it
        }
    }


    return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void    SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
    edict_t *spot = NULL;

    if (deathmatch->value)
        spot = SelectDeathmatchSpawnPoint (NULL);
    else if (coop->value)
        spot = SelectCoopSpawnPoint (ent);

    // find a single player start spot
    if (!spot)
    {
        while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
        {
            if (!game.spawnpoint[0] && !spot->targetname)
                break;

            if (!game.spawnpoint[0] || !spot->targetname)
                continue;

            if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
                break;
        }

        if (!spot)
        {
            if (!game.spawnpoint[0])
            {   // there wasn't a spawnpoint without a target, so use any
                spot = G_Find (spot, FOFS(classname), "info_player_start");
            }
            if (!spot)
                gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
        }
    }

    VectorCopy (spot->s.origin, origin);
    origin[2] += 9;
    VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
    int     i;
    edict_t *ent;

    level.body_que = 0;
    for (i=0; i<BODY_QUEUE_SIZE ; i++)
    {
        ent = G_Spawn();
        ent->classname = "bodyque";
    }
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    int n;

    if (self->health < -40)
    {
        gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n= 0; n < 4; n++)
            ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        self->s.origin[2] -= 48;
        ThrowClientHead (self, damage);
        self->takedamage = DAMAGE_NO;
    }
}

void CopyToBodyQue (edict_t *ent)
{
    edict_t     *body;

    // grab a body que and cycle to the next one
    body = &g_edicts[(int)maxclients->value + level.body_que + 1];
    level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

    // FIXME: send an effect on the removed body

    gi.unlinkentity (ent);

    //SWB - attempt to gib the existing body
    if (body->die)
    {
        //ensure that we can gib any corpse
        body->health = body->gib_health;
        if (body->health > -40)
            body->health = -40;

        //attempt to gib it
        body->die(body, body, body, 0, body->s.origin);
    }

    gi.unlinkentity (body);
    body->s = ent->s;
    body->s.number = body - g_edicts;

    body->svflags = ent->svflags;
    body->svflags &= ~SVF_NOCLIENT; //remove noclient attributes from bodies
    VectorCopy (ent->mins, body->mins);
    VectorCopy (ent->maxs, body->maxs);
    VectorCopy (ent->absmin, body->absmin);
    VectorCopy (ent->absmax, body->absmax);
    VectorCopy (ent->size, body->size);
    body->solid = ent->solid;
    body->clipmask = ent->clipmask;
    body->owner = ent->owner;
    body->movetype = ent->movetype;

    //SWB - set the dead flag for the body and keep the health info
    body->deadflag = DEAD_DEAD;
    body->gib_health = ent->gib_health;
    body->health = ent->health;
    VectorCopy(ent->velocity, body->velocity); //Usually should be all 0's but if it got gibbed it will take on the heads velocity so clear it
    VectorCopy(ent->avelocity, body->avelocity);

    //SWB - try to keep monster die functions for gibbing
    if ((ent->svflags & SVF_MONSTER) && ent->die)
    {
        body->die = ent->die;

        //store the old classname for resurrecting
        body->targetname = ent->classname;
    }
    else
    {
        body->die = body_die;
        body->targetname = NULL;
    }

    body->takedamage = DAMAGE_YES;

    gi.linkentity (body);
}


void respawn (edict_t *self)
{
    if (deathmatch->value || coop->value)
    {
        // spectator's don't leave bodies
        if (self->movetype != MOVETYPE_NOCLIP)
            CopyToBodyQue (self);
        self->svflags &= ~SVF_NOCLIENT;
        PutClientInServer (self);

        // add a teleportation effect
        self->s.event = EV_PLAYER_TELEPORT;

        // hold in place briefly
        self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        self->client->ps.pmove.pm_time = 14;

        self->client->respawn_time = level.time;

        //SWB
        Kots_CharacterRespawn(self);

        return;
    }

    // restart the entire server
    gi.AddCommandString ("menu_loadgame\n");
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn (edict_t *ent)
{
    int i, numspec;

    // if the user wants to become a spectator, make sure he doesn't
    // exceed max_spectators

    if (ent->client->pers.spectator) {
        char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
        if (*spectator_password->string && 
            strcmp(spectator_password->string, "none") && 
            strcmp(spectator_password->string, value)) {
            gi.cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
            ent->client->pers.spectator = false;
            gi.WriteByte (svc_stufftext);
            gi.WriteString ("spectator 0\n");
            gi.unicast(ent, true);
            return;
        }

        // count spectators
        for (i = 1, numspec = 0; i <= maxclients->value; i++)
            if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
                numspec++;

        if (false) {//numspec >= maxspectators->value) {
            gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.\n");
            ent->client->pers.spectator = false;
            // reset his spectator var
            gi.WriteByte (svc_stufftext);
            gi.WriteString ("spectator 0\n");
            gi.unicast(ent, true);
            return;
        }
    } else {
        // he was a spectator and wants to join the game
        // he must have the right password
        char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
        if (*password->string && strcmp(password->string, "none") && 
            strcmp(password->string, value)) {
            gi.cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
            ent->client->pers.spectator = true;
            gi.WriteByte (svc_stufftext);
            gi.WriteString ("spectator 1\n");
            gi.unicast(ent, true);
            return;
        }
    }

    // clear client on respawn
    ent->client->resp.score = ent->client->pers.score = 0;

    ent->svflags &= ~SVF_NOCLIENT;
    PutClientInServer (ent);

    // add a teleportation effect
    if (!ent->client->pers.spectator)  {
        // send effect
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_LOGIN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        // hold in place briefly
        ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        ent->client->ps.pmove.pm_time = 14;
    }

    ent->client->respawn_time = level.time;

    if (ent->client->pers.spectator) 
    {
        gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
    }
    else
    {
        gi.bprintf (PRINT_HIGH, "%s started their reign\n", ent->client->pers.netname);
    }
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
    vec3_t  mins = {-16, -16, -24};
    vec3_t  maxs = {16, 16, 32};
    int     index;
    vec3_t  spawn_origin, spawn_angles;
    gclient_t   *client;
    int     i;
    client_persistant_t saved;
    client_respawn_t    resp;

    // find a spawn point
    // do it before setting health back up, so farthest
    // ranging doesn't count this client
    SelectSpawnPoint (ent, spawn_origin, spawn_angles);

    index = ent-g_edicts-1;
    client = ent->client;

    // deathmatch wipes most client data every spawn
    if (deathmatch->value)
    {
        char        userinfo[MAX_INFO_STRING];

        resp = client->resp;
        memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
        InitClientPersistant (client);
        ClientUserinfoChanged (ent, userinfo);
    }
    else if (coop->value)
    {
//      int         n;
        char        userinfo[MAX_INFO_STRING];

        resp = client->resp;
        memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
        // this is kind of ugly, but it's how we want to handle keys in coop
//      for (n = 0; n < game.num_items; n++)
//      {
//          if (itemlist[n].flags & IT_KEY)
//              resp.coop_respawn.inventory[n] = client->pers.inventory[n];
//      }
        resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
        resp.coop_respawn.helpchanged = client->pers.helpchanged;
        client->pers = resp.coop_respawn;
        ClientUserinfoChanged (ent, userinfo);
        if (resp.score > client->pers.score)
            client->pers.score = resp.score;
    }
    else
    {
        memset (&resp, 0, sizeof(resp));
    }

    // clear everything but the persistant data
    saved = client->pers;
    memset (client, 0, sizeof(*client));
    client->pers = saved;
    if (client->pers.health <= 0)
        InitClientPersistant(client);
    client->resp = resp;

    // copy some data from the client to the entity
    FetchClientEntData (ent);

    // clear entity values
    ent->groundentity = NULL;
    ent->client = &game.clients[index];
    ent->takedamage = DAMAGE_AIM;
    ent->movetype = MOVETYPE_WALK;
    ent->viewheight = 22;
    ent->inuse = true;
    ent->classname = "player";
    ent->mass = 200;
    ent->solid = SOLID_BBOX;
    ent->deadflag = DEAD_NO;
    ent->air_finished = level.time + 12;
    ent->clipmask = MASK_PLAYERSOLID;
    ent->model = "players/male/tris.md2";
    ent->pain = player_pain;
    ent->die = player_die;
    ent->waterlevel = 0;
    ent->watertype = 0;
    ent->flags &= ~FL_NO_KNOCKBACK;
    ent->svflags &= ~SVF_DEADMONSTER;

    VectorCopy (mins, ent->mins);
    VectorCopy (maxs, ent->maxs);
    VectorClear (ent->velocity);

    // clear playerstate values
    memset (&ent->client->ps, 0, sizeof(client->ps));

    client->ps.pmove.origin[0] = spawn_origin[0]*8;
    client->ps.pmove.origin[1] = spawn_origin[1]*8;
    client->ps.pmove.origin[2] = spawn_origin[2]*8;

    if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
    {
        client->ps.fov = 90;
    }
    else
    {
        client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
        if (client->ps.fov < 1)
            client->ps.fov = 90;
        else if (client->ps.fov > 160)
            client->ps.fov = 160;
    }

    client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

    // clear entity state values
    ent->s.effects = 0;
    ent->s.modelindex = 255;        // will use the skin specified model
    ent->s.modelindex2 = 255;       // custom gun model
    // sknum is player num and weapon number
    // weapon number will be added in changeweapon
    ent->s.skinnum = ent - g_edicts - 1;

    ent->s.frame = 0;
    VectorCopy (spawn_origin, ent->s.origin);
    ent->s.origin[2] += 1;  // make sure off ground
    VectorCopy (ent->s.origin, ent->s.old_origin);

    // set the delta angle
    for (i=0 ; i<3 ; i++)
    {
        client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
    }

    ent->s.angles[PITCH] = 0;
    ent->s.angles[YAW] = spawn_angles[YAW];
    ent->s.angles[ROLL] = 0;
    VectorCopy (ent->s.angles, client->ps.viewangles);
    VectorCopy (ent->s.angles, client->v_angle);

    // spawn a spectator
    if (client->pers.spectator) {
        client->chase_target = NULL;

        client->resp.spectator = true;

        ent->movetype = MOVETYPE_NOCLIP;
        ent->solid = SOLID_NOT;
        ent->svflags |= SVF_NOCLIENT;
        ent->client->ps.gunindex = 0;
        gi.linkentity (ent);
        return;
    } else
        client->resp.spectator = false;

    if (!KillBox (ent))
    {   // could't spawn in?
    }

    gi.linkentity (ent);

    // force the current weapon up
    client->newweapon = client->pers.weapon;
    ChangeWeapon (ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
    G_InitEdict (ent);

    InitClientResp (ent->client);

    // locate ent at a spawn point
    PutClientInServer (ent);

    if (level.intermissiontime)
    {
        MoveClientToIntermission (ent);
    }
    else
    {
        // send effect
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_LOGIN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
    }

    gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

    // make sure all view stuff is valid
    ClientEndServerFrame (ent);

    //SWB - check if the ip address is banned
    if (Kots_CheckIpBan(ent))
    {
        gi.bprintf(PRINT_HIGH, "%s is banned.\n", ent->client->pers.netname);
        Kots_Admin_KickUser(NULL, ent);
        return;
    }

    //SWB
    //start the character stuff
    Kots_CharacterClientBegin(ent);
    Kots_PrintMotd(ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
    int     i;

    ent->client = game.clients + (ent - g_edicts - 1);

    if (deathmatch->value)
    {
        ClientBeginDeathmatch (ent);
        return;
    }

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (ent->inuse == true)
    {
        // the client has cleared the client side viewangles upon
        // connecting to the server, which is different than the
        // state when the game is saved, so we need to compensate
        // with deltaangles
        for (i=0 ; i<3 ; i++)
            ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
    }
    else
    {
        // a spawn point will completely reinitialize the entity
        // except for the persistant data that was initialized at
        // ClientConnect() time
        G_InitEdict (ent);
        ent->classname = "player";
        InitClientResp (ent->client);
        PutClientInServer (ent);
    }

    if (level.intermissiontime)
    {
        MoveClientToIntermission (ent);
    }
    else
    {
        // send effect if in a multiplayer game
        if (game.maxclients > 1)
        {
            gi.WriteByte (svc_muzzleflash);
            gi.WriteShort (ent-g_edicts);
            gi.WriteByte (MZ_LOGIN);
            gi.multicast (ent->s.origin, MULTICAST_PVS);

            gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
        }
    }

    // make sure all view stuff is valid
    ClientEndServerFrame (ent);
    
    //SWB
    //start the character stuff
    Kots_CharacterClientBegin(ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
    char    *s, *s2;
    int     playernum;

    // check for malformed or illegal info strings
    if (!Info_Validate(userinfo))
    {
        strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
    }

    // set name
    s = Info_ValueForKey (userinfo, "name");
    strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

    //SWB
    //Spectator mode is default and players require login
    // set spectator
    //s = Info_ValueForKey (userinfo, "spectator");
    // spectators are only supported in deathmatch
    if (!ent->character || !ent->character->is_loggedin) //if (deathmatch->value && *s && strcmp(s, "0"))
        ent->client->pers.spectator = true;
    else
        ent->client->pers.spectator = false;

    // set skin
    s = Info_ValueForKey (userinfo, "skin");

    playernum = ent-g_edicts-1;

    // combine name and skin into a configstring
    gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

    // fov
    if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
    {
        ent->client->ps.fov = 90;
    }
    else
    {
        ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
        if (ent->client->ps.fov < 1)
            ent->client->ps.fov = 90;
        else if (ent->client->ps.fov > 160)
            ent->client->ps.fov = 160;
    }

    // handedness
    s = Info_ValueForKey (userinfo, "hand");
    if (strlen(s))
    {
        ent->client->pers.hand = atoi(s);
    }

    //SWB
    s = Info_ValueForKey(userinfo, "name");
    if (ent->character && ent->character->is_loggedin)
    {
        if (strcmp(s, ent->character->name) != 0)
        {
            char cmd[128];

            gi.cprintf(ent, PRINT_HIGH, "You must logout if you want to change your character.\n");
            Info_SetValueForKey(userinfo, "name", ent->character->name);
            strcpy(ent->client->pers.netname, ent->character->name);
            sprintf(cmd, "name \"%s\"", ent->character->name);
            stuffcmd(ent, cmd);
        }
    }
    else
    {

        //SWB
        s2 = Info_ValueForKey(ent->client->pers.userinfo, "name");
        if (ent->inuse && *ent->client->pers.userinfo != '\0' && Q_stricmp(s, s2) != 0)
        {
            gi.bprintf(PRINT_HIGH, "%s changed name to %s.\n", s2, s);
            stuffcmd(ent, va("exec %s.cfg\n", s));
        }
    }

    // save off the userinfo in case we want to check something later
    strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
    char    *value;
    char    *ip_address;
    /*
    int     j;
    edict_t *cl_ent;
    char    *ip = NULL;
    char    *ip1 = NULL;
    char    *ip2 = NULL;
    char    delim[] = ":";
    */

    // check to see if they are on the banned IP list
    value = Info_ValueForKey (userinfo, "ip");
    ip_address = value;
    if (SV_FilterPacket(value)) {
        Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
        return false;
    }

    // Aldarn - check for duplicate ip - FIXME: crashed at strcmp??? - crashes when 3rd person connects //
    /*
    for (j=0 ; j<game.maxclients ; j++)
    {
        cl_ent = g_edicts + 1 + j;
        if (!cl_ent->inuse)
            continue;

        ip = Info_ValueForKey (cl_ent->client->pers.userinfo, "ip");
        ip1 = strtok(value,delim);
        ip2 = strtok(ip,delim);

        if(strcmp(ip1,ip2) == 0)
        {
            Info_SetValueForKey(userinfo, "rejmsg", "Error - Your ip is already in use on this server.");
            return false;
        }

        ip1 = NULL;
        ip2 = NULL;
        ip = NULL;
    }
    */
    // End //

    // check for a spectator
    value = Info_ValueForKey (userinfo, "spectator");
    if (deathmatch->value && *value && strcmp(value, "0") == 0) {
        int i, numspec;

        if (*spectator_password->string && 
            strcmp(spectator_password->string, "none") && 
            strcmp(spectator_password->string, value)) {
            Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
            return false;
        }

        // count spectators
        for (i = numspec = 0; i < maxclients->value; i++)
            if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
                numspec++;

        if (numspec >= maxspectators->value) {
            Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
            return false;
        }
    } else {
        // check for a password
        value = Info_ValueForKey (userinfo, "password");
        if (*password->string && strcmp(password->string, "none") && 
            strcmp(password->string, value)) {
            Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
            return false;
        }
    }


    // they can connect
    ent->client = game.clients + (ent - g_edicts - 1);

    //SWB - Initialize the persistent kots info
    InitKotsPersist(ent->client);
    
    if (ip_address)
    {
        char *port = strstr(ip_address, ":");
        int length = (port ? port - ip_address + 1 : 16);

        if (length > 16)
            length = 16;

        Kots_strncpy(ent->client->pers.kots_persist.ip_address, ip_address, length);

        //check if the ip address is banned 
        if (Kots_CheckIpBan(ent))
        {
            Info_SetValueForKey(userinfo, "rejmsg", "Your IP address is banned. Please contact an admin.");
            return false;
        }
    }

    //Attempt to load from connection persist data
    Kots_LoadConPersistData(ent);

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (ent->inuse == false)
    {
        // clear the respawning variables
        InitClientResp (ent->client);
        if (!game.autosaved || !ent->client->pers.weapon)
            InitClientPersistant (ent->client);
    }
    
    ClientUserinfoChanged (ent, userinfo);

    if (game.maxclients > 1)
        gi.dprintf ("%s connected\n", ent->client->pers.netname);


    ent->svflags = 0; // make sure we start with known default
    ent->client->pers.connected = true;

    //SWB - check if the client is muted
    Kots_MuteCheck(ent);

    return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
    int     playernum;

    if (!ent->client)
    {
        memset(&ent->s, 0, sizeof(ent->s));
        return;
    }

    //SWB - Logout if logged in
    if (ent->character->is_loggedin)
    {
        Kots_CharacterLogout(ent, false, true);
        Kots_CharacterClearEdicts(ent);
    }

    //SWB - clear entity state information otherwise R1Q2 will complain
    memset(&ent->s, 0, sizeof(ent->s));

    
    //SWB - Reset the character info
    Kots_CharacterInit(ent);

    //SWB - clear clients queued items
    Kots_ClearSlowPrints(ent);
    Kots_ClearQueuedSounds(ent);

    gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

    // send effect
    gi.WriteByte (svc_muzzleflash);
    gi.WriteShort (ent-g_edicts);
    gi.WriteByte (MZ_LOGOUT);
    gi.multicast (ent->s.origin, MULTICAST_PVS);

    gi.unlinkentity (ent);
    ent->s.modelindex = 0;
    ent->solid = SOLID_NOT;
    ent->inuse = false;
    ent->classname = "disconnected";
    ent->client->pers.connected = false;

    playernum = ent-g_edicts-1;
    gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t *pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
    if (pm_passent->health > 0)
        return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
    else
        return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
    int v,i;
    v = 0;
    for (i=0 ; i<c ; i++)
        v+= ((byte *)b)[i];
    return v;
}
void PrintPmove (pmove_t *pm)
{
    unsigned    c1, c2;

    c1 = CheckBlock (&pm->s, sizeof(pm->s));
    c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
    Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
    gclient_t   *client;
    edict_t *other;
    int     i, j;
    pmove_t pm;

    level.current_entity = ent;
    client = ent->client;

    if (level.intermissiontime)
    {
        client->ps.pmove.pm_type = PM_FREEZE;
        // can exit intermission after five seconds
        //if (level.time > level.intermissiontime + 5.0 
            //&& (ucmd->buttons & BUTTON_ANY) )
            //level.exitintermission = true;

        //track button presses for intermission scoreboard
        client->oldbuttons = client->buttons;
        client->buttons = ucmd->buttons;

        //SWB - acknowledge scoreboard and go to the next
        if ((ent->client->buttons & BUTTON_ATTACK) && !(ent->client->oldbuttons & BUTTON_ATTACK))
        {
            if (!ent->client->intermission_clicked)
            {
                ent->client->intermission_clicked = (!ent->client->intermission_clicked ? true : false);
                level.intermission_dirty = true;
            }
        }
        return;
    }

    pm_passent = ent;

    if (ent->client->chase_target) {

        client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
        client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
        client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

    } else {

        // set up for pmove
        memset (&pm, 0, sizeof(pm));

        if (ent->movetype == MOVETYPE_NOCLIP)
            client->ps.pmove.pm_type = PM_SPECTATOR;
        else if (ent->s.modelindex != 255)
            client->ps.pmove.pm_type = PM_GIB;
        else if (ent->deadflag)
            client->ps.pmove.pm_type = PM_DEAD;
        else
            client->ps.pmove.pm_type = PM_NORMAL;

        client->ps.pmove.gravity = sv_gravity->value;
        pm.s = client->ps.pmove;

        for (i=0 ; i<3 ; i++)
        {
            pm.s.origin[i] = ent->s.origin[i]*8;
            pm.s.velocity[i] = ent->velocity[i]*8;
        }

        if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
        {
            pm.snapinitial = true;
    //      gi.dprintf ("pmove changed!\n");
        }

        pm.cmd = *ucmd;

        pm.trace = PM_trace;    // adds default parms
        pm.pointcontents = gi.pointcontents;

        //SWB
        Kots_CharacterPreThink(ent, &pm);

        // perform a pmove
        gi.Pmove (&pm);
        
        //SWB
        Kots_CharacterThink(ent, &pm);

        // save results of pmove
        client->ps.pmove = pm.s;
        client->old_pmove = pm.s;

        for (i=0 ; i<3 ; i++)
        {
            ent->s.origin[i] = pm.s.origin[i]*0.125;
            ent->velocity[i] = pm.s.velocity[i]*0.125;
        }

        VectorCopy (pm.mins, ent->mins);
        VectorCopy (pm.maxs, ent->maxs);

        client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
        client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
        client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

        if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
        {
            //SWB
            if (ent->character->cur_dexterity < 2)
            {
                gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
                PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
            }
        }

        ent->viewheight = pm.viewheight;
        ent->waterlevel = pm.waterlevel;
        ent->watertype = pm.watertype;
        ent->groundentity = pm.groundentity;
        if (pm.groundentity)
            ent->groundentity_linkcount = pm.groundentity->linkcount;

        if (ent->deadflag)
        {
            client->ps.viewangles[ROLL] = 40;
            client->ps.viewangles[PITCH] = -15;
            client->ps.viewangles[YAW] = client->killer_yaw;
        }
        else
        {
            VectorCopy (pm.viewangles, client->v_angle);
            VectorCopy (pm.viewangles, client->ps.viewangles);
        }
                    
        //we have to move the player after client think so we do it here  
        if (ent->client->ctf_grapple)
        {
            if (ent->character->cur_dexterity >= 3)
                CTFGrapplePull(ent->client->ctf_grapple);
            else //if for some reason we no longer have enough dex
                CTFPlayerResetGrapple(ent);
        }

        gi.linkentity (ent);

        if (ent->movetype != MOVETYPE_NOCLIP)
            G_TouchTriggers (ent);

        // touch other objects
        for (i=0 ; i<pm.numtouch ; i++)
        {
            other = pm.touchents[i];
            for (j=0 ; j<i ; j++)
                if (pm.touchents[j] == other)
                    break;
            if (j != i)
                continue;   // duplicated
            if (!other->touch)
                continue;
            other->touch (other, ent, NULL, NULL);
        }

    }

    client->oldbuttons = client->buttons;
    client->buttons = ucmd->buttons;
    client->latched_buttons |= client->buttons & ~client->oldbuttons;

    // save light level the player is standing on for
    // monster sighting AI
    ent->light_level = ucmd->lightlevel;

    // fire weapon from final position if needed
    if (client->latched_buttons & BUTTON_ATTACK)
    {
        if (client->resp.spectator) {

            client->latched_buttons = 0;

            //SWB - allow 3 chase modes
            switch (client->chase_mode)
            {
            case KOTS_SPECTATOR_MODE_EYES:
                gi.cprintf(ent, PRINT_MEDIUM, "Free look mode.\n");
                client->chase_mode = KOTS_SPECTATOR_MODE_FREE;
                client->chase_target = NULL;
                client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                break;
            case KOTS_SPECTATOR_MODE_CHASE:
                gi.cprintf(ent, PRINT_MEDIUM, "In-eyes camera mode.\n");
                client->chase_mode = KOTS_SPECTATOR_MODE_EYES;
                UpdateChaseCam(ent);
                break;
            case KOTS_SPECTATOR_MODE_FREE:
            default:
                client->chase_mode = KOTS_SPECTATOR_MODE_CHASE;
                GetChaseTarget(ent);
                
                //may not have anyone to chase so we have to check again
                if (client->chase_mode == KOTS_SPECTATOR_MODE_CHASE)
                    gi.cprintf(ent, PRINT_MEDIUM, "Chase camera mode.\n");

                break;
            }

        } else if (!client->weapon_thunk) {
            client->weapon_thunk = true;
            Think_Weapon (ent);
        }
    }

    if (client->resp.spectator) {
        if (ucmd->upmove >= 10) {
            if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
                client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
                if (client->chase_target)
                    ChaseNext(ent);
                else
                    GetChaseTarget(ent);
            }
        } else
            client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
    }

    // update chase cam if being followed
    for (i = 1; i <= maxclients->value; i++) {
        other = g_edicts + i;
        if (other->inuse && other->client->chase_target == ent)
            UpdateChaseCam(other);
    }


    //SWB - PMenu changes
    if (client->menudirty && client->menutime <= level.time) {
        PMenu_Do_Update(ent);
        gi.unicast (ent, true);
        client->menutime = level.time;
        client->menudirty = false;
    }
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
    gclient_t   *client;
    int         buttonMask;

    if (level.intermissiontime)
        return;

    client = ent->client;


    if (deathmatch->value &&
        client->pers.spectator != client->resp.spectator &&
        (level.time - client->respawn_time) >= 5) {
        spectator_respawn(ent);
        return;
    }

    // run weapon animations if it hasn't been done by a ucmd_t
    if (!client->weapon_thunk && !client->resp.spectator)
        Think_Weapon (ent);
    else
        client->weapon_thunk = false;

    if (ent->deadflag)
    {
        // wait for any button just going down
        if ( level.time > client->respawn_time)
        {
            // in deathmatch, only wait for attack button
            if (deathmatch->value)
                buttonMask = BUTTON_ATTACK;
            else
                buttonMask = -1;

            if ( ( client->latched_buttons & buttonMask ) ||
                (deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
            {
                respawn(ent);
                client->latched_buttons = 0;
            }
        }
        return;
    }

    // add player trail so monsters can follow
    if (!deathmatch->value)
        if (!visible (ent, PlayerTrail_LastSpot() ) )
            PlayerTrail_Add (ent->s.old_origin);

    client->latched_buttons = 0;
}

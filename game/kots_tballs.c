#include "kots_tballs.h"
#include "kots_runes.h"
#include "kots_hook.h"
#include "kots_utils.h"
#include "kots_abilities.h"

//NOTE: TBall code taken from KOTS Orbit
//      Original Author: Mother


//*************************************************************************************
//*************************************************************************************
// Function: KOTSTeleport
//*************************************************************************************
//*************************************************************************************

qboolean Kots_Teleport( edict_t *other )
{
    int     i;
    edict_t *dest;
    edict_t *player;

    if ( other->character->spree >= 25 )
        return false;

    dest = SelectDeathmatchSpawnPoint(other);

    if ( !dest )
        return false;

    gi.unlinkentity (other);

    //SWB - reset our own grapple
    CTFPlayerResetGrapple(other);

    //SWB - remove any grapples that may have latched on
    player = g_edicts + 1;
    for (i = 0; i < game.maxclients; i++)
    {
        if (!player->inuse || !player->character->is_loggedin)
            continue;

        //if the grapple is latched on to the person teleporting then remove it
        if (player->client->ctf_grapple && player->client->ctf_grapple->enemy == other)
            CTFPlayerResetGrapple(player);
    }

    VectorCopy( dest->s.origin, other->s.origin );
    VectorCopy( dest->s.origin, other->s.old_origin );

    other->s.origin[2] += 10;

    // clear the velocity and hold them in place briefly
    VectorClear( other->velocity );
    if (other->client)
    {
        //SWB - Prevent falling damage from tballing
        VectorClear(other->client->oldvelocity);

        other->client->ps.pmove.pm_time   = 160 >> 3;       // hold time
        other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
    }

    other->s.event = EV_PLAYER_TELEPORT;

    // set angles
    if (other->client)
        for( i = 0; i < 3; i++ )
            other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT( dest->s.angles[i] - other->client->resp.cmd_angles[i] );

    VectorClear( other->s.angles );

    if (other->client)
    {
        VectorClear( other->client->ps.viewangles );
        VectorClear( other->client->v_angle );
    }

    KillBox( other );

    gi.linkentity( other );

    return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: T_RadiusTeleport
//*************************************************************************************
//*************************************************************************************

void T_RadiusTeleport( edict_t *inflictor, edict_t *attacker, float damage, 
                       edict_t *ignore, float radius )
{
    int     x;
    int     tball_self = 0;
    float     points;
    vec3_t  v;
    edict_t *ent = NULL;
    edict_t *tballed[MAX_CLIENTS];
    int     count = 0;
    char    attacker_name[100];

    Kots_strncpy(attacker_name, Kots_CharacterGetFullName(attacker), 100);

    while ( ( ent = findradius( ent, inflictor->s.origin, radius ) ) != NULL )
    {
        if (count >= MAX_CLIENTS)
            break;

        if ( ent == ignore || !ent->character )
            continue;

        if ( !ent->takedamage || !ent->inuse || ent->health <= 0 )
            continue;

        if (!ent->character->is_loggedin)
            continue;

        //no teleporting monsters... for now
        if (ent->svflags & SVF_MONSTER)
            continue;

        VectorAdd     ( ent->mins, ent->maxs, v   );
        VectorMA      ( ent->s.origin, 0.5, v, v  );
        VectorSubtract( inflictor->s.origin, v, v );

        points = damage - 0.5 * VectorLength( v );

        if ( ent == attacker )
        {
            tball_self = 1;
            points = points * 0.5;
        }

        if ( points <= 0 )
            continue;

        if ( !CanDamage( ent, inflictor ) )
            continue;

        /* No idea what this was
        if ( ent->client->pers.kots_stoptball )
        {
            ent->client->pers.inventory[ ITEM_INDEX( FindItem( "tball" ) ) ]++;

            gi.bprintf( PRINT_HIGH, "%s stopped %s's tball\n", ent->client->pers.netname, attacker->client->pers.netname );
            return;
        }
        */

        if (count < MAX_CLIENTS)
            tballed[count++] = ent;
    }

    for ( x = 0; x < count; x++ )
    {
        ent = (edict_t *)tballed[x];

        //only attacker drops rune
        if (ent == attacker)
        {
            //drop any runes before tballing
            if (ent->character->rune)
                Kots_RuneDrop(ent);
        }

        if (!Kots_Teleport(ent))
            continue;

        gi.sound( ent, CHAN_ITEM, gi.soundindex ("misc/tele1.wav"), 1, ATTN_NORM, 0 );

        if (ent != attacker)
        {
            gi.bprintf(PRINT_HIGH, "%s was teleported by %s\n", Kots_CharacterGetFullName(ent), attacker_name);
            ++attacker->character->teleports;

            if (!tball_self)
            {
                ++attacker->character->exp;
                ++attacker->character->score;
            }
        }
        else
        { 
            gi.bprintf( PRINT_HIGH,"%s teleports away\n", attacker_name);
        }
    }
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSRadiusTeleport
//*************************************************************************************
//*************************************************************************************

void Kots_RadiusTeleport( edict_t *ent )
{
    T_RadiusTeleport( ent, ent, 160, ent, 100 );
}

//*************************************************************************************
//*************************************************************************************
// Function: T_Ballz_Explode
//*************************************************************************************
//*************************************************************************************

static void T_Ballz_Explode (edict_t *ent)
{
    int    effect = TE_BFG_EXPLOSION;
    vec3_t origin;

    if (ent->owner->client)
        PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);//bounce?

    //FIXME: if we are onground then raise our Z just a bit since we are a point?
    T_RadiusTeleport(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius + 40);

    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
    gi.WriteByte (svc_temp_entity);
    if (ent->waterlevel)
    {
        if (ent->groundentity)
            gi.WriteByte (effect);
        else
            gi.WriteByte (effect);
    }
    else
    {
        if (ent->groundentity)
            gi.WriteByte (effect);
        else
            gi.WriteByte (effect);
    }
    gi.WritePosition (origin);
    gi.multicast (ent->s.origin, MULTICAST_PVS);

    G_FreeEdict (ent);
}

//=============================Mother=============================

static void T_Ball_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other == ent->owner)
        return;

    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (ent);
        return;
    }

    if (!other->takedamage && ent->movetype == MOVETYPE_BOUNCE)
    {
        if (ent->spawnflags & 1)
        {
            if (random() > 0.5)
                gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
            else
                gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
        }
        else
        {
            gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
        }
        return;
    }
    T_Ballz_Explode (ent);
}


//===============================Mother===============================

void fire_T_Ballz (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float idk, float damage_radius, int bMe )
{
    edict_t *tball;
    vec3_t  dir;
    vec3_t  forward, right, up;

    vectoangles (aimdir, dir);
    AngleVectors (dir, forward, right, up);

    tball = G_Spawn();//get g_spawn
    VectorCopy (start, tball->s.origin);
    VectorScale (aimdir, speed, tball->velocity);

    if (self->character->rune && self->character->rune->tball_speed)
    {
        VectorScale(tball->velocity, 2.0, tball->velocity);
        VectorSet (tball->avelocity, 300, 300, 300);
        tball->movetype = MOVETYPE_FLYMISSILE;
    }
    else
    {
        VectorMA (tball->velocity, 200 + crandom() * 10.0, up, tball->velocity);
        VectorMA (tball->velocity, crandom() * 10.0, right, tball->velocity);
        VectorSet (tball->avelocity, 300, 300, 300);
        tball->movetype = MOVETYPE_BOUNCE;
        tball->nextthink = level.time + 1;
        tball->think = T_Ballz_Explode;
    }

    tball->clipmask = MASK_SHOT;
    tball->solid = SOLID_BBOX;
    tball->s.effects |= EF_GRENADE;
    //VectorClear (tball->mins);//was for 0 size object
    //VectorClear (tball->maxs);
    //try to set the min and max bounding size
    VectorSet (tball->mins, -8, -8, -8);
    VectorSet (tball->maxs, 8, 8, 8);
    //tball->s.modelindex = gi.modelindex ("models/objects/smoke/tris.md2");
    tball->s.modelindex = gi.modelindex ("models/items/ammo/grenades/medium/tris.md2");
    tball->owner = self;
    tball->touch = T_Ball_Touch;
    tball->dmg = damage;
    tball->dmg_radius = damage_radius;
    tball->classname = "T_Ball";

    gi.linkentity (tball);

    if ( bMe )
        T_Ballz_Explode (tball);
}



//=======================end Mother===============================



//*************************************************************************************
//*************************************************************************************
// Function: 
//*************************************************************************************
//*************************************************************************************

void Kots_Use_T_Ball(edict_t *ent, char *args)
{
    //function to call if you want a player to fire a tball!
    vec3_t offset;
    vec3_t forward, right;
    vec3_t start;
    int      damage = 160;
    int    bMe    = false;
    float    radius;

    if (!Q_stricmp(args, "self"))
        bMe = true;

    if (ent->character->tballs <= 0)
    {
        gi.cprintf(ent, PRINT_MEDIUM, "You don't have any tballs.\n" );      
        return;
    }
 
    if ( ent->character->spree >= 25 )
    {
        gi.cprintf(ent, PRINT_MEDIUM, "No tballing while on a spree war!\n" );      
        return;
    }

    radius = damage;

    VectorSet(offset, 8, 8, ent->viewheight-8);
    AngleVectors (ent->client->v_angle, forward, right, NULL);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;

    //Uncloak the character
    Kots_CharacterUncloak(ent);

    fire_T_Ballz (ent, start, forward, damage, 600, 2.5, radius, bMe);

    PlayerNoise(ent, start, PNOISE_WEAPON);

    ent->character->tballs -= 1;
    gi.cprintf(ent,PRINT_HIGH, "You now have %i tballs left\n", ent->character->tballs);
}

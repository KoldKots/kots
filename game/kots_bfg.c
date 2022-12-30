#include "kots_bfg.h"
#include "kots_pweapon.h"
#include "kots_runes.h"
#include "kots_weapondamage.h"
#include "kots_utils.h"

void weapon_kotsbfg_fire (edict_t *ent)
{
    vec3_t  offset, start;
    vec3_t  forward, right;
    int     damage = 35;
    int     ammo_cost = 3;
    //int       num = 12; // Aldarn - make it higher for more view fuckage 
    //SWB - number was WAY too high
    // Aldarn - number was too low :P
    if (ent->character->cur_bfg >= 10 && ent->character->rune && ent->character->rune->bfg > 0)
        ammo_cost = 2;

    if (ent->client->pers.inventory[ent->client->ammo_index] < ammo_cost)
    {
        ent->client->ps.gunframe++;
        return;
    }

    if (ent->client->ps.gunframe == 5)
    {
        is_silenced = (ent->character->cur_bfg >= 2 ? MZ_SILENCED : is_silenced);

        // send muzzle flash
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_BFG | is_silenced);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        ent->client->ps.gunframe++;

        PlayerNoise(ent, start, PNOISE_WEAPON);
        return;
    }

    damage = Kots_CharacterBfgDamage(ent, damage);

    //determine how much damage each beam should be
    //only first 3 beams do full damage
    if (ent->client->ps.gunframe > 8)
        damage = Kots_RandDivide(damage, 2);

    ++ent->character->shots;

    if (is_quad)
        damage *= 4;

    // Aldarn - In Kots2k, BFG firing fucks view angle
    //VectorSet(offset, crandom()*num, crandom()*num, ent->viewheight-crandom()*num);
    //AngleVectors (ent->client->v_angle, forward, right, NULL);
    //VectorScale (forward, crandom()*-num, ent->client->kick_origin);
    //ent->client->kick_angles[0] = crandom()*-num;
    //ent->client->kick_angles[1] = crandom()*-num;
    //ent->client->kick_angles[2] = crandom()*-num;
    //SWB - effect is more of just a rocking back and forth so only modify the roll angle

    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 8, 8, ent->viewheight-8);

    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    fire_kotsbfg (ent, start, forward, damage);

    ent->client->ps.gunframe++;
    ent->client->pers.inventory[ent->client->ammo_index] -= ammo_cost;
}

void Weapon_KotsBfg (edict_t *ent)
{
    static int  pause_frames[]  = {39, 45, 50, 55, 0};
    static int  fire_frames1[]  = {5, 6, 7, 8, 9, 0};
    static int  fire_frames2[]  = {5, 6, 7, 8, 9, 10, 11, 12, 0};

    if (ent->character->cur_bfg >= 4)
        Weapon_Generic (ent, 4, 18, 55, 58, pause_frames, fire_frames2, weapon_kotsbfg_fire);
    else
        Weapon_Generic (ent, 4, 15, 55, 58, pause_frames, fire_frames1, weapon_kotsbfg_fire);
}


void fire_kotsbfg (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
    edict_t *ignore;
    trace_t tr;
    vec3_t from;
    vec3_t end;
    int mask;
    int count = 0;

    //create starting and ending positions for trace
    VectorCopy (start, from);
    VectorMA (start, 8192, dir, end);

    //taken from railgun fire code
    ignore = self;
    mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
    while (ignore)
    {
        tr = gi.trace (from, NULL, NULL, end, ignore, mask);

        if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
            mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
        else
        {
            //ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
            if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->solid == SOLID_BBOX))
                ignore = tr.ent;
            else
                ignore = NULL;

            if ((tr.ent != self) && (tr.ent->takedamage))
            {
                //K2K lies it's only 1 shot per beam
                T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 100, DAMAGE_ENERGY, MOD_BFG_LASER);

                //SWB
                ++self->character->hits;
            }
        }

        VectorCopy (tr.endpos, from);

        if (++count >= MAX_EDICTS)
            break;
    }
    
    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_LASER_SPARKS);
    gi.WriteByte (4);
    gi.WritePosition (tr.endpos);
    gi.WriteDir (tr.plane.normal);
    gi.WriteByte ((byte)0xd0d1d2d3); //green sparks
    gi.multicast (tr.endpos, MULTICAST_PVS);

    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_BFG_LASER);
    gi.WritePosition (start);
    gi.WritePosition (tr.endpos);
    gi.multicast (start, MULTICAST_PHS);

    if (self->character->cur_bfg >= 10 && self->character->rune && self->character->rune->bfg > 0)
    {
        //create a second beam to make it brighter
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_BFG_LASER);
        gi.WritePosition (start);
        gi.WritePosition (tr.endpos);
        gi.multicast (start, MULTICAST_PHS);
    }
}

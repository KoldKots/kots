#include "kots_sabre.h"
#include "kots_pweapon.h"
#include "kots_runes.h"
#include "kots_weapondamage.h"
#include "kots_utils.h"

void weapon_sabre_fire (edict_t *ent)
{
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      offset;
    int         damage;
    int         length;
    int         color;
    damage = 80;
    damage = Kots_CharacterSabreDamage(ent, damage);

    if (ent->client->ps.gunframe == 5 && ent->character->cur_sabre < 2)
        ent->client->weapon_sound = gi.soundindex("misc/power1.wav");
    else if (ent->client->ps.gunframe == 13)
        ent->client->weapon_sound = 0;

    // Aldarn - Modified so letting go of fire stops firing!
    if (!(ent->client->buttons & BUTTON_ATTACK))
    {
        if (ent->client->ps.gunframe < 13)
        {
            ent->client->weaponstate = WEAPON_READY;
            ent->client->sabre_refire = level.time + 0.9;
            ent->client->sabre_frame = ent->client->ps.gunframe;
        }
        else //no wait to refire because this was the last frame
            ent->client->sabre_refire = 0;

        ent->client->ps.gunframe++;
        ent->client->weapon_sound = 0;
        return;
    }

    //determine how much damage each slash should be
    switch (ent->client->ps.gunframe)
    {
        //first 3 do full damage
        case 5:
        case 6:
        case 7:
            break;

        case 8: //next 3 do half damage
        case 9:
        case 10:
            damage = Kots_RandDivide(damage, 2);
            break;

        case 11: //final 3 do 1/3 damage
        case 12:
        case 13:
            damage = Kots_RandDivide(damage, 3);
            break;
    }

    ++ent->character->shots;

    if (is_quad)
        damage *= 4;

    //get length and color
    switch (ent->character->cur_sabre)
    {
        case 0:
            length = 80;
            color = 0xf3f3f1f1; //blue
            break;
        case 1:
        case 2:
        case 3:
            length = 100;
            color = 0xd0d1d2d3; //green
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            length = 120;
            color = 0xe0e1e2e3; //orange
            break;
        case 10:
        default:

            if (!ent->character->rune || ent->character->rune->sabre <= 0)
            {
                length = 140;
                color = 0xf2f2f0f0; //red
            }
            else //level 10 + rune ability 
            {
                length = 160;
                color = 0xf3f2f1f0;
            }
            break;
    }

    AngleVectors (ent->client->v_angle, forward, right, NULL); 
    VectorSet(offset, 8, 8, ent->viewheight-8);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    fire_sabre (ent, start, forward, damage, length, color);

    ent->client->ps.gunframe++;
}


void Weapon_Sabre (edict_t *ent)
{
    static int  pause_frames[]  = {19, 32, 0};
    static int  fire_frames[]   = {5, 6, 7, 8, 9, 10, 11, 12, 13, 0};

    Weapon_Generic (ent, 4, 23, 52, 55, pause_frames, fire_frames, weapon_sabre_fire);
}

void fire_sabre (edict_t *self, vec3_t start, vec3_t dir, int damage, int length, int color)
{
    edict_t *ignore;
    trace_t tr, tr2;
    vec3_t from, end;
    vec3_t mins, maxs;
    qboolean water;
    int mask;
    int count = 0;

    VectorSet(mins, -2, -2, -2);
    VectorSet(maxs, 2, 2, 2);

    //create starting and ending positions for trace
    VectorCopy(start, from);
    VectorMA(start, length, dir, end);

    //taken from railgun fire code
    ignore = self;
    water = false;
    mask = MASK_SHOT;
    while (ignore)
    {
        tr = gi.trace (from, mins, maxs, end, ignore, mask);

        //ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
        if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
            (tr.ent->solid == SOLID_BBOX))
            ignore = tr.ent;
        else
            ignore = NULL;

        if ((tr.ent != self) && (tr.ent->takedamage))
        {
            T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 100, DAMAGE_ENERGY, MOD_SABRE);
            ++self->character->hits;
        }

        VectorCopy (tr.endpos, from);

        if (++count >= MAX_EDICTS)
            break;
    }

    //trace forward and back to find water
    tr2 = gi.trace(tr.endpos, NULL, NULL, start, tr.ent, MASK_WATER);

    if (tr2.contents & MASK_WATER)
        water = true;
    else //water not found going backwards so try forwards
    {
        tr2 = gi.trace(start, NULL, NULL, tr.endpos, self, MASK_WATER);

        if (tr2.contents & MASK_WATER)
            water = true;
    }

    CreateLaser(self, start, tr.endpos, 0, color, 4, 4, 0.1);

    if (water) //create a laser in the reverse direction so it shows up in and out of water
        CreateLaser(self, tr.endpos, start, 0, color, 4, 4, 0.1);
}

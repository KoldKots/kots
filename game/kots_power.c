#include "kots_power.h"
#include "kots_resist.h"
#include "kots_weapondamage.h"
#include "kots_abilities.h"
#include "kots_utils.h"
#include "kots_hook.h"
#include "kots_runes.h"
#include "kots_pweapon.h"

//Power names and functions to add and resist them
#define TotalPowerPoints 7
powerpoint_t PowerPoints[TotalPowerPoints] =
{
    "expack", KOTS_POWER_EXPACK, Kots_CharacterAddExpack, Kots_CharacterResistExpack,
    "spiral", KOTS_POWER_SPIRAL, Kots_CharacterAddSpiral, Kots_CharacterResistSpiral,
    "bide", KOTS_POWER_BIDE, Kots_CharacterAddBide, Kots_CharacterResistBide,
    "throw", KOTS_POWER_THROW, Kots_CharacterAddThrow, Kots_CharacterResistThrow,
    "kotsthrow", KOTS_POWER_THROW, Kots_CharacterAddThrow, Kots_CharacterResistThrow,
    "anti", KOTS_POWER_ANTIWEAPON, Kots_CharacterAddAntiweapon, NULL,
    "antiweapon", KOTS_POWER_ANTIWEAPON, Kots_CharacterAddAntiweapon, NULL
};

powerpoint_t *Kots_GetPowerTree(char *name)
{
    int i;

    for (i = 0; i < TotalPowerPoints; i++)
    {
        if (Q_stricmp(name, PowerPoints[i].name) == 0)
            return &PowerPoints[i];
    }

    return NULL;
}

int *Kots_CharacterGetPowerLevel(edict_t *ent, int power)
{
    switch (power)
    {
    case KOTS_POWER_EXPACK:
        return &ent->character->expack;
    case KOTS_POWER_SPIRAL:
        return &ent->character->spiral;
    case KOTS_POWER_BIDE:
        return &ent->character->bide;
    case KOTS_POWER_THROW:
        return &ent->character->kotsthrow;
    case KOTS_POWER_ANTIWEAPON:
        return &ent->character->antiweapon;
    default:
        return &ent->character->powerpoints;
    }
}

qboolean Kots_CharacterCanAddPower(edict_t *ent)
{
    if (ent->character->powerpoints > 0)
        return true;

    gi.cprintf(ent, PRINT_HIGH, "No power points available.\n");
    return false;
}

void Kots_CharacterAddExpack(edict_t *ent)
{
    if (Kots_CharacterCanAddPower(ent))
    {
        if (ent->character->resist == POW_EXPACK)
            gi.cprintf(ent, PRINT_HIGH, "You cannot train powers you resist.\n");
        else if (ent->character->level < 6)
            gi.cprintf(ent, PRINT_HIGH, "You must be at least level 6 to train this.\n");
        else if (ent->character->expack >= ent->character->level - 7) //allow 1 expack per level above 7 (so only 1 at 8)
            gi.cprintf(ent, PRINT_HIGH, "You must increase your level before adding any more points.\n");
        else
        {
            ent->character->expack++;
            ent->character->cur_expack++;
            ent->character->powerpoints--;
            ent->character->expacksleft += 2;
            gi.cprintf(ent, PRINT_HIGH, "Expack is now %i.\n", ent->character->expack);
        }
    }
}

void Kots_CharacterAddSpiral(edict_t *ent)
{
    if (Kots_CharacterCanAddPower(ent) &&
        Kots_PowerCharacterCheckLevelCap(ent, KOTS_POWER_SPIRAL, ent->character->spiral + 1))
    {
        if (ent->character->resist == POW_SPIRAL)
            gi.cprintf(ent, PRINT_HIGH, "You cannot train powers you resist.\n");
        else if (ent->character->level < 6)
            gi.cprintf(ent, PRINT_HIGH, "You must be at least level 6 to train this.\n");
        else if (ent->character->spiral >= ent->character->level - 5) //allow 1 spiral per level above 5 (so only 1 at 6)
            gi.cprintf(ent, PRINT_HIGH, "You must increase your level before adding any more points.\n");
        else
        {
            ent->character->spiral++;
            ent->character->cur_spiral++;
            ent->character->powerpoints--;
            ent->character->spiralsleft += 2;
            gi.cprintf(ent, PRINT_HIGH, "Spiral is now %i.\n", ent->character->spiral);
        }
    }
}

void Kots_CharacterAddBide(edict_t *ent)
{
    if (Kots_CharacterCanAddPower(ent))
    {
        if (ent->character->resist == POW_BIDE)
            gi.cprintf(ent, PRINT_HIGH, "You cannot train powers you resist.\n");
        else if (ent->character->level < 6)
            gi.cprintf(ent, PRINT_HIGH, "You must be at least level 6 to train this.\n");
        else if (ent->character->bide >= ent->character->level - 5) //allow 1 bide per level above 5 (so only 1 at 6)
            gi.cprintf(ent, PRINT_HIGH, "You must increase your level before adding any more points.\n");
        else
        {
            ent->character->bide++;
            ent->character->cur_bide++;
            ent->character->powerpoints--;
            ent->character->bidesleft += 1;
            gi.cprintf(ent, PRINT_HIGH, "Bide is now %i.\n", ent->character->bide);
        }
    }
}

void Kots_CharacterAddThrow(edict_t *ent)
{
    if (Kots_CharacterCanAddPower(ent))
    {
        if (ent->character->resist == POW_THROW)
            gi.cprintf(ent, PRINT_HIGH, "You cannot train powers you resist.\n");
        else if (ent->character->level < 3)
            gi.cprintf(ent, PRINT_HIGH, "You must be at least level 3 to train this.\n");
        else
        {
            ent->character->kotsthrow++;
            ent->character->cur_kotsthrow++;
            ent->character->powerpoints--;
            ent->character->throwsleft += 1;
            gi.cprintf(ent, PRINT_HIGH, "Throw is now %i.\n", ent->character->kotsthrow);
        }
    }
}

void Kots_CharacterAddAntiweapon(edict_t *ent)
{
    if (Kots_CharacterCanAddPower(ent))
    {
        ent->character->antiweapon++;
        ent->character->cur_antiweapon++;
        ent->character->powerpoints--;
        gi.cprintf(ent, PRINT_HIGH, "Antiweapon is now %i.\n", ent->character->antiweapon);
    }
}

void Kots_ExpackExplode(edict_t *ent)
{
    vec3_t      origin;

    //T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_EXPACK);

    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
    gi.WriteByte (svc_temp_entity);
    if (ent->waterlevel)
        gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
    else
        gi.WriteByte (TE_ROCKET_EXPLOSION);
    gi.WritePosition (origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);

    G_FreeEdict (ent);
}

qboolean Kots_ExpackTouch(edict_t *ent, edict_t *other)
{
    int dmg;
    vec3_t      origin;

    if (!other->client && !(other->svflags & SVF_MONSTER))
        return false;

    // calculate position for the explosion entity
    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

    if (other->takedamage)
    {
        if (ent->think == Kots_ExpackExplode)
            dmg = ent->dmg - ((level.time - (ent->nextthink - 40.0)) * 6);
        else
            dmg = ent->dmg;

        T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, NULL, dmg, 0, 0, MOD_EXPACK);
    }

    //T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_EXPACK);

    gi.WriteByte (svc_temp_entity);
    if (ent->waterlevel)
        gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
    else
        gi.WriteByte (TE_ROCKET_EXPLOSION);
    gi.WritePosition (origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);

    return true;
}

void Kots_ExpackMakeTouchable(edict_t *ent)
{
    ent->touch = Touch_Item;

    //explode after the specified period even if nothing touches it
    //we might get some blast damage
    ent->think = Kots_ExpackExplode;
    ent->nextthink = level.time + 40.0;
}

void Kots_ExpackDrop(edict_t *ent)
{
    if (ent->character->cur_expack <= 0)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the power to drop expacks.\n");
    else if (ent->character->expacksleft <= 0)
        gi.cprintf(ent, PRINT_HIGH, "You have no more expacks left.\n");
    else if (ent->character->expacksout >= 4 || ent->character->expacksout >= ceil(ent->character->cur_expack / 2.0))
        gi.cprintf(ent, PRINT_HIGH, "You already have the maximum number of expacks out.\n");
    else
    {
        gitem_t *item = FindItem("Kots Expack");
        edict_t *expack = Drop_Item(ent, item);

        //decrement the number of expacks left and increment the number out
        --ent->character->expacksleft;
        ++ent->character->expacksout;

        expack->dmg = Kots_CharacterMunitionDamage(ent, 250 + ((ent->character->cur_expack - 1) * 10));
        expack->radius_dmg = 250;
        expack->dmg_radius = 200;
        expack->touch = NULL; //Expack is not touchable for a little while
        expack->think = Kots_ExpackMakeTouchable;   //otherwise it explodes in our face
        expack->s.effects = EF_GREENGIB;

        //uncloak the character if cloaked
        Kots_CharacterUncloak(ent);
    }
}

void Kots_CharacterThrow(edict_t *ent, char *args)
{
    if (ent->character->cur_kotsthrow <= 0)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the power to throw.\n");
    else if (ent->character->throwsleft <= 0)
        gi.cprintf(ent, PRINT_HIGH, "You do not have any throws left.\n");
    else
    {
        //throw other
        if (!args || *args == '\0')
            Kots_CharacterThrowOther(ent);
        else if (Q_stricmp(args, "self") == 0)
            Kots_CharacterThrowSelf(ent);
        else if (Q_stricmp(args, "up") == 0)
            Kots_CharacterThrowUp(ent);
        else
            gi.cprintf(ent, PRINT_HIGH, "Unrecognized throw type.\n");
    }
}

void Kots_CharacterThrowOther(edict_t *ent)
{
    if (ent->character->next_throw > level.time)
        gi.cprintf(ent, PRINT_HIGH, "You must wait a while before throwing again.\n");
    else
    {
        edict_t *targ = NULL, *who = NULL, *best = NULL;
        float bd = 0, d = 0;
        vec3_t  forward, dir;
        int count = 0;

        AngleVectors(ent->client->v_angle, forward, NULL, NULL);
        while ((targ = findradius(targ, ent->s.origin, 2000)) != NULL)
        {
            if (++count >= MAX_EDICTS)
                break;

            if (!targ->character || !targ->character->is_loggedin || targ->health <= 0 || targ == ent)
                continue;

            who = targ;
            VectorSubtract(who->s.origin, ent->s.origin, dir);
            VectorNormalize(dir);
            d = DotProduct(forward, dir);

            if (d > bd && loc_CanSee(ent, who))
            {
                bd = d;
                best = who;
            }
        }

        if (bd < 0.90)
            return;

        //if we found someone to throw
        if (best)
        {
            vec3_t dir;
            int distance = 2048;

            if (best->character->resist == POW_THROW)
                distance /= 5;

            AngleVectors(ent->client->v_angle, dir, NULL, NULL);
            VectorMA(best->s.origin, distance, dir, best->velocity);
            VectorSubtract(best->velocity, best->s.origin, best->velocity);
            best->velocity[2] = 500; // always throw slightly up
            gi.sound(best, CHAN_AUTO, gi.soundindex("misc/windfly.wav"), 1, ATTN_NORM, 0);

            //if enemy is trying to use the grapple then break it
            if (best->client && best->client->ctf_grapple)
                CTFPlayerResetGrapple(best);

            --ent->character->throwsleft;
            ent->character->next_throw = level.time + 2.0;

            //uncloak the character if cloaked
            Kots_CharacterUncloak(ent);
        }

        /*
        edict_t *ignore;
        trace_t tr;
        vec3_t mins, maxs;
        vec3_t from;
        vec3_t end;
        vec3_t dir;
        int mask;
        int count = 0;

        AngleVectors(ent->client->v_angle, dir, NULL, NULL);
        VectorCopy(ent->s.origin, from);
        VectorMA(from, 8096, dir, end);

        // Aldarn - increase throw radius
        VectorSet(mins, -10, -10, -10);
        VectorSet(maxs, 10, 10, 10);
        //SWB - this is too much, we should use what's done in lift but need to figure out why it wasn't working for karma id

        ignore = ent;
        mask = MASK_SHOT;
        while (ignore)
        {
            tr = gi.trace (from, mins, maxs, end, ignore, mask);

            if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
                (tr.ent->solid == SOLID_BBOX))
                ignore = tr.ent;
            else
                ignore = NULL;

            if (tr.ent != ent && tr.ent->character && tr.ent->health > 0)
            {
                int distance = 2048;

                if (ent->character->resist == POW_THROW)
                    distance /= 2;
                if (tr.ent->character->resist == POW_THROW)
                    distance = (int)(distance * 0.75);

                VectorMA(tr.ent->s.origin, distance, dir, tr.ent->velocity);
                VectorSubtract(tr.ent->velocity, tr.ent->s.origin, tr.ent->velocity);
                tr.ent->velocity[2] = 500; // always throw self slightly up
                gi.sound(tr.ent, CHAN_AUTO, gi.soundindex("misc/windfly.wav"), 1, ATTN_NORM, 0);

                --ent->character->throwsleft;
                ent->character->next_throw = level.time + 1.0;

                //uncloak the character if cloaked
                Kots_CharacterUncloak(ent);
                break;
            }

            VectorCopy (tr.endpos, from);

            if (++count >= MAX_EDICTS)
                break;
        }
        */
    }
}

void Kots_CharacterThrowSelf(edict_t *ent)
{
    vec3_t dir;
    int distance = 2048;

    if (ent->character->resist == POW_THROW)
        distance /= 2;

    --ent->character->throwsleft;
    AngleVectors(ent->client->v_angle, dir, NULL, NULL);
    VectorMA(ent->s.origin, distance, dir, ent->velocity);
    VectorSubtract(ent->velocity, ent->s.origin, ent->velocity);

    if (ent->groundentity && ent->velocity[2] < 200)
    {
        ent->velocity[2] = 200; //minimum velocity so we can get off the ground a bit

        //lift up a little bit to start
        //ent->s.old_origin[2] += 20;
        //ent->s.origin[2] += 20;
    }

    //play throw sound
    gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/windfly.wav"), 1, ATTN_NORM, 0);
}

void Kots_CharacterThrowUp(edict_t *ent)
{
    --ent->character->throwsleft;
    ent->velocity[2] = 2048; // throw straight up

    if (ent->character->resist == POW_THROW)
        ent->velocity[2] /= 2;

    //play throw sound
    gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/windfly.wav"), 1, ATTN_NORM, 0);
}

void Kots_Bide_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t      origin;
    int hits;

    if (other == ent->owner)
        return;

    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (ent);
        return;
    }

    //owner might be null if client disconnected?
    if (ent->owner && ent->owner->client)
        PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    // calculate position for the explosion entity
    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

    //keep track of the previous number of hits because we don't want it to count for accuracy
    if (ent->owner && ent->owner->character)
        hits = ent->owner->character->hits;

    if (other->takedamage)
        T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_BIDE);

    T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_BIDERADIUS);

    //reset the number of hits in case it increased from radius damage
    if (ent->owner && ent->owner->character)
        ent->owner->character->hits = hits;

    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_BFG_EXPLOSION);
    gi.WritePosition (ent->s.origin);
    gi.multicast (ent->s.origin, MULTICAST_PVS);

    //play explosion sound
    gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/Bfg__x1b.wav"), 1, ATTN_NORM, 0);

    G_FreeEdict (ent);
}

void Kots_Fire_Bide (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
    edict_t *bide;

    bide = G_Spawn();
    VectorCopy (start, bide->s.origin);
    VectorCopy (dir, bide->movedir);
    vectoangles (dir, bide->s.angles);
    VectorScale (dir, speed, bide->velocity);
    VectorSet (bide->avelocity, 300, 300, 300); //add rotation
    bide->movetype = MOVETYPE_FLYMISSILE;
    bide->clipmask = MASK_SHOT;
    bide->solid = SOLID_BBOX;
    bide->s.effects = EF_COLOR_SHELL | EF_TELEPORTER; // Aldarn - this isn't right
    bide->s.renderfx = RF_SHELL_RED | RF_SHELL_GREEN | RF_FULLBRIGHT; // Or this?
    VectorClear (bide->mins);
    VectorClear (bide->maxs);
    bide->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2"); // Is it a different model?
    bide->owner = self;
    bide->touch = Kots_Bide_Touch;
    bide->nextthink = level.time + 8000/speed;
    bide->think = G_FreeEdict;
    bide->dmg = damage;
    bide->radius_dmg = radius_damage;
    bide->dmg_radius = damage_radius;
    bide->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
    bide->classname = "kotsbide";

    gi.linkentity (bide);
}

void Kots_Bide_Fire (edict_t *ent)
{
    vec3_t  offset, start;
    vec3_t  forward, right;
    float max_damage, bide_damage;
    float bide_start = 100 + (50 * ent->character->cur_bide);
    float radius = KOTS_BIDE_MIN_RADIUS;

    //limit bide start to the max bide start damage
    if (bide_start > KOTS_BIDE_START_CAP)
        bide_start = KOTS_BIDE_START_CAP;

    //must have had a rune to start it with so go back to original start damage
    if (bide_start > ent->character->bidedmg)
        bide_start = 150 + (50 * ent->character->bide);

    //maximum damage is 10 times our starting amount
    max_damage = bide_start * 10;
    bide_damage = ent->character->bidedmg - bide_start;

    //determine how much of a radius we should use
    if (max_damage > bide_damage)
        radius += (KOTS_BIDE_MAX_RADIUS - KOTS_BIDE_MIN_RADIUS) * (bide_damage / max_damage);
    else
        radius = KOTS_BIDE_MAX_RADIUS;

    AngleVectors (ent->client->v_angle, forward, right, NULL);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;

    VectorSet(offset, 8, 8, ent->viewheight-8);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    Kots_Fire_Bide (ent, start, forward, ent->character->bidedmg, 700, radius, ent->character->bidedmg / 2);

    //uncloak the character if cloaked
    Kots_CharacterUncloak(ent);
}

void Kots_CharacterBide(edict_t *ent)
{
    if ((ent->character->bidestart < level.time-10) && (ent->character->bidestart > level.time-40) && (ent->character->bideon)) // ready to fire
    {
        // fire it
        Kots_Bide_Fire(ent);

        gi.cprintf(ent,PRINT_HIGH,"Bide cast for %i damage!\n",ent->character->bidedmg);
        gi.positioned_sound (ent->s.origin, ent, CHAN_WEAPON, gi.soundindex("world/lite_out.wav"), 1, ATTN_NORM, 0);

        //reset bide timer and damage
        ent->character->bideon = false;
        ent->character->bidestart = 0;
        ent->character->bidedmg = 0;
    }
    else if(!ent->character->bideon) // ready to start
    {
        if(ent->character->bidesleft > 0)
        {
            ent->character->bidesleft--;
            ent->character->bidedmg = 150 + (50 * ent->character->cur_bide);
            ent->character->bidestart = level.time;
            ent->character->bideon = true;

            //limit to 500 damage max
            if (ent->character->bidedmg > KOTS_BIDE_START_CAP)
                ent->character->bidedmg = KOTS_BIDE_START_CAP;

            gi.positioned_sound (ent->s.origin, ent, CHAN_WEAPON, gi.soundindex("makron/head1.wav"), 1, ATTN_NORM, 0);
            gi.cprintf(ent,PRINT_HIGH,"Bide charging. You have %i charges left.\n",ent->character->bidesleft);
        }
        else
            gi.cprintf(ent,PRINT_HIGH,"You have no bide charges remaining!\n");
    }
}


void Kots_CharacterBideCheckTimer(edict_t *ent)
{
    if (ent->character->bideon && ent->character->bidestart)
    {
        //check if bide wasn't fired within the time limit
        if (ent->character->bidestart <= level.time - 40)
        {
            gi.sound(ent, CHAN_WEAPON, gi.soundindex("makron/popup.wav"), 1, ATTN_NORM, 0);
            ent->character->bideon = false;
            ent->character->bidestart = 0;
            ent->character->bidedmg = 0;
        }
    }
}

void Kots_CharacterBideExplode(edict_t *ent)
{
    if (ent->character->bideon)
    {
        vec3_t      origin;
        int mod = meansOfDeath; //keep track of the means of death
        ent->character->bideon = false; // turn off bide, we're done now

        //if we're still charging only do the minimum of damage
        if (ent->character->bidestart > level.time - 10)
            T_RadiusDamage(ent, ent, 200, ent, KOTS_BIDE_MIN_RADIUS, MOD_BIDERADIUS);
        else //otherwise do half what we would have done if we had fired
            T_RadiusDamage(ent, ent, ent->character->bidedmg / 2, ent, KOTS_BIDE_MIN_RADIUS, MOD_BIDERADIUS);

        //reset the means of death back
        meansOfDeath = mod;

        VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
        gi.WriteByte (svc_temp_entity);
        if (ent->waterlevel)
            gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
        else
            gi.WriteByte (TE_ROCKET_EXPLOSION);
        gi.WritePosition (origin);
        gi.multicast (ent->s.origin, MULTICAST_PHS);
    }
}

void Kots_SpiralBeamCopy(edict_t *from, edict_t *to)
{
    VectorCopy(from->s.origin, to->s.origin);
    VectorCopy(from->s.old_origin, to->s.old_origin);
    VectorCopy(from->velocity, to->velocity);
    to->s.frame = from->s.frame;
    gi.linkentity(to);
}

void Kots_SpiralBeamThink(edict_t *ent)
{
    //when the origin is gone we must go
    if (!ent->owner || !ent->owner->inuse || !ent->goalentity || !ent->owner->inuse || !ent->goalentity->inuse)
    {
        //free our duplicate above water
        if (ent->target_ent)
            G_FreeEdict(ent->target_ent);

        G_FreeEdict(ent);
        return;
    }

    else
    {
        static edict_t *hits[MAX_EDICTS];
        vec3_t angle, moveto;
        trace_t trace;
        edict_t *ignore;
        float timeleft = (ent->goalentity->nextthink - level.time);
        float percent = timeleft / KOTS_SPIRAL_TIME;
        int raduis = ((KOTS_SPIRAL_MAX_RADIUS - KOTS_SPIRAL_MIN_RADIUS) * percent) + KOTS_SPIRAL_MIN_RADIUS;
        int hitcount = 0;

        //set the next movement time
        ent->nextthink = level.time + FRAMETIME;

        //create area to hit
        VectorSet(ent->maxs, ent->s.frame, ent->s.frame, ent->s.frame);
        VectorCopy(ent->maxs, ent->mins);
        VectorInverse(ent->mins);

        //find people standing in the beam
        if (ent->goalentity->dmg_radius)
        {
            int dmg = ent->goalentity->dmg_radius / 2;
            int count = 0;

            dmg += ceil(dmg * percent);

            VectorCopy(ent->moveinfo.end_origin, moveto);
            ignore = NULL;
            while (true)
            {
                trace = gi.trace(moveto, ent->mins, ent->maxs, ent->moveinfo.start_origin, ignore, MASK_SHOT);

                if (trace.fraction >= 1.0)
                    break;

                ignore = trace.ent;
                if (ignore->takedamage)
                {
                    //add edict to our hits array
                    hits[hitcount++] = ignore;

                    //unlink the entity so we don't hit it again
                    //otherwise we'll just keep hitting it a million times
                    gi.unlinkentity(ignore);

                    //don't hit the spiral owner
                    if (ignore != ent->owner)
                        T_Damage(ignore, ent, ent->owner, ent->velocity, trace.endpos, NULL, dmg, 0, DAMAGE_ENERGY | DAMAGE_NO_KNOCKBACK, MOD_SPIRAL);
                }

                VectorCopy(trace.endpos, moveto);

                if (++count >= MAX_EDICTS)
                    break;
            }

            //if we hit some things we'll need to relink them
            if (hitcount > 0)
            {
                int i;
                for (i = 0; i < hitcount; i++)
                {
                    gi.linkentity(hits[i]);
                    hits[i] = NULL;
                }
            }
        }

        //create next movement point
        ent->s.angles[2] += KOTS_SPIRAL_SPEED;
        VectorSet(angle, 0, ent->s.angles[2], 0);
        AngleVectors(angle, angle, NULL, NULL);
        VectorMA(ent->goalentity->s.origin, raduis, angle, moveto);

        //find ground
        VectorCopy(moveto, ent->s.origin);
        moveto[2] -= 512;
        trace = gi.trace(ent->s.origin, NULL, NULL, moveto, NULL, MASK_SOLID);

        //if we started in a wall
        if (trace.startsolid)
        {
            moveto[2] += 1024;
            trace = gi.trace(ent->s.origin, NULL, NULL, moveto, NULL, MASK_SOLID);

            if (!trace.allsolid)
            {
                moveto[2] -= 1024;
                trace = gi.trace(trace.endpos, NULL, NULL, moveto, NULL, MASK_SOLID);
            }
        }

        ent->s.origin[2] = trace.endpos[2] + 10;

        //create top point 30 degrees from bottom
        angle[1] = ent->s.angles[2] + 35;
        AngleVectors(angle, angle, NULL, NULL);
        VectorMA(ent->goalentity->s.origin, raduis, angle, moveto);
        VectorCopy(moveto, ent->s.old_origin);

        //make sure we don't go past the sky
        moveto[2] += 512;
        trace = gi.trace(ent->s.origin, NULL, NULL, moveto, NULL, MASK_SOLID);
        VectorCopy(trace.endpos, ent->s.old_origin);
        ent->s.old_origin[2] -= 10;

        //create the velocity for the beam
        VectorSubtract(ent->s.old_origin, ent->s.origin, ent->velocity);
        VectorScale(ent->velocity, 10, ent->velocity);

        //shrink based on time remaining
        ent->s.frame = 1 + (int)((KOTS_SPIRAL_BEAMSIZE - 1) * (timeleft / KOTS_SPIRAL_TIME));

        //keep the old movement info
        VectorCopy(ent->s.origin, ent->moveinfo.start_origin);
        VectorCopy(ent->s.old_origin, ent->moveinfo.end_origin);

        //check if we're partly in water
        trace = gi.trace(ent->s.old_origin, NULL, NULL, ent->s.origin, NULL, MASK_WATER);
        if (trace.fraction < 1.0)
        {

            //if we don't have a duplicate then create one now
            //this is because when things are partly in water only part is shown
            if (!ent->target_ent)
            {
                ent->target_ent = G_Spawn();
                ent->target_ent->owner = ent->owner;
                ent->target_ent->goalentity = ent->goalentity;
                ent->target_ent->s.skinnum = ent->s.skinnum;
                ent->target_ent->s.renderfx = ent->s.renderfx;
                ent->target_ent->s.effects = ent->s.effects & ~EF_TELEPORTER; //don't use teleporter effect or it will be on ceiling
                ent->target_ent->movetype = ent->movetype;
                ent->target_ent->clipmask = ent->clipmask;
                ent->target_ent->model = ent->model;
                ent->target_ent->s.modelindex = ent->s.modelindex;
            }

            //copy position to the target
            Kots_SpiralBeamCopy(ent, ent->target_ent);
        }
        else
        {
            //free the duplicate while it's not needed
            if (ent->target_ent)
            {
                G_FreeEdict(ent->target_ent);
                ent->target_ent = NULL;
            }
        }

        gi.linkentity(ent); //relink after moving
    }
}

void Kots_CharacterSpiral(edict_t *ent, char *args)
{
    if (ent->character->cur_spiral <= 0)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the power to use spiral.\n");
    else if (ent->character->spiralsleft <= 0)
        gi.cprintf(ent, PRINT_HIGH, "You do not have any spirals left.\n");
    else if (ent->character->next_spiral > level.time)
        gi.cprintf(ent, PRINT_HIGH, "You must wait a while before using spiral again.\n");
    else if (!(*args))
        Kots_CharacterCreateSpiral(ent, false);
    else if (Q_stricmp(args, "forward") == 0)
        Kots_CharacterCreateSpiral(ent, true);
    else
        gi.cprintf(ent, PRINT_HIGH, "Invalid spiral argument '%s'.\n", args);
}

void Kots_CharacterCreateSpiral(edict_t *ent, qboolean forward)
{
    int i;

    //create the central point
    edict_t *spiral = G_Spawn();
    VectorCopy(ent->s.origin, spiral->s.origin);
    spiral->s.origin[2] += ent->viewheight; //raise it up a bit
    VectorCopy(spiral->s.origin, spiral->s.old_origin);
    spiral->owner = ent;
    spiral->movetype = MOVETYPE_NOCLIP;
    spiral->model = NULL;
    spiral->think = G_FreeEdict;
    spiral->nextthink = level.time + KOTS_SPIRAL_TIME;
    spiral->s.sound = gi.soundindex("weapons/Bfg__l1a.wav");
    gi.sound(ent, CHAN_WEAPON, gi.soundindex("chick/Chkatck2.wav"), 1, ATTN_NORM, 0);

    if (forward)
    {
        vec3_t angle, end, velocity;
        AngleVectors(ent->client->v_angle, angle, NULL, NULL);
        VectorMA(ent->s.origin, KOTS_SPIRAL_FORWARD, angle, end);
        VectorSubtract(end, ent->s.origin, velocity);
        VectorScale(velocity, 10.0 / KOTS_SPIRAL_TIME, spiral->velocity);
    }

    gi.linkentity(spiral);

    //create the  beams
    for (i = 0; i < 4; i++)
    {
        vec3_t angle;
        edict_t *beam = G_Spawn();
        beam->owner = ent;
        beam->goalentity = spiral;
        beam->think = Kots_SpiralBeamThink;
        beam->s.angles[2] = i * 90; //use this angle to determine angle from center
        beam->s.frame = KOTS_SPIRAL_BEAMSIZE; //diameter
        beam->s.skinnum = 0xdcdddedf; //yellow
        beam->s.renderfx = RF_BEAM | RF_TRANSLUCENT;
        beam->s.effects = EF_TELEPORTER | EF_PLASMA;
        beam->movetype = MOVETYPE_NOCLIP;
        beam->clipmask = MASK_ALL;
        beam->model = NULL;
        beam->s.modelindex = 1;

        //set the initial beam position
        VectorSet(angle, 0, beam->s.angles[2], 0);
        AngleVectors(angle, angle, NULL, NULL);
        VectorMA(spiral->s.origin, KOTS_SPIRAL_MAX_RADIUS, angle, beam->s.origin);

        //kick off the beams movement
        beam->think(beam);
    }

    //reduce spirals by 1
    --ent->character->spiralsleft;
    ent->character->next_spiral = level.time + KOTS_SPIRAL_TIME;

    //set the damage here otherwise the initial beam move will cause damage
    spiral->dmg_radius = Kots_CharacterMunitionDamage(ent, ent->character->cur_spiral * KOTS_SPIRAL_DAMAGE);

    //ensure the maximum damage is capped
    if (spiral->dmg_radius > KOTS_SPIRAL_DAMAGE_CAP)
        spiral->dmg_radius = KOTS_SPIRAL_DAMAGE_CAP;

    spiral->dmg_radius *= FRAMETIME; //divide the damage among each frame

    //uncloak the character if cloaked
    Kots_CharacterUncloak(ent);
}
void Kots_CharacterAddPower(edict_t *ent, char *power)
{
    if (!power || power[0] == '\0')
        gi.cprintf(ent, PRINT_HIGH, "Usage: %s PowerTree\n", gi.argv(0));
    else
    {
        powerpoint_t *power_tree = Kots_GetPowerTree(power);

        if (!power_tree)
            gi.cprintf(ent, PRINT_HIGH, "%s is not a valid power.\n", power);
        else
            power_tree->add(ent);
    }
}

void Kots_CharacterSetPower(edict_t *ent)
{
    if (gi.argc() != 4)
        gi.cprintf(ent, PRINT_HIGH, "Usage: %s PlayerName Power Level\n", gi.argv(0));
    else
    {
        int i;
        edict_t *other = g_edicts + 1;
        char *name = gi.argv(1);
        char *power = gi.argv(2);
        int level = atoi(gi.argv(3));

        for (i = 0; i < game.maxclients; i++, other++)
        {
            if (!other->inuse || !other->character || !other->character->is_loggedin)
                continue;

            if (Q_stricmp(other->client->pers.netname, name) == 0)
            {
                powerpoint_t *power_tree = Kots_GetPowerTree(power);

                if (!power_tree)
                {
                    gi.cprintf(ent, PRINT_HIGH, "Unrecognized power %s.\n", power);
                    return;
                }
                else
                {
                    int *points = Kots_CharacterGetPowerLevel(other, power_tree->power);

                    if (level < 0)
                    {
                        gi.cprintf(ent, PRINT_HIGH, "Invalid power level specified.\n");
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

                        gi.cprintf(other, PRINT_HIGH, "Your %s power is now level %i thanks to %s.\n", power_tree->name, *points, ent->character->name);
                        gi.cprintf(ent, PRINT_HIGH, "You changed %s's %s level to %i.\n", other->character->name, power_tree->name, *points);
                        return;
                    }
                }
            }
        }

        gi.cprintf(ent, PRINT_HIGH, "Unable to find a player logged in with that name.\n");
    }
}

qboolean Kots_PowerCharacterCheckLevelCap(edict_t *ent, int power, int total)
{
    // The cap only applies when you're below level KOTS_POWER_TRAINING_LIMIT_LEVEL_CAP
    if (ent->character->level < KOTS_POWER_TRAINING_LIMIT_LEVEL_CAP)
    {
        switch (power)
        {
            case KOTS_POWER_SPIRAL:
                // Only allow 1 point for every 3 levels
                // todo: see if I can remove the +1 factor in this function for readability
                if (total > ((ent->character->level / 3) + 1))
                {
                    int next_level = (total - 1) * 3;
                    if (next_level > KOTS_POWER_TRAINING_LIMIT_LEVEL_CAP)
                        next_level = KOTS_POWER_TRAINING_LIMIT_LEVEL_CAP;

                    gi.cprintf(ent, PRINT_HIGH, "You cannot train this power tree again until level %d.\n", next_level);
                    return false;
                }

                break;
            default:
                break;
        }
    }

    return true;
}

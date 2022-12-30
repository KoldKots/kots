#include "kots_hook.h"
#include "kots_utils.h"
#include "kots_pweapon.h"

#define KOTS_HOOK_FIRESPEED_DEFAULT     800
#define KOTS_HOOK_FIRESPEED_LEVEL5      1200
#define KOTS_HOOK_FIRESPEED_LEVEL7      1500

#define KOTS_HOOK_PULLSPEED_DEFAULT     800
#define KOTS_HOOK_PULLSPEED_LEVEL5      1000
#define KOTS_HOOK_PULLSPEED_LEVEL7      1200

/*------------------------------------------------------------------------*/
/* GRAPPLE                                                                */
/*------------------------------------------------------------------------*/

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
    if (ent->client && ent->client->ctf_grapple)
        CTFResetGrapple(ent->client->ctf_grapple);
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
    if (self->owner->client->ctf_grapple) {
        gclient_t *cl;

        //keep track of the last time we released the hook for falling damage
        if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
            self->owner->character->last_hookrelease = level.time;

        cl = self->owner->client;
        cl->ctf_grapple = NULL;
        cl->ctf_grapplereleasetime = level.time;
        cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
        cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
        G_FreeEdict(self);
    }
}

void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other == self->owner)
        return;

    if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
        return;

    //only level 5 dexterity grapple can latch onto the sky
    if (surf && (surf->flags & SURF_SKY) && self->owner->character->cur_dexterity < 10)
    {
        CTFResetGrapple(self);
        return;
    }

    VectorCopy(vec3_origin, self->velocity);

    PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

    if (other->takedamage) {
        T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_GRAPPLE);

        //Level 7 dexterity grapple can hang on to other people
        if (self->owner->character->cur_dexterity < 7)
        {
            CTFResetGrapple(self);
            return;
        }
    }

    // Set our next release time
    // We're only allowed to keep our grapple stuck for so long
    self->owner->character->next_hookrelease = level.time + KOTS_HOOK_MAX_TIME;

    self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
    self->enemy = other;

    self->solid = SOLID_NOT;

    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_SPARKS);
    gi.WritePosition (self->s.origin);
    if (!plane)
        gi.WriteDir (vec3_origin);
    else
        gi.WriteDir (plane->normal);
    gi.multicast (self->s.origin, MULTICAST_PVS);
}

void SV_AddGravity (edict_t *ent);

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
    vec3_t hookdir, v;
    float vlen;

    if (self->owner->health <= 0)
    {
        CTFResetGrapple(self);
        return;
    }

    if (self->enemy) {
        if (self->enemy->solid == SOLID_NOT)
        {
            CTFResetGrapple(self);
            return;
        }
        if (self->enemy->solid == SOLID_BBOX)
        {
            VectorScale(self->enemy->size, 0.5, v);
            VectorAdd(v, self->enemy->s.origin, v);
            VectorAdd(v, self->enemy->mins, self->s.origin);
            gi.linkentity (self);
        } else
            VectorCopy(self->enemy->velocity, self->velocity);

        if (self->enemy->takedamage && !CheckTeamDamage (self->enemy, self->owner))
        {
            //break the cable if we can't see it anymore
            if (!loc_CanSee(self->owner, self))
            {
                CTFResetGrapple(self);
                return;
            }
            else
            {
                float volume = 1.0;


                if (self->owner->character->cur_dexterity >= 7)
                    volume = 0;
                else if (self->owner->character->cur_dexterity >= 5)
                    volume = 0.5;

                if (self->owner->client->silencer_shots)
                    volume = 0.2;

                T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
            }
        }
        if (self->enemy->deadflag) { // he died
            CTFResetGrapple(self);
            return;
        }
    }

    if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
        // pull player toward grapple
        // this causes icky stuff with prediction, we need to extend
        // the prediction layer to include two new fields in the player
        // move stuff: a point and a velocity.  The client should add
        // that velociy in the direction of the point
        vec3_t forward, up;
        int speed = KOTS_HOOK_PULLSPEED_DEFAULT;

        if (self->owner->character->cur_dexterity >= 7)
            speed = KOTS_HOOK_PULLSPEED_LEVEL7;
        else if (self->owner->character->cur_dexterity >= 5)
            speed = KOTS_HOOK_PULLSPEED_LEVEL5;

        AngleVectors (self->owner->client->v_angle, forward, NULL, up);
        VectorCopy(self->owner->s.origin, v);
        v[2] += self->owner->viewheight;
        VectorSubtract (self->s.origin, v, hookdir);

        vlen = VectorLength(hookdir);

        if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
            vlen < 64) {

            self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
            self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
        }

        VectorNormalize (hookdir);

        if (vlen >= 64)
            VectorScale(hookdir, speed, hookdir);
        else
            VectorScale(hookdir, 200, hookdir);

        VectorCopy(hookdir, self->owner->velocity);

        if (vlen >= 64)
            SV_AddGravity(self->owner);

        VectorCopy(self->owner->velocity, self->owner->client->oldvelocity);
    }
}

// move the two ends of the laser beam to the proper positions
void CTFGrapple_Think(edict_t *self) {
    vec3_t forward, right, offset, start;

    // stupid check for NULL pointers ...
    if(!(self && self->owner && self->owner->owner && self->owner->owner->client)) {
        G_FreeEdict(self);
        return;
    }

    // put start position into start
    AngleVectors (self->owner->owner->client->v_angle, forward, right, NULL);
    VectorSet(offset, 0, 0, self->owner->owner->viewheight-8);
    P_ProjectSource(self->owner->owner->client, self->owner->owner->s.origin, offset, forward, right, start);

    // move the two ends
    VectorCopy(start, self->s.origin);
    VectorCopy(self->owner->s.origin, self->s.old_origin);

    gi.linkentity(self);

    // set up to go again
    self->nextthink = level.time + FRAMETIME;
}

void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
    edict_t *grapple;
    trace_t tr;

    VectorNormalize (dir);

    grapple = G_Spawn();
    grapple->movetype = MOVETYPE_FLYMISSILE;
    grapple->solid = SOLID_BBOX;
    grapple->clipmask = MASK_SHOT;
    grapple->owner = self;

    vectoangles (dir, grapple->s.angles);
    VectorScale (dir, speed, grapple->velocity);

    grapple->touch = CTFGrappleTouch;
    grapple->dmg = damage;
    self->client->ctf_grapple = grapple;
    self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook

    VectorCopy (start, grapple->s.origin);
    VectorCopy (start, grapple->s.old_origin);

    VectorClear (grapple->mins);
    VectorClear (grapple->maxs);

    edict_t *laser;

    laser = G_Spawn();
    laser->movetype = MOVETYPE_NONE;
    laser->solid = SOLID_NOT;
    laser->s.renderfx |= RF_BEAM | RF_TRANSLUCENT;
    laser->s.modelindex = 1;         // must be non-zero
    laser->owner = grapple;

    // set the beam diameter
    laser->s.frame = 4;

    // set the color
    if (self->character->laserhook_color)
        laser->s.skinnum = self->character->laserhook_color;
    else
        laser->s.skinnum = LASERHOOK_COLOR_RED;

    laser->think = CTFGrapple_Think;

    VectorSet(laser->mins, -8, -8, -8);
    VectorSet(laser->maxs, 8, 8, 8);
    gi.linkentity(laser);

    self->spawnflags |= 0x80000001;
    self->svflags &= ~SVF_NOCLIENT;
    CTFGrapple_Think(laser);

    gi.linkentity(grapple);

    tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
    if (tr.fraction < 1.0)
    {
        VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
        grapple->touch (grapple, tr.ent, NULL, NULL);
    }
}

void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
    //SWB - prevent refiring the grapple if it's already out
    if (!ent->client->ctf_grapple || !ent->client->ctf_grapple->inuse || ent->client->ctf_grapple->owner != ent)
    {
        vec3_t  forward, right;
        vec3_t  start;
        vec3_t  offset;
        float volume = 1.0;
        int speed = KOTS_HOOK_FIRESPEED_DEFAULT;

        if (ent->character->cur_dexterity >= 7)
            speed = KOTS_HOOK_FIRESPEED_LEVEL7;
        else if (ent->character->cur_dexterity >= 5)
            speed = KOTS_HOOK_FIRESPEED_LEVEL5;

        if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
            return; // it's already out

        // If it's been less than a second since the last hook then don't allow us to shoot it again
        if ((ent->character->last_hookrelease + KOTS_HOOK_COOLDOWN) > level.time)
            return;

        AngleVectors (ent->client->v_angle, forward, right, NULL);
    //  VectorSet(offset, 24, 16, ent->viewheight-8+2);
        VectorSet(offset, 0, 0, ent->viewheight-8+2);
        VectorAdd (offset, g_offset, offset);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        VectorScale (forward, -2, ent->client->kick_origin);
        ent->client->kick_angles[0] = -1;

        if (ent->character->cur_dexterity >= 7)
            volume = 0;
        else if (ent->character->cur_dexterity >= 5)
            volume = 0.5;

        if (ent->client->silencer_shots)
            volume = 0.2;

        gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("flyer/Flyatck3.wav"), volume, ATTN_NORM, 0);
        CTFFireGrapple (ent, start, forward, damage, speed, effect);

        PlayerNoise(ent, start, PNOISE_WEAPON);
    }
}


void CTFWeapon_Grapple_Fire (edict_t *ent)
{
    int     damage;

    damage = 2;
    CTFGrappleFire (ent, vec3_origin, damage, 0);
    ent->client->ps.gunframe++;
}

void CTFWeapon_Grapple (edict_t *ent)
{
    static int  pause_frames[]  = {10, 18, 27, 0};
    static int  fire_frames[]   = {6, 0};
    int prevstate;

    // if the the attack button is still down, stay in the firing frame
    if ((ent->client->buttons & BUTTON_ATTACK) &&
        ent->client->weaponstate == WEAPON_FIRING &&
        ent->client->ctf_grapple)
        ent->client->ps.gunframe = 9;

    if (!(ent->client->buttons & BUTTON_ATTACK) &&
        ent->client->ctf_grapple) {
        CTFResetGrapple(ent->client->ctf_grapple);
        if (ent->client->weaponstate == WEAPON_FIRING)
            ent->client->weaponstate = WEAPON_READY;
    }


    if (ent->client->newweapon &&
        ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
        ent->client->weaponstate == WEAPON_FIRING) {
        // he wants to change weapons while grappled
        ent->client->weaponstate = WEAPON_DROPPING;
        ent->client->ps.gunframe = 32;
    }

    prevstate = ent->client->weaponstate;
    Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames,
        CTFWeapon_Grapple_Fire);

    // if we just switched back to grapple, immediately go to fire frame
    if (prevstate == WEAPON_ACTIVATING &&
        ent->client->weaponstate == WEAPON_READY &&
        ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
        if (!(ent->client->buttons & BUTTON_ATTACK))
            ent->client->ps.gunframe = 9;
        else
            ent->client->ps.gunframe = 5;
        ent->client->weaponstate = WEAPON_FIRING;
    }
}

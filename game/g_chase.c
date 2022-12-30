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

//SWB - function to update all clients chasing specified edict
void UpdateAllChaseCams(edict_t *chase)
{
    int i;
    edict_t *other;

    // update chase cam if being followed
    for (i = 1; i <= maxclients->value; i++)
    {
        other = g_edicts + i;
        if (other->inuse && other->client->chase_target == chase)
            UpdateChaseCam(other);
    }
}

void UpdateChaseCam(edict_t *ent)
{
    vec3_t o, ownerv, goal;
    edict_t *targ;
    vec3_t forward, right;
    trace_t trace;
    int i;
    vec3_t oldgoal;
    vec3_t angles;

    if (!ent->client->chase_target)
        return;

    //SWB - switch to chase mode from free mode
    if (ent->client->chase_mode == KOTS_SPECTATOR_MODE_FREE)
        ent->client->chase_mode = KOTS_SPECTATOR_MODE_CHASE;

    // is our chase target gone?
    if (!ent->client->chase_target->inuse
        || (ent->client->chase_target->client && ent->client->chase_target->client->resp.spectator)
        || (!ent->client->chase_target->character || !ent->client->chase_target->character->is_loggedin)
        || ((ent->client->chase_target->svflags & SVF_MONSTER) && ent->client->chase_target->health <= 0)
        || level.intermissiontime) {
        edict_t *old = ent->client->chase_target;
        ChaseNext(ent);
        if (ent->client->chase_target == old) {

            //SWB - switch back to free look spectator mode
            ent->client->chase_mode = KOTS_SPECTATOR_MODE_FREE;

            ent->client->chase_target = NULL;
            ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
            return;
        }
    }

    targ = ent->client->chase_target;

    VectorCopy(targ->s.origin, ownerv);
    VectorCopy(ent->s.origin, oldgoal);

    ownerv[2] += targ->viewheight;

    //SWB - account for monsters
    if (targ->client)
        VectorCopy(targ->client->v_angle, angles);
    else
    {
        ownerv[2] += (targ->mins[2] / 2) + (targ->maxs[2] / 2);
        ownerv[1] += (targ->mins[1] / 2) + (targ->maxs[1] / 2); //center the target
        ownerv[0] += (targ->mins[0] / 2) + (targ->maxs[0] / 2); //center the target
        VectorCopy(targ->s.angles, angles);
    }

    if (angles[PITCH] > 56)
        angles[PITCH] = 56;
    AngleVectors (angles, forward, right, NULL);
    VectorNormalize(forward);

    if (ent->client->chase_mode == KOTS_SPECTATOR_MODE_CHASE)
    {
        if (targ->client)
        {
            ownerv[2] += 20;
            VectorMA(ownerv, -50, forward, o);
        }
        else
        {
            ownerv[2] += 40;
            VectorMA(ownerv, -50, forward, o);
        }


        if (o[2] < targ->s.origin[2] + 20)
            o[2] = targ->s.origin[2] + 20;
    }
    else
    {
        //move back a little for clients and forward a little for monsters
        if (targ->client)
        {
            VectorMA(ownerv, targ->maxs[0], forward, o);
            o[2] = targ->s.origin[2] + targ->viewheight;
        }
        else
        {
            VectorMA(ownerv, targ->maxs[0] + 6, forward, o);
            o[2] = targ->s.origin[2] + targ->maxs[2] - 10;
        }
    }

    // jump animation lifts
    if (!targ->groundentity && targ->client) //SWB - this should only be for chasing clients
        o[2] += 16;

    trace = gi.trace(targ->s.origin, vec3_origin, vec3_origin, o, targ, MASK_SOLID);

    VectorCopy(trace.endpos, goal);

    VectorMA(goal, 2, forward, goal);

    // pad for floors and ceilings
    VectorCopy(goal, o);
    o[2] += 6;
    trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
    if (trace.fraction < 1) {
        VectorCopy(trace.endpos, goal);
        goal[2] -= 6;
    }

    VectorCopy(goal, o);
    o[2] -= 6;
    trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
    if (trace.fraction < 1) {
        VectorCopy(trace.endpos, goal);
        goal[2] += 6;
    }

    if (targ->deadflag)
        ent->client->ps.pmove.pm_type = PM_DEAD;
    else
        ent->client->ps.pmove.pm_type = PM_FREEZE;

    VectorCopy(goal, ent->s.origin);

    //SWB - account for monsters
    if (targ->client)
    {
        for (i=0 ; i<3 ; i++)
            ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
    }
    else
    {

    }

    if (targ->deadflag) {
        ent->client->ps.viewangles[ROLL] = 40;
        ent->client->ps.viewangles[PITCH] = -15;

        //SWB - account for monsters
        if (targ->client)
            ent->client->ps.viewangles[YAW] = targ->client->killer_yaw;
        else
            ent->client->ps.viewangles[YAW] = targ->s.angles[YAW];

    } else {

        //SWB - account for monsters
        if (targ->client)
        {
            VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
            VectorCopy(targ->client->v_angle, ent->client->v_angle);
        }
        else
        {
            VectorCopy(targ->s.angles, ent->client->ps.viewangles);
            VectorCopy(targ->s.angles, ent->client->v_angle);

            if (ent->client->chase_mode == KOTS_SPECTATOR_MODE_CHASE)
            {
                ent->client->ps.viewangles[0] = 20;
                ent->client->v_angle[0] = 20;
            }
        }
    }

    // Aldarn - blend chasers screen if theyre chasing someone cloaked
    if(targ->character->is_cloaked)
    {
        VectorSet(ent->client->damage_blend, 0, 0, 0);
        ent->client->damage_alpha = 0.33;
    }

    ent->viewheight = 0;
    ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
    gi.linkentity(ent);
}

void ChaseNext(edict_t *ent)
{
    int i;
    edict_t *e;

    if (!ent->client->chase_target)
        return;

    //SWB - don't allow chase at intermission
    if (level.intermissiontime)
        return;

    i = ent->client->chase_target - g_edicts;

    //SWB - account for monsters
    do {
        i++;
        if (i >= maxentities->value)
            i = 1;

        e = g_edicts + i;
        if (!e->inuse)
            continue;
        if (e->client && !e->client->resp.spectator)
            break;
        if ((e->svflags & SVF_MONSTER) && !e->deadflag)
            break;
    } while (e != ent->client->chase_target);

    ent->client->chase_target = e;
    ent->client->update_chase = true;
}

void ChasePrev(edict_t *ent)
{
    int i;
    edict_t *e;

    if (!ent->client->chase_target)
        return;

    //SWB - don't allow chase at intermission
    if (level.intermissiontime)
        return;

    i = ent->client->chase_target - g_edicts;
    //SWB - account for monsters
    do {
        i--;
        if (i < 1)
            i = maxentities->value - 1;

        e = g_edicts + i;
        if (!e->inuse)
            continue;
        if (e->client && !e->client->resp.spectator)
            break;
        if ((e->svflags & SVF_MONSTER) && !e->deadflag)
            break;
    } while (e != ent->client->chase_target);

    ent->client->chase_target = e;
    ent->client->update_chase = true;
}

void GetChaseTarget(edict_t *ent)
{
    int i;
    edict_t *other;

    for (i = 1; i <= maxclients->value; i++) {
        other = g_edicts + i;

        if (other->inuse && !other->client->resp.spectator) {
            ent->client->chase_target = other;
            ent->client->update_chase = true;
            UpdateChaseCam(ent);
            return;
        }
    }

    //SWB - account for monsters
    for ( ; i < maxentities->value; i++)
    {
        other = g_edicts + i;

        if (other->inuse && (other->svflags & SVF_MONSTER) && !other->deadflag)
        {
            ent->client->chase_target = other;
            ent->client->update_chase = true;
            UpdateChaseCam(ent);
            return;
        }
    }

    //SWB - free look spectator mode
    ent->client->chase_mode = KOTS_SPECTATOR_MODE_FREE;

    gi.centerprintf(ent, "No other players to chase.");
}


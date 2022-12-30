#include "kots_pweapon.h"

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
    vec3_t  _distance;

    VectorCopy (distance, _distance);
    if (client->pers.hand == LEFT_HANDED)
        _distance[1] *= -1;
    else if (client->pers.hand == CENTER_HANDED)
        _distance[1] = 0;
    G_ProjectSource (point, _distance, forward, right, result);
}

void VectorCreateVelocity (vec3_t start, vec3_t end, vec3_t velocity)
{
    velocity[0] = (end[0] - start[0]) / FRAMETIME;
    velocity[1] = (end[1] - start[1]) / FRAMETIME;
    velocity[2] = (end[2] - start[2]) / FRAMETIME;
}

void Laser_PreThink(edict_t *self)
{   
    VectorCopy(self->moveinfo.start_origin, self->s.origin);
    VectorCopy(self->moveinfo.end_origin, self->s.old_origin);
    //VectorCreateVelocity(self->s.old_origin, self->s.origin, self->velocity);
    VectorSubtract(self->s.old_origin, self->s.origin, self->velocity);
    VectorScale(self->velocity, 10, self->velocity);

    //if the radius isn't the final length then adjust it
    if (self->s.frame != self->health)
    {
        int difference = self->health - self->max_health;
        float total_time = self->delay - self->wait;
        float time_ellapsed = level.time - self->wait;
        self->s.frame = self->max_health + (int)((difference / total_time) * time_ellapsed);
    }

    if (level.time >= self->delay) 
    {
        G_FreeEdict(self);
        return;
    }

    self->nextthink = level.time + FRAMETIME;
    gi.linkentity(self);
}


/*
colors
The way laser colors work is based on color palette indexes found in pak0 pics/colormap.pcx
Each byte of the long represents 1 stage of the laser and each of the 4 bytes get cycled through in order

0xf2f2f0f0 - red
0xd0d1d2d3 - green
0xf3f3f1f1 - blue
0xdcdddedf - yellow
0xe0e1e2e3 - orange
0x80818283 - dark purple
0x70717273 - light blue
0x90919293 - different green
0xb0b1b2b3 - purple
0x40414243 - different red
0xe2e5e3e6 - orange
0xd0f1d3f3 - blue and green
0xf2f3f0f1 - inner = red, outer = blue
0xf3f2f1f0 - inner = blue, outer = red
0xdad0dcd2 - inner = green, outer = yellow
0xd0dad2dc - inner = yellow, outer = green 
*/
edict_t *CreateLaser(edict_t *owner, vec3_t start, vec3_t end, int effects, int color, int diameter, int final_diameter, float time)
{
    edict_t *laser = G_Spawn();
    laser->classname = "laser"; 
    laser->owner = owner;
    laser->think = Laser_PreThink;
    laser->nextthink = level.time + FRAMETIME;
    laser->s.frame = diameter;
    laser->s.skinnum = color;
    laser->s.renderfx = RF_BEAM | RF_TRANSLUCENT;
    laser->s.effects = effects;
    laser->movetype = MOVETYPE_NOCLIP;
    laser->clipmask = MASK_ALL;
    laser->model = NULL;
    laser->s.modelindex = 1;
    laser->max_health = diameter; //store original diameter here 
    laser->health = final_diameter; //store final diameter here
    laser->wait = level.time; //store original time created here for calculating final size
    laser->delay = level.time + time;

    //store the start and end points between frames
    VectorCopy(start, laser->moveinfo.start_origin);
    VectorCopy(end, laser->moveinfo.end_origin);

    //kick off the lasers movement
    laser->think(laser);

    return laser;
}

#include <time.h>
#include <pthread.h>
#include "kots_runes.h"
#include "kots_dbthread.h"
#include "kots_array.h"
#include "kots_mysql_helper.h"
#include "kots_server.h"
#include "kots_utils.h"
#include "kots_items.h"

#define Kots_RuneIncreaseStat(stat, val, max)   {stat += val; if (stat > max) stat = max;}
#define KOTS_RUNE_HELATH    10

static array_t *runes = NULL;
static array_t *runes_on_map = NULL;
static time_t next_runecheck = 0;

//static function prototypes
static void Kots_RunesUpdate(array_t *temp_runes);

void Kots_RuneDie(edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    targ->enemy = NULL;
    targ->health = KOTS_RUNE_HELATH;
    Kots_RuneExplode(targ);
}

void Kots_RuneTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other == ent->owner || other == ent)
        return;

    //play rune drop sound
    if (!other->takedamage && other->solid == SOLID_BSP)
    {
        if (ent->enemy != other)
        {
            gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/HGRENB1A.wav"), 1, ATTN_NORM, 0);
            ent->enemy = other; //keep track of the last thing we hit
        }

        ent->count = 0; //reset hit count
    }
    else //we hit something that wouldn't make a bounce so reset the enemy to let us bounce again 
        ent->enemy = NULL;

    Touch_Item (ent, other, plane, surf);
}

void Kots_RunePreThink(edict_t *ent)
{
    if (ent->enemy)
    {
        ent->count++;

        if (ent->count > 1)
        {
            ent->enemy = NULL;
            ent->count = 0;
        }
    }

    if (ent->waterlevel > 0)
    {
        if (ent->velocity[2] < 150)
        {
            trace_t tr;
            vec3_t dest, *origin;

            ent->velocity[2] += 100;
            if (ent->velocity[2] > 150)
                ent->velocity[2] = 150;
            
            //trace from the destination down to see if we're going to be above water
            origin = CenterEdict(ent);
            VectorCopy((*origin), dest);
            dest[2] += ent->velocity[2] * FRAMETIME;
            tr = gi.trace(dest, NULL, NULL, (*origin), ent, MASK_WATER);

            //if we hit water 
            if (tr.fraction < 1.0)
            {
                ent->velocity[2] = ((tr.endpos[2] - (*origin)[2]) / FRAMETIME);
                ent->velocity[2] += 50 + rand() % 28;
            }
        }
    }
}

void Kots_RuneMakeTouchable(edict_t *ent)
{
    //TODO: Perhaps runes should expire, but after quite a while to make room for new ones
    //runes teleport away after inactivity
    ent->nextthink = level.time + 180;
    ent->think = Kots_RuneTeleport;

    //just remove the owner to make it touchable because we need the same function for sound
    ent->owner = NULL;
}

qboolean Kots_RunePickup(edict_t *ent, edict_t *other)
{
    if (!other->character || !other->character->is_loggedin || other->character->rune)
        return false;

    if (other->character->spree >= 16)
        return false;

    //reassociate this drop with the player that picked it up
    Kots_RuneRemoveDropped(ent, ent->rune);
    Kots_RuneAddDropped(other, ent->rune);

    if (ent->rune->pickup_text[0] != '\0')
        gi.cprintf(other, PRINT_HIGH, "%s\n", ent->rune->pickup_text);

    Kots_RuneAddAbilities(other, ent->rune);
    other->character->rune = ent->rune;
    return true;
}

edict_t *Kots_RuneDrop(edict_t *ent)
{
    edict_t *rune;

    //don't have a rune so nothing to drop
    if (!ent->character->rune)
    {
        gi.cprintf(ent, PRINT_HIGH, "You do not have a rune to drop.\n");
        return NULL;
    }

    //remove the abilities from the player
    Kots_RuneRemoveAbilities(ent, ent->character->rune);

    //drop the rune but don't add it to the dropped list it's already there
    rune = Kots_RuneDropAt(ent, ent->character->rune);

    //now remove the reference to the rune from the player
    ent->character->rune = NULL;

    return rune;
}

//turn the specified entity into a rune
void Kots_RuneCreate(edict_t *ent, rune_t *rune)
{
    ent->s.modelindex = gi.modelindex(rune->model_name);
    ent->s.effects |= rune->effects | EF_HYPERBLASTER;
    ent->s.renderfx |= rune->renderfx;
    ent->rune = rune;
}

void Kots_RuneAddAbilities(edict_t *ent, rune_t *rune)
{
    //add player points
    Kots_RuneIncreaseStat(ent->character->cur_dexterity, rune->dexterity, 10);
    Kots_RuneIncreaseStat(ent->character->cur_strength, rune->strength, 10);
    Kots_RuneIncreaseStat(ent->character->cur_karma, rune->karma, 10);
    Kots_RuneIncreaseStat(ent->character->cur_wisdom, rune->wisdom, 10);
    Kots_RuneIncreaseStat(ent->character->cur_technical, rune->technical, 10);
    Kots_RuneIncreaseStat(ent->character->cur_spirit, rune->spirit, 10);
    Kots_RuneIncreaseStat(ent->character->cur_rage, rune->rage, 10);
    ent->character->cur_vitarmor += rune->vitarmor;
    ent->character->cur_vithealth += rune->vithealth;
    ent->character->cur_munition += rune->munition;

    //add power points
    ent->character->cur_expack += rune->expack;
    ent->character->cur_spiral += rune->spiral;
    ent->character->cur_bide += rune->bide;
    ent->character->cur_kotsthrow += rune->kotsthrow;
    ent->character->cur_antiweapon += rune->antiweapon;

    //calculate new power charges
    if (rune->expack)
        ent->character->expacksleft += rune->expack * 2;
    if (rune->spiral)
        ent->character->spiralsleft += rune->spiral * 2;
    if (rune->kotsthrow)
        ent->character->throwsleft += rune->kotsthrow * 2;
    if (rune->bide)
        ent->character->bidesleft += rune->bide;

    //add weapon points
    ent->character->cur_sabre += rune->sabre;
    ent->character->cur_shotgun += rune->shotgun;
    ent->character->cur_machinegun += rune->machinegun;
    ent->character->cur_chaingun += rune->chaingun;
    ent->character->cur_supershotgun += rune->supershotgun;
    ent->character->cur_grenade += rune->grenade;
    ent->character->cur_grenadelauncher += rune->grenadelauncher;
    ent->character->cur_rocketlauncher += rune->rocketlauncher;
    ent->character->cur_hyperblaster += rune->hyperblaster;
    ent->character->cur_railgun += rune->railgun;
    ent->character->cur_bfg += rune->bfg;

    //give weapons if the rune adds to that weapon
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Shotgun")), rune->shotgun > 0);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Super Shotgun")), rune->supershotgun > 0);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Machinegun")), rune->machinegun > 0);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Chaingun")), rune->chaingun > 0);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Grenade Launcher")), rune->grenadelauncher > 0);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Rocket Launcher")), rune->rocketlauncher > 0);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Hyperblaster")), rune->hyperblaster > 0);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Railgun")), rune->railgun > 0);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("BFG10K")), rune->bfg > 0);

    //add other bonuses
    ent->character->tballs += rune->tballs;
}

void Kots_RuneRemoveAbilities(edict_t *ent, rune_t *rune)
{
    //reset stats back to normal
    Kots_CharacterResetStats(ent);

    //calculate new power charges
    if (rune->expack)
        ent->character->expacksleft -= rune->expack * 2;
    if (rune->spiral)
        ent->character->spiralsleft -= rune->spiral * 2;
    if (rune->kotsthrow)
        ent->character->throwsleft -= rune->kotsthrow * 2;
    if (rune->bide)
        ent->character->bidesleft -= rune->bide;

    //add other bonuses
    ent->character->tballs -= rune->tballs;
}

void Kots_RuneIndexAll()
{
    ULONG i;
    rune_t *rune;

    for (i = 0; i < runes->length; i++)
    {
        rune = (rune_t *)Array_GetValueAt(runes, i);
        gi.modelindex(rune->model_name);
        gi.imageindex(rune->image_name);
        gi.soundindex(rune->sound_name);
    }
}

rune_t *Kots_RuneFindById(int id)
{
    ULONG i;
    rune_t *rune;

    for (i = 0; i < runes->length; i++)
    {
        rune = (rune_t *)Array_GetValueAt(runes, i);

        if (rune->id == id)
            return rune;
    }

    return NULL;
}

rune_t *Kots_RuneFindByName(char *name)
{
    ULONG i;
    rune_t *rune;

    for (i = 0; i < runes->length; i++)
    {
        rune = (rune_t *)Array_GetValueAt(runes, i);

        if (Q_stricmp(rune->name, name) == 0)
            return rune;
    }

    return NULL;
}

void Kots_RunesInit()
{
    //Create arrays if they don't already exist
    if (!runes)
    {
        runes = Array_Create(NULL, MAX_CLIENTS, MAX_CLIENTS);
        runes_on_map = Array_Create(NULL, KOTS_RUNES_MAX_DROPPED, KOTS_RUNES_MAX_DROPPED);
        next_runecheck = time(NULL) + 5; //wait a little bit before updating rune list
    }
    else //otherwise index all the current runes for the new map
        Kots_RuneIndexAll();

    //clear the list of runes dropped for the current map
    Array_DeleteAll(runes_on_map);
}

void Kots_RunesFreeInfo()
{
    if (runes)
    {
        Array_Delete(runes);
        Array_Delete(runes_on_map);

        runes = NULL;
        runes_on_map = NULL;
    }
}


void Kots_RunesProcessOutput(jobitem_t *job)
{
    array_t *temp_runes = (array_t *)job->args;
    
    //process result
    switch (job->result)
    {
    case KOTS_SERVER_SUCCESS:
        //update the runes
        Kots_RunesUpdate(temp_runes);
        break;

    case KOTS_SERVER_CONNECTIONDOWN:
        gi.dprintf("DB: ERROR - Failed to obtain rune list from server. The server may be down.\n");
        break;
    case KOTS_SERVER_UNKNOWNERROR:
        gi.dprintf("DB: ERROR - Failed to obtain rune list from server. An unknown error occurred.\n");
        break;
    }

    //delete the runes and don't leave the pointer dangling because it will be freed later
    if (temp_runes)
    {
        Array_Delete(temp_runes);
        job->args = NULL;
    }

    //set the next check interval from this time
    next_runecheck = time(NULL) + KOTS_RUNES_CHECK_INTERVAL;
}

static void Kots_RunesUpdate(array_t *temp_runes)
{
    dropped_rune_t *dropped;
    rune_t *rune;
    edict_t *ent;
    ULONG i;

    if (!runes)
    {
        Array_DeleteAll(temp_runes);
        gi.dprintf("DB: ERROR - Runes were not updated because the game may be unloading.\n");
        return;
    }

    //remove runes from all players with runes until we determine if any were deleted
    for (i = 0; i < runes_on_map->length; i++)
    {
        dropped = Array_GetValueAt(runes_on_map, i);

        if (dropped->owner->character)
        {
            Kots_RuneRemoveAbilities(dropped->owner, dropped->rune);
            dropped->owner->character->rune = NULL;
        }
    }

    //delete all current runes and copy over new ones
    Array_DeleteAll(runes);
    Array_CopyValues(temp_runes, runes);
    Array_ClearAll(temp_runes);
    Array_Shuffle(runes, 5);

    //now determine if any runes on the map were deleted and if so remove them
    for (i = 0; i < runes_on_map->length; i++)
    {
        dropped = Array_GetValueAt(runes_on_map, i);
        rune = Kots_RuneFindById(dropped->id);

        //rune could not be found so it must no longer exist
        if (!rune)
        {
            //if the owner is a character alert them of the change
            if (dropped->owner->character)
                gi.cprintf(dropped->owner, PRINT_HIGH, "The rune you were holding no longer exists.\n");

            else //otherwise the owner is the actual edict so we need to free it
                G_FreeEdict(dropped->owner);

            //delete the dropped rune from the array and check the same index again
            Array_DeleteAt(runes_on_map, i);
            i--;
        }

        else
        {
            //update the rune references and add the abilities back (possibly with changes)
            dropped->rune = rune;

            if (dropped->owner->character)
            {
                dropped->owner->character->rune = rune;
                Kots_RuneAddAbilities(dropped->owner, rune);
            }
            else
            {
                Kots_RuneCreate(dropped->owner, rune);
            }
        }
    }

    //now ensure that all the runes are indexed
    Kots_RuneIndexAll();

    //finally check to ensure that all characters with special runes get them or they're at least on the map
    for (i = 0, ent = g_edicts + 1; i < (ULONG)maxclients->value; i++, ent++)
    {
        if (ent->inuse && ent->character && ent->character->is_loggedin)
            Kots_RuneCheckAssociated(ent);
    }
    
    //alert the server that the new runes were loaded
    gi.dprintf("DB: Successfully updated rune list from server...\n");
}

void Kots_RunesCheckLoad()
{
    time_t now = time(NULL);

    //next_runecheck will be 0 until the last rune load finishes
    if (next_runecheck != 0 && now >= next_runecheck)
    {
        //reset the next interval until finished
        next_runecheck = 0;

        //add the job to the queue
        Kots_AddRuneUpdateJob(Kots_RunesProcessOutput);
    }
}

qboolean Kots_RuneIsDropped(int rune_id)
{
    dropped_rune_t *dropped;
    ULONG i;

    for (i = 0; i < runes_on_map->length; i++)
    {
        dropped = Array_GetValueAt(runes_on_map, i);

        if (dropped->id == rune_id)
            return true;
    }

    return false;
}

qboolean Kots_RuneIsPersisted(int rune_id)
{
    int i;
    edict_t *ent;

    ent = g_edicts + 1;
    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (!ent->inuse)
            continue;

        if (ent->client->pers.kots_persist.rune_id == rune_id)
            return true;
    }

    return false;
}

void Kots_RuneCheckAssociated(edict_t *ent)
{
    rune_t *rune;

    //check to see if the character has a special rune and it isn't already dropped
    if (ent->character->rune_id && !Kots_RuneIsDropped(ent->character->rune_id) && !Kots_RuneIsPersisted(ent->character->rune_id))
    {
        rune = Kots_RuneFindById(ent->character->rune_id);

        //if the rune was found
        if (rune)
        {
            //if the player doesn't have another rune then give it to him
            if (!ent->character->rune)
            {
                ent->character->rune = rune;
                Kots_RuneAddAbilities(ent, rune);
                Kots_RuneAddDropped(ent, rune);
            }
            else //otherwise spawn the rune somewhere on the map
                Kots_RuneSpawn(rune);
        }
    }
}

void Kots_RuneAddDropped(edict_t *ent, rune_t *rune)
{
    dropped_rune_t *dropped = malloc(sizeof(*dropped));
    dropped->id = rune->id;
    dropped->rune = rune;
    dropped->owner = ent;

    Array_PushBack(runes_on_map, dropped);
}

void Kots_RuneRemoveDropped(edict_t *ent, rune_t *rune)
{
    ULONG i;
    dropped_rune_t *dropped;

    //delete the first matching owner and rune combo
    for (i = 0; i < runes_on_map->length; i++)
    {
        dropped = Array_GetValueAt(runes_on_map, i);

        if (dropped->owner == ent && dropped->rune == rune)
        {
            Array_DeleteAt(runes_on_map, i);
            break;
        }
    }
}

void Kots_RuneSpawn(rune_t *rune)
{
    //TODO: Should probably come up with better places to drop these
    edict_t *spot = SelectRandomDeathmatchSpawnPoint();
    Kots_RuneDropAt(spot, rune);
}

edict_t *Kots_RuneDropAt(edict_t *spot, rune_t *rune)
{
    float dir = 22.5;
    edict_t *ent;
    gitem_t *item = FindItem("Kots Rune");

    //change the angle before dropping when dead
    if (spot->character && spot->health <= 0)
    {
        if (spot->client)
            spot->client->v_angle[YAW] += dir;
        else
            spot->s.angles[YAW] += dir;
    }

    //remove rune from dropped list
    //it will be added back later with the new owner if successful
    Kots_RuneRemoveDropped(spot, rune); //assume spot is the owner

    //drop and create the rune
    ent = Drop_Item_ex(spot, item, rune->mins, rune->maxs);
    ent->think = Kots_RuneMakeTouchable;
    ent->touch = Kots_RuneTouch;
    ent->prethink = Kots_RunePreThink;
    ent->die = Kots_RuneDie;
    ent->health = KOTS_RUNE_HELATH;
    ent->takedamage = DAMAGE_YES;
    Kots_RuneCreate(ent, rune);

    //reset the angle back after dropping
    if (spot->character && spot->health <= 0)
    {
        if (spot->client)
            spot->client->v_angle[YAW] -= dir;
        else
            spot->s.angles[YAW] -= dir;
    }

    //runes model could not be indexed so we're just going to ditch it
    if (!ent->s.modelindex)
    {
        G_FreeEdict(ent);
        return NULL;
    }

    //add back to drop list with new owner
    Kots_RuneAddDropped(ent, rune);

    return ent;
}

void Kots_RuneDeathDrop(edict_t *ent)
{
    //if the character had a rune then drop it instead of a new one
    if (ent->character->rune)
    {
        Kots_RuneDrop(ent);
        return;
    }

    //if we are still over the rune limit never drop anything 
    if (runes_on_map->length >= KOTS_RUNES_MAX_DROPPED)
        return;

    //only a certain % chance a rune will drop
    if (Kots_RandRound(0.05))
    {
        rune_t *rune = Kots_RunePickRandom();

        //if a rune was picked spawn it
        if (rune)
            Kots_RuneDropAt(ent, rune);
    }
}

rune_t *Kots_RunePickRandom()
{
    if (runes->length > 0)
    {
        rune_t *rune = NULL;
        ULONG start = rand() % runes->length;
        ULONG i = start;
        float value;

        do
        {
            rune = Array_GetValueAt(runes, i);

            //don't drop more than one of the same rune on the map
            if (!Kots_RuneIsDropped(rune->id))
            {
                if (rune->rarity > 0)
                {
                    value = random();

                    if (value <= rune->rarity)
                        return rune;
                }
            }

            if (++i >= runes->length)
                i = 0;
        } while (i != start);
    }

    return NULL;
}

void Kots_RuneGive(edict_t *ent, int rune_id)
{
    rune_t *rune = Kots_RuneFindById(rune_id);

    if (!rune)
    {
        gi.dprintf("Unable to find rune with id %d.\n", rune_id);
        return;
    }

    ent->character->rune = rune;
    Kots_RuneAddAbilities(ent, rune);
    Kots_RuneAddDropped(ent, rune);
}

void Kots_RuneExplode(edict_t *ent)
{
    //TODO: Should probably come up with better places to drop these
    edict_t *spot = SelectRandomDeathmatchSpawnPoint();
    vec3_t forward, right;

    //spawn explosion at previous location
    gi.WriteByte (svc_temp_entity);
    if (ent->waterlevel)
        gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
    else
        gi.WriteByte (TE_ROCKET_EXPLOSION);
    gi.WritePosition (ent->s.origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);

    ent->s.event = EV_OTHER_TELEPORT;
    ent->nextthink = level.time + 1;
    ent->think = Kots_RuneMakeTouchable;

    //move the rune to the new location and set it's velocity
    AngleVectors(spot->s.angles, forward, right, NULL);
    VectorCopy(spot->s.origin, ent->s.origin);
    VectorScale (forward, 100, ent->velocity);
    ent->velocity[2] = 300;
}

void Kots_RuneTeleport(edict_t *ent)
{
    //TODO: Should probably come up with better places to drop these
    edict_t *spot = SelectRandomDeathmatchSpawnPoint();
    vec3_t forward, right;

    //spawn explosion at previous location
    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_TELEPORT_EFFECT);
    gi.WritePosition (ent->s.origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);

    ent->s.event = EV_OTHER_TELEPORT;
    ent->nextthink = level.time + 1;
    ent->think = Kots_RuneMakeTouchable;

    //move the rune to the new location and set it's velocity
    AngleVectors(spot->s.angles, forward, right, NULL);
    VectorCopy(spot->s.origin, ent->s.origin);
    VectorScale (forward, 100, ent->velocity);
    ent->velocity[2] = 300;
}

int Kots_RunesOnMap()
{
    return runes_on_map->length;
}

void Kots_RunesList(edict_t *ent)
{
    ULONG i;
    rune_t *rune;

    Kots_SlowPrint(ent, "\n------====== Rune List ======------\n");
    for (i = 0; i < runes->length; i++)
    {
        rune = (rune_t *)Array_GetValueAt(runes, i);
        Kots_SlowPrint(ent, "%s\n", rune->name);
    }
}

void Kots_RunesShow(edict_t *ent)
{
    ULONG i;
    dropped_rune_t *dropped;

    if (!runes_on_map->length)
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no runes currently on this map.\n");
        return;
    }

    Kots_SlowPrint(ent, "\nRunes On Map\n%-30s - %-15s\n----------------------------------------------------------------------\n", "Name", "Location");
    for (i = 0; i < runes_on_map->length; i++)
    {
        dropped = (dropped_rune_t *)Array_GetValueAt(runes_on_map, i);

        if (dropped->owner->character && dropped->owner->client)
            Kots_SlowPrint(ent, "%-30s - %-15s\n", dropped->rune->name, dropped->owner->character->name);
        else
            Kots_SlowPrint(ent, "%-30s - On Map\n", dropped->rune->name);
    }
}

void Kots_RuneDestroy(edict_t *ent, char *name)
{
    ULONG i;
    dropped_rune_t *dropped;
    rune_t *rune = Kots_RuneFindByName(name);

    if (!rune)
    {
        gi.cprintf(ent, PRINT_HIGH, "Could not find rune '%s'.\n", name);
        return;
    }

    for (i = 0; i < runes_on_map->length; i++) 
    {
        dropped = (dropped_rune_t *)Array_GetValueAt(runes_on_map, i);

        if (dropped->rune == rune)
        {
            if (dropped->owner->character && dropped->owner->client)
            {
                //print message to client
                gi.cprintf(dropped->owner, PRINT_HIGH, "Your rune was destroyed by %s.\n", ent->character->name);

                //print message to admin
                gi.cprintf(ent, PRINT_HIGH, "Destroyed '%s' held by %s.\n", dropped->rune->name, dropped->owner->character->name);
                
                Kots_RuneRemoveAbilities(dropped->owner, dropped->rune);
                dropped->owner->character->rune = NULL;
            }
            else
            {
                gi.cprintf(ent, PRINT_HIGH, "Destroyed '%s' found on map.\n", dropped->rune->name);
                dropped->owner->rune = NULL; //set the rune to null so it doesn't get removed when freeing
                G_FreeEdict(dropped->owner);
            }

            Array_DeleteAt(runes_on_map, i);
            return;
        }
    }
}

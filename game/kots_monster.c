#include "kots_monster.h"
#include "kots_array.h"
#include "kots_utils.h"

//track monsters on map
array_t *monsters = NULL;

int KOTS_MONSTER_VOTE;
int KOTS_MONSTER_NEXT_SPAWN;
int KOTS_MONSTER_NEXT_VOTE_INTERVAL;

void SP_monster_makron(edict_t *self);

#define MonsterCount 22
kots_monster_t kots_monsters[MonsterCount] = {
    {"monster_soldier_light", "Soldier Light", 4.0, 0.5, 10.0, 0, SP_monster_soldier_light, 50, true, false},
    {"monster_soldier", "Soldier", 4.25, 0.6, 5.5, 0, SP_monster_soldier, 100, true, false},
    {"monster_soldier_ss", "Soldier SS", 4.5, 0.75, 5.75, 0, SP_monster_soldier_ss, 150, true, false},
    {"monster_berserk", "Berserk", 5.0, 1.0, 5.0, 0, SP_monster_berserk, 200, true, false},
    {"monster_gladiator", "Gladiator", 4.75, 1.0, 2.5, 0, SP_monster_gladiator, 250, true, false},
    {"monster_gunner", "Gunner", 4.5, 1.0, 5.0, 0, SP_monster_gunner, 300, true, false},
    {"monster_infantry", "Infantry", 5.5, 1.0, 6.0, 0, SP_monster_infantry, 350, true, false},
    {"monster_tank", "Tank", 15.0, 5.0, 7.50, 25, SP_monster_tank, 360, true, false},
    {"monster_tank_commander", "Tank Commander", 20.0, 10.0, 10.0, 50, SP_monster_tank, 365, true, false},
    {"monster_medic", "Medic", 5.5, 1.0, 20.0, 0, SP_monster_medic, 400, true, false},
    {"monster_flipper", "Flipper", 6.0, 0.0, 7.5, 0, SP_monster_flipper, 0, false, false},
    {"monster_chick", "Chick", 6.0, 2.0, 3.5, 0, SP_monster_chick, 450, true, false},
    {"monster_parasite", "Parasite", 5.0, 0.0, 6.5, 0, SP_monster_parasite, 475, true, false},
    {"monster_flyer", "Flyer", 4.5, 2.0, 10.0, 0, SP_monster_flyer, 500, true, false},
    {"monster_brain", "Brain", 8.0, 2.0, 7.5, 0, SP_monster_brain, 525, true, false},
    {"monster_floater", "Floater", 7.0, 6.5, 16.0, 0, SP_monster_floater, 575, true, false},
    {"monster_hover", "Hover", 7.0, 4.0, 18.0, 0, SP_monster_hover, 625, true, false},
    {"monster_mutant", "Mutant", 6.5, 2.0, 4.0, 0, SP_monster_mutant, 675, true, false},
    {"monster_supertank", "Supertank", 30.0, 10.0, 15.0, 100, SP_monster_supertank, 680, true, false},
    {"monster_boss2", "Prince of the Air", 40.0, 20.0, 25.0, 100, SP_monster_boss2, 684, true, false},
    {"monster_jorg", "Jorg", 50.0, 20.0, 25.0, 500, SP_monster_jorg, 685, true, true},
    {"monster_makron", "Makron", 60.0, 20.0, 30.0, 500, SP_monster_makron, 686, true, true}
};

#define MonsterBuildCount 5
monster_build_t monster_builds[MonsterBuildCount] = {
    //dex str karma wis tech spirit rage vita vith mun
    {0, 0, 1.0, 0.75, 0.25, 0, 0, 0.25, 0.75, 0},
    {0, 0, 0.25, 0.75, 1.0, 0, 0, 0, 1.0, 0},
    /*{0, 0, 0, 1.0, 0, 0, 0, 1.0, 0, 0},*/
    {0, 0, 0, 0, 0, 0, 0, 0, 0.5, 0.5},
    {0, 0, 0.99, 1.0, 0, 0, 0, 1.0, 0},
    {0, 0, 0, 0, 1.0, 0, 0, 0, 1.0, 0}
};

qboolean Kots_MonstersEnabled()
{
    if (coop->value)
        return true;
    if(KOTS_MONSTER_VOTE == 1) // monsters off
        return false;
    return true;
}

void Kots_MonsterRecreateAll()
{
    int i = 1 + (int)maxclients->value;
    edict_t *ent = &g_edicts[i];

    for (; i < (int)maxentities->value; i++, ent++)
    {
        //TODO: Use something else because a monster that regened back shouldn't be recreated
        //only recreate monsters with full health
        if (ent->character && (ent->svflags & SVF_MONSTER))
        {
            //if the monster hasn't been hurt recreate it to adjust the level
            if (ent->health == ent->max_health)
                Kots_MonsterStart(ent);
        }
    }
}

void Kots_MonsterStart(edict_t *ent)
{
    Kots_CharacterInit(ent);
    Kots_MonsterCreate(ent);
    Kots_MonsterCreateBuild(ent);
    Kots_CharacterRespawn(ent);
    Kots_MonsterAddBonuses(ent);

    //make monsters logged in so some of our checking later is easier
    ent->character->is_loggedin = true;

    Array_PushBack(monsters, ent);
}

void Kots_MonsterCreate(edict_t *ent)
{
    int i, count, sum;
    edict_t *player;
    kots_monster_t *monster = Kots_MonsterGetBonus(ent);

    count = 0;
    sum = 0;
    for (i = 1; i <= maxclients->value ; i++)
    {
        player = g_edicts + i;

        if (player->inuse && player->character && player->character->is_loggedin)
        {
            ++count;
            sum += player->character->level;
        }
    }


    ent->character->monster_build = &monster_builds[rand() % MonsterBuildCount];
    if (count > 0)
        ent->character->level = ceil(((float)sum / count) + (rand() % 6));
    else
        ent->character->level = rand() % 6;

    if (ent->character->level <= 0)
    {
        ent->character->level = 0;
        ent->character->exp = 0;
    }
    else if (ent->character->level > 0)
    {
        ent->character->exp = Kots_CharacterGetNextLevelExp(ent->character->level - 1);
    }

    if (monster)
        Kots_strncpy(ent->character->name, monster->name, 50);
    else
        Kots_strncpy(ent->character->name, ent->classname, 50);

    // Aldarn - give bosses a bonus!
    if(strcmp(ent->character->name,"Jorg")==0 || strcmp(ent->character->name,"Makron")==0)
        ent->character->level *= 3;

    ent->character->playerpoints = ent->character->level;
    ent->character->weaponpoints = ent->character->level;
    ent->character->powerpoints = ent->character->level / 3;
    ent->character->monster_info = monster;
}

kots_monster_t *Kots_MonsterGetBonus(edict_t *ent)
{
    int i;
    for (i = 0; i < MonsterCount; i++)
        if (Q_stricmp(ent->classname, kots_monsters[i].classname) == 0)
            return &kots_monsters[i];

    return NULL;
}

void Kots_MonsterAddBonuses(edict_t *ent)
{
    kots_monster_t *monster = Kots_MonsterGetBonus(ent);
    if (!monster)
        return;

    ent->health = Kots_CharacterGetStartHealth(ent);
    ent->health *= monster->health;
    ent->max_health = ent->health;
}

void Kots_MonsterAddDamageBonus(edict_t *ent, int *damage)
{
    kots_monster_t *bonus = Kots_MonsterGetBonus(ent);

    if (!bonus)
        return;

    //determine monster base damage by adding bonus first
    *damage += Kots_RandMultiply(*damage, bonus->damage);

    //now add bonuses from level and munition
    *damage += Kots_RandMultiply((*damage), (0.05 * ent->character->level) + (ent->character->cur_munition * MUNITION_BONUS));
}

void Kots_MonsterMasterSkill(edict_t *ent, int *player)
{
    if (ent->character->playerpoints < 7)
        *player = ent->character->playerpoints;
    else
        *player = 7;

    ent->character->playerpoints -= *player;
}

void Kots_MonsterAddSkill(edict_t *ent, int *player, int amount, int limit)
{
    if (amount == 0)
        return;

    if (limit != 0 && limit < amount)
        amount = limit;

    if (ent->character->playerpoints < amount)
        *player = ent->character->playerpoints;
    else
        *player = amount;

    ent->character->playerpoints -= *player;
}

void Kots_MonsterCreateBuild(edict_t *ent)
{
    monster_build_t *build = ent->character->monster_build;

    //first find the mastery if there is one
    if (build->dexterity == 1.0)
        Kots_MonsterMasterSkill(ent, &ent->character->dexterity);
    else if (build->strength == 1.0)
        Kots_MonsterMasterSkill(ent, &ent->character->strength);
    else if (build->karma == 1.0)
        Kots_MonsterMasterSkill(ent, &ent->character->karma);
    else if (build->wisdom == 1.0)
        Kots_MonsterMasterSkill(ent, &ent->character->wisdom);
    else if (build->technical == 1.0)
        Kots_MonsterMasterSkill(ent, &ent->character->technical);
    else if (build->spirit == 1.0)
        Kots_MonsterMasterSkill(ent, &ent->character->spirit);
    else if (build->rage == 1.0)
        Kots_MonsterMasterSkill(ent, &ent->character->rage);

    //add to main abilities here first
    if (build->dexterity > 0)
        Kots_MonsterAddSkill(ent, &ent->character->dexterity, (int)ceil(ent->character->playerpoints * build->dexterity), 5);
    if (build->strength > 0)
        Kots_MonsterAddSkill(ent, &ent->character->strength, (int)ceil(ent->character->playerpoints * build->strength), 5);
    if (build->karma > 0)
        Kots_MonsterAddSkill(ent, &ent->character->karma, (int)ceil(ent->character->playerpoints * build->karma), 5);
    if (build->wisdom > 0)
        Kots_MonsterAddSkill(ent, &ent->character->wisdom, (int)ceil(ent->character->playerpoints * build->wisdom), 5);
    if (build->technical > 0)
        Kots_MonsterAddSkill(ent, &ent->character->technical, (int)ceil(ent->character->playerpoints * build->technical), 5);
    if (build->spirit > 0)
        Kots_MonsterAddSkill(ent, &ent->character->spirit, (int)ceil(ent->character->playerpoints * build->spirit), 5);
    if (build->rage > 0)
        Kots_MonsterAddSkill(ent, &ent->character->rage, (int)ceil(ent->character->playerpoints * build->rage), 5);


    //now put all remaining points in vith or vita or mun
    while (ent->character->playerpoints > 0)
    {
        if (build->vithealth > 0)
            Kots_MonsterAddSkill(ent, &ent->character->vithealth, (int)ceil(ent->character->playerpoints * build->vithealth), 0);
        if (build->vitarmor > 0)
            Kots_MonsterAddSkill(ent, &ent->character->vitarmor, (int)ceil(ent->character->playerpoints * build->vitarmor), 0);
        if (build->munition > 0)
            Kots_MonsterAddSkill(ent, &ent->character->munition, (int)ceil(ent->character->playerpoints * build->munition), 0);
    }

    //put all power points into antiweapon
    ent->character->antiweapon = ent->character->powerpoints;
}

void Kots_MonsterDead(edict_t *old)
{
    //don't spawn new monsters in coop
    if (!coop->value)
    {
        //keep track of total monsters for later
        level.total_monsters--;
    }

    if (spreewar.warent == old)
    {
        //reset the spreewar edict and all damages
        memset(&spreewar, 0, sizeof(spreewar));
    }

    UpdateAllChaseCams(old);
}

void Kots_MonsterKillAll()
{
    if (level.total_monsters > 0)
    {
        int i = (int)maxclients->value + 1;
        edict_t *ent = &g_edicts[i];

        for (; i < MAX_EDICTS; i++, ent++)
        {
            if (!ent->inuse)
                continue;

            if (ent->svflags & SVF_MONSTER && ent->character)
                G_FreeEdict(ent);
        }

        level.total_monsters = 0;
    }
}

int Kots_MonsterGetClientsLoggedIn()
{
    int i, count = 0;
    edict_t *ent = &g_edicts[1];
    for (i = 1; i <= (int)maxclients->value; i++, ent++)
    {
        if (!ent->inuse || !ent->character || !ent->character->is_loggedin)
            continue;

        ++count;
    }

    return count;
}

void Kots_MonsterCheckSpawn()
{
    //don't more spawn monsters in coop
    if (coop->value)
        return;

    //don't spawn or check for votes during intermission
    if (level.intermissiontime)
        return;

#ifndef KOTS_PVP_ONLY
    //determine new vote setting
    if (level.time >= KOTS_MONSTER_NEXT_VOTE_INTERVAL)
    {
        KOTS_MONSTER_NEXT_VOTE_INTERVAL = level.time + KOTS_MONSTERS_VOTE_INTERVAL;
        Kots_MonsterCheckVote();
    }
#endif

    //determine if we should spawn a new monster and the next interval
    if (level.time >= KOTS_MONSTER_NEXT_SPAWN)
    {
        int max = 0;
        int min = 0;
        int players = Kots_MonsterGetClientsLoggedIn();

        //determine how many monsters we should have for this mode
        switch (KOTS_MONSTER_VOTE)
        {
        case KOTS_MONSTERS_ONLY:
            max = KOTS_MONSTERS_ONLY_MAX;
            min = (players + 1) * 2;
            KOTS_MONSTER_NEXT_SPAWN = level.time + KOTS_MONSTERS_ONLY_SPAWN_INTERVAL;
            break;
        case KOTS_MONSTERS_MIXED:

            if (players > 5)
                KOTS_MONSTER_NEXT_SPAWN = level.time + KOTS_MONSTERS_MIXED_SPAWN_INTERVAL - 5;
            else
                KOTS_MONSTER_NEXT_SPAWN = level.time + KOTS_MONSTERS_MIXED_SPAWN_INTERVAL - players;

            max = KOTS_MONSTERS_MIXED_MAX;
            min = (players + 1) * 2;

            if (level.last_playerkilled && level.time < (level.last_playerkilled + KOTS_MONSTERS_SPAWN_AFTERPK))
            {
                KOTS_MONSTER_NEXT_SPAWN = level.last_playerkilled + KOTS_MONSTERS_SPAWN_AFTERPK;
                return;
            }

            break;
        case KOTS_MONSTERS_OFF:
        default:
            KOTS_MONSTER_NEXT_SPAWN = level.time + KOTS_MONSTERS_ONLY_SPAWN_INTERVAL;
            return;
        }

        if (!spreewar.warent)
        {
            qboolean success = false;
            //increment max based on clients
            max += players * KOTS_MONSTERS_PER_CLIENT;

            //if we need to spawn more
            if (level.total_monsters < max && level.total_monsters < KOTS_MONSTERS_MAX)
            {
                int i;
                gi.dprintf("Attempting to spawn a monster.\n");

                //we're only going to try a few times to spawn a monster
                //if we can't we'll simply try again next time
                for (i = 0; i < KOTS_MONSTERS_SPAWN_TRIES; i++)
                {
                    kots_monster_t *monster = Kots_MonsterGetRandom();
                    success = Kots_MonsterSpawnNew(monster);
                    if (success)
                        break;
                }

                if (!success)
                {
                    //couldn't spawn a monster
                    gi.dprintf("Monster spawn failed after too many attempts.\n");
                }
            }
        }

        //if monsters aren't voted off check to see how many monsters are currently spawned
        if (KOTS_MONSTER_VOTE != KOTS_MONSTERS_OFF)
        {
            //limit the minimum number of monsters
            if (min > max)
                min = max;

            //if we have less than the minimum monsters then spawn the next one sooner
            if (level.total_monsters < min)
            {
                KOTS_MONSTER_NEXT_SPAWN = level.time + 3.0;
            }
        }
    }
}

kots_monster_t *Kots_MonsterFind(char *name)
{
    int i;

    for (i = 0; i < MonsterCount; i++)
        if (Q_stricmp(name, kots_monsters[i].name) == 0)
            return &kots_monsters[i];

    return NULL;
}

qboolean Kots_MonsterSpawnNew(kots_monster_t *monster)
{
    edict_t *ent = G_Spawn();
    ent->classname = monster->classname;
    monster->spawn(ent);

    //ionly allow it to spawn if it was indexed and we found a spawn
    if (ent->s.modelindex && Kots_MonsterSelectSpawn(ent))
    {
        if (monster->boss)
            Kots_CenterPrintAll("WARNING: %s has entered the arena!!!", monster->name);

        return true;
    }
    else
    {
        //we couldn't spawn this monster so decrement the monster count
        Array_Remove(monsters, ent);
        G_FreeEdict(ent);
        level.total_monsters--;
        return false;
    }
}

qboolean Kots_MonsterSelectSpawn(edict_t *ent)
{
    int i, j, start;
    edict_t *spot = NULL;
    trace_t trace;
    vec3_t origin, dest;
    vec3_t angle;
    vec3_t below;

    for (i= 0; i < KOTS_MONSTERS_SPAWN_LOC_TRIES; i++)
    {
        spot = Kots_MonsterFindRandomEdict();

        //if we didn't find a spot this time then we won't next time
        if (!spot)
            return false;

        //we're going to move around in 45 degree increments
        //looking for openings starting at a random angle
        start = rand() % 8 * 45;
        for (j = 0; j < 8; j++)
        {
            VectorSet(angle, 0, start + j * 45, 0);
            AngleVectors(angle, angle, NULL, NULL);
            VectorCopy(spot->s.origin, origin);

            if (ent->mins[2] < 0)
                origin[2] += 9 + -1 * ent->mins[2];
            else
                origin[2] += 9; //raise the origin slightly

            VectorCopy(origin, dest);
            VectorMA(origin, ent->size[0] * 2, angle, origin);

            trace = gi.trace(origin, ent->mins, ent->maxs, dest, NULL, MASK_MONSTERSOLID | MASK_WATER);

            //if we started in the wall or solid or water try the next spot
            if (trace.startsolid || (trace.contents & (MASK_MONSTERSOLID | MASK_WATER)))
                continue;

            //make sure we're above a solid and not water or a monster
            VectorCopy(origin, below);
            below[2] -= 8192;
            trace = gi.trace(origin, ent->mins, ent->maxs, below, NULL, MASK_MONSTERSOLID | MASK_WATER);

            if (!(trace.contents & MASK_MONSTERSOLID))
                continue;
            if (gi.pointcontents(trace.endpos) & MASK_WATER)
                continue;

            //make sure there's nothing on the spot
            //and room for the monster
            //if (!(other = findradius(NULL, origin, ent->size[0])))
            //{
                VectorCopy(ent->s.origin, origin);
                VectorCopy(trace.endpos, ent->s.origin);
                if (Kots_MonsterDropToFloor(ent))
                {
                    VectorCopy (ent->s.origin, ent->s.old_origin);
                    VectorCopy (spot->s.angles, ent->s.angles);
                    ent->s.event = EV_PLAYER_TELEPORT;
                    gi.dprintf("Spawning monster '%s' near '%s'\n", ent->classname, spot->classname);
                    return true;
                }
                else
                {
                    //failed so make sure we copy the old origin back
                    VectorCopy(origin, ent->s.origin);
                }
            //}
        }
    }

    return false;
}

//we recreate this function so the monster can drop as far as necessary
qboolean Kots_MonsterDropToFloor(edict_t *ent)
{
    vec3_t      end;
    trace_t     trace;

    ent->s.origin[2] += 1;
    VectorCopy (ent->s.origin, end);
    end[2] -= 8192;

    trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

    //either it's way too far down or it's all solid
    if (trace.fraction == 1.0 || trace.allsolid)
        return false;

    VectorCopy (trace.endpos, ent->s.origin);
    M_droptofloor(ent); //now really drop monster to floor

    return true;
}


kots_monster_t *Kots_MonsterGetRandom()
{
    int i;
    int last = kots_monsters[MonsterCount -1].frequency;
    int monster;

    while (true)
    {
        monster = rand() % last + 1;
        for (i = 0; i < MonsterCount; i++)
        {
            if (monster <= kots_monsters[i].frequency && kots_monsters[i].enabled)
                return &kots_monsters[i];
        }
    }

    return NULL;
}

static edict_t *spots[MAX_EDICTS];
static int total_spots;
void Kots_MonsterInitSpawnPoints()
{
    int i;
    edict_t *ent;
    total_spots = 0;

    i = 1 + (int)maxclients->value;
    ent = &g_edicts[i];
    for ( ; i < (int)maxentities->value; i++, ent++)
    {
        if (!ent->inuse)
            continue;

        //don't use clients as a starting position
        if (ent->client)
            continue;

        //don't use monsters as a starting position
        if (ent->svflags & SVF_MONSTER)
            continue;

        //don't use items owned by clients as a starting position
        if (ent->owner != NULL && ent->owner->client)
            continue;

        //ensure that we have an item
        if (!ent->item)
            continue;

        spots[total_spots] = ent;
        total_spots++;
    }
}

edict_t *Kots_MonsterFindRandomEdict()
{
    int i, start;
    edict_t *ent;

    //no spots?
    if (!total_spots)
        return NULL;

    //determine a random starting entity
    start = rand() % total_spots;
    i = start;

    do
    {
        ent = spots[i];

        if (++i >= total_spots)
            i = 0;

        if (!ent->inuse)
            continue;

        return ent;

    } while (i != start);

    return NULL; //couldn't find one
}

void Kots_MonsterWriteStats(edict_t *ent)
{
    edict_t *monster = ent->client->chase_target;

    if (monster && monster->inuse && monster->character)
    {
        ent->client->ps.stats[STAT_AMMO] = 0;
        ent->client->ps.stats[STAT_AMMO_ICON] = 0;
        ent->client->ps.stats[STAT_ARMOR] = 0;
        ent->client->ps.stats[STAT_ARMOR_ICON] = ARMOR_INDEX;
        ent->client->ps.stats[STAT_FRAGS] = monster->character->kills;
        ent->client->ps.stats[STAT_HEALTH] = monster->health;
        ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
        ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
        ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
        ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
        ent->client->ps.stats[STAT_SELECTED_ITEM] = 0;
        ent->client->ps.stats[STAT_TIMER] = 0;
        ent->client->ps.stats[STAT_TIMER_ICON] = 0;
        ent->client->ps.stats[STAT_CHASE] = CS_MODELS + monster->s.modelindex;
        ent->client->ps.stats[STAT_KOTS_SCORE] = monster->character->score;
        ent->client->ps.stats[STAT_KOTS_SPREE] = monster->character->spree;
        ent->client->ps.stats[STAT_KOTS_CUBES] = monster->character->cubes;
        ent->client->ps.stats[STAT_KOTS_EXPACK] = monster->character->expacksleft;
        ent->client->ps.stats[STAT_KOTS_THROW] = monster->character->throwsleft;
        ent->client->ps.stats[STAT_KOTS_SPIRAL] = monster->character->spiralsleft;
        ent->client->ps.stats[STAT_KOTS_BIDE] = monster->character->bidesleft;
        ent->client->ps.stats[STAT_KOTS_HASEXPACK] = monster->character->cur_expack;
        ent->client->ps.stats[STAT_KOTS_HASTHROW] = monster->character->cur_kotsthrow;
        ent->client->ps.stats[STAT_KOTS_HASSPIRAL] = monster->character->cur_spiral;
        ent->client->ps.stats[STAT_KOTS_HASBIDE] = monster->character->cur_bide;
        ent->client->ps.stats[STAT_KOTS_EMPATHY] = monster->character->using_empathy;
        ent->client->ps.stats[STAT_KOTS_TBALLS] = monster->character->tballs;
    }
}

//Look around for a new goal entity to walk to
qboolean Kots_MonsterFindGoal(edict_t *ent)
{
    edict_t *ignore = ent->enemy;
    edict_t *best = NULL;
    edict_t *targ = NULL;
    vec3_t forward, dir;
    float dp, bestdp = 0;
    int radius = 1024;
    int count = 0;

    //determine the monsters angle
    AngleVectors(ent->s.angles, forward, NULL, NULL);

    while ((targ = findradius(targ, ent->s.origin, radius)) != NULL)
    {
        if (++count >= MAX_EDICTS)
            break;

        //only search for items that we can move to
        if (!targ->item)
            continue;

        if (ignore && targ == ignore)
            continue;

        VectorSubtract(targ->s.origin, ent->s.origin, dir);
        VectorNormalize(dir);
        dp = DotProduct(forward, dir);

        if (dp > bestdp && Kots_MonsterCanWalkTo(ent, targ))
        {
            bestdp = dp;
            best = targ;
        }
    }

    if (best)
    {
        ent->goalentity = ent->movetarget = ent->enemy = best;

        if (ent->monsterinfo.walk)
            ent->monsterinfo.walk(ent);
        else if (ent->monsterinfo.run)
            ent->monsterinfo.run(ent);

        return true;
    }

    return false;
}

qboolean Kots_MonsterCanWalkTo(edict_t *self, edict_t *targ)
{
    //int max_stepsize = 6;
    //trace_t tr;

    //TODO: trace to verify the monster can actually reach the goal
    return true;
}

void Kots_MonsterVoteCommand(edict_t *ent)
{
    if (gi.argc() < 3)
        gi.cprintf(ent, PRINT_HIGH, "Usage: vote monsters (on/only/mixed/off)\n");
    else
    {
        char *value = gi.argv(2);

        if (Q_stricmp(value, "off") == 0)
            Kots_MonsterVote(ent, KOTS_MONSTERS_OFF);
        else if (Q_stricmp(value, "mixed") == 0)
            Kots_MonsterVote(ent, KOTS_MONSTERS_MIXED);
        else if (Q_stricmp(value, "on") == 0 || Q_stricmp(value, "only") == 0)
            Kots_MonsterVote(ent, KOTS_MONSTERS_ONLY);
        else
            gi.cprintf(ent, PRINT_HIGH, "Unrecognized monster vote '%s'.\n", value);
    }
}

void Kots_MonsterVote (edict_t *ent, int vote)
{
    switch (vote)
    {
    case KOTS_MONSTERS_OFF:
        gi.cprintf(ent, PRINT_HIGH, "You have voted monsters off.\n");
        break;
    case KOTS_MONSTERS_MIXED:
        gi.cprintf(ent, PRINT_HIGH, "You have voted monsters mixed.\n");
        break;
    case KOTS_MONSTERS_ONLY:
        gi.cprintf(ent, PRINT_HIGH, "You have voted monsters only.\n");
        break;
    default:
        //if we somehow pass an invalid value default to mixed
        vote = KOTS_MONSTERS_MIXED;
        break;
    }

    ent->client->pers.kots_persist.monster_vote = vote;

    //Vote will be checked at next interval
    //Kots_MonsterCheckVote(ent);
}

void Kots_MonsterCheckVote()
{
    //don't check monster vote at intermission time
    if (!level.intermissiontime)
    {
        float   first_vote = 0.0;
        float   first_time = 0.0;
        float   players = 0.0;
        float   votes = 0.0;
        float   average = 0.0; // float so decimals work
        int     i;
        edict_t *cl_ent = g_edicts + 1;

        // Go through each player slot and add up any players logged in, so we can get average vote
        for (i = 0; i < game.maxclients; i++, cl_ent++)
        {
            if (!cl_ent->inuse)
                continue;
            if(!cl_ent->character || !cl_ent->character->is_loggedin)
                continue;

            players++;
            votes += cl_ent->client->pers.kots_persist.monster_vote;

            if (first_time == 0.0 || cl_ent->client->pers.kots_persist.connect_time < first_time)
            {
                first_time = cl_ent->client->pers.kots_persist.connect_time;
                first_vote = cl_ent->client->pers.kots_persist.monster_vote;
            }
        }

        //if there are only 2 or fewer people then only the first players vote counts
        if (players <= 2)
            average = first_vote;

        //otherwise calculate votes normally
        else if(votes > 0 && players > 0)
            average = votes / players;


        if(average)
        {
            if(average <= 1.5) //no mons
            {
                //if the vote didn't change then don't bother doing anything
                if (KOTS_MONSTER_VOTE != KOTS_MONSTERS_OFF)
                {
                    KOTS_MONSTER_VOTE = KOTS_MONSTERS_OFF;

                    for(i=1;i<=maxclients->value;i++)
                    {
                        if ((cl_ent=&g_edicts[i]) && cl_ent->inuse)
                        {
                            gi.centerprintf (cl_ent, "Monsters voted OFF");
                            gi.sound (cl_ent, CHAN_BODY, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
                        }
                    }
                }
            }
            else if(average > 1.5 && average <= 2.5) //mixed
            {
                //if the vote didn't change then don't bother doing anything
                if (KOTS_MONSTER_VOTE != KOTS_MONSTERS_MIXED)
                {
                    KOTS_MONSTER_VOTE = KOTS_MONSTERS_MIXED;

                    for(i=1;i<=maxclients->value;i++)
                    {
                        if ((cl_ent=&g_edicts[i]) && cl_ent->inuse)
                        {
                            gi.centerprintf (cl_ent, "Monsters voted MIXED");
                            gi.sound (cl_ent, CHAN_BODY, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
                        }
                    }
                }
            }
            else if(average > 2.5) //only
            {
                //if the vote didn't change then don't bother doing anything
                if (KOTS_MONSTER_VOTE != KOTS_MONSTERS_ONLY)
                {
                    KOTS_MONSTER_VOTE = KOTS_MONSTERS_ONLY;

                    for(i=1;i<=maxclients->value;i++)
                    {
                        if ((cl_ent=&g_edicts[i]) && cl_ent->inuse)
                        {
                            gi.centerprintf (cl_ent, "Monsters voted ONLY");
                            gi.sound (cl_ent, CHAN_BODY, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
                        }
                    }
                }
            }
        }
    }
}

//TODO: Keep a list of monsters so we don't have to go through everything
float Kots_MonstersRangeFromSpot(edict_t *spot)
{
    edict_t *monster;
    float   bestdistance;
    vec3_t  v;
    unsigned long i;
    float   distance;
    qboolean found = false;

    bestdistance = 9999999;

    for (i = 0; i < monsters->length; i++)
    {
        monster = Array_GetValueAt(monsters, i);

        if (!monster->inuse)
            continue;

        if (!(monster->svflags & SVF_MONSTER))
            continue;

        if (monster->health <= 0)
            continue;

        if (!monster->character)
            continue;

        found = true;
        VectorSubtract (spot->s.origin, monster->s.origin, v);
        distance = VectorLength (v);

        if (distance < bestdistance)
            bestdistance = distance;
    }

    //return 0 if no monsters found otherwise the first spawn is always going to be used in spawn farthest
    if (found)
        return bestdistance;
    else
        return 0;
}

void Kots_MonstersInit()
{
    if (!monsters)
    {
        monsters = Array_Create(NULL, KOTS_MONSTERS_MAX, KOTS_MONSTERS_MAX);
        monsters->FreeItem = NULL;
    }
}

void Kots_MonstersFree()
{
    Array_Delete(monsters);
    monsters = NULL;
}

void Kots_MonstersClear()
{
    Array_ClearAll(monsters);
}

void Kots_MonsterPrintVoteInfo(edict_t *ent)
{
    static char *vote_names[] = {"Off", "Mixed", "Only"};
    edict_t *player = NULL;
    int i, count = 0;

    Kots_SlowPrint(ent, "Monster Vote Info:\n");
    Kots_SlowPrint(ent, "Current Setting: %s\n", vote_names[KOTS_MONSTER_VOTE]);
    Kots_SlowPrint(ent, "\nPlayer Votes:\n");
    Kots_SlowPrint(ent, "%-20s %-6s\n", "Name", "Vote");

    player = g_edicts + 1;
    for (i = 0; i < game.maxclients; i++, player++)
    {
        if (player->inuse && player->character && player->character->is_loggedin)
        {
            Kots_SlowPrint(ent, "%-20s %-6s\n", player->client->pers.netname, vote_names[player->client->pers.kots_persist.monster_vote]);
            count++;
        }
    }

    if (count == 0)
        Kots_SlowPrint(ent, "No players in the game.\n");
}


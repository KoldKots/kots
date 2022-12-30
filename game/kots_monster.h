#ifndef KOTS_MONSTER_H
#define KOTS_MONSTER_H

#include "kots_character.h"


//monster definitions
#define KOTS_MONSTERS_ONLY_SPAWN_INTERVAL   5.0
#define KOTS_MONSTERS_MIXED_SPAWN_INTERVAL  15.0
#define KOTS_MONSTERS_MAX                   25
#define KOTS_MONSTERS_ONLY_MAX              10
#define KOTS_MONSTERS_MIXED_MAX             4
#define KOTS_MONSTERS_PER_CLIENT            2
#define KOTS_MONSTERS_SPAWN_TRIES           5 /*Try spawning a monster # times before giving up*/
#define KOTS_MONSTERS_SPAWN_LOC_TRIES       10 /*Try # locations before giving up*/
#define KOTS_MONSTERS_SPAWN_AFTERPK         45.0 /*Don't spawn a monster for this long after a player kill*/
#define KOTS_MONSTERS_VOTE_INTERVAL         15.0 /*Check for monster vote changes this often*/

//monster vote settings
#define KOTS_MONSTERS_OFF                       1
#define KOTS_MONSTERS_MIXED                     2
#define KOTS_MONSTERS_ONLY                      3

extern int KOTS_MONSTER_VOTE;
extern int KOTS_MONSTER_NEXT_SPAWN;
extern int KOTS_MONSTER_NEXT_VOTE_INTERVAL;

typedef struct kots_monster_s
{
    char *classname;
    char *name;
    float health;
    float armor;
    float damage;
    int bonus_exp;
    void (*spawn)(edict_t *self);
    int frequency;
    qboolean enabled;
    qboolean boss;
} kots_monster_t;

typedef struct monster_build_s
{
    float dexterity;
    float strength;
    float karma;
    float wisdom;
    float technical;
    float spirit;
    float rage;
    float vitarmor;
    float vithealth;
    float munition;
} monster_build_t;

//define quake 2 methods referenced
void SP_monster_jorg(edict_t *self);
void SP_monster_boss2(edict_t *self);
void SP_monster_berserk(edict_t *self);
void SP_monster_soldier_ss(edict_t *self);
void SP_monster_soldier(edict_t *self);
void SP_monster_brain(edict_t *self);
void SP_monster_chick(edict_t *self);
void SP_monster_flipper(edict_t *self);
void SP_monster_floater(edict_t *self);
void SP_monster_flyer(edict_t *self);
void SP_monster_gladiator(edict_t *self);
void SP_monster_hover(edict_t *self);
void SP_monster_infantry(edict_t *self);
void SP_misc_insane(edict_t *self);
void SP_monster_medic(edict_t *self);
void SP_monster_mutant(edict_t *self);
void SP_monster_parasite(edict_t *self);
void SP_monster_tank(edict_t *self);
//void SP_monster_makron(edict_t *self);
void SP_monster_gunner(edict_t *self);
void SP_monster_supertank(edict_t *self);
void SP_monster_soldier_light(edict_t *self);


//define kots methods
qboolean Kots_MonstersEnabled();
void Kots_MonsterRecreateAll();
void Kots_MonsterStart(edict_t *ent);
void Kots_MonsterCreate(edict_t *ent);
kots_monster_t *Kots_MonsterGetBonus(edict_t *ent);
void Kots_MonsterAddBonuses(edict_t *ent);
void Kots_MonsterAddDamageBonus(edict_t *ent, int *damage);
void Kots_MonsterCreateBuild(edict_t *ent);
void Kots_MonsterDead(edict_t *old);
kots_monster_t *Kots_MonsterGetRandom();
void Kots_MonsterKillAll();
int Kots_MonsterGetClientsLoggedIn();
void Kots_MonsterCheckSpawn();
qboolean Kots_MonsterSelectSpawn(edict_t *ent);
qboolean Kots_MonsterDropToFloor(edict_t *ent);
edict_t *Kots_MonsterFindRandomEdict();
void Kots_MonsterInitSpawnPoints();
kots_monster_t *Kots_MonsterFind(char *name);
qboolean Kots_MonsterSpawnNew(kots_monster_t *monster);
void Kots_MonsterWriteStats(edict_t *ent);
qboolean Kots_MonsterFindGoal(edict_t *ent);
qboolean Kots_MonsterCanWalkTo(edict_t *self, edict_t *targ);
void Kots_MonsterVoteCommand(edict_t *ent);
void Kots_MonsterVote (edict_t *ent, int vote);
void Kots_MonsterCheckVote();
float Kots_MonstersRangeFromSpot(edict_t *spot);
void Kots_MonstersInit();
void Kots_MonstersFree();
void Kots_MonstersClear();
void Kots_MonsterPrintVoteInfo(edict_t *ent);

#endif

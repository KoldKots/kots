#ifndef KOTS_RUNES_H
#define KOTS_RUNES_H

#include "kots_character.h"

#define KOTS_RUNES_MAX_DROPPED      8
#define KOTS_RUNES_CHECK_INTERVAL   360

typedef struct rune_s
{
    int id;
    char name[51];
    char pickup_text[257];
    char model_name[MAX_QPATH];
    char image_name[MAX_QPATH];
    char sound_name[MAX_QPATH];
    vec3_t mins;
    vec3_t maxs;
    int effects;
    int renderfx;
    int character_id; //for runes tied to a specific character
    float rarity;

    //player points
    int dexterity;
    int strength;
    int karma;
    int wisdom;
    int technical;
    int spirit;
    int rage;
    int vitarmor;
    int vithealth;
    int munition;

    //power points
    int expack;
    int spiral;
    int bide;
    int kotsthrow;
    int antiweapon;

    //weapon points
    int sabre;
    int shotgun;
    int machinegun;
    int chaingun;
    int supershotgun;
    int grenade;
    int grenadelauncher;
    int rocketlauncher;
    int hyperblaster;
    int railgun;
    int bfg;

    //other bonuses
    int tballs;
    int tball_regen;
    int tball_speed;
    qboolean normal_resist;
    qboolean energy_resist;

} rune_t;


typedef struct dropped_rune_s
{
    int id;
    rune_t *rune;
    edict_t *owner;
} dropped_rune_t;

//quake 2 functions used
edict_t *SelectRandomDeathmatchSpawnPoint();

//function prototypes
qboolean Kots_RunePickup(edict_t *ent, edict_t *other);
edict_t *Kots_RuneDrop(edict_t *ent);
void Kots_RuneCreate(edict_t *ent, rune_t *rune);
void Kots_RuneAddAbilities(edict_t *ent, rune_t *rune);
void Kots_RuneRemoveAbilities(edict_t *ent, rune_t *rune);
void Kots_RuneIndexAll();
rune_t *Kots_RuneFindById(int id);
rune_t *Kots_RuneFindByName(char *name);
void Kots_RunesInit();
void Kots_RunesFreeInfo();
void Kots_RunesCheckLoad();
qboolean Kots_RuneIsDropped(int rune_id);
void Kots_RuneCheckAssociated(edict_t *ent);
void Kots_RuneAddDropped(edict_t *ent, rune_t *rune);
void Kots_RuneRemoveDropped(edict_t *ent, rune_t *rune);
void Kots_RuneSpawn(rune_t *rune);
edict_t *Kots_RuneDropAt(edict_t *spot, rune_t *rune);
void Kots_RuneDeathDrop(edict_t *ent);
rune_t *Kots_RunePickRandom();
void Kots_RuneGive(edict_t *ent, int rune_id);
int Kots_RunesOnMap();
void Kots_RuneExplode(edict_t *ent);
void Kots_RuneTeleport(edict_t *ent);
void Kots_RunesList(edict_t *ent);
void Kots_RunesShow(edict_t *ent);
void Kots_RuneDestroy(edict_t *ent, char *name);

#endif

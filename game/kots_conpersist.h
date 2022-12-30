#ifndef KOTS_CONPERSIST_H
#define KOTS_CONPERSIST_H

#include "kots_character.h"

/*

The way this is done is kind of crappy and doesn't really solve the real issue.
The whole reason for this is that there seems to be some sort of issue with R1Q2 performing hard reconnects occasionally.
This causes all the persisted client data to be erased.

The way it works is every time a map ends all data is cleared and new data is saved.
Characters that sucessfully make it to the next map and attempt to login are cleared.
After a new map loads the player has 30 seconds to connect and reclaim their persistent data.
Persistant data is matched by name AND IP Address.

*/

typedef struct
{
    char name[16];
    kots_persist_t persist;
} kots_conpersist_t;

void Kots_LoadConPersistData(edict_t *ent);
void Kots_ClearConPersistData(edict_t *ent);
void Kots_ClearAllConPersistData();
void Kots_SaveAllConPersistData();

#endif

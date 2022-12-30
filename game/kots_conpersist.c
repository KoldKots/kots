#include "kots_conpersist.h"
#include "kots_utils.h"

#define KOTS_CONPERSIST_MAXWAIT     30.0

static kots_conpersist_t *conpersist[MAX_CLIENTS];
static int persist_length = 0;

int Kots_ConPersistFindIndex(edict_t *ent)
{
    int i;

    for (i = 0; i < persist_length; i++)
    {
        if (conpersist[i] != NULL)
        {
            if (strcmp(conpersist[i]->name, ent->client->pers.netname) == 0
                && strcmp(conpersist[i]->persist.ip_address, ent->client->pers.kots_persist.ip_address) == 0)
                return i;
        }
    }

    return -1;
}

void Kots_LoadConPersistData(edict_t *ent)
{
    if (!level.intermissiontime)
    {
        int index = Kots_ConPersistFindIndex(ent);

        if (index != -1)
        {
            if (level.time < KOTS_CONPERSIST_MAXWAIT)
            {
                ent->client->pers.kots_persist = conpersist[index]->persist;
                gi.dprintf("Loaded connection persistent info for '%s'.\n", ent->client->pers.netname);
            }
            else
                gi.dprintf("Unable to load connection persistent info for '%s' because too much time has ellapsed.\n", ent->client->pers.netname);

            gi.TagFree(conpersist[index]);
            conpersist[index] = NULL;
        }
    }
}

void Kots_ClearConPersistData(edict_t *ent)
{
    int index = Kots_ConPersistFindIndex(ent);

    if (index != -1)
    {
        gi.TagFree(conpersist[index]);
        conpersist[index] = NULL;
    }
}

void Kots_ClearAllConPersistData()
{
    int i;

    for (i = 0; i < persist_length; i++)
    {
        if (conpersist[i] != NULL)
        {
            gi.TagFree(conpersist[i]);
            conpersist[i] = NULL;
        }
    }

    persist_length = 0;
}

void Kots_SaveConPersistData(edict_t *ent)
{
    if (!ent->inuse)
        return;

    //only persist between maps if logged in otherwise no need
    if (!ent->client->pers.kots_persist.is_loggedin)
        return;

    if (persist_length >= MAX_CLIENTS)
        return;

    conpersist[persist_length] = gi.TagMalloc(sizeof(kots_conpersist_t), TAG_GAME);
    Kots_strncpy(conpersist[persist_length]->name, ent->client->pers.netname, 16);
    conpersist[persist_length]->persist = ent->client->pers.kots_persist;
    persist_length++;
}

void Kots_SaveAllConPersistData()
{
    int i;
    edict_t *ent;

    ent = g_edicts + 1;
    for (i = 0; i < game.maxclients; i++, ent++)
        Kots_SaveConPersistData(ent);
}

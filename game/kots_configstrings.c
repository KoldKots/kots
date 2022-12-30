#include "kots_configstrings.h"
#include "kots_utils.h"

typedef int (*indexmethod_t)(char *name);

//references to base methods
void (*configstring_base)(int index, char *value);
int (*soundindex_base)(char *name);
int (*modelindex_base)(char *name);
int (*imageindex_base)(char *name);
int (*imageindex_base)(char *name);
void (*setmodel_base)(edict_t *ent, char *name);

//function prototypes
void Kots_ConfigStringWrapper(int index, char *value);
void Kots_SetModel(edict_t *ent, char *name);
int Kots_SoundIndex(char *name);
int Kots_ModelIndex(char *name);
int Kots_ImageIndex(char *name);
int Kots_ConfigStringFindIndex(char *name, int start, int length, indexmethod_t method);

//static variables used in this file
static char configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];

//maps use models so we need to keep track of when the first model gets indexed
static qboolean first_model_indexed;

void Kots_ConfigStringInit()
{
    if (gi.configstring != Kots_ConfigStringWrapper)
    {
        configstring_base = gi.configstring;
        gi.configstring = Kots_ConfigStringWrapper;
    }

    //reset all config strings
    memset(&configstrings, 0, sizeof(configstrings));
}

void Kots_ConfigStringWrapper(int index, char *value)
{
    static unsigned long max_length = MAX_CONFIGSTRINGS * MAX_QPATH;
    static char tempvalue[MAX_CONFIGSTRINGS * MAX_QPATH];

    //make sure we're within the valid range of config strings
    if (index < 0 || index >= MAX_CONFIGSTRINGS)
    {
        gi.dprintf("ERROR: Tried to add a configstring '%s' out of range (%i).", value, index);
        return;
    }

    //ignore null or empty strings
    if (!value || !*value)
        return;

    //determine if the value has changed
    if (strcmp(value, configstrings[index]) != 0)
    {
        //determine the maximum length for this specific index and limit the size
        int realmax = max_length - (index * MAX_QPATH);
        int length = Kots_snprintf(tempvalue, realmax, "%s", value);

        //if it was truncated we need to compare again
        if (length == realmax)
        {
            gi.dprintf("ERROR: Tried to add a configstring that was too long.\n  Original value: '%s'\n  Truncated value: '%s'\n", value, tempvalue);

            //if the truncated string is the same then exit
            if (strcmp(tempvalue, configstrings[index]) == 0)
                return;
        }

        //copy the truncated value to the config strings and update clients
        strcpy(configstrings[index], tempvalue);
        configstring_base(index, tempvalue);
    }
}

void Kots_ConfigStringIndexInit()
{
    //redirect methods if not already
    if (gi.soundindex != Kots_SoundIndex)
    {
        soundindex_base = gi.soundindex;
        modelindex_base = gi.modelindex;
        imageindex_base = gi.imageindex;
        setmodel_base = gi.setmodel;

        gi.soundindex = Kots_SoundIndex;
        gi.modelindex = Kots_ModelIndex;
        gi.imageindex = Kots_ImageIndex;
        gi.setmodel = Kots_SetModel;
    }

    //reset our model index indicator
    first_model_indexed = false;
}

void Kots_SetModel(edict_t *ent, char *name)
{
    //let the exe handle inline models
    if (name && name[0] == '*')
        setmodel_base(ent, name);
    else
        ent->s.modelindex = Kots_ModelIndex(name);
}

int Kots_SoundIndex(char *name)
{
    return Kots_ConfigStringFindIndex(name, CS_SOUNDS, MAX_SOUNDS, soundindex_base);
}

int Kots_ModelIndex(char *name)
{
    //last model index shouldn't be used for some reason because it has odd results
    int index = Kots_ConfigStringFindIndex(name, CS_MODELS, MAX_MODELS - 1, modelindex_base);

    //if this is the first model indexed we need to fill in the ones used
    //this assumes a map doesn't use all 256 and we're left with nothing
    if (!first_model_indexed)
    {
        int i;

        Kots_strncpy(configstrings[CS_MODELS + 1], level.mapname, MAX_QPATH);

        for (i = 1; i < index; i++)
            Kots_snprintf(configstrings[i + CS_MODELS + 1], MAX_QPATH, "*%i", i);

        first_model_indexed = true;
    }

    return index;
}

int Kots_ImageIndex(char *name)
{
    return Kots_ConfigStringFindIndex(name, CS_IMAGES, MAX_IMAGES, imageindex_base);
}

int Kots_ConfigStringFindIndex(char *name, int start, int length, indexmethod_t method)
{
    int i;
    int max = start + length;

    if (!name || !*name)
        return 0;

    //see if it's already cached
    for (i = start + 1; i < max; i++)
    {
        if (configstrings[i][0] == '\0')
            break;
        else if (strncmp(name, configstrings[i], MAX_QPATH) == 0)
            return i - start;
    }

    //it's possible that a map used all indexes, but very unlikely
    //we tried to go over the max, print a debug message and return no sound
    if (i >= max)
    {
        gi.dprintf("ERROR: Tried to go over max index count with '%s'.\n", name);
        return 0;
    }

    //otherwise it's a new sound so index it
    else
    {
        i = method(name);
        Kots_strncpy(configstrings[i + start], name, MAX_QPATH);
        return i;
    }
}

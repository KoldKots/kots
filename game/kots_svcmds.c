#include "kots_svcmds.h"
#include "kots_character.h"
#include "kots_maplist.h"

void Kots_CheckCvarChanges()
{
    //check if the maplist changed
    if (Q_stricmp(kots_maplist->string, maplist.filename))
    {
        //reset it back after failing to load
        if (Kots_Maplist_Load(kots_maplist->string))
        {
            //when loaded through the cvar default to random rotation
            maplist.rotationflag = ML_ROTATE_RANDOM;
        }
        else
        {
            gi.cvar_forceset(kots_maplist->name, maplist.filename);
        }
    }

    //check if the rotation cvar changed
    if (kots_maprotate->value != maplist.rotationflag)
    {
        int value = kots_maprotate->value;

        if (value != ML_ROTATE_RANDOM && value != ML_ROTATE_SEQ)
        {
            //Invalid value, set it back
            gi.dprintf("Invalid maprotate setting of %i", value);
            gi.cvar_forceset(kots_maprotate->name, va("%i", maplist.rotationflag));
            return;
        }

        maplist.rotationflag = kots_maprotate->value;
    }
}

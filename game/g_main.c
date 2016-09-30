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


//SWB - KOTS Includes
#include "kots_logging.h"
#include "kots_maplist.h"
#include "kots_server.h"
#include "kots_runes.h"
#include "kots_monster.h"
#include "kots_utils.h"
#include "kots_hud.h"
#include "kots_svcmds.h"
#include "kots_conpersist.h"


game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
int	snd_fry;
int meansOfDeath;

edict_t		*g_edicts;

cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*password;
cvar_t	*spectator_password;
cvar_t	*needpass;
cvar_t	*maxclients;
cvar_t	*maxspectators;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*dedicated;

cvar_t	*filterban;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

cvar_t	*sv_maplist;

//SWB
//kots cvar list
cvar_t	*kots_pass;
cvar_t	*kots_nas;
cvar_t	*kots_maplist;
cvar_t	*kots_maprotate;
cvar_t	*kots_public;
cvar_t	*kots_motd;
cvar_t	*kots_prevmaps;
cvar_t	*gamever;
cvar_t	*gamedir;
cvar_t	*hostname;
cvar_t	*mapname;
cvar_t	*port;

void SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void RunEntity (edict_t *ent);
void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);
void InitGame (void);
void G_RunFrame (void);


//===================================================================


void ShutdownGame (void)
{
	int i;
	edict_t *ent;

	//SWB - perform logging before we exit
	Kots_LogOnDisconnect();

	//SWB
	//Ensure that we save all logged in characters before shutting down
	for (i = 0; i < maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;

		if (ent->character->is_loggedin)
		{
			//shutting down so we don't want to drop the rune
			ent->character->rune = NULL;

			Kots_CharacterLogout(ent, false, false);
		}

		Kots_CharacterClearEdicts(ent);
		Kots_CharacterFree(ent);
	}

	//SWB - free up some things that are used
	Kots_FreeServer();
	Kots_RunesFreeInfo();
	Kots_MonstersFree();
	Kots_Maplist_Free();
	Kots_DeleteSoundQueue();

	gi.dprintf ("==== ShutdownGame ====\n");

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	//SWB
	Kots_InitializeServer();
	Kots_RunesInit();
	Kots_MonstersInit();
	Kots_Maplist_Init();


	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);

	//SWB - prevent overflows by using vsnprintf
	Kots_vsnprintf (text, 1024, error, argptr);

	va_end (argptr);

	gi.error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);

	//SWB - prevent overflows by using vsnprintf
	Kots_vsnprintf (text, 1024, msg, argptr);

	va_end (argptr);

	gi.dprintf ("%s", text);
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;

		ClientEndServerFrame (ent);

		//SWB
		Kots_CharacterEndServerFrame(ent);
	}

}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	edict_t		*ent;

	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		BeginIntermission (CreateTargetChangeLevel (level.mapname) );
		return;
	}
	// Aldarn - Map list and map voting //
	else if (maplist.maps->length > 0)  // maplist active?
	{
		//SWB - if we didn't get here from a vote
		if(!level.mapvotestarted || !level.passed)
		{
			//pick a map from the maplist depending on rotation and store it in the vote value
			if (maplist.rotationflag == ML_ROTATE_RANDOM)
				level.mapvote = Kots_Maplist_GetRandomMap();
			else
				level.mapvote = Kots_Maplist_GetNextMap();
		}

		//SWB - begin intermission and then set the new current map
		//it's important that we set the current map AFTER beginning intermission
		//so that the correct previous map gets added to the previous list
		BeginIntermission (CreateTargetChangeLevel (Kots_Maplist_GetName(level.mapvote)) );
		maplist.currentmap = level.mapvote;
		return;
	}
	
	if (level.nextmap[0]) // go to a specific map
		BeginIntermission (CreateTargetChangeLevel (level.nextmap) );
	else {	// search for a changelevel
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			return;
		}
		BeginIntermission (ent);
	}
}


/*
=================
CheckNeedPass
=================
*/
void CheckNeedPass (void)
{
	int need;

	// if password or spectator_password has changed, update needpass
	// as needed
	if (password->modified || spectator_password->modified) 
	{
		password->modified = spectator_password->modified = false;

		need = 0;

		if (*password->string && Q_stricmp(password->string, "none"))
			need |= 1;
		if (*spectator_password->string && Q_stricmp(spectator_password->string, "none"))
			need |= 2;

		gi.cvar_set("needpass", va("%d", need));
	}
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i;
	gclient_t	*cl;

	if (level.intermissiontime)
	{
		//SWB - go through all intermission screens before proceeding
		Kots_HudIntermissionCheckScreen();

		//SWB - exit intermission after 10 seconds
		//if ((level.intermissiontime + 10.0) <= level.time)
			//level.exitintermission = true;

		return;
	}

	if (!deathmatch->value)
		return;

	//SWB - timelimit is forced in g_spawn with other forced values
	if (timelimit->value)
	{
		if (level.time >= timelimit->value) // Aldarn - Kots hardcodes timelimit as 30min
		{
			gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
		}
	}

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl = game.clients + i;
		if (!g_edicts[i+1].inuse)
			continue;

		if (cl->resp.score >= fraglimit->value) // Aldarn - Kots hardcodes fraglimit as 50 frags
		{
			gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
			EndDMLevel ();
			return;
		}
	}
}


/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i;
	edict_t	*ent;
	char	command [256];

	//SWB - mark the level as ended
	//we can use this to determine if next map failed to load
	level.map_ended = true;

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;

		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
	}

	//clear all previous persistant data and save all current data
	Kots_ClearAllConPersistData();
	Kots_SaveAllConPersistData();
}


/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame (void)
{
	int		i;
	edict_t	*ent;

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// choose a client for monsters to target this frame
	AI_SetSightClient ();

	// exit intermissions
	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}

	if (!level.intermissiontime)
	{
		//SWB - check for failed loading of maps
		if (level.map_ended)
		{
			char *mapname;
			
			if (maplist.currentmap != -1)
			{
				//need to copy the map name since we'll be removing it
				mapname = G_CopyString(Kots_Maplist_GetName(maplist.currentmap));
				gi.dprintf("ERROR: Removing map '%s' from maplist.\n", mapname);
				Kots_Maplist_RemoveMap(maplist.currentmap);
			}
			else
				mapname = "unknown";

			//print out the error to the server and players
			gi.dprintf("ERROR: There appears to have been an error loading map %s.\n", mapname);
			gi.bprintf(PRINT_HIGH, "Server couldn't load map '%s'. Restarting current map...\n", mapname);
			
			//restart the current map
			level.changemap = G_CopyString(level.mapname);
			ExitLevel();
			return;
		}

		//SWB
		//load queued characters from load thread
		Kots_ServerProcessOutput();
	}

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		VectorCopy (ent->s.origin, ent->s.old_origin);

		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
		}

		if (i > 0 && i <= maxclients->value)
		{
			//SWB
			Kots_CharacterBeginServerFrame(ent);

			ClientBeginServerFrame (ent);

			//SWB
			Kots_CharacterRunFrame(ent);
			continue;
		}

		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// see if needpass needs updated
	CheckNeedPass ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
	
	// Aldarn - Expire map vote
	Kots_Maplist_CheckVoteTimeout(NULL);

	//SWB - Spawn new monsters
	if (!coop->value)
		Kots_MonsterCheckSpawn();

	//SWB - process the slow printing queue
	Kots_ProcessSlowPrintQueue();
	Kots_ProcessQueuedSounds();

	//SWB - Check for loaded runes or if we need to reload them
	Kots_RunesCheckLoad();

	//SWB - Check for cvar changes
	Kots_CheckCvarChanges();
}


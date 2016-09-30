#include "kots_mute.h"
#include "kots_array.h"
#include "kots_admin.h"
#include "kots_server.h"
#include "kots_utils.h"

array_t *kots_mutes = NULL;

void Kots_MuteInit()
{
	kots_mutes = Array_Create(NULL, game.maxclients, game.maxclients);
}

void Kots_MuteFree()
{
	if (kots_mutes)
	{
		Array_Delete(kots_mutes);
		kots_mutes = NULL;
	}
}

void Kots_MuteListAdd(mute_info_t *info)
{
	ULONG i;

	for (i = 0; i < kots_mutes->length; i++)
	{
		mute_entry_t *entry = (mute_entry_t *)Array_GetValueAt(kots_mutes, i);

		//check if the ip address matches
		if (Q_stricmp(info->ip_address, entry->ip_address) == 0)
		{
			//already in the list
			return;
		}
	}

	{
		mute_entry_t *entry = malloc(sizeof(*entry));
		entry->character_id = info->character_id;
		Kots_strncpy(entry->ip_address, info->ip_address, sizeof(entry->ip_address));
		Kots_strncpy(entry->name, info->name, sizeof(entry->name));

		//not found in the list so add it
		Array_PushBack(kots_mutes, entry);
	}
}

void Kots_MuteListRemove(mute_info_t *info)
{
	ULONG i;

	for (i = 0; i < kots_mutes->length; i++)
	{
		mute_entry_t *entry = (mute_entry_t *)Array_GetValueAt(kots_mutes, i);

		//check if the ip address matches
		if (Q_stricmp(info->ip_address, entry->ip_address) == 0)
		{
			Array_DeleteAt(kots_mutes, i);
			break;
		}
	}
}

void Kots_MuteCheckAll()
{
	int i;
	edict_t *ent = g_edicts + 1;

	//update all the players mute settings
	for (i = 0; i < game.maxclients; i++, ent++)
	{
		if (ent->inuse)
		{
			Kots_MuteCheck(ent);
		}
	}
}

void Kots_MuteCheck(edict_t *ent)
{
	ULONG i;
	qboolean muted = false;

	for (i = 0; i < kots_mutes->length; i++)
	{
		mute_entry_t *info = (mute_entry_t *)Array_GetValueAt(kots_mutes, i);

		//check if character id matches first
		if (ent->character && ent->character->id == info->character_id)
		{
			muted = true;
			break;
		}

		//check if the ip address matches
		if (Q_stricmp(ent->client->pers.kots_persist.ip_address, info->ip_address) == 0)
		{
			muted = true;
			break;
		}
	}

	//not found must not be muted
	if (ent->client->pers.kots_persist.is_muted != muted)
	{
		if (muted)
			gi.centerprintf(ent, "You are currently muted.");
		else
			gi.centerprintf(ent, "You are no longer muted.");

		ent->client->pers.kots_persist.is_muted = muted;
	}
}

void Kots_UpdateMuteList(jobitem_t *job)
{

	if (job->result == KOTS_SERVER_SUCCESS)
	{
		array_t *list = (array_t *)job->args;

		if (list != NULL)
		{
			Array_DeleteAll(kots_mutes);
			Array_CopyValues(list, kots_mutes);
			Array_ClearAll(list);
			Array_Delete(list);
			list = NULL;
			job->args = NULL;

			//update all the players mute settings
			Kots_MuteCheckAll();

			gi.dprintf("DB: Successfully updated mute list.\n");
		}
		else
		{
			gi.dprintf("DB: ERROR - Unknown error occurred loading mute list!\n");
		}
	}
	else
	{
		gi.dprintf("DB: ERROR - Failed to load mute list!\n");
	}
}

void Kots_MutePlayer(edict_t *admin, char *name)
{
	edict_t *player = Kots_AdminFindPlayer(name, admin);

	if (player)
	{
		if (player->client->pers.kots_persist.is_muted)
			gi.cprintf(admin, PRINT_HIGH, "Player %s (%s) is already muted.\n", name, player->client->pers.kots_persist.ip_address);
		else
		{
			mute_info_t *info = malloc(sizeof(*info));

			info->admin = admin;
			info->admin_id = admin->client->pers.kots_persist.client_id;
			info->player = player;
			info->player_id = player->client->pers.kots_persist.client_id;
			info->character_id = (player->client->pers.kots_persist.is_loggedin ? player->character->id : 0);
			Kots_strncpy(info->ip_address, player->client->pers.kots_persist.ip_address, sizeof(info->ip_address));
			Kots_strncpy(info->name, player->client->pers.netname, sizeof(info->name));
		}
	}
	else
	{
		gi.cprintf(admin, PRINT_HIGH, "Unable to find player named %s.\n", name);
	}
}

void Kots_MuteIP(edict_t *admin, char *ip_address)
{
	edict_t *player = Kots_AdminFindPlayerByIp(ip_address);

	if (player == admin)
		gi.cprintf(admin, PRINT_HIGH, "You probably don't want to mute yourself.\n");
	else if (!player)
		gi.cprintf(admin, PRINT_HIGH, "You can't mute a player unless they are on the server.\n");
	else if (player->client->pers.kots_persist.is_muted)
		gi.cprintf(admin, PRINT_HIGH, "Player %s (%s) is already muted.\n", player->client->pers.netname, player->client->pers.kots_persist.ip_address);
	else
	{
		mute_info_t *info = malloc(sizeof(*info));

		info->admin = admin;
		info->admin_id = admin->client->pers.kots_persist.client_id;
		info->player = player;
		info->player_id = player->client->pers.kots_persist.client_id;
		info->character_id = (player->client->pers.kots_persist.is_loggedin ? player->character->id : 0);
		Kots_strncpy(info->name, player->client->pers.netname, sizeof(info->name));
		Kots_strncpy(info->ip_address, ip_address, sizeof(info->ip_address));
		Kots_AddMuteJob(info);
	}
}

void Kots_UnmutePlayer(edict_t *admin, char *name)
{
	edict_t *player = Kots_AdminFindPlayer(name, admin);

	if (player)
	{
		if (!player->client->pers.kots_persist.is_muted)
			gi.cprintf(admin, PRINT_HIGH, "Player %s (%s) is not muted.\n", name, player->client->pers.kots_persist.ip_address);
		else
		{
			mute_info_t *info = malloc(sizeof(*info));

			info->admin = admin;
			info->admin_id = admin->client->pers.kots_persist.client_id;
			info->player = player;
			info->player_id = player->client->pers.kots_persist.client_id;
			info->character_id = (player->client->pers.kots_persist.is_loggedin ? player->character->id : 0);
			Kots_strncpy(info->ip_address, player->client->pers.kots_persist.ip_address, sizeof(info->ip_address));
			Kots_strncpy(info->name, player->client->pers.netname, sizeof(info->name));
			Kots_AddUnmuteJob(info);
		}
	}
	else
	{
		gi.cprintf(admin, PRINT_HIGH, "Unable to find player named %s.\n", name);
		gi.cprintf(admin, PRINT_HIGH, "If you want to unmute someone not on the server you must use their IP address.\n");
	}
}

void Kots_UnmuteIP(edict_t *admin, char *ip_address)
{
	edict_t *player = Kots_AdminFindPlayerByIp(ip_address);

	if (player && !player->client->pers.kots_persist.is_muted)
		gi.cprintf(admin, PRINT_HIGH, "Player %s (%s) is not muted.\n", player->client->pers.netname, player->client->pers.kots_persist.ip_address);
	else
	{
		mute_info_t *info = malloc(sizeof(*info));

		info->admin = admin;
		info->admin_id = admin->client->pers.kots_persist.client_id;

		if (player)
		{
			info->player = player;
			info->player_id = player->client->pers.kots_persist.client_id;
			info->character_id = (player->client->pers.kots_persist.is_loggedin ? player->character->id : 0);
			Kots_strncpy(info->name, player->client->pers.netname, sizeof(info->name));
		}
		else
		{
			info->player = NULL;
			info->player_id = 0;
			info->character_id = 0;
			info->name[0] = '\0';
		}

		Kots_strncpy(info->ip_address, ip_address, sizeof(info->ip_address));
		Kots_AddUnmuteJob(info);
	}
}

void Kots_MuteList(edict_t *admin)
{
	ULONG i;
	mute_entry_t *entry;
	edict_t *ent;

	//print header for the list
	Kots_SlowPrint(admin, "%-15s - %-15s\n", "IP Address", "Name");

	//search through the array of muted ips and print a message to those still playing
	for (i = 0; i < kots_mutes->length; i++)
	{
		entry = (mute_entry_t *)Array_GetValueAt(kots_mutes, i);
		ent = Kots_AdminFindPlayerByIp(entry->ip_address);

		//if we found the player print their current name
		if (ent)
			Kots_SlowPrint(admin, "%-15s - %-15s\n", entry->ip_address, ent->client->pers.netname);
		else //otherwise print the saved name
			Kots_SlowPrint(admin, "%-15s - %-15s\n", entry->ip_address, entry->name);
	}
}

void Kots_MuteComplete(jobitem_t *job)
{
	mute_info_t *info = (mute_info_t *)job->args;

	if (Kots_ServerIsClientValid(info->admin, info->admin_id))
	{
		if (job->result == KOTS_SERVER_SUCCESS)
			gi.cprintf(info->admin, PRINT_HIGH, "Successfully muted %s (%s).\n", info->name, info->ip_address);
		else
			gi.cprintf(info->admin, PRINT_HIGH, "Failed to mute %s (%s).\n", info->name, info->ip_address);
	}

	if (job->result == KOTS_SERVER_SUCCESS)
	{
		//add the new entry to the list
		Kots_MuteListAdd(info);
		Kots_MuteCheckAll();
	}
}

void Kots_UnmuteComplete(jobitem_t *job)
{
	mute_info_t *info = (mute_info_t *)job->args;

	if (Kots_ServerIsClientValid(info->admin, info->admin_id))
	{
		if (job->result == KOTS_SERVER_SUCCESS)
			gi.cprintf(info->admin, PRINT_HIGH, "Successfully unmuted %s (%s).\n", info->name, info->ip_address);
		else
			gi.cprintf(info->admin, PRINT_HIGH, "Failed to unmute %s (%s).\n", info->name, info->ip_address);
	}

	if (job->result == KOTS_SERVER_SUCCESS)
	{
		//add the new entry to the list
		Kots_MuteListRemove(info);
		Kots_MuteCheckAll();
	}
}

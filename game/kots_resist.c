#include "kots_resist.h"

//Check if a character can set a resist or not
qboolean Kots_CharacterCanResist(edict_t *ent)
{
	if (ent->character->resist == WEAP_NONE)
		return true;

	gi.cprintf(ent, PRINT_HIGH, "You already have a resistance.\n");
	return false;
}

//Set character resistance to expack
void Kots_CharacterResistExpack(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		if (ent->character->expack > 0)
		{
			gi.cprintf(ent, PRINT_HIGH, "You cannot resist powers you have trained.\n");
			return;
		}

		ent->character->resist = POW_EXPACK;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to expack.\n");
	}
}

//Set character resistance to spiral
void Kots_CharacterResistSpiral(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		if (ent->character->spiral > 0)
		{
			gi.cprintf(ent, PRINT_HIGH, "You cannot resist powers you have trained.\n");
			return;
		}

		ent->character->resist = POW_SPIRAL;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to spiral.\n");
	}
}

//Set character resistance to bide
void Kots_CharacterResistBide(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		if (ent->character->bide > 0)
		{
			gi.cprintf(ent, PRINT_HIGH, "You cannot resist powers you have trained.\n");
			return;
		}

		ent->character->resist = POW_BIDE;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to bide.\n");
	}
}

//Set character resistance to throw
void Kots_CharacterResistThrow(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		if (ent->character->kotsthrow > 0)
		{
			gi.cprintf(ent, PRINT_HIGH, "You cannot resist powers you have trained.\n");
			return;
		}

		ent->character->resist = POW_THROW;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to throw.\n");
	}
}


void Kots_CharacterResistSabre(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_SABRE;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to sabre.\n");
	}
}

void Kots_CharacterResistShotgun(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_SHOTGUN;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to shotgun.\n");
	}
}

void Kots_CharacterResistMachinegun(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_MACHINEGUN;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to machinegun.\n");
	}
}

void Kots_CharacterResistChaingun(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_CHAINGUN;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to chaingun.\n");
	}
}

void Kots_CharacterResistSuperShotgun(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_SUPERSHOTGUN;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to super shotgun.\n");
	}
}
void Kots_CharacterResistGrenade(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_GRENADES;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to grenade.\n");
	}
}

void Kots_CharacterResistGrenadeLauncher(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_GRENADELAUNCHER;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to grenade launcher.\n");
	}
}

void Kots_CharacterResistRocketLauncher(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_ROCKETLAUNCHER;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to rocket launcher.\n");
	}
}

void Kots_CharacterResistHyperblaster(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_HYPERBLASTER;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to hyperblaster.\n");
	}
}

void Kots_CharacterResistRailgun(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_RAILGUN;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to railgun.\n");
	}
}

void Kots_CharacterResistBfg(edict_t *ent)
{
	if (Kots_CharacterCanResist(ent))
	{
		ent->character->resist = WEAP_BFG;
		gi.cprintf(ent, PRINT_HIGH, "Resistance set to BFG10K.\n");
	}
}

#include "kots_weapon.h"
#include "kots_resist.h"
#include "kots_runes.h"

qboolean is_quad;
byte     is_silenced;

//Weapon names and functions to add or resist them
#define TotalWeaponPoints 24
weaponpoint_t WeaponPoints[TotalWeaponPoints] =
{
    "sabre", WEAP_SABRE, Kots_CharacterAddSabre, Kots_CharacterResistSabre,
    "machinegun", WEAP_MACHINEGUN, Kots_CharacterAddMachinegun, Kots_CharacterResistMachinegun,
    "mg", WEAP_MACHINEGUN, Kots_CharacterAddMachinegun, Kots_CharacterResistMachinegun,
    "shotgun", WEAP_SHOTGUN, Kots_CharacterAddShotgun, Kots_CharacterResistShotgun,
    "sg", WEAP_SHOTGUN, Kots_CharacterAddShotgun, Kots_CharacterResistShotgun,
    "chaingun", WEAP_CHAINGUN, Kots_CharacterAddChaingun, Kots_CharacterResistChaingun,
    "cg", WEAP_CHAINGUN, Kots_CharacterAddChaingun, Kots_CharacterResistChaingun,
    "supershotgun", WEAP_SUPERSHOTGUN, Kots_CharacterAddSuperShotgun, Kots_CharacterResistSuperShotgun,
    "ssg", WEAP_SUPERSHOTGUN, Kots_CharacterAddSuperShotgun, Kots_CharacterResistSuperShotgun,
    "grenade", WEAP_GRENADES, Kots_CharacterAddGrenade, Kots_CharacterResistGrenade,
    "hg", WEAP_GRENADES, Kots_CharacterAddGrenade, Kots_CharacterResistGrenade,
    "grenadelauncher", WEAP_GRENADELAUNCHER, Kots_CharacterAddGrenadeLauncher, Kots_CharacterResistGrenadeLauncher,
    "gl", WEAP_GRENADELAUNCHER, Kots_CharacterAddGrenadeLauncher, Kots_CharacterResistGrenadeLauncher,
    "rocketlauncher", WEAP_ROCKETLAUNCHER, Kots_CharacterAddRocketLauncher, Kots_CharacterResistRocketLauncher,
    "rl", WEAP_ROCKETLAUNCHER, Kots_CharacterAddRocketLauncher, Kots_CharacterResistRocketLauncher,
    "hyperblaster", WEAP_HYPERBLASTER, Kots_CharacterAddHyperblaster, Kots_CharacterResistHyperblaster,
    "hb", WEAP_HYPERBLASTER, Kots_CharacterAddHyperblaster, Kots_CharacterResistHyperblaster,
    "railgun", WEAP_RAILGUN, Kots_CharacterAddRailgun, Kots_CharacterResistRailgun,
    "rail", WEAP_RAILGUN, Kots_CharacterAddRailgun, Kots_CharacterResistRailgun,
    "rg", WEAP_RAILGUN, Kots_CharacterAddRailgun, Kots_CharacterResistRailgun,
    "bfg", WEAP_BFG, Kots_CharacterAddBfg, Kots_CharacterResistBfg,
    "bfg10k", WEAP_BFG, Kots_CharacterAddBfg, Kots_CharacterResistBfg,
    "anti", WEAP_ANTI, Kots_CharacterAddWAntiweapon, NULL,
    "antiweapon", WEAP_ANTI, Kots_CharacterAddWAntiweapon, NULL
};


weaponpoint_t *Kots_GetWeaponTree(char *name)
{
    int i;

    for (i = 0; i < TotalWeaponPoints; i++)
    {
        if (Q_stricmp(name, WeaponPoints[i].name) == 0)
            return &WeaponPoints[i];
    }

    return NULL;
}

int *Kots_CharacterGetWeaponLevel(edict_t *ent, int weapon)
{
    switch (weapon)
    {
    case WEAP_SABRE:
        return &ent->character->sabre;
    case WEAP_SHOTGUN:
        return &ent->character->shotgun;
    case WEAP_SUPERSHOTGUN:
        return &ent->character->supershotgun;
    case WEAP_MACHINEGUN:
        return &ent->character->machinegun;
    case WEAP_CHAINGUN:
        return &ent->character->chaingun;
    case WEAP_GRENADES:
        return &ent->character->grenade;
    case WEAP_GRENADELAUNCHER:
        return &ent->character->grenadelauncher;
    case WEAP_ROCKETLAUNCHER:
        return &ent->character->rocketlauncher;
    case WEAP_HYPERBLASTER:
        return &ent->character->hyperblaster;
    case WEAP_RAILGUN:
        return &ent->character->railgun;
    case WEAP_BFG:
        return &ent->character->bfg;
    case WEAP_ANTI:
        return &ent->character->wantiweapon;
    case WEAP_NONE:
    default:
        return &ent->character->weaponpoints;
    }
}

int Kots_CharacterGetCurrentWeaponLevel(edict_t *ent, int weapon)
{
    switch (weapon)
    {
    case WEAP_SABRE:
        return ent->character->cur_sabre;
    case WEAP_SHOTGUN:
        return ent->character->cur_shotgun;
    case WEAP_SUPERSHOTGUN:
        return ent->character->cur_supershotgun;
    case WEAP_MACHINEGUN:
        return ent->character->cur_machinegun;
    case WEAP_CHAINGUN:
        return ent->character->cur_chaingun;
    case WEAP_GRENADES:
        return ent->character->cur_grenade;
    case WEAP_GRENADELAUNCHER:
        return ent->character->cur_grenadelauncher;
    case WEAP_ROCKETLAUNCHER:
        return ent->character->cur_rocketlauncher;
    case WEAP_HYPERBLASTER:
        return ent->character->cur_hyperblaster;
    case WEAP_RAILGUN:
        return ent->character->cur_railgun;
    case WEAP_BFG:
        return ent->character->cur_bfg;
    case WEAP_ANTI:
        return ent->character->cur_wantiweapon;
    case WEAP_NONE:
    default:
        return ent->character->weaponpoints;
    }
}

int Kots_CharacterGetWeaponCost(int weapon, int curlevel)
{
    // if max is enabled return 0 otherwise 2 points past mastery and 1 point normal
    if (weapon != WEAP_ANTI && curlevel >= 6)
        return (KOTS_ENABLE_MAX_WEAPON_SKILL ? 0 : 2);
    else
        return 1;
}

qboolean Kots_CharacterCanAddWeapon(edict_t *ent, int cost)
{
    if (cost == 0)
    {
        gi.cprintf(ent, PRINT_HIGH, "You are currently at the max and cannot train this weapon anymore.\n");
        return false;
    }

    if (ent->character->weaponpoints >= cost)
        return true;

    if (cost > 1)
        gi.cprintf(ent, PRINT_HIGH, "Not enough weapon points available (%u points needed).\n", cost);
    else
        gi.cprintf(ent, PRINT_HIGH, "Not enough weapon points available (%u point needed).\n", cost);

    return false;
}

void Kots_CharacterAddSabre(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_SABRE, ent->character->sabre);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->sabre++;
        ent->character->cur_sabre++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Sabre is now %i.\n", ent->character->sabre);
    }
}
void Kots_CharacterAddShotgun(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_SHOTGUN, ent->character->shotgun);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->shotgun++;
        ent->character->cur_shotgun++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Shotgun is now %i.\n", ent->character->shotgun);
    }
}
void Kots_CharacterAddMachinegun(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_MACHINEGUN, ent->character->machinegun);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->machinegun++;
        ent->character->cur_machinegun++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Machinegun is now %i.\n", ent->character->machinegun);
    }
}
void Kots_CharacterAddChaingun(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_CHAINGUN, ent->character->chaingun);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->chaingun++;
        ent->character->cur_chaingun++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Chaingun is now %i.\n", ent->character->chaingun);
    }
}
void Kots_CharacterAddSuperShotgun(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_SUPERSHOTGUN, ent->character->supershotgun);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->supershotgun++;
        ent->character->cur_supershotgun++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Super Shotgun is now %i.\n", ent->character->supershotgun);
    }
}
void Kots_CharacterAddGrenade(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_GRENADES, ent->character->grenade);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->grenade++;
        ent->character->cur_grenade++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Grenade is now %i.\n", ent->character->grenade);
    }
}
void Kots_CharacterAddGrenadeLauncher(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_GRENADELAUNCHER, ent->character->grenadelauncher);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->grenadelauncher++;
        ent->character->cur_grenadelauncher++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Grenade Launcher is now %i.\n", ent->character->grenadelauncher);
    }
}
void Kots_CharacterAddRocketLauncher(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_ROCKETLAUNCHER, ent->character->rocketlauncher);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->rocketlauncher++;
        ent->character->cur_rocketlauncher++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Rocket Launcher is now %i.\n", ent->character->rocketlauncher);
    }
}
void Kots_CharacterAddHyperblaster(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_HYPERBLASTER, ent->character->hyperblaster);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->hyperblaster++;
        ent->character->cur_hyperblaster++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Hyperblaster is now %i.\n", ent->character->hyperblaster);
    }
}
void Kots_CharacterAddRailgun(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_RAILGUN, ent->character->railgun);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->railgun++;
        ent->character->cur_railgun++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Railgun is now %i.\n", ent->character->railgun);
    }
}
void Kots_CharacterAddBfg(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_BFG, ent->character->bfg);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->bfg++;
        ent->character->cur_bfg++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "BFG is now %i.\n", ent->character->bfg);
    }
}

void Kots_CharacterAddWAntiweapon(edict_t *ent)
{
    int cost = Kots_CharacterGetWeaponCost(WEAP_ANTI, ent->character->wantiweapon);

    if (Kots_CharacterCanAddWeapon(ent, cost))
    {
        ent->character->wantiweapon++;
        ent->character->cur_wantiweapon++;
        ent->character->weaponpoints -= cost;
        gi.cprintf(ent, PRINT_HIGH, "Antiweapon is now %i.\n", ent->character->wantiweapon);
    }
}

void Kots_CharacterAddWeapon(edict_t *ent, char *weapon)
{
    if (!weapon || weapon[0] == '\0')
        gi.cprintf(ent, PRINT_HIGH, "Usage: %s Weapon\n", gi.argv(0));
    else
    {
        weaponpoint_t *weapon_tree = Kots_GetWeaponTree(weapon);

        if (!weapon_tree)
            gi.cprintf(ent, PRINT_HIGH, "%s is not a valid weapon tree.\n", weapon);
        else
            weapon_tree->add(ent);
    }
}

void Kots_CharacterSetWeapon(edict_t *ent)
{
    if (gi.argc() != 4)
        gi.cprintf(ent, PRINT_HIGH, "Usage: %s PlayerName Weapon Level\n", gi.argv(0));
    else
    {
        int i;
        edict_t *other = g_edicts + 1;
        char *name = gi.argv(1);
        char *weapon = gi.argv(2);
        int level = atoi(gi.argv(3));

        for (i = 0; i < game.maxclients; i++, other++)
        {
            if (!other->inuse || !other->character || !other->character->is_loggedin)
                continue;

            if (Q_stricmp(other->client->pers.netname, name) == 0)
            {
                weaponpoint_t *weapon_tree = Kots_GetWeaponTree(weapon);
                
                if (!weapon_tree)
                {
                    gi.cprintf(ent, PRINT_HIGH, "Unrecognized weapon, %s.\n", weapon);
                    return;
                }
                else
                {
                    int *points = Kots_CharacterGetWeaponLevel(other, weapon_tree->weapon);

                    if (level < 0)
                    {
                        gi.cprintf(ent, PRINT_HIGH, "Invalid weapon level specified.\n");
                        return;
                    }
                    else
                    {
                        if (other->character->rune)
                            Kots_RuneRemoveAbilities(other, other->character->rune);

                        *points = level;
                        Kots_CharacterResetStats(other);

                        if (other->character->rune)
                            Kots_RuneAddAbilities(other, other->character->rune);

                        gi.cprintf(other, PRINT_HIGH, "Your %s is now level %i thanks to %s.\n", weapon_tree->name, *points, ent->character->name);
                        gi.cprintf(ent, PRINT_HIGH, "You changed %s's %s level to %i.\n", other->character->name, weapon_tree->name, *points);
                        return;
                    }
                }
            }
        }

        gi.cprintf(ent, PRINT_HIGH, "Unable to find a player logged in with that name.\n");
    }
}

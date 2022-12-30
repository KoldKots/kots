#include "kots_weapondamage.h"
#include "kots_runes.h"
#include "kots_utils.h"

int Kots_CharacterSabreDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_sabre)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 90);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 100);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 120);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 140);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 160);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 200);
    default: // > 6
        return Kots_CharacterMunitionDamagePlus(ent, 200, ent->character->cur_sabre);
    }
}

int Kots_CharacterShotgunDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_shotgun)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 8);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 10);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 12);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 14);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 15);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 16);
    default: // > 6
        return Kots_CharacterMunitionDamagePlus(ent, 16, ent->character->cur_shotgun);
    }
}

int Kots_CharacterSuperShotgunDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_supershotgun)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 7);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 8);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 9);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 12);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 13);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 14 + Kots_RandRound(0.70)); //14.7
    default: // > 6
        return Kots_CharacterMunitionDamagePlus(ent, 15, ent->character->cur_supershotgun);
    }
}

int Kots_CharacterMachinegunDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_machinegun)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 10);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 12);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 14);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 16);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 20);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 25);
    default: // > 6
        if (ent->character->cur_machinegun >= 10 && ent->character->rune && ent->character->rune->machinegun > 0)
            return Kots_CharacterMunitionDamagePlus(ent, 32, ent->character->cur_machinegun);
        else
            return Kots_CharacterMunitionDamagePlus(ent, 25, ent->character->cur_machinegun);
    }
}

int Kots_CharacterChaingunDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_chaingun)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 7);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 8);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 9);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 12);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 15);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 18);
    default: // > 6
        if (ent->character->cur_chaingun >= 10 && ent->character->rune && ent->character->rune->chaingun > 0)
            return Kots_CharacterMunitionDamagePlus(ent, 21, ent->character->cur_chaingun);
        else
            return Kots_CharacterMunitionDamagePlus(ent, 18, ent->character->cur_chaingun);
    }
}

int Kots_CharacterGrenadeDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_grenade)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 150);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 175);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 200);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 225);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 275);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 350);
    default: // > 6
        if (ent->character->cur_grenade >= 10 && ent->character->rune && ent->character->rune->grenade > 0)
            return Kots_CharacterMunitionDamagePlus(ent, 400, ent->character->cur_grenade);
        else
            return Kots_CharacterMunitionDamagePlus(ent, 350, ent->character->cur_grenade);
    }
}

int Kots_CharacterGrenadeLauncherDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_grenadelauncher)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 140);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 180);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 200);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 220);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 240);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 300);
    default: // > 6
        return Kots_CharacterMunitionDamagePlus(ent, 300, ent->character->cur_grenadelauncher);
    }
}

int Kots_CharacterRocketLauncherDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_rocketlauncher)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 140);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 180);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 200);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 220);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 240);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 300);
    default: // > 6
        //rune ability does less damage but fires 2
        if (ent->character->cur_rocketlauncher >= 10 && ent->character->rune && ent->character->rune->rocketlauncher > 0)
            return Kots_CharacterMunitionDamagePlus(ent, 200, ent->character->cur_rocketlauncher);
        else
            return Kots_CharacterMunitionDamagePlus(ent, 300, ent->character->cur_rocketlauncher);
    }
}

int Kots_CharacterHyperblasterDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_hyperblaster)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 20);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 25);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 30);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 40);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 45);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 55);
    default: // > 6
        return Kots_CharacterMunitionDamagePlus(ent, 45, ent->character->cur_hyperblaster);
    }
}

int Kots_CharacterRailgunDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_railgun)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 125);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 150);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 175);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 200);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 225);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 250);
    default: // > 6
        return Kots_CharacterMunitionDamagePlus(ent, 250, ent->character->cur_railgun);
    }
}

int Kots_CharacterBfgDamage(edict_t *ent, int damage)
{
    switch (ent->character->cur_bfg)
    {
    case 0:
        return Kots_CharacterMunitionDamage(ent, damage);
    case 1:
        return Kots_CharacterMunitionDamage(ent, 43);
    case 2:
        return Kots_CharacterMunitionDamage(ent, 48);
    case 3:
        return Kots_CharacterMunitionDamage(ent, 53);
    case 4:
        return Kots_CharacterMunitionDamage(ent, 60);
    case 5:
        return Kots_CharacterMunitionDamage(ent, 70);
    case 6:
        return Kots_CharacterMunitionDamage(ent, 88);
    default: // > 6
        return Kots_CharacterMunitionDamagePlus(ent, 88, ent->character->cur_bfg);
    }
}

int Kots_CharacterMunitionDamagePlus(edict_t *ent, float damage, int weapon_level)
{
    // If our weapon level is above 6 and max weapon skill is not enabled then add the bonus damage
    // Otherwise don't add bonus damage above level 6
    if (weapon_level > 6 && !KOTS_ENABLE_MAX_WEAPON_SKILL)
    {
        if (ent->character->cur_munition == 0)
            return Kots_RandMultiply(damage, 1 + KOTS_BONUS_WEAPON * (weapon_level - 6));
        else
            return Kots_RandMultiply(damage, 1 + (KOTS_BONUS_WEAPON * (weapon_level - 6)) + (MUNITION_BONUS * ent->character->cur_munition));
    }
    else
        return Kots_CharacterMunitionDamage(ent, Kots_RandMultiply(1, damage));
}

int Kots_CharacterMunitionDamage(edict_t *ent, int damage)
{
    if (ent->character->cur_munition == 0)
        return damage;
    else
        return Kots_RandMultiply(damage, 1 + MUNITION_BONUS * ent->character->cur_munition);
}

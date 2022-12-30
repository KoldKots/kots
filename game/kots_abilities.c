#include "kots_abilities.h"
#include "kots_items.h"
#include "kots_runes.h"
#include "kots_utils.h"
#include "kots_weapon.h"
#include "kots_pickups.h"
#include "kots_pweapon.h"
#include "kots_hook.h"
#include "kots_weapondamage.h"

typedef struct ammotypes_s
{
    char *ammotype;
    int weapons[2];
    int weaponcount;
} ammotypes_t;

const int AmmoTypeLength = 6;
ammotypes_t ammo_types[] = {
    {"Shells", {WEAP_SHOTGUN, WEAP_SUPERSHOTGUN}, 2},
    {"Bullets", {WEAP_MACHINEGUN, WEAP_CHAINGUN}, 2},
    {"Grenades", {WEAP_GRENADES, WEAP_GRENADELAUNCHER}, 2},
    {"Rockets", {WEAP_ROCKETLAUNCHER}, 1},
    {"Cells", {WEAP_HYPERBLASTER, WEAP_BFG}, 2},
    {"Slugs", {WEAP_RAILGUN}, 1}
};

void Kots_CheckFastWeaponSwitch(edict_t *ent)
{
    if (ent->character->cur_dexterity >= 1)
    {
        if (ent->client->weaponstate != WEAPON_FIRING)
            ChangeWeapon(ent);
    }
}

void Kots_CharacterDoDamageBasedSkills(edict_t *targ, edict_t *attacker, int health, int total, int realtotal)
{
    if (targ != attacker)
    {
        //keep the means of death it may change mid frame based on stuff like empathy
        int modBefore = meansOfDeath;

        //don't do this stuff when the attacker is dead
        if (attacker->health > 0)
        {
            //Since monsters can do a ton of damage they should only vamp what is really done
            if (!attacker->client)
            {
                if (health > targ->health)
                    Kots_CharacterCheckVampire(targ, attacker, targ->health);
                else
                    Kots_CharacterCheckVampire(targ, attacker, health);
            }
            else
                Kots_CharacterCheckVampire(targ, attacker, health);

            Kots_CharacterCheckAmmoSteal(targ, attacker, total);
            Kots_CharacterCheckQuadRage(targ, attacker, realtotal);

            //Since monsters do a ton of damage don't empathy more damage than was really done
            if (!attacker->client)
                Kots_CharacterCheckEmpathyDamage(targ, attacker, realtotal);
            else
                Kots_CharacterCheckEmpathyDamage(targ, attacker, total);

            //SWB - moved here because this needs to take place
            //after monster bonuses and other things that cause invulnerability to damage
            if (targ->character->bideon && targ->character->bidestart > level.time - 10)
                targ->character->bidedmg += total / 2; // 50%
        }

        Kots_CharacterCheckKnock(targ, attacker, total);
        Kots_CharacterCheckPoison(targ, attacker, total);
        Kots_CheckArmorKnock(targ, attacker, total);

        //restore means of death in case it changed;
        meansOfDeath = modBefore;
    }
}

void Kots_CharacterFly(edict_t *ent)
{
    if (ent->character->is_loggedin)
    {
        if (ent->character->cur_wisdom < 3)
            gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to fly.\n");
        else if (ent->character->cubes < 1)
            gi.cprintf(ent, PRINT_HIGH, "You do not have enough cubes.\n");
        else if (ent->character->next_empdone > level.time)
            gi.cprintf(ent, PRINT_HIGH, "You were hit by an EMP Shockwave and this ability is currently disabled!\n"); //hit by emp shockwave can't fly
        else
        {
            if (!ent->character->is_flying)
            {
                if (level.time < ent->character->next_flytime)
                {
                    gi.cprintf(ent, PRINT_HIGH, "You must wait %0.1f more seconds to fly again.\n", ent->character->next_flytime - level.time);
                    return;
                }

                ent->character->is_flying = true;

                //if our next cube usage will not be immediately then use a cube now
                if (ent->character->next_fast_regen >= level.time + 0.1)
                    ent->character->cubes -= 1;
            }
            else
                ent->character->is_flying = false;
        }
    }
}

void Kots_CharacterLand(edict_t *ent)
{
    if (ent->character->is_flying)
    {
        ent->character->is_flying = false;
        ent->character->next_flycharge = level.time + KOTS_FLY_CHARGE_COOLDOWN;
    }
}

void Kots_CharacterHighJumpToggle(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->character->using_highjump = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->character->using_highjump = false;
    else if (!ent->character->using_highjump)
        ent->character->using_highjump = true;
    else
        ent->character->using_highjump = false;

    if (ent->character->using_highjump)
        gi.cprintf(ent, PRINT_HIGH, "KOTS Jump is enabled.\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "KOTS Jump is disabled.\n");
}

void Kots_CharacterCheckDoubleJump(edict_t *ent, pmove_t *pm)
{
    if (pm->groundentity != NULL || pm->waterlevel >= 2)
    {
        ent->character->canDoubleJump = true;
        return;
    }
    else if (!ent->character->canDoubleJump)
        return;

    //no double jumps from anywhere in water
    if (pm->waterlevel)
        return;

    if (ent->character->cur_strength < 2)
        return;

    if (!ent->character->using_highjump)
        return;

    if (pm->cmd.upmove < 10)
        return;

    // must wait for jump to be released
    if (pm->s.pm_flags & PMF_JUMP_HELD)
        return;

    if (pm->s.pm_type == PM_DEAD)
        return;

    if (ent->client->pers.spectator)
        return;

    ent->character->canDoubleJump = false;
    pm->s.pm_flags |= PMF_JUMP_HELD;

    pm->s.velocity[2] += 270 * 8;
    if (pm->s.velocity[2] < 270 * 8)
        pm->s.velocity[2] = 270 * 8;

    //keep us from taking fall damage
    VectorClear(ent->client->oldvelocity);

    if (ent->character->cur_dexterity < 2)
    {
        gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
        PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
    }
}

void Kots_CharacterCheckFly(edict_t *ent, pmove_t *pm)
{
    if (ent->character->is_flying)
    {
        //must meet the minimum velocity to get up off the ground
        if (ent->groundentity)
        {
            //ent->groundentity = NULL;
            pm->s.velocity[2] = 270 * 6;
        }
        else //once in the air move at half jump velocity
        {
            pm->s.velocity[2] += 270 * 5; // Aldarn - Modified so you move upwards slightly more
            if (pm->s.velocity[2] > 270 * 5)
                pm->s.velocity[2] = 270 * 5;
        }

        //keep us from taking fall damage
        VectorClear(ent->client->oldvelocity);
    }
}

void Kots_CharacterFlyCheckCharge(edict_t *ent)
{
    if (ent->character->is_flying)
    {
        ent->character->fly_charge -= 0.1;
        if (ent->character->fly_charge <= 0)
        {
            // If we run out of charge then stop us from flying and force a full recharge
            Kots_CharacterLand(ent);
            ent->character->next_flytime = level.time + KOTS_FLY_COOLDOWN;
            return;
        }
    }
    else if (ent->character->next_flycharge < level.time)
    {
        // Recharge while we're not flying
        if (ent->character->fly_charge < KOTS_FLY_MAX_CHARGE)
        {
            ent->character->fly_charge += 0.1;
        }
    }
}

void Kots_CharacterFlyUseCubes(edict_t *ent)
{
    if (ent->character->is_flying)
    {
        if (ent->character->cubes < 1)
            Kots_CharacterLand(ent);
        else
            ent->character->cubes -= 1;
    }
}

void Kots_CharacterCheckHook(edict_t *ent)
{
    // Check if our hook is still out
    if (ent->character->next_hookrelease > 0)
    {
        // If our hook has been attached for too long then reset it
        if (ent->character->next_hookrelease <= level.time)
        {
            CTFPlayerResetGrapple(ent);
            ent->character->next_hookrelease = 0;
        }
    }
}

void Kots_CharacterCheckHealthRegen(edict_t *ent)
{
    int startHealth = Kots_CharacterGetStartHealth(ent);
    int maxHealth = Kots_CharacterGetMaxRegenHealth(ent);
    int regen = 0;

    if (ent->character->cur_karma >= 10)
        regen = 12;
    else if (ent->character->cur_karma >= 7)
        regen = 9;
    else if (ent->character->cur_karma >= 5)
        regen = 4;
    else if (ent->character->cur_karma >= 3)
        regen = 2;
    else
        return;

    if (ent->health < 50)
        regen *= 2;
    else if (ent->health >= startHealth)
    {
        if (ent->character->cur_karma >= 10)
            regen = 2;
        else if (ent->character->cur_karma >= 7)
            regen = 1;
        else
            return;
    } else if (ent->character->cur_karma < 7 && ent->health + regen > startHealth)
        regen = (startHealth - ent->health);

    if (regen > 0)
    {
        if (ent->health < maxHealth)
        {
            ent->health += regen;

            // Sound is annoying!
            //if (ent->health < startHealth)
            //  gi.sound(ent, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);

            if (ent->health > maxHealth)
                ent->health = maxHealth;
        }
    }
}

void Kots_CharacterCheckArmorRegen(edict_t *ent)
{
    if (!ent->client)
        return;

    if (ent->character->cur_technical >= 5)
    {
        int maxarmor = Kots_CharacterGetMaxArmorRegen(ent);
        int index = ITEM_INDEX(FindItem("Body Armor"));

        if (ent->client->pers.inventory[index] < maxarmor)
        {
            if (ent->character->cur_technical >= 7)
                ent->client->pers.inventory[index] += 2;
            if (ent->client->pers.inventory[index] > maxarmor)
                ent->client->pers.inventory[index] = maxarmor;
        }
    }
}

void Kots_CharacterCheckTballRegen(edict_t *ent)
{
    if (ent->character->rune && ent->character->rune->tball_regen > 0)
    {
        //tballs regen every 5 seconds so if this isn't a multiple of 5 then exit now
        if ((int)ent->character->next_regen % 5)
            return;

        //maximum of 8 tballs at a time
        if (ent->character->tballs <= 0)
            ent->character->tballs = 1;
        else if (ent->character->tballs < 8)
            ent->character->tballs++;
    }
}

void Kots_CharacterCheckAmmoRegen(edict_t *ent)
{
    if (!ent->client)
        return;

    if (ent->character->cur_strength >= 7)
    {
        int i, j, count;

        if (ent->character->cur_strength < 10)
        {
            //strength 7-9 regenerate every 3 seconds
            if ((int)ent->character->next_regen % 3)
                return;
        } //strength 10+ regen every 2 seconds
        else if ((int)ent->character->next_regen % 2)
            return;

        for (i = 0; i < AmmoTypeLength; i++)
        {
            count = 0;
            //for every weapon level 2 or more we get 20% of the base pickup
            for (j = 0; j < ammo_types[i].weaponcount; j++)
            {
                if (Kots_CharacterGetCurrentWeaponLevel(ent, ammo_types[i].weapons[j]) >= 2)
                    ++count;
            }

            if (count > 0)
            {
                int max, index;
                gitem_t *ammo = FindItem(ammo_types[i].ammotype);
                max = Kots_CharacterGetMaxAmmo(ent, ammo->tag);
                index = ITEM_INDEX(ammo);

                switch (ammo->tag)
                {
                case AMMO_BULLETS:
                case AMMO_CELLS:
                    ent->client->pers.inventory[index] += count * ammo->quantity * 0.10;
                    break;
                default:
                    ent->client->pers.inventory[index] += count * ammo->quantity * 0.20;
                    break;
                }

                //ensure we don't go over max ammo
                if (ent->client->pers.inventory[index] > max)
                    ent->client->pers.inventory[index] = max;
            }
        }
    }
}

void Kots_CharacterCheckCubeRegen(edict_t *ent)
{
    if (ent->character->cur_wisdom >= 7)
    {
        if ((int)ent->character->next_regen % 2)
            return;

        Kots_CharacterPickupCubes(ent, 1);
    }
}

void Kots_CharacterLiftItem(edict_t *ent)
{
    edict_t *targ = NULL, *who = NULL, *best = NULL;
    float bd = 0, d = 0, dx = 0, dy = 0;
    vec3_t  forward, dir;
    int count = 0;


    if (ent->character->cubes < 2)
    {
        gi.cprintf(ent, PRINT_HIGH, "You do not have enough cubes.\n");
        return;
    }

    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    while ((targ = findradius(targ, ent->s.origin, 2000)) != NULL)
    {
        if (++count >= MAX_EDICTS)
            break;

        if (!targ->item)
            continue;

        if (!(targ->spawnflags & (DROPPED_PLAYER_ITEM| DROPPED_ITEM)) && !(targ->item->tag & KOTS_ITEM_LIFTABLE))
            continue;
        //if (!(targ->item->tag & KOTS_ITEM_LIFTABLE) &&
            //(!targ->owner || !targ->item->weapmodel))
            //continue;

        who = targ;
        VectorSubtract(who->s.origin, ent->s.origin, dir);
        VectorNormalize(dir);
        d = DotProduct(forward, dir);

        //give preferred items a small boost in dot product if they meet min reqs
        if (d >= 0.90 && targ->item->tag & KOTS_ITEM_LIFT_PREFERRED)
            d += 0.25;

        if (d > bd && loc_CanSee(ent, who))
        {
            bd = d;
            best = who;
        }
    }

    if (bd < 0.90)
        return;
    if (best == NULL)
        return;

    targ = best;
    dy = ent->s.origin[1] - targ->s.origin[1];
    dx = ent->s.origin[0] - targ->s.origin[0];
    VectorSet (targ->velocity, dx, dy, 400);

    //track who lifted/tossed item last and when for kills
    targ->target_ent = ent;
    targ->wait = level.time;

    //remove the cubes used
    ent->character->cubes -= 2;

    //uncloak the character if cloaked
    Kots_CharacterUncloak(ent);
}

void Kots_CharacterTossItem(edict_t *ent)
{
    edict_t *targ = NULL, *who = NULL, *best = NULL;
    float bd = 0, d = 0, dx = 0, dy = 0, absx, absy;
    vec3_t  forward, dir;
    int count = 0;


    if (ent->character->cubes < 2)
    {
        gi.cprintf(ent, PRINT_HIGH, "You do not have enough cubes.\n");
        return;
    }

    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    while ((targ = findradius(targ, ent->s.origin, 2000)) != NULL)
    {
        if (++count >= MAX_EDICTS)
            break;

        if (!targ->item)
            continue;

        if (!(targ->spawnflags & (DROPPED_PLAYER_ITEM| DROPPED_ITEM)) && !(targ->item->tag & KOTS_ITEM_LIFTABLE))
            continue;

        who = targ;
        VectorSubtract(who->s.origin, ent->s.origin, dir);
        VectorNormalize(dir);
        d = DotProduct(forward, dir);

        //give preferred items a small boost in dot product if they meet min reqs
        if (d >= 0.90 && targ->item->tag & KOTS_ITEM_LIFT_PREFERRED)
            d += 0.25;

        if (d > bd && loc_CanSee(ent, who))
        {
            bd = d;
            best = who;
        }
    }

    if (bd < 0.90)
        return;
    if (best == NULL)
        return;

    targ = best;
    dy = targ->s.origin[1] - ent->s.origin[1];
    dx = targ->s.origin[0] - ent->s.origin[0];
    absx = abs(dx);
    absy = abs(dy);

    //if x is greater than y we scale based on x because it's our biggest velocity
    if (absx > absy)
    {
        //check minimum velocity
        if (absx < 150.0)
        {
            dy *= 100.0 / absx;
            dx *= 100.0 / absx;
        }
        //check maximum velocty
        else if (absy > 600.0)
        {
            dx *= 600.0 / absy;
            dy *= 600.0 / absy;
        }
    }
    else //otherwise scale from y
    {
        //check minimum velocity
        if (absy < 150.0)
        {
            dx *= 100.0 / absy;
            dy *= 100.0 / absy;
        }
        //check maximum veloctys
        else if (absx > 600.0)
        {
            dy *= 600.0 / absx;
            dx *= 600.0 / absx;
        }
    }

    VectorSet (targ->velocity, dx * 2, dy * 2, 400);

    //track who lifted/tossed item last and when for kills
    targ->target_ent = ent;
    targ->wait = level.time;

    //remove the cubes used
    ent->character->cubes -= 2;

    //uncloak the character if cloaked
    Kots_CharacterUncloak(ent);
}

void Kots_CharacterToggleCloaking(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        Kots_CharacterCloak(ent);
    else if (Q_stricmp(value, "off") == 0)
        Kots_CharacterUncloak(ent);
    else if (!ent->character->is_cloaked && ent->character->next_cloak < level.time)
        Kots_CharacterCloak(ent);
    else
        Kots_CharacterUncloak(ent);
}

void Kots_CharacterCloak(edict_t *ent)
{
    if (ent->character->cur_wisdom < 5)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to cloak.\n");
    else if (ent == spreewar.warent)
        gi.cprintf(ent, PRINT_HIGH, "You can't cloak while you are on a spree war!\n");
    else if (ent->character->next_empdone > level.time)
        gi.cprintf(ent, PRINT_HIGH, "You were hit by an EMP Shockwave and this ability is currently disabled!\n"); //hit by emp shockwave can't cloak
    else if (!ent->character->is_flashlighted && ent->character->next_empdone <= level.time)
    {
        //don't allow cloaking while weapon is still firing
        if (ent->client->weaponstate == WEAPON_FIRING)
            return;

        //if we're not cloaked
        if (!ent->character->is_cloaked && ent->character->next_cloak < level.time)
        {
            if (ent->character->cur_wisdom >= 7)
                ent->character->next_cloak = level.time + 1.0;
            else
                ent->character->next_cloak = level.time + 2.0;

            gi.bprintf(PRINT_HIGH, "%s cloaks.\n", ent->character->name);
            gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/spawn1.wav"), 1, ATTN_NORM, 0);
        }
    }
}

void Kots_CharacterUncloak(edict_t *ent)
{
    //TODO: Maybe use this in the future
    //      But what about accidentally firing right after spawn?
    //anything that causes us to uncloak should cause us to lose our spawn invuln
    /*
    if (ent->character->next_candamage > level.time)
    {
        if (ent->client)
        {
            //remove only spawn invuln from client
            ent->client->invincible_framenum -= self->character->next_candamage - level.time;
        }

        ent->character->next_candamage = 0;
    }
    */

    if (ent->character->is_cloaked || ent->character->next_cloak > level.time)
    {
        gi.cprintf(ent, PRINT_HIGH, "Cloaking deactivated.\n");
        ent->character->next_cloak = 0;
        ent->character->is_cloaked = false;
    }
}

void Kots_CharacterCloakUseCubes(edict_t *ent)
{
    if (ent->character->is_cloaked)
    {
        if (ent->character->cubes > 1)
            ent->character->cubes -= 1;
        else
            Kots_CharacterUncloak(ent);
    }
}

void Kots_CharacterCheckCloak(edict_t *ent)
{
    //reset noclient, we'll determine this later
    ent->svflags &= ~SVF_NOCLIENT;

    if (ent->health <= 0)
        return;

    if (ent->character->next_cloak && ent->character->next_cloak <= level.time)
    {
        ent->character->is_cloaked = true;
        ent->character->next_cloak = 0; //reset cloak time
    }

    if (ent->character->next_cloak > level.time)
        ent->s.renderfx |= RF_TRANSLUCENT;
    else if (ent->character->is_cloaked)
    {
        if (ent->character->is_flashlighted)
        {
            if (ent->character->is_flashlighted == KOTS_FLASHLIGHT_MASTER)
                ent->s.effects |= EF_SPHERETRANS; //alpha 0.6
            else
                ent->s.effects |= EF_SPHERETRANS | EF_TRACKER; //alpha 0.3
        }
        else
        {
            ent->svflags |= SVF_NOCLIENT;

            if (ent->client)
            {
                VectorSet(ent->client->damage_blend, 0, 0, 0);
                ent->client->damage_alpha = 0.33;
            }
        }
    }
}

void Kots_CharacterToggleFlashlight(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        Kots_CharacterFlashlightOn(ent);
    else if (Q_stricmp(value, "off") == 0)
        Kots_CharacterFlashlightOff(ent);
    else if (!ent->client->pers.kots_persist.using_flashlight || !ent->character->flashlight)
        Kots_CharacterFlashlightOn(ent);
    else
        Kots_CharacterFlashlightOff(ent);
}

void Kots_CharacterFlashlightOn(edict_t *ent)
{
    if (ent->character->cur_spirit < 1)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to use the flashlight.\n");
    else
    {
        gi.cprintf(ent, PRINT_HIGH, "Flashlight activated.\n");
        ent->client->pers.kots_persist.using_flashlight = true;

        if (ent->character->flashlight)
            G_FreeEdict(ent->character->flashlight);

        ent->character->flashlight = G_Spawn();
        ent->character->flashlight->owner = ent;
        ent->character->flashlight->movetype = MOVETYPE_NOCLIP;
        ent->character->flashlight->solid = SOLID_NOT;
        ent->character->flashlight->classname = "flashlight";
        ent->character->flashlight->s.modelindex = gi.modelindex("models/objects/flash/tris.md2");
        //ent->character->flashlight->s.modelindex = gi.modelindex("models/objects/smoke/tris.md2");
        ent->character->flashlight->s.skinnum = 0;
        ent->character->flashlight->s.renderfx = RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_FULLBRIGHT;
        ent->character->flashlight->s.effects = EF_SPHERETRANS | EF_HYPERBLASTER;
        ent->character->flashlight->think = Kots_CharacterFlashlightThink;
        ent->character->flashlight->nextthink = level.time + 0.1;
        VectorSet(ent->character->flashlight->mins, -2, -2, -2);
        VectorSet(ent->character->flashlight->maxs, 2, 2, 2);
    }
}

void Kots_CharacterFlashlightOff(edict_t *ent)
{
    if (ent->client->pers.kots_persist.using_flashlight)
    {
        gi.cprintf(ent, PRINT_HIGH, "Flashlight deactivated.\n");
        ent->client->pers.kots_persist.using_flashlight = false;

        if (ent->character->flashlight)
            G_FreeEdict(ent->character->flashlight);
    }

    ent->character->flashlight = NULL;
}

void Kots_CharacterFlashlightThink(edict_t *self)
{
    if (!self->owner || !self->owner->inuse || !self->owner->character || !self->owner->character->is_loggedin
        || !self->owner->client || !self->owner->client->pers.kots_persist.using_flashlight)
    {
        if (self->owner)
            self->owner->character->flashlight = NULL;

        G_FreeEdict(self);
        return;
    }
    else if (self->owner->health <= 0)
    {
        //hide the flashlight while dead
        self->svflags |= SVF_NOCLIENT;
    }
    else
    {
        vec3_t start, end, offset;
        vec3_t  forward, right;
        trace_t tr;

        AngleVectors(self->owner->client->v_angle, forward, right, NULL);
        VectorSet(offset, 8 , 6, self->owner->viewheight - 7);
        P_ProjectSource(self->owner->client, self->owner->s.origin, offset, forward, right, start);
        VectorMA(start, 8192, forward, end);

        //perform initial trace with a wider area
        tr = gi.trace(start, self->mins, self->maxs, end, self->owner, CONTENTS_SOLID | CONTENTS_MONSTER | MASK_WATER);

        //only go through transparent water
        if ((tr.contents & MASK_WATER) && (tr.surface->flags & (SURF_TRANS33 | SURF_TRANS66)))
            tr = gi.trace(tr.endpos, NULL, NULL, end, tr.ent, CONTENTS_SOLID | CONTENTS_MONSTER | MASK_WATER);

        //if we didn't find a living character in our path do a tace without the width
        if (tr.ent->character && tr.ent->health > 0)
        {
            float recloak_time = 1.0;

            //if already cloaking this will make them somewhat visible
            if (self->owner->character->spirit >= 7)
            {
                tr.ent->character->is_flashlighted = KOTS_FLASHLIGHT_MASTER;
                recloak_time = 1.0;
            }
            else
            {
                tr.ent->character->is_flashlighted = KOTS_FLASHLIGHT_NORM;

                if (self->owner->character->spirit >= 5)
                    recloak_time = 0.5;
                else
                    recloak_time = 0.25;
            }

            //prevent from cloaking if not fully clocked
            if (tr.ent->character->next_cloak > level.time)
                tr.ent->character->next_cloak = 0;
            else
            {
                float new_time;
                if (tr.ent->character->cur_wisdom >= 7)
                    new_time = level.time + (1.0 * recloak_time);
                else
                    new_time = level.time + (2.0 * recloak_time);

                if (new_time > tr.ent->character->next_unflashlight)
                    tr.ent->character->next_unflashlight = new_time;
            }

        }

        VectorCopy(tr.endpos, self->s.origin);

        //make sure the entity is visible
        self->svflags &= ~SVF_NOCLIENT;
        gi.linkentity (self);
    }

    self->nextthink = level.time + 0.1;
}

void Kots_CharacterHook(edict_t *ent)
{
    if (ent->character->cur_dexterity >= 3)
    {
        int cubes;

        if (ent->character->cur_dexterity >= 10)
            cubes = 1;
        else if (ent->character->cur_dexterity >= 7)
            cubes = 2;
        else
            cubes = 8;

        if (ent->character->cubes < cubes)
        {
            gi.cprintf(ent, PRINT_HIGH, "Not enough cubes to use the hook.\n");
            return;
        }

        //uncloak the character if cloaked
        Kots_CharacterUncloak(ent);

        ent->character->cubes -= cubes; // Aldarn - negate cubes
        CTFGrappleFire (ent, vec3_origin, 10, 0);
    }
    else
        gi.cprintf(ent, PRINT_HIGH, "You do not have the power to use the hook yet.\n");
}

void Kots_CharacterUnhook(edict_t *ent)
{
    CTFPlayerResetGrapple(ent);
}

void Kots_CharacterKnockWeapon(edict_t *ent)
{
    if (!ent->client)
        return;

    if (ent->client->pers.weapon)
    {
        if (ent->client->pers.weapon->weapmodel != WEAP_SABRE && ent->client->pers.weapon->weapmodel != WEAP_GRENADES)
        {
            gitem_t *oldweapon = ent->client->pers.weapon;
            Use_Weapon(ent, FindItem("Sabre"));
            ChangeWeapon(ent);
            Drop_Weapon(ent, oldweapon);
            gi.sound(ent, CHAN_AUTO, gi.soundindex("gladiator/GLDSRCH1.WAV"), 1, ATTN_NORM, 0);
        }
    }
}

void Kots_CharacterCheckKnock(edict_t *targ, edict_t *attacker, int damage)
{
    int knock;

    if (targ->character->cur_dexterity >= 10)
        return;

    if (attacker->character->cur_strength < 5)
        return;

    if (attacker->client && !attacker->client->pers.kots_persist.using_knock)
        return;

    if (targ->character->next_knock > level.time)
        return;

    targ->character->knockdamage += damage;

    if (attacker->character->cur_strength >= 7)
        knock = 250;
    else if (attacker->character->cur_strength >= 5)
        knock = 400;

    if (targ->character->knockdamage > knock)
    {
        float chance = 0.5;
        if (targ->character->cur_dexterity >= 7)
            chance -= 0.06 * targ->character->cur_dexterity;
        else if (targ->character->cur_dexterity > 0)
            chance -= 0.02 * targ->character->cur_dexterity;

        if (Kots_RandRound(chance))
            Kots_CharacterKnockWeapon(targ);

        targ->character->knockdamage = 0;
        targ->character->next_knock = level.time + 2;
    }
}

void Kots_CharacterCheckKnockCounter(edict_t *ent)
{
    if (ent->character->knockdamage > 0)
    {
        if (ent->character->cur_dexterity >= 7)
            ent->character->knockdamage -= 30;
        else if (ent->character->cur_dexterity >= 5)
            ent->character->knockdamage -= 20;
        else
            ent->character->knockdamage -= 10;

        if (ent->character->knockdamage < 0)
            ent->character->knockdamage = 0;
    }
}

void Kots_CharacterToggleKnock(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->client->pers.kots_persist.using_knock = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->client->pers.kots_persist.using_knock = false;
    else if (!ent->client->pers.kots_persist.using_knock)
        ent->client->pers.kots_persist.using_knock = true;
    else
        ent->client->pers.kots_persist.using_knock = false;

    if (ent->client->pers.kots_persist.using_knock)
        gi.cprintf(ent, PRINT_HIGH, "Weapon knock is enabled.\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "Weapon knock is disabled.\n");
}

void Kots_CharacterCheckVampire(edict_t *targ, edict_t *attacker, int damage)
{
    //check for vampire (health steal)
    //don't steal while using empathy
    if (attacker->character->cur_wisdom >= 1 && !targ->character->using_empathy)
    {
        //no stealing health over max
        if (attacker->health < attacker->max_health)
        {
            //only receive 50% health steal from monsters
            if (!(attacker->svflags & SVF_MONSTER) && (targ->svflags & SVF_MONSTER))
                damage /= 2;

            //steal 25% of the health
            attacker->health += Kots_RandMultiply(damage, 0.25);

            //ensure we don't steal over our max
            if (attacker->health > attacker->max_health)
                attacker->health = attacker->max_health;
        }
    }
}

void Kots_CharacterCheckAmmoSteal(edict_t *targ, edict_t *attacker, int damage)
{
    //check for dexterity ammo steal
    if (attacker->character->cur_dexterity >= 5)
    {
        if (targ->client && targ->character->cur_karma < 10)
        {
            int ammoindex, ammosteal;
            gitem_t *ammo = NULL;

            //randomly pick an ammo index
            ammoindex = rand() % AmmoTypeLength;
            ammo = FindItem(ammo_types[ammoindex].ammotype);
            ammoindex = ITEM_INDEX(ammo);

            if (targ->client->pers.inventory[ammoindex] > 0)
            {
                if (attacker->character->cur_dexterity >= 7)
                    ammosteal = (ammo->quantity / 50.0) * (damage * 0.25);
                else
                    ammosteal = (ammo->quantity / 50.0) * (damage * 0.125);

                //perform karma reduction
                if (targ->character->cur_karma >= 7)
                    ammosteal = Kots_RandMultiply(ammosteal, 0.1 * targ->character->cur_karma);
                else if (targ->character->cur_karma > 0)
                    ammosteal = Kots_RandMultiply(ammosteal, 0.05 * targ->character->cur_karma);

                //if no ammo is stolen then that's too bad
                //ensure that at least 1 is stolen
                if (ammosteal > 0)
                {
                    //ensure that we don't steal more than they have
                    if (ammosteal > targ->client->pers.inventory[ammoindex])
                        ammosteal = targ->client->pers.inventory[ammoindex];

                    //steal the ammo
                    targ->client->pers.inventory[ammoindex] -= ammosteal;

                    //monsters don't use it but still steal it
                    if (attacker->client)
                        attacker->client->pers.inventory[ammoindex] += ammosteal;
                }
            }
        }
    }
}

void Kots_CharacterPlayerId(edict_t *ent)
{
    //karma sight is pointless for monsters
    if (!ent->client)
        return;

    if (ent->character->cur_karma < 1)
    {
        ent->character->karma_id_ent = NULL;
        return;
    }

    if (level.time >= ent->character->next_karma_id)
    {
        edict_t *targ = NULL, *who = NULL, *best = NULL;
        float bd = 0, d = 0;
        vec3_t  forward, dir;
        int count = 0;

        AngleVectors(ent->client->v_angle, forward, NULL, NULL);
        while ((targ = findradius(targ, ent->s.origin, 2000)) != NULL)
        {
            if (++count >= MAX_EDICTS)
                break;

            if (!targ->character || !targ->character->is_loggedin || targ->health <= 0 || targ == ent)
                continue;

            who = targ;
            VectorSubtract(who->s.origin, ent->s.origin, dir);
            VectorNormalize(dir);
            d = DotProduct(forward, dir);

            if (d > bd && loc_CanSee(ent, who))
            {
                bd = d;
                best = who;
            }
        }

        if (bd < 0.90)
            best = NULL;

        ent->character->karma_id_ent = best;

        //if we found something to id then don't attempt to id again for a bit
        if (best)
            ent->character->next_karma_id = level.time + 1.0;
    }
}

void Kots_CharacterCreateHealth(edict_t *ent)
{
    if (ent->character->cur_karma < 4)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to create health.\n");
    else
    {
        int cubes;
        gitem_t *item;
        edict_t *health;

        if (ent->character->cur_karma < 7)
            cubes = 25;
        else
            cubes = 20;

        if (ent->character->cubes < cubes)
        {
            gi.cprintf(ent, PRINT_HIGH, "You do not have enough cubes.\n");
            return;
        }

        item = FindItem("Kots Health");
        health = Drop_Item(ent, item);
        health->style = HEALTH_IGNORE_MAX;
        health->count = 50; // Aldarn - kotshealth gives 50!

        ent->character->cubes -= cubes;

        //uncloak the character if cloaked
        Kots_CharacterUncloak(ent);
    }
}

void Kots_CharacterCreateFakeStim(edict_t *ent)
{
    Kots_CharacterDropFakeHealth(ent, "Fake Stim");
}

void Kots_CharacterCreateFakeMega(edict_t *ent)
{
    Kots_CharacterDropFakeHealth(ent, "Fake Mega");
}

void Kots_CharacterToggleSpiritSwim(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->character->using_spiritswim = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->character->using_spiritswim = false;
    else if (!ent->character->using_spiritswim)
        ent->character->using_spiritswim = true;
    else
        ent->character->using_spiritswim = false;

    if (ent->character->using_spiritswim)
        gi.cprintf(ent, PRINT_HIGH, "Spirit swim is enabled.\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "Spirit swim is disabled.\n");
}

void Kots_CharacterCheckSpiritSwim(edict_t *ent, pmove_t *pm)
{
    if (ent->waterlevel && ent->character->using_spiritswim)
    {
        int levelreq;

        if ((ent->watertype & CONTENTS_SLIME))
            levelreq = 7;
        else
            levelreq = 2;


        if (ent->character->cur_spirit >= levelreq)
        {
            gi.Pmove(pm);

            if (ent->waterlevel > 1)
                gi.Pmove(pm);

            //if the water level changes on us increase our velocity
            //unless this is a water jump
            if (ent->waterlevel > 1 && pm->waterlevel <= 1 && !(pm->s.pm_flags & PMF_TIME_WATERJUMP))
            {
                pm->s.velocity[0] *= 5;
                pm->s.velocity[1] *= 5;
                pm->s.velocity[2] *= 5;

                //ensure we don't get too ridiculous jumping out of the water
                if (pm->s.velocity[2] > 5000)
                    pm->s.velocity[2] = 5000;
                if (pm->s.velocity[1] > 3000)
                    pm->s.velocity[1] = 3000;
                if (pm->s.velocity[0] > 3000)
                    pm->s.velocity[0] = 3000;
            }
        }
    }
}

void Kots_CharacterEmpathyOff(edict_t *ent)
{
    if (ent->character->using_empathy)
    {
        ent->character->using_empathy = false;
        gi.cprintf(ent, PRINT_HIGH, "Empathy shield disabled.\n");
    }
}

void Kots_CharacterEmpathyOn(edict_t *ent)
{
    if (ent->character->using_empathy)
        gi.cprintf(ent, PRINT_HIGH, "Empathy shield already active.\n");
    else if (ent->character->cur_spirit < 3)
        gi.cprintf(ent, PRINT_HIGH, "You do not have ability to use empathy.\n");
    else if (ent->character->cubes < 5)
        gi.cprintf(ent, PRINT_HIGH, "You do not have enough cubes.\n");
    else if (ent->character->next_empdone > level.time)
        gi.cprintf(ent, PRINT_HIGH, "You were hit by an EMP Shockwave and this ability is currently disabled!\n"); //hit by emp shockwave can't use empathy
    else
    {
        ent->character->using_empathy = true;
        ent->character->cubes -= 5;
        gi.cprintf(ent, PRINT_HIGH, "Empathy shield is now active.\n");
    }
}

void Kots_CharacterEmpathyToggle(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        Kots_CharacterEmpathyOn(ent);
    else if (Q_stricmp(value, "off") == 0)
        Kots_CharacterEmpathyOff(ent);
    else if (ent->character->using_empathy)
        Kots_CharacterEmpathyOff(ent);
    else
        Kots_CharacterEmpathyOn(ent);
}

void Kots_CharacterCheckEmpathyDamage(edict_t *targ, edict_t *attacker, int damage)
{
    if (targ->character->using_empathy)
    {
        float ratio;
        if (targ->character->cur_spirit >= 10)
            ratio = 0.55;
        else if (targ->character->cur_spirit >= 7)
            ratio = 0.40;
        else if (targ->character->cur_spirit >= 5)
            ratio = 0.27;
        else
            ratio = 0.18;

        damage = Kots_RandMultiply(damage, ratio);
        T_Damage(attacker, targ, targ, vec3_origin, targ->s.origin, NULL, damage, 0, DAMAGE_NO_RESIST | DAMAGE_NO_BONUS | DAMAGE_NO_KNOCKBACK, MOD_EMPATHY);
    }
}

void Kots_CharacterEmpathyUseCubes(edict_t *ent)
{
    if (ent->character->using_empathy)
    {
        if (ent->character->cubes > 0)
            ent->character->cubes -= 1;
        else
            Kots_CharacterEmpathyOff(ent);
    }
}

void Kots_CharacterKotsArmor(edict_t *ent)
{
    if (ent->character->cur_spirit < 4)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to create armor.\n");
    else if (ent->character->cubes < 30)
        gi.cprintf(ent, PRINT_HIGH, "You do not have enough cubes.\n");
    else if (ent->character->kots_armor > 50)
        gi.cprintf(ent, PRINT_HIGH, "You must wait to queue more.\n");
    else
    {
        ent->character->kots_armor += 50;
        ent->character->cubes -= 30;
    }
}

void Kots_CharacterCheckKotsArmor(edict_t *ent)
{
    if (ent->character->kots_armor > 0)
    {
        int maxarmor = Kots_CharacterGetMaxArmor(ent);
        int index = ITEM_INDEX(FindItem("Body Armor"));

        //kots armor can only be depleted when it's actually being used
        //so only use as much kots armor as we actually regen
        if (ent->client->pers.inventory[index] < maxarmor)
        {
            // Aldarn - modded so kots armor regenerates quicker
            ent->client->pers.inventory[index] += 2;
            ent->character->kots_armor -= 2;

            if (ent->client->pers.inventory[index] > maxarmor)
            {
                ent->character->kots_armor += ent->client->pers.inventory[index] - maxarmor;
                ent->client->pers.inventory[index] = maxarmor;
            }
        }
    }
}

void Kots_CharacterConvertCellsToggle(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->character->using_pconvert = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->character->using_pconvert = false;
    else if (!ent->character->using_pconvert)
        ent->character->using_pconvert = true;
    else
        ent->character->using_pconvert = false;

    if (ent->character->using_pconvert)
        gi.cprintf(ent, PRINT_HIGH, "Power cube convert activated.\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "Power cube convert deactivated.\n");
}

void Kots_CharacterCheckConvertCells(edict_t *ent)
{
    if (ent->character->cur_spirit >= 5 && ent->character->using_pconvert)
    {
        int max = Kots_CharacterGetMaxCubes(ent);

        if (ent->character->cubes <= (max - 15))
        {
            int index = ITEM_INDEX(FindItem("Cells"));
            if (ent->client->pers.inventory[index] >= 50)
                Kots_CharacterConvertCells(ent);
        }
    }
}

void Kots_CharacterConvertCells(edict_t *ent)
{
    if (ent->character->cur_spirit < 5)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to convert cells to cubes.\n");
    else if (ent->health > 0)
    {
        int max = Kots_CharacterGetMaxCubes(ent);
        int index = ITEM_INDEX(FindItem("Cells"));
        if (ent->client->pers.inventory[index] < 50)
        {
            gi.cprintf(ent, PRINT_HIGH, "You do not have enough cells.\n");
            return;
        }

        if (ent->character->cubes < max)
        {
            ent->client->pers.inventory[index] -= 50;
            ent->character->cubes += 15;
        }
    }
}


void Kots_CharacterConvertAmmoToggle(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->character->using_cgconvert = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->character->using_cgconvert = false;
    else if (!ent->character->using_cgconvert)
        ent->character->using_cgconvert = true;
    else
        ent->character->using_cgconvert = false;

    if (ent->character->using_cgconvert)
        gi.cprintf(ent, PRINT_HIGH, "Ammo convert activated.\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "Ammo convert deactivated.\n");
}

void Kots_CharacterCheckConvertAmmo(edict_t *ent)
{
    int convertible_ammo_indeces[] =
    {
        ITEM_INDEX(FindItem("Shells")),
        ITEM_INDEX(FindItem("Cells")),
        ITEM_INDEX(FindItem("Grenades")),
        ITEM_INDEX(FindItem("Rockets")),
        ITEM_INDEX(FindItem("Slugs")),
    };

    // eg. 5 grenades need to be turned to 50 bullets -> multiplication by 10
    const int ammo_conversion_factor[] =
    {
        5,  // Shels
        1,  // Cells
        10, // Grenades
        10, // Rockets
        5,  // Slugs
    };

    if (ent->character->cur_chaingun >= 4 && ent->character->respawn_weapon == WEAP_CHAINGUN && ent->character->using_cgconvert)
    {
        int bullet_max = Kots_CharacterGetMaxAmmo(ent, AMMO_BULLETS);
        int bullet_index = ITEM_INDEX(FindItem("Bullets"));

        printf("Ammo amount: %d\n", ent->client->pers.inventory[convertible_ammo_indeces[2]]);

        for (int i = 0; i < sizeof(convertible_ammo_indeces) / sizeof(int); i++)
        {
            // Make sure the amount missing from max is at least the value of one converted ammo -- otherwise we're wasting ammo during the conversion
            if ((bullet_max - ent->client->pers.inventory[bullet_index]) >= ammo_conversion_factor[i])
            {
                // If there's ammo to convert
                if (ent->client->pers.inventory[convertible_ammo_indeces[i]])
                {
                    int bullets_to_max = bullet_max - ent->client->pers.inventory[bullet_index];

                    // If there's enough ammo to cover the entire missing amount
                    if (bullets_to_max <= (ent->client->pers.inventory[convertible_ammo_indeces[i]] * ammo_conversion_factor[i]))
                    {
                        // Intentional integer truncation -- prevents ammo waste
                        ent->client->pers.inventory[convertible_ammo_indeces[i]] -= bullets_to_max / ammo_conversion_factor[i];
                        ent->client->pers.inventory[bullet_index] += (bullets_to_max / ammo_conversion_factor[i]) * ammo_conversion_factor[i];
                        break;
                    }
                    else
                    {
                        ent->client->pers.inventory[bullet_index] += (ent->client->pers.inventory[convertible_ammo_indeces[i]] * ammo_conversion_factor[i]);
                        ent->client->pers.inventory[convertible_ammo_indeces[i]] = 0;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
}

qboolean Kots_CharacterCheckRailDeflect(edict_t *targ, edict_t *attacker, vec3_t dir, int damage)
{
    if (targ->character->cur_spirit >= 7)
    {
        if (targ->client && !targ->client->pers.kots_persist.using_deflect)
            return false;

        if (level.time >= targ->character->next_deflect)
        {
            float chance;

            if (targ->character->cur_spirit >= 10)
                chance = 0.40;
            else
                chance = 0.25;

            if (Kots_RandRound(chance))
            {
                vec3_t deflect_dir;
                VectorCopy(dir, deflect_dir);
                VectorInverse(deflect_dir);
                deflect_dir[0] += (-90 + rand() % 181) * M_PI / 180.0;
                deflect_dir[2] = 0;

                fire_rail2(attacker, targ, targ->s.origin, deflect_dir, damage, 0);
                gi.sound(targ, CHAN_AUTO, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
                targ->character->next_deflect = level.time + 3.0;
                return true;
            }
        }
    }

    return false;
}

void Kots_CharacterToggleDeflect(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->client->pers.kots_persist.using_deflect = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->client->pers.kots_persist.using_deflect = false;
    else if (!ent->client->pers.kots_persist.using_deflect)
        ent->client->pers.kots_persist.using_deflect = true;
    else
        ent->client->pers.kots_persist.using_deflect = false;

    if (ent->client->pers.kots_persist.using_deflect)
        gi.cprintf(ent, PRINT_HIGH, "Rail deflection is enabled.\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "Rail deflection is disabled.\n");

}

void Kots_CharacterSpite(edict_t *ent)
{
    if (ent->character->cur_rage < 1)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to spite.\n");
    else if (ent->character->respawn_weapon == WEAP_SABRE || ent->character->respawn_weapon == WEAP_NONE)
        gi.cprintf(ent, PRINT_HIGH, "You can't use spite with your current respawn weapon.\n");
    else if (ent->character->next_spite >= level.time)
        return; //can't spite yet, wait longer
    else
    {
        gitem_t *weapon = Kots_GetWeaponById(ent->character->respawn_weapon);
        gitem_t *ammo = FindItem(weapon->ammo);
        int weaponindex = ITEM_INDEX(weapon);
        int ammoindex = ITEM_INDEX(ammo);
        int cur = ent->client->pers.inventory[ammoindex];
        int dmg = ((int)ceil(ent->health / 2)) + 10;
        int count;

        if (ent->character->cur_rage >= 7)
            count = ammo->quantity * 4;
        else if (ent->character->cur_rage >= 4)
            count = ammo->quantity * 3;
        else
            count = ammo->quantity * 2;

        //if we already have this much ammo and have the weapon don't do it
        if (count <= cur && (ent->character->cur_rage < 7 || ent->client->pers.inventory[weaponindex]))
        {
            //TODO: Not sure if I want to print a message or not yet
            gi.cprintf(ent, PRINT_HIGH, "You can't spite any more ammo.\n");
            return;
        }

        if (dmg >= ent->health)
        {
            //TODO: Not sure if I want to print a message or not yet
            //      Not even sure I don't want to let them die
            //gi.cprintf(ent, PRINT_HIGH, "You can't spite any more or you'll die.\n");
            return;
        }

        if (ent->character->cur_rage >= 7)
            ent->client->pers.inventory[weaponindex] = 1;

        ent->client->pers.inventory[ammoindex] = count;
        ent->health -= dmg;
        //T_Damage(ent, ent, ent, ent->velocity, ent->s.origin, NULL, dmg, 0, DAMAGE_NO_RESIST | DAMAGE_NO_ARMOR | DAMAGE_NO_KNOCKBACK, MOD_SMITE);
        ent->character->next_spite = level.time + 0.5;

        //play the sound
        gi.sound(ent, CHAN_VOICE, gi.soundindex("*pain25_1.wav"), 1, ATTN_NORM, 0);
        gi.sound(ent, CHAN_BODY, gi.soundindex("misc/am_pkup.wav"), 1, ATTN_NORM, 0);

        //uncloak the character if cloaked
        Kots_CharacterUncloak(ent);
    }
}

void Kots_CharacterCheckQuadRage(edict_t *targ, edict_t *attacker, int dmg)
{
    //TODO: Target not currently used, but may add reductions to quad rage
    //      based on attacker skills

    //TODO: Allow monsters to quad rage
    if (attacker->client && attacker->character->cur_rage > 0
        && level.time >= attacker->character->next_quadrage)
    {
        //ensure that quad rage is enabled for clients
        if (attacker->client->pers.kots_persist.using_quadrage)
        {
            //increment counter
            attacker->character->quadragedamage += dmg;

            //we hit the damage level required to quad rage
            if (attacker->character->quadragedamage >= 800)
            {
                float quadtime = 0.0;
                float chance;

                if (attacker->client->quad_framenum > level.framenum)
                    quadtime = (attacker->client->quad_framenum - level.framenum) * FRAMETIME;

                if (quadtime > 6.0)
                    return;

                if (attacker->character->cur_rage >= 7)
                    chance = 0.06 * attacker->character->cur_rage;
                else
                    chance = 0.04 * attacker->character->cur_rage;

                //only half as likely to quad rage against monsters
                if (!(attacker->svflags & SVF_MONSTER) && (targ->svflags & SVF_MONSTER))
                    chance /= 4.0;

                //reset damage calc
                attacker->character->quadragedamage = 0;

                //check and see if we got it
                if (Kots_RandRound(chance))
                {
                    int quadtime = Kots_CharacterGetQuadRageTime(attacker);
                    if (attacker->client->quad_framenum > level.framenum)
                        attacker->client->quad_framenum += quadtime * 10;
                    else
                        attacker->client->quad_framenum = level.framenum + quadtime * 10;

                    //play the quad sound
                    gi.sound(attacker, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);

                    //don't allow overlapping quad rages
                    attacker->character->next_quadrage = level.time + quadtime;
                }

            }
        }
    }
}

int Kots_CharacterGetQuadRageTime(edict_t *ent)
{
    switch (ent->client->pers.weapon->weapmodel)
    {
    case WEAP_SHOTGUN:
        return 4;
    case WEAP_SUPERSHOTGUN:
        return 3;
    case WEAP_MACHINEGUN:
        return 3;
    case WEAP_CHAINGUN:
        return 2;
    case WEAP_GRENADES:
        return 5;
    case WEAP_GRENADELAUNCHER:
    case WEAP_ROCKETLAUNCHER:
        return 4;
    case WEAP_HYPERBLASTER:
        return 3;
    case WEAP_RAILGUN:
        return 3;
    case WEAP_BFG:
        return 4;
    case WEAP_NONE:
    case WEAP_SABRE:
    default:
        return 5;
    }
}

void Kots_CharacterCheckPoison(edict_t *targ, edict_t *attacker, int dmg)
{
    //target can't be poisoned twice and another person can't steal poison damage
    if (!targ->character->is_poisoned && attacker->character->cur_rage >= 2)
    {
        //ensure that poison is enabled for clients
        if (!attacker->client || attacker->client->pers.kots_persist.using_poison)
        {
            float chance;
            if (attacker->character->cur_rage < 5)
                chance = 0.0005 * dmg;
            else
                chance = 0.001 * dmg;

            //max chance is 30%
            if (chance > 0.30)
                chance = 0.30;

            if (targ->character->cur_dexterity > 0)
                chance -= chance * (targ->character->cur_dexterity * 0.08);

            //check and see if we poisoned them
            if (Kots_RandRound(chance))
            {
                targ->character->poisonedby = attacker;
                targ->character->is_poisoned = true;

                //Spawn some initial green blood to indicate the poison
                SpawnDamage(TE_SPLASH, targ->s.origin, vec3_origin, dmg);
            }
        }
    }
}

void Kots_CharacterCheckPoisonDamage(edict_t *ent)
{
    if (ent->character->is_poisoned)
    {
        //if the character that poisoned us is gone then remove poison
        if (ent->character->poisonedby && (!ent->character->poisonedby->inuse || !ent->character->poisonedby->character || !ent->character->poisonedby->character->is_loggedin))
        {
            ent->character->poisonedby = NULL;
            ent->character->is_poisoned = false;
            return;
        }

        T_Damage(ent, ent->character->poisonedby, ent->character->poisonedby, vec3_origin, *CenterEdict(ent), NULL, 10, 0, DAMAGE_NO_RESIST | DAMAGE_NO_ARMOR | DAMAGE_NO_KNOCKBACK , MOD_POISON);
    }
}

void Kots_CharacterDropMine(edict_t *ent)
{
    if (ent->character->cur_rage < 2)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to create mines.\n");
    else if (ent->character->minesout >= 4)
        gi.cprintf(ent, PRINT_HIGH, "You have already dropped the maximum number of mines.\n");
    else if (ent->character->next_mine > level.time)
        gi.cprintf(ent, PRINT_HIGH, "You must wait %u more seconds before dropping more mines.\n", (int)ceil(ent->character->next_mine - level.time));
    else
    {
        int rocket_index = ITEM_INDEX(FindItem("Rockets"));
        int grenade_index = ITEM_INDEX(FindItem("Grenades"));
        int needed;

        //determine ammo needed
        if (ent->character->cur_rage >= 7)
            needed = 5;
        else
            needed = 10;

        //is there enough ammo?
        if (ent->client->pers.inventory[rocket_index] >= needed &&
            ent->client->pers.inventory[grenade_index] >= needed)
        {
            gitem_t *mine;
            edict_t *drop;

            //randomly pick a mine to drop
            if (rand() % 2)
                mine = FindItem("Rocket Pack Mine");
            else
                mine = FindItem("Grenade Mine");

            //drop the mine
            drop = Drop_Item(ent, mine);

            //determine damage bonus
            if (ent->character->cur_rage >= 7)
                drop->dmg = 300;
            else
                drop->dmg = 200;

            //set a few item properties
            drop->dmg_radius = drop->dmg;
            drop->think = Kots_MineMakeTouchable;

            //remove the ammo
            ent->client->pers.inventory[rocket_index] -= needed;
            ent->client->pers.inventory[grenade_index] -= needed;

            //keep track of when the next mine can be dropped
            ent->character->next_mine = level.time + 5.0;
            ent->character->minesout++;

            //uncloak the character if cloaked
            Kots_CharacterUncloak(ent);
        }
        else
            gi.cprintf(ent, PRINT_HIGH, "Not enough ammo to create a mine.\n");
    }
}

void Kots_CheckArmorKnock(edict_t *targ, edict_t *attacker, int dmg)
{
    //TODO: Doesn't work on monsters yet
    if (attacker->character->cur_rage >= 5 && targ->client)
    {
        //we need to wait longer to knock
        if (attacker->character->next_armorknock > level.time)
            return;

        //add to knock damage
        targ->character->armorknock_damage += dmg;

        //check if knock damage reached
        if (targ->character->armorknock_damage >= 200)
        {
            float chance;

            //reset damage
            targ->character->armorknock_damage = 0;

            //determine chance of knocking
            if (attacker->character->cur_rage >= 10)
                chance = 1.0;
            else if (attacker->character->cur_rage >= 7)
                chance = 0.5;
            else
                chance = 0.25;

            if (targ->character->cur_strength >= 7)
                chance *= 1.0 - (targ->character->cur_strength * 0.06);
            else if (targ->character->cur_strength > 0)
                chance *= 1.0 - (targ->character->cur_strength * 0.05);

            //if we knocked
            if (Kots_RandRound(chance))
            {
                int index = ARMOR_INDEX;

                //determine if the player has armor
                if (targ->client->pers.inventory[index] > 0)
                {
                    gitem_t *shard = FindItem("Knocked Armor Shard");
                    edict_t *drop = Drop_Item(targ, shard);
                    drop->count = targ->client->pers.inventory[index];

                    //drop at most 50 armor
                    if (drop->count > 50)
                        drop->count = 50;

                    //remove the armor from the target
                    targ->client->pers.inventory[index] -= drop->count;

                    //set the next knock timer
                    attacker->character->next_armorknock = level.time + 5;
                }
            }
        }
    }
}

void Kots_CharacterFlail(edict_t *ent)
{
    //are we able to flail?
    if (ent->character->cur_rage < 4)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to flail.\n");
    else if (level.time < ent->character->next_flail) //can we flail yet?
        gi.cprintf(ent, PRINT_HIGH, "You must wait a while before flailing again.\n");
    else if (ent->character->cubes < 20) // not enough cubes
        gi.cprintf(ent, PRINT_HIGH, "You do not have enough cubes to flail.\n");
    else
    {
        vec3_t      dir, angle;
        edict_t     *enemy = NULL;
        qboolean    hit = false;

        //SWB - dex damage reduction like all other damage reductions
        //      should be in Kots_CharacterCheckReductions
        //      and resistances should be in Kots_CharacterCheckResistances
        int         dmg = 100; // Aldarn - dex lowers flail damage
        int         count = 0;

        //SWB - munition raises flail damage
        dmg = Kots_CharacterMunitionDamage(ent, dmg);

        //check for quad damage
        if (ent->client->quad_framenum >= level.framenum)
            dmg *= 4;

        ent->character->next_flail = level.time + 1;
        ent->character->cubes -= 20;

        //find nearby enemies
        while ((enemy = findradius(enemy, ent->s.origin, 100)))
        {
            if (++count >= MAX_EDICTS)
                break;

            //don't hit ourselves, we'll do that later if necessary
            if (enemy == ent)
                continue;

            //must hit either a client or monster
            if (!enemy->client && !(enemy->svflags & SVF_MONSTER))
                continue;

            //we got a hit, now we won't have to hit ourselves later
            hit = true;

            // Aldarn - Knock them flying
            VectorSubtract(enemy->s.origin, ent->s.origin, angle);
            dir[0] = (angle[1] ? atan(angle[0]) : 90 * M_PI / 180);
            dir[1] = (angle[0] ? atan(angle[1]) : 90 * M_PI / 180);
            dir[2] = 30 * M_PI / 180.0; // upwards velocity

            //if enemy is using grapple then break it
            if (enemy->client && enemy->client->ctf_grapple)
                CTFPlayerResetGrapple(enemy);

            // Damage
            T_Damage(enemy, ent, ent, dir, enemy->s.origin, vec3_origin, dmg, 750, 0, MOD_FLAIL);
        }

        //sorry didn't hit anything now you get hit
        if (!hit)
            T_Damage(ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, dmg / 5, 0, 0, MOD_FLAIL);

        //SWB - removed after Dave's whining
        //adds an effect but not necessary
        //ent->s.event = EV_PLAYER_TELEPORT;

        //uncloak the character if cloaked
        Kots_CharacterUncloak(ent);
    }
}

//returns true if maximum number of conflags have been reached
qboolean Kots_CharacterCheckMaxConflags()
{
    int i, count = 0;
    edict_t *ent = g_edicts + 1;

    for (i = 0; i < game.maxclients; i++, ent++)
    {
        if (!ent->inuse || !ent->character->is_loggedin)
            continue;

        //if this character is conflagging then increase the count
        if (ent->character->next_conflag && ent->character->next_conflag >= level.time)
            count++;
    }

    //2 conflags is currently the max so return that we hit the max
    if (count >= 2)
        return true;
    else
        return false;
}

void Kots_CharacterConflagration(edict_t *ent)
{
    if (ent->character->cur_rage < 3)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to use conflagration.\n");
    else if (ent->character->next_conflag && ent->character->next_conflag >= level.time)
        gi.cprintf(ent, PRINT_HIGH, "You are already using conflagration.\n");
    else if (Kots_CharacterCheckMaxConflags())
        gi.cprintf(ent, PRINT_HIGH, "The maximum number of conflagrations are currently active right now. Try again later.\n");
    else
    {
        int rocketIndex = ITEM_INDEX(FindItem("Rockets"));
        int grenadeIndex = ITEM_INDEX(FindItem("Grenades"));
        int rockets = ent->client->pers.inventory[rocketIndex];
        int grenades = ent->client->pers.inventory[grenadeIndex];
        int left = (ent->health / 3) - 10;

        //don't allow it if we don't have any ammo
        if (rockets <= 0 && grenades <= 0)
        {
            gi.cprintf(ent, PRINT_HIGH, "You do not have any rockets or grenades.\n");
            return;
        }

        //require a combined total of at least 20 rockets and grenades
        if ((rockets + grenades) < 20)
        {
            gi.cprintf(ent, PRINT_HIGH, "You must have at least 20 rockets and grenades to use conflagration.\n");
            return;
        }

        //don't allow conflagging yourself to death
        if (left <= 0)
        {
            gi.cprintf(ent, PRINT_HIGH, "You do not have enough health to use conflagration.\n");
            return;
        }

        //remove health and armor
        ent->health = left;
        ent->client->pers.inventory[ARMOR_INDEX] = 0;

        //play the damage sound
        gi.sound(ent, CHAN_AUTO, gi.soundindex("*pain25_1.wav"), 1, ATTN_NORM, 0);

        //start the conflagration timer
        ent->character->next_conflag = level.time;
        Kots_CharacterUncloak(ent);
    }
}

void Kots_CharacterCheckConflagration(edict_t *ent)
{
    if (ent->character->next_conflag && ent->character->next_conflag <= level.time)
    {
        int rocketIndex = ITEM_INDEX(FindItem("Rockets"));
        int grenadeIndex = ITEM_INDEX(FindItem("Grenades"));
        int rockets = ent->client->pers.inventory[rocketIndex];
        int grenades = ent->client->pers.inventory[grenadeIndex];
        vec3_t angles, forward, right, start;
        int damage = 120;
        int i;

        //pick a random angle to start with
        VectorClear(angles);
        angles[YAW] = rand() % 361;

        //add munition damage
        damage = Kots_CharacterMunitionDamage(ent, damage);

        //check for quad damage
        if (ent->client->quad_framenum >= level.framenum)
            damage *= 4;

        //only fire this many times
        //always try to keep it even to fire the same number of rockets and grenades
        for (i = 0; i < 6; i++)
        {
            //create random angle to aim
            angles[YAW] += 1 + rand() % 30;
            angles[PITCH] = -5 + rand() % 11; //aim +/-5 degrees

            AngleVectors (angles, forward, right, NULL);
            VectorCopy(ent->s.origin, start);
            start[0] += 8;
            start[1] += 8;
            start[2] += ent->viewheight - 8;
            //VectorSet(offset, 8, 8, ent->viewheight - 8);
            //P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

            //alternate between rockets and grenades if we have both
            if ((rockets > 0 && i % 2) || (grenades <= 0 && rockets > 0))
            {
                --rockets;
                fire_rocket_ex(ent, start, forward, damage, 800, 120.0, damage, false, false);
            }
            else if (grenades > 0)
            {
                --grenades;
                fire_grenade_ex(ent, start, forward, damage, 800, 1.0, 120.0, damage, false, false);
            }
            else
                break;
        }

        //update the ammo count in clients inventory
        ent->client->pers.inventory[rocketIndex] = rockets;
        ent->client->pers.inventory[grenadeIndex] = grenades;

        //uncloak every time we fire with conflag
        Kots_CharacterUncloak(ent);

        //determine the next firing frame if we have more ammo
        if (rockets <= 0 && grenades <= 0)
            ent->character->next_conflag = 0;
        else
            ent->character->next_conflag = level.time + 0.2;
    }
}

void Kots_VerifyHeadShot (vec3_t point, vec3_t dir, float height, vec3_t newpoint)
{
    vec3_t normdir;
    vec3_t normdir2;

    //if length is 0 then vectornormalize won't set the values so we should clear it first
    VectorClear(normdir);

    VectorNormalize2(dir, normdir);
    VectorScale( normdir, height, normdir2 );
    VectorAdd( point, normdir2, newpoint );
}

#define HEAD_HEIGHT 16

qboolean Kots_IsHeadShot (edict_t *attacker, edict_t *targ, vec3_t dir, vec3_t point, int damage)
{
    float       height, lower_bound, upper_bound; //height will be distance from the top of BBOX
    float       z_rel;
    float       from_top;
    qboolean    headshot = false;

    if (damage < 1)
        return false;

    //no headshots at these points
    if (point == NULL || VectorCompare(point, targ->s.origin) || VectorCompare(point, vec3_origin))
        return false;

    height = 7;
    z_rel = point[2] - targ->s.origin[2];
    from_top = targ->maxs[2] - z_rel;

    if (from_top < 2 * HEAD_HEIGHT)
    {
        vec3_t new_point;

        Kots_VerifyHeadShot( point, dir, HEAD_HEIGHT, new_point );

        VectorSubtract( new_point, targ->s.origin, new_point );
        if (targ->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
            upper_bound = targ->maxs[2];
            lower_bound = targ->maxs[2] - 7;
        }
        else
        {
            upper_bound = targ->maxs[2] - height;
            lower_bound = targ->maxs[2] - (height +7);
        }

        if((upper_bound - new_point[2]) < 0)
            return false;

        if((lower_bound - new_point[2]) > 0)
            return false;

        if ((upper_bound - new_point[2]) < HEAD_HEIGHT && (abs(new_point[1])) < HEAD_HEIGHT*.5 && (abs(new_point[0])) < HEAD_HEIGHT*.5)
            headshot = true;
    }

    if (headshot)
    {
        if (level.time >= attacker->character->next_headshot)
        {
            attacker->character->next_headshot = level.time + FRAMETIME;
            gi.cprintf(attacker,PRINT_HIGH,"You hit %s in the head!\n", targ->character->name);
            gi.cprintf(targ,PRINT_HIGH,"Head Damage!\n");
            gi.sound(targ, CHAN_WEAPON, gi.soundindex("makron/brain1.wav"), 1, ATTN_NORM, 0);
        }

        return true;
    }

    return false;
}

void Kots_CharacterCheckDizzy(edict_t *ent, pmove_t *pm)
{
    if (ent->character->next_dizzydone > level.time && ent->client)
    {
        ent->client->kick_angles[0] = crandom() * 40;
        ent->client->kick_angles[1] = crandom() * 40;
        ent->client->kick_angles[2] = crandom() * 40;
    }
}

void Kots_CharacterCheckStun(edict_t *ent, pmove_t *pm)
{
    if (ent->character->next_stundone > level.time)
    {
        ent->velocity[0] = 0;
        ent->velocity[1] = 0;

        if (pm)
        {
            pm->s.velocity[0] = 0;
            pm->s.velocity[1] = 0;

            pm->cmd.sidemove = 0;
            pm->cmd.forwardmove = 0;
            pm->cmd.upmove = 0;
        }
    }
}
void Kots_CharacterTogglePoison(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->client->pers.kots_persist.using_poison = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->client->pers.kots_persist.using_poison = false;
    else if (!ent->client->pers.kots_persist.using_poison)
        ent->client->pers.kots_persist.using_poison = true;
    else
        ent->client->pers.kots_persist.using_poison = false;

    if (ent->client->pers.kots_persist.using_poison)
        gi.cprintf(ent, PRINT_HIGH, "Poison is enabled.\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "Poison is disabled.\n");
}

void Kots_CharacterToggleQuadRage(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->client->pers.kots_persist.using_quadrage = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->client->pers.kots_persist.using_quadrage = false;
    else if (!ent->client->pers.kots_persist.using_quadrage)
        ent->client->pers.kots_persist.using_quadrage = true;
    else
        ent->client->pers.kots_persist.using_quadrage = false;

    if (ent->client->pers.kots_persist.using_quadrage)
        gi.cprintf(ent, PRINT_HIGH, "Quad rage is enabled.\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "Quad rage is disabled.\n");
}
void Kots_CharacterToggleLaser(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->client->pers.kots_persist.using_lasersight = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->client->pers.kots_persist.using_lasersight = false;
    else if (!ent->client->pers.kots_persist.using_lasersight)
        ent->client->pers.kots_persist.using_lasersight = true;
    else
        ent->client->pers.kots_persist.using_lasersight = false;

    if (ent->client->pers.kots_persist.using_lasersight)
    {
        if (ent->character->cur_technical < 7)
        {
            ent->client->pers.kots_persist.using_lasersight = false;
            gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to user laser sight.\n");
            return;
        }

        gi.cprintf(ent, PRINT_HIGH, "Laser sight is enabled.\n");
        ent->client->pers.kots_persist.using_laserball = false;
        Kots_CharacterCreateLaser(ent);
    }
    else
        gi.cprintf(ent, PRINT_HIGH, "Laser sight is disabled.\n");
}

void Kots_CharacterToggleLaserBall(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "on") == 0)
        ent->client->pers.kots_persist.using_laserball = true;
    else if (Q_stricmp(value, "off") == 0)
        ent->client->pers.kots_persist.using_laserball = false;
    else if (!ent->client->pers.kots_persist.using_laserball)
        ent->client->pers.kots_persist.using_laserball = true;
    else
        ent->client->pers.kots_persist.using_laserball = false;

    if (ent->client->pers.kots_persist.using_laserball)
    {
        if (ent->character->cur_technical < 7)
        {
            ent->client->pers.kots_persist.using_laserball = false;
            gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to user laser sight ball.\n");
            return;
        }

        gi.cprintf(ent, PRINT_HIGH, "Laser sight ball is enabled.\n");
        ent->client->pers.kots_persist.using_lasersight = false;
        Kots_CharacterCreateLaserBall(ent);
    }
    else
        gi.cprintf(ent, PRINT_HIGH, "Laser sight ball is disabled.\n");
}

void Kots_CharacterCreateLaser(edict_t *ent)
{
    if (ent->character->lasersight)
        G_FreeEdict(ent->character->lasersight);

    ent->character->lasersight = CreateLaser(ent, vec3_origin, vec3_origin, 0, 0xf2f2f0f0, 2, 2, FRAMETIME);
    VectorSet(ent->character->lasersight->mins, -1, -1, -1);
    VectorSet(ent->character->lasersight->maxs, 1, 1, 1);
    ent->character->lasersight->prethink = NULL;
    ent->character->lasersight->think = Kots_CharacterLaserThink;
    ent->character->lasersight->nextthink = level.time + 0.1;
    ent->character->lasersight->think(ent->character->lasersight);
}

void Kots_CharacterCreateLaserBall(edict_t *ent)
{
    if (ent->character->lasersight)
        G_FreeEdict(ent->character->lasersight);

    ent->character->lasersight = G_Spawn();
    VectorSet(ent->character->lasersight->mins, -6, -6, -6);
    VectorSet(ent->character->lasersight->maxs, 6, 6, 6);
    ent->character->lasersight->owner = ent;
    ent->character->lasersight->movetype = MOVETYPE_NOCLIP;
    ent->character->lasersight->solid = SOLID_NOT;
    ent->character->lasersight->classname = "laserball";
    ent->character->lasersight->s.modelindex = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
    ent->character->lasersight->s.renderfx = RF_SHELL_RED;
    ent->character->lasersight->think = Kots_CharacterLaserThink;
    ent->character->lasersight->nextthink = level.time + 0.1;
    ent->character->lasersight->think(ent->character->lasersight);
}

void Kots_CharacterLaserThink(edict_t *self)
{
    if (!self->owner || !self->owner->character || !self->owner->character->is_loggedin
        || (!self->owner->client->pers.kots_persist.using_lasersight && !self->owner->client->pers.kots_persist.using_laserball)
        || level.intermissiontime)
    {
        if (self->owner)
            self->owner->character->lasersight = NULL;

        G_FreeEdict(self);
        return;
    }
    else if (self->owner->health <= 0)
        self->svflags |= SVF_NOCLIENT;
    else
    {
        trace_t tr;
        vec3_t forward, right, offset;
        vec3_t start, end;

        //determine the point to trace to
        AngleVectors(self->owner->client->v_angle, forward, right, NULL);
        VectorSet(offset, 8, 6, self->owner->viewheight - 7);
        P_ProjectSource(self->owner->client, self->owner->s.origin, offset, forward, right, start);
        VectorMA(start, 8192, forward, end);

        tr = gi.trace(start, self->mins, self->maxs, end, self->owner, MASK_SHOT);

        if (self->s.renderfx & RF_BEAM)
        {
            VectorCopy(start, self->s.origin);
            VectorCopy(tr.endpos, self->s.old_origin);
            VectorCreateVelocity(start, tr.endpos, self->velocity);
        }
        else
        {
            VectorCopy(tr.endpos, self->s.origin);
            VectorCopy(tr.endpos, self->s.old_origin);
        }

        //make sure it's visible
        self->svflags &= ~SVF_NOCLIENT;

        //relink the laser
        gi.linkentity(self);
    }

    //set the next think time
    self->nextthink = level.time + 0.1;
}
void Kots_CharacterHaste(edict_t *ent)
{
    if (ent->character->cur_dexterity < 7)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to use haste.\n");
    else if (ent->character->cubes < 50)
        gi.cprintf(ent, PRINT_HIGH, "You must have 50 cubes to activate haste.\n");
    else if (ent->character->next_haste > level.time)
        gi.cprintf(ent, PRINT_HIGH, "You must wait a while before activating haste again.\n");
    else
    {
        ent->character->cubes -= 50;
        ent->character->next_haste = level.time + 30.0;
        ent->character->next_hastecomplete = level.time + 15.0;

        gi.sound(ent, CHAN_VOICE, gi.soundindex("world/airhiss1.wav"), 1, ATTN_NORM, 0);

        //uncloak the character if cloaked
        Kots_CharacterUncloak(ent);
    }
}
void Kots_CharacterCheckHaste(edict_t *ent, pmove_t *pm)
{
    if (ent->character->next_hastecomplete > level.time)
    {
        //no speed boost in water
        if (!pm->waterlevel)
        {
            //increase movement speed while active
            gi.Pmove(pm);
        }
    }
}

void weapon_grenade_fire(edict_t *ent, qboolean held);
void Kots_CharacterDetonate(edict_t *ent)
{
    if (ent->character->cur_grenade < 4)
        gi.cprintf(ent, PRINT_HIGH, "You don't have the ability to detonate grenades.\n");
    else if (ent->health <= 0)
        gi.cprintf(ent, PRINT_HIGH, "You can't detonate your grenade while dead.\n");
    else if (!ent->client->pers.weapon || ent->client->pers.weapon->weapmodel != WEAP_GRENADES)
        gi.cprintf(ent, PRINT_HIGH, "You are not currently using grenades.\n");
    else if (ent->client->pers.inventory[ent->client->ammo_index] <= 0)
        gi.cprintf(ent, PRINT_HIGH, "You do not have any grenades.\n");
    else if (ent->client->weaponstate != WEAPON_FIRING || level.time > ent->client->grenade_time || ent->client->ps.gunframe == 15)
        gi.cprintf(ent, PRINT_HIGH, "Your grenade is not ready to be detonated.\n");
    else
    {
        vec3_t oldvelocity;
        int damage = Kots_CharacterMunitionDamagePlus(ent, 120, ent->character->cur_grenade);
        int hits = ent->character->hits;

        //keep track of old velocity, we don't want to stop the player
        VectorCopy(ent->velocity, oldvelocity);

        //uncloak and blow up the current grenade in hand
        Kots_CharacterUncloak(ent);

        //add quad damage
        if (ent->client->quad_framenum > level.framenum)
            damage *= 4;

        //do radius damage on opponents and damage self
        T_RadiusDamage(ent, ent, damage, ent, 160, MOD_HG_SPLASH);

        //reset number of hits since these shouldn't count
        ent->character->hits = hits;

        //don't count quad against yourself
        if (ent->client->quad_framenum > level.framenum)
            damage /= 4;

        //do a quarter of the damage to yourself
        damage /= 4;

        //never do more than your own max health to yourself
        if (damage > ent->max_health)
            damage = ent->max_health - 1;

        T_Damage(ent, ent, ent, vec3_origin, vec3_origin, vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK, MOD_HG_SPLASH);

        //create explosion
        gi.WriteByte(svc_temp_entity);
        if (ent->waterlevel)
        {
            if (ent->groundentity)
                gi.WriteByte(TE_GRENADE_EXPLOSION_WATER);
            else
                gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
        }
        else
        {
            if (ent->groundentity)
                gi.WriteByte(TE_GRENADE_EXPLOSION);
            else
                gi.WriteByte(TE_ROCKET_EXPLOSION);
        }
        gi.WritePosition(ent->s.origin);
        gi.multicast(ent->s.origin, MULTICAST_PHS);

        //reset the grenade to it's previous state
        ent->client->weaponstate = WEAPON_FIRING;
        ent->client->ps.gunframe = 15;
        ent->client->weapon_sound = 0;
        ent->client->grenade_time = level.time + 1.0;

        //restore old velocity and don't make us take falling damage
        VectorCopy(oldvelocity, ent->velocity);
        VectorCopy(oldvelocity, ent->client->oldvelocity);
    }
}


void Kots_CharacterHookColor(edict_t *ent, char *value)
{
    if (Q_stricmp(value, "red") == 0)
        ent->character->laserhook_color = LASERHOOK_COLOR_RED;
    else if (Q_stricmp(value, "blue") == 0)
        ent->character->laserhook_color = LASERHOOK_COLOR_BLUE;
    else if (Q_stricmp(value, "green") == 0)
        ent->character->laserhook_color = LASERHOOK_COLOR_GREEN;
    else if (Q_stricmp(value, "yellow") == 0)
        ent->character->laserhook_color = LASERHOOK_COLOR_YELLOW;
    else if (Q_stricmp(value, "orange") == 0)
        ent->character->laserhook_color = LASERHOOK_COLOR_ORANGE;
    else
        gi.cprintf(ent, PRINT_HIGH, "Invalid color.\n");
        return;

    gi.cprintf(ent, PRINT_HIGH, "Color changed.\n");
}

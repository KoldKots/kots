#include "kots_pickups.h"
#include "kots_utils.h"

#define HEALTH_IGNORE_MAX   1
#define HEALTH_TIMED        2
void MegaHealth_think (edict_t *self);

int Kots_CharacterGetMaxAmmoBonus(edict_t *ent)
{
    switch (ent->character->cur_strength)
    {
        case 0:
            return 1;
        case 1:
        case 2:
            return 2;
        case 3:
        case 4:
            return 3;
        case 5:
        case 6:
            return 4;
        case 7:
        default:
            return 5;
    }
}

int Kots_CharacterGetMaxAmmo(edict_t *ent, int ammotype)
{
    int basemax;

    switch (ammotype)
    {
    case AMMO_BULLETS:
        basemax = ent->client->pers.max_bullets;
        break;
    case AMMO_SHELLS:
        basemax = ent->client->pers.max_shells;
        break;
    case AMMO_ROCKETS:
        basemax = ent->client->pers.max_rockets;
        break;
    case AMMO_GRENADES:
        basemax = ent->client->pers.max_grenades;
        break;
    case AMMO_CELLS:
        basemax = ent->client->pers.max_cells;
        break;
    case AMMO_SLUGS:
        basemax = ent->client->pers.max_slugs;
        break;
    default:
        basemax = 0;
        break;
    }

    return basemax * Kots_CharacterGetMaxAmmoBonus(ent);
}

int Kots_CharacterGetAmmoPickup(edict_t *ent, int baseammo)
{
    switch (ent->character->cur_dexterity)
    {
    case 0:
        return baseammo;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        return baseammo + Kots_RandMultiply(baseammo, ent->character->cur_dexterity * 0.10);
    case 7:
    case 8:
    case 9:
        return baseammo * 2;
    case 10:
    default:
        return baseammo * 3;
    }
}


void Kots_CharacterPickupAmmo(edict_t *ent, gitem_t *ammo, int amount)
{
    int max = Kots_CharacterGetMaxAmmo(ent, ammo->tag);
    int index = ITEM_INDEX(ammo);

    if (ent->client->pers.inventory[index] < max)
    {
        ent->client->pers.inventory[index] += amount;

        if (ent->client->pers.inventory[index] > max)
            ent->client->pers.inventory[index] = max;
    }
}

qboolean Kots_CharacterPickupArmor(edict_t *ent, edict_t *other)
{
    int pickup;
    int index = ARMOR_INDEX;
    int max = Kots_CharacterGetMaxArmor(other);
    gitem_armor_t *info = (gitem_armor_t *)ent->item->info;
    int spirit_cube_bonus = other->character->cur_spirit;

    // handle armor shards specially
    if (ent->item->tag == ARMOR_SHARD)
    {
        pickup = 2;

        // Aldarn - cube bonuses
        if(other->character->cur_spirit >= 7) // 7+
            spirit_cube_bonus += 2; // 2 extra at level 7 (not sure why)

        other->character->cubes += 5 + spirit_cube_bonus;
    }
    else if (ent->count)
        pickup = ent->count;
    else if (ent->item->quantity)
        pickup = ent->item->quantity;
    else
        pickup = info->base_count;

    //dropped and knocked shards don't give bonus
    if (!(ent->item->tag & KOTS_ITEM_SHARD))
    {
        //add tech bonuses
        if (other->character->cur_technical < 7)
            pickup += other->character->cur_technical;
        else
            pickup += other->character->cur_technical + 1;
    }

    //add the pickup and ignore max if a shard
    if (ent->item->tag == ARMOR_SHARD || (ent->item->tag & KOTS_ITEM_SHARD))
    {
        max *= 2; //max armor for shards is 2x max armor

        if (other->client->pers.inventory[index] < max)
        {
            other->client->pers.inventory[index] += pickup;

            if (other->client->pers.inventory[index] > max)
                other->client->pers.inventory[index] = max;
        }
    }
    else //if it wasn't an armor shard limit to the max armor
    {
        if (other->client->pers.inventory[index] >= max)
            return false;

        other->client->pers.inventory[index] += pickup;
        if (other->client->pers.inventory[index] > max)
            other->client->pers.inventory[index] = max;
    }

    //start the armor respawning
    if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
    {
        if (Q_stricmp(ent->item->pickup_name, "Power Shield") == 0 || Q_stricmp(ent->item->pickup_name, "Power Screen") == 0)
            SetRespawn (ent, 120 + (rand() % 61));
        else
            SetRespawn (ent, 20);
    }

    return true;
}

qboolean Kots_CharacterPickupHealth(edict_t *ent, edict_t *other)
{
    qboolean cure_poison = false;
    int realmax = Kots_CharacterGetMaxRegenHealth(other);
    int count = ent->count;

    if (other->character->is_poisoned)
    {
        //all health cures poison except for stimpacks, they only cure when level 6 or higher karma
        if (other->character->cur_karma >= 6 || Q_stricmp(ent->item->pickup_name, "Kots Stimpack") != 0)
            cure_poison = true;
    }

    if (other->health >= realmax && !cure_poison)
        return false;

    if (!(ent->style & HEALTH_IGNORE_MAX))
    {
        if (other->health >= other->max_health)
            count = 0; //only pickup the bonus health above max
        else if ((other->health + count) > other->max_health)
            count = other->max_health - other->health;
    }

    //karma 2 is required to pickup bonus health above max
    if (count == 0 && other->character->cur_karma < 2 && !cure_poison)
        return false;
    else if (other->character->cur_karma >= 7)
        count += other->character->cur_karma + 3;
    else
        count += other->character->cur_karma;

    if (other->health < realmax)
    {
        other->health += count;

        //health can never go over max regen
        if (other->health > realmax)
            other->health = realmax;
    }

    //dead player stims give 2 cubes each
    if (ent->item->tag & KOTS_ITEM_LIFTABLE && ent->style & HEALTH_IGNORE_MAX)
    {
        if (Q_stricmp(ent->item->pickup_name, "Kots Stimpack") == 0)
            other->character->cubes += 2;
    }

    //cure poison
    //ensure that we either have 6 or more karma or that it's not a dead player stim
    if (cure_poison)
    {
        other->character->is_poisoned = false;
        other->character->poisonedby = NULL;
    }

    if (ent->style & HEALTH_TIMED)
    {
        ent->think = MegaHealth_think;
        ent->nextthink = level.time + 5;
        ent->owner = other;
        ent->flags |= FL_RESPAWN;
        ent->svflags |= SVF_NOCLIENT;
        ent->solid = SOLID_NOT;
    }
    else
    {
        if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
            SetRespawn (ent, 30);
    }

    return true;
}

void Drop_Ammo(edict_t *ent, gitem_t *item);
void Kots_CharacterCheckAmmoRot(edict_t *ent)
{
    static char *ammo_types[] =
        {"Shells", "Bullets", "Grenades", "Rockets", "Cells", "Slugs"};

    if (ent->client)
    {
        //we'll start with this ammo type and work our way
        //around until we find something over the max
        int start = rand() % 6;
        int i = start;
        int max, index;
        gitem_t *item;

        do
        {
            item = FindItem(ammo_types[i]);
            max = Kots_CharacterGetMaxAmmo(ent, item->tag);
            index = ITEM_INDEX(item);

            if (ent->client->pers.inventory[index] > max)
            {
                Drop_Ammo(ent, item);
                break; //we'll drop more later (maybe)
            }

            //increment i or wrap back around to the start
            if (++i >= 6)
                i = 0;

        } while (i != start);
    }
}

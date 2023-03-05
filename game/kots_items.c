#include "kots_items.h"
#include "kots_pickups.h"
#include "kots_utils.h"
#include "kots_abilities.h"

#define HEALTH_IGNORE_MAX   1
#define HEALTH_TIMED        2

void Kots_GiveWeapon(edict_t *ent, int index, qboolean give)
{
    if (give && !ent->client->pers.inventory[index])
        ent->client->pers.inventory[index] = 1;
}

qboolean Kots_PackPickup(edict_t *pack, edict_t *ent)
{
    float ammopickup;

    if (!ent->client)
        return false;

    if (ent->character->cur_dexterity >= 7)
        ammopickup = 2;
    else
        ammopickup = 0.5;

    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Shotgun")), pack->pack->shotgun);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Super Shotgun")), pack->pack->supershotgun);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Machinegun")), pack->pack->machinegun);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Chaingun")), pack->pack->chaingun);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Grenade Launcher")), pack->pack->grenadelauncher);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Rocket Launcher")), pack->pack->rocketlauncher);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Hyperblaster")), pack->pack->hyperblaster);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("Railgun")), pack->pack->railgun);
    Kots_GiveWeapon(ent, ITEM_INDEX(FindItem("BFG10K")), pack->pack->bfg);
    Kots_CharacterPickupAmmo(ent, FindItem("Shells"), (int)(pack->pack->shells * ammopickup));
    Kots_CharacterPickupAmmo(ent, FindItem("Bullets"), (int)(pack->pack->bullets * ammopickup));
    Kots_CharacterPickupAmmo(ent, FindItem("Grenades"), (int)(pack->pack->grenades * ammopickup));
    Kots_CharacterPickupAmmo(ent, FindItem("Cells"), (int)(pack->pack->cells * ammopickup));
    Kots_CharacterPickupAmmo(ent, FindItem("Slugs"), (int)(pack->pack->slugs * ammopickup));
    Kots_CharacterPickupAmmo(ent, FindItem("Rockets"), (int)(pack->pack->rockets * ammopickup));

    if (ent->character->cur_karma > pack->pack->cubes || ent->character->cur_karma >= 7)
        Kots_CharacterPickupCubes(ent, pack->pack->cubes);
    else if (pack->pack->cubes > 0)
        Kots_CharacterPickupCubes(ent, ent->character->cur_karma);

    if (Q_stricmp(pack->pack->owner_name, ent->character->name) == 0)
    {
        ent->character->credits += 1;
        ent->character->total_credits += 1;
        ent->character->level_credits += 1;
        gi.cprintf(ent, PRINT_HIGH, "You picked up your own pack containing 10 credits.\n");
    }
    else
    {
        ent->character->credits += pack->pack->credits;
        ent->character->total_credits += pack->pack->credits;
        ent->character->level_credits += pack->pack->credits;
        gi.cprintf(ent, PRINT_HIGH, "You picked up %s's pack containing %i credit%s.\n", pack->pack->owner_name, pack->pack->credits, (pack->pack->credits != 1 ? "s" : ""));
    }

    //keep track of total number of packs picked up
    ent->character->total_packs++;
    ent->character->level_packs++;

    return true;
}

void Kots_PackTeleport(edict_t *ent)
{
    vec3_t forward;
    vec3_t end;
    trace_t trace;

    //don't teleport if the enemy no longer exists or is not logged in
    if (ent->enemy && ent->enemy->character && ent->enemy->character->is_loggedin && ent->enemy->health > 0)
    {
        AngleVectors (ent->enemy->s.angles, forward, NULL, NULL);
        forward[2] = 0; //don't move it up or down
        VectorMA(ent->enemy->s.origin, 50, forward, end);
        trace = gi.trace(ent->enemy->s.origin, ent->mins, ent->maxs, end, ent->enemy, MASK_SOLID);
        VectorCopy(trace.endpos, ent->s.origin);
        ent->velocity[2] = 300; //make it throw up a little
        ent->s.event = EV_OTHER_TELEPORT;
        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Hyprbf1a.wav"), 1, ATTN_NORM, 0);
        gi.linkentity(ent);
    }

    ent->touch = Touch_Item;
    ent->think = NULL;
}

void Kots_PackMakeTouchable(edict_t *ent)
{
    ent->touch = Touch_Item;
    ent->nextthink = level.time + 60.0;
    ent->think = G_FreeEdict;
}

edict_t *Kots_PackDrop(edict_t *targ, edict_t *attacker)
{
    gitem_t *item = FindItem("Kots Pack");
    edict_t *pack = Drop_Item(targ, item);
    pack->enemy = attacker;
    pack->s.effects |= EF_GIB;

    pack->pack = gi.TagMalloc(sizeof(pack_t), TAG_LEVEL);
    memset(pack->pack, 0, sizeof(pack_t));

    //store the name so we have it to print out the message
    Kots_strncpy(pack->pack->owner_name, Kots_CharacterGetFullName(targ), 100);

    if (targ->client)
    {
        float ammo = (targ->character->cur_dexterity >= 7 ? 0.5 : 1.0);
        pack->pack->shotgun = targ->client->pers.inventory[ITEM_INDEX(FindItem("Shotgun"))];
        pack->pack->supershotgun = targ->client->pers.inventory[ITEM_INDEX(FindItem("Super Shotgun"))];
        pack->pack->machinegun = targ->client->pers.inventory[ITEM_INDEX(FindItem("Machinegun"))];
        pack->pack->chaingun = targ->client->pers.inventory[ITEM_INDEX(FindItem("Chaingun"))];
        pack->pack->grenadelauncher = targ->client->pers.inventory[ITEM_INDEX(FindItem("Grenade Launcher"))];
        pack->pack->rocketlauncher = targ->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))];
        pack->pack->hyperblaster = targ->client->pers.inventory[ITEM_INDEX(FindItem("Hyperblaster"))];
        pack->pack->railgun = targ->client->pers.inventory[ITEM_INDEX(FindItem("Railgun"))];
        pack->pack->bfg = targ->client->pers.inventory[ITEM_INDEX(FindItem("BFG10K"))];
        pack->pack->shells = (int)(targ->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] * ammo);
        pack->pack->bullets = (int)(targ->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] * ammo);
        pack->pack->grenades = (int)(targ->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] * ammo);
        pack->pack->cells = (int)(targ->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] * ammo);
        pack->pack->slugs = (int)(targ->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] * ammo);
        pack->pack->rockets = (int)(targ->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] * ammo);

        //maximum cube/credit limit
        pack->pack->credits = (targ->character->level / 2) + 1;
        if (pack->pack->credits > 20)
            pack->pack->credits = 20;

    }
    else
    {
        //monsters give fewer credits
        pack->pack->credits = (targ->character->level / 6) + 1;
        if (pack->pack->credits > 2)
            pack->pack->credits = 2;

        pack->pack->shells = (Kots_RandRound(0.50) ? 10 : 0);
        pack->pack->bullets = (Kots_RandRound(0.50) ? 50 : 0);
        pack->pack->grenades = (Kots_RandRound(0.50) ? 5 : 0);
        pack->pack->cells = (Kots_RandRound(0.50) ? 50 : 0);
        pack->pack->slugs = (Kots_RandRound(0.50) ? 10 : 0);
        pack->pack->rockets = (Kots_RandRound(0.50) ? 5 : 0);
    }

    //always have the same number of cubes and credits
    pack->pack->cubes = pack->pack->credits;

    if (targ != attacker && attacker->character && attacker->character->cur_spirit >= 7)
        pack->think = Kots_PackTeleport;
    else
        pack->think = Kots_PackMakeTouchable;

    return pack;
}


qboolean Kots_FakeHealthPickup(edict_t *item, edict_t *ent)
{
    if (!ent->client && !(ent->svflags & SVF_MONSTER))
        return false;

    //damage the ent that picked it up
    T_Damage(ent, item, item->owner, item->velocity, item->s.origin, NULL, item->dmg, 0, 0, MOD_FAKEHEALTH);
    return true;
}

void Kots_FakeHealthMakeTouchable(edict_t *ent)
{
    ent->touch = Touch_Item;
    ent->nextthink = level.time + 120.0;
    ent->think = G_FreeEdict;
}

void Kots_CharacterDropFakeHealth(edict_t *ent, char *pickup_name)
{
    if (ent->character->cur_karma < 7)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to create fake healths.\n");
    else if (ent->character->cubes < 50)
        gi.cprintf(ent, PRINT_HIGH, "You do not have enough cubes.\n");
    else if (ent->character->fakehealths >= 4)
        gi.cprintf(ent, PRINT_HIGH, "You already have the maximum fake healths out.\n");
    else if (ent->character->next_fakehealth > level.time)
        gi.cprintf(ent, PRINT_HIGH, "You must wait a while before dropping more fake healths.\n");
    else
    {
        gitem_t *item = FindItem(pickup_name);
        edict_t *health = Drop_Item(ent, item);
        health->dmg = 200;
        health->think = Kots_FakeHealthMakeTouchable;

        ent->character->cubes -= 50;
        ++ent->character->fakehealths;
        ent->character->next_fakehealth = level.time + 1;

        //uncloak the character if cloaked
        Kots_CharacterUncloak(ent);
    }
}

qboolean Kots_MinePickup(edict_t *item, edict_t *ent)
{
    vec3_t      origin;

    if (!ent->client && !(ent->svflags & SVF_MONSTER))
        return false;

    //damage the ent that picked it up
    T_Damage(ent, item, item->owner, item->velocity, item->s.origin, NULL, item->dmg, 0, 0, MOD_MINE);

    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
    gi.WriteByte (svc_temp_entity);
    if (ent->waterlevel)
        gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
    else
        gi.WriteByte (TE_ROCKET_EXPLOSION);
    gi.WritePosition (origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);
    return true;
}

void Kots_MineExplode(edict_t *ent)
{
    vec3_t      origin;

    T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_MINE);

    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
    gi.WriteByte (svc_temp_entity);
    if (ent->waterlevel)
        gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
    else
        gi.WriteByte (TE_ROCKET_EXPLOSION);
    gi.WritePosition (origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);

    G_FreeEdict (ent);
}

void Kots_MineMakeTouchable(edict_t *ent)
{
    ent->touch = Touch_Item;
    ent->nextthink = level.time + 30.0;
    ent->think = Kots_MineExplode;
}

void Kots_CharacterDropStims(edict_t *ent)
{
    //there's a 25% chance we'll drop some stims
    if (Kots_RandRound(0.25))
    {
        int i;
        float old_angle;
        float *angle = NULL;
        gitem_t *item = FindItem("Kots Stimpack");
        edict_t *stim;

        if (ent->client)
        {
            old_angle = ent->client->v_angle[1];
            angle = &ent->client->v_angle[1];
        }
        else
        {
            old_angle = ent->s.angles[1];
            angle = &ent->s.angles[1];
        }

        //drop stim packs in a random direction
        for (i = 0; i < 3; i++)
        {
            *angle = rand() % 361;
            stim = Drop_Item(ent, item);
            stim->count = 2;
            stim->style = HEALTH_IGNORE_MAX;
        }

        //reset the old angle
        *angle = old_angle;
    }
}


void Kots_CharacterDropShards(edict_t *ent)
{
    int armor = 6; //default for monsters

    //if we died with armor drop some shards
    if (ent->client)
    {
        armor = ent->client->pers.inventory[ARMOR_INDEX];

        if (armor < 10)
            return;
    }
    else
    {
        //monsters have a 25% chance of dropping shards
        if (!Kots_RandRound(0.25))
            return;
    }

    if (armor > 0)
    {
        float old_angle;
        float *angle = NULL;
        edict_t *shard;
        gitem_t *item = FindItem("Kots Shard");
        int shards, quantity, i;

        //determine the number of shards to drop (25 max)
        if (armor >= 50)
            shards = 25;
        else
            shards = armor / 2;

        //determine the amount of armor per shard
        quantity = armor / shards;

        if (ent->client)
        {
            old_angle = ent->client->v_angle[YAW];
            angle = &ent->client->v_angle[YAW];
        }
        else
        {
            old_angle = ent->s.angles[YAW];
            angle = &ent->s.angles[YAW];
        }

        //drop shards in a random direction
        for (i = 0; i < shards; i++)
        {
            *angle += (360 / shards) + rand() % 30;
            shard = Drop_Item(ent, item);
            shard->count = quantity;
        }

        //reset the old angle
        *angle = old_angle;
    }
}

qboolean Kots_FakeShardPickup(edict_t *item, edict_t *ent)
{
    if (!ent->client && !(ent->svflags & SVF_MONSTER))
        return false;

    //damage the ent that picked it up
    T_Damage(ent, item, item->owner, item->velocity, item->s.origin, NULL, item->dmg, 0, DAMAGE_NO_RESIST | DAMAGE_ALL_ARMOR, MOD_FAKESHARD);
    return true;
}

void Kots_FakeShardMakeTouchable(edict_t *ent)
{
    ent->touch = Touch_Item;
    ent->nextthink = level.time + 120.0;
    ent->think = G_FreeEdict;
}

void Kots_CharacterDropFakeShard(edict_t *ent)
{
    if (ent->character->cur_technical < 7)
        gi.cprintf(ent, PRINT_HIGH, "You do not have the ability to create fake shards.\n");
    else if (ent->character->fakeshards >= 4)
        gi.cprintf(ent, PRINT_HIGH, "You already have the maximum fake shards out.\n");
    else if (ent->character->next_fakeshard > level.time)
        gi.cprintf(ent, PRINT_HIGH, "You must wait a while before dropping more fake shards.\n");
    else
    {
        int index = ARMOR_INDEX;

        if (ent->client->pers.inventory[index] < 50)
            gi.cprintf(ent, PRINT_HIGH, "You need at least 50 armor to create a fake shard.\n");
        else
        {
            gitem_t *item = FindItem("Fake Shard");
            edict_t *shard = Drop_Item(ent, item);
            shard->think = Kots_FakeShardMakeTouchable;
            shard->dmg = 200;

            ent->client->pers.inventory[index] -= 50;
            ++ent->character->fakeshards;
            ent->character->next_fakeshard = level.time + 1.0;

            //uncloak the character if cloaked
            Kots_CharacterUncloak(ent);
        }
    }
}

void Kots_TelefragItem(edict_t *ent)
{
    if (ent->item && (ent->item->tag & KOTS_ITEM_TELEFRAG) && ent->owner && ent->owner->character)
    {
        //make the item think
        ent->think(ent);

        //if it's still in use then it must've just been dropped so think again to really free it
        if (ent->inuse)
        {
            ent->think(ent);

            //if for some reason it still hasn't been freed then we're just going to force it to free
            if (ent->inuse)
                G_FreeEdict(ent);
        }
    }
}
void Kots_FreeItem(edict_t *ent)
{
    if (ent->item)
    {
        //currently only liftable items need special handling on freeing
        //this will save us from doing string comparisons on everything
        //but in the future we might want a tag for it
        if (ent->item->tag & KOTS_ITEM_LIFTABLE)
        {
            //all the following items require an owner
            if (ent->owner && ent->owner->character)
            {
                if (Q_stricmp(ent->item->pickup_name, "Fake Shard") == 0)
                    --ent->owner->character->fakeshards;
                else if (Q_stricmp(ent->item->pickup_name, "Fake Mega") == 0)
                    --ent->owner->character->fakehealths;
                else if (Q_stricmp(ent->item->pickup_name, "Fake Stim") == 0)
                    --ent->owner->character->fakehealths;
                else if (Q_stricmp(ent->item->pickup_name, "Kots Expack") == 0)
                    --ent->owner->character->expacksout;
                else if (Q_stricmp(ent->item->pickup_name, "Rocket Pack Mine") == 0 || Q_stricmp(ent->item->pickup_name, "Grenade Mine") == 0)
                    --ent->owner->character->minesout;
            }
        }
    }
}

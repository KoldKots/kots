CREATE PROCEDURE `Load2`(
        name varchar(45),
        out return_val bool
)
BEGIN

        declare id int;

        select a.id into id
        from characters a where a.name = name limit 1;

        if id is null
        then
                set return_val = 2;
        else

                /*
                  We're not selecting id as a basic precaution
                  This will simply help prevent people from finding
                  out other characters id's. Any user or server
                  with the database access should be trustworthy,
                  but still this might be a useful security feature.
                */
                select        /*a.id,*/ a.level, a.exp, a.resist, a.cubes, a.credits, a.rune_id, a.respawn_weapon, a.respawns, a.gender,
                              b.sabre, b.shotgun, b.supershotgun, b.machinegun, b.chaingun, b.grenade, b.grenadelauncher, b.rocketlauncher, b.hyperblaster, b.railgun, b.bfg, b.weaponpoints, b.antiweapon as wantiweapon,
                              c.dexterity, c.strength, c.karma, c.wisdom, c.technical, c.spirit, c.rage, c.vithealth, c.vitarmor, c.munition, c.playerpoints,
                              d.expack, d.spiral, d.bide, d.kotsthrow, d.antiweapon, d.powerpoints,
                              f.kills, f.killed, f.telefrags, f.twofers, f.threefers, f.highestfer, f.sprees, f.spreewars, f.spreesbroken, f.spreewarsbroken, f.longestspree, f.suicides, f.teleports, f.timeplayed,
                              g.highjump, g.spiritswim, g.pconvert, g.laserhook_color, g.cgconvert
                from          characters a inner join
                              characters_weapon b on a.id = b.character_id inner join
                              characters_player c on a.id = c.character_id inner join
                              characters_power d on a.id = d.character_id inner join
                              characters_stats f on a.id = f.character_id inner join
                              characters_settings g on a.id = g.character_id
                where         a.id = id;

                set return_val = 0;
        end if;

END

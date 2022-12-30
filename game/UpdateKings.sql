CREATE PROCEDURE `UpdateKings`()
BEGIN

    update kings
    set character_id = (select a.id from v_possible_kings a order by a.exp desc limit 1)
    where id = 1;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.dexterity desc, a.exp desc limit 1)
    where id = 2;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.strength desc, a.exp desc limit 1)
    where id = 3;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.karma desc, a.exp desc limit 1)
    where id = 4;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.wisdom desc, a.exp desc limit 1)
    where id = 5;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.technical desc, a.exp desc limit 1)
    where id = 6;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.spirit desc, a.exp desc limit 1)
    where id = 7;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.rage desc, a.exp desc limit 1)
    where id = 8;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.vithealth desc, a.exp desc limit 1)
    where id = 9;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.vitarmor desc, a.exp desc limit 1)
    where id = 10;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.munition desc, a.exp desc limit 1)
    where id = 11;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.kills desc, a.exp desc limit 1)
    where id = 12;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by (b.kills / b.timeplayed / 60) desc, a.exp desc limit 1)
    where id = 13;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.sprees desc, a.exp desc limit 1)
    where id = 14;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.spreewars desc, a.exp desc limit 1)
    where id = 15;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.spreesbroken desc, a.exp desc limit 1)
    where id = 16;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.spreewarsbroken desc, a.exp desc limit 1)
    where id = 17;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.twofers desc, a.exp desc limit 1)
    where id = 18;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.threefers desc, a.exp desc limit 1)
    where id = 19;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 1 order by b.sabre desc, a.exp desc limit 1)
    where id = 20;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 4 order by b.machinegun desc, a.exp desc limit 1)
    where id = 21;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 2 order by b.shotgun desc, a.exp desc limit 1)
    where id = 22;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 3 order by b.supershotgun desc, a.exp desc limit 1)
    where id = 23;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 5 order by b.chaingun desc, a.exp desc limit 1)
    where id = 24;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 6 order by b.grenade desc, a.exp desc limit 1)
    where id = 25;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 7 order by b.grenadelauncher desc, a.exp desc limit 1)
    where id = 26;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 8 order by b.rocketlauncher desc, a.exp desc limit 1)
    where id = 27;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 9 order by b.hyperblaster desc, a.exp desc limit 1)
    where id = 28;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 10 order by b.railgun desc, a.exp desc limit 1)
    where id = 29;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 11 order by b.bfg desc, a.exp desc limit 1)
    where id = 30;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.teleports desc, a.exp desc limit 1)
    where id = 31;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.telefrags desc, a.exp desc limit 1)
    where id = 32;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by (b.kills / (b.kills + b.killed)) desc, a.exp desc limit 1)
    where id = 33;

    update kings
    set character_id = (select a.id from v_possible_kings a order by a.credits desc, a.exp desc limit 1)
    where id = 34;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.suicides asc, a.exp desc limit 1)
    where id = 35;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.suicides desc, a.exp desc limit 1)
    where id = 36;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by (b.kills / (b.kills + b.killed)) asc, a.exp desc limit 1)
    where id = 37;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.killed desc, a.exp desc limit 1)
    where id = 38;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id where b.dexterity < 7 and b.strength < 7 and b.karma < 7 and b.wisdom < 7 and b.technical < 7 and b.spirit < 7 and b.rage < 7 order by a.exp desc limit 1)
    where id = 39;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where b.sabre < 6 and b.shotgun < 6 and b.supershotgun < 6 and b.machinegun < 6 and b.chaingun < 6 and b.grenade < 6 and b.grenadelauncher < 6 and b.rocketlauncher < 6 and b.hyperblaster < 6 and b.railgun < 6 and b.bfg < 6 order by a.exp desc limit 1)
    where id = 40;

    update kings
    set character_id = (select a.id from v_possible_kings a inner join characters_power b on a.id = b.character_id where b.spiral > 0 and b.bide > 0 and b.kotsthrow > 0 and b.expack > 0 and b.antiweapon > 0 order by a.exp desc limit 1)
    where id = 41;

    /* reset all the current kings */
    update characters
    set king_id = null
    where king_id is not null;

    /* determine the best title for all kings */
    update characters
    set king_id = (select a.id from kings a where a.character_id = characters.id order by a.priority asc limit 1)
    where id in (select character_id from kings);

END
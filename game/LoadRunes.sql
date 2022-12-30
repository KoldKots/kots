CREATE PROCEDURE `LoadRunes`()
BEGIN

SELECT  a.id, a.name, a.pickup_text, a.model_name, a.image_name, a.sound_name, a.effects, a.renderfx, a.rarity, a.mins, a.maxs,
        b.dexterity, b.strength, b.karma, b.wisdom, b.technical, b.spirit, b.rage, b.vithealth, b.vitarmor, b.munition,
        c.sabre, c.shotgun, c.supershotgun, c.machinegun, c.chaingun, c.grenade, c.grenadelauncher, c.rocketlauncher, c.hyperblaster, c.railgun, c.bfg,
        d.expack, d.spiral, d.bide, d.throw, d.antiweapon,
        e.tballs, e.tball_regen, e.tball_speed, e.normal_resist, e.energy_resist
FROM    runes a inner join
        runes_player b on a.id = b.rune_id inner join
        runes_weapon c on a.id = c.rune_id inner join
        runes_power d on a.id = d.rune_id inner join
        runes_other e on a.id = e.rune_id;

END

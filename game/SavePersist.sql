CREATE PROCEDURE `SavePersist`(
        id int,
        health int,
        armor int,
        weapon int,
        persist tinyint(1),
        shotgun tinyint(1),
        supershotgun tinyint(1),
        machinegun tinyint(1),
        chaingun tinyint(1),
        grenadelauncher tinyint(1),
        rocketlauncher tinyint(1),
        hyperblaster tinyint(1),
        railgun tinyint(1),
        bfg tinyint(1),
        shells int,
        bullets int,
        grenades int,
        rockets int,
        cells int,
        slugs int
)
BEGIN

        update        characters_persist a
        set           a.health = health,
                      a.armor = armor,
                      a.shotgun = shotgun,
                      a.supershotgun = supershotgun,
                      a.machinegun = machinegun,
                      a.chaingun = chaingun,
                      a.grenadelauncher = grenadelauncher,
                      a.rocketlauncher = rocketlauncher,
                      a.hyperblaster = hyperblaster,
                      a.railgun = railgun,
                      a.bfg = bfg,
                      a.weapon = weapon,
                      a.shells = shells,
                      a.bullets = bullets,
                      a.grenades = grenades,
                      a.rockets = rockets,
                      a.cells = cells,
                      a.slugs = slugs,
                      a.persist = persist
        where         a.character_id = id;

END

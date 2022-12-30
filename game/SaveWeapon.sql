CREATE PROCEDURE `SaveWeapon3`(
        id int,
        sabre int,
        shotgun int,
        supershotgun int,
        machinegun int,
        chaingun int,
        grenade int,
        grenadelauncher int,
        rocketlauncher int,
        hyperblaster int,
        railgun int,
        bfg int,
        antiweapon int,
        weaponsbought int,
        weaponpoints int
)
BEGIN

        update        characters_weapon a
        set           a.sabre = sabre,
                      a.shotgun = shotgun,
                      a.supershotgun = supershotgun,
                      a.machinegun = machinegun,
                      a.chaingun = chaingun,
                      a.grenade = grenade,
                      a.grenadelauncher = grenadelauncher,
                      a.rocketlauncher = rocketlauncher,
                      a.hyperblaster = hyperblaster,
                      a.railgun = railgun,
                      a.bfg = bfg,
                      a.antiweapon = antiweapon,
                      a.weaponsbought = weaponsbought,
                      a.weaponpoints = weaponpoints
        where         a.character_id = id;

END

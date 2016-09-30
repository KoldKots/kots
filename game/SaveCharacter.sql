CREATE PROCEDURE `SaveCharacter3`(
        id int,
        `level` int,
        exp int,
        gender char,
        resist int,
        cubes int,
        credits int,
        respawn_weapon int,
        respawns int,
        loggedin tinyint(1),
        cursed tinyint(1),
        isadmin tinyint(1),
		respec_points int
)
BEGIN

        update        characters a
        set           a.`level` = `level`,
                      a.exp = exp,
                      a.gender = gender,
                      a.resist = resist,
                      a.cubes = cubes,
                      a.credits = credits,
                      a.respawn_weapon = respawn_weapon,
                      a.respawns = respawns,
                      a.lastsaved = NOW(),
                      a.loggedin = loggedin,
                      a.cursed = cursed,
                      a.isadmin = isadmin,
					  a.respec_points = respec_points
        where         a.id = id;
        
END

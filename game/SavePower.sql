CREATE PROCEDURE `SavePower2`(
        id int,
        expack int,
        spiral int,
        bide int,
        kotsthrow int,
        antiweapon int,
        powersbought int,
        powerpoints int
)
BEGIN

        update        characters_power a
        set           a.expack = expack,
                      a.spiral = spiral,
                      a.bide = bide,
                      a.kotsthrow = kotsthrow,
                      a.antiweapon = antiweapon,
                      a.powersbought = powersbought,
                      a.powerpoints = powerpoints
        where         a.character_id = id;

END

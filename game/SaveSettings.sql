CREATE PROCEDURE `SaveSettings2`(
        id int,
        highjump tinyint(1),
        spiritswim tinyint(1),
        pconvert tinyint(1),
        laserhook_color int(1),
        cgconvert tinyint(1)
)
BEGIN

        update        characters_settings a
        set           a.highjump = highjump,
                      a.spiritswim = spiritswim,
                      a.pconvert = pconvert,
                      a.laserhook_color = laserhook_color,
                      a.cgconvert = cgconvert
        where         a.character_id = id;

END

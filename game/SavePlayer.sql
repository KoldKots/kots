CREATE PROCEDURE `SavePlayer2`(
        id int,
        dexterity int,
        strength int,
        karma int,
        wisdom int,
        technical int,
        spirit int,
        rage int,
        vithealth int,
        vitarmor int,
        munition int,
        playersbought int,
        playerpoints int
)
BEGIN

        update        characters_player a
        set           a.dexterity = dexterity,
                      a.strength = strength,
                      a.karma = karma,
                      a.wisdom = wisdom,
                      a.technical = technical,
                      a.spirit = spirit,
                      a.rage = rage,
                      a.vithealth = vithealth,
                      a.vitarmor = vitarmor,
                      a.munition = munition,
                      a.playersbought = playersbought,
                      a.playerpoints = playerpoints
        where         a.character_id = id;

END

CREATE PROCEDURE `DeleteAllCharacters`()
BEGIN

        update kings set character_id = null;
        delete from characters_persist;
        delete from characters_player;
        delete from characters_power;
        delete from characters_weapon;
        delete from characters;

END

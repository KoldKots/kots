CREATE PROCEDURE `DeleteCharacter`(id int)
BEGIN

		update kings set character_id = null where kings.character_id = id;
        delete from characters_persist where characters_player.character_id = id;
        delete from characters_player where characters_player.character_id = id;
        delete from characters_power where characters_power.character_id = id;
        delete from characters_weapon where characters_weapon.character_id = id;
        delete from characters where characters.id = id;

END

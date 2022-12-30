CREATE PROCEDURE `CreateCharacter2`(
        name varchar(45),
        pass varchar(45),
        out return_val int
)
BEGIN

        declare id int;

                /* NOTE: For list of return values see kots_server.h */

        if exists (select 1 from settings a where a.name = 'disable_login' and value = '1')
        then

                        /* Login has been disabled */
                        set return_val = 8;

        elseif exists (select 1 from characters a where a.name = name for update)
        then

                        /* Character already exists */
                        set return_val = 5;

        else

            insert into characters (name, pass, datecreated)
            values (name, MD5(pass), NOW());

            set id = @@IDENTITY;
            insert into characters_power (character_id) values (id);
            insert into characters_weapon (character_id) values (id);
            insert into characters_player (character_id) values (id);
            insert into characters_persist (character_id, persist) values (id, 0);
            insert into characters_stats (character_id) values (id);
            insert into characters_settings (character_id) values (id);

                        /* Everythign was successful */
            set return_val = 0;

        end if;

END

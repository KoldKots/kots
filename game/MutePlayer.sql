CREATE PROCEDURE `MutePlayer`(
        ip_address varchar(15),
        name varchar(16),
        character_id int
)
BEGIN

        declare id int;

        select a.id into id
        from mutelist a where a.ip_address = ip_address limit 1 for update;

        if id is null
        then
        
                                if character_id = 0
                                then
                                        set character_id = 0;
                                end if;
        
                insert into             mutelist
                                                                (ip_address, name, character_id, date_muted)
                                values                  (ip_address, name, character_id, NOW());
        end if;

END

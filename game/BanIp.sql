CREATE PROCEDURE `BanIp`(
        ip_address varchar(15),
        description varchar(512)
)
BEGIN

        declare id int;

        select a.id into id
        from ipbans a where a.ip_address = ip_address limit 1 for update;

        if id is null
        then
                insert into             ipbans
                                                                (ip_address, date, description)
                                values                  (ip_address, NOW(), description);
        end if;

END

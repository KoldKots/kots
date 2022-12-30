CREATE PROCEDURE `LoadServers`()
BEGIN

    /*
    Servers not updated for a while are considered down
    */
    select  name, ip_address, port, max_players, cur_players, map, hostname,
            CASE
                WHEN NOW() < DATE_ADD(last_updated, interval 60 minute) THEN 1
                ELSE 0
            END as status
    from    `servers`
    where   is_public = 1
    having  status = 1;
        
END
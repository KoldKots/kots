CREATE PROCEDURE `UpdateServer3`(
        name varchar(128),
        port int(4),
        max_players int(4),
        cur_players int(4),
        map varchar(64),
        is_public tinyint
)
BEGIN

	declare id int;
	declare hostname varchar(256);
	declare username varchar(128);

	/* get ip address and server id if one exists */
	select substr(user(), instr(user(), '@') + 1) into hostname;
	select substr(user(), 1, instr(user(), '@') - 1) into username;
	select a.id from `servers` a where a.hostname = hostname and a.port = port into id;
	
	if id is null
    then
    
            insert into `servers` (name, port, max_players, cur_players, map, username, last_updated, is_public, hostname)
            values (name, port, max_players, cur_players, map, username, NOW(), is_public, hostname);
            
    else
            
            update	`servers`
            set		servers.name = name,
					servers.max_players = max_players,
					servers.cur_players = cur_players,
					servers.map = map,
					servers.username = username,
					servers.is_public = is_public,
					servers.last_updated = NOW()
			where	servers.id = id;
            
    end if;
	
END
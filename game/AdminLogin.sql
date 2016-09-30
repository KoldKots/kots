CREATE PROCEDURE `AdminLogin`(
        name varchar(45),
        pass varchar(45),
        ip_address varchar(15),
        out return_val int
)
BEGIN

        declare realpass varchar(45);
        declare id int;
        declare isadmin tinyint;
        
        /* NOTE: See kots_server.h for list of return values */
        /* TODO: Possibly add an admins table that this uses instead of characters... */
        
        /* Default the return value to everything ok */
        set return_val = 0;
        
        /* Get the character id and real password */
        select a.id, a.pass, a.isadmin into id, realpass, isadmin
        from characters a where a.name = name limit 1;

		/* Check if the character was found */
        if id is null
        then
        
				/* Not found */
                set return_val = 2;
                
        elseif (isadmin <> 1)
        then
        
				/* Not an admin */
				set return_val = 7;
                
        elseif (realpass <> MD5(pass))
        then
        
				/* Incorrect password */
				set return_val = 3;
                
                /* TODO: Possibly add some logging in here */
        end if;

END

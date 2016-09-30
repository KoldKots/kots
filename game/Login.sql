CREATE PROCEDURE `Login3`(
        name varchar(45),
        pass varchar(45),
        ip_address varchar(15),
        out return_val int
)
BEGIN

	declare title varchar(50);
    declare realpass varchar(45);
    declare id int;
    declare lastsaved datetime;
    declare isloggedin bool;
    declare allow_login bool;

	if exists (select 1 from settings a where a.name = 'disable_login' and value = '1')
	then
	
		/* Login has been disabled */
		set return_val = 8;
		
	else
	
        select a.id, a.pass, a.lastsaved, a.loggedin, a.allow_login into id, realpass, lastsaved, isloggedin, allow_login
        from characters a where a.name = name limit 1 for update;

        if id is null
        then
        
			/* Character doesn't exist */
            set return_val = 2;
                
        elseif (realpass <> MD5(pass))
        then
        
            /* Incorrect password */
            set return_val = 3;

        elseif (isloggedin AND lastsaved is not null AND DATE_ADD(lastsaved, interval 10 minute) > NOW())
        then
        
            /* Already logged in */
            set return_val = 4;
            
        elseif not allow_login
        then
        
			/* Login for this character has been disabled */
			set return_val = 8;

        else
        
			update        characters a
			set           loggedin = 1,
							lastsaved = NOW()
			where         a.id = id;

			insert into		characters_history
							(character_id, logindate, ip_address)
			values			(id, NOW(), ip_address);

			select        a.id, a.level, a.exp, a.resist, a.cubes, a.credits, a.rune_id, a.respawn_weapon, a.respawns, a.isadmin, a.gender, a.cursed,
							case
							when length(a.title) <> 0 then a.title
							when a.gender = 'f' then h.title_female
							else h.title_male
							end as title,
							b.sabre, b.shotgun, b.supershotgun, b.machinegun, b.chaingun, b.grenade, b.grenadelauncher, b.rocketlauncher, b.hyperblaster, b.railgun, b.bfg, b.weaponpoints, b.antiweapon as wantiweapon, b.weaponsbought,
							c.dexterity, c.strength, c.karma, c.wisdom, c.technical, c.spirit, c.rage, c.vithealth, c.vitarmor, c.munition, c.playerpoints, c.playersbought,
							d.expack, d.spiral, d.bide, d.kotsthrow, d.antiweapon, d.powerpoints, d.powersbought,
							e.health, e.armor, e.weapon, e.persist, e.shotgun as persist_shotgun, e.supershotgun as persist_supershotgun, e.machinegun as persist_machinegun, e.chaingun as persist_chaingun, e.grenadelauncher as persist_grenadelauncher, e.rocketlauncher as persist_rocketlauncher, e.hyperblaster as persist_hyperblaster, e.railgun as persist_railgun, e.bfg as persist_bfg, e.shells, e.bullets, e.grenades, e.rockets, e.cells, e.slugs,
							f.kills, f.killed, f.telefrags, f.twofers, f.threefers, f.highestfer, f.sprees, f.spreewars, f.spreesbroken, f.spreewarsbroken, f.longestspree, f.suicides, f.teleports, f.timeplayed, f.total_credits, f.total_packs,
							g.highjump, g.spiritswim, g.pconvert, a.respec_points
			from          characters a inner join
							characters_weapon b on a.id = b.character_id inner join
							characters_player c on a.id = c.character_id inner join
							characters_power d on a.id = d.character_id inner join
							characters_persist e on a.id = e.character_id inner join
							characters_stats f on a.id = f.character_id inner join
							characters_settings g on a.id = g.character_id left join
							kings h on a.king_id = h.id
			where         a.id = id;
			
			set return_val = 0;
        
        end if;
	end if;

END

-- phpMyAdmin SQL Dump
-- version 2.11.2.1
-- http://www.phpmyadmin.net
--
-- Generation Time: Jun 02, 2008 at 09:18 PM
-- Server version: 5.0.24
-- PHP Version: 4.4.7

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

--
-- Database: `kots`
--

-- --------------------------------------------------------

--
-- Table structure for table `characters`
--

CREATE TABLE IF NOT EXISTS `characters` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name` varchar(45) NOT NULL,
  `pass` varchar(45) NOT NULL,
  `level` int(10) unsigned NOT NULL default '0',
  `exp` int(10) unsigned NOT NULL default '0',
  `resist` int(10) unsigned NOT NULL default '0',
  `cubes` int(10) unsigned NOT NULL default '200',
  `credits` int(10) unsigned NOT NULL default '0',
  `rune_id` int(10) unsigned default NULL,
  `datecreated` datetime NOT NULL,
  `lastsaved` datetime default NULL,
  `respawn_weapon` int(10) unsigned NOT NULL default '8',
  `respawns` int(10) unsigned NOT NULL default '200',
  `cursed` tinyint(1) NOT NULL default '0',
  `loggedin` tinyint(1) NOT NULL default '0',
  `isadmin` tinyint(1) NOT NULL default '0',
  `isboss` tinyint(1) NOT NULL default '0',
  `title` varchar(30) default NULL,
  `gender` char(1) NOT NULL default 'M',
  `king_id` int(4) unsigned default NULL,
  `allow_login` tinyint(1) NOT NULL default '1',
  `respec_points` int(4) NOT NULL default '0'
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name_ix` USING BTREE (`name`),
  KEY `rune_id_ix` USING BTREE (`rune_id`),
  KEY `respawn_weapon_ix` USING BTREE (`respawn_weapon`),
  KEY `resist_ix` USING BTREE (`resist`),
  KEY `king_id` (`king_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=2147 ;

-- --------------------------------------------------------

--
-- Table structure for table `characters_history`
--

CREATE TABLE IF NOT EXISTS `characters_history` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `character_id` int(10) unsigned NOT NULL,
  `logindate` datetime NOT NULL,
  `ip_address` varchar(15) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `ip_address` (`ip_address`),
  KEY `FK_character_id` (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=170866 ;

-- --------------------------------------------------------

--
-- Table structure for table `characters_persist`
--

CREATE TABLE IF NOT EXISTS `characters_persist` (
  `character_id` int(10) unsigned NOT NULL,
  `health` int(10) unsigned NOT NULL default '100',
  `armor` int(10) unsigned NOT NULL default '0',
  `shotgun` tinyint(1) NOT NULL default '0',
  `supershotgun` tinyint(1) NOT NULL default '0',
  `machinegun` tinyint(1) NOT NULL default '0',
  `chaingun` tinyint(1) NOT NULL default '0',
  `grenadelauncher` tinyint(1) NOT NULL default '0',
  `rocketlauncher` tinyint(1) NOT NULL default '0',
  `hyperblaster` tinyint(1) NOT NULL default '0',
  `railgun` tinyint(1) NOT NULL default '0',
  `bfg` tinyint(1) NOT NULL default '0',
  `weapon` int(10) unsigned NOT NULL default '12',
  `shells` int(10) unsigned NOT NULL default '0',
  `bullets` int(10) unsigned NOT NULL default '0',
  `grenades` int(10) unsigned NOT NULL default '0',
  `rockets` int(10) unsigned NOT NULL default '0',
  `cells` int(10) unsigned NOT NULL default '0',
  `slugs` int(10) unsigned NOT NULL default '0',
  `persist` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `characters_player`
--

CREATE TABLE IF NOT EXISTS `characters_player` (
  `character_id` int(10) unsigned NOT NULL,
  `dexterity` int(10) unsigned NOT NULL default '0',
  `strength` int(10) unsigned NOT NULL default '0',
  `karma` int(10) unsigned NOT NULL default '0',
  `wisdom` int(10) unsigned NOT NULL default '0',
  `technical` int(10) unsigned NOT NULL default '0',
  `spirit` int(10) unsigned NOT NULL default '0',
  `rage` int(10) unsigned NOT NULL default '0',
  `vithealth` int(10) unsigned NOT NULL default '0',
  `vitarmor` int(10) unsigned NOT NULL default '0',
  `munition` int(10) unsigned NOT NULL default '0',
  `playersbought` int(10) unsigned NOT NULL default '0',
  `playerpoints` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `characters_power`
--

CREATE TABLE IF NOT EXISTS `characters_power` (
  `character_id` int(10) unsigned NOT NULL,
  `expack` int(10) unsigned NOT NULL default '0',
  `spiral` int(10) unsigned NOT NULL default '0',
  `bide` int(10) unsigned NOT NULL default '0',
  `kotsthrow` int(10) unsigned NOT NULL default '0',
  `antiweapon` int(10) unsigned NOT NULL default '0',
  `powersbought` int(10) unsigned NOT NULL default '0',
  `powerpoints` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `characters_settings`
--

CREATE TABLE IF NOT EXISTS `characters_settings` (
  `character_id` int(10) unsigned NOT NULL,
  `highjump` tinyint(1) NOT NULL default '1',
  `spiritswim` tinyint(1) NOT NULL default '1',
  `pconvert` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `characters_stats`
--

CREATE TABLE IF NOT EXISTS `characters_stats` (
  `character_id` int(10) unsigned NOT NULL,
  `kills` int(10) unsigned NOT NULL default '0',
  `killed` int(10) unsigned NOT NULL default '0',
  `telefrags` int(10) unsigned NOT NULL default '0',
  `twofers` int(10) unsigned NOT NULL default '0',
  `threefers` int(10) unsigned NOT NULL default '0',
  `highestfer` int(10) unsigned NOT NULL default '0',
  `sprees` int(10) unsigned NOT NULL default '0',
  `spreewars` int(10) unsigned NOT NULL default '0',
  `spreesbroken` int(10) unsigned NOT NULL default '0',
  `spreewarsbroken` int(10) unsigned NOT NULL default '0',
  `longestspree` int(10) unsigned NOT NULL default '0',
  `suicides` int(10) unsigned NOT NULL default '0',
  `teleports` int(10) unsigned NOT NULL default '0',
  `timeplayed` int(10) unsigned NOT NULL default '0',
  `total_packs` int(10) unsigned NOT NULL default '0',
  `total_credits` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `characters_weapon`
--

CREATE TABLE IF NOT EXISTS `characters_weapon` (
  `character_id` int(10) unsigned NOT NULL,
  `sabre` int(10) unsigned NOT NULL default '0',
  `shotgun` int(10) unsigned NOT NULL default '0',
  `supershotgun` int(10) unsigned NOT NULL default '0',
  `machinegun` int(10) unsigned NOT NULL default '0',
  `chaingun` int(10) unsigned NOT NULL default '0',
  `grenade` int(10) unsigned NOT NULL default '0',
  `grenadelauncher` int(10) unsigned NOT NULL default '0',
  `rocketlauncher` int(10) unsigned NOT NULL default '0',
  `hyperblaster` int(10) unsigned NOT NULL default '0',
  `railgun` int(10) unsigned NOT NULL default '0',
  `bfg` int(10) unsigned NOT NULL default '0',
  `antiweapon` int(4) unsigned NOT NULL default '0',
  `weaponsbought` int(10) unsigned NOT NULL default '0',
  `weaponpoints` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `characters_web`
--

CREATE TABLE IF NOT EXISTS `characters_web` (
  `name` varchar(45) NOT NULL,
  `email` varchar(50) NOT NULL,
  `location` varchar(25) NOT NULL,
  `age` int(11) NOT NULL,
  `posts` int(11) NOT NULL,
  `sig` text NOT NULL,
  `lastseen` int(11) NOT NULL,
  `avatar` int(3) NOT NULL default '1',
  `av_ups` int(2) NOT NULL,
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `comments`
--

CREATE TABLE IF NOT EXISTS `comments` (
  `id` int(4) unsigned NOT NULL auto_increment,
  `news_id` int(4) unsigned NOT NULL,
  `msg` text NOT NULL,
  `when` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `by` varchar(45) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `news_id_ix` (`news_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=20 ;

-- --------------------------------------------------------

--
-- Table structure for table `ipbans`
--

CREATE TABLE IF NOT EXISTS `ipbans` (
  `id` int(4) unsigned NOT NULL auto_increment,
  `ip_address` varchar(15) NOT NULL,
  `date` datetime NOT NULL,
  `description` varchar(512) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `ip_address` (`ip_address`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=9 ;

-- --------------------------------------------------------

--
-- Table structure for table `kings`
--

CREATE TABLE IF NOT EXISTS `kings` (
  `id` int(4) unsigned NOT NULL auto_increment,
  `character_id` int(10) unsigned default NULL,
  `title_male` varchar(50) NOT NULL,
  `title_female` varchar(50) NOT NULL,
  `desc` varchar(100) NOT NULL,
  `priority` int(4) unsigned NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `ix_priority` (`priority`),
  KEY `ix_character_id` (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=42 ;

-- --------------------------------------------------------

--
-- Table structure for table `log`
--

CREATE TABLE IF NOT EXISTS `log` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `date` datetime NOT NULL,
  `info` varchar(2048) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=4358 ;

-- --------------------------------------------------------

--
-- Table structure for table `mutelist`
--

CREATE TABLE IF NOT EXISTS `mutelist` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `date_muted` datetime NOT NULL,
  `ip_address` varchar(15) NOT NULL,
  `character_id` int(10) unsigned default NULL,
  `name` varchar(45) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `mutes_ip_address_ix` (`ip_address`),
  KEY `mutes_character_id_fk` (`character_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `news`
--

CREATE TABLE IF NOT EXISTS `news` (
  `id` int(4) unsigned NOT NULL auto_increment,
  `subj` varchar(255) NOT NULL,
  `msg` text NOT NULL,
  `when` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `by` varchar(45) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=24 ;

-- --------------------------------------------------------

--
-- Table structure for table `runes`
--

CREATE TABLE IF NOT EXISTS `runes` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name` varchar(30) NOT NULL,
  `pickup_text` varchar(256) NOT NULL,
  `model_name` varchar(64) NOT NULL,
  `image_name` varchar(64) NOT NULL,
  `sound_name` varchar(64) default NULL,
  `mins` varchar(30) NOT NULL default '-16,-16,-16',
  `maxs` varchar(30) NOT NULL default '16,16,16',
  `effects` int(4) unsigned NOT NULL default '0',
  `renderfx` int(4) unsigned NOT NULL default '0',
  `rarity` double NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=39 ;

-- --------------------------------------------------------

--
-- Table structure for table `runes_other`
--

CREATE TABLE IF NOT EXISTS `runes_other` (
  `rune_id` int(10) unsigned NOT NULL,
  `tballs` int(10) NOT NULL default '0',
  `tball_regen` tinyint(1) NOT NULL default '0',
  `tball_speed` tinyint(1) NOT NULL default '0',
  `normal_resist` tinyint(1) NOT NULL default '0',
  `energy_resist` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`rune_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `runes_player`
--

CREATE TABLE IF NOT EXISTS `runes_player` (
  `rune_id` int(10) unsigned NOT NULL,
  `dexterity` int(4) NOT NULL default '0',
  `strength` int(4) NOT NULL default '0',
  `karma` int(4) NOT NULL default '0',
  `wisdom` int(4) NOT NULL default '0',
  `technical` int(4) NOT NULL default '0',
  `spirit` int(4) NOT NULL default '0',
  `rage` int(4) NOT NULL default '0',
  `vithealth` int(4) NOT NULL default '0',
  `vitarmor` int(4) NOT NULL default '0',
  `munition` int(4) NOT NULL default '0',
  PRIMARY KEY  (`rune_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `runes_power`
--

CREATE TABLE IF NOT EXISTS `runes_power` (
  `rune_id` int(10) unsigned NOT NULL,
  `expack` int(4) NOT NULL default '0',
  `spiral` int(4) NOT NULL default '0',
  `bide` int(4) NOT NULL default '0',
  `throw` int(4) NOT NULL default '0',
  `antiweapon` int(4) NOT NULL default '0',
  PRIMARY KEY  (`rune_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `runes_weapon`
--

CREATE TABLE IF NOT EXISTS `runes_weapon` (
  `rune_id` int(10) unsigned NOT NULL,
  `sabre` int(4) NOT NULL default '0',
  `shotgun` int(4) NOT NULL default '0',
  `supershotgun` int(4) NOT NULL default '0',
  `machinegun` int(4) NOT NULL default '0',
  `chaingun` int(4) NOT NULL default '0',
  `grenade` int(4) NOT NULL default '0',
  `grenadelauncher` int(4) NOT NULL default '0',
  `rocketlauncher` int(4) NOT NULL default '0',
  `hyperblaster` int(4) NOT NULL default '0',
  `railgun` int(4) NOT NULL default '0',
  `bfg` int(4) NOT NULL default '0',
  PRIMARY KEY  (`rune_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `servers`
--

CREATE TABLE IF NOT EXISTS `servers` (
  `id` int(4) unsigned NOT NULL auto_increment,
  `name` varchar(128) NOT NULL,
  `ip_address` varchar(15) default NULL,
  `port` int(4) NOT NULL,
  `max_players` int(4) NOT NULL,
  `cur_players` int(4) NOT NULL,
  `map` varchar(64) NOT NULL,
  `last_updated` datetime NOT NULL,
  `username` varchar(128) NOT NULL,
  `is_public` tinyint(1) NOT NULL default '1',
  `hostname` varchar(256) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `ix_servers_address` USING BTREE (`hostname`,`port`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=48 ;

-- --------------------------------------------------------

--
-- Table structure for table `settings`
--

CREATE TABLE IF NOT EXISTS `settings` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name` varchar(50) NOT NULL,
  `value` varchar(50) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=3 ;

-- --------------------------------------------------------

--
-- Stand-in structure for view `v_possible_kings`
--
CREATE TABLE IF NOT EXISTS `v_possible_kings` (
`id` int(10) unsigned
,`name` varchar(45)
,`pass` varchar(45)
,`level` int(10) unsigned
,`exp` int(10) unsigned
,`resist` int(10) unsigned
,`cubes` int(10) unsigned
,`credits` int(10) unsigned
,`rune_id` int(10) unsigned
,`datecreated` datetime
,`lastsaved` datetime
,`respawn_weapon` int(10) unsigned
,`respawns` int(10) unsigned
,`cursed` tinyint(1)
,`loggedin` tinyint(1)
,`isadmin` tinyint(1)
,`title` varchar(30)
);
-- --------------------------------------------------------

--
-- Table structure for table `weapons`
--

CREATE TABLE IF NOT EXISTS `weapons` (
  `id` int(10) unsigned NOT NULL,
  `name` varchar(45) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure for view `v_possible_kings`
--
DROP TABLE IF EXISTS `v_possible_kings`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `kots`.`v_possible_kings` AS select `a`.`id` AS `id`,`a`.`name` AS `name`,`a`.`pass` AS `pass`,`a`.`level` AS `level`,`a`.`exp` AS `exp`,`a`.`resist` AS `resist`,`a`.`cubes` AS `cubes`,`a`.`credits` AS `credits`,`a`.`rune_id` AS `rune_id`,`a`.`datecreated` AS `datecreated`,`a`.`lastsaved` AS `lastsaved`,`a`.`respawn_weapon` AS `respawn_weapon`,`a`.`respawns` AS `respawns`,`a`.`cursed` AS `cursed`,`a`.`loggedin` AS `loggedin`,`a`.`isadmin` AS `isadmin`,`a`.`title` AS `title` from (`kots`.`characters` `a` join `kots`.`characters_stats` `b` on((`a`.`id` = `b`.`character_id`))) where ((`a`.`isboss` = 0) and (`a`.`cursed` = 0) and (`a`.`level` >= 7) and (`a`.`lastsaved` >= (now() - interval 2 week)) and (`b`.`timeplayed` > ((60 * 60) * 2)));

--
-- Constraints for dumped tables
--

--
-- Constraints for table `characters_history`
--
ALTER TABLE `characters_history`
  ADD CONSTRAINT `FK_character_id` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `characters_persist`
--
ALTER TABLE `characters_persist`
  ADD CONSTRAINT `FK_characters_persist_id` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `characters_player`
--
ALTER TABLE `characters_player`
  ADD CONSTRAINT `FK_characters_player_id` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `characters_power`
--
ALTER TABLE `characters_power`
  ADD CONSTRAINT `FK_characters_power_id` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `characters_settings`
--
ALTER TABLE `characters_settings`
  ADD CONSTRAINT `FK_characters_settings_id` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `characters_stats`
--
ALTER TABLE `characters_stats`
  ADD CONSTRAINT `FK_characters_stats_id` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `characters_weapon`
--
ALTER TABLE `characters_weapon`
  ADD CONSTRAINT `FK_characters_weapon_id` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `kings`
--
ALTER TABLE `kings`
  ADD CONSTRAINT `FK_kings_character_id` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE SET NULL;

--
-- Constraints for table `mutelist`
--
ALTER TABLE `mutelist`
  ADD CONSTRAINT `mutes_character_id_fk` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`);

--
-- Procedures
--
DELIMITER $$
--
$$

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

$$
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
$$
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
                insert into		ipbans
								(ip_address, date, description)
				values			(ip_address, NOW(), description);
        end if;

END

$$
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

$$
CREATE PROCEDURE `DeleteAllCharacters`()
BEGIN

		update kings set character_id = null;
        delete from characters_persist;
        delete from characters_player;
        delete from characters_power;
        delete from characters_weapon;
        delete from characters;

END

$$
CREATE PROCEDURE `DeleteCharacter`(id int)
BEGIN

		update kings set character_id = null where kings.character_id = id;
        delete from characters_persist where characters_player.character_id = id;
        delete from characters_player where characters_player.character_id = id;
        delete from characters_power where characters_power.character_id = id;
        delete from characters_weapon where characters_weapon.character_id = id;
        delete from characters where characters.id = id;

END

$$
CREATE PROCEDURE `Load2`(
        name varchar(45),
        out return_val bool
)
BEGIN

        declare id int;

        select a.id into id
        from characters a where a.name = name limit 1;

        if id is null
        then
                set return_val = 2;
        else

                /*
                  We're not selecting id as a basic precaution
                  This will simply help prevent people from finding
                  out other characters id's. Any user or server
                  with the database access should be trustworthy,
                  but still this might be a useful security feature.
                */
                select        /*a.id,*/ a.level, a.exp, a.resist, a.cubes, a.credits, a.rune_id, a.respawn_weapon, a.respawns, a.gender,
                              b.sabre, b.shotgun, b.supershotgun, b.machinegun, b.chaingun, b.grenade, b.grenadelauncher, b.rocketlauncher, b.hyperblaster, b.railgun, b.bfg, b.weaponpoints, b.antiweapon as wantiweapon,
                              c.dexterity, c.strength, c.karma, c.wisdom, c.technical, c.spirit, c.rage, c.vithealth, c.vitarmor, c.munition, c.playerpoints,
                              d.expack, d.spiral, d.bide, d.kotsthrow, d.antiweapon, d.powerpoints,
                              f.kills, f.killed, f.telefrags, f.twofers, f.threefers, f.highestfer, f.sprees, f.spreewars, f.spreesbroken, f.spreewarsbroken, f.longestspree, f.suicides, f.teleports, f.timeplayed,
                              g.highjump, g.spiritswim, g.pconvert
                from          characters a inner join
                              characters_weapon b on a.id = b.character_id inner join
                              characters_player c on a.id = c.character_id inner join
                              characters_power d on a.id = d.character_id inner join
                              characters_stats f on a.id = f.character_id inner join
                              characters_settings g on a.id = g.character_id
                where         a.id = id;
                
                set return_val = 0;
        end if;

END

$$
CREATE PROCEDURE `LoadIpBans`()
BEGIN

        select ip_address from ipbans;

END

$$
CREATE PROCEDURE `LoadMuteList`()
BEGIN

        select a.ip_address, a.name, a.character_id
        from mutelist a;

END

$$
CREATE PROCEDURE `LoadRunes`()
BEGIN

SELECT	a.id, a.name, a.pickup_text, a.model_name, a.image_name, a.sound_name, a.effects, a.renderfx, a.rarity, a.mins, a.maxs,
		b.dexterity, b.strength, b.karma, b.wisdom, b.technical, b.spirit, b.rage, b.vithealth, b.vitarmor, b.munition,
		c.sabre, c.shotgun, c.supershotgun, c.machinegun, c.chaingun, c.grenade, c.grenadelauncher, c.rocketlauncher, c.hyperblaster, c.railgun, c.bfg,
		d.expack, d.spiral, d.bide, d.throw, d.antiweapon,
		e.tballs, e.tball_regen, e.tball_speed, e.normal_resist, e.energy_resist
FROM	runes a inner join
		runes_player b on a.id = b.rune_id inner join
		runes_weapon c on a.id = c.rune_id inner join
		runes_power d on a.id = d.rune_id inner join
		runes_other e on a.id = e.rune_id;

END

$$
CREATE PROCEDURE `LoadServers`()
BEGIN

	/*
	Servers not updated for a while are considered down
	*/
	select	name, ip_address, port, max_players, cur_players, map, hostname,
			CASE
				WHEN NOW() < DATE_ADD(last_updated, interval 60 minute) THEN 1
				ELSE 0
			END as status
	from	`servers`
	where	is_public = 1
	having	status = 1;
		
END
$$
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

$$
CREATE PROCEDURE `LogInfo`(
        info varchar(2048)
)
BEGIN

	insert into `log` (`date`, `info`)
	values ( NOW(), info );

END

$$
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
        
                insert into		mutelist
								(ip_address, name, character_id, date_muted)
				values			(ip_address, name, character_id, NOW());
        end if;

END

$$
CREATE PROCEDURE `SaveCharacter3`(
        id int,
        `level` int,
        exp int,
        gender char,
        resist int,
        cubes int,
        credits int,
        respawn_weapon int,
        respawns int,
        loggedin tinyint(1),
        cursed tinyint(1),
        isadmin tinyint(1),
		respec_points int
)
BEGIN

        update        characters a
        set           a.`level` = `level`,
                      a.exp = exp,
                      a.gender = gender,
                      a.resist = resist,
                      a.cubes = cubes,
                      a.credits = credits,
                      a.respawn_weapon = respawn_weapon,
                      a.respawns = respawns,
                      a.lastsaved = NOW(),
                      a.loggedin = loggedin,
                      a.cursed = cursed,
                      a.isadmin = isadmin,
					  a.respec_points = respec_points
        where         a.id = id;
        
END

$$
CREATE PROCEDURE `SavePersist`(
        id int,
        health int,
        armor int,
        weapon int,
        persist tinyint(1),
        shotgun tinyint(1),
        supershotgun tinyint(1),
        machinegun tinyint(1),
        chaingun tinyint(1),
        grenadelauncher tinyint(1),
        rocketlauncher tinyint(1),
        hyperblaster tinyint(1),
        railgun tinyint(1),
        bfg tinyint(1),
        shells int,
        bullets int,
        grenades int,
        rockets int,
        cells int,
        slugs int
)
BEGIN

        update        characters_persist a
        set           a.health = health,
                      a.armor = armor,
                      a.shotgun = shotgun,
                      a.supershotgun = supershotgun,
                      a.machinegun = machinegun,
                      a.chaingun = chaingun,
                      a.grenadelauncher = grenadelauncher,
                      a.rocketlauncher = rocketlauncher,
                      a.hyperblaster = hyperblaster,
                      a.railgun = railgun,
                      a.bfg = bfg,
                      a.weapon = weapon,
                      a.shells = shells,
                      a.bullets = bullets,
                      a.grenades = grenades,
                      a.rockets = rockets,
                      a.cells = cells,
                      a.slugs = slugs,
                      a.persist = persist
        where         a.character_id = id;

END

$$
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

$$
CREATE PROCEDURE `SavePower2`(
        id int,
        expack int,
        spiral int,
        bide int,
        kotsthrow int,
        antiweapon int,
        powersbought int,
        powerpoints int
)
BEGIN

        update        characters_power a
        set           a.expack = expack,
                      a.spiral = spiral,
                      a.bide = bide,
                      a.kotsthrow = kotsthrow,
                      a.antiweapon = antiweapon,
                      a.powersbought = powersbought,
                      a.powerpoints = powerpoints
        where         a.character_id = id;

END

$$
CREATE PROCEDURE `SaveSettings2`(
        id int,
        highjump tinyint(1),
        spiritswim tinyint(1),
        pconvert tinyint(1)
)
BEGIN

        update        characters_settings a
        set           a.highjump = highjump,
                      a.spiritswim = spiritswim,
                      a.pconvert = pconvert
        where         a.character_id = id;

END

$$
CREATE PROCEDURE `SaveStats2`(
        id int,
        kills int,
        killed int,
        telefrags int,
        twofers int,
        threefers int,
        highestfer int,
        sprees int,
        spreewars int,
        spreesbroken int,
        spreewarsbroken int,
        longestspree int,
        suicides int,
        teleports int,
        timeplayed int,
        total_credits int,
        total_packs int
)
BEGIN

        update        characters_stats a
        set           a.kills = kills,
                      a.killed = killed,
                      a.telefrags = telefrags,
                      a.twofers = twofers,
                      a.threefers = threefers,
                      a.highestfer = highestfer,
                      a.sprees = sprees,
                      a.spreewars = spreewars,
                      a.spreesbroken = spreesbroken,
                      a.spreewarsbroken = spreewarsbroken,
                      a.longestspree = longestspree,
                      a.suicides = suicides,
                      a.teleports = teleports,
                      a.timeplayed = timeplayed,
                      a.total_credits = total_credits,
                      a.total_packs = total_packs
        where         a.character_id = id;

END

$$
CREATE PROCEDURE `SaveWeapon3`(
        id int,
        sabre int,
        shotgun int,
        supershotgun int,
        machinegun int,
        chaingun int,
        grenade int,
        grenadelauncher int,
        rocketlauncher int,
        hyperblaster int,
        railgun int,
        bfg int,
        antiweapon int,
        weaponsbought int,
        weaponpoints int
)
BEGIN

        update        characters_weapon a
        set           a.sabre = sabre,
                      a.shotgun = shotgun,
                      a.supershotgun = supershotgun,
                      a.machinegun = machinegun,
                      a.chaingun = chaingun,
                      a.grenade = grenade,
                      a.grenadelauncher = grenadelauncher,
                      a.rocketlauncher = rocketlauncher,
                      a.hyperblaster = hyperblaster,
                      a.railgun = railgun,
                      a.bfg = bfg,
                      a.antiweapon = antiweapon,
                      a.weaponsbought = weaponsbought,
                      a.weaponpoints = weaponpoints
        where         a.character_id = id;

END

$$
CREATE PROCEDURE `UnmutePlayer`(
        ip_address varchar(15)
)
BEGIN

        delete from mutelist where mutelist.ip_address = ip_address;

END

$$
CREATE PROCEDURE `UpdateKings`()
BEGIN

	update kings
	set character_id = (select a.id from v_possible_kings a order by a.exp desc limit 1)
	where id = 1;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.dexterity desc, a.exp desc limit 1)
	where id = 2;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.strength desc, a.exp desc limit 1)
	where id = 3;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.karma desc, a.exp desc limit 1)
	where id = 4;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.wisdom desc, a.exp desc limit 1)
	where id = 5;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.technical desc, a.exp desc limit 1)
	where id = 6;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.spirit desc, a.exp desc limit 1)
	where id = 7;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.rage desc, a.exp desc limit 1)
	where id = 8;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.vithealth desc, a.exp desc limit 1)
	where id = 9;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.vitarmor desc, a.exp desc limit 1)
	where id = 10;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id order by b.munition desc, a.exp desc limit 1)
	where id = 11;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.kills desc, a.exp desc limit 1)
	where id = 12;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by (b.kills / b.timeplayed / 60) desc, a.exp desc limit 1)
	where id = 13;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.sprees desc, a.exp desc limit 1)
	where id = 14;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.spreewars desc, a.exp desc limit 1)
	where id = 15;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.spreesbroken desc, a.exp desc limit 1)
	where id = 16;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.spreewarsbroken desc, a.exp desc limit 1)
	where id = 17;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.twofers desc, a.exp desc limit 1)
	where id = 18;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.threefers desc, a.exp desc limit 1)
	where id = 19;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 1 order by b.sabre desc, a.exp desc limit 1)
	where id = 20;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 4 order by b.machinegun desc, a.exp desc limit 1)
	where id = 21;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 2 order by b.shotgun desc, a.exp desc limit 1)
	where id = 22;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 3 order by b.supershotgun desc, a.exp desc limit 1)
	where id = 23;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 5 order by b.chaingun desc, a.exp desc limit 1)
	where id = 24;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 6 order by b.grenade desc, a.exp desc limit 1)
	where id = 25;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 7 order by b.grenadelauncher desc, a.exp desc limit 1)
	where id = 26;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 8 order by b.rocketlauncher desc, a.exp desc limit 1)
	where id = 27;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 9 order by b.hyperblaster desc, a.exp desc limit 1)
	where id = 28;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 10 order by b.railgun desc, a.exp desc limit 1)
	where id = 29;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where a.respawn_weapon = 11 order by b.bfg desc, a.exp desc limit 1)
	where id = 30;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.teleports desc, a.exp desc limit 1)
	where id = 31;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.telefrags desc, a.exp desc limit 1)
	where id = 32;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by (b.kills / (b.kills + b.killed)) desc, a.exp desc limit 1)
	where id = 33;

	update kings
	set character_id = (select a.id from v_possible_kings a order by a.credits desc, a.exp desc limit 1)
	where id = 34;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.suicides asc, a.exp desc limit 1)
	where id = 35;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.suicides desc, a.exp desc limit 1)
	where id = 36;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by (b.kills / (b.kills + b.killed)) asc, a.exp desc limit 1)
	where id = 37;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_stats b on a.id = b.character_id order by b.killed desc, a.exp desc limit 1)
	where id = 38;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_player b on a.id = b.character_id where b.dexterity < 7 and b.strength < 7 and b.karma < 7 and b.wisdom < 7 and b.technical < 7 and b.spirit < 7 and b.rage < 7 order by a.exp desc limit 1)
	where id = 39;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_weapon b on a.id = b.character_id where b.sabre < 6 and b.shotgun < 6 and b.supershotgun < 6 and b.machinegun < 6 and b.chaingun < 6 and b.grenade < 6 and b.grenadelauncher < 6 and b.rocketlauncher < 6 and b.hyperblaster < 6 and b.railgun < 6 and b.bfg < 6 order by a.exp desc limit 1)
	where id = 40;

	update kings
	set character_id = (select a.id from v_possible_kings a inner join characters_power b on a.id = b.character_id where b.spiral > 0 and b.bide > 0 and b.kotsthrow > 0 and b.expack > 0 and b.antiweapon > 0 order by a.exp desc limit 1)
	where id = 41;

	/* reset all the current kings */
	update characters
	set king_id = null
	where king_id is not null;

	/* determine the best title for all kings */
	update characters
	set king_id = (select a.id from kings a where a.character_id = characters.id order by a.priority asc limit 1)
	where id in (select character_id from kings);

END
$$

--
DELIMITER ;
--

-- phpMyAdmin SQL Dump
-- version 2.11.2.1
-- http://www.phpmyadmin.net
--
-- Host: kots.ihasacrayon.net
-- Generation Time: Jun 02, 2008 at 09:36 PM
-- Server version: 5.0.24
-- PHP Version: 4.4.7

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

--
-- Database: `kots`
--

--
-- Dumping data for table `kings`
--

INSERT INTO `kings` (`id`, `character_id`, `title_male`, `title_female`, `desc`, `priority`) VALUES
(1, NULL, 'King of all Servers', 'Queen of all Servers', 'Highest ranked player', 1),
(2, NULL, 'Dexterous Knight', 'Dexterous Dame', 'Highest ranked dexterity master', 2),
(3, NULL, 'Strong Knight', 'Strong Dame', 'Highest ranked strength master', 3),
(4, NULL, 'Karmic Knight', 'Karmic Dame', 'Highest ranked karma master', 4),
(5, NULL, 'Wise Knight', 'Wise Dame', 'Highest ranked wisdom master', 5),
(6, NULL, 'Technical Knight', 'Technical Dame', 'Highest ranked technical master', 6),
(7, NULL, 'Spiritual Knight', 'Spiritual Dame', 'Highest ranked spirit master', 7),
(8, NULL, 'Enraged Knight', 'Enraged Dame', 'Highest ranked rage master', 8),
(9, NULL, 'Healthy Knight', 'Healthy Dame', 'Highest ranked player with the most VitH', 9),
(10, NULL, 'Armored Knight', 'Armored Dame', 'Highest ranked player with the most VitA', 10),
(11, NULL, 'Munitioned Knight', 'Munitioned Dame', 'Highest ranked player with the most munition', 11),
(12, NULL, 'Knight of the Rose Order', 'Dame of the Rose Order', 'Player with the most frags', 12),
(13, NULL, 'Knight of the Cheetah Order', 'Dame of the Cheetah Order', 'Player with the most frags per minute', 13),
(14, NULL, 'Knight of the Fire Order', 'Dame of the Fire Order', 'Player with the most sprees', 14),
(15, NULL, 'Knight of the Hellenic Order', 'Dame of the Hellenic Order', 'Player with the most spree wars', 15),
(16, NULL, 'Royal Order of Norb', 'Royal Order of Norb', 'Player with the most spree breaks', 16),
(17, NULL, 'Royal Order of Mother', 'Royal Order of Mother', 'Player with the most spree war breaks', 17),
(18, NULL, 'Knight of the Doppleganger', 'Dame of the Doppleganger', 'Player with the most 2-fers', 19),
(19, NULL, 'Berserker''s Fury', 'Berserker''s Fury', 'Player with the most 3-fers', 18),
(20, NULL, 'Lord of Sabre', 'Lady of Sabre', 'Highest ranked player with the most sabre', 20),
(21, NULL, 'Lord of Machinegun', 'Lady of Machinegun', 'Highest ranked player with the most machinegun', 21),
(22, NULL, 'Lord of Shotgun', 'Lady of Shotgun', 'Highest ranked player with the most shotgun', 22),
(23, NULL, 'Lord of Supershotgun', 'Lady of Supershotgun', 'Highest ranked player with the most supershotgun', 23),
(24, NULL, 'Lord of Chaingun', 'Lady of Chaingun', 'Highest ranked player with the most chaingun', 24),
(25, NULL, 'Lord of Hand Grenade', 'Lady of Hand Grenade', 'Highest ranked player with the most hand grenade', 25),
(26, NULL, 'Lord of Grenade Launcher', 'Lady of Grenade Launcher', 'Highest ranked player with the most grenade launcher', 26),
(27, NULL, 'Lord of Rocket Launcher', 'Lady of Rocket Launcher', 'Highest ranked player with the most rocket launcher', 27),
(28, NULL, 'Lord of Hyperblaster', 'Lady of Hyperblaster', 'Highest ranked player with the most hyperblaster', 28),
(29, NULL, 'Lord of Railgun', 'Lady of Railgun', 'Highest ranked player with the most railgun', 29),
(30, NULL, 'Lord of BFG', 'Lady of BFG', 'Highest ranked player with the most BFG', 30),
(31, NULL, 'Wizard of all Servers', 'Sorceress of all Servers', 'Player with the most teleports', 31),
(32, NULL, 'Space Invader', 'Space Invader', 'Player with the most telefrags', 32),
(33, NULL, 'Knight of the Fox Order', 'Dame of the Fox Order', 'Player with the highest kill ratio', 33),
(34, NULL, 'Tax Collector', 'Tax Collector', 'Player with the most credits', 34),
(35, NULL, 'Knight of the Trooper Order', 'Dame of the Trooper Order', 'Player with the least suicides', 35),
(36, NULL, 'Knight of the Putrid Odor', 'Dame of the Putrid Odor', 'Player with the most suicides', 36),
(37, NULL, 'Knight of the Dodo Order', 'Dame of the Dodo Order', 'Player with the lowest kill ratio', 37),
(38, NULL, 'Jester of all Servers', 'Jester of all Servers', 'Player with the most deaths', 38),
(39, NULL, 'Indecisive Knight', 'Indecisive Dame', 'Highest ranked player with no player mastery', 39),
(40, NULL, 'Irresolute Knight', 'Irresolute Dame', 'Highest ranked player with no weapon mastery', 40),
(41, NULL, 'Vacillating Knight', 'Vacillating Dame', 'Highest ranked player with all powers', 41);

--
-- Dumping data for table `runes`
--

INSERT INTO `runes` (`id`, `name`, `pickup_text`, `model_name`, `image_name`, `sound_name`, `mins`, `maxs`, `effects`, `renderfx`, `rarity`) VALUES
(1, 'Missile Battery', '', 'models/weapons/g_rocket/tris.md2', 'w_rlauncher', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(2, 'Developer''s Disc', '', 'models/items/keys/data_cd/tris.md2', 'k_datacd', 'misc/tele_up.wav', '-16,-16,-16', '16,16,16', 272, 7168, 0),
(3, 'Sword of Justice', '', 'models/objects/minelite/light1/tris.md2', 'w_blaster', NULL, '-16,-16,-28', '16,16,4', 256, 1024, 0.8),
(4, 'Sword of a Thousand Truths', '', 'models/objects/minelite/light1/tris.md2', 'w_blaster', NULL, '-16,-16,-28', '16,16,4', 256, 5120, 0.05),
(5, 'Wisdom Rune', '', 'models/items/keys/power/tris.md2', 'k_powercube', NULL, '-16,-16,-24', '16,16,8', 256, 6144, 0.7),
(6, 'Expack Rune', '', 'models/items/pack/tris.md2', 'i_pack', NULL, '-16,-16,-16', '16,16,16', 256, 2048, 0.6),
(7, 'Munition Rune', '', 'models/items/quaddama/tris.md2', 'p_quad', NULL, '-16,-16,-16', '16,16,16', 256, 6144, 0.7),
(8, 'Shotgun Relic', '', 'models/weapons/g_shotg/tris.md2', 'w_shotgun', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(9, 'Supershotgun Relic', '', 'models/weapons/g_shotg2/tris.md2', 'w_sshotgun', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(10, 'Flesh Shredder', '', 'models/weapons/g_chain/tris.md2', 'w_chaingun', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(11, 'Holy Handgrenade', '', 'models/items/ammo/grenades/medium/tris.md2', 'a_grenades', NULL, '-16,-16,-28', '16,16,4', 256, 1024, 0.8),
(12, 'Shock Bomber', '', 'models/weapons/g_launch/tris.md2', 'w_glauncher', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(13, 'Neural Disruptor', '', 'models/weapons/g_hyperb/tris.md2', 'w_hyperblaster', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(14, 'EMP Shockwave', '', 'models/weapons/g_rail/tris.md2', 'w_railgun', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(15, 'Plasma Regulator', '', 'models/weapons/g_bfg/tris.md2', 'w_bfg', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(16, 'Sniper Cannon', '', 'models/weapons/g_machn/tris.md2', 'w_machinegun', NULL, '-16,-16,-16', '16,16,16', 256, 1024, 0.8),
(17, 'Pugnus Caelorum', '', 'models/items/keys/pyramid/tris.md2', 'k_pyramid', 'misc/secret.wav', '-16,-16,-16', '16,16,16', 272, 7168, 0.02),
(18, 'Banishment Device', '', 'models/objects/grenade2/tris.md2', 'a_grenades', NULL, '-16,-16,-24', '16,16,8', 256, 2048, 0.6),
(19, 'Throw Rune', '', 'models/objects/gibs/arm/tris.md2', 'i_airstrike', NULL, '-16,-16,-20', '16,16,12', 256, 2048, 0.6),
(20, 'Bide Rune', '', 'models/items/keys/spinner/tris.md2', 'k_dataspin', NULL, '-16,-16,-28', '16,16,4', 256, 2048, 0.6),
(21, 'Vitality Rune', '', 'models/items/adrenal/tris.md2', 'p_adrenaline', NULL, '-16,-16,-16', '16,16,16', 256, 6144, 0.7),
(22, 'Dexterity Rune', '', 'models/items/silencer/tris.md2', 'p_silencer', NULL, '-16,-16,-16', '16,16,16', 256, 6144, 0.7),
(23, 'Strength Rune', '', 'models/items/pack/tris.md2', 'i_pack', NULL, '-16,-16,-16', '16,16,16', 256, 6144, 0.7),
(24, 'Karma Rune', '', 'models/items/mega_h/tris.md2', 'i_health', NULL, '-16,-16,-16', '16,16,16', 256, 6144, 0.7),
(25, 'Technical Rune', '', 'models/items/armor/shard/tris.md2', 'i_jacketarmor', NULL, '-16,-16,-16', '16,16,16', 256, 6144, 0.7),
(26, 'Spirit Rune', '', 'models/items/band/tris.md2', 'p_bandolier', NULL, '-16,-16,-16', '16,16,16', 256, 6144, 0.7),
(27, 'Rage Rune', '', 'models/objects/rocket/tris.md2', 'a_rockets', NULL, '-16,-16,-24', '16,16,8', 256, 6144, 0.7),
(28, 'Spiral Rune', '', 'models/items/keys/data_cd/tris.md2', 'k_datacd', NULL, '-16,-16,-16', '16,16,16', 256, 2048, 0.6),
(29, 'Jack of all Trades', 'It''s a little bit of everything!', 'models/items/ammo/mines/tris.md2', 'a_grenades', 'misc/comp_up.wav', '-16,-16,-28', '16,16,4', 272, 3072, 0.05),
(30, 'Herbal Remedy', 'For medicinal use only.', 'models/items/keys/target/tris.md2', 'i_airstrike', 'world/bubbl4.wav', '-16,-16,-16', '16,16,16', 272, 3072, 0.05),
(31, 'EM Blade', 'You''ve found the EdgeMaster, Whispering Death''s personal Sword.', 'models/objects/minelite/light1/tris.md2', 'w_blaster', 'world/steam1.wav', '-16,-16,-28', '16,16,4', 272, 3072, 0.05),
(32, 'Royal Bomber', 'You''ve obtained the Royal Bomber. Do Not Whine.', 'models/items/ammo/rockets/medium/tris.md2', 'a_rockets', 'makron/rail_up.wav', '-16,-16,-28', '16,16,4', 272, 3072, 0.05),
(33, 'Happy Handgrenade', 'You have the Happy Handgrenade, the ultimate weapon of mass destruction!', 'models/items/ammo/grenades/medium/tris.md2', 'a_grenades', 'weapons/Hgrena1b.wav', '-16,-16,-28', '16,16,4', 272, 3072, 0.05),
(34, 'Normal Resist Rune', '', 'models/items/armor/jacket/tris.md2', 'i_jacketarmor', NULL, '-16,-16,-16', '16,16,16', 256, 5120, 0.1),
(35, 'Energy Resist Rune', '', 'models/items/enviro/tris.md2', 'p_envirosuit', NULL, '-16,-16,-16', '16,16,16', 256, 5120, 0.15),
(36, 'Resist All Rune', '', 'models/items/invulner/tris.md2', 'p_invulnerability', NULL, '-16,-16,-16', '16,16,16', 256, 5120, 0.075),
(37, 'BirdmaN''s Fury', 'In memory of BirdmaN.', 'models/weapons/g_launch/tris.md2', 'w_glauncher', 'misc/spawn1.wav', '-16,-16,-16', '16,16,16', 272, 3072, 0.05),
(38, 'SoHnBOT''s Spirit', 'In Memory of SoHnBOT', 'models/weapons/g_rail/tris.md2', 'w_railgun', 'misc/power2.wav', '-16,-16,-16', '16,16,16', 272, 3072, 0.05);

--
-- Dumping data for table `runes_other`
--

INSERT INTO `runes_other` (`rune_id`, `tballs`, `tball_regen`, `tball_speed`, `normal_resist`, `energy_resist`) VALUES
(1, 0, 0, 0, 0, 0),
(2, 0, 1, 1, 0, 0),
(3, 0, 0, 0, 0, 0),
(4, 0, 1, 1, 0, 0),
(5, 0, 0, 0, 0, 0),
(6, 0, 0, 0, 0, 0),
(7, 0, 0, 0, 0, 0),
(8, 0, 0, 0, 0, 0),
(9, 0, 0, 0, 0, 0),
(10, 0, 0, 0, 0, 0),
(11, 0, 0, 0, 0, 0),
(12, 0, 0, 0, 0, 0),
(13, 0, 0, 0, 0, 0),
(14, 0, 0, 0, 0, 0),
(15, 0, 0, 0, 0, 0),
(16, 0, 0, 0, 0, 0),
(17, 0, 0, 0, 0, 0),
(18, 1, 1, 1, 0, 0),
(19, 0, 0, 0, 0, 0),
(20, 0, 0, 0, 0, 0),
(21, 0, 0, 0, 0, 0),
(22, 0, 0, 0, 0, 0),
(23, 0, 0, 0, 0, 0),
(24, 0, 0, 0, 0, 0),
(25, 0, 0, 0, 0, 0),
(26, 0, 0, 0, 0, 0),
(27, 0, 0, 0, 0, 0),
(28, 0, 0, 0, 0, 0),
(29, 0, 0, 0, 0, 0),
(30, 0, 0, 0, 0, 0),
(31, 0, 0, 0, 0, 0),
(32, 0, 0, 0, 0, 0),
(33, 0, 0, 0, 0, 0),
(34, 0, 0, 0, 1, 0),
(35, 0, 0, 0, 0, 1),
(36, 0, 0, 0, 1, 1),
(37, 0, 0, 0, 0, 0),
(38, 0, 0, 0, 0, 1);

--
-- Dumping data for table `runes_player`
--

INSERT INTO `runes_player` (`rune_id`, `dexterity`, `strength`, `karma`, `wisdom`, `technical`, `spirit`, `rage`, `vithealth`, `vitarmor`, `munition`) VALUES
(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(2, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10),
(3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(4, 2, 4, 0, 5, 0, 0, 3, 0, 0, 5),
(5, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0),
(6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
(8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(21, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0),
(22, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(23, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0),
(24, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0),
(25, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0),
(26, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0),
(27, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0),
(28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(29, 2, 2, 1, 1, 0, 0, 2, 2, 0, 2),
(30, 0, 0, 0, 0, 0, 0, 2, 1, 0, 0),
(31, 2, 0, 3, 0, 0, 0, 0, 1, 0, 0),
(32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(33, 0, 0, 0, 0, 0, 0, 0, 3, 0, 2),
(34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(37, 0, 0, 3, 4, 0, 0, 3, 0, 0, 2),
(38, 0, 0, 0, 0, 5, 3, 0, 0, 0, 0);

--
-- Dumping data for table `runes_power`
--

INSERT INTO `runes_power` (`rune_id`, `expack`, `spiral`, `bide`, `throw`, `antiweapon`) VALUES
(1, 0, 0, 0, 0, 0),
(2, 10, 10, 10, 10, 10),
(3, 0, 0, 0, 0, 0),
(4, 0, 0, 0, 2, 0),
(5, 0, 0, 0, 0, 0),
(6, 2, 0, 0, 0, 0),
(7, 0, 0, 0, 0, 0),
(8, 0, 0, 0, 0, 0),
(9, 0, 0, 0, 0, 0),
(10, 0, 0, 0, 0, 0),
(11, 0, 0, 0, 0, 0),
(12, 0, 0, 0, 0, 0),
(13, 0, 0, 0, 0, 0),
(14, 0, 0, 0, 0, 0),
(15, 0, 0, 0, 0, 0),
(16, 0, 0, 0, 0, 0),
(17, 0, 0, 0, 0, 0),
(18, 0, 0, 0, 0, 0),
(19, 0, 0, 0, 2, 0),
(20, 0, 0, 2, 0, 0),
(21, 0, 0, 0, 0, 0),
(22, 0, 0, 0, 0, 0),
(23, 0, 0, 0, 0, 0),
(24, 0, 0, 0, 0, 0),
(25, 0, 0, 0, 0, 3),
(26, 0, 0, 0, 0, 0),
(27, 0, 0, 0, 0, 0),
(28, 0, 2, 0, 0, 0),
(29, 0, 0, 0, 0, 0),
(30, 0, 1, 0, 0, 0),
(31, 0, 0, 0, 1, 0),
(32, 1, 0, 0, 0, 0),
(33, 2, 0, 2, 0, 0),
(34, 0, 0, 0, 0, 0),
(35, 0, 0, 0, 0, 0),
(36, 0, 0, 0, 0, 0),
(37, 0, 0, 0, 0, 0),
(38, 0, 0, 0, 0, 3);

--
-- Dumping data for table `runes_weapon`
--

INSERT INTO `runes_weapon` (`rune_id`, `sabre`, `shotgun`, `supershotgun`, `machinegun`, `chaingun`, `grenade`, `grenadelauncher`, `rocketlauncher`, `hyperblaster`, `railgun`, `bfg`) VALUES
(1, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0),
(2, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10),
(3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(4, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(8, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(9, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0),
(10, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0),
(11, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0),
(12, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0),
(13, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0),
(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0),
(15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4),
(16, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0),
(17, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4),
(18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(29, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0),
(30, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0),
(31, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(32, 0, 0, 0, 0, 0, 3, 4, 4, 0, 0, 0),
(33, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0),
(34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(37, 4, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0),
(38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0);

--
-- Dumping data for table `weapons`
--

INSERT INTO `weapons` (`id`, `name`) VALUES
(0, 'None'),
(1, 'Sabre'),
(2, 'Shotgun'),
(3, 'Super Shotgun'),
(4, 'Machinegun'),
(5, 'Chaingun'),
(6, 'Hand Grenade'),
(7, 'Grenade Launcher'),
(8, 'Rocket Launcher'),
(9, 'Hyperblaster'),
(10, 'Railgun'),
(11, 'BFG10k'),
(20, 'Expack'),
(21, 'Spiral'),
(22, 'Bide'),
(23, 'Throw');


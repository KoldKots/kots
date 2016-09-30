CREATE OR REPLACE VIEW `v_possible_kings` AS

select	a.id AS id, a.name AS name, a.pass AS pass, a.level AS level, a.exp AS exp, a.resist AS resist, a.cubes AS cubes,
		a.credits AS credits, a.rune_id AS rune_id, a.datecreated AS datecreated, a.lastsaved AS lastsaved,
		a.respawn_weapon AS respawn_weapon, a.respawns AS respawns,
		a.cursed AS cursed, a.loggedin AS loggedin, a.isadmin AS isadmin, a.title AS title
from	characters a inner join
		characters_stats b on a.id = b.character_id
where	a.isboss = 0 and a.cursed = 0 and a.level >= 7 and a.lastsaved >= (now() - interval 2 week) and b.timeplayed > (60 * 60 * 2);
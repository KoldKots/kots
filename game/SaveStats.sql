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

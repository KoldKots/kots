CREATE PROCEDURE `LoadMuteList`()
BEGIN

        select a.ip_address, a.name, a.character_id
        from mutelist a;

END

CREATE PROCEDURE `LoadIpBans`()
BEGIN

        select ip_address from ipbans;

END

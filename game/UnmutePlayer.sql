CREATE PROCEDURE `UnmutePlayer`(
        ip_address varchar(15)
)
BEGIN

        delete from mutelist where mutelist.ip_address = ip_address;

END

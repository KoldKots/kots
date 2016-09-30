CREATE PROCEDURE `LogInfo`(
        info varchar(2048)
)
BEGIN

	insert into `log` (`date`, `info`)
	values ( NOW(), info );

END

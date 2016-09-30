
Getting Started
----------------------------------------------------------
Licensing:
I have no licensing restrictions on my code other than the GPL which is already in place thanks to the Quake 2 source code. You are free to do whatever you like with it as long as it adheres to the licenses already in place.

Environment Variables:
%Q2DIR% - Set this to your Quake 2 folder (ie C:\Quake2\)

Dependencies:
KOTS2007 makes use of libcurl, Pthreads-win32, and libmysql. In order to run the mod you will need these libraries in your Quake2 root folder. For windows this simply means copying the files to the folder. If you are using the Visual Studio projects then this will be done for you if you use have the %Q2DIR% environment variable set properly.

Database (Character Server) setup:
You will need a working MySQL server in order to join the game. You can use an existing server or install it on your computer. There are free servers out there you can use, but that is beyond the scope of this brief guide. Once you have a server you can import the database structure found in kots_db.sql.

Once your MySQL server is setup you will need to ensure your in-game server credentials match. You can do this by changing the default values in the header files or use sv commands to set them. The sv commands you will need to be concerned with are:
sv user root
sv pass rootPassword
sv dbname kots
sv dbhostname localhost

Debugging:
If using the Visual Studio projects then they have already been set up to copy the files to the necessary locations. All you have to do is specify the debug command and working directory. A basic example (assuming you're using the environment variable) is:
Command: $(Q2DIR)\r1q2.exe
Command Arguments: +set game kots2007dev +set deathmatch 1 +set public 0 +map q2dm1
Working Directory: $(Q2DIR)


Commands
----------------------------------------------------------

cvars:
maplist
Usage: maplist "maplist.txt"
This command is used to load a maplist. The path is relative to your
kots2007 mod folder. If you specify an empty string then this will
clear the maplist*.

maplist
Usage: maprotation <0/1>
This command sets the way the map rotation works. The default value is
1 (random rotation). A 0 specifies that they should be rotated through
in the order they are in your maplist. A 1 indicates that they should
be rotated through in a random order.

motd
Usage: motd "motd.txt"
This command sets the filename that the message of the day should be
read from. The path is relative to your kots2007 mod folder.

prevmaps
Usage: prevmaps <1 or higher>
This command is used to determine how many previous maps should be
kept track of. Maps previously played are not allowed to be voted on
and they will not be selected when picking the next map in a random
rotation. The default value is 5 and the minimum value is 1. I
recommend keeping this at around 5 or higher to encourage playing
multiple maps rather than letting people constantly vote for the same
maps.

public
Usage: public <0/1>
In addition to it's normal usage in Quake 2 this will also determine
whether or not the server should be visible via the kots_servers
command (and eventually the servers page). The default value is 1.
This should not be used except in the case of testing server settings,
etc. All servers should be publicly visible and keep in mind that the
server information still gets saved so I will still know if a server
is being run in private mode.

Fixed cvar settings:
The following cvar settings are enforced by the game and cannot be changed.

coop: 0
deathmatch: 1
dmflags: 532
fraglimit: 50
skill: 5
sv_cheats: 0
timelimit: 30


Server Commands
sv user
Usage: sv user "username"
This will set the username used to connect to the character server
with. The default value is an empty string and this will cause it to
use a hard-coded value. This should only be used on servers with
dynamic IP addresses in order to get around IP address restrictions
used by the character server.

sv pass
Usage: sv pass "password"
This will set the password used to connect to the character server
with. The default value is an empty string and this will cause it to
use a hard-coded value. This should be used in conjunction with sv
user.

sv dbname
Usage: sv dbname "name"
This will set the name of the database to use on the MySQL server. The
default value is an empty string and this will cause it to use a
hard-coded value of "kots."

sv dbhostname
Usage: sv dbhostname "hostname"
This will set the hostname of the database to use on the MySQL server.
The default value is an empty string and this will cause it to use a
hard-coded value of "localhost."

sv nologin
Usage: sv nologin
This forces all players to logout of their characters and sends them
into spectator mode and prevents everyone from logging in. This can be
used when your changing server settings or need to restart the server
as a way to ensure characters are logged out and saved properly. Note
that shutting down the server should logout of characters properly,
but this can be used as an extra precaution.

sv allowlogin
Usage: sv allowlogin
This will allow characters to login if login has been disabled by the server.

sv maplist
Usage: sv maplist "maplist.txt" [<0/1>]
This works the same as using the maplist cvar. You can use the
optional parameter for rotation to set the maprotation at the same
time.

sv maplist next
Usage: sv maplist next
This forces the current map to end and automatically go to the next
map in the rotation.

sv maplist clear
Usage: sv maplist clear
This will clear the current maplist.

sv maplist show
Usage: sv maplist show
This will show all the maps in the current maplist.

sv maplist goto
Usage: sv maplist goto <mapname>
This will force the current map to end and go to the specified map.

sv maplist rotation
Usage: sv maplist rotation <0/1>
This works the same as the maprotation cvar.

sv maplist reload*
Usage: sv maplist reload
This will reload the current maplist after changes have been made.
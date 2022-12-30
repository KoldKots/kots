#include "kots_motd.h"
#include <stdio.h>


void Kots_PrintMotd(edict_t *ent)
{
    static char motd[501];
    static char line[81];
    int line_length;
    int length = 0;
    FILE *fp;

    //reset the motd
    motd[0] = '\0';

    fp = fopen(va("%s/%s", gamedir->string, kots_motd->string), "r");
    if (fp)
    {
        //read the motd 
        while (fgets(line, sizeof(line), fp))
        {
            strncat(motd, line, sizeof(motd) - length);
            line_length = strlen(line);
            length += line_length;

            if (length >= sizeof(motd))
            {
                motd[sizeof(motd) - 1] = '\0';
                gi.dprintf("ERROR: Message of the day is too long and had to be truncated.\n");
                break;
            }
            else if (line_length == (sizeof(line) - 1))
            {
                //force line breaks if necessary 
                strcat(motd, "\n");
                length++;
            }

        }

        //if the message of the day was read print it out
        if (motd[0] != '\0')
            gi.centerprintf(ent, "%s\n", motd);

        fclose(fp);
    }
}

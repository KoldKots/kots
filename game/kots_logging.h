#include "kots_character.h"

#ifndef KOTS_LOGGING_H
#define KOTS_LOGGING_H

void Kots_GetPlayers(int *players, int *specs);
void Kots_GetUniquePlayers(int *players, int *specs, int *unique);
void Kots_LogHighScores();
void Kots_LogLowScores();
void Kots_LogHighRatio();
void Kots_LogSpreeWar();
void Kots_LogOnDisconnect();
struct loginfo_s *Kots_ServerBuildLogInfo(char *reason);

#endif


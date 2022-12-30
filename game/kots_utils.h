#ifndef KOTS_UTILS_H
#define KOTS_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "g_local.h"

#ifdef WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

void Kots_SlowPrint(edict_t *ent, const char *format, ...);
void Kots_ProcessSlowPrintQueue();
void Kots_ClearSlowPrintQueue();
void Kots_ClearSlowPrints(edict_t *ent);
void Kots_CenterPrintAll(char *format, ...);

void Kots_SoundIndexWrapperDefault(int index);
int Kots_SoundIndexWrapper(char *name);
void Kots_SoundIndexWrapperInit();

int Kots_RandDivide(int i, int j);
int Kots_Round(float i);
int Kots_RandRound(float i);
int Kots_RandMultiply(int i, double j);


void Kots_PlaySound(edict_t *ent, int channel, int soundIndex, float volume, float attenuation, float timeofs, qboolean unicast);
void Kots_UnicastSound(edict_t *ent, int soundIndex, float volume);
void Kots_ClearSoundQueue();
void Kots_DeleteSoundQueue();
void Kots_QueueSound(edict_t *ent, int sound_index, float volume, float play_time);
void Kots_ClearQueuedSounds(edict_t *ent);
void Kots_ProcessQueuedSounds();


//These methods are curteousy of disruptor taken from KOTS Orbit
qboolean loc_CanSee (edict_t *targ, edict_t *inflictor);

int Kots_snprintf(char *out, size_t size, const char *in, ...);
int Kots_vsnprintf(char *out, size_t size, const char *in, va_list args);
char *Kots_strncpy(char *out, const char *in, size_t size);

vec3_t *CenterEdict(edict_t *ent);
void Kots_strtov(char *value, vec3_t *out);
qboolean Kots_ValidateString(char *value, char *valid_chars);
void Kots_WrapText(char *input, char *output, int lineWidth, int length);

#endif

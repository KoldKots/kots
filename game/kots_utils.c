#include "kots_utils.h"
#include "kots_array.h"

#define MASK_VOLUME			1
#define MASK_ATTENUATION	2
#define MASK_POSITION		4
#define MASK_ENTITY_CHANNEL	8
#define MASK_TIMEOFS		16
#define svc_sound			9


#define MAX_SLOWPRINTS_PERFRAME		2
#define MAX_SLOWPRINTS				512
#define MAX_MESSAGESIZE				512

static char *slowprints[MAX_CLIENTS][MAX_SLOWPRINTS];
static int slowprint_end[MAX_CLIENTS];
static int slowprint_start[MAX_CLIENTS];

void Kots_SlowPrint(edict_t *ent, const char *format, ...)
{
	char message[MAX_MESSAGESIZE];
	va_list args;
	int index = ent - g_edicts - 1;

	if (ent)
	{
		if (index >= (int)maxclients->value || index < 0)
		{
			gi.dprintf("ERROR: Attempted to slow print to non-client.\n");
			return;
		}

		if ((slowprint_end[index] + 1 == slowprint_start[index])
			|| (slowprint_end[index] == MAX_SLOWPRINTS - 1 && slowprint_start[index] == 0))
		{
			gi.dprintf("ERROR: Attempted to add too many slow prints to the queue for client '%s'.\n", ent->client->pers.netname);
			return;
		}
	}

	//format the message
	va_start(args, format);
	Kots_vsnprintf(message, MAX_MESSAGESIZE, format, args);
	va_end(args);

	if (ent)
	{
		//copy the string into the printing queue
		slowprints[index][slowprint_end[index]] = G_CopyString(message);

		//increment our position in the queue
		if (++slowprint_end[index] >= MAX_SLOWPRINTS)
			slowprint_end[index] = 0;
	}
	else
		gi.cprintf(ent, PRINT_HIGH, message);
}

//print the messages for this frame
void Kots_ProcessSlowPrintQueue()
{
	int i;
	int index;
	edict_t *ent = g_edicts + 1;

	for (index = 0; index < (int)maxclients->value; index++, ent++)
	{
		if (!ent->inuse || !ent->client)
			continue;

		for (i = 0; i < MAX_SLOWPRINTS_PERFRAME; i++)
		{
			if (slowprint_start[index] == slowprint_end[index])
				break;

			gi.cprintf(ent, PRINT_HIGH, slowprints[index][slowprint_start[index]]);

			//free the tag and set it to null
			gi.TagFree(slowprints[index][slowprint_start[index]]);
			slowprints[index][slowprint_start[index]] = NULL;

			if (++slowprint_start[index] >= MAX_SLOWPRINTS)
				slowprint_start[index] = 0;
		}
	}
}

//no need to free the items because tagged malloc will do that for us
void Kots_ClearSlowPrintQueue()
{
	int i, j;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		for (j = 0; j < MAX_SLOWPRINTS; j++)
			slowprints[i][j] = NULL;

		slowprint_end[i] = 0;
		slowprint_start[i] = 0;
	}
}

//call this when a client disconnects
void Kots_ClearSlowPrints(edict_t *ent)
{
	int i;
	int index = ent - g_edicts - 1;

	for (i = 0; i < MAX_SLOWPRINTS; i++)
		slowprints[index][i] = NULL;

	slowprint_end[index] = 0;
	slowprint_start[index] = 0;
}


static int (*kots_soundindex)(char *name);
static char *KOTS_SOUNDS[MAX_SOUNDS];
static int KOTS_SOUNDINDEXES[MAX_SOUNDS];
static int KOTS_SOUNDCOUNT = 0;

int Kots_SoundIndexWrapper(char *name)
{
	int i;
	int index;

	if (!name || !*name)
		return 0;

	//see if it's already cached
	for (i = 0; i < KOTS_SOUNDCOUNT; i++)
		if (Q_stricmp(name, KOTS_SOUNDS[i]) == 0)
			return KOTS_SOUNDINDEXES[i];

	//we tried to go over the max, print a debug message and return no sound
	if (KOTS_SOUNDCOUNT + 1 >= MAX_SOUNDS)
	{
		gi.dprintf("Tried to go over max sound count with sound '%s'.\n", name);
		return 0;
	}

	//try to index the sound (cross your fingers)
	index = kots_soundindex(name);

	//if the sound was indexed keep track of it
	if (index != 0)
	{
		KOTS_SOUNDS[KOTS_SOUNDCOUNT] = G_CopyString(name);
		KOTS_SOUNDINDEXES[KOTS_SOUNDCOUNT] = index;
		KOTS_SOUNDCOUNT++;
	}

	//return the index
	return index;
}

void Kots_SoundIndexWrapperInit()
{
	int i;

	//reset all indexed sounds
	KOTS_SOUNDCOUNT = 0;
	for (i = 0; i < MAX_SOUNDS; i++)
	{
		KOTS_SOUNDS[i] = 0;
		KOTS_SOUNDINDEXES[i] = 0;
	}

	//ensure we have the function wrapped
	if (gi.soundindex != Kots_SoundIndexWrapper)
	{
		kots_soundindex = gi.soundindex;
		gi.soundindex = Kots_SoundIndexWrapper;
	}
}

int Kots_RandDivide(int i, int j)
{
	return Kots_Round(i / (float)j);
}

int Kots_RandMultiply(int i, double j)
{
	return Kots_Round(i * j);
}

int Kots_Round(float i)
{
	int rounded = (int)i;
	float remainder = (i - rounded);

	if (remainder != 0 && Kots_RandRound(remainder))
		return rounded + 1;
	else
		return rounded;
}

//Expects a float value representing a percent
//The percentage is the likelihood that it should be rounded up
//Returns 1 if the number should be rounded up or 0 for down
int Kots_RandRound(float i)
{
	if (i < 1.0)
	{
		float rnd = random();

		if (i >= rnd)
			return 1;
		else
			return 0;
	}
	else //100% or higher always yields 1
		return 1;
}


void Kots_CenterPrintAll(char *format, ...)
{
	char message[1024];
	va_list args;
	int i;
	edict_t *ent = &g_edicts[1];

	//format the message
	va_start(args, format);
	Kots_vsnprintf(message, 1024, format, args);
	va_end(args);

	//print the message to all clients
	for (i = 0; i < maxclients->value; i++, ent++)
		if (ent->inuse)
			gi.centerprintf(ent, message);
}

/*
KOTS Orbit methods
Thanks Disruptor
*/
static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

	// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) 
	{
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}
	return false;
}

int Kots_snprintf(char *out, size_t size, const char *in, ...)
{
	int length;
	va_list args;

	va_start(args, in);
	length = Kots_vsnprintf(out, size, in, args);
	va_end(args);

	return length;
}

int Kots_vsnprintf(char *out, size_t size, const char *in, va_list args)
{
	int length = vsnprintf(out, size, in, args);

	//ensure we append the null terminator for MS compilers
	if (size > 0 && length >= size)
		out[size - 1] = '\0';

	return length;
}

char *Kots_strncpy(char *out, const char *in, size_t size)
{
	//don't error out on null strings just use an empty string
	if (!in)
	{
		*out = '\0';
		return out;
	}

	strncpy(out, in, size);

	//ensure the string is null terminated
	if (size > 0)
		out[size - 1] = '\0';

	return out;
}

vec3_t *CenterEdict(edict_t *ent)
{
	static vec3_t center;

	VectorCopy(ent->s.origin, center);
	center[0] += (ent->mins[0] / 2) + (ent->maxs[0] / 2);
	center[1] += (ent->mins[1] / 2) + (ent->maxs[1] / 2);
	center[2] += (ent->mins[2] / 2) + (ent->maxs[2] / 2);

	return &center;
}

void Kots_strtov(char *value, vec3_t *out)
{
	VectorClear((*out));
	sscanf(value, "%f,%f,%f", &(*out)[0], &(*out)[1], &(*out)[2]);
}

qboolean Kots_ValidateChar(char *value, char *valid_chars)
{
	char *v;
	for (v = valid_chars; *v != '\0'; v++)
	{
		//if this value is valid then return true
		if (*value == *v)
			return true;
	}

	return false;
}

qboolean Kots_ValidateString(char *value, char *valid_chars)
{
	char *c;

	for (c = value; *c != '\0'; c++)
	{
		//if the char was not valid then return false
		if (!Kots_ValidateChar(c, valid_chars))
			return false;
	}

	//all values passed so return true
	return true;
}

void Kots_UnicastSound(edict_t *ent, int soundIndex, float volume)
{
	int mask = MASK_ENTITY_CHANNEL;

	if (volume != 1.0)
		mask |= MASK_VOLUME;

	gi.WriteByte(svc_sound);
	gi.WriteByte((byte)mask);
	gi.WriteByte((byte)soundIndex);

	if (mask & MASK_VOLUME)
		gi.WriteByte((byte)(volume * 255));

	gi.WriteShort(((ent - g_edicts - 1) << 3) + CHAN_NO_PHS_ADD);
	gi.unicast(ent, true);
}

void Kots_PlaySound(edict_t *ent, int channel, int soundIndex, float volume, float attenuation, float timeofs, qboolean unicast)
{
	if (unicast)
		Kots_UnicastSound(ent, soundIndex, volume);
	else
		gi.sound(ent, channel, soundIndex, volume, attenuation, timeofs);
}

typedef struct queuedsound_s
{
	edict_t	*ent;
	int		sound_index;
	float	volume;
	float	play_time;
} queuedsound_t;

static array_t *kots_soundqueue = NULL;
void Kots_ClearSoundQueue()
{
	//if the sound queue doesn't exist then create it
	if (kots_soundqueue == NULL)
		kots_soundqueue = Array_Create(NULL, MAX_CLIENTS, MAX_CLIENTS);

	//Remove all elements from the array
	Array_DeleteAll(kots_soundqueue);
}

void Kots_DeleteSoundQueue()
{
	if (kots_soundqueue)
	{
		Array_Delete(kots_soundqueue);
		kots_soundqueue = NULL;
	}
}

void Kots_QueueSound(edict_t *ent, int sound_index, float volume, float play_time)
{
	queuedsound_t *queued = malloc(sizeof(*queued));

	queued->ent = ent;
	queued->sound_index = sound_index;
	queued->volume = volume;
	queued->play_time = play_time;

	Array_PushBack(kots_soundqueue, queued);
}

void Kots_ClearQueuedSounds(edict_t *ent)
{
	unsigned long i;
	queuedsound_t *queued = NULL;

	for (i = 0; i < kots_soundqueue->length; i++)
	{
		queued = Array_GetValueAt(kots_soundqueue, i);

		if (queued->ent == ent)
		{
			Array_DeleteAt(kots_soundqueue, i);
			i--;
		}
	}
}

void Kots_ProcessQueuedSounds()
{
	unsigned long i;
	queuedsound_t *queued = NULL;

	for (i = 0; i < kots_soundqueue->length; i++)
	{
		queued = Array_GetValueAt(kots_soundqueue, i);

		if (queued->play_time <= level.time)
		{
			if (queued->ent && queued->ent->inuse)
				Kots_UnicastSound(queued->ent, queued->sound_index, queued->volume);

			Array_DeleteAt(kots_soundqueue, i);
			i--;
		}
	}
}

void Kots_WrapText(char *input, char *output, int lineWidth, int length)
{
	static char *breaks = " .,-/:;])";
	char *p = input;
	int start = 0, end = 0;
	int j;

	while (*p != '\0' && end < length)
	{
		//go to the end of the line
		end = start + lineWidth;
		if (end >= length || *(input + end) == '\0')
		{
			//determine how many characters this line was
			end = length - (end - lineWidth) + 1;
			Kots_strncpy(output, p, end);
			output += end;

			//pad the end with trailing spaces
			for (; end < lineWidth; end++)
			{
				*(++output) = ' ';
			}

			*(++output) = '\n';
			break;
		}

		//move the pointer to the end of the line
		p = (input + end);

		//if the next character is a space just remove the space
		if (*(p + 1) == ' ')
		{
			Kots_strncpy(output, input + start, lineWidth + 1);
			output += lineWidth + 1;

			//append the newline and a null character
			*output = '\n';
			*(++output) = '\0';

			//increment the end to skip the space
			end++;

			//if we've reached the end of the string break now
			if (end >= length)
				break;
		}

		//otherwise search for the first breakable character on the line
		else
		{
			for (j = 0; j < length; j++, p--)
			{
				if (Kots_ValidateChar(p, breaks))
				{
					Kots_strncpy(output, input + start, lineWidth - j + 2);
					output += lineWidth - j;

					//move the end back to where we broke the line
					end -= j;

					//pad the end with trailing spaces
					for (; j > 0; j--)
					{
						*(++output) = ' ';
					}

					//append the newline and a null character
					*(++output) = '\n';
					*(++output) = '\0';
					break;
				}
			}
		}

		//position everything to the start of the next line
		start = end + 1;
		p = input + start;
	}
}

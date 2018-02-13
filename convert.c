#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "id_heads.h"

#define PC_PIT_RATE 1193182
//#define OPL_SAMPLE_RATE 49716
//#define OPL_SAMPLE_RATE 22050
#define OPL_SAMPLE_RATE 11025
//#define OPL_SAMPLE_RATE 8000
//#define OPL_SAMPLE_RATE 7000
//typedef int16_t BE_ST_SndSample_T;
//typedef int8_t BE_ST_SndSample_T;
unsigned int g_sampleRate = OPL_SAMPLE_RATE;
uint32_t g_lastBlockAvg;
//#define BLOCK_AVG_CUTOFF 10000
//#define BLOCK_AVG_CUTOFF 100*3
#ifdef GAMEVER_RESTORATION_AOG
#define FILE_EXT "bs6"
#elif defined(GAMEVER_RESTORATION_VSI)
#define FILE_EXT "vsi"
#endif
#define CONVERT_MUSIC

static void (*g_sdlCallbackSDFuncPtr)(void) = 0;

// sound related
static uint32_t g_sdlSampleOffsetInSound, g_sdlSamplePerPart;

// refkeen compatibility
#define BE_Cross_LogMessage(x, fmt, ...) fprintf(stderr, (fmt), __VA_ARGS__); 
#define BE_Cross_TypedMin(T, x, y) ({T _x = (x), _y = (y); (_x < _y) ? _x : _y;})


// be_st_sdl_audio_timer.c

void BE_ST_ShortSleep(void)
{
}

void BE_ST_SetTimer(uint16_t speed)
{
	g_sdlSamplePerPart = (int32_t)speed * g_sampleRate / PC_PIT_RATE;
}

void BE_ST_StartAudioSDService(void (*funcPtr)(void))
{
	g_sdlCallbackSDFuncPtr = funcPtr;
}

void BE_ST_StopAudioSDService(void)
{
	g_sdlCallbackSDFuncPtr = 0;
}

int BE_ST_InitAudio(void) { return 0; }
void BE_ST_ShutdownAudio(void) {}
void BE_ST_PlayDigiSound(uint8_t *data, int32_t length) {}
void BE_ST_SetDigiSoundVol(uint32_t volume, uint32_t pan) {}
void BE_ST_StopDigiSound(void) {}
void BE_ST_PlaySound(int sound){}
void BE_ST_StopSound(void){}
void BEL_ST_LoadDigiSounds(void){}
void BEL_ST_FreeDigiSounds(void){}
void BE_ST_PlayMusic(int music){}
void BE_ST_MusicOn(void){}
void BE_ST_MusicOff(void){}
void BE_ST_ShutMusic(void){}

/*******************************************************************************
OPL emulation, powered by dbopl from DOSBox and using bits of code from Wolf4SDL
*******************************************************************************/

#include "opl/dbopl.h"

#define OPL_NUM_OF_SAMPLES 2048 // About 40ms of OPL sound data
//#define OPL_SAMPLE_RATE 49716
// Use this if the audio subsystem is disabled for most (we want a BYTES rate of 1000Hz, same units as used in values returned by SDL_GetTicks())
#define NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM 1000

static int8_t g_sdlALOutSamples[OPL_NUM_OF_SAMPLES];
static uint32_t g_sdlALOutSamplesEnd = 0;

Chip oplChip;

static inline void YM3812Init(int numChips, int clock, int rate)
{
	DBOPL_InitTables();
	Chip__Chip(&oplChip);
	Chip__Setup(&oplChip, rate);
}

static inline void YM3812Write(Chip *which, Bit32u reg, Bit8u val)
{
	Chip__WriteReg(which, reg, val);
}

static inline void YM3812UpdateOne(Chip *which, int8_t *stream, int length)
{
	Bit32s buffer[OPL_NUM_OF_SAMPLES * 2];
	int i;

	// length should be at least the max. samplesPerMusicTick
	// in Catacomb 3-D and Keen 4-6, which is param_samplerate / 700.
	// So 512 is sufficient for a sample rate of 358.4 kHz.
	if(length > OPL_NUM_OF_SAMPLES)
		length = OPL_NUM_OF_SAMPLES;

	Chip__GenerateBlock2(which, length, buffer);

	g_lastBlockAvg = 0;
	// GenerateBlock2 generates a number of "length" 32-bit mono samples
	// so we only need to convert them to 16-bit mono samples
	for(i = 0; i < length; i++)
	{
		// Scale volume
#if 1
		int16_t sample = buffer[i];
		sample >>= 5;
		//if (sample > 127 || sample < -128) printf("%s clipping warning %d %d\n", __FUNCTION__, sample, length);
		if(sample > 127) sample = 127;
		else if(sample < -128) sample = -128;
		stream[i] = sample;
#else
		Bit32s sample = 2*buffer[i];
		if(sample > 16383) sample = 16383;
		else if(sample < -16384) sample = -16384;
		//stream[i] = sample >> 6;
		stream[i] = sample >> 7; // 6 breaks track02
#endif
		//printf("stream[%d] = %d\n", i, stream[i]);
		g_lastBlockAvg += (uint32_t)abs(stream[i]);
	}
}

void BE_ST_ALOut(uint8_t reg,uint8_t val)
{
	// FIXME: The original code for alOut adds 6 reads of the
	// register port after writing to it (3.3 microseconds), and
	// then 35 more reads of register port after writing to the
	// data port (23 microseconds).
	//
	// It is apparently important for a portion of the fuse
	// breakage sound at the least. For now a hack is implied.
	YM3812Write(&oplChip, reg, val);

	// Hack comes with a "magic number"
	// that appears to make it work better
	unsigned int length = g_sampleRate / 10000;

	if (length > OPL_NUM_OF_SAMPLES - g_sdlALOutSamplesEnd)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BE_ST_ALOut overflow, want %u, have %u\n", length, OPL_NUM_OF_SAMPLES - g_sdlALOutSamplesEnd); // FIXME - Other thread
		length = OPL_NUM_OF_SAMPLES - g_sdlALOutSamplesEnd;
	}
	if (length)
	{
		YM3812UpdateOne(&oplChip, &g_sdlALOutSamples[g_sdlALOutSamplesEnd], length);
		g_sdlALOutSamplesEnd += length;
	}
}

static void BEL_ST_Simple_CallBack(void *unused, uint8_t *stream, int len)
{
	int8_t *currSamplePtr = (int8_t *)stream;
	uint32_t currNumOfSamples;
	memset(stream, 0, len);

	while (len)
	{
		if (!g_sdlSampleOffsetInSound)
		{
			// FUNCTION VARIABLE (We should use this and we want to kind-of separate what we have here from original code.)
			g_sdlCallbackSDFuncPtr();
		}
		// Now generate sound
		currNumOfSamples = BE_Cross_TypedMin(uint32_t, len/sizeof(int8_t), g_sdlSamplePerPart-g_sdlSampleOffsetInSound);
		// PC Speaker
		/*if (g_sdlPCSpeakerOn)
			PCSpeakerUpdateOne(currSamplePtr, currNumOfSamples);*/
		/*** AdLib (including hack for alOut delays) ***/
		//if (g_sdlEmulatedOPLChipReady)
		{
			// We may have pending AL data ready, but probably less than required
			// for filling the stream buffer, so generate some silence.
			//
			// Make sure we don't overthrow the AL buffer, though.
			uint32_t targetALSamples = currNumOfSamples;
			if (targetALSamples > OPL_NUM_OF_SAMPLES)
			{
				BE_Cross_LogMessage(BE_LOG_MSG_WARNING, "BEL_ST_Simple_CallBack AL overflow, want %u, have %u\n", targetALSamples, (unsigned int)OPL_NUM_OF_SAMPLES); // FIXME - Other thread
				targetALSamples = OPL_NUM_OF_SAMPLES;
			}
			// TODO Output overflow warning if there's any
			if (targetALSamples > g_sdlALOutSamplesEnd)
			{
				YM3812UpdateOne(&oplChip, &g_sdlALOutSamples[g_sdlALOutSamplesEnd], targetALSamples - g_sdlALOutSamplesEnd);
				g_sdlALOutSamplesEnd = targetALSamples;
			}
			// Mix with AL data
			for (uint32_t i = 0; i < targetALSamples; ++i)
				//currSamplePtr[i] = (currSamplePtr[i] + g_sdlALOutSamples[i]) / 2;
				currSamplePtr[i] = g_sdlALOutSamples[i];
			// Move pending AL data
			if (targetALSamples < g_sdlALOutSamplesEnd)
				memmove(g_sdlALOutSamples, &g_sdlALOutSamples[targetALSamples], sizeof(int8_t) * (g_sdlALOutSamplesEnd - targetALSamples));
			g_sdlALOutSamplesEnd -= targetALSamples;
		}
		// We're done for now
		currSamplePtr += currNumOfSamples;
		g_sdlSampleOffsetInSound += currNumOfSamples;
		len -= sizeof(int8_t)*currNumOfSamples;
		// End of part?
		if (g_sdlSampleOffsetInSound >= g_sdlSamplePerPart)
		{
			g_sdlSampleOffsetInSound = 0;
		}
	}
}

#include "id_heads.h"
#define id0_char_t char
//#define id0_word_t word
#define id0_unsigned_long_t uint32_t
#define id0_boolean_t boolean
#define id0_int_t int16_t
#define id0_seg

typedef struct
{
	uint32_t magic; /* magic number */
	uint32_t hdr_size; /* size of this header */ 
	uint32_t data_size; /* length of data (optional) */ 
	uint32_t encoding; /* data encoding format */
	uint32_t sample_rate; /* samples per second */
	uint32_t channels; /* number of interleaved channels */
} Audio_filehdr;

#define AUDIO_FILE_MAGIC ((uint32_t)0x2e736e64) /* Define the magic number */  
#define AUDIO_FILE_ENCODING_LINEAR_8 (2) /* 8-bit linear PCM */

uint8_t buff[NUM_OF_BYTES_FOR_SOUND_CALLBACK_WITH_DISABLED_SUBSYSTEM];

static size_t write_header(FILE *fd)
{
	Audio_filehdr header;
	header.magic = SWAP32BE(AUDIO_FILE_MAGIC);
	header.hdr_size = SWAP32BE(sizeof(header));
	header.data_size = 0xffffffff;
	header.encoding = SWAP32BE(AUDIO_FILE_ENCODING_LINEAR_8);
	header.sample_rate = SWAP32BE(OPL_SAMPLE_RATE);
	header.channels = SWAP32BE((uint32_t)1);
	return fwrite(&header, sizeof(header), 1, fd);
}

static void store_size(FILE *fd)
{return;
	uint32_t size = ftell(fd);
	size = SWAP32BE(size);
	fseek(fd, offsetof(Audio_filehdr, data_size), SEEK_SET);
	fwrite(&size, sizeof(size), 1, fd);
	// TODO seek back?
}

// converter entry point
int _argc;
char **_argv;
int main(int argc, char **argv)
{
	char filename[256];
	FILE *fd;

	_argc = argc;
	_argv = argv;

	if (argc >= 2)
	{
		g_sampleRate = atoi(argv[1]);
	}
	printf("Conversion init, sample rate: %d Hz\n", g_sampleRate);
	YM3812Init(1, 3579545, g_sampleRate);

	printf("Conversion startup...\n");
	CheckForEpisodes();
	PM_Startup ();
	SD_Startup ();
	CA_Startup ();

	printf("Loading sounds...\n");
	SD_SetSoundMode(sdm_AdLib);
	CA_LoadAllSounds();

	//printf("starting conversion\n");

	mkdir("adlib", 0755); 

	// play them
	for (int i = 0; i < LASTSOUND; i++)	//int i = 0;
	{
		snprintf(filename, sizeof(filename), "adlib/sound%02d.au", i);

		fd = fopen(filename, "r");
		if (fd)
		{
			// already exists, skip this one
			fclose(fd);
			continue;
		}

		printf("Converting sound %d/%d", i, LASTSOUND-1);
		SD_PlaySound(i);

		fd = fopen(filename, "w");

		/*
		Audio_filehdr header;
		header.magic = SWAP32BE(AUDIO_FILE_MAGIC);
		header.hdr_size = SWAP32BE(sizeof(header));
		header.data_size = 0xffffffff;
		header.encoding = SWAP32BE(AUDIO_FILE_ENCODING_LINEAR_8);
		header.sample_rate = SWAP32BE(OPL_SAMPLE_RATE);
		header.channels = SWAP32BE((uint32_t)1);
		fwrite(&header, sizeof(header), 1, fd);
		*/
		write_header(fd);

		// render the sound data
		do
		{
			BEL_ST_Simple_CallBack(NULL, buff, sizeof(buff));
			//printf("\nplaying %d lastavg %u", SD_SoundPlaying(), g_lastBlockAvg);
			putchar('.'); fflush(stdout);
			fwrite(buff, sizeof(buff), 1, fd);
		}
#ifdef BLOCK_AVG_CUTOFF
		while ((i > 0 && SD_SoundPlaying()) || g_lastBlockAvg > BLOCK_AVG_CUTOFF);
#else
		while (SD_SoundPlaying());
#endif
		printf("\n");

		// write data size
		/*
		uint32_t size = ftell(fd);
		size = SWAP32BE(size);
		fseek(fd, offsetof(Audio_filehdr, data_size), SEEK_SET);
		fwrite(&size, sizeof(size), 1, fd);
		*/
		store_size(fd);

		fclose(fd);
	}

#ifdef CONVERT_MUSIC
	SD_SetMusicMode(smm_AdLib);
	for (int i = 0; i < LASTMUSIC; i++)	//int i = 0;
	{
		snprintf(filename, sizeof(filename), "adlib/music%02d.au", i);

		fd = fopen(filename, "r");
		if (fd)
		{
			// already exists, skip this one
			fclose(fd);
			continue;
		}

		printf("Converting music %d/%d", i, LASTMUSIC-1);
		CA_CacheAudioChunk(STARTMUSIC+i);
		SD_StartMusic((MusicGroup far *)audiosegs[STARTMUSIC+i]);

		fd = fopen(filename, "w");
		write_header(fd);

		SD_MusicOn();
		// render the sound data
		do
		{
			BEL_ST_Simple_CallBack(NULL, buff, sizeof(buff));
			//printf("\nmusic playing %d playedonce %d", SD_MusicPlaying(), sqPlayedOnce);
			putchar('.'); fflush(stdout);
			fwrite(buff, sizeof(buff), 1, fd);
		}
		while (/*SD_MusicPlaying() &&*/ !sqPlayedOnce);
		SD_MusicOff();

		printf("\n");

		store_size(fd);
		fclose(fd);
	}
#endif

	printf("Conversion shutdown...\n");
	SD_Shutdown ();
	PM_Shutdown ();
	CA_Shutdown ();

	printf("Conversion finished!\n");
}

// id_mm.c

id0_boolean_t		mmerror;
char buffermem[BUFFERSIZE];
memptr		bufferseg = buffermem;
mminfotype	mminfo;

void MM_Startup (void)
{
}

void MM_Shutdown (void)
{
}

void MM_GetPtr (memptr *baseptr,id0_unsigned_long_t size)
{
	*baseptr = malloc(size);
}

void MM_FreePtr (memptr *baseptr)
{
	free(*baseptr);
}

void MM_SetPurge (memptr *baseptr, id0_int_t purge)
{
}

void MM_SetLock (memptr *baseptr, id0_boolean_t locked)
{
}

void MM_BombOnError (boolean bomb)
{
}

char QuitMsg[] = {"Unit: $%02x Error: $%02x"};
void Quit (/*const*/ id0_char_t *error, ...)
{
	uint16_t unit,err;
	va_list ap;

	//ShutdownId ();
	if (error && *error)
	{
		//puts(error);
		va_start(ap,error);
		unit=va_arg(ap,int);
		err=va_arg(ap,int);
		printf(error,unit,err);
		printf("\n");
		va_end(ap);

		exit(1);
	}

	exit(0);
}

// 3d_play.c
void OpenPageFile(void)
{
	// this function is actually needed by PM_Startup
	PML_OpenPageFile(PageFileName);
}

// id_us_1.c
int16_t		US_CheckParm(char far *parm,char far * far * strings){ return(-1); }

// stubs
pictabletype	id0_seg *pictable;
pictabletype	id0_seg *picmtable;
uint16_t bufferofs;
uint8_t vga_memory[256 * 1024];

int16_t VW_MarkUpdateBlock (int16_t x1, int16_t y1, int16_t x2, int16_t y2) { return 0; }

// stubs for jm_free.c which has some CA and PM stuff moved to it...
#include "3d_def.h"
boolean         IsA386;
gametype gamestate;
int16_t starting_episode,starting_level,starting_difficulty;
boolean PowerBall = false;
uint8_t vgapal[768];
boolean		IN_Started;
boolean			MousePresent;
boolean			JoysPresent[MaxJoys];
char			far * far IN_ParmStrings[] = {nil};
boolean		US_Started;
char		far * far US_ParmStrings[] = {"TEDLEVEL","NOWAIT"},
					far * far US_ParmStrings2[] = {"COMP","NOCOMP"};
boolean		compatability,usedummy=false, nevermark = false;
int32_t		far finetangent[FINEANGLES/4];
fixed 		far sintable[ANGLES+ANGLES/4],far *costable = sintable+(ANGLES/4);
const uint8_t colormap[16896];
int16_t		horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];
char	configname[13]="CONFIG.";
char tempPath[256];
HighScore	Scores[MaxScores];
boolean		mouseenabled,joystickenabled,joypadenabled,joystickprogressive;
int16_t			joystickport;
int16_t			dirscan[4];
int16_t			buttonscan[NUMBUTTONS];
int16_t			buttonmouse[4];
int16_t			buttonjoy[4];
int16_t                     viewsize;
int16_t             mouseadjustment;
CP_itemtype far MainMenu[] = {{AT_ENABLED,"",0}};
CP_iteminfo far MainItems;
MovieStuff_t Movies[] ={{{"IANIM."},1,3,0,0,200}};
char far SaveGameNames[10][GAME_DESCRIPTION_LEN+1],far SaveName[13]="SAVEGAM?.";
char	demoname[13] = "DEMO?.";
boolean		Keyboard[NumCodes];
int16_t			bordertime,DebugOk = false,InstantWin = 0,InstantQuit = 0;
byte		tilemap[MAPSIZE][MAPSIZE];
uint16_t	farmapylookup[MAPSIZE];
byte		*nearmapylookup[MAPSIZE];
uint16_t	uwidthtable[UPDATEHIGH];
uint16_t	blockstarts[UPDATEWIDE*UPDATEHIGH];
byte		update[UPDATESIZE];
uint16_t	displayofs,pelpan;
char far * far MainStrs[] = { "" };
byte		*updateptr;
#ifdef GAMEVER_RESTORATION_AOG
int16_t EpisodeSelect[6]={1};
CP_itemtype far NewEmenu[]= {{AT_ENABLED,"",0}};
char destPath[256];
#endif

void MakeDestPath(char *file){}
void fprint(char far *text){}
void IN_GetJoyAbs(word joy,word *xp,word *yp) {}
void IN_SetupJoy(word joy,word minx,word maxx,word miny,word maxy) {}
void INL_StartKbd(void) {}
boolean INL_StartMouse(void) {return false;}
void IN_Shutdown(void) {}
//void OpenPageFile(void){ }
void US_InitRndT(boolean randomize){}
int findfirst(const char *pathname, struct ffblk *ffblk, int attrib) { return 0; }
void VL_SetPaletteIntensity(int16_t start, int16_t end, byte far *palette, int8_t intensity){}
void VL_SetPalette (byte firstreg, uint16_t numregs, byte far *palette){}
void VL_FadeOut (int16_t start, int16_t end, int16_t red, int16_t green, int16_t blue, int16_t steps) {}
void VL_FadeIn (int16_t start, int16_t end, byte far *palette, int16_t steps) {}
void VL_WaitVBL (int16_t vbls){}
void VL_Bar (int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {}
void VH_UpdateScreen(void){}
void IN_StartAck(void) {}
boolean IN_CheckAck (void) {return false;}
boolean IN_UserInput(longword delay) {return false;}
#ifdef GAMEVER_RESTORATION_AOG
boolean DoMovie(movie_t movie) {return false;}
#else
boolean DoMovie(movie_t movie, memptr palette){return false;}
#endif
void ClearMemory (void){}
void CacheDrawPic(int16_t x, int16_t y, int16_t pic){}
boolean         MS_CheckParm (char far *string) {return false;}
void VL_Startup (void) {}
void VL_SetVGAPlaneMode (void) {}
boolean CheckForSpecialCode(uint16_t ItemNum) {return false;}
void	ShutdownId(void){}
void LoadFonts(void) {}
void 	LoadLatchMem (void) {}
void            NewViewSize (int16_t width) {}
void InitRedShifts (void) {}
#ifndef GAMEVER_RESTORATION_AOG
void InitDestPath(void){}
#endif
boolean CheckDiskSpace(int32_t needed,char far *text,cds_io_type io_type) {return false;}

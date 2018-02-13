// ID_CA.H
//===========================================================================

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
#define NUM_EPISODES			6
#define MAPS_PER_EPISODE	15 // Looks like this was used instead of 11
#define MAPS_WITH_STATS		9
#else
#define NUM_EPISODES			1
#define MAPS_PER_EPISODE	25
#define MAPS_WITH_STATS		20
#endif

#define NUMMAPS				NUM_EPISODES*MAPS_PER_EPISODE
#define MAPPLANES	2

#define UNCACHEGRCHUNK(chunk)	{MM_FreePtr(&grsegs[chunk]);grneeded[chunk]&=~ca_levelbit;}

#define THREEBYTEGRSTARTS

#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE	3
#else
#define FILEPOSSIZE	4
#endif

//===========================================================================

#pragma pack(2)
typedef	struct
{
	int32_t		planestart[3];
	uint16_t	planelength[3];
	uint16_t	width,height;
	char		name[16];
} maptype;
#pragma pack()

#pragma pack(2)
typedef struct
{
  uint16_t bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;
#pragma pack()

#pragma pack(2)
typedef struct
{
	uint16_t	RLEWtag;
	int32_t		headeroffsets[100];
	byte		tileinfo[];
} mapfiletype;
#pragma pack()

//===========================================================================

extern	char		audioname[13];

extern	byte 		_seg	*tinf;
extern	int16_t			mapon;

extern	uint16_t	_seg	*mapsegs[MAPPLANES];
extern	maptype		_seg	*mapheaderseg[NUMMAPS];
extern	byte		_seg	*audiosegs[NUMSNDCHUNKS];
extern	void		_seg	*grsegs[NUMCHUNKS];

extern	byte		far	grneeded[NUMCHUNKS];
extern	byte		ca_levelbit,ca_levelnum;

extern	char		*titleptr[8];

extern	int			profilehandle,debughandle;

extern	char		extension[5],
			gheadname[10],
			gfilename[10],
			gdictname[10],
			mheadname[10],
			mfilename[10],
			aheadname[10],
			afilename[10];

extern int32_t		_seg *grstarts;	// array of offsets in egagraph, -1 for sparse
extern int32_t		_seg *audiostarts;	// array of offsets in audio / audiot
//
// hooks for custom cache dialogs
//
extern	void	(*drawcachebox)		(char *title, uint16_t numcache);
extern	void	(*updatecachebox)	(void);
extern	void	(*finishcachebox)	(void);

extern int			grhandle;		// handle to EGAGRAPH
extern int			maphandle;		// handle to MAPTEMP / GAMEMAPS
extern int			audiohandle;	// handle to AUDIOT / AUDIO
extern int32_t		chunkcomplen,chunkexplen;

#ifdef GRHEADERLINKED
extern huffnode	*grhuffman;
#else
extern huffnode	grhuffman[255];
#endif

//===========================================================================

// just for the score box reshifting

void CAL_ShiftSprite (uint16_t segment,uint16_t source,uint16_t dest,
	uint16_t width, uint16_t height, uint16_t pixshift);

//===========================================================================

void CA_OpenDebug (void);
void CA_CloseDebug (void);
boolean CA_FarRead (int handle, byte far *dest, int32_t length);
boolean CA_FarWrite (int handle, byte far *source, int32_t length);
boolean CA_ReadFile (char *filename, memptr *ptr);
boolean CA_LoadFile (char *filename, memptr *ptr);
boolean CA_WriteFile (char *filename, void far *ptr, int32_t length);

int32_t CA_RLEWCompress (uint16_t huge *source, int32_t length, uint16_t huge *dest,
  uint16_t rlewtag);

void CA_RLEWexpand (uint16_t huge *source, uint16_t huge *dest,int32_t length,
  uint16_t rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

void CA_SetGrPurge (void);
void CA_CacheAudioChunk (int16_t chunk);
void CA_LoadAllSounds (void);

void CA_UpLevel (void);
void CA_DownLevel (void);

void CA_SetAllPurge (void);

void CA_ClearMarks (void);
void CA_ClearAllMarks (void);

#define CA_MarkGrChunk(chunk)	grneeded[chunk]|=ca_levelbit

void CA_CacheGrChunk (int16_t chunk);
void CA_CacheMap (int16_t mapnum);

void CA_CacheMarks (void);

void CAL_SetupAudioFile (void);
void CAL_SetupGrFile (void);
void CAL_SetupMapFile (void);
void CAL_HuffExpand (byte huge *source, byte huge *dest,
  int32_t length,huffnode far *hufftable, boolean screenhack);

void CloseGrFile(void);
void OpenGrFile(void);

//
//	ID Engine
//	ID_SD.c - Sound Manager for Wolfenstein 3D
//	v1.3 (revised for **********, screwed with for Blake Stone)
//	By Jason Blochowiak
//

//
//	This module handles dealing with generating sound on the appropriate
//		hardware
//
//	Depends on: User Mgr (for parm checking)
//
//	Globals:
//		For User Mgr:
//			SoundSourcePresent - Sound Source thingie present?
//			SoundBlasterPresent - SoundBlaster card present?
//			AdLibPresent - AdLib card present?
//			SoundMode - What device is used for sound effects
//				(Use SM_SetSoundMode() to set)
//			MusicMode - What device is used for music
//				(Use SM_SetMusicMode() to set)
//			DigiMode - What device is used for digitized sound effects
//				(Use SM_SetDigiDevice() to set)
//
//		For Cache Mgr:
//			NeedsDigitized - load digitized sounds?
//			NeedsMusic - load music?
//

//#pragma hdrstop		// Wierdo thing with MUSE

//#include <dos.h>

#ifdef	_MUSE_      // Will be defined in ID_Types.h
#include "ID_SD.h"
#else
#include "id_heads.h"
#endif
#ifdef __AMIGA__
#include "lowlevel.h"
void SDL_SetDS(void) {}
static void interrupt SDL_t0Service(void);
#endif
//#pragma	hdrstop
//#pragma	warn	-pia

#ifdef	nil
#undef	nil
#endif
#define	nil	0

#define	SDL_SoundFinished()	{SoundNumber = SoundPriority = 0;}

// Macros for SoundBlaster stuff
#define	sbOut(n,b)				outportb((n) + sbLocation,b)
#define	sbIn(n)					inportb((n) + sbLocation)
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// Use macro as in Wolfenstein 3D
#ifdef GAMEVER_RESTORATION_AOG_100
#define	sbWriteDelay()	while (sbIn(sbWriteStat) & 0x80);
#else
#define	sbSimpleWriteDelay()	while (sbIn(sbWriteStat) & 0x80);
#endif
#define	sbReadDelay()			while (sbIn(sbDataAvail) & 0x80);

// Macros for AdLib stuff
#define	selreg(n)	outportb(alFMAddr,n)
#define	writereg(n)	outportb(alFMData,n)
#define	readstat()	inportb(alFMStatus)

//	Imports from ID_SD_A.ASM
extern	void			SDL_SetDS(void);
#ifdef __AMIGA__
#define SDL_t0FastAsmService SDL_t0Service
#define SDL_t0SlowAsmService SDL_t0Service
#else
extern	void interrupt	SDL_t0FastAsmService(void),
						SDL_t0SlowAsmService(void);
#endif

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
//	Imports from ID_SDD.C
#undef	NUMSOUNDS
#undef	NUMSNDCHUNKS
#undef	STARTPCSOUNDS
#undef	STARTADLIBSOUNDS
#undef	STARTDIGISOUNDS
#undef	STARTMUSIC
#endif

//	Internal variables
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// Some variables cannot be static since moving code into JM_FREE.C
// (while still accessing at least some of the variables here as well)
#ifdef GAMEVER_RESTORATION_AOG_100
#define GAMEVER_RESTORATION_CONDSTATIC
#else
#define GAMEVER_RESTORATION_CONDSTATIC static
#endif


extern	word	sdStartPCSounds;
extern	word	sdStartALSounds;
extern	int16_t		sdLastSound;
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
int16_t		DigiMap[LASTSOUND];
#else
extern	int16_t		DigiMap[];
#endif

//	Global variables
	boolean		SoundSourcePresent,
				AdLibPresent,
				SoundBlasterPresent,SBProPresent,
				NeedsDigitized,NeedsMusic,
				SoundPositioned;
	SDMode		SoundMode;
	SMMode		MusicMode;
	SDSMode		DigiMode;
	longword	TimeCount;
	word		HackCount;
	SoundCommon		**SoundTable;
	//word		*SoundTable;	// Really * _seg *SoundTable, but that don't work
	boolean		ssIsTandy;
	word		ssPort = 2;

//	Internal variables
GAMEVER_RESTORATION_CONDSTATIC	boolean			SD_Started;
		boolean			nextsoundpos;
		longword		TimerDivisor,TimerCount;
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// Table used in JM_FREE.C as of AOG v1.0
#ifdef GAMEVER_RESTORATION_AOG_100
char far * 		far SD_ParmStrings[] =
#else
static	char far * 		far ParmStrings[] =
#endif
						{
							"noal",
							"nosb",
							"nopro",
							"noss",
							"sst",
							"ss1",
							"ss2",
							"ss3",
							nil
						};
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
static	void			(*SoundUserHook)(void);
#endif
		soundnames		SoundNumber,DigiNumber;
		word			SoundPriority,DigiPriority;
		int16_t				LeftPosition,RightPosition;
		void interrupt	(*t0OldService)(void);
		int32_t			LocalTime;
		word			TimerRate;

		word				NumDigi,DigiLeft,DigiPage;
		word				_seg *DigiList;
		word				DigiLastStart,DigiLastEnd;
		volatile boolean	DigiPlaying;
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// As in Wolfenstein 3D
#ifdef GAMEVER_RESTORATION_AOG_100
static	boolean			DigiMissed,DigiLastSegment;
static	memptr			DigiNextAddr;
static	word			DigiNextLen;
#else
static	volatile boolean	DigiMissed,DigiLastSegment;
static	volatile memptr		DigiNextAddr;
static	volatile word		DigiNextLen;
		volatile longword	DigiFailSafe;
		longword			DigiFailTriggered;
#endif

//	SoundBlaster variables
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
		word					SBResetCount;
#endif
GAMEVER_RESTORATION_CONDSTATIC	boolean					sbNoCheck,sbNoProCheck;
static	volatile boolean		sbSamplePlaying;
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// As in Wolfenstein 3D
#ifdef GAMEVER_RESTORATION_AOG_100
static	byte					sbOldIntMask = -1;
#else
static	byte					sbPIC1Mask,sbPIC2Mask;
static	byte					sbOldIntMask = -1,sbOldIntMask2 = -1;
#endif
static	volatile byte			huge *sbNextSegPtr;
GAMEVER_RESTORATION_CONDSTATIC	byte					sbDMA = 1,
								sbDMAa1 = 0x83,sbDMAa2 = 2,sbDMAa3 = 3,
								sba1Vals[] = {0x87,0x83,0,0x82},
								sba2Vals[] = {0,2,0,6},
								sba3Vals[] = {1,3,0,7};
GAMEVER_RESTORATION_CONDSTATIC	int16_t						sbLocation = -1,sbInterrupt = 7,sbIntVec = 0xf,
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
								sbIntVectors[] = {-1,-1,0xa,0xb,-1,0xd,-1,0xf,-1,-1,-1};
#else
								sbIntVectors[] = {-1,-1,0xa,0xb,-1,0xd,-1,0xf,-1,-1,0x72};
#endif
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
static	volatile byte			sbLastTimeValue;
#endif
static	volatile longword		sbNextSegLen;
static	volatile SampledSound	huge *sbSamples;
GAMEVER_RESTORATION_CONDSTATIC	void interrupt			(*sbOldIntHand)(void);
GAMEVER_RESTORATION_CONDSTATIC	byte					sbpOldFMMix,sbpOldVOCMix;

//	SoundSource variables
		boolean				ssNoCheck;
		boolean				ssActive;
		word				ssControl,ssStatus,ssData;
		byte				ssOn,ssOff;
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
		word				ssVol;
		byte				ssVolTable[256];
#endif
		volatile byte		far *ssSample;
		volatile longword	ssLengthLeft;

//	PC Sound variables
		volatile byte	pcLastSample,far *pcSound;
		longword		pcLengthLeft;
		word			pcSoundLookup[255];

//	AdLib variables
		boolean			alNoCheck;
		byte			far *alSound;
		word			alBlock;
		longword		alLengthLeft;
		longword		alTimeCount;
		Instrument		alZeroInst;

// This table maps channel numbers to carrier and modulator op cells
static	byte			carriers[9] =  { 3, 4, 5,11,12,13,19,20,21},
						modifiers[9] = { 0, 1, 2, 8, 9,10,16,17,18},
// This table maps percussive voice numbers to op cells
						pcarriers[5] = {19,0xff,0xff,0xff,0xff},
						pmodifiers[5] = {16,17,18,20,21};

//	Sequencer variables
		boolean			sqActive;
static	word			alFXReg;
static	ActiveTrack		*tracks[sqMaxTracks],
						mytracks[sqMaxTracks];
static	word			sqMode,sqFadeStep;
		word			far *sqHack,far *sqHackPtr,sqHackLen,sqHackSeqLen;
		int32_t			sqHackTime;
		boolean			sqPlayedOnce;

//	Internal routines
		void			SDL_DigitizedDone(void);

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetTimer0() - Sets system timer 0 to the specified speed
//
///////////////////////////////////////////////////////////////////////////
//#pragma	argsused
static void
SDL_SetTimer0(word speed)
{
#ifdef __AMIGA__
	BE_ST_SetTimer(speed);
	TimerDivisor = speed;
#else
#ifndef TPROF	// If using Borland's profiling, don't screw with the timer
asm	pushf
asm	cli

	outportb(0x43,0x36);				// Change timer 0
	outportb(0x40,speed);
	outportb(0x40,speed >> 8);
	// Kludge to handle special case for digitized PC sounds
	if (TimerDivisor == (1192030 / (TickBase * 100)))
		TimerDivisor = (1192030 / (TickBase * 10));
	else
		TimerDivisor = speed;

asm	popf
#else
	TimerDivisor = 0x10000;
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetIntsPerSec() - Uses SDL_SetTimer0() to set the number of
//		interrupts generated by system timer 0 per second
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_SetIntsPerSec(word ints)
{
	TimerRate = ints;
	SDL_SetTimer0(1192030 / ints);
}

static void
SDL_SetTimerSpeed(void)
{
	word	rate;
	void interrupt	(*isr)(void);

	if
	(
		(MusicMode == smm_AdLib)
	||	((DigiMode == sds_SoundSource) && DigiPlaying)
	)
	{
		rate = TickBase * 10;
		isr = SDL_t0FastAsmService;
	}
	else
	{
		rate = TickBase * 2;
		isr = SDL_t0SlowAsmService;
	}

	if (rate != TimerRate)
	{
#ifdef __AMIGA__
		BE_ST_StopAudioSDService();
		BE_ST_StartAudioSDService(isr);
#else
		setvect(8,isr);
#endif
		SDL_SetIntsPerSec(rate);
		TimerRate = rate;
	}
}

//
//	SoundBlaster code
//

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// Use macro as in Wolfenstein 3D
#ifndef GAMEVER_RESTORATION_AOG_100
///////////////////////////////////////////////////////////////////////////
//
//	sbWriteDelay() - Waits for the card to become ready. If it doesn't
//		become ready, we reset the card, and reprogram the last time value.
//
///////////////////////////////////////////////////////////////////////////
boolean
sbWriteDelay(void)
{
	int16_t	i;

#ifndef __AMIGA__
	// Try to avoid hitting the card while it's doing a DMA transfer
	for (i = 0;i < 256;i++)
		if (sbIn(sbWriteStat) & 0x80)
			break;
	// See if the DSP is available. If it doesn't go active after 1000
	// cycles, assume it hung, and reset the damn thing.
	for (i = 0;i < 1000;i++)
		if (!(sbIn(sbWriteStat) & 0x80))
			break;
	if (i == 1000)								// Assume that the DSP hung
	{
		SBResetCount++;							// Increment our counter

		sbOut(sbReset,true);					// Reset the SoundBlaster DSP
		for (i = 0;i < 9;i++)					// Wait >4usec
			inportb(alFMStatus);

		sbOut(sbReset,false);					// Turn off sb DSP reset
		for (i = 0;i < 100;i++)					// Wait >100usec
			inportb(alFMStatus);

		sbSimpleWriteDelay();
		sbOut(sbWriteCmd,0xd1);					// Turn on DSP speaker

		sbSimpleWriteDelay();					// Reprogram last time value
		sbOut(sbWriteCmd,0x40);
		sbSimpleWriteDelay();
		sbOut(sbWriteData,sbLastTimeValue);

		return(true);
	}
	else
#endif
		return(false);
}
#endif


/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// These ones are also not present in Wolfenstein 3D
#ifndef GAMEVER_RESTORATION_AOG_100
///////////////////////////////////////////////////////////////////////////
//
//	SDL_EnableDMAInt() - Save old interrupt status and unmask DMA interrupt
//
///////////////////////////////////////////////////////////////////////////
void
SDL_EnableDMAInt(void)
{
#ifndef __AMIGA__
	sbOldIntMask = inportb(0x21);
	outportb(0x21,sbOldIntMask & ~sbPIC1Mask);

	if (sbInterrupt >= 8)
	{
		sbOldIntMask2 = inportb(0xa1);
		outportb(0xa1,sbOldIntMask2 & ~sbPIC2Mask);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_RestoreDMAInt() - Restore DMA interrupt mask bit(s)
//
///////////////////////////////////////////////////////////////////////////
void
SDL_RestoreDMAInt(void)
{
#ifndef __AMIGA__
	byte	is;

	is = inportb(0x21);
	if (sbOldIntMask & sbPIC1Mask)
		is |= sbPIC1Mask;
	else
		is &= ~sbPIC1Mask;
	outportb(0x21,is);

	if (sbInterrupt >= 8)
	{
		is = inportb(0xa1);
		if (sbOldIntMask2 & sbPIC2Mask)
			is |= sbPIC2Mask;
		else
			is &= ~sbPIC2Mask;
		outportb(0xa1,is);
	}
#endif
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBStopSample() - Stops any active sampled sound and causes DMA
//		requests from the SoundBlaster to cease
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SBStopSample(void)
{
	byte	is;
	int16_t		i;
#ifdef __AMIGA__
	BE_ST_StopDigiSound();
#else
asm	pushf
asm	cli

	if (sbSamplePlaying)
	{
		sbSamplePlaying = false;

		sbWriteDelay();
		sbOut(sbWriteCmd,0xd0);	// Turn off DSP DMA
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
		for (i = 0;i < 256;i++)	// was 80/256
			sbIn(sbWriteStat);
#endif

#ifdef GAMEVER_RESTORATION_AOG_100
//#if 0
		is = inportb(0x21);	// Restore interrupt mask bit
		if (sbOldIntMask & (1 << sbInterrupt))
			is |= (1 << sbInterrupt);
		else
			is &= ~(1 << sbInterrupt);
		outportb(0x21,is);
#endif
	}

asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBPlaySeg() - Plays a chunk of sampled sound on the SoundBlaster
//	Insures that the chunk doesn't cross a bank boundary, programs the DMA
//	 controller, and tells the SB to start doing DMA requests for DAC
//
///////////////////////////////////////////////////////////////////////////
static longword
SDL_SBPlaySeg(volatile byte huge *data,longword length)
{
	uint16_t		datapage;
	longword		dataofs,uselen;

#ifdef __AMIGA__
	uselen = length-1;
	BE_ST_PlayDigiSound(data, length);
#else
	uselen = length;
	datapage = FP_SEG(data) >> 12;
	dataofs = ((FP_SEG(data) & 0xfff) << 4) + FP_OFF(data);
	if (dataofs >= 0x10000)
	{
		datapage++;
		dataofs -= 0x10000;
	}

	if (dataofs + uselen > 0x10000)
		uselen = 0x10000 - dataofs;

	uselen--;

	// Program the DMA controller
asm	pushf
asm	cli
	outportb(0x0a,sbDMA | 4);					// Mask off DMA on channel sbDMA
	outportb(0x0c,0);							// Clear byte ptr flip-flop to lower byte
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
	// As in Wolfenstein 3D
#ifdef GAMEVER_RESTORATION_AOG_100
	outportb(0x0b,0x49);						// Set transfer mode for D/A conv
#else
	outportb(0x0b,sbDMA | 0x48);				// Set transfer mode for D/A conv (Single, address increment, read)
#endif
	outportb(sbDMAa2,(byte)dataofs);			// Give LSB of address
	outportb(sbDMAa2,(byte)(dataofs >> 8));		// Give MSB of address
	outportb(sbDMAa1,(byte)datapage);			// Give page of address
	outportb(sbDMAa3,(byte)uselen);				// Give LSB of length
	outportb(sbDMAa3,(byte)(uselen >> 8));		// Give MSB of length
	outportb(0x0a,sbDMA);						// Re-enable DMA on channel sbDMA

	// Start playing the thing
	sbWriteDelay();
	sbOut(sbWriteCmd,0x14);
	sbWriteDelay();
	sbOut(sbWriteData,(byte)uselen);
	sbWriteDelay();
	sbOut(sbWriteData,(byte)(uselen >> 8));
asm	popf
#endif

	return(uselen + 1);
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBService() - Services the SoundBlaster DMA interrupt
//
///////////////////////////////////////////////////////////////////////////
GAMEVER_RESTORATION_CONDSTATIC void interrupt
SDL_SBService(void)
{
	longword	used;

#ifndef __AMIGA__
#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,10	// bright green
asm	out	dx,al
#endif

	sbIn(sbDataAvail);	// Ack interrupt to SB

	if (sbNextSegPtr)
	{
		used = SDL_SBPlaySeg(sbNextSegPtr,sbNextSegLen);
		if (sbNextSegLen <= used)
			sbNextSegPtr = nil;
		else
		{
			sbNextSegPtr += used;
			sbNextSegLen -= used;
		}
	}
	else
	{
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		SDL_SBStopSample();
#else
//		SDL_SBStopSample();
		sbSamplePlaying = false;
#endif
		SDL_DigitizedDone();
	}

	outportb(0x20,0x20);	// Ack interrupt
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	if (sbInterrupt >= 8)	// If necessary, ack to cascade PIC
		outportb(0xa0,0x20);
#endif

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,3	// blue
asm	out	dx,al
asm	mov	al,0x20	// normal
asm	out	dx,al
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBPlaySample() - Plays a sampled sound on the SoundBlaster. Sets up
//		DMA to play the sound
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SBPlaySample(byte huge *data,longword len)
{
	longword	used;

	SDL_SBStopSample();

#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	used = SDL_SBPlaySeg(data,len);
	if (len <= used)
		sbNextSegPtr = nil;
	else
	{
		sbNextSegPtr = data + used;
		sbNextSegLen = len - used;
	}

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
//#if 0
	// Save old interrupt status and unmask ours
	sbOldIntMask = inportb(0x21);
	outportb(0x21,sbOldIntMask & ~(1 << sbInterrupt));
#endif

#ifdef GAMEVER_RESTORATION_AOG_100
//#if 0
	sbWriteDelay();
	sbOut(sbWriteCmd,0xd4);						// Make sure DSP DMA is enabled
#endif

	sbSamplePlaying = true;

#ifndef __AMIGA__
asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PositionSBP() - Sets the attenuation levels for the left and right
//		channels by using the mixer chip on the SB Pro. This hits a hole in
//		the address map for normal SBs.
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_PositionSBP(int16_t leftpos,int16_t rightpos)
{
	byte	v;


	if (!SBProPresent)
		return;

#ifdef __AMIGA__
	// convert 0-8 positions to volumes 0-15, sum them and transform to 0-65536
	uint32_t volume = ((15-leftpos)+(15-rightpos))*(65536/30);
	// convert the channel volumes into panning
	uint32_t pan = (16-leftpos+rightpos)*2048;
	//uint32_t pan = (8-leftpos+rightpos)*4096; // maximum stereo separation
	BE_ST_SetDigiSoundVol(volume, pan);
#else
	leftpos = 15 - leftpos;
	rightpos = 15 - rightpos;
	v = ((leftpos & 0x0f) << 4) | (rightpos & 0x0f);

asm	pushf
asm	cli

	sbOut(sbpMixerAddr,sbpmVoiceVol);
	sbOut(sbpMixerData,v);

asm	popf
#endif
}

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// Code originally relocated to JM_FREE.C in AOG v1.0,
// with minor changes to SDL_StartSB
#ifndef GAMEVER_RESTORATION_AOG_100
///////////////////////////////////////////////////////////////////////////
//
//	SDL_CheckSB() - Checks to see if a SoundBlaster resides at a
//		particular I/O location
//
///////////////////////////////////////////////////////////////////////////
static boolean
SDL_CheckSB(int16_t port)
{
	int16_t	i;

#ifdef __AMIGA__
	return true;
#else
	sbLocation = port << 4;		// Initialize stuff for later use

	sbOut(sbReset,true);		// Reset the SoundBlaster DSP
asm	mov	dx,alFMStatus			// Wait >4usec
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx
asm	in	al, dx

	sbOut(sbReset,false);		// Turn off sb DSP reset
asm	mov	dx,alFMStatus			// Wait >100usec
asm	mov	cx,100
usecloop:
asm	in	al,dx
asm	loop usecloop

	for (i = 0;i < 100;i++)
	{
		if (sbIn(sbDataAvail) & 0x80)		// If data is available...
		{
			if (sbIn(sbReadData) == 0xaa)	// If it matches correct value
				return(true);
			else
			{
				sbLocation = -1;			// Otherwise not a SoundBlaster
				return(false);
			}
		}
	}
	sbLocation = -1;						// Retry count exceeded - fail
#endif
	return(false);
}

///////////////////////////////////////////////////////////////////////////
//
//	Checks to see if a SoundBlaster is in the system. If the port passed is
//		-1, then it scans through all possible I/O locations. If the port
//		passed is 0, then it uses the default (2). If the port is >0, then
//		it just passes it directly to SDL_CheckSB()
//
///////////////////////////////////////////////////////////////////////////
static boolean
SDL_DetectSoundBlaster(int16_t port)
{
	int16_t	i;

	if (port == 0)					// If user specifies default, use 2
		port = 2;
	if (port == -1)
	{
		if (SDL_CheckSB(2))			// Check default before scanning
			return(true);

		if (SDL_CheckSB(4))			// Check other SB Pro location before scan
			return(true);

		for (i = 1;i <= 6;i++)		// Scan through possible SB locations
		{
			if ((i == 2) || (i == 4))
				continue;

			if (SDL_CheckSB(i))		// If found at this address,
				return(true);		//	return success
		}
		return(false);				// All addresses failed, return failure
	}
	else
		return(SDL_CheckSB(port));	// User specified address or default
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SBSetDMA() - Sets the DMA channel to be used by the SoundBlaster
//		code. Sets up sbDMA, and sbDMAa1-sbDMAa3 (used by SDL_SBPlaySeg()).
//
///////////////////////////////////////////////////////////////////////////
void
SDL_SBSetDMA(byte channel)
{
	if (channel > 3)
		SD_ERROR(SD_STARTUP_BAD_DMA);

	sbDMA = channel;
	sbDMAa1 = sba1Vals[channel];
	sbDMAa2 = sba2Vals[channel];
	sbDMAa3 = sba3Vals[channel];
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartSB() - Turns on the SoundBlaster
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartSB(void)
{
	byte	timevalue,test;

#ifdef __AMIGA__
	// for the panning support
	SBProPresent = true;
	BE_ST_InitAudio();
#else
	sbIntVec = sbIntVectors[sbInterrupt];
	if (sbIntVec < 0)
		SD_ERROR(SD_STARTUP_BAD_INTERRUPT);

	sbPIC1Mask = 1 << (sbInterrupt & 7);
	sbPIC2Mask = 1 << 2;

	sbOldIntHand = getvect(sbIntVec);	// Get old interrupt handler
	setvect(sbIntVec,SDL_SBService);	// Set mine

	SDL_EnableDMAInt();					// Enable DMA interrupt

	sbWriteDelay();
	sbOut(sbWriteCmd,0xd1);				// Turn on DSP speaker

	// Set the SoundBlaster DAC time constant for 7KHz
	timevalue = 256 - (1000000 / 7000);
	sbLastTimeValue = timevalue;
	sbWriteDelay();
	sbOut(sbWriteCmd,0x40);
	sbWriteDelay();
	sbOut(sbWriteData,timevalue);

	SBProPresent = false;
	if (sbNoProCheck)
		return;

	// Check to see if this is a SB Pro
	sbOut(sbpMixerAddr,sbpmFMVol);
	sbpOldFMMix = sbIn(sbpMixerData);
	sbOut(sbpMixerData,0xbb);
	test = sbIn(sbpMixerData);
	if (test == 0xbb)
	{
		// Boost FM output levels to be equivilent with digitized output
		sbOut(sbpMixerData,0xff);
		test = sbIn(sbpMixerData);
		if (test == 0xff)
		{
			SBProPresent = true;

			// Save old Voice output levels (SB Pro)
			sbOut(sbpMixerAddr,sbpmVoiceVol);
			sbpOldVOCMix = sbIn(sbpMixerData);

			// Turn SB Pro stereo DAC off
			sbOut(sbpMixerAddr,sbpmControl);
			sbOut(sbpMixerData,0);				// 0=off,2=on
		}
	}
#endif
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutSB() - Turns off the SoundBlaster
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutSB(void)
{
	SDL_SBStopSample();
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
	// Also not present in Wolfenstein 3D
#ifndef GAMEVER_RESTORATION_AOG_100
	SDL_RestoreDMAInt();				// Restore DMA interrupt
#endif

#ifdef __AMIGA__
	BE_ST_ShutdownAudio();
#else
	if (SBProPresent)
	{
		// Restore FM output levels (SB Pro)
		sbOut(sbpMixerAddr,sbpmFMVol);
		sbOut(sbpMixerData,sbpOldFMMix);

		// Restore Voice output levels (SB Pro)
		sbOut(sbpMixerAddr,sbpmVoiceVol);
		sbOut(sbpMixerData,sbpOldVOCMix);
	}

	setvect(sbIntVec,sbOldIntHand);		// Set vector back
#endif
}

//	Sound Source Code

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
void
SDL_SSSetVol(word vol)
{
	int16_t		i;
	int32_t	v;

	if (vol == ssVol)
		return;

	ssVol = vol;
#if 1
	v = vol * -128;
	for (i = 0;i < 256;i++)
	{
		ssVolTable[i] = (v >> 8) + 0x80;
		v += vol;
	}
#else
	for (i = 0;i < 256;i++)
		ssVolTable[i] = (((i - 0x80) * vol) >> 8) + 0x80;
#endif
}

static void
SDL_PositionSS(int16_t leftpos,int16_t rightpos)
{
	int16_t	pos;

	pos = (leftpos > rightpos)? rightpos : leftpos;
	pos = 15 - pos;
	pos *= 17;
	SDL_SSSetVol(pos);
}
#endif


///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSStopSample() - Stops a sample playing on the Sound Source
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SSStopSample(void)
{
#ifndef __AMIGA__
asm	pushf
asm	cli

	(int32_t)ssSample = 0;

asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSService() - Handles playing the next sample on the Sound Source
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_SSService(void)
{
	boolean	gotit;
	byte	v;

#ifndef __AMIGA__
	while (ssSample)
	{
	asm	mov		dx,[ssStatus]	// Check to see if FIFO is currently empty
	asm	in		al,dx
	asm	test	al,0x40
	asm	jnz		done			// Nope - don't push any more data out

		v = *ssSample++;
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
		v = ssVolTable[v];
#endif
		if (!(--ssLengthLeft))
		{
			(int32_t)ssSample = 0;
			SDL_DigitizedDone();
		}

	asm	mov		dx,[ssData]		// Pump the value out
	asm	mov		al,[v]
	asm	out		dx,al

	asm	mov		dx,[ssControl]	// Pulse printer select
	asm	mov		al,[ssOff]
	asm	out		dx,al
	asm	push	ax
	asm	pop		ax
	asm	mov		al,[ssOn]
	asm	out		dx,al

	asm	push	ax				// Delay a int16_t while
	asm	pop		ax
	asm	push	ax
	asm	pop		ax
	}
done:;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_SSPlaySample() - Plays the specified sample on the Sound Source
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_SSPlaySample(byte huge *data,longword len)
{
#ifndef __AMIGA__
asm	pushf
asm	cli

	ssLengthLeft = len;
	ssSample = (volatile byte far *)data;

asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartSS() - Sets up for and turns on the Sound Source
//
///////////////////////////////////////////////////////////////////////////
GAMEVER_RESTORATION_CONDSTATIC void
SDL_StartSS(void)
{
#ifndef __AMIGA__
	if (ssPort == 3)
		ssControl = 0x27a;	// If using LPT3
	else if (ssPort == 2)
		ssControl = 0x37a;	// If using LPT2
	else
		ssControl = 0x3be;	// If using LPT1
	ssStatus = ssControl - 1;
	ssData = ssStatus - 1;

	ssOn = 0x04;
	if (ssIsTandy)
		ssOff = 0x0e;				// Tandy wierdness
	else
		ssOff = 0x0c;				// For normal machines

	outportb(ssControl,ssOn);		// Enable SS

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	SDL_SSSetVol(0x100);
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutSS() - Turns off the Sound Source
//
///////////////////////////////////////////////////////////////////////////
GAMEVER_RESTORATION_CONDSTATIC void
SDL_ShutSS(void)
{
#ifndef __AMIGA__
	outportb(ssControl,ssOff);
#endif
}

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// Code originally relocated to JM_FREE.C in AOG v1.0
#ifndef GAMEVER_RESTORATION_AOG_100
///////////////////////////////////////////////////////////////////////////
//
//	SDL_CheckSS() - Checks to see if a Sound Source is present at the
//		location specified by the sound source variables
//
///////////////////////////////////////////////////////////////////////////
static boolean
SDL_CheckSS(void)
{
	boolean		present = false;
	longword	lasttime;

#ifndef __AMIGA__
	// Turn the Sound Source on and wait awhile (4 ticks)
	SDL_StartSS();

	lasttime = TimeCount;
	while (TimeCount < lasttime + 4)
		;

asm	mov		dx,[ssStatus]	// Check to see if FIFO is currently empty
asm	in		al,dx
asm	test	al,0x40
asm	jnz		checkdone		// Nope - Sound Source not here

asm	mov		cx,32			// Force FIFO overflow (FIFO is 16 bytes)
outloop:
asm	mov		dx,[ssData]		// Pump a neutral value out
asm	mov		al,0x80
asm	out		dx,al

asm	mov		dx,[ssControl]	// Pulse printer select
asm	mov		al,[ssOff]
asm	out		dx,al
asm	push	ax
asm	pop		ax
asm	mov		al,[ssOn]
asm	out		dx,al

asm	push	ax				// Delay a int16_t while before we do this again
asm	pop		ax
asm	push	ax
asm	pop		ax

asm	loop	outloop

asm	mov		dx,[ssStatus]	// Is FIFO overflowed now?
asm	in		al,dx
asm	test	al,0x40
asm	jz		checkdone		// Nope, still not - Sound Source not here

	present = true;			// Yes - it's here!

checkdone:
	SDL_ShutSS();
#endif
	return(present);
}

static boolean
SDL_DetectSoundSource(void)
{
	for (ssPort = 1;ssPort <= 3;ssPort++)
		if (SDL_CheckSS())
			return(true);
	return(false);
}
#endif

//
//	PC Sound code
//

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCPlaySound() - Plays the specified sound on the PC speaker
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCPlaySound(PCSound far *sound)
{
#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	pcLastSample = -1;
	pcLengthLeft = sound->common.length;
	pcSound = sound->data;

#ifndef __AMIGA__
asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCStopSound() - Stops the current sound playing on the PC Speaker
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_PCStopSound(void)
{
#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	pcSound = 0;

#ifdef __AMIGA__

#else
asm	in	al,0x61		  	// Turn the speaker off
asm	and	al,0xfd			// ~2
asm	out	0x61,al

asm	popf
#endif
}

#if defined(__AMIGA__)//0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_PCService() - Handles playing the next sample in a PC sound
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_PCService(void)
{
	byte	s;
	word	t;

	if (pcSound)
	{
		s = *pcSound++;
		if (s != pcLastSample)
		{
#ifndef __AMIGA__
		asm	pushf
		asm	cli
#endif

			pcLastSample = s;
			if (s)					// We have a frequency!
			{
				t = pcSoundLookup[s];
#ifdef __AMIGA__

#else
			asm	mov	bx,[t]

			asm	mov	al,0xb6			// Write to channel 2 (speaker) timer
			asm	out	43h,al
			asm	mov	al,bl
			asm	out	42h,al			// Low byte
			asm	mov	al,bh
			asm	out	42h,al			// High byte

			asm	in	al,0x61			// Turn the speaker & gate on
			asm	or	al,3
			asm	out	0x61,al
#endif
			}
			else					// Time for some silence
			{
#ifdef __AMIGA__

#else
			asm	in	al,0x61		  	// Turn the speaker & gate off
			asm	and	al,0xfc			// ~3
			asm	out	0x61,al
#endif
			}

#ifndef __AMIGA__
		asm	popf
#endif
		}

		if (!(--pcLengthLeft))
		{
			SDL_PCStopSound();
			SDL_SoundFinished();
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutPC() - Turns off the pc speaker
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutPC(void)
{
#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	pcSound = 0;

#ifdef __AMIGA__

#else
asm	in	al,0x61		  	// Turn the speaker & gate off
asm	and	al,0xfc			// ~3
asm	out	0x61,al

asm	popf
#endif
}

//
//	Stuff for digitized sounds
//
memptr
SDL_LoadDigiSegment(word page)
{
	memptr	addr;

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,10	// bright green
asm	out	dx,al
#endif

	addr = PM_GetSoundPage(page);
	PM_SetPageLock(PMSoundStart + page,pml_Locked);

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,3	// blue
asm	out	dx,al
asm	mov	al,0x20	// normal
asm	out	dx,al
#endif

	return(addr);
}

void
SDL_PlayDigiSegment(memptr addr,word len)
{
	switch (DigiMode)
	{
	case sds_SoundSource:
		SDL_SSPlaySample(addr,len);
		break;
	case sds_SoundBlaster:
		SDL_SBPlaySample(addr,len);
		break;
	}
}

void
SD_StopDigitized(void)
{
	int16_t	i;

#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	DigiFailSafe = 0;
#endif
	DigiLeft = 0;
	DigiNextAddr = nil;
	DigiNextLen = 0;
	DigiMissed = false;
	DigiPlaying = false;
	DigiNumber = DigiPriority = 0;
	SoundPositioned = false;
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
		SDL_SoundFinished();
#endif

	switch (DigiMode)
	{
	case sds_SoundSource:
		SDL_SSStopSample();
		break;
	case sds_SoundBlaster:
		SDL_SBStopSample();
		break;
	}

#ifndef __AMIGA__
asm	popf
#endif

	for (i = DigiLastStart;i < DigiLastEnd;i++)
		PM_SetPageLock(i + PMSoundStart,pml_Unlocked);
	DigiLastStart = 1;
	DigiLastEnd = 0;
}

void
SD_Poll(void)
{
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	if (DigiFailSafe && (DigiFailSafe < TimeCount))
	{
		DigiFailTriggered++;
		SD_StopDigitized();
	}
#endif

	if (DigiLeft && !DigiNextAddr)
	{
		DigiNextLen = (DigiLeft >= PMPageSize)? PMPageSize : (DigiLeft % PMPageSize);
		DigiLeft -= DigiNextLen;
		if (!DigiLeft)
			DigiLastSegment = true;
		DigiNextAddr = SDL_LoadDigiSegment(DigiPage++);
	}
	if (DigiMissed && DigiNextAddr)
	{
		SDL_PlayDigiSegment(DigiNextAddr,DigiNextLen);
		DigiNextAddr = nil;
		DigiMissed = false;
		if (DigiLastSegment)
		{
			DigiPlaying = false;
			DigiLastSegment = false;
		}
	}
	SDL_SetTimerSpeed();

#if 0
	if (sbSamplePlaying)
	{
	asm	mov	dx,STATUS_REGISTER_1
	asm	in	al,dx
	asm	mov	dx,ATR_INDEX
	asm	mov	al,ATR_OVERSCAN
	asm	out	dx,al
	asm	mov	al,10	// bright green
	asm	out	dx,al
	asm	mov	al,0x20	// normal
	asm	out	dx,al
	}
	else
	{
	asm	mov	dx,STATUS_REGISTER_1
	asm	in	al,dx
	asm	mov	dx,ATR_INDEX
	asm	mov	al,ATR_OVERSCAN
	asm	out	dx,al
	asm	mov	al,0	// black
	asm	out	dx,al
	asm	mov	al,0x20	// normal
	asm	out	dx,al
	}
#endif
}

void
SD_SetPosition(int16_t leftpos,int16_t rightpos)
{
	if
	(
		(leftpos < 0)
	||	(leftpos > 15)
	||	(rightpos < 0)
	||	(rightpos > 15)
	||	((leftpos == 15) && (rightpos == 15))
	)
		SD_ERROR(SD_SETPOSITION_BAD_POSITION);

	switch (DigiMode)
	{
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	case sds_SoundSource:
		SDL_PositionSS(leftpos,rightpos);
		break;
#endif
	case sds_SoundBlaster:
		SDL_PositionSBP(leftpos,rightpos);
		break;
	}
}

void
SD_PlayDigitized(word which,int16_t leftpos,int16_t rightpos)
{
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	int16_t		i;
	byte	timevalue;
	word	pages;
#endif
	word	len;
	memptr	addr;

	if (!DigiMode)
		return;

	SD_StopDigitized();
	if (which >= NumDigi)
	{
//		SD_ERROR(SD_PLAYDIGITIZED_BAD_SOUND);
		which = 1;
		return;
	}

	SD_SetPosition(leftpos,rightpos);

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef __AMIGA__
#ifndef GAMEVER_RESTORATION_AOG_100
//#if 1
	if (DigiMode == sds_SoundBlaster)
	{
		// Set the SoundBlaster DAC time constant for 7KHz
		timevalue = 256 - (1000000 / 7000);
		sbLastTimeValue = timevalue;
		sbWriteDelay();
		sbOut(sbWriteCmd,0x40);
		sbWriteDelay();
		sbOut(sbWriteData,timevalue);
		for (i = 0;i < 80;i++)
			sbIn(sbWriteStat);
	}
#endif
#endif

	DigiPage = DigiList[(which * 2) + 0];
	DigiLeft = DigiList[(which * 2) + 1];

	DigiLastStart = DigiPage;
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	DigiLastEnd = DigiPage + ((DigiLeft + (PMPageSize - 1)) / PMPageSize);
#else
	pages = ((DigiLeft + (PMPageSize - 1)) / PMPageSize);
	DigiLastEnd = DigiPage + pages;
	// Set up failsafe at ~105% of when the sound should end. This is
	// computed as 7000Hz/70Hz=100, then only dividing by 95 for the slop.
	DigiFailSafe = TimeCount + ((DigiLeft + 100) / 95);
#endif

	len = (DigiLeft >= PMPageSize)? PMPageSize : (DigiLeft % PMPageSize);
	addr = SDL_LoadDigiSegment(DigiPage++);

	DigiPlaying = true;
	DigiLastSegment = false;

	SDL_PlayDigiSegment(addr,len);
	DigiLeft -= len;
	if (!DigiLeft)
		DigiLastSegment = true;

	SD_Poll();
}

void
SDL_DigitizedDone(void)
{
	if (DigiNextAddr)
	{
		SDL_PlayDigiSegment(DigiNextAddr,DigiNextLen);
		DigiNextAddr = nil;
		DigiMissed = false;
	}
	else
	{
		if (DigiLastSegment)
		{
			DigiPlaying = false;
			DigiLastSegment = false;
			/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
			if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
			{
				SDL_SoundFinished();
			}
			else
#endif
			{
				DigiNumber = DigiPriority = 0;
				/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
				DigiFailSafe = 0;
#endif
			}
			SoundPositioned = false;
		}
		else
			DigiMissed = true;
	}
}

void
SD_SetDigiDevice(SDSMode mode)
{
	boolean	devicenotpresent;

	if (mode == DigiMode)
		return;

	SD_StopDigitized();

	devicenotpresent = false;
	switch (mode)
	{
	case sds_SoundBlaster:
		if (!SoundBlasterPresent)
		{
			if (SoundSourcePresent)
				mode = sds_SoundSource;
			else
				devicenotpresent = true;
		}
		break;
	case sds_SoundSource:
		if (!SoundSourcePresent)
			devicenotpresent = true;
		break;
	}

	if (!devicenotpresent)
	{
		if (DigiMode == sds_SoundSource)
			SDL_ShutSS();

		DigiMode = mode;

		if (mode == sds_SoundSource)
			SDL_StartSS();

		SDL_SetTimerSpeed();
	}
}

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// Code originally relocated to JM_FREE.C in AOG v1.0
#ifndef GAMEVER_RESTORATION_AOG_100
void
SDL_SetupDigi(void)
{
	memptr	list;
	word	far *p,
			pg;
	int16_t		i;

	PM_UnlockMainMem();
	MM_GetPtr(&list,PMPageSize);
	PM_CheckMainMem();
	//p = (word far *)MK_FP(PM_GetPage(ChunksInFile - 1),0);
	p = (word far *)PM_GetPage(ChunksInFile - 1);
	_fmemcpy((void far *)list,(void far *)p,PMPageSize);
	pg = PMSoundStart;
	for (i = 0;i < PMPageSize / (sizeof(word) * 2);i++,p += 2)
	{
		if (pg >= ChunksInFile - 1)
			break;
#ifdef __AMIGA__
		pg += (SWAP16LE(p[1]) + (PMPageSize - 1)) / PMPageSize;
#else
		pg += (p[1] + (PMPageSize - 1)) / PMPageSize;
#endif
	}
	PM_UnlockMainMem();
	MM_GetPtr((memptr *)&DigiList,i * sizeof(word) * 2);
	_fmemcpy((void far *)DigiList,(void far *)list,i * sizeof(word) * 2);
	MM_FreePtr(&list);
	NumDigi = i;
#ifdef __AMIGA__
	for (i = 0; i < NumDigi * 2; i++)
	{
		DigiList[i] = SWAP16LE(DigiList[i]);
	}
#endif

	for (i = 0;i < sdLastSound;i++)
		DigiMap[i] = -1;
}
#endif

// 	AdLib Code

///////////////////////////////////////////////////////////////////////////
//
//	alOut(n,b) - Puts b in AdLib card register n
//
///////////////////////////////////////////////////////////////////////////

void
alOut(byte n,byte b)
{
#ifdef __AMIGA__
	BE_ST_ALOut(n,b);
#else
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
	// From Wolfenstein 3D
#ifdef GAMEVER_RESTORATION_AOG_100
asm	pushf
asm	cli

asm	mov	dx,0x388
asm	mov	al,[n]
asm	out	dx,al
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	inc	dx
asm	mov	al,[b]
asm	out	dx,al

asm	popf

asm	dec	dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx

asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx

asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx

asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
asm	in	al,dx
#else
	int16_t	i;

asm	pushf
asm	cli

	outportb(alFMAddr,n);
	for (i = 0;i < 6;i++)
		inportb(alFMStatus);
	outportb(alFMData,b);

asm	popf

	for (i = 0;i < 35;i++)
		inportb(alFMStatus);
#endif
#endif
}

#if 0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_SetInstrument() - Puts an instrument into a generator
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_SetInstrument(int16_t track,int16_t which,Instrument far *inst,boolean percussive)
{
	byte		c,m;

	if (percussive)
	{
		c = pcarriers[which];
		m = pmodifiers[which];
	}
	else
	{
		c = carriers[which];
		m = modifiers[which];
	}

	tracks[track - 1]->inst = *inst;
	tracks[track - 1]->percussive = percussive;

	alOut(m + alChar,inst->mChar);
	alOut(m + alScale,inst->mScale);
	alOut(m + alAttack,inst->mAttack);
	alOut(m + alSus,inst->mSus);
	alOut(m + alWave,inst->mWave);

	// Most percussive instruments only use one cell
	if (c != 0xff)
	{
		alOut(c + alChar,inst->cChar);
		alOut(c + alScale,inst->cScale);
		alOut(c + alAttack,inst->cAttack);
		alOut(c + alSus,inst->cSus);
		alOut(c + alWave,inst->cWave);
	}

	alOut(which + alFeedCon,inst->nConn);	// DEBUG - I think this is right
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ALStopSound() - Turns off any sound effects playing through the
//		AdLib card
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_ALStopSound(void)
{
#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	alSound = 0;
	alOut(alFreqH + 0,0);

#ifndef __AMIGA__
asm	popf
#endif
}

static void
SDL_AlSetFXInst(Instrument far *inst)
{
	byte		c,m;

	m = modifiers[0];
	c = carriers[0];
	alOut(m + alChar,inst->mChar);
	alOut(m + alScale,inst->mScale);
	alOut(m + alAttack,inst->mAttack);
	alOut(m + alSus,inst->mSus);
	alOut(m + alWave,inst->mWave);
	alOut(c + alChar,inst->cChar);
	alOut(c + alScale,inst->cScale);
	alOut(c + alAttack,inst->cAttack);
	alOut(c + alSus,inst->cSus);
	alOut(c + alWave,inst->cWave);

	// Note: Switch commenting on these lines for old MUSE compatibility
//	alOut(alFeedCon,inst->nConn);
	alOut(alFeedCon,0);
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ALPlaySound() - Plays the specified sound on the AdLib card
//
///////////////////////////////////////////////////////////////////////////
#ifdef	_MUSE_
void
#else
static void
#endif
SDL_ALPlaySound(AdLibSound far *sound)
{
	Instrument	far *inst;
	byte		huge *data;

	SDL_ALStopSound();

#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	alLengthLeft = sound->common.length;
	data = sound->data;
	data++;
	data--;
	alSound = (byte far *)data;
	alBlock = ((sound->block & 7) << 2) | 0x20;
	inst = &sound->inst;

	if (!(inst->mSus | inst->cSus))
	{
#ifndef __AMIGA__
	asm	popf
#endif
		SD_ERROR(SDL_ALPLAYSOUND_BAD_INST);
	}

	SDL_AlSetFXInst(&alZeroInst);	// DEBUG
	SDL_AlSetFXInst(inst);

#ifndef __AMIGA__
asm	popf
#endif
}

#if defined(__AMIGA__) //0
///////////////////////////////////////////////////////////////////////////
//
// 	SDL_ALSoundService() - Plays the next sample out through the AdLib card
//
///////////////////////////////////////////////////////////////////////////
//static void
void
SDL_ALSoundService(void)
{
	byte	s;

	if (alSound)
	{
		s = *alSound++;
		if (!s)
			alOut(alFreqH + 0,0);
		else
		{
			alOut(alFreqL + 0,s);
			alOut(alFreqH + 0,alBlock);
		}

		if (!(--alLengthLeft))
		{
			//(int32_t)alSound = 0;
			alSound = 0;
			alOut(alFreqH + 0,0);
			SDL_SoundFinished();
		}
	}
}
#endif

#if defined(__AMIGA__) //0
void
SDL_ALService(void)
{
	byte	a,v;
	word	w;

	if (!sqActive)
		return;

	while (sqHackLen && (sqHackTime <= alTimeCount))
	{
		w = *sqHackPtr++;
		sqHackTime = alTimeCount + *sqHackPtr++;
#ifdef __AMIGA__
		a = *((byte *)&w);
		v = *((byte *)&w + 1);
#else
	asm	mov	dx,[w]
	asm	mov	[a],dl
	asm	mov	[v],dh
#endif
		alOut(a,v);
		sqHackLen -= 4;
	}
	alTimeCount++;
	if (!sqHackLen)
	{
		sqHackPtr = (word far *)sqHack;
		sqHackLen = sqHackSeqLen;
		alTimeCount = sqHackTime = 0;
#ifdef __AMIGA__
		sqPlayedOnce = true;
#endif
	}
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutAL() - Shuts down the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutAL(void)
{
#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	alOut(alEffects,0);
	alOut(alFreqH + 0,0);
	SDL_AlSetFXInst(&alZeroInst);
	alSound = 0;

#ifndef __AMIGA__
asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CleanAL() - Totally shuts down the AdLib card
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_CleanAL(void)
{
	int16_t	i;

#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	alOut(alEffects,0);
	for (i = 1;i < 0xf5;i++)
		alOut(i,0);

#ifndef __AMIGA__
asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartAL() - Starts up the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartAL(void)
{
	alFXReg = 0;
	alOut(alEffects,alFXReg);
	SDL_AlSetFXInst(&alZeroInst);
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_DetectAdLib() - Determines if there's an AdLib (or SoundBlaster
//		emulating an AdLib) present
//
///////////////////////////////////////////////////////////////////////////
GAMEVER_RESTORATION_CONDSTATIC boolean
SDL_DetectAdLib(void)
{
#ifdef __AMIGA__
	return true;
#else
	byte	status1,status2;
	int16_t		i;

	alOut(4,0x60);	// Reset T1 & T2
	alOut(4,0x80);	// Reset IRQ
	status1 = readstat();
	alOut(2,0xff);	// Set timer 1
	alOut(4,0x21);	// Start timer 1
#if 0
	SDL_Delay(TimerDelay100);
#else
asm	mov	dx,alFMStatus
asm	mov	cx,100
usecloop:
asm	in	al,dx
asm	loop usecloop
#endif

	status2 = readstat();
	alOut(4,0x60);
	alOut(4,0x80);

	if (((status1 & 0xe0) == 0x00) && ((status2 & 0xe0) == 0xc0))
	{
		for (i = 1;i <= 0xf5;i++)	// Zero all the registers
			alOut(i,0);

		alOut(1,0x20);	// Set WSE=1
		alOut(8,0);		// Set CSM=0 & SEL=0

		return(true);
	}
	else
#endif
		return(false);
}

#if defined(__AMIGA__)//0
///////////////////////////////////////////////////////////////////////////
//
//	SDL_t0Service() - My timer 0 ISR which handles the different timings and
//		dispatches to whatever other routines are appropriate
//
///////////////////////////////////////////////////////////////////////////
static void interrupt
SDL_t0Service(void)
{
static	word	count = 1;

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,4	// red
asm	out	dx,al
#endif

	HackCount++;

	if ((MusicMode == smm_AdLib) || (DigiMode == sds_SoundSource))
	{
		SDL_ALService();
		SDL_SSService();
//		if (!(++count & 7))
		if (!(++count % 10))
		{
			LocalTime++;
			TimeCount++;
			if (SoundUserHook)
				SoundUserHook();
		}
//		if (!(count & 3))
		if (!(count % 5))
		{
			switch (SoundMode)
			{
			case sdm_PC:
				SDL_PCService();
				break;
			case sdm_AdLib:
				SDL_ALSoundService();
				break;
			}
		}
	}
	else
	{
		if (!(++count & 1))
		{
			LocalTime++;
			TimeCount++;
			if (SoundUserHook)
				SoundUserHook();
		}
		switch (SoundMode)
		{
		case sdm_PC:
			SDL_PCService();
			break;
		case sdm_AdLib:
			SDL_ALSoundService();
			break;
		}
	}

#ifdef __AMIGA__
	//TimerCount += TimerDivisor;
#else
asm	mov	ax,[WORD PTR TimerCount]
asm	add	ax,[WORD PTR TimerDivisor]
asm	mov	[WORD PTR TimerCount],ax
asm	jnc	myack
	t0OldService();			// If we overflow a word, time to call old int16_t handler
asm	jmp	olddone
myack:;
	outportb(0x20,0x20);	// Ack the interrupt
olddone:;
#endif

#if 0	// for debugging
asm	mov	dx,STATUS_REGISTER_1
asm	in	al,dx
asm	mov	dx,ATR_INDEX
asm	mov	al,ATR_OVERSCAN
asm	out	dx,al
asm	mov	al,3	// blue
asm	out	dx,al
asm	mov	al,0x20	// normal
asm	out	dx,al
#endif
}
#endif

////////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutDevice() - turns off whatever device was being used for sound fx
//
////////////////////////////////////////////////////////////////////////////
static void
SDL_ShutDevice(void)
{
	switch (SoundMode)
	{
	case sdm_PC:
		SDL_ShutPC();
		break;
	case sdm_AdLib:
#ifdef __AMIGA__
		BEL_ST_FreeDigiSounds();
#endif
		SDL_ShutAL();
		break;
	}
	SoundMode = sdm_Off;
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CleanDevice() - totally shuts down all sound devices
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_CleanDevice(void)
{
	if ((SoundMode == sdm_AdLib) || (MusicMode == smm_AdLib))
		SDL_CleanAL();
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartDevice() - turns on whatever device is to be used for sound fx
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartDevice(void)
{
	switch (SoundMode)
	{
	case sdm_AdLib:
#ifdef __AMIGA__
		BEL_ST_LoadDigiSounds();
#endif
		SDL_StartAL();
		break;
	}
	SoundNumber = SoundPriority = 0;
}

//	Public routines

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_SetSoundMode(SDMode mode)
{
	boolean	result = false;
	word	tableoffset;

	SD_StopSound();

#ifndef	_MUSE_
	if ((mode == sdm_AdLib) && !AdLibPresent)
		mode = sdm_PC;

	switch (mode)
	{
	case sdm_Off:
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
		tableoffset = sdStartPCSounds;
#endif
		NeedsDigitized = false;
		result = true;
		break;
	case sdm_PC:
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		tableoffset = STARTPCSOUNDS;
#else
		tableoffset = sdStartPCSounds;
#endif
		NeedsDigitized = false;
		result = true;
		break;
	case sdm_AdLib:
		if (AdLibPresent)
		{
			/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
			tableoffset = STARTADLIBSOUNDS;
#else
			tableoffset = sdStartALSounds;
#endif
			NeedsDigitized = false;
			result = true;
		}
		break;
	}
#else
	result = true;
#endif

	if (result && (mode != SoundMode))
	{
		SDL_ShutDevice();
		SoundMode = mode;
#ifndef	_MUSE_
		//SoundTable = (word *)(&audiosegs[tableoffset]);
		SoundTable = (SoundCommon **)&audiosegs[tableoffset];
#endif
		SDL_StartDevice();
	}

	SDL_SetTimerSpeed();

	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_SetMusicMode(SMMode mode)
{
	boolean	result = false;

	SD_FadeOutMusic();
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	while (SD_MusicPlaying())
#ifdef __AMIGA__
		//VL_WaitVBL(1)
		BE_ST_ShortSleep()
#endif
		;
#endif

	switch (mode)
	{
	case smm_Off:
		NeedsMusic = false;
		result = true;
#ifdef __AMIGA__
		BE_ST_ShutMusic();
#endif
		break;
	case smm_AdLib:
		if (AdLibPresent)
		{
			NeedsMusic = true;
			result = true;
		}
		break;
	}

	if (result)
		MusicMode = mode;

	SDL_SetTimerSpeed();

	return(result);
}

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// - SD_Startup was originally relocated to JM_FREE.C in AOG v1.0, although
// SoundUserHook is *not* set (probably since it's an unused static variable).
// - SD_Default is also gone from v1.0, although it's also unused
// in other versions.
#ifndef GAMEVER_RESTORATION_AOG_100
///////////////////////////////////////////////////////////////////////////
//
//	SD_Startup() - starts up the Sound Mgr
//		Detects all additional sound hardware and installs my ISR
//
///////////////////////////////////////////////////////////////////////////
void
SD_Startup(void)
{
	int16_t	i;

	if (SD_Started)
		return;

	SDL_SetDS();

	ssIsTandy = false;
	ssNoCheck = false;
	alNoCheck = false;
	sbNoCheck = false;
	sbNoProCheck = false;
#ifndef	_MUSE_
	for (i = 1;i < _argc;i++)
	{
		switch (US_CheckParm(_argv[i],ParmStrings))
		{
		case 0:						// No AdLib detection
			alNoCheck = true;
			break;
		case 1:						// No SoundBlaster detection
			sbNoCheck = true;
			break;
		case 2:						// No SoundBlaster Pro detection
			sbNoProCheck = true;
			break;
		case 3:
			ssNoCheck = true;		// No Sound Source detection
			break;
		case 4:						// Tandy Sound Source handling
			ssIsTandy = true;
			break;
		case 5:						// Sound Source present at LPT1
			ssPort = 1;
			ssNoCheck = SoundSourcePresent = true;
			break;
		case 6:                     // Sound Source present at LPT2
			ssPort = 2;
			ssNoCheck = SoundSourcePresent = true;
			break;
		case 7:                     // Sound Source present at LPT3
			ssPort = 3;
			ssNoCheck = SoundSourcePresent = true;
			break;
		}
	}
#endif

	SoundUserHook = 0;

	//t0OldService = getvect(8);	// Get old timer 0 ISR

	LocalTime = TimeCount = alTimeCount = 0;

	SD_SetSoundMode(sdm_Off);
	SD_SetMusicMode(smm_Off);

	if (!ssNoCheck)
		SoundSourcePresent = SDL_DetectSoundSource();

	if (!alNoCheck)
	{
		AdLibPresent = SDL_DetectAdLib();
		if (AdLibPresent && !sbNoCheck)
		{
			int16_t port = -1;
			char *env = getenv("BLASTER");
			if (env)
			{
				int32_t temp;
				while (*env)
				{
					while (isspace(*env))
						env++;

					switch (toupper(*env))
					{
					case 'A':
						temp = strtol(env + 1,&env,16);
						if
						(
							(temp >= 0x210)
						&&	(temp <= 0x280)
						&&	(!(temp & 0x00f))
						)
							port = (temp - 0x200) >> 4;
						else
							SD_ERROR(SD_STARTUP_BAD_ADDRESS);
						break;
					case 'I':
						temp = strtol(env + 1,&env,10);
						if
						(
							(temp >= 0)
						&&	(temp <= 10)
						&&	(sbIntVectors[temp] != -1)
						)
						{
							sbInterrupt = temp;
							sbIntVec = sbIntVectors[sbInterrupt];
						}
						else
							SD_ERROR(SD_STARTUP_BAD_INTERRUPT);
						break;
					case 'D':
						temp = strtol(env + 1,&env,10);
						if ((temp == 0) || (temp == 1) || (temp == 3))
							SDL_SBSetDMA(temp);
						else
							SD_ERROR(SD_STARTUP_BAD_DMA);
						break;
					default:
						while (isspace(*env))
							env++;
						while (*env && !isspace(*env))
							env++;
						break;
					}
				}
			}
			SoundBlasterPresent = SDL_DetectSoundBlaster(port);
		}
	}

	for (i = 0;i < 255;i++)
		pcSoundLookup[i] = i * 60;

	if (SoundBlasterPresent)
		SDL_StartSB();

	SDL_SetupDigi();

	SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Default() - Sets up the default behaviour for the Sound Mgr whether
//		the config file was present or not.
//
///////////////////////////////////////////////////////////////////////////
void
SD_Default(boolean gotit,SDMode sd,SMMode sm)
{
	boolean	gotsd,gotsm;

	gotsd = gotsm = gotit;

	if (gotsd)	// Make sure requested sound hardware is available
	{
		switch (sd)
		{
		case sdm_AdLib:
			gotsd = AdLibPresent;
			break;
		}
	}
	if (!gotsd)
	{
		if (AdLibPresent)
			sd = sdm_AdLib;
		else
			sd = sdm_PC;
	}
	if (sd != SoundMode)
		SD_SetSoundMode(sd);


	if (gotsm)	// Make sure requested music hardware is available
	{
		switch (sm)
		{
		case sdm_AdLib:
			gotsm = AdLibPresent;
			break;
		}
	}
	if (!gotsm)
	{
		if (AdLibPresent)
			sm = smm_AdLib;
	}
	if (sm != MusicMode)
		SD_SetMusicMode(sm);
}
#endif // GAMEVER_RESTORATION_AOG_100

///////////////////////////////////////////////////////////////////////////
//
//	SD_Shutdown() - shuts down the Sound Mgr
//		Removes sound ISR and turns off whatever sound hardware was active
//
///////////////////////////////////////////////////////////////////////////
void
SD_Shutdown(void)
{
	if (!SD_Started)
		return;

	SD_MusicOff();
	SD_StopSound();
	SDL_ShutDevice();
	SDL_CleanDevice();

	if (SoundBlasterPresent)
		SDL_ShutSB();

	if (SoundSourcePresent)
		SDL_ShutSS();

#ifndef __AMIGA__
	asm	pushf
	asm	cli
#endif

	SDL_SetTimer0(0);

#ifdef __AMIGA__
	BE_ST_StopAudioSDService();
#else
	setvect(8,t0OldService);

	asm	popf
#endif

	SD_Started = false;
}

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
///////////////////////////////////////////////////////////////////////////
//
//	SD_SetUserHook() - sets the routine that the Sound Mgr calls every 1/70th
//		of a second from its timer 0 ISR
//
///////////////////////////////////////////////////////////////////////////
void
SD_SetUserHook(void (* hook)(void))
{
	SoundUserHook = hook;
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	SD_PositionSound() - Sets up a stereo imaging location for the next
//		sound to be played. Each channel ranges from 0 to 15.
//
///////////////////////////////////////////////////////////////////////////
void
SD_PositionSound(int16_t leftvol,int16_t rightvol)
{
	LeftPosition = leftvol;
	RightPosition = rightvol;
	nextsoundpos = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_PlaySound(soundnames sound)
{
	boolean		ispos;
	SoundCommon	far *s;
	int16_t	lp,rp;

	lp = LeftPosition;
	rp = RightPosition;
	LeftPosition = 0;
	RightPosition = 0;

	ispos = nextsoundpos;
	nextsoundpos = false;

	if (sound == -1)
		return(false);

	//s = (void *)MK_FP((size_t)SoundTable[sound],0);
	s = (SoundCommon *)(SoundTable[sound]);
	if ((SoundMode != sdm_Off) && !s)
		SD_ERROR(SD_PLAYSOUND_UNCACHED);

	if ((DigiMode != sds_Off) && (DigiMap[sound] != -1))
	{
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
		if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
		{
			if (s->priority < SoundPriority)
				return(false);

			SDL_PCStopSound();

			SD_PlayDigitized(DigiMap[sound],lp,rp);
			SoundPositioned = ispos;
			SoundNumber = sound;
			SoundPriority = s->priority;
		}
		else
#endif
		{
#ifndef __AMIGA__
		asm	pushf
		asm	cli
#endif
			if (DigiPriority && !DigiNumber)
			{
#ifndef __AMIGA__
			asm	popf
#endif
				SD_ERROR(SD_PLAYSOUND_PRI_NO_SOUND);
			}
#ifndef __AMIGA__
		asm	popf
#endif

			if (s->priority < DigiPriority)
				return(false);

			SD_PlayDigitized(DigiMap[sound],lp,rp);
			SoundPositioned = ispos;
			DigiNumber = sound;
			DigiPriority = s->priority;
		}

		return(true);
	}

	if (SoundMode == sdm_Off)
		return(false);
	if (!s->length)
		SD_ERROR(SD_PLAYSOUND_ZERO_LEN);
	if (s->priority < SoundPriority)
		return(false);

	switch (SoundMode)
	{
	case sdm_PC:
		SDL_PCPlaySound((void far *)s);
		break;
	case sdm_AdLib:
#ifdef __AMIGA__
		BE_ST_PlaySound(sound);
#endif
		SDL_ALPlaySound((void far *)s);
		break;
	}

	SoundNumber = sound;
	SoundPriority = s->priority;

	return(false);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//		no sound is playing
//
///////////////////////////////////////////////////////////////////////////
word
SD_SoundPlaying(void)
{
	boolean	result = false;

	switch (SoundMode)
	{
	case sdm_PC:
		result = pcSound? true : false;
		break;
	case sdm_AdLib:
		result = alSound? true : false;
		break;
	}

	if (result)
		return(SoundNumber);
	else
		return(false);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////
void
SD_StopSound(void)
{
	if (DigiPlaying)
		SD_StopDigitized();

	switch (SoundMode)
	{
	case sdm_PC:
		SDL_PCStopSound();
		break;
	case sdm_AdLib:
#ifdef __AMIGA__
		BE_ST_StopSound();
#endif
		SDL_ALStopSound();
		break;
	}

	SoundPositioned = false;

	SDL_SoundFinished();
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////
void
SD_WaitSoundDone(void)
{
	while (SD_SoundPlaying())
#ifdef __AMIGA__
		//VL_WaitVBL(1)
		BE_ST_ShortSleep()
#endif
		;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////
void
SD_MusicOn(void)
{
	sqActive = true;
#ifdef __AMIGA__
	switch (MusicMode)
	{
	case smm_AdLib:
		BE_ST_MusicOn();
		break;
	}
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOff() - turns off the sequencer and any playing notes
//
///////////////////////////////////////////////////////////////////////////
void
SD_MusicOff(void)
{
	word	i;

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	sqActive = false;
#endif
	switch (MusicMode)
	{
	case smm_AdLib:
		alFXReg = 0;
		alOut(alEffects,0);
		for (i = 0;i < sqMaxTracks;i++)
			alOut(alFreqH + i + 1,0);
#ifdef __AMIGA__
		BE_ST_MusicOff();
#endif
		break;
	}
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	sqActive = false;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////
void
SD_StartMusic(MusicGroup far *music)
{
	SD_MusicOff();
#ifndef __AMIGA__
asm	pushf
asm	cli
#endif

	sqPlayedOnce = false;

	if (MusicMode == smm_AdLib)
	{
		sqHackPtr = sqHack = music->values;
		sqHackSeqLen = sqHackLen = music->length;
		sqHackTime = 0;
		alTimeCount = 0;
#ifdef __AMIGA__
	int i = 0;
#ifndef STARTMUSIC
#define STARTMUSIC 300
#endif
	for (i = 0; i < LASTMUSIC; i++)
	{
		if (music == audiosegs[STARTMUSIC+i])
			break;
	}
	if (i < LASTMUSIC)
	{
		//printf("%s found music chunk %d %p\n", __FUNCTION__, i, music);
		BE_ST_PlayMusic(i);
	}
#endif
		SD_MusicOn();
	}
	else
		sqPlayedOnce = true;

#ifndef __AMIGA__
asm	popf
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_FadeOutMusic() - starts fading out the music. Call SD_MusicPlaying()
//		to see if the fadeout is complete
//
///////////////////////////////////////////////////////////////////////////
void
SD_FadeOutMusic(void)
{
	switch (MusicMode)
	{
	case smm_AdLib:
		// DEBUG - quick hack to turn the music off
		SD_MusicOff();
		break;
	}
}

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicPlaying() - returns true if music is currently playing, false if
//		not
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_MusicPlaying(void)
{
	boolean	result;

	switch (MusicMode)
	{
	case smm_AdLib:
		result = false;
		// DEBUG - not written
		break;
	default:
		result = false;
	}

	return(result);
}
#endif

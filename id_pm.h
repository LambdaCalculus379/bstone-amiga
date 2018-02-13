//
//	ID_PM.H
//	Header file for Id Engine's Page Manager
//

//	NOTE! PMPageSize must be an even divisor of EMSPageSize, and >= 1024
#define	EMSPageSize		16384
#define	EMSPageSizeSeg	(EMSPageSize >> 4)
#define	EMSPageSizeKB	(EMSPageSize >> 10)
#define	EMSFrameCount	4
#define	PMPageSize		4096
#define	PMPageSizeSeg	(PMPageSize >> 4)
#define	PMPageSizeKB	(PMPageSize >> 10)
#define	PMEMSSubPage	(EMSPageSize / PMPageSize)

#define	PMMinMainMem	10			// Min acceptable # of pages from main
#define	PMMaxMainMem	100			// Max number of pages in main memory

#define	PMThrashThreshold	1	// Number of page thrashes before panic mode
#define	PMUnThrashThreshold	5	// Number of non-thrashing frames before leaving panic mode

typedef	enum
		{
			pml_Unlocked,
			pml_Locked
		} PMLockType;

typedef	enum
		{
			pmba_Unused = 0,
			pmba_Used = 1,
			pmba_Allocated = 2
		} PMBlockAttr;


typedef enum
{
	sd_NONE,
   sd_NO_SHADOWS,
   sd_SHADOWS,

} shadfile_t;

typedef	struct
		{
			longword	offset;		// Offset of chunk into file
			word		length;		// Length of the chunk

			int16_t			xmsPage;	// If in XMS, (xmsPage * PMPageSize) gives offset into XMS handle

			PMLockType	locked;		// If set, this page can't be purged
			int16_t			emsPage;	// If in EMS, logical page/offset into page
			int16_t			mainPage;	// If in Main, index into handle array

			longword	lastHit;	// Last frame number of hit
		} PageListStruct;

typedef	struct
		{
			int16_t			baseEMSPage;	// Base EMS page for this phys frame
			longword	lastHit;		// Last frame number of hit
		} EMSListStruct;

extern	boolean			XMSPresent,EMSPresent,PageManagerInstalled;
extern	word			XMSPagesAvail,EMSPagesAvail;

extern	word			ChunksInFile,
						PMSpriteStart,PMSoundStart;
extern	PageListStruct	far *PMPages;

#define	PM_GetSoundPage(v)	PM_GetPage(PMSoundStart + (v))
#define	PM_GetSpritePage(v)	PM_GetPage(PMSpriteStart + (v))

#define	PM_LockMainMem()	PM_SetMainMemPurge(0)
#define	PM_UnlockMainMem()	PM_SetMainMemPurge(3)


extern	char	PageFileName[13];
extern   char			AltPageFileName[13];
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
extern 	boolean ShadowsAvail;
extern   shadfile_t 		FileUsed;
#endif


extern	void	PM_Startup(void),
				PM_Shutdown(void),
				PM_Reset(void),
				PM_Preload(void (*update)(word current,word total)),
				PM_NextFrame(void),
				PM_SetPageLock(int16_t pagenum,PMLockType lock),
				PM_SetMainPurge(int16_t level),
				PM_CheckMainMem(void);
extern	memptr	PM_GetPageAddress(int16_t pagenum),
				PM_GetPage(int16_t pagenum);		// Use this one to cache page

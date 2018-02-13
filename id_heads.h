#ifndef _ID_HEADS_H_
#define _ID_HEADS_H_

// ID_GLOB.H


//#include <ALLOC.H>
#include <ctype.h>
//#include <DOS.H>
#include <errno.h>
#include <fcntl.h>
//#include <IO.H>
//#include <MEM.H>
//#include <PROCESS.H>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
//#include <VALUES.H>
//#include <DIR.H>
#ifdef __AMIGA__
#include <stdint.h>
#ifdef __linux__
#include <endian.h>
#else
#include <machine/endian.h>
#endif
#include <unistd.h>
// TODO move this into a separate header?
#define SWAP16(x) (uint16_t)(((uint16_t)(x) & 0xff) << 8 | ((uint16_t)(x) & 0xff00) >> 8)
#define SWAP32(x) (uint32_t)(((uint32_t)(x) & 0xff) << 24 | ((uint32_t)(x) & 0xff00) << 8 | \
					((uint32_t)(x) & 0xff0000) >> 8 | ((uint32_t)(x) & 0xff000000) >> 24)

#if BYTE_ORDER == BIG_ENDIAN
#define SWAP16LE(x) SWAP16((x))
#define SWAP32LE(x) SWAP32((x))
#define SWAP16BE(x) (x)
#define SWAP32BE(x) (x)
#else
#define SWAP16LE(x) (x)
#define SWAP32LE(x) (x)
#define SWAP16BE(x) SWAP16((x))
#define SWAP32BE(x) SWAP32((x))
#endif
#define MAXLONG INT32_MAX
#include "findfirst.h"
#define far
#define _seg
#define huge
#define interrupt
#define _fmemset memset
#define _fstrstr strstr
#define _fmemchr memchr
#define _fmemcpy memcpy
#define _fstrcat strcat
#define _fstrcpy strcpy
#define _fstrlen strlen
#define _fmemcmp memcmp
#define _fstricmp strcasecmp
#define _fmemicmp memicmp
char *ultoa(unsigned long value, char *str, int radix);
char *ltoa(long value, char *str, int radix);
char *itoa(int value, char *str, int radix);
/*
#define itoa(num, str, base) ({sprintf(str, "%d", (num)); str;})
#define ltoa(num, str, base) ({sprintf(str, "%d", (num)); str;})
#define ultoa(num, str, base) ({sprintf(str, "%u", (num)); str;})
*/
#ifndef O_BINARY
#define O_BINARY 0
#endif
long tell(int handle);
long filelength(int handle);
int memicmp(const void *s1, const void *s2, size_t n);
//#define chsize ftruncate
int chsize(int fd, long length);
//#define seek lseek
//#define MK_FP(seg, off) ((char *)seg + off)
/*#define outportb(n,b)
#define outp(a,b);
#define inportb(n) 0
#define getvect(x) NULL
#define setvect(x,y)*/
extern int _argc;
extern char **_argv;
#endif
#define __ID_GLOB__
/*** BLAKE STONE VERSIONS RESTORATION ***/
#include "gamever.h"

//--------------------------------------------------------------------------

#define DEBUG_VALUE
#define CEILING_FLOOR_COLORS


//#define CARMACIZED
#define WOLF
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
#define FREE_FUNCTIONS					(true)
#else
#define FREE_FUNCTIONS					(false)
#endif
#define FREE_DATA							(false)
#ifdef __AMIGA__
#define DEMOS_ENABLED					(1)
#else
#define DEMOS_ENABLED					(true)
#endif
#define RESTART_PICTURE_PAUSE			(false)
#define GEORGE_CHEAT						(false)

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// A hack for disabling some ID_CA related calls not done in v1.0
#ifdef GAMEVER_RESTORATION_AOG_100
#define FORCE_FILE_CLOSE				(false)
#else
#define FORCE_FILE_CLOSE				(true)		// true - forces all files closed once they are used
#endif

//
// GAME VERSION TYPES
//

#define SHAREWARE_VERSION			0x0001
#define MISSIONS_1_THR_3			0x0003
#define MISSIONS_4_THR_6			0x0004
#define MISSIONS_1_THR_6			0x0007

//
// CURRENT GAME VERSION DEFINE - Change this define according to the
//											game release versions 1,1-3,4-6, or 1-6.

/*** BLAKE STONE VERSIONS RESTORATION ***/
#ifdef GAMEVER_RESTORATION_BS1
#define GAME_VERSION					(SHAREWARE_VERSION)
#else
#define GAME_VERSION					(MISSIONS_1_THR_6)
#endif
//#define GAME_VERSION					(MISSIONS_1_THR_6)
//#define GAME_VERSION      			(MISSIONS_1_THR_3)
//#define GAME_VERSION					(SHAREWARE_VERSION)


#define TECH_SUPPORT_VERSION		(false)
#define IN_DEVELOPMENT				(false)

#define ERROR_LOG						"ERROR.LOG"			// Text filename for critical memory errors
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
#define DUAL_SWAP_FILES				(GAME_VERSION != SHAREWARE_VERSION)				// Support for Shadowed and NonShadowed page files
#else
#define DUAL_SWAP_FILES				(false)				//(GAME_VERSION != SHAREWARE_VERSION)				// Support for Shadowed and NonShadowed page files
#endif

extern  char            far signonv1;
#define introscn        signonv1

#ifndef SPEAR

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
#include "gfxv_bs1.h"
#include "audiobs1.h"
#include "mapsbs1.h"
#else
#include "gfxv_vsi.h"
#include "audiovsi.h"
#include "mapsvsi.h"
#endif

#else

#include "gfxv_sod.h"
#include "audiosod.h"
#include "mapssod.h"

#endif

//-----------------


#define GREXT   "VGA"

//
//      ID Engine
//      Types.h - Generic types, #defines, etc.
//      v1.0d1
//

typedef enum    {false,true}    boolean;
typedef uint8_t                    byte;
typedef uint16_t                     word;
typedef uint32_t            longword;
typedef byte *                                  Ptr;

typedef struct
		{
			int16_t     x,y;
		} Point;
typedef struct
		{
			Point   ul,lr;
		} Rect;

#define nil     (0l)


#include "id_mm.h"
#include "id_pm.h"
#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"

#include "jm_tp.h"
#include "jm_debug.h"
#include "jm_error.h"

#include "movie.h"

void    Quit (char *error,...);             // defined in user program

extern void CalcMemFree(void);

//
// replacing refresh manager with custom routines
//

#define PORTTILESWIDE           20      // all drawing takes place inside a
#define PORTTILESHIGH           13              // non displayed port of this size

#define UPDATEWIDE                      PORTTILESWIDE
#define UPDATEHIGH                      PORTTILESHIGH

#define MAXTICS                         10
#define DEMOTICS                        4

#define UPDATETERMINATE 0x0301

extern  uint16_t        mapwidth,mapheight,tics,realtics;
extern  boolean         compatability;

extern  byte            *updateptr;
extern  uint16_t        uwidthtable[UPDATEHIGH];
extern  uint16_t        blockstarts[UPDATEWIDE*UPDATEHIGH];

extern  byte            fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;

#include "3d_menu.h"


#define CA_FarRead(h,d,s)	IO_FarRead((int)h,(byte far *)d,(int32_t)s)




// BBi
//#define UPDATESIZE (UPDATEWIDE * UPDATEHIGH)
//extern uint8_t update[UPDATESIZE];
//extern uint8_t* vga_memory;
extern uint8_t vga_memory[256 * 1024];
// BBi

#endif

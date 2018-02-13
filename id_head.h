// ID_HEAD.H

/*** BLAKE STONE VERSIONS RESTORATION ***/
#include "gamever.h"

#define WOLF
#define FREE_FUNCTIONS   (true)

#define	TEXTGR	0
#define	CGAGR	1
#define	EGAGR	2
#define	VGAGR	3

#define GRMODE	VGAGR

#include <stdint.h>
typedef	enum	{false,true}	boolean;
typedef	uint8_t		byte;
typedef	uint16_t			word;
typedef	uint32_t		longword;
typedef	byte *					Ptr;
#define far
#define _fmemcpy memcpy

typedef	struct
		{
			int16_t	x,y;
		} Point;

typedef	struct
		{
			Point	ul,lr;
		} Rect;


void	Quit (char *error,...);		// defined in user program


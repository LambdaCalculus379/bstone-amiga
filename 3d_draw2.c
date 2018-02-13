// WOLFHACK.C

#include "3d_def.h"

#define	MAXVIEWHEIGHT	200
#ifdef __AMIGA__
#define  GAMESTATE_TEST 	(1) // TODO HACK!!!
#else
#define  GAMESTATE_TEST 	(true)
#endif


uint16_t CeilingTile=126, FloorTile=126;

void (*MapRowPtr)();

int16_t  far	spanstart[MAXVIEWHEIGHT/2];		// jtr - far

fixed	stepscale[MAXVIEWHEIGHT/2];
fixed	basedist[MAXVIEWHEIGHT/2];

extern uint8_t	far	planepics[8192];	// 4k of ceiling, 4k of floor

int16_t		halfheight = 0;

#ifdef __AMIGA__
int16_t	planeylookup[MAXVIEWHEIGHT/2];
#else
byte	far *planeylookup[MAXVIEWHEIGHT/2];
#endif
uint16_t	far mirrorofs[MAXVIEWHEIGHT/2];

fixed	psin, pcos;

fixed FixedMul (fixed a, fixed b)
{
	return (a>>8)*(b>>8);
}


int		mr_rowofs;
int16_t		mr_count;
int16_t		mr_xstep;
int16_t		mr_ystep;
int16_t		mr_xfrac;
int16_t		mr_yfrac;
int		mr_dest;


/*
==============
=
= DrawSpans
=
= Height ranges from 0 (infinity) to viewheight/2 (nearest)
==============
*/
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
extern byte far * lightsource;
extern byte far * shadingtable;
#endif


void DrawSpans (int16_t x1, int16_t x2, int16_t height)
{
	fixed		length;
	int16_t			ofs;
	int16_t			prestep;
	fixed		startxfrac, startyfrac;

	int16_t			x, startx, count, plane, startplane;
#ifdef __AMIGA__
	int toprow;
#else
	byte		far	*toprow, far *dest;
#endif
	int32_t i;

#ifdef __AMIGA__
	toprow = planeylookup[height]+vl_get_offset(bufferofs, 0, 0);
#else
	toprow = planeylookup[height]+bufferofs;
#endif
	mr_rowofs = mirrorofs[height];

#ifdef __AMIGA__
	mr_xstep = psin / (2 * height);
	mr_ystep = pcos / (2 * height);
#else
	mr_xstep = (psin<<1)/height;
	mr_ystep = (pcos<<1)/height;
#endif

	length = basedist[height];
	startxfrac = (viewx + FixedMul(length,pcos));
	startyfrac = (viewy - FixedMul(length,psin));

// draw two spans simultaniously

	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	if (gamestate.flags & GS_LIGHTING)
	{

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	i=shade_max-(63l*(int32_t)height/(int32_t)normalshade);
#else
	i=shade_max-(63l*(uint32_t)height/(uint32_t)normalshade);
#endif
	if (i<0)
		i=0;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
   else
   	if (i>63)
      	i = 63;
#endif
	shadingtable=lightsource+(i<<8);
	plane = startplane = x1&3;
	prestep = viewwidth/2 - x1;
#ifdef __AMIGA__
		mr_xfrac = startxfrac - mr_xstep*prestep;
		mr_yfrac = startyfrac - mr_ystep*prestep;

		mr_dest = toprow + x1;
		mr_count = x2-x1 + 1;

		if (mr_count)
			MapRowPtr();
#else
	do
	{
		outportb (SC_INDEX+1,1<<plane);
		mr_xfrac = startxfrac - (mr_xstep>>2)*prestep;
		mr_yfrac = startyfrac - (mr_ystep>>2)*prestep;

		startx = x1>>2;
		mr_dest = (uint16_t)(size_t)toprow + startx;
		mr_count = ((x2-plane)>>2) - startx + 1;
		x1++;
		prestep--;

#if GAMESTATE_TEST
		if (mr_count)
			MapRowPtr();
#else
		if (mr_count)
			MapLSRow ();
#endif

		plane = (plane+1)&3;
	} while (plane != startplane);
#endif
	}
	else
#endif
	{
	prestep = viewwidth/2 - x1;
#ifdef __AMIGA__
		mr_xfrac = startxfrac - mr_xstep*prestep;
		mr_yfrac = startyfrac - mr_ystep*prestep;

		mr_dest = toprow + x1;
		mr_count = x2-x1 + 1;

		if (mr_count)
			MapRowPtr();
#else
	do
	{
		outportb (SC_INDEX+1,1<<plane);
		mr_xfrac = startxfrac - (mr_xstep>>2)*prestep;
		mr_yfrac = startyfrac - (mr_ystep>>2)*prestep;

		startx = x1>>2;
		mr_dest = (uint16_t)(size_t)toprow + startx;
		mr_count = ((x2-plane)>>2) - startx + 1;
		x1++;
		prestep--;

#if GAMESTATE_TEST
		if (mr_count)
			MapRowPtr();
#else
		if (mr_count)
			MapRow ();
#endif

		plane = (plane+1)&3;
	} while (plane != startplane);
#endif
	}
}




/*
===================
=
= SetPlaneViewSize
=
===================
*/

void SetPlaneViewSize (void)
{
	int16_t		x,y;
	byte 	far *dest, far *src;

	halfheight = viewheight>>1;


	for (y=0 ; y<halfheight ; y++)
	{
#ifdef __AMIGA__
		planeylookup[y] = (halfheight - 1 - y) * vga_width;
		mirrorofs[y] = (y * 2 + 1) * vga_width;
#else
		planeylookup[y] = (byte far *)0xa0000000l + (halfheight-1-y)*SCREENBWIDE;;
		mirrorofs[y] = (y*2+1)*SCREENBWIDE;
#endif

		stepscale[y] = y*GLOBAL1/32;
		if (y>0)
			basedist[y] = GLOBAL1/2*scale/y;
	}

	src = PM_GetPage(CeilingTile);
	dest = planepics;
	for (x=0 ; x<4096 ; x++)
	{
		*dest = *src++;
		dest += 2;
	}
	src = PM_GetPage(FloorTile);
	dest = planepics+1;
	for (x=0 ; x<4096 ; x++)
	{
		*dest = *src++;
		dest += 2;
	}

}


/*
===================
=
= DrawPlanes
=
===================
*/

void DrawPlanes (void)
{
	int16_t		height, lastheight;
	int16_t		x;

#if IN_DEVELOPMENT
	if (!MapRowPtr)
   	DRAW2_ERROR(NULL_FUNC_PTR_PASSED);
#endif


	if (viewheight>>1 != halfheight)
		SetPlaneViewSize ();		// screen size has changed


	psin = viewsin;
	if (psin < 0)
		psin = -(psin&0xffff);
	pcos = viewcos;
	if (pcos < 0)
		pcos = -(pcos&0xffff);

//
// loop over all columns
//
	lastheight = halfheight;

	for (x=0 ; x<viewwidth ; x++)
	{
		height = wallheight[x]>>3;
		if (height < lastheight)
		{	// more starts
			do
			{
				spanstart[--lastheight] = x;
			} while (lastheight > height);
		}
		else if (height > lastheight)
		{	// draw spans
			if (height > halfheight)
				height = halfheight;
			for ( ; lastheight < height ; lastheight++)
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
         	if (lastheight>0)
#endif
					DrawSpans (spanstart[lastheight], x-1, lastheight);
		}
	}
//return;
	height = halfheight;
	for ( ; lastheight < height ; lastheight++)
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
     	if (lastheight>0)
#endif
			DrawSpans (spanstart[lastheight], x-1, lastheight);
}


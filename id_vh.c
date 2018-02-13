// ID_VH.C

#include "id_heads.h"

#define	SCREENWIDTH		80
#define CHARWIDTH		2
#define TILEWIDTH		4
#define GRPLANES		4
#define BYTEPIXELS		4

#define SCREENXMASK		(~3)
#define SCREENXPLUS		(3)
#define SCREENXDIV		(4)

#define VIEWWIDTH		80

#define PIXTOBLOCK		4		// 16 pixels to an update block

#define UNCACHEGRCHUNK(chunk)	{MM_FreePtr(&grsegs[chunk]);grneeded[chunk]&=~ca_levelbit;}

//byte	update[UPDATEHIGH][UPDATEWIDE];	// MIKE this is the second declaration for this variable!?!?

//==========================================================================

pictabletype	_seg *pictable;
pictabletype   _seg *picmtable;			


int16_t	px,py;
byte	fontcolor,backcolor;
int16_t	fontnumber;
int16_t bufferwidth,bufferheight;
boolean allcaps = false;


//==========================================================================

void	VWL_UpdateScreenBlocks (void);

//==========================================================================

void VW_DrawPropString (char far *string)
{
	fontstruct	far	*font;
	int16_t		width,step,height,i;
	byte	far *source, far *dest, far *origdest;
	byte	ch,mask;

	font = (fontstruct far *)grsegs[STARTFONT+fontnumber];
	height = bufferheight = font->height;
#ifdef __AMIGA__
	dest = &vga_memory[vl_get_offset(bufferofs,px,py)];
#else
	dest = origdest = MK_FP(SCREENSEG,bufferofs+ylookup[py]+(px>>2));
	mask = 1<<(px&3);
#endif


	while ((ch = *string++)!=0)
	{
		width = step = font->width[ch];
		source = ((byte far *)font)+font->location[ch];

		while (width--)
		{
#ifdef __AMIGA__
			for (i=0; i<height; i++)
			{
				if (source[i*step] != 0) 
				{
					dest[i*vga_width]=fontcolor;
				}
			}
			source++;
			dest++;
			px++;
#else
			VGAMAPMASK(mask);

asm	mov	ah,[BYTE PTR fontcolor]
asm	mov	bx,[step]
asm	mov	cx,[height]
asm	mov	dx,[linewidth]
asm	lds	si,[source]
asm	les	di,[dest]

vertloop:
asm	mov	al,[si]
asm	or	al,al
asm	je	next
asm	mov	[es:di],ah			// draw color

next:
asm	add	si,bx
asm	add	di,dx
asm	loop	vertloop
asm	mov	ax,ss
asm	mov	ds,ax

			source++;
			px++;
			mask <<= 1;
			if (mask == 16)
			{
				mask = 1;
				dest++;
			}
#endif
		}
	}
#ifndef __AMIGA__
bufferheight = height;
bufferwidth = ((dest+1)-origdest)*4;
#endif
}


//==========================================================================

#if 0

/*
=================
=
= VL_MungePic
=
=================
*/

void VL_MungePic (byte far *source, uint16_t width, uint16_t height)
{
	uint16_t	x,y,plane,size,pwidth;
	byte		_seg *temp, far *dest, far *srcline;

	size = width*height;

	if (width&3)
		VH_ERROR(VL_MUNGEPIC_NO_DIV_FOUR);

//
// copy the pic to a temp buffer
//
	MM_GetPtr (&(memptr)temp,size);
	_fmemcpy (temp,source,size);

//
// munge it back into the original buffer
//
	dest = source;
	pwidth = width/4;

	for (plane=0;plane<4;plane++)
	{
		srcline = temp;
		for (y=0;y<height;y++)
		{
			for (x=0;x<pwidth;x++)
				*dest++ = *(srcline+x*4+plane);
			srcline+=width;
		}
	}

	MM_FreePtr (&(memptr)temp);
}

#endif

void VWL_MeasureString (char far *string, word *width, word *height
	, fontstruct _seg *font)
{
	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*((byte far *)string)];	// proportional width
}

void	VW_MeasurePropString (char far *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct _seg *)grsegs[STARTFONT+fontnumber]);
}

#if 0

void	VW_MeasureMPropString  (char far *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct _seg *)grsegs[STARTFONTM+fontnumber]);
}
#endif


/*
=============================================================================

				Double buffer management routines

=============================================================================
*/


/*
=======================
=
= VW_MarkUpdateBlock
=
= Takes a pixel bounded block and marks the tiles in bufferblocks
= Returns 0 if the entire block is off the buffer screen
=
=======================
*/

int16_t VW_MarkUpdateBlock (int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int16_t	x,y,xt1,yt1,xt2,yt2,nextline;
	byte *mark;

	xt1 = x1>>PIXTOBLOCK;
	yt1 = y1>>PIXTOBLOCK;

	xt2 = x2>>PIXTOBLOCK;
	yt2 = y2>>PIXTOBLOCK;

	if (xt1<0)
		xt1=0;
	else if (xt1>=UPDATEWIDE)
		return 0;

	if (yt1<0)
		yt1=0;
	else if (yt1>UPDATEHIGH)
		return 0;

	if (xt2<0)
		return 0;
	else if (xt2>=UPDATEWIDE)
		xt2 = UPDATEWIDE-1;

	if (yt2<0)
		return 0;
	else if (yt2>=UPDATEHIGH)
		yt2 = UPDATEHIGH-1;

	mark = updateptr + uwidthtable[yt1] + xt1;
	nextline = UPDATEWIDE - (xt2-xt1) - 1;

	for (y=yt1;y<=yt2;y++)
	{
		for (x=xt1;x<=xt2;x++)
			*mark++ = 1;			// this tile will need to be updated

		mark += nextline;
	}

	return 1;
}

void VWB_DrawTile8 (int16_t x, int16_t y, int16_t tile)
{
	if (VW_MarkUpdateBlock (x,y,x+7,y+7))
		LatchDrawChar(x,y,tile);
}

#if 0

void VWB_DrawTile8M (int16_t x, int16_t y, int16_t tile)
{
	if (VW_MarkUpdateBlock (x,y,x+7,y+7))
		VL_MemToScreen (((byte far *)grsegs[STARTTILE8M])+tile*64,8,8,x,y);
}
#endif


void VWB_DrawPic (int16_t x, int16_t y, int16_t chunknum)
{
	int16_t	picnum = chunknum - STARTPICS;
	uint16_t width,height;

	x &= ~7;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	if (VW_MarkUpdateBlock (x,y,x+width-1,y+height-1))
		VL_MemToScreen (grsegs[chunknum],width,height,x,y);
}


/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
//--------------------------------------------------------------------------
// VWB_DrawMPic()
//--------------------------------------------------------------------------
void VWB_DrawMPic (int16_t x, int16_t y, int16_t chunknum)
{
	int16_t	picnum = chunknum - STARTPICS;
	uint16_t width,height;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	if (VW_MarkUpdateBlock (x,y,x+width-1,y+height-1))
		VL_MaskMemToScreen (grsegs[chunknum],width,height,x,y,255);
}
#endif


void VWB_DrawPropString	 (char far *string)
{
	int16_t x;
	x=px;
	VW_DrawPropString (string);
	VW_MarkUpdateBlock(x,py,px-1,py+bufferheight-1);
}


void VWB_Bar (int16_t x, int16_t y, int16_t width, int16_t height, int16_t color)
{
	if (VW_MarkUpdateBlock (x,y,x+width,y+height-1) )
		VW_Bar (x,y,width,height,color);
}

void VWB_Plot (int16_t x, int16_t y, int16_t color)
{
	if (VW_MarkUpdateBlock (x,y,x,y))
		VW_Plot(x,y,color);
}

void VWB_Hlin (int16_t x1, int16_t x2, int16_t y, int16_t color)
{
	if (VW_MarkUpdateBlock (x1,y,x2,y))
		VW_Hlin(x1,x2,y,color);
}

void VWB_Vlin (int16_t y1, int16_t y2, int16_t x, int16_t color)
{
	if (VW_MarkUpdateBlock (x,y1,x,y2))
		VW_Vlin(y1,y2,x,color);
}


#if 0		/// MADE A MACRO
void VW_UpdateScreen (void)
{
	VH_UpdateScreen ();
}
#endif


/*
=============================================================================

						WOLFENSTEIN STUFF

=============================================================================
*/

/*
=====================
=
= LatchDrawPic
=
=====================
*/


void LatchDrawPic (uint16_t x, uint16_t y, uint16_t picnum)
{
	uint16_t wide, height, source;

	x <<= 3;
	wide = pictable[picnum-STARTPICS].width;
	height = pictable[picnum-STARTPICS].height;
	source = latchpics[2+picnum-LATCHPICS_LUMP_START];

	if (VW_MarkUpdateBlock (x,y,x+wide-1,y+height-1))
		VL_LatchToScreen (source,wide/4,height,x,y);
}


//==========================================================================

/*
===================
=
= LoadLatchMem
=
===================
*/

//uint16_t LatchMemFree = 0xffff;		
uint16_t	destoff;

void LoadLatchMem (void)
{
	int16_t	i,j,p,m,width,height;
	byte	far *src;
	uint16_t	picnum=0;


//
// tile 8s
//
	latchpics[picnum++] = freelatch;
	CA_CacheGrChunk (STARTTILE8);
	src = (byte _seg *)grsegs[STARTTILE8];
	destoff = freelatch;

	for (i=0;i<NUMTILE8;i++)
	{
		VL_MemToLatch (src,8,8,destoff);
		src += 64;
		destoff +=16;
	}
	UNCACHEGRCHUNK (STARTTILE8);

#if 0	// ran out of latch space!
//
// tile 16s
//
	src = (byte _seg *)grsegs[STARTTILE16];
	latchpics[picnum++] = destoff;

	for (i=0;i<NUMTILE16;i++)
	{
		CA_CacheGrChunk (STARTTILE16+i);
		src = (byte _seg *)grsegs[STARTTILE16+i];
		VL_MemToLatch (src,16,16,destoff);
		destoff+=64;
		if (src)
			UNCACHEGRCHUNK (STARTTILE16+i);
	}
#endif

//
// pics
//
	picnum++;
	for (i=LATCHPICS_LUMP_START;i<=LATCHPICS_LUMP_END;i++)	  
	{
		latchpics[picnum++] = destoff;
		CA_CacheGrChunk (i);
		width = pictable[i-STARTPICS].width;
		height = pictable[i-STARTPICS].height;
		VL_MemToLatch (grsegs[i],width,height,destoff);
		destoff += width/4 *height;
		UNCACHEGRCHUNK(i);
	}

	EGAMAPMASK(15);
}


//==========================================================================


/*
===================
=
= FizzleFade
=
= returns true if aborted
=
===================
*/

extern	ControlInfo	c;

boolean FizzleFade (uint16_t source, uint16_t dest,
	uint16_t width,uint16_t height, uint16_t frames, boolean abortable)
{
	int16_t			pixperframe;
	uint16_t	drawofs,pagedelta;
	byte 		mask,maskb[8] = {1,2,4,8};
	uint16_t	x,y,p,frame;
	//int32_t		rndval;
	uint32_t		rndval;

	pagedelta = dest-source;
	rndval = 1;
	y = 0;
	pixperframe = 64000/frames;

	IN_StartAck ();

	TimeCount=frame=0;
	do	// while (1)
	{ 
		if (abortable && IN_CheckAck () )
			return true;

#ifndef __AMIGA__
		asm	mov	es,[screenseg]
#endif

		for (p=0;p<pixperframe;p++)
		{
			//
			// seperate random value into x/y pair
			//
#ifdef __AMIGA__
			x = (rndval >> 8) & 0xFFFF;
			y = ((rndval & 0xFF) - 1) & 0xFF;
#else
			asm	mov	ax,[WORD PTR rndval]
			asm	mov	dx,[WORD PTR rndval+2]
			asm	mov	bx,ax
			asm	dec	bl
			asm	mov	[BYTE PTR y],bl			// low 8 bits - 1 = y xoordinate
			asm	mov	bx,ax
			asm	mov	cx,dx
			asm	mov	[BYTE PTR x],ah			// next 9 bits = x xoordinate
			asm	mov	[BYTE PTR x+1],dl
#endif
			//
			// advance to next random element
			//
#ifdef __AMIGA__
			boolean carry = ((rndval & 1) != 0);

			rndval >>= 1;

			if (carry)
				rndval ^= 0x00012000;
#else
			asm	shr	dx,1
			asm	rcr	ax,1
			asm	jnc	noxor
			asm	xor	dx,0x0001
			asm	xor	ax,0x2000
noxor:
			asm	mov	[WORD PTR rndval],ax
			asm	mov	[WORD PTR rndval+2],dx
#endif

			if (x>width || y>height)
				continue;
			drawofs = source+ylookup[y] + (x>>2);

			//
			// copy one pixel
			//
#ifdef __AMIGA__
			vga_memory[vl_get_offset(dest, x, y)] = vga_memory[vl_get_offset(source, x, y)];
			//vga_memory[vl_get_offset(drawofs, 0, 0)] = vga_memory[vl_get_offset(drawofs+pagedelta, 0, 0)];
#else
			mask = x&3;
			VGAREADMAP(mask);
			mask = maskb[mask];
			VGAMAPMASK(mask);

			asm	mov	di,[drawofs]
			asm	mov	al,[es:di]
			asm add	di,[pagedelta]
			asm	mov	[es:di],al
#endif

			if (rndval == 1)		// entire sequence has been completed
				return false;
		}
		frame++;
#ifdef __AMIGA__
		VL_RefreshScreen();
#endif
		while (TimeCount<frame)		// don't go too fast
#ifdef __AMIGA__
			//VL_WaitVBL(1)
			BE_ST_ShortSleep()
#endif
		;

		CalcTics();
		if (!(frame&3))
			ForceUpdateStatusBar();

	} while (1);
}

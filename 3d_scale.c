// 3D_SCALE.C

#include "3d_def.h"
//#pragma hdrstop

#define OP_RETF	0xcb

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
#define CLOAKED_SHAPES			(false)
#else
#ifdef __AMIGA__
#define CLOAKED_SHAPES (1)
#else
#define CLOAKED_SHAPES			(true)
#endif
#endif

/*
=============================================================================

						  GLOBALS

=============================================================================
*/

//t_compscale _seg *scaledirectory[MAXSCALEHEIGHT+1];
//int32_t			fullscalefarcall[MAXSCALEHEIGHT+1];

int16_t			maxscale,maxscaleshl2;
uint16_t    centery;

int16_t normalshade;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
int16_t normalshade_div = SHADE_DIV;
int16_t shade_max = SHADE_MAX;
#else
int16_t normalshade_div = 1;
int16_t shade_max = 1;
#endif

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
int16_t nsd_table[] = { 1, 6, 3, 4, 1, 2};
int16_t sm_table[] =  {36,51,62,63,18,52};
#endif


/*
=============================================================================

						  LOCALS

=============================================================================
*/

//t_compscale 	_seg *work;
uint16_t BuildCompScale (int16_t height, memptr *finalspot);

int16_t			stepbytwo;

//===========================================================================

#if 0
/*
==============
=
= BadScale
=
==============
*/

void far BadScale (void)
{
	SCALE_ERROR(BADSCALE_ERROR);
}
#endif




/*
==========================
=
= SetupScaling
=
==========================
*/

void SetupScaling (int16_t maxscaleheight)
{
	int16_t		i,x,y;
	byte	far *dest;

	maxscaleheight/=2;			// one scaler every two pixels

	maxscale = maxscaleheight-1;
	maxscaleshl2 = maxscale<<2;
	normalshade=(3*(maxscale>>2))/normalshade_div;
	centery=viewheight>>1;
}

//===========================================================================

/*
========================
=
= BuildCompScale
=
= Builds a compiled scaler object that will scale a 64 tall object to
= the given height (centered vertically on the screen)
=
= height should be even
=
= Call with
= ---------
= DS:SI		Source for scale
= ES:DI		Dest for scale
=
= Calling the compiled scaler only destroys AL
=
========================
*/

#if 0
uint16_t BuildCompScale (int16_t height, memptr *finalspot)
{
	byte		far *code;

	int16_t			i;
	int32_t		fix,step;
	uint16_t	src,totalscaled,totalsize;
	int16_t			startpix,endpix,toppix;


	step = ((int32_t)height<<16) / 64;
	code = &work->code[0];
	toppix = (viewheight-height)/2;
	fix = 0;

	for (src=0;src<=64;src++)
	{
		startpix = fix>>16;
		fix += step;
		endpix = fix>>16;

		if (endpix>startpix)
			work->width[src] = endpix-startpix;
		else
			work->width[src] = 0;

//
// mark the start of the code
//
		work->codeofs[src] = FP_OFF(code);

//
// compile some code if the source pixel generates any screen pixels
//
		startpix+=toppix;
		endpix+=toppix;

		if (startpix == endpix || endpix < 0 || startpix >= viewheight || src == 64)
			continue;

	//
	// mov al,[si+src]
	//
		*code++ = 0x8a;
		*code++ = 0x44;
		*code++ = src;

		for (;startpix<endpix;startpix++)
		{
			if (startpix >= viewheight)
				break;						// off the bottom of the view area
			if (startpix < 0)
				continue;					// not into the view area

		//
		// mov [es:di+heightofs],al
		//
			*code++ = 0x26;
			*code++ = 0x88;
			*code++ = 0x85;
			*((uint16_t far *)code)++ = startpix*SCREENBWIDE;
		}

	}

//
// retf
//
	*code++ = 0xcb;

	totalsize = FP_OFF(code);
	MM_GetPtr (finalspot,totalsize);
	_fmemcpy ((byte _seg *)(*finalspot),(byte _seg *)work,totalsize);

	return totalsize;
}

#endif


// Draw Column vars

longword dc_iscale;
longword dc_frac;
uint16_t dc_source;
//uint16_t dc_seg;
uint8_t *dc_seg;
int dc_length;
int dc_dest;
/*
uint16_t dc_length;
uint16_t dc_dest;
*/

#define SFRACUNIT 0x10000

extern uint16_t far * linecmds;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
extern boolean useBounceOffset;

fixed bounceOffset=0;
#endif

/*
=======================
=
= ScaleMaskedLSPost with Light sourcing
=
=======================
*/


void ScaleMaskedLSPost (int16_t height, int buf)
{
	int16_t  length;
	uint16_t end;
	uint16_t start;
	int32_t sprtopoffset;
	int32_t topscreen;
	int32_t bottomscreen;
	longword screenstep;
	int32_t dc_yl,dc_yh;
	uint16_t far * srcpost;


/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	fixed bounce;

	if (useBounceOffset)
		bounce=bounceOffset;
	else
		bounce=0;
#endif

	srcpost=linecmds;
	dc_iscale=(64u*65536u)/(longword)height;
	screenstep = ((longword)height)<<10;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	sprtopoffset=((int32_t)viewheight<<15)-((int32_t)height<<15);
#else
	sprtopoffset=((int32_t)viewheight<<15)-((int32_t)height<<15)+(bounce>>1);
#endif
	//dc_seg=*(((uint16_t *)&srcpost)+1);

	end=(*(srcpost++))>>1;
	for (;end!=0;)
	{
		dc_source=*(srcpost++);

		start=(*(srcpost++))>>1;

		dc_source+=start;
		length=end-start;
		topscreen = sprtopoffset + (int32_t)(screenstep*(int32_t)start);
		bottomscreen = topscreen + (int32_t)(screenstep*(int32_t)length);

		dc_yl = (topscreen+SFRACUNIT-1)>>16;
		dc_yh = (bottomscreen-1)>>16;

		if (dc_yh >= viewheight)
			dc_yh = viewheight-1;

		if (dc_yl < 0)
		{
			dc_frac=dc_iscale*(-dc_yl);
			dc_yl = 0;
		}
		else
			dc_frac=0;

		if (dc_yl<=dc_yh)
		{
/*#ifdef __AMIGA__
			dc_dest=vl_get_offset(buf, 0, 0)+(uint16_t)ylookup[(uint16_t)dc_yl];
#else*/
			dc_dest=buf+(uint16_t)ylookup[(uint16_t)dc_yl];
//#endif
			dc_length=(uint16_t)(dc_yh-dc_yl+1);
#if CLOAKED_SHAPES
         if (cloaked_shape)
				R_DrawSLSColumn();
         else
#endif
				R_DrawLSColumn();
		}
		end=(*(srcpost++))>>1;
	}
}

#ifndef __AMIGA__
/*
=======================
=
= ScaleMaskedWideLSPost with Light sourcing
=
=======================
*/
void ScaleMaskedWideLSPost (int16_t height, uint16_t buf, uint16_t xx, uint16_t pwidth)
{
	byte  ofs;
	byte  msk;
	uint16_t ii;

	buf+=(uint16_t)xx>>2;
	ofs=((byte)(xx&3)<<3)+(byte)pwidth-1;
	outp(SC_INDEX+1,(byte)*((byte *)mapmasks1+ofs));
	ScaleMaskedLSPost(height,buf);
	msk=(byte)*((byte *)mapmasks2+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedLSPost(height,buf);
	msk=(byte)*((byte *)mapmasks3+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedLSPost(height,buf);
}
#endif

/*
=======================
=
= ScaleMaskedPost without Light sourcing
=
=======================
*/
void ScaleMaskedPost (int16_t height, int buf)
{
	int16_t  length;
	uint16_t end;
	uint16_t start;
	int32_t sprtopoffset;
	int32_t topscreen;
	int32_t bottomscreen;
	longword screenstep;
	int32_t dc_yl,dc_yh;
	uint16_t far * srcpost;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	fixed bounce;

	if (useBounceOffset)
		bounce=bounceOffset;
	else
		bounce=0;
#endif

	srcpost=linecmds;
	dc_iscale=(64u*65536u)/(longword)height;
	screenstep = ((longword)height)<<10;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	sprtopoffset=((int32_t)viewheight<<15)-((int32_t)height<<15);
#else
	sprtopoffset=((int32_t)viewheight<<15)-((int32_t)height<<15)+(bounce>>1);
#endif
	//dc_seg=*(((uint16_t *)&srcpost)+1);

	end=(*(srcpost++))>>1;
	for (;end!=0;)
		{
		dc_source=*(srcpost++);
		start=(*(srcpost++))>>1;

		dc_source+=start;
		length=end-start;
		topscreen = sprtopoffset + (int32_t)(screenstep*(int32_t)start);
		bottomscreen = topscreen + (int32_t)(screenstep*(int32_t)length);
		dc_yl = (topscreen+SFRACUNIT-1)>>16;
		dc_yh = (bottomscreen-1)>>16;
		if (dc_yh >= viewheight)
			dc_yh = viewheight-1;
		if (dc_yl < 0)
			{
			dc_frac=dc_iscale*(-dc_yl);
			dc_yl = 0;
			}
		else
			dc_frac=0;
		if (dc_yl<=dc_yh)
			{
/*#ifdef __AMIGA__
			dc_dest=vl_get_offset(buf, 0, 0)+(uint16_t)ylookup[(uint16_t)dc_yl];
#else*/
			dc_dest=buf+(uint16_t)ylookup[(uint16_t)dc_yl];
//#endif
			dc_length=(uint16_t)(dc_yh-dc_yl+1);
			R_DrawColumn();
			}
		end=(*(srcpost++))>>1;
		}

}


/*
=======================
=
= ScaleMaskedWidePost without Light sourcing
=
=======================
*/
#ifndef __AMIGA__
void ScaleMaskedWidePost (int16_t height, uint16_t buf, uint16_t xx, uint16_t pwidth)
{
	byte  ofs;
	byte  msk;
	uint16_t ii;

	buf+=(uint16_t)xx>>2;
	ofs=((byte)(xx&3)<<3)+(byte)pwidth-1;
	outp(SC_INDEX+1,(byte)*((byte *)mapmasks1+ofs));
	ScaleMaskedPost(height,buf);
	msk=(byte)*((byte *)mapmasks2+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedPost(height,buf);
	msk=(byte)*((byte *)mapmasks3+ofs);
	if (msk==0)
		return;
	buf++;
	outp(SC_INDEX+1,msk);
	ScaleMaskedPost(height,buf);
}
#endif



/*
=======================
=
= ScaleLSShape with Light sourcing
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/
extern byte far * shadingtable;
extern byte far * lightsource;

void ScaleLSShape (int16_t xcenter, int16_t shapenum, uint16_t height, int8_t lighting)
{
	t_compshape	_seg *shape;
	int16_t      dest;
	int16_t      i;
	longword frac;
	uint16_t width;
	int16_t      x1,x2;
	longword xscale;
	longword screenscale;
	int32_t		texturecolumn;
	int32_t     lastcolumn;
	int16_t      startx;
	uint16_t swidth;
	int32_t     xcent;
return;
	if ((height>>1>maxscaleshl2)||(!(height>>1)))
		return;
	shape = PM_GetSpritePage (shapenum);
	//*(((uint16_t *)&linecmds)+1)=(uint16_t)shape;		// seg of shape
	dc_seg = (uint8_t*)shape;
	xscale=(longword)height<<12;
	xcent=(int32_t)((int32_t)xcenter<<20)-((int32_t)height<<17)+0x80000;
//
// calculate edges of the shape
//
	x1 = (int16_t)((int32_t)(xcent+((int32_t)shape->leftpix*xscale))>>20);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (int16_t)((int32_t)(xcent+((int32_t)shape->rightpix*xscale))>>20);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(256L<<20L)/(longword)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=((int32_t)-x1)*(int32_t)screenscale;
		x1=0;
		}
	else
		frac=screenscale>>1;
	x2 = x2 >= viewwidth ? viewwidth-1 : x2;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	i=shade_max-(63l*(int32_t)(height>>3)/(int32_t)normalshade)+lighting;
#else
	i=shade_max-(63l*(uint32_t)(height>>3)/(uint32_t)normalshade)+lighting;
#endif

	if (i<0)
		i=0;
   else
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	i %= 64;
#else
  	if (i > 63)
   	i = 63;
#endif

	shadingtable=lightsource+(i<<8);
	swidth=shape->rightpix-shape->leftpix;
	if (height>256)
		{
		width=1;
		startx=0;
		lastcolumn=-1;
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				{
				if (lastcolumn>=0)
					{
					//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
					linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
					ScaleMaskedWideLSPost(height>>2,(uint16_t)bufferofs,(uint16_t)startx,width);
					width=1;
					lastcolumn=-1;
					}
				continue;
				}
			texturecolumn = (int32_t)(frac>>20);
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
					//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
					linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
					ScaleMaskedWideLSPost(height>>2,(uint16_t)bufferofs,(uint16_t)startx,width);
					width=1;
					startx=x1;
					lastcolumn=texturecolumn;
					}
				else
					{
					startx=x1;
					width=1;
					lastcolumn=texturecolumn;
					}
				}
			}
		if (lastcolumn!=-1)
			{
			//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
			linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
			ScaleMaskedWideLSPost(height>>2,bufferofs,(uint16_t)startx,width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				continue;
			outp(SC_INDEX+1,1<<(byte)(x1&3));
			texturecolumn=frac>>20;
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)texturecolumn];
			linecmds = (uint16_t *)&dc_seg[shape->dataofs[texturecolumn]];
			ScaleMaskedLSPost(height>>2,bufferofs+((uint16_t)x1>>2));
			}
		}
}




/*
=======================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/
void ScaleShape (int16_t xcenter, int16_t shapenum, uint16_t height)
{
	t_compshape	_seg *shape;
	int16_t      dest;
	int16_t      i;
	longword frac;
	uint16_t width;
	int16_t      x1,x2;
	longword xscale;
	longword screenscale;
	int32_t		texturecolumn;
	int32_t     lastcolumn;
	int16_t      startx;
	int32_t     xcent;
	uint16_t swidth;
return;

	if ((height>>1>maxscaleshl2)||(!(height>>1)))
		return;
	shape = PM_GetSpritePage (shapenum);
	//*(((uint16_t *)&linecmds)+1)=(uint16_t)shape;		// seg of shape
	dc_seg = (uint8_t *)shape;
	xscale=(longword)height<<12;
	xcent=(int32_t)((int32_t)xcenter<<20)-((int32_t)height<<(17))+0x80000;
//
// calculate edges of the shape
//
	x1 = (int16_t)((int32_t)(xcent+((int32_t)shape->leftpix*xscale))>>20);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (int16_t)((int32_t)(xcent+((int32_t)shape->rightpix*xscale))>>20);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(256L<<20L)/(longword)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=((int32_t)-x1)*(int32_t)screenscale;
		x1=0;
		}
	else
		frac=screenscale>>1;
	x2 = x2 >= viewwidth ? viewwidth-1 : x2;
	swidth=shape->rightpix-shape->leftpix;
	if (height>256)
		{
		width=1;
		startx=0;
		lastcolumn=-1;
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				{
				if (lastcolumn>=0)
					{
					//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
					linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
					ScaleMaskedWidePost(height>>2,(uint16_t)bufferofs,(uint16_t)startx,width);
					width=1;
					lastcolumn=-1;
					}
				continue;
				}
			texturecolumn = (int32_t)(frac>>20);
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
					//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
					linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
					ScaleMaskedWidePost(height>>2,(uint16_t)bufferofs,(uint16_t)startx,width);
					width=1;
					startx=x1;
					lastcolumn=texturecolumn;
					}
				else
					{
					startx=x1;
					width=1;
					lastcolumn=texturecolumn;
					}
				}
			}
		if (lastcolumn!=-1)
			{
			//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
			linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
			ScaleMaskedWidePost(height>>2,bufferofs,(uint16_t)startx,width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			if (wallheight[x1]>height)
				continue;
			outp(SC_INDEX+1,1<<(byte)(x1&3));
			texturecolumn=frac>>20;
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)texturecolumn];
			linecmds = (uint16_t *)&dc_seg[shape->dataofs[texturecolumn]];
			ScaleMaskedPost(height>>2,bufferofs+((uint16_t)x1>>2));
			}
		}
}



/*
=======================
=
= SimpleScaleShape
=
= NO CLIPPING, height in pixels
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

void SimpleScaleShape (int16_t xcenter, int16_t shapenum, uint16_t height)
{
	t_compshape	_seg *shape;
	int16_t      dest;
	int16_t      i;
	longword frac;
	int16_t      width;
	int16_t      x1,x2;
	longword xscale;
	longword screenscale;
	int32_t		texturecolumn;
	int32_t     lastcolumn;
	int16_t      startx;
	int32_t     xcent;
	uint16_t swidth;
return;
	shape = PM_GetSpritePage (shapenum);
	//*(((uint16_t *)&linecmds)+1)=(uint16_t)shape;		// seg of shape
	dc_seg = (uint8_t *)shape;
	xscale=(longword)height<<10;
	xcent=(int32_t)((int32_t)xcenter<<16)-((int32_t)height<<(15))+0x8000;
//
// calculate edges of the shape
//
	x1 = (int16_t)((int32_t)(xcent+((int32_t)shape->leftpix*xscale))>>16);
	if (x1 >= viewwidth)
		 return;               // off the right side
	x2 = (int16_t)((int32_t)(xcent+((int32_t)shape->rightpix*xscale))>>16);
	if (x2 < 0)
		 return;         // off the left side
	screenscale=(64*65536)/(longword)height;
//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=screenscale*((int32_t)-x1);
		x1=0;
		}
	else
		frac=screenscale>>1;
	x2 = x2 >= viewwidth ? viewwidth-1 : x2;
	swidth=shape->rightpix-shape->leftpix;
	if (height>64)
		{
		width=1;
		startx=0;
		lastcolumn=-1;
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			texturecolumn = (int32_t)(frac>>16);
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
					//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
					linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
					ScaleMaskedWidePost(height,bufferofs,startx,width);
					width=1;
					startx=x1;
					lastcolumn=texturecolumn;
					}
				else
					{
					startx=x1;
					lastcolumn=texturecolumn;
					}
				}
			}
		if (lastcolumn!=-1)
			{
			//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
			linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
			ScaleMaskedWidePost(height,bufferofs,startx,width);
			}
		}
	else
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			outp(SC_INDEX+1,1<<(x1&3));
			texturecolumn=frac>>16;
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)texturecolumn];
			linecmds = (uint16_t *)&dc_seg[shape->dataofs[texturecolumn]];
			ScaleMaskedPost(height,bufferofs+(x1>>2));
			}
		}
}

//-------------------------------------------------------------------------
// MegaSimpleScaleShape()
//
// NOTE: Parameter SHADE determines which Shade palette to use on the shape.
//       0 == NO Shading
//       63 == Max Shade (BLACK or near)
//-------------------------------------------------------------------------
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
void MegaSimpleScaleShape (int16_t xcenter, int16_t ycenter, int16_t shapenum, uint16_t height)
#else
void MegaSimpleScaleShape (int16_t xcenter, int16_t ycenter, int16_t shapenum, uint16_t height, uint16_t shade)
#endif
{
	t_compshape	_seg *shape;
	int16_t      dest;
	int16_t      i;
	longword frac;
	int16_t      width;
	int16_t      x1,x2;
	longword xscale;
	longword screenscale;
	int32_t		texturecolumn;
	int32_t     lastcolumn;
	int16_t      startx;
	int32_t     xcent;
	uint16_t old_bufferofs;
	int16_t 		swidth;


	old_bufferofs = bufferofs;
	ycenter -=34;
	bufferofs -= ((viewheight-64)>>1)*SCREENBWIDE;
	bufferofs += SCREENBWIDE*ycenter;

	shape = PM_GetSpritePage (shapenum);
	//*(((uint16_t *)&linecmds)+1)=(uint16_t)shape;		// seg of shape
	dc_seg = (uint8_t *)shape;
	xscale=(longword)height<<14;
	xcent=(int32_t)((int32_t)xcenter<<20)-((int32_t)height<<(19))+0x80000;
//
// calculate edges of the shape
//
	x1 = (int16_t)((int32_t)(xcent+((int32_t)shape->leftpix*xscale))>>20);

	if (x1 >= viewwidth)
		 return;               // off the right side

	x2 = (int16_t)((int32_t)(xcent+((int32_t)shape->rightpix*xscale))>>20);

	if (x2 < 0)
		 return;         // off the left side

	screenscale=(64L<<20L)/(longword)height;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
//
// Choose shade table.
//
	shadingtable=lightsource+(shade<<8);	
#endif

//
// store information in a vissprite
//
	if (x1<0)
		{
		frac=screenscale*((int32_t)-x1);
		x1=0;
		}
	else
		frac=screenscale>>1;
	x2 = x2 >= viewwidth ? viewwidth-1 : x2;
	swidth=shape->rightpix-shape->leftpix;

#ifndef __AMIGA__
	if (height>64)
		{
		width=1;
		startx=0;
		lastcolumn=-1;
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
			texturecolumn = (int32_t)(frac>>20);
			if (texturecolumn==lastcolumn)
				{
				width++;
				continue;
				}
			else
				{
				if (lastcolumn>=0)
					{
					//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
					linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
					ScaleMaskedWidePost(height,bufferofs,startx,width);
#else
					ScaleMaskedWideLSPost(height,bufferofs,startx,width);
#endif
					width=1;
					startx=x1;
					lastcolumn=texturecolumn;
					}
				else
					{
					startx=x1;
					lastcolumn=texturecolumn;
					}
				}
			}
		if (lastcolumn!=-1)
			{
			//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)lastcolumn];
			linecmds = (uint16_t *)&dc_seg[shape->dataofs[lastcolumn]];
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
			ScaleMaskedWidePost(height,bufferofs,startx,width);
#else
			ScaleMaskedWideLSPost(height,bufferofs,startx,width);
#endif
			}
		}
	else
#endif
		{
		for (; x1<=x2 ; x1++, frac += screenscale)
		  {
#ifndef __AMIGA__
			outp(SC_INDEX+1,1<<(x1&3));
#endif
			texturecolumn=frac>>20;
			if (texturecolumn>swidth)
				texturecolumn=swidth;
			//(uint16_t)linecmds=(uint16_t)shape->dataofs[(uint16_t)texturecolumn];
			linecmds = (uint16_t *)&dc_seg[shape->dataofs[texturecolumn]];

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
#ifdef __AMIGA__
			ScaleMaskedPost(height,vl_get_offset(bufferofs, x1, 0));
#else
			ScaleMaskedPost(height,bufferofs+(x1>>2));
#endif
#else
#ifdef __AMIGA__
			ScaleMaskedLSPost(height,vl_get_offset(bufferofs, x1, 0));
#else
			ScaleMaskedLSPost(height,bufferofs+(x1>>2));
#endif
#endif
			}
		}
	bufferofs = old_bufferofs;

}


//
// bit mask tables for drawing scaled strips up to eight pixels wide
//
// down here so the STUPID inline assembler doesn't get confused!
//


byte	mapmasks1[4][8] = {
{1 ,3 ,7 ,15,15,15,15,15},
{2 ,6 ,14,14,14,14,14,14},
{4 ,12,12,12,12,12,12,12},
{8 ,8 ,8 ,8 ,8 ,8 ,8 ,8} };

byte	mapmasks2[4][8] = {
{0 ,0 ,0 ,0 ,1 ,3 ,7 ,15},
{0 ,0 ,0 ,1 ,3 ,7 ,15,15},
{0 ,0 ,1 ,3 ,7 ,15,15,15},
{0 ,1 ,3 ,7 ,15,15,15,15} };

byte	mapmasks3[4][8] = {
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1},
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,3},
{0 ,0 ,0 ,0 ,0 ,1 ,3 ,7} };


#if 0

uint16_t	wordmasks[8][8] = {
{0x0080,0x00c0,0x00e0,0x00f0,0x00f8,0x00fc,0x00fe,0x00ff},
{0x0040,0x0060,0x0070,0x0078,0x007c,0x007e,0x007f,0x807f},
{0x0020,0x0030,0x0038,0x003c,0x003e,0x003f,0x803f,0xc03f},
{0x0010,0x0018,0x001c,0x001e,0x001f,0x801f,0xc01f,0xe01f},
{0x0008,0x000c,0x000e,0x000f,0x800f,0xc00f,0xe00f,0xf00f},
{0x0004,0x0006,0x0007,0x8007,0xc007,0xe007,0xf007,0xf807},
{0x0002,0x0003,0x8003,0xc003,0xe003,0xf003,0xf803,0xfc03},
{0x0001,0x8001,0xc001,0xe001,0xf001,0xf801,0xfc01,0xfe01} };

#endif

int16_t			slinex,slinewidth;
uint16_t	far *linecmds;
int32_t		linescale;
uint16_t	maskword;


// 3D_DRAW.C

#include "3d_def.h"
#include <time.h>
//#pragma hdrstop

//#define DEBUGWALLS
//#define DEBUGTICS

//#define WOLFDOORS

#define MASKABLE_DOORS		(false)
#define MASKABLE_POSTS		(false | MASKABLE_DOORS)

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

// the door is the last picture before the sprites

#define DOORWALL	(PMSpriteStart-(NUMDOORTYPES))

#define ACTORSIZE	0x4000

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
void DrawRadar(void);
#endif

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

//
// player interface stuff
//
int16_t		weaponchangetics,itemchangetics,bodychangetics;
int16_t		plaqueon,plaquetime,getpic;

star_t *firststar,*laststar;		


#ifdef DEBUGWALLS
uint16_t screenloc[3]= {PAGE1START,PAGE1START,PAGE1START};
#else
uint16_t screenloc[3]= {PAGE1START,PAGE2START,PAGE3START};
#endif
uint16_t freelatch = FREESTART;

int32_t 	lasttimecount;
int32_t 	frameon;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
int32_t framecount;
#endif

uint16_t	wallheight[MAXVIEWWIDTH];

fixed	mindist		= MINDIST;


//
// math tables
//
int16_t			pixelangle[MAXVIEWWIDTH];
int32_t		far finetangent[FINEANGLES/4];
fixed 		far sintable[ANGLES+ANGLES/4],far *costable = sintable+(ANGLES/4);

//
// refresh variables
//
fixed	viewx,viewy;			// the focal point
int16_t		viewangle;
fixed	viewsin,viewcos;

#ifndef WOLFDOORS
char far thetile[64];
byte far * mytile;
#endif


fixed	FixedByFrac (fixed a, fixed b);
void	TransformActor (objtype *ob);
void	BuildTables (void);
void	ClearScreen (void);
int16_t		CalcRotate (objtype *ob);
void	DrawScaleds (void);
void	CalcTics (void);
void	FixOfs (void);
void	ThreeDRefresh (void);



//
// wall optimization variables
//
int16_t		lastside;		// true for vertical
int32_t	lastintercept;
int16_t		lasttilehit;


//
// ray tracing variables
//
int16_t			focaltx,focalty,viewtx,viewty;

int16_t			midangle,angle;
uint16_t	xpartial,ypartial;
uint16_t	xpartialup,xpartialdown,ypartialup,ypartialdown;
uint16_t	xinttile,yinttile;

uint16_t	tilehit;
uint16_t	pixx;

int16_t		xtile,ytile;
int16_t		xtilestep,ytilestep;
int32_t	xintercept,yintercept;
int32_t	xstep,ystep;

int16_t		horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];



uint16_t viewflags;
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
extern byte lightson;
#endif

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
// Global Cloaked Shape flag..

boolean cloaked_shape = false;
#endif



/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


void AsmRefresh (void);					// in 3D_DR_A.ASM
void NoWallAsmRefresh (void);			// in 3D_DR_A.ASM

// BBi
static int last_texture_offset = -1;
static uint8_t* last_texture_data = NULL;

/*
============================================================================

			   3 - D  DEFINITIONS

============================================================================
*/


//==========================================================================


/*
========================
=
= FixedByFrac
=
= multiply a 16/16 bit, 2's complement fixed point number by a 16 bit
= fraction, passed as a signed magnitude 32 bit number
=
========================
*/

//#pragma warn -rvl			// I stick the return value in with ASMs

fixed FixedByFrac (fixed a, fixed b)
{
#ifdef __AMIGA__
#define id0_longword_t longword
#define id0_word_t word
	int result_sign = (b < 0) ? -1 : 1; // sign of result == sign of fraction
	id0_longword_t a_as_unsigned = a;
	if (a < 0) // negative?
	{
		//2's complement...
		a_as_unsigned = -a;
		//a_as_unsigned ^= -1;
		//++a_as_unsigned;
		result_sign *= -1; // toggle sign of result
	}
	//
	// Multiply a_as_unsigned by the low 8 bits of b
	//
	id0_word_t b_lo = b&0xFFFF;
	id0_longword_t result = b_lo*(a_as_unsigned>>16) + ((b_lo*(a_as_unsigned&0xFFFF))>>16);
	//id0_longword_t result = b_lo*(a_as_unsigned>>16) + b_lo*(a_as_unsigned&0xFFFF);
	//
	// put result in 2's complement
	//
	if (result_sign < 0) // Is the result negative?
	{
		//2's complement...
		result = -result;
		//result ^= -1;
		//++result;
	}
	return result;
/*
    int b_sign;
    uint32_t ub;
    int32_t fracs;
    int32_t ints;
    int32_t result;

    b_sign = (b < 0) ? -1 : 1;

    if (b_sign < 0) {
        a = -a;
        b_sign = -b_sign;
    }

    ub = (uint32_t)b & 0xFFFF;
    fracs = (((uint32_t)a & 0xFFFF) * ub) >> 16;
    ints = (a >> 16) * ub;
    result = ints + fracs;
    result *= b_sign;

    return result;

	int32_t b_sign;
	uint32_t ub;
	int32_t fracs;
	int32_t ints;
	int32_t result;

	b_sign = (b < 0) ? -1 : 1;

	if (b_sign < 0)
	{
		a = -a;
		b_sign = -b_sign;
	}

	ub = (uint32_t)b & 0xFFFF;
	fracs = (((uint32_t)a & 0xFFFF) * ub) >> 16;
	ints = (a >> 16) * ub;
	result = ints + fracs;
	result *= b_sign;

	return result;*/
#else
//
// setup
//
asm	mov	si,[WORD PTR b+2]	// sign of result = sign of fraction

asm	mov	ax,[WORD PTR a]
asm	mov	cx,[WORD PTR a+2]

asm	or		cx,cx
asm	jns	aok:				// negative?
asm	neg	cx
asm	neg	ax
asm	sbb	cx,0
asm	xor	si,0x8000			// toggle sign of result
aok:

//
// multiply  cx:ax by bx
//
asm	mov	bx,[WORD PTR b]
asm	mul	bx					// fraction*fraction
asm	mov	di,dx				// di is low word of result
asm	mov	ax,cx				//
asm	mul	bx					// units*fraction
asm 	add	ax,di
asm	adc	dx,0

//
// put result dx:ax in 2's complement
//
asm	test	si,0x8000		// is the result negative?
asm	jz		ansok:
asm	neg	dx
asm	neg	ax
asm	sbb	dx,0

ansok:;
#endif

}

//#pragma warn +rvl

//==========================================================================

/*
========================
=
= TransformActor
=
= Takes paramaters:
=   gx,gy				: globalx/globaly of point
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=   scale				: conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
========================
*/


//
// transform actor
//
void TransformActor (objtype *ob)
{
	int16_t ratio;
	fixed gx,gy,gxt,gyt,nx,ny;
	int32_t	temp;

//
// translate point to view centered coordinates
//
	gx = ob->x-viewx;
	gy = ob->y-viewy;

//
// calculate newx
//
	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-ACTORSIZE;		// fudge the shape forward a bit, because
										// the midpoint could put parts of the shape
										// into an adjacent wall

//
// calculate newy
//
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;

//
// calculate perspective ratio
//
	ob->transx = nx;
	ob->transy = ny;

	if (nx<mindist)			// too close, don't overflow the divide
	{
	  ob->viewheight = 0;
	  return;
	}

	ob->viewx = centerx + ny*scale/nx;	// DEBUG: use assembly divide

//
// calculate height (heightnumerator/(nx>>8))
//
#ifdef __AMIGA__
	/*
	int32_t q = (heightnumerator / (nx >> 8)) & 0xFFFF;
	int32_t r = (heightnumerator % (nx >> 8)) & 0xFFFF;
	temp = (r << 16) | q;
	*/
	temp = heightnumerator/(nx>>8);
#else
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8
	asm	mov	[WORD PTR temp],ax
	asm	mov	[WORD PTR temp+2],dx
#endif

	ob->viewheight = temp;
}

//==========================================================================

/*
========================
=
= TransformTile
=
= Takes paramaters:
=   tx,ty		: tile the object is centered in
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=   scale		: conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
= Returns true if the tile is withing getting distance
=
========================
*/
boolean TransformTile (int16_t tx, int16_t ty, int16_t *dispx, int16_t *dispheight)
{
	int16_t ratio;
	fixed gx,gy,gxt,gyt,nx,ny;
	int32_t	temp;

//
// translate point to view centered coordinates
//
	gx = ((int32_t)tx<<TILESHIFT)+0x8000-viewx;
	gy = ((int32_t)ty<<TILESHIFT)+0x8000-viewy;

//
// calculate newx
//
	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-0x2000;		// 0x2000 is size of object

//
// calculate newy
//
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;


//
// calculate perspective ratio
//
	if (nx<mindist)			// too close, don't overflow the divide
	{
		*dispheight = 0;
		return false;
	}

	*dispx = centerx + ny*scale/nx;	// DEBUG: use assembly divide


//
// calculate height (heightnumerator/(nx>>8))
//
#ifdef __AMIGA__
	/*
	int32_t q = (heightnumerator / (nx >> 8)) & 0xFFFF;
	int32_t r = (heightnumerator % (nx >> 8)) & 0xFFFF;
	temp = (r << 16) | q;
	*/
	temp = heightnumerator/(nx>>8);
#else
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8
	asm	mov	[WORD PTR temp],ax
	asm	mov	[WORD PTR temp+2],dx
#endif

	*dispheight = temp;

//
// see if it should be grabbed
//
	if (nx<TILEGLOBAL && ny>-TILEGLOBAL/2 && ny<TILEGLOBAL/2)
		return true;
	else
		return false;
}




//==========================================================================

/*
====================
=
= CalcHeight
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
====================
*/

//#pragma warn -rvl			// I stick the return value in with ASMs

int16_t	CalcHeight (void)
{
	int16_t	transheight;
	int16_t 	ratio;
	fixed gxt,gyt,nx,ny;
	int32_t	gx,gy;

	gx = xintercept-viewx;
	gxt = FixedByFrac(gx,viewcos);

	gy = yintercept-viewy;
	gyt = FixedByFrac(gy,viewsin);

	nx = gxt-gyt;

  	//
  	// calculate perspective ratio (heightnumerator/(nx>>8))
	//

	if (nx<mindist)
		nx=mindist;			// don't let divide overflow

#ifdef __AMIGA__
	//int16_t result = heightnumerator / (nx / 256);
	int16_t result = heightnumerator/(nx>>8);

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	if (result < 8)
		result = 8;
#endif

	return result;
#else
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
   asm	cmp	ax,8
   asm	jge  	exit_func
	asm	mov	ax,8
#endif

	return result;
exit_func:
#endif

}


//==========================================================================



/*
===================
=
= ScalePost
=
===================
*/

//int32_t           postsource;
uint8_t* postsource;
uint16_t       postx;
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
uint16_t       bufx;
#endif
uint16_t       postwidth;
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
uint16_t       postheight;
byte far *     shadingtable;
extern byte far * lightsource;
#endif

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
// Basically a copy-and-paste from Wolfenstein 3D
#ifdef GAMEVER_RESTORATION_AOG_100
void	near ScalePost (void)		// VGA version
{
	asm	mov	ax,SCREENSEG
	asm	mov	es,ax

	asm	mov	bx,[postx]
	asm	shl	bx,1
	asm	mov	bp,WORD PTR [wallheight+bx]		// fractional height (low 3 bits frac)
	asm	and	bp,0xfff8				// bp = heightscaler*4
	asm	shr	bp,1
	asm	cmp	bp,[maxscaleshl2]
	asm	jle	heightok
	asm	mov	bp,[maxscaleshl2]
heightok:
	asm	add	bp,OFFSET fullscalefarcall
	//
	// scale a byte wide strip of wall
	//
	asm	mov	bx,[postx]
	asm	mov	di,bx
	asm	shr	di,2						// X in bytes
	asm	add	di,[bufferofs]

	asm	and	bx,3
	asm	shl	bx,3						// bx = pixel*8+pixwidth
	asm	add	bx,[postwidth]

	asm	mov	al,BYTE PTR [mapmasks1-1+bx]	// -1 because no widths of 0
	asm	mov	dx,SC_INDEX+1
	asm	out	dx,al						// set bit mask register
	asm	lds	si,DWORD PTR [postsource]
	asm	call DWORD PTR [bp]				// scale the line of pixels

	asm	mov	al,BYTE PTR [ss:mapmasks2-1+bx]   // -1 because no widths of 0
	asm	or	al,al
	asm	jz	nomore

	//
	// draw a second byte for vertical strips that cross two bytes
	//
	asm	inc	di
	asm	out	dx,al						// set bit mask register
	asm	call DWORD PTR [bp]				// scale the line of pixels

	asm	mov	al,BYTE PTR [ss:mapmasks3-1+bx]	// -1 because no widths of 0
	asm	or	al,al
	asm	jz	nomore
	//
	// draw a third byte for vertical strips that cross three bytes
	//
	asm	inc	di
	asm	out	dx,al						// set bit mask register
	asm	call DWORD PTR [bp]				// scale the line of pixels


nomore:
	asm	mov	ax,ss
	asm	mov	ds,ax
}
#else
void   ScalePost (void)      // VGA version
{
	int16_t height;
	int32_t i;
	byte ofs;
	byte msk;

	height=(wallheight[postx])>>3;
	postheight=height;
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
      	if (i > 63)
         	i = 63;					// Debugging.. put break point here!
#endif

		shadingtable=lightsource+(i<<8);
#ifndef __AMIGA__
		bufx=postx>>2;
		ofs=((postx&3)<<3)+postwidth-1;
		outp(SC_INDEX+1,(byte)*((byte *)mapmasks1+ofs));
		DrawLSPost();
		msk=(byte)*((byte *)mapmasks2+ofs);
		if (msk==0)
			return;
		bufx++;
		outp(SC_INDEX+1,msk);
		DrawLSPost();
		msk=(byte)*((byte *)mapmasks3+ofs);
		if (msk==0)
			return;
		bufx++;
		outp(SC_INDEX+1,msk);
#endif
		DrawLSPost();
		}
	else
		{
#ifndef __AMIGA__
		bufx=postx>>2;
		ofs=((postx&3)<<3)+postwidth-1;
		outp(SC_INDEX+1,(byte)*((byte *)mapmasks1+ofs));
		DrawPost();
		msk=(byte)*((byte *)mapmasks2+ofs);
		if (msk==0)
			return;
		bufx++;
		outp(SC_INDEX+1,msk);
		DrawPost();
		msk=(byte)*((byte *)mapmasks3+ofs);
		if (msk==0)
			return;
		bufx++;
		outp(SC_INDEX+1,msk);
#endif
		DrawPost();
		}
}
#endif // GAMEVER_RESTORATION_AOG_100

void  FarScalePost ()				// just so other files can call
{
	ScalePost ();
}


/*
====================
=
= HitVertWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

uint16_t far DoorJamsShade[] =
{
	BIO_JAM_SHADE,					// dr_bio
	SPACE_JAM_2_SHADE,			// dr_normal
	STEEL_JAM_SHADE,				// dr_prison
	SPACE_JAM_2_SHADE,			// dr_elevator
	STEEL_JAM_SHADE,				// dr_high_sec
	OFFICE_JAM_SHADE,				// dr_office
	STEEL_JAM_SHADE,				// dr_oneway_left
	STEEL_JAM_SHADE,				// dr_oneway_up
	STEEL_JAM_SHADE,				// dr_oneway_right
	STEEL_JAM_SHADE,				// dr_oneway_down
	SPACE_JAM_SHADE,				// dr_space
};

uint16_t far DoorJams[] =
{
	BIO_JAM,					// dr_bio
	SPACE_JAM_2,			// dr_normal
	STEEL_JAM,				// dr_prison
	SPACE_JAM_2,			// dr_elevator
	STEEL_JAM,				// dr_high_sec
	OFFICE_JAM,				// dr_office
	STEEL_JAM,				// dr_oneway_left
	STEEL_JAM,				// dr_oneway_up
	STEEL_JAM,				// dr_oneway_right
	STEEL_JAM,				// dr_oneway_down
	SPACE_JAM,				// dr_space
};



void HitVertWall (void)
{
	int16_t			wallpic;
	uint16_t	texture;
	uint8_t doornum;

	texture = (yintercept>>4)&0xfc0;
	if (xtilestep == -1)
	{
		texture = 0xfc0-texture;
		xintercept += TILEGLOBAL;
	}

	wallheight[pixx] = CalcHeight();

	if (lastside==1 && lastintercept == xtile && lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		if (texture == /*(uint16_t)postsource*/last_texture_offset)
#else
		if (texture == /*(uint16_t)postsource*/last_texture_offset && postwidth < 8)
#endif
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			//postsource = texture;
			last_texture_offset = texture;
			postsource = &last_texture_data[last_texture_offset];
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall

		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lastside = true;
		lastintercept = xtile;

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		if (tilehit & 0x40)
		{
			// check for adjacent doors
         //

			ytile = yintercept>>TILESHIFT;

			if ((doornum = tilemap[xtile-xtilestep][ytile])&0x80 )
				wallpic = DOORWALL+DoorJamsShade[doorobjlist[doornum & 0x7f].type];
			else
				wallpic = vertwall[tilehit & ~0x40];
		}
		else
			wallpic = vertwall[tilehit];

		//*(((uint16_t *)&postsource)+1) = (uint16_t)PM_GetPage(wallpic);
		//postsource = texture;
		last_texture_data = (uint8_t *)PM_GetPage(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}
}


/*
====================
=
= HitHorizWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/
void HitHorizWall (void)
{
	int16_t			wallpic;
	uint16_t	texture;
	uint8_t doornum;

	texture = (xintercept>>4)&0xfc0;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL;
	else
		texture = 0xfc0-texture;
	wallheight[pixx] = CalcHeight();

	if (lastside==0 && lastintercept == ytile && lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		if (texture == /*(uint16_t)postsource*/last_texture_offset)
#else
		if (texture == /*(uint16_t)postsource*/last_texture_offset && postwidth < 8)
#endif
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			//(uint16_t)postsource = texture;
			last_texture_offset = texture;
			postsource = &last_texture_data[last_texture_offset];
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lastside = 0;
		lastintercept = ytile;

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;



		if (tilehit & 0x40)
		{								// check for adjacent doors

			xtile = xintercept>>TILESHIFT;
			if ((doornum = tilemap[xtile][ytile-ytilestep]) & 0x80)
			{
				wallpic = DOORWALL+DoorJams[doorobjlist[doornum & 0x7f].type];
			}
			else
				wallpic = horizwall[tilehit & ~0x40];
		}
		else
			wallpic = horizwall[tilehit];


		//*( ((uint16_t *)&postsource)+1) = (uint16_t)PM_GetPage(wallpic);
		//(uint16_t)postsource = texture;
		last_texture_data = (uint8_t *)PM_GetPage(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

}


//==========================================================================

/*
====================
=
= HitHorizDoor
=
====================
*/

void HitHorizDoor (void)
{
	uint16_t	texture,doorpage = -1,doornum,xint;
	boolean lockable = true;

	doornum = tilehit&0x7f;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	if (doorobjlist[doornum].action == dr_jammed)
		return;
#endif

#ifdef WOLFDOORS
	texture = ((xintercept-doorposition[doornum]) >> 4) &0xfc0;
#else
   xint=xintercept&0xffff;

	if (xint>0x7fff)
		texture = ( (xint-(uint16_t)(doorposition[doornum]>>1)) >> 4) &0xfc0;
   else
		texture = ( (xint+(uint16_t)(doorposition[doornum]>>1)) >> 4) &0xfc0;
#endif

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		// in the same door as last time, so check for optimized draw

		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		if (texture == /*(uint16_t)postsource*/last_texture_offset)
#else
		if (texture == /*(uint16_t)postsource*/last_texture_offset && postwidth < 8)
#endif
		{
			// wide scale

			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
#if MASKABLE_DOORS
			ScaleMPost();
#else
			ScalePost ();
#endif
			//(uint16_t)postsource = texture;
			last_texture_offset = texture;
			postsource = &last_texture_data[last_texture_offset];
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
		if (lastside != -1)				// if not the first scaled post
#if MASKABLE_DOORS
			ScaleMPost();
#else
			ScalePost ();
#endif

		// first pixel in this door

		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		switch (doorobjlist[doornum].type)
		{
			case dr_normal:
				doorpage = DOORWALL+L_METAL;
				break;

			case dr_elevator:
				doorpage = DOORWALL+L_ELEVATOR;
				break;

			case dr_prison:
				doorpage = DOORWALL+L_PRISON;
				break;

			case dr_space:
				doorpage = DOORWALL+L_SPACE;
				break;

			case dr_bio:
				doorpage = DOORWALL+L_BIO;
				break;

			case dr_high_security:
					doorpage = DOORWALL+L_HIGH_SECURITY;		       	// Reverse View
			break;

			case dr_oneway_up:
			case dr_oneway_left:
				if (player->tiley > doorobjlist[doornum].tiley)
					doorpage = DOORWALL+L_ENTER_ONLY;				// normal view
				else
				{
					doorpage = DOORWALL+NOEXIT;		 	      	// Reverse View
					lockable = false;
				}
				break;

			case dr_oneway_right:
			case dr_oneway_down:
				if (player->tiley > doorobjlist[doornum].tiley)
				{
					doorpage = DOORWALL+NOEXIT;						// normal view
					lockable = false;
				}
				else
					doorpage = DOORWALL+L_ENTER_ONLY;			// Reverse View
				break;

			case dr_office:
				doorpage = DOORWALL+L_HIGH_TECH;
				break;
		}


		//
		// If door is unlocked, Inc shape ptr to unlocked door shapes
		//

		if (lockable && doorobjlist[doornum].lock == kt_none)
			doorpage += UL_METAL;


		//*( ((uint16_t *)&postsource)+1) = (uint16_t)PM_GetPage(doorpage);
		//(uint16_t)postsource = texture;
		last_texture_data = (uint8_t *)PM_GetPage(doorpage);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}
}

//==========================================================================



/*
====================
=
= HitVertDoor
=
====================
*/

void HitVertDoor (void)
{
	uint16_t	texture,doorpage,doornum,yint;
	boolean lockable = true;

	doornum = tilehit&0x7f;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	if (doorobjlist[doornum].action == dr_jammed)
		return;
#endif

#ifdef WOLFDOORS
	texture = ( (yintercept-doorposition[doornum]) >> 4) &0xfc0;
#else
		  yint=yintercept&0xffff;
		  if (yint>0x7fff)
			  texture = ( (yint-(uint16_t)(doorposition[doornum]>>1)) >> 4) &0xfc0;
		  else
			  texture = ( (yint+(uint16_t)(doorposition[doornum]>>1)) >> 4) &0xfc0;
#endif

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
	// in the same door as last time, so check for optimized draw
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		if (texture == /*(uint16_t)postsource*/last_texture_offset)
#else
		if (texture == /*(uint16_t)postsource*/last_texture_offset && postwidth < 8)
#endif
		{
			// wide scale

			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
#if MASKABLE_DOORS
			ScaleMPost();
#else
			ScalePost ();
#endif
			//(uint16_t)postsource = texture;
			last_texture_offset = texture;
			postsource = &last_texture_data[last_texture_offset];
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
		if (lastside != -1)				// if not the first scaled post
#if MASKABLE_DOORS
			ScaleMPost();
#else
			ScalePost ();
#endif

		// first pixel in this door

		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		switch (doorobjlist[doornum].type)
		{
			case dr_normal:
				doorpage = DOORWALL+L_METAL_SHADE;
				break;

			case dr_elevator:
				doorpage = DOORWALL+L_ELEVATOR_SHADE;
				break;

			case dr_prison:
				doorpage = DOORWALL+L_PRISON_SHADE;
				break;

			case dr_space:
				doorpage = DOORWALL+L_SPACE_SHADE;
				break;

			case dr_bio:
         	doorpage = DOORWALL+L_BIO;
				break;

			case dr_high_security:
					doorpage = DOORWALL+L_HIGH_SECURITY_SHADE;
			break;

			case dr_oneway_left:
			case dr_oneway_up:
				if (player->tilex > doorobjlist[doornum].tilex)
					doorpage = DOORWALL+L_ENTER_ONLY_SHADE;			// Reverse View
				else
				{
					doorpage = DOORWALL+NOEXIT_SHADE;       			// Normal view
					lockable = false;
				}
				break;

			case dr_oneway_right:
			case dr_oneway_down:
				if (player->tilex > doorobjlist[doornum].tilex)
				{
					doorpage = DOORWALL+NOEXIT_SHADE;       		// Reverse View
					lockable = false;
				}
				else
					doorpage = DOORWALL+L_ENTER_ONLY_SHADE;		// Normal View
				break;


			case dr_office:
				doorpage = DOORWALL+L_HIGH_TECH_SHADE;
				break;

		}

		//
		// If door is unlocked, Inc shape ptr to unlocked door shapes
		//

		if (lockable && doorobjlist[doornum].lock == kt_none)
			doorpage += UL_METAL;

		//*(((uint16_t *)&postsource)+1) = (uint16_t)PM_GetPage(doorpage);
		//(uint16_t)postsource = texture;
		last_texture_data = (uint8_t *)PM_GetPage(doorpage);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}
}

//==========================================================================

/*
====================
=
= HitHorizPWall
=
= A pushable wall in action has been hit
=
====================
*/

void HitHorizPWall (void)
{
	int16_t			wallpic;
	uint16_t	texture,offset;

	texture = (xintercept>>4)&0xfc0;
	offset = pwallpos<<10;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL-offset;
	else
	{
		texture = 0xfc0-texture;
		yintercept += offset;
	}

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		if (texture == /*(uint16_t)postsource*/last_texture_offset)
#else
		if (texture == /*(uint16_t)postsource*/last_texture_offset && postwidth < 8)
#endif
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			//(uint16_t)postsource = texture;
			last_texture_offset = texture;
			postsource = &last_texture_data[last_texture_offset];
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		wallpic = horizwall[tilehit&63];

		//*( ((uint16_t *)&postsource)+1) = (uint16_t)PM_GetPage(wallpic);
		//(uint16_t)postsource = texture;
		last_texture_data = (uint8_t *)PM_GetPage(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

}


/*
====================
=
= HitVertPWall
=
= A pushable wall in action has been hit
=
====================
*/

void HitVertPWall (void)
{
	int16_t			wallpic;
	uint16_t	texture,offset;

	texture = (yintercept>>4)&0xfc0;
	offset = pwallpos<<10;
	if (xtilestep == -1)
	{
		xintercept += TILEGLOBAL-offset;
		texture = 0xfc0-texture;
	}
	else
		xintercept += offset;

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		if (texture == /*(uint16_t)postsource*/last_texture_offset)
#else
		if (texture == /*(uint16_t)postsource*/last_texture_offset && postwidth < 8)
#endif
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			//(uint16_t)postsource = texture;
			last_texture_offset = texture;
			postsource = &last_texture_data[last_texture_offset];
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		wallpic = vertwall[tilehit&63];

		//*( ((uint16_t *)&postsource)+1) = (uint16_t)PM_GetPage(wallpic);
		//(uint16_t)postsource = texture;
		last_texture_data = (uint8_t *)PM_GetPage(wallpic);
		last_texture_offset = texture;
		postsource = &last_texture_data[last_texture_offset];
	}

}

//==========================================================================

//==========================================================================

#if 0
/*
=====================
=
= ClearScreen
=
=====================
*/

void ClearScreen (void)
{
 uint16_t floor=egaFloor[gamestate.episode*MAPS_PER_EPISODE+mapon],
	  ceiling=egaCeiling[gamestate.episode*MAPS_PER_EPISODE+mapon];

  //
  // clear the screen
  //
asm	mov	dx,GC_INDEX
asm	mov	ax,GC_MODE + 256*2		// read mode 0, write mode 2
asm	out	dx,ax
asm	mov	ax,GC_BITMASK + 255*256
asm	out	dx,ax

asm	mov	dx,40
asm	mov	ax,[viewwidth]
asm	shr	ax,3
asm	sub	dx,ax					// dx = 40-viewwidth/8

asm	mov	bx,[viewwidth]
asm	shr	bx,4					// bl = viewwidth/16
asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1					// half height

asm	mov	ax,[ceiling]
asm	mov	es,[screenseg]
asm	mov	di,[bufferofs]

toploop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	toploop

asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1					// half height
asm	mov	ax,[floor]

bottomloop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	bottomloop


asm	mov	dx,GC_INDEX
asm	mov	ax,GC_MODE + 256*10		// read mode 1, write mode 2
asm	out	dx,ax
asm	mov	al,GC_BITMASK
asm	out	dx,al

}


#endif
//==========================================================================


#ifdef CEILING_FLOOR_COLORS
/*
=====================
=
= VGAClearScreen
=
= NOTE: Before calling this function - Check to see if there even needs
= ====  to be a solid floor or solid ceiling color drawn.
=
=====================
*/

// BBi
/*static void vga_clear_screen(int y_offset, int height, int color)
{
	uint8_t *dest = &vga_memory[vl_get_offset(bufferofs, 0, y_offset)];

	if (viewwidth == vga_width)
	{
		memset(dest, (uint8_t)color, height * vga_width);
	}
	else
	{
		int dst_pitch = vga_width-viewwidth;
		for (int y = 0; y < height; ++y)
		{
			memset(dest, color, viewwidth);
			dest += dst_pitch;
		}
	}
}*/
// BBi

void VGAClearScreen (void)
{
	viewflags = gamestate.flags;

#ifdef __AMIGA__
	int half_height = viewheight / 2;

	if ((viewflags & GS_DRAW_CEILING) == 0)
		//vga_clear_screen(0, half_height, TopColor);
		VL_Bar(0, 0, viewwidth, half_height, TopColor);

	if ((viewflags & GS_DRAW_FLOOR) == 0)
		//vga_clear_screen(viewheight - half_height, half_height, BottomColor);
		VL_Bar(0, viewheight - half_height, viewwidth, half_height, BottomColor);
#else
//
// clear the screen
//

asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256	// write through all planes
asm	out	dx,ax

asm	mov	dx,80
asm	mov	ax,[viewwidth]
asm	shr	ax,2
asm	sub	dx,ax					// dx = 40-viewwidth/2

asm	mov	bx,[viewwidth]
asm	shr	bx,3					// bl = viewwidth/8
asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1

asm	mov	es,[screenseg]
asm	mov	di,[bufferofs]

asm 	mov	ax,[viewflags]
asm	test	ax,GS_DRAW_CEILING
asm   jnz   skiptop

asm	mov	ax,[TopColor]

//
// Draw Top
//

toploop:

asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	toploop

//
//   Skip 'SkipTop' mods...
//


asm	jmp	bottominit

//
//  SkipTop mods - Compute the correct offset for the  floor
//

skiptop:
asm	mov	al,bh
asm	mov   cl,80
asm	mul	cl
asm	add	di,ax

//
// Test to see if bottom needs drawing
//

bottominit:
asm 	mov	ax,[viewflags]
asm	test	ax,GS_DRAW_FLOOR
asm   jnz   exit_mofo

asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1
asm	mov	ax,[BottomColor]

//
// Draw Bottom
//

bottomloop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	bottomloop

exit_mofo:
#endif

}
#endif

//==========================================================================

/*
=====================
=
= CalcRotate
=
=====================
*/

int16_t	CalcRotate (objtype *ob)
{
	int16_t	angle,viewangle;
	dirtype dir=ob->dir;

	// this isn't exactly correct, as it should vary by a trig value,
	// but it is close enough with only eight rotations

	viewangle = player->angle + (centerx - ob->viewx)/8;

	if (dir == nodir)
		dir = ob->trydir&127;
	angle =  (viewangle-180)- dirangle[dir];

	angle+=ANGLES/16;
	while (angle>=ANGLES)
		angle-=ANGLES;
	while (angle<0)
		angle+=ANGLES;

	if ((ob->state->flags & SF_PAINFRAME)) // 2 rotation pain frame
		return 4*(angle/(ANGLES/2));        // seperated by 3 (art layout...)

	return angle/(ANGLES/8);
}


/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visable
=
=====================
*/

#define MAXVISABLE	50


#if 0
typedef struct
{
	int16_t	viewx,
			viewheight,
			shapenum;
} visobj_t;
#endif


visobj_t	vislist[MAXVISABLE],*visptr,*visstep,*farthest;


void DrawScaleds (void)
{
	int16_t 		i,j,least,numvisable,height;
	memptr	shape;
	byte		*tilespot,*visspot;
	int16_t		shapenum;
	uint16_t	spotloc;

	statobj_t	*statptr;
	objtype		*obj;

	visptr = &vislist[0];

//
// place static objects
//
	for (statptr = &statobjlist[0] ; statptr != laststatobj ; statptr++)
	{
		if ((visptr->shapenum = statptr->shapenum) == -1)
			continue;						// object has been deleted

		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
		if ((Keyboard[sc_6] && (Keyboard[sc_7] || Keyboard[sc_8]) && DebugOk) && (statptr->flags & FL_BONUS))
		{
			GetBonus(statptr);
			continue;
		}
#endif

		if (!*statptr->visspot)
			continue;						// not visable


		if (TransformTile(statptr->tilex,statptr->tiley,&visptr->viewx,&visptr->viewheight) &&
			 (statptr->flags & FL_BONUS))
		{
			GetBonus (statptr);
			continue;
		}

		if (!visptr->viewheight)
			continue;						// to close to the object

		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
		visptr->cloaked = false;
#endif
#ifndef GAMEVER_RESTORATION_AOG_100
		visptr->lighting = statptr->lighting;			// Could add additional
        															// flashing/lighting
#endif
		if (visptr < &vislist[MAXVISABLE-1])	// don't let it overflow
			visptr++;
	}

//
// place active objects
//
	for (obj = player->next;obj;obj=obj->next)
	{


		if (obj->flags & FL_OFFSET_STATES)
		{
			if (!(visptr->shapenum = obj->temp1+obj->state->shapenum))
				continue;					// no shape
		}
		else
		if (!(visptr->shapenum = obj->state->shapenum))
			continue;						// no shape

		spotloc = (obj->tilex<<6)+obj->tiley;	// optimize: keep in struct?
#ifdef __AMIGA__
		// BBi Do not draw detonator if it's not visible.
		if (spotloc == 0)
			continue;
#endif
		visspot = &spotvis[0][0]+spotloc;
		tilespot = &tilemap[0][0]+spotloc;

		//
		// could be in any of the nine surrounding tiles
		//

		if (*visspot
		|| ( *(visspot-1) && !*(tilespot-1) )
		|| ( *(visspot+1) && !*(tilespot+1) )
		|| ( *(visspot-65) && !*(tilespot-65) )
		|| ( *(visspot-64) && !*(tilespot-64) )
		|| ( *(visspot-63) && !*(tilespot-63) )
		|| ( *(visspot+65) && !*(tilespot+65) )
		|| ( *(visspot+64) && !*(tilespot+64) )
		|| ( *(visspot+63) && !*(tilespot+63) ))
		{
			obj->active = true;

			TransformActor (obj);

			if (!obj->viewheight)
				continue;						// too close or far away

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
#ifndef GAMEVER_RESTORATION_AOG_100
			visptr->lighting = obj->lighting;
#endif
#else
         if ((obj->flags2 & (FL2_CLOAKED|FL2_DAMAGE_CLOAK)) == (FL2_CLOAKED))
         {
				visptr->cloaked = 1;
				visptr->lighting = 0;
         }
         else
         {
         	visptr->cloaked = 0;
				visptr->lighting = obj->lighting;
         }

			if (!(obj->flags & FL_DEADGUY))
				obj->flags2 &= ~FL2_DAMAGE_CLOAK;
#endif

			visptr->viewx = obj->viewx;
			visptr->viewheight = obj->viewheight;

			if (visptr->shapenum == -1)
				visptr->shapenum = obj->temp1;	// special shape

			if (obj->state->flags & SF_ROTATE)
				visptr->shapenum += CalcRotate (obj);

			if (visptr < &vislist[MAXVISABLE-1])	// don't let it overflow
				visptr++;
			obj->flags |= FL_VISABLE;
		}
		else
			obj->flags &= ~FL_VISABLE;
	}

//
// draw from back to front
//
	numvisable = visptr-&vislist[0];

	if (!numvisable)
		return;									// no visable objects

	for (i = 0; i<numvisable; i++)
	{
		least = 32000;
		for (visstep=&vislist[0] ; visstep<visptr ; visstep++)
		{
			height = visstep->viewheight;
			if (height < least)
			{
				least = height;
				farthest = visstep;
			}
		}

      //
      // Init our global flag...
      //

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
      cloaked_shape = farthest->cloaked;
#endif

		//
		// draw farthest
		//
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
#ifdef GAMEVER_RESTORATION_AOG
		if (gamestate.flags & GS_LIGHTING && (farthest->lighting != NO_SHADING))
#else
		if ((gamestate.flags & GS_LIGHTING && (farthest->lighting != NO_SHADING)) || cloaked_shape)
#endif
			ScaleLSShape(farthest->viewx,farthest->shapenum,farthest->viewheight,farthest->lighting);
		else
#endif
			ScaleShape(farthest->viewx,farthest->shapenum,farthest->viewheight);

		farthest->viewheight = 32000;
	}

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	cloaked_shape = false;
#endif

}


//==========================================================================

/*
==============
=
= DrawPlayerWeapon
=
= Draw the player's hands
=
==============
*/

int16_t	weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY,SPR_PISTOLREADY
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
		,SPR_MACHINEGUNREADY,SPR_CHAINREADY,SPR_GRENADEREADY,0};
#else
		,SPR_MACHINEGUNREADY,SPR_CHAINREADY,SPR_GRENADEREADY,SPR_BFG_WEAPON1,0};
#endif

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
boolean useBounceOffset=false;
#endif

void DrawPlayerWeapon (void)
{
	int16_t	shapenum;

	if (playstate==ex_victorious)
		return;

	if (gamestate.weapon != -1)
	{
		shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
		if (shapenum)
		{
			static int16_t vh=63;
			static int16_t ce=100;

			int8_t v_table[15]={87,81,77,63,61,60,56,53,50,47,43,41,39,35,31};
			int8_t c_table[15]={88,85,81,80,75,70,64,59,55,50,44,39,34,28,24};

			int16_t oldviewheight=viewheight;
			int16_t centery;

			useBounceOffset=true;
#endif
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
//#if 1
#if 0
			if (Keyboard[sc_PgUp])
			{
				vh++;
				Keyboard[sc_PgUp] = 0;
			}

			if (Keyboard[sc_PgDn])
			{
				if (vh)
					vh--;
				Keyboard[sc_PgDn] = 0;
			}

			if (Keyboard[sc_End])
			{
				ce++;
				Keyboard[sc_End] = 0;
			}

			if (Keyboard[sc_Home])
			{
				if (ce)
					ce--;
				Keyboard[sc_Home] = 0;
			}

			viewheight = vh;
			centery = ce;
#endif

			viewheight = v_table[20-viewsize];
			centery = c_table[20-viewsize];
			MegaSimpleScaleShape(centerx,centery,shapenum,viewheight+1,0);

#if 0
			mclear();
			mprintf("viewheight: %d   \n",viewheight);
			mprintf("   centery: %d   \n",centery);
#endif
#else
			SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
#endif
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
			useBounceOffset=false;

			viewheight=oldviewheight;
		}
#endif
	}
}

//==========================================================================


/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics (void)
{
	int32_t	newtime,oldtimecount;

#ifdef MYPROFILE
	tics = 3;
	return;
#endif

//
// calculate tics since last refresh for adaptive timing
//
	if (lasttimecount > TimeCount)
		TimeCount = lasttimecount;		// if the game was paused a LONG time


/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
//#if 0

	if (DemoMode)					// demo recording and playback needs
	{								// to be constant
//
// take DEMOTICS or more tics, and modify Timecount to reflect time taken
//
		oldtimecount = lasttimecount;
		while (TimeCount<oldtimecount+DEMOTICS*2)
#ifdef __AMIGA__
			VL_WaitVBL(1)
			//BE_ST_ShortSleep()
#endif
		;
		lasttimecount = oldtimecount + DEMOTICS;
		TimeCount = lasttimecount + DEMOTICS;
		tics = DEMOTICS;
	}
	else
#endif
	{
//
// non demo, so report actual time
//
#ifdef __AMIGA__
		// TODO: this is a bit wonky
		newtime = TimeCount;
		tics = newtime-lasttimecount;
		while (!tics)
		{
			VL_WaitVBL(1);
			//BE_ST_ShortSleep();
			newtime = TimeCount;
			tics = newtime-lasttimecount;
		}
#else
		do
		{
			newtime = TimeCount;
			tics = newtime-lasttimecount;
		} while (!tics);			// make sure at least one tic passes
#endif

		lasttimecount = newtime;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
      framecount++;
#endif

#ifdef FILEPROFILE
			strcpy (scratch,"\tTics:");
			itoa (tics,str,10);
			strcat (scratch,str);
			strcat (scratch,"\n");
			write (profilehandle,scratch,strlen(scratch));
#endif

#ifdef DEBUGTICS
		VW_SetAtrReg (ATR_OVERSCAN,tics);
#endif

		realtics=tics;
		if (tics>MAXTICS)
		{
			TimeCount -= (tics-MAXTICS);
			tics = MAXTICS;
		}
	}
}


//==========================================================================


/*
========================
=
= FixOfs
=
========================
*/

void	FixOfs (void)
{
	VW_ScreenToScreen (displayofs,bufferofs,viewwidth/8,viewheight);
}


//==========================================================================



/*
====================
=
= WallRefresh
=
====================
*/

void WallRefresh (void)
{
//
// set up variables for this view
//

	viewangle = player->angle;
	midangle = viewangle*(FINEANGLES/ANGLES);
	viewsin = sintable[viewangle];
	viewcos = costable[viewangle];
	viewx = player->x - FixedByFrac(focallength,viewcos);
	viewy = player->y + FixedByFrac(focallength,viewsin);

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;

	viewtx = player->x >> TILESHIFT;
	viewty = player->y >> TILESHIFT;

	xpartialdown = viewx&(TILEGLOBAL-1);
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = viewy&(TILEGLOBAL-1);
	ypartialup = TILEGLOBAL-ypartialdown;

	lastside = -1;			// the first pixel is on a new wall

	AsmRefresh();
	ScalePost ();			// no more optimization on last post
}



//==========================================================================

extern int16_t MsgTicsRemain;
extern uint16_t LastMsgPri;

//-------------------------------------------------------------------------
// RedrawStatusAreas()
//-------------------------------------------------------------------------
void RedrawStatusAreas()
{
	int8_t loop;

   DrawInfoArea_COUNT = InitInfoArea_COUNT = 3;


	for (loop=0; loop<3; loop++)
	{
		LatchDrawPic(0,0,TOP_STATUSBARPIC);
		ShadowPrintLocationText(sp_normal);

		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
		LatchDrawPic(0,200-STATUSLINES,STATUSBARPIC);
#else
		JLatchDrawPic(0,200-STATUSLINES,STATUSBARPIC);
#endif
		DrawAmmoPic();
		DrawScoreNum();
		DrawWeaponPic();
		DrawAmmoNum();
		DrawKeyPics();
		DrawHealthNum();
		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
		DrawHealthMonitor();
		DrawHealthPic();
		LatchDrawPic(15,184,ECG_GRID_PIECE);
#endif

		bufferofs += SCREENSIZE;
		if (bufferofs > PAGE3START)
			bufferofs = PAGE1START;
	}
}

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
void F_MapLSRow();
void C_MapLSRow();
void MapLSRow();
#endif


/*
========================
=
= ThreeDRefresh
=
========================
*/
int16_t NextBuffer();
void	ThreeDRefresh (void)
{
	int16_t tracedir;

#ifndef __AMIGA__
// this wouldn't need to be done except for my debugger/video wierdness
	outportb (SC_INDEX,SC_MAPMASK);
#endif

//
// clear out the traced array
//
#ifdef __AMIGA__
	memset(spotvis, 0, sizeof(spotvis));
#else
asm	mov	ax,ds
asm	mov	es,ax
asm	mov	di,OFFSET spotvis
asm	xor	ax,ax
asm	mov	cx,2048							// 64*64 / 2
asm	rep stosw
#endif

#ifndef PAGEFLIP
	bufferofs = displayofs = screenloc[0];
#endif

	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
	// Different location for AOG
#ifndef GAMEVER_RESTORATION_AOG
	UpdateInfoAreaClock();
	UpdateStatusBar();
#endif

	bufferofs += screenofs;

//
// follow the walls from there to the right, drawwing as we go
//
#if 1
#ifdef CEILING_FLOOR_COLORS
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	if (gamestate.flags & GS_LIGHTING)
	switch (gamestate.flags & (GS_DRAW_FLOOR|GS_DRAW_CEILING))
	{
		case GS_DRAW_FLOOR|GS_DRAW_CEILING:
			MapRowPtr = MapLSRow;
			WallRefresh();
			DrawPlanes();
			break;

		case GS_DRAW_FLOOR:
			MapRowPtr = F_MapLSRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
			break;

		case GS_DRAW_CEILING:
			MapRowPtr = C_MapLSRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
			break;

		default:
			VGAClearScreen();
			WallRefresh();
			break;
	}
	else
#endif
	switch (gamestate.flags & (GS_DRAW_FLOOR|GS_DRAW_CEILING))
	{
		case GS_DRAW_FLOOR|GS_DRAW_CEILING:
			MapRowPtr = MapRow;
			WallRefresh();
			DrawPlanes();
			break;

		case GS_DRAW_FLOOR:
			MapRowPtr = F_MapRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
			break;

		case GS_DRAW_CEILING:
			MapRowPtr = C_MapRow;
			VGAClearScreen();
			WallRefresh();
			DrawPlanes();
			break;

		default:
			VGAClearScreen();
			WallRefresh();
			break;
	}
#else

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	if (gamestate.flags & GS_LIGHTING)
		MapRowPtr = MapLSRow;
	else
#endif
		MapRowPtr = MapRow;

	WallRefresh();
	DrawPlanes();

#endif
#endif
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
	// Different location for AOG
#ifndef GAMEVER_RESTORATION_AOG
	UpdateTravelTable();
#endif
#if 1
//
// draw all the scaled images
//

	DrawScaleds();			// draw scaled stuf

	DrawPlayerWeapon ();	// draw player's hands
#endif

//
// show screen and time last cycle
//
	if (fizzlein)
	{
		FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,70,false);
		fizzlein = false;

		lasttimecount = TimeCount;		// don't make a big tic count
	}

	bufferofs -= screenofs;

	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	displayofs = bufferofs;

#ifndef GAMEVER_RESTORATION_AOG_100
	if (gamestate.flags & GS_SHOW_OVERHEAD)
	{
		if (Keyboard[sc_LShift] || Keyboard[sc_RShift])
			ShowOverhead(0,136,OV_ACTORS|OV_PLAYER|OV_NODEBUG|OV_HIDDEN|OV_ROTATED|OV_DOORS|OV_KEYS);
		else
			ShowOverhead(0,136,OV_ACTORS|OV_PLAYER|OV_HIDDEN|OV_ROTATED|OV_DOORS|OV_KEYS);
	}
#endif
	UpdateTravelTable();
#else
#if 1
	DrawRadar();
#endif
#endif

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	VW_WaitVBL(1);
#else
//	VW_WaitVBL(1);		// mike check this out
#endif

#ifdef PAGEFLIP
	NextBuffer();
#endif

	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
	// Different location for AOG
#ifdef GAMEVER_RESTORATION_AOG
	UpdateInfoAreaClock();
	UpdateStatusBar();
#endif

#ifdef __AMIGA__
//#define DRAW_MEM
#ifdef DRAW_MEM
	BE_ST_DebugText(0, 16, "U %4d T %4d\n", MM_UnusedMemory()/1024, MM_TotalFree()/1024);
			/*BE_ST_DebugText(0, 8, "C %4d(%4d) F %4d(%4d)", 
				AvailMem(MEMF_CHIP)/1024, AvailMem(MEMF_CHIP | MEMF_LARGEST)/1024,
				AvailMem(MEMF_FAST)/1024, AvailMem(MEMF_FAST | MEMF_LARGEST)/1024);*/
#endif
#endif

	frameon++;
	PM_NextFrame();
}

//--------------------------------------------------------------------------
// NextBuffer()
//--------------------------------------------------------------------------
int16_t NextBuffer()
{
	displayofs=bufferofs;

#ifdef PAGEFLIP
#ifdef __AMIGA__
	VH_UpdateScreen();
	//VL_RefreshScreen(view_xl/*+TOP_STRIP_HEIGHT*/, view_yl, view_xh, view_yh);
#else
	asm	cli
	asm	mov	cx,[bufferofs]
	asm	mov	dx,3d4h		// CRTC address register
	asm	mov	al,0ch		// start address high register
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,ch
	asm	out	dx,al   	// set the high byte
	asm	sti
#endif
#endif

	bufferofs += SCREENSIZE;
	if (bufferofs > PAGE3START)
		bufferofs = PAGE1START;

#ifdef __AMIGA__
	return 0;
#endif
}

byte far TravelTable[MAPSIZE][MAPSIZE];

//--------------------------------------------------------------------------
// UpdateTravelTable()
//--------------------------------------------------------------------------
void UpdateTravelTable()
{
#ifdef __AMIGA__
	byte *travel = &TravelTable[0][0];
	byte *spot = &spotvis[0][0];

	for (int i = 0; i < MAPSIZE*MAPSIZE; i++)
	{
		*travel++ |= *spot++;
	}
	/*
	for (int i = 0; i < MAPSIZE; ++i)
	{
		for (int j = 0; j < MAPSIZE; ++j)
		{
			TravelTable[i][j] |= spotvis[i][j];
		}
	}
	*/
#else
asm	mov	si,OFFSET [spotvis]
asm	mov	ax,SEG [TravelTable]
asm	mov	es,ax
asm	mov	di,OFFSET [TravelTable]
asm	mov	cx,00800h							// HARDCODED for 64x64 / 2!!

loop1:
asm	mov	ax,[si]
asm	inc	si
asm   inc   si
asm	or		[es:di],ax
asm	inc	di
asm	inc	di
asm	loop	loop1
#endif
}

extern int16_t an_offset[];


/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
//--------------------------------------------------------------------------
// DrawRadar()
//--------------------------------------------------------------------------
void DrawRadar()
{
	int8_t zoom=gamestate.rzoom;

	byte flags = OV_KEYS|OV_PUSHWALLS|OV_ACTORS;

	if (gamestate.rpower)
	{
	   if ((frameon & 1) && (!godmode))
			if (zoom)
				gamestate.rpower -= tics<<zoom;

		if (gamestate.rpower < 0)
		{
			gamestate.rpower=0;
			DISPLAY_TIMED_MSG(RadarEnergyGoneMsg,MP_WEAPON_AVAIL,MT_GENERAL);
		}
		UpdateRadarGuage();
	}
	else
		zoom = 0;

	ShowOverhead(192,156,16,zoom,flags);
}

clock_t tc_start,tc_end;
uint16_t tc_time;
#endif


//--------------------------------------------------------------------------
// ShowOverhead()
//--------------------------------------------------------------------------
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
void ShowOverhead(int16_t bx, int16_t by, uint16_t flags)
#else
void ShowOverhead(int16_t bx, int16_t by, int16_t radius, int16_t zoom, uint16_t flags)
#endif
{
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG

#ifdef GAMEVER_RESTORATION_AOG_100
	#define PLAYER_COLOR 	0xf0
#else
	#define PLAYER_COLOR 	0xf1
#endif

	extern byte pixmasks[];

	statobj_t **stat;
#ifdef GAMEVER_RESTORATION_AOG_100
	fixed playerx,playery,baselmx,baselmy,dx,dy,psin,pcos;
#else
	fixed xoff,yoff,baselmx,baselmy,dx,dy,psin,pcos;
#endif
	uint16_t far *ptr[2];
#ifndef GAMEVER_RESTORATION_AOG_100
	int16_t iconnum;
#endif
	byte lastmask=0xFF,mask,unmappedcolor;
	// VERSIONS RESTORATION - A bit of cheating:
	// Swapping mapx,mapy with mx,my for convenience, for AOG v1.0
#ifdef GAMEVER_RESTORATION_AOG_100
	int16_t mapx,mapy,mx,my,mapbound;
	boolean checkhidden=true;
#else
	int16_t mx,my,mapx,mapy,playertx,playerty,angle;
	boolean checkhidden=true,drawplayerok=true;
#endif
	byte color,tile,door;
	objtype *ob;

	stat = KeyStatObjs;
#ifdef GAMEVER_RESTORATION_AOG_100
	playerx = player->x;
	playery = player->y;

	psin = sintable[player->angle];
	pcos = costable[player->angle];
#else
	playertx = player->tilex;
	playerty = player->tiley;
	xoff = (1L<<21)|8L;
	yoff = (1L<<21)|8L;

	angle = (ANGLES-1-player->angle)-ANGLES/2;
	if (angle < 0)
		angle += ANGLES;

	psin = sintable[angle];
	pcos = costable[angle];
#endif

	unmappedcolor = 6; /* Default (unmapped) gray shade of color */
	VW_Bar (bx,by,MAPSIZE,MAPSIZE,unmappedcolor);
#ifdef GAMEVER_RESTORATION_AOG_100
	if (flags & OV_ROTATED)
	{
		bx += 32;
		by += 32;
	}
#endif
	ptr[0] = mapsegs[0];
	ptr[1] = mapsegs[1];

	// VERSIONS RESTORATION - Cheating with mx/my and mapx/mapy again
#ifdef GAMEVER_RESTORATION_AOG_100
	for (my=0;my<MAPSIZE;++my)
		for (mx=0;mx<MAPSIZE;++mx,ptr[0]++,ptr[1]++)
#else
	for (mapy=0;mapy<MAPSIZE;++mapy)
		for (mapx=0;mapx<MAPSIZE;++mapx,ptr[0]++,ptr[1]++)
#endif
		{
			color = unmappedcolor;
#ifndef GAMEVER_RESTORATION_AOG_100
			if (flags & OV_ROTATED)
			{
				dx = xoff-(((uint32_t)mapx<<TILESHIFT)+8);
				dy = (((uint32_t)mapy<<TILESHIFT)+8)-yoff;
				baselmx = FixedByFrac(dy,pcos)-FixedByFrac(dx,psin);
				baselmy = FixedByFrac(dy,psin)+FixedByFrac(dx,pcos);

				mx = playertx+(baselmx>>16);
				my = playerty+(baselmy>>16);

				if (mx<0 || mx>63 || my<0 || my>63)
					continue;
			}
			else
			{
				mx = mapx;
				my = mapy;
			}
#endif
			tile=tilemap[mx][my];
			door=tile&0x3f;
			ob=(objtype *)actorat[mx][my];
#ifndef GAMEVER_RESTORATION_AOG_100
			iconnum = *(mapsegs[1]+farmapylookup[my]+mx);
#endif

			if (tile)
			{
				checkhidden = false;
				if ((flags & OV_DOORS) && (tile & 0x80))
				{
					if (doorobjlist[door].type == dr_elevator)
						color = 0xfb;
					else if (doorobjlist[door].lock != kt_none)
						color = 0x18;
					else if (doorobjlist[door].action == dr_closed)
						color = 0x58;
					else
						checkhidden = true;
				}
			}

			if (checkhidden)
			{
				if (flags & OV_HIDDEN)
				{
					GetAreaNumber(mx,my);
					if (GAN_HiddenArea)
						color = 0x52;
					else
						color = 0x55;
				}
				if (flags & OV_TILES)
					color = *(ptr[0])&0xFF;
			}
			else
				checkhidden = true;

			if ((flags & OV_ACTORS) && (ob >= objlist))
				color = 0x10+ob->obclass;

#ifndef GAMEVER_RESTORATION_AOG_100
			if ((flags & OV_PUSHWALLS) && (iconnum == PUSHABLETILE))
			{
				if (TravelTable[mx+1][my]
				||  TravelTable[mx-1][my]
				||  TravelTable[mx][my+1]
				||  TravelTable[mx][my-1]
				||  !(flags & OV_NODEBUG))
				{
					color = 0xf7;
					TravelTable[mx][my] = TT_TRAVELED;
				}
			}
#endif

#ifdef __AMIGA__
			// TODO (TravelTable[mx][my] & TT_KEYS) != 0
			if ((flags & OV_KEYS) && *stat && ((*stat)->tilex == mx) && ((*stat)->tiley == my))
#else
			if ((flags & OV_KEYS) && stat && *stat && ((*stat)->tilex == mx) && ((*stat)->tiley == my))
#endif
			{
				if (((*stat)->shapenum != -1) && ((*stat)->itemnumber >= bo_red_key) && ((*stat)->itemnumber <= bo_gold_key))
					color = 0xf2;
				++stat;
			}

			if (flags & OV_NODEBUG)
				if (!TravelTable[mx][my])
					color = unmappedcolor;

#ifdef GAMEVER_RESTORATION_AOG_100
			if ((flags & OV_PLAYER) && (player->tilex == mx) && (player->tiley == my))
#else
			if ((flags & OV_PLAYER) && (player->tilex == mx) && (player->tiley == my) && drawplayerok)
#endif
			{
				color = PLAYER_COLOR;
#ifndef GAMEVER_RESTORATION_AOG_100
				drawplayerok = false;
#endif
			}
//
			if (color != unmappedcolor)
			{
				// VERSIONS RESTORATION - Cheating with mx/my and mapx/mapy again
#ifdef GAMEVER_RESTORATION_AOG_100
				if (flags & OV_ROTATED)
				{
					dx = playerx-(((uint32_t)mx<<TILESHIFT)+8);
					dy = (((uint32_t)my<<TILESHIFT)+8)-playery;
					baselmx = FixedByFrac(dy,pcos)-FixedByFrac(dx,psin);
					baselmy = FixedByFrac(dy,psin)+FixedByFrac(dx,pcos);

					mapx = baselmx>>16;
					mapy = baselmy>>16;
					mapbound = 32;
				}
				else
				{
					mapx = mx;
					mapy = my;
					mapbound = 64;
				}
#endif
#ifndef __AMIGA__
				mask = pixmasks[(bx+mapx)&3];
				if (mask != lastmask)
				{
					lastmask = mask;
					VGAMAPMASK(mask);
				}
#endif
				// (VERSIONS RESTORATION) This is compiled to the desired code
#ifdef GAMEVER_RESTORATION_AOG_100
				if ((-mapbound < mapx) && (mapx < mapbound) && (-mapbound < mapy) && (mapy < mapbound))
#endif
					//*(byte far *)MK_FP(SCREENSEG,ylookup[by+mapy]+((bx+mapx)>>2)+bufferofs) = color;
					vga_memory[vl_get_offset(bufferofs, bx+mapx, by+mapy)] = color;
			}
		}

	VGAMAPMASK(15);
#else // VERSIONS RESTORATION
	#define PLAYER_COLOR 	0xf1
	#define UNMAPPED_COLOR	0x52
	#define MAPPED_COLOR		0x55

	extern byte pixmasks[];
	extern byte far rndtable[];

	byte color,quad;
	byte tile,door;
	objtype *ob;

	fixed dx,dy,psin,pcos,lmx,lmy,baselmx,baselmy,xinc,yinc;
	int16_t rx,ry,mx,my;
	byte far *dstptr,far *basedst,mask,startmask;
	boolean drawplayerok=true;
	byte rndindex;
	boolean snow=false;

// -zoom == make it snow!
//
	if (zoom<0)
	{
		zoom = 0;
		snow = true;
		rndindex = US_RndT();
	}

	zoom = 1<<zoom;
	radius /= zoom;

// Get sin/cos values
//
	psin=sintable[player->angle];
	pcos=costable[player->angle];

// Convert radius to fixed integer and calc rotation.
//
	dx = dy = (int32_t)radius<<TILESHIFT;
	baselmx = player->x+(FixedByFrac(dx,pcos)-FixedByFrac(dy,psin));
	baselmy = player->y-(FixedByFrac(dx,psin)+FixedByFrac(dy,pcos));
//printf("ang %d x %d y %d sin0 %08x cos0 %08x baselmx %d baselmy %d\n", player->angle, player->x, player->y, psin, pcos, baselmx, baselmy);
// Carmack's sin/cos tables use one's complement for negative numbers --
// convert it to two's complement!
//
	if (pcos & 0x80000000)
		pcos = -(pcos & 0xffff);

	if (psin & 0x80000000)
		psin = -(psin & 0xffff);

// Get x/y increment values.
//
	xinc = -pcos;
	yinc = psin;

// Calculate starting destination address.
//
	//basedst=MK_FP(SCREENSEG,bufferofs+ylookup[by]+(bx>>2));
	basedst=&vga_memory[vl_get_offset(bufferofs, bx, by)];
#ifndef __AMIGA__
	switch (zoom)
	{
		case 1:
			startmask = 1;
			mask = pixmasks[bx&3];
		break;

		case 2:							// bx MUST be byte aligned for 2x zoom
			mask = startmask = 3;
		break;

		case 4:							// bx MUST be byte aligned for 4x zoom
			mask = startmask = 15;
		break;
	}
	VGAMAPMASK(mask);
#endif

// Draw rotated radar.
//
	rx = radius*2;
	while (rx--)
	{
		lmx = baselmx;
		lmy = baselmy;

		dstptr = basedst;

		ry = radius*2;
		while (ry--)
		{
			if (snow)
			{
				color = 0x42+(rndtable[rndindex]&3);
				rndindex++; 		// += ((rndindex<<1) + 1);
				goto nextx;
			}

		// Don't evaluate if point is outside of map.
		//
			color = UNMAPPED_COLOR;
			mx = lmx>>16;
			my = lmy>>16;
			if (mx<0 || mx>63 || my<0 || my>63)
				goto nextx;

		// SHOW PLAYER
		//
			if (drawplayerok && player->tilex==mx && player->tiley==my)
			{
				color = PLAYER_COLOR;
				drawplayerok=false;
			}
			else
		// SHOW TRAVELED
		//
			if ((TravelTable[mx][my] & TT_TRAVELED) || (flags & OV_SHOWALL))
			{
			// What's at this map location?
			//
				tile=tilemap[mx][my];
				door=tile&0x3f;

			// Evaluate wall or floor?
			//
				if (tile)
				{
				// SHOW DOORS
				//
					if (tile & 0x80)
					{
							if (doorobjlist[door].lock!=kt_none)
								color=0x18;										// locked!
							else
								if (doorobjlist[door].action==dr_closed)
									color=0x58;									// closed!
								else
									color = MAPPED_COLOR;					// floor!
					}
				}
				else
					color = MAPPED_COLOR;									// floor!

			// SHOW KEYS
			//
				if ((flags & OV_KEYS) && (TravelTable[mx][my] & TT_KEYS))
					color = 0xf3;

				if ((zoom > 1) || (ExtraRadarFlags & OV_ACTORS))
				{
					ob=(objtype *)actorat[mx][my];

				// SHOW ACTORS
				//
					if ((flags & OV_ACTORS) && (ob >= objlist) && (!(ob->flags & FL_DEADGUY)) &&
						 (ob->obclass > deadobj) && (ob->obclass < SPACER1_OBJ))
						color = 0x10+ob->obclass;

					if ((zoom == 4) || (ExtraRadarFlags & OV_PUSHWALLS))
					{
						uint16_t iconnum;

						iconnum = *(mapsegs[1]+farmapylookup[my]+mx);

					// SHOW PUSHWALLS
					//
						if ((flags & OV_PUSHWALLS) && (iconnum == PUSHABLETILE))
							color = 0x79;
					}
				}
			}
			else
				color = UNMAPPED_COLOR;

nextx:;
		// Display pixel for this quadrant and add x/y increments
		//
#ifdef __AMIGA__
			// this is kinda shitty
			if (zoom == 1)
			{
				*dstptr = color;
				dstptr+=vga_width;
			}
			else
			{
				for (int16_t i = 0; i < zoom; i++)
				{
					for (int16_t j = 0; j < zoom; j++)
					{
						*dstptr++ = color;
					}
					dstptr += (vga_width-zoom);
				}
			}
#else
			*dstptr = color;
			dstptr += 80;

			if (zoom > 1)						// handle 2x zoom
			{
				*dstptr = color;
				dstptr += 80;

				if (zoom > 2)					// handle 4x zoom
				{
					*dstptr = color;
					dstptr += 80;

					*dstptr = color;
					dstptr += 80;
				}
			}
#endif

			lmx += xinc;
			lmy += yinc;
		}

		baselmx += yinc;
		baselmy -= xinc;

#ifdef __AMIGA__
		basedst+=zoom;
#else
		mask <<= zoom;
		if (mask>15)
		{
			mask=startmask;
			basedst++;
		}
		VGAMAPMASK(mask);
#endif
	}

	VGAMAPMASK(15);
#endif
}

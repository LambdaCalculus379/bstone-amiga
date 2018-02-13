/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include <cybergraphx/cybergraphics.h>
#include <proto/intuition.h>
#include <proto/cybergraphics.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#define IPTR ULONG
#define Point BSPoint
#include "id_heads.h"
#define USE_DOUBLEBUFFER
//#define TEST_OFFSET 0
//#define TEST_OFFSET 150
//#undef KALMS_C2P

uint16_t bufferofs;
uint16_t displayofs;

uint16_t	linewidth;
uint16_t	ylookup[MAXSCANLINES];

boolean screenfaded;

uint8_t palette1[256][3];
uint8_t palette2[256][3];


//
// Common stuff
//

static struct Screen *screen = NULL;
static struct Window *window = NULL;
static UWORD *pointermem = NULL;
struct Library *CyberGfxBase = NULL;
static boolean contiguous = false;
//static struct BitMap *screenBitMap;
static int currentBitMap;
//#ifdef USE_DOUBLEBUFFER
struct ScreenBuffer *sbuf[2];
//#endif

uint8_t vga_palette1[3 * 256];
uint8_t vga_palette[3 * 256];
uint8_t vga_palette2[3 * 256];

//extern byte vgapal[768];

uint8_t vga_memory[256 * 1024];


// ===========================================================================

// asm

void VL_WaitVBL(int16_t vbls)
{
	while (vbls-- > 0)
		WaitTOF();
}

#ifdef KALMS_C2P
extern void c2p1x1_8_c5_030_init(
	register WORD chunkyx __asm("d0"),
	register WORD chunkyy __asm("d1"),
	register WORD scroffsy __asm("d3"));

extern void c2p1x1_8_c5_030(
	register APTR c2pscreen __asm("a0"),
	register APTR bitplanes __asm("a1"));

extern void c2p1x1_8_c5_040_init(
	register WORD chunkyx __asm("d0"),
	register WORD chunkyy __asm("d1"),
	register WORD scroffsy __asm("d3"),
	register LONG bplsize __asm("d5"));

extern void c2p1x1_8_c5_040(
	register APTR c2pscreen __asm("a0"),
	register APTR bitplanes __asm("a1"));

extern void c2p1x1_8_c3b1_init(
	register WORD chunkyx __asm("d0"),
	register WORD chunkyy __asm("d1"),
	register WORD scroffsy __asm("d3"),
	register LONG bplsize __asm("d5"));

extern void c2p1x1_8_c3b1(
	register APTR c2pscreen __asm("a0"),
	register APTR bitplanes __asm("a1"));
#endif

// ===========================================================================


/*
=======================
=
= VL_Startup
=
=======================
*/

// BBi Moved from jm_free.cpp
void VL_Startup(void)
{
}
// BBi

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown(void)
{
	if (sbuf[0])
	{
		FreeScreenBuffer(screen, sbuf[0]);
		sbuf[0] = NULL;
	}

#ifdef USE_DOUBLEBUFFER
	if (sbuf[1])
	{
		FreeScreenBuffer(screen,sbuf[1]);
		sbuf[1] = NULL;
	}
#endif

	if (window)
	{
		CloseWindow(window);
		window = NULL;
	}

	if (screen)
	{
		CloseScreen(screen);
		screen = NULL;
	}

	if (pointermem)
	{
		FreeVec(pointermem);
		pointermem = NULL;
	}

	if (CyberGfxBase)
	{
		CloseLibrary(CyberGfxBase);
		CyberGfxBase = NULL;
	}
}

/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

static ULONG filterFunc(struct Hook *hook, struct ScreenModeRequester *req, ULONG modeid)
{
	APTR handle;
	struct DimensionInfo diminfo;
	struct DisplayInfo dispinfo;
	struct NameInfo nameinfo;

	if (!(handle = FindDisplayInfo(modeid)))
		return 0;
	if (!GetDisplayInfoData(handle, (UBYTE *)&diminfo, sizeof(diminfo), DTAG_DIMS, 0))
		return 0;
	if (!GetDisplayInfoData(handle, (UBYTE *)&dispinfo, sizeof(dispinfo), DTAG_DISP, 0))
		return 0;

	#undef MaxX
	#undef MaxY

	/*if (GetDisplayInfoData(handle, (UBYTE *)&nameinfo, sizeof(nameinfo), DTAG_NAME, 0))
	{
		printf("%s modeid %08x name %s\n", __FUNCTION__, modeid, nameinfo.Name);
		printf("%s d %ld mx %d my %d pf %d\n", __FUNCTION__, diminfo.MaxDepth, diminfo.Nominal.MaxX+1, diminfo.Nominal.MaxY+1, (dispinfo.PropertyFlags & DIPF_IS_LACE));
	}*/

	return (diminfo.MaxDepth == 8 &&
		diminfo.Nominal.MaxX+1 == vga_width &&
		diminfo.Nominal.MaxY+1 >= vga_height /*&&
		(dispinfo.PropertyFlags & DIPF_IS_LACE) == 0*/);
}

void VL_SetVGAPlaneMode(void)
{
	memset(vga_palette, 0, sizeof(vga_palette));

	ULONG modeid = INVALID_ID;

	/*if (!CyberGfxBase)
		CyberGfxBase = OpenLibrary("cybergraphics.library", 0);*/

	/*if (CyberGfxBase)
	{
		modeid = BestCModeIDTags(
			CYBRBIDTG_NominalWidth, vga_width,
			CYBRBIDTG_NominalHeight, vga_height,
			CYBRBIDTG_Depth, 8,
			TAG_DONE);
	}*/

	if (modeid == INVALID_ID)
	{
		modeid = BestModeID(
			BIDTAG_NominalWidth, vga_width,
			BIDTAG_NominalHeight, vga_height,
			BIDTAG_Depth, 8,
			BIDTAG_MonitorID, DEFAULT_MONITOR_ID,
			TAG_DONE);
	}

	struct ScreenModeRequester *req;
	if (/*_argc == 0 &&*/ (req = AllocAslRequestTags(ASL_ScreenModeRequest, TAG_DONE)))
	{
		struct Hook filterHook;
		filterHook.h_Entry = HookEntry;
		filterHook.h_SubEntry = (HOOKFUNC)filterFunc;
		if (AslRequestTags(req, ASLSM_InitialDisplayID, modeid, ASLSM_FilterFunc, (IPTR)&filterHook, TAG_DONE))
		{
			modeid=req->sm_DisplayID;
			printf("%s got modeid %lx\n", __FUNCTION__, modeid);
		}
		FreeAslRequest(req);
	}

	if ((screen = OpenScreenTags(NULL, 
		SA_DisplayID, modeid,
		SA_Width, vga_width,
		SA_Height, vga_height,
		SA_Depth, 8,
		SA_ShowTitle, FALSE,
		SA_Quiet, TRUE,
		SA_Draggable, FALSE,
		SA_Type, CUSTOMSCREEN,
		TAG_DONE)))
	{
		contiguous = false;
#ifdef KALMS_C2P
		// make sure we actually got an contiguous standard bitmap
		struct BitMap *bm = screen->RastPort.BitMap;
		//printf("%s bpr %d rows %d flags %08x\n", __FUNCTION__, bm->BytesPerRow, bm->Rows, GetBitMapAttr(bm, BMA_FLAGS));

		if ((GetBitMapAttr(bm, BMA_FLAGS) & BMF_STANDARD) /*== BMF_STANDARD*/)
		{
			contiguous = true;
			for (int i = 1; i < bm->Depth; i++)
			{
				intptr_t diff = (intptr_t)bm->Planes[i]-(intptr_t)bm->Planes[i-1];
				/*
				printf("%s Planes[%d] = %p\n", __FUNCTION__, i, bm->Planes[i]);
				printf("%s diff %ld\n", __FUNCTION__, diff);
				*/
				if (diff != bm->BytesPerRow*bm->Rows)
				{
					//printf("%s non-contiguous bitmap, fallback to WCP8...\n", __FUNCTION__);
					contiguous = false;
					break;
				}
			}
		}

		if (contiguous)
		{
			//printf("%s c2p setup %d %d %d\n", __FUNCTION__, vga_width, vga_height, bm->BytesPerRow*bm->Rows);
			if (SysBase->AttnFlags & AFF_68040)
			{
				c2p1x1_8_c5_040_init(vga_width, vga_height, 0, bm->BytesPerRow*bm->Rows);
			}
			else
			{
				c2p1x1_8_c5_030_init(vga_width, vga_height, 0);
				//c2p1x1_8_c3b1_init(vga_width, vga_height, 0, bm->BytesPerRow*bm->Rows);
			}
		}

		/*printf("%s C %4d(%4d) F %4d(%4d)\n", __FUNCTION__,
				AvailMem(MEMF_CHIP)/1024, AvailMem(MEMF_CHIP | MEMF_LARGEST)/1024,
				AvailMem(MEMF_FAST)/1024, AvailMem(MEMF_FAST | MEMF_LARGEST)/1024);*/
		//Quit("test");
#endif

		sbuf[0] = AllocScreenBuffer(screen, 0, SB_SCREEN_BITMAP);
#ifdef USE_DOUBLEBUFFER
		if (contiguous)
		{
			sbuf[1] = AllocScreenBuffer(screen, 0, /*SB_COPY_BITMAP*/0);
			if (!sbuf[1])
			{
				printf("%s can't allocate the second screen buffer, fallback to WPA8\n", __FUNCTION__);
				contiguous = false;
			}
		}
#endif
		currentBitMap = 0;
		// this fixes some RTG modes which would otherwise display garbage on the 1st buffer swap
		extern void VL_RefreshScreen(void);
		VL_RefreshScreen();

		if ((window = OpenWindowTags(NULL,
			WA_Flags, WFLG_BACKDROP | WFLG_REPORTMOUSE | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP | WFLG_SIMPLE_REFRESH,
			WA_InnerWidth, vga_width,
			WA_InnerHeight, vga_height,
			WA_CustomScreen, (IPTR)screen,
			TAG_DONE)))
		{
			pointermem = (UWORD *)AllocVec(16 * 16, MEMF_CLEAR | MEMF_CHIP);
			SetPointer(window, pointermem, 16, 16, 0, 0);
			return;
		}
	}

	VL_Shutdown();
	// painful death awaits
	Quit("%s can't open the screen", __FUNCTION__);
}

// ===========================================================================

/*
====================
=
= VL_SetLineWidth
=
= Line witdh is in WORDS, 40 words is normal width for vgaplanegr
=
====================
*/

void VL_SetLineWidth(uint16_t width)
{
	int16_t i,offset;

//
// set up lookup tables
//
	linewidth = width*2;

	offset = 0;

	for (i=0;i<MAXSCANLINES;i++)
	{
		ylookup[i]=offset;
		offset += linewidth;
	}
}


/*
=============================================================================

								PALETTE OPS

				To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/


/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette (int16_t red, int16_t green, int16_t blue)
{
	for (int i = 0; i < 256; ++i)
	{
		vga_palette[(3 * i) + 0] = (uint8_t)red;
		vga_palette[(3 * i) + 1] = (uint8_t)green;
		vga_palette[(3 * i) + 2] = (uint8_t)blue;
	}

	VL_SetPalette(0, 255, vga_palette);
}

//===========================================================================

/*
=================
=
= VL_SetPalette
=
= If fast palette setting has been tested for, it is used
= (some cards don't like outsb palette setting)
=
=================
*/

void VL_SetPalette (byte firstreg, uint16_t numregs, byte far *palette)
{
	int entry;
	ULONG palette32[256 * 3 + 2];
	/*
	for (entry = firstreg; entry < numregs; ++entry)
	{
		vga_palette[entry * 3 + 0] = *palette++;
		vga_palette[entry * 3 + 1] = *palette++;
		vga_palette[entry * 3 + 2] = *palette++;
	}
	*/
	memcpy(&vga_palette[3 * firstreg], palette, 3 * numregs);

	palette32[0] = 256 << 16 | 0;
	for (entry = 0; entry < 256; entry++)
	{
		palette32[entry * 3 + 1] = vga_palette[entry * 3 + 0] << 26;
		palette32[entry * 3 + 2] = vga_palette[entry * 3 + 1] << 26;
		palette32[entry * 3 + 3] = vga_palette[entry * 3 + 2] << 26;
	}
	palette32[entry * 3 + 1] = 0;
	LoadRGB32(&screen->ViewPort, palette32);
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
= This does not use the port string instructions,
= due to some incompatabilities
=
=================
*/

void VL_GetPalette (byte firstreg, uint16_t numregs, byte far *palette)
{
	/*
	for (int entry = firstreg; entry < numregs; ++entry)
	{
		palette[entry * 3 + 0] = vga_palette[entry * 3 + 0];
		palette[entry * 3 + 1] = vga_palette[entry * 3 + 1];
		palette[entry * 3 + 2] = vga_palette[entry * 3 + 2];
	}
	*/
	memcpy(palette, &vga_palette[3 * firstreg], 3 * numregs);
}

//===========================================================================


/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut (int16_t start, int16_t end, int16_t red, int16_t green, int16_t blue, int16_t steps)
{
	int orig;
	int delta;

	VL_GetPalette(0, 256, &palette1[0][0]);
	memcpy(palette2, palette1, 768);

	//
	// fade through intermediate frames
	//
	for (int i = 0; i < steps; ++i)
	{
		uint8_t *origptr = &palette1[start][0];
		uint8_t *newptr = &palette2[start][0];
		for (int j = start; j <= end; ++j)
		{
			orig = *origptr++;
			delta = red - orig;
			*newptr++ = (uint8_t)(orig + ((delta * i) / steps));
			orig = *origptr++;
			delta = green - orig;
			*newptr++ = (uint8_t)(orig + ((delta * i) / steps));
			orig = *origptr++;
			delta = blue - orig;
			*newptr++ = (uint8_t)(orig + ((delta * i) / steps));
		}

		VL_SetPalette(0, 256, &palette2[0][0]);
		VL_WaitVBL(1);
	}

//
// final color
//
	VL_FillPalette((uint8_t)red, (uint8_t)green, (uint8_t)blue);

	VL_WaitVBL(1);

	screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn (int16_t start, int16_t end, byte far *palette, int16_t steps)
{
	VL_GetPalette(0, 256, &palette1[0][0]);
	memcpy(palette2, palette1, 768);

	start *= 3;
	end = (end * 3) + 2;

//
// fade through intermediate frames
//
	for (int i = 0; i < steps; ++i)
	{
		for (int j = start; j <= end; ++j)
		{
			int delta = palette[j] - palette1[0][j];
			palette2[0][j] = (uint8_t)(palette1[0][j] + ((delta * i) / steps));
		}

		VL_SetPalette(0, 256, &palette2[0][0]);
		VL_WaitVBL(1);
	}

//
// final color
//
	VL_SetPalette(0, 256, palette);
	VL_WaitVBL(1);

	screenfaded = false;
}

//------------------------------------------------------------------------
// VL_SetPaletteIntensity()
//------------------------------------------------------------------------
void VL_SetPaletteIntensity(int16_t start, int16_t end, byte far *palette, int8_t intensity)
{
	int16_t loop;
	int8_t red,green,blue;
	byte far *cmap = &palette1[0][0]+start*3;

	intensity = 63 - intensity;
	for (loop=start; loop<=end;loop++)
	{
		red = *palette++ - intensity;
		if (red < 0)
			red = 0;
		*cmap++ = red;

		green = *palette++ - intensity;
		if (green < 0)
			green = 0;
		*cmap++ = green;

		blue = *palette++ - intensity;
		if (blue < 0)
			blue = 0;
		*cmap++ = blue;
	}

	VL_SetPalette(start,end-start+1,&palette1[0][0]);
}

void VL_ColorBorder (int16_t color)
{
	// not implemented, only used for debug purposes
}

/*
=============================================================================

 PIXEL OPS

=============================================================================
*/


/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot (int16_t x, int16_t y, int16_t color)
{
	vga_memory[vl_get_offset(bufferofs, x, y)] = color;
}


/*
=================
=
= VL_Hlin
=
=================
*/

void VL_Hlin (uint16_t x, uint16_t y, uint16_t width, uint16_t color)
{
	//VL_Bar(x, y, width, 1, color);
	uint8_t *dest = &vga_memory[vl_get_offset(bufferofs, x, y)];
	memset(dest, color, width);
}


/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (int16_t x, int16_t y, int16_t height, int16_t color)
{
	//VL_Bar(x, y, 1, height, color);
	uint8_t *dest = &vga_memory[vl_get_offset(bufferofs, x, y)];

	while (height--)
	{
		*dest = color;
		dest += vga_width;
	}
}


/*
=================
=
= VL_Bar
=
=================
*/

void VL_Bar (int16_t x, int16_t y, int16_t width, int16_t height, int16_t color)
{
	uint8_t *dest = &vga_memory[vl_get_offset(bufferofs, x, y)];

	if (x == 0 && width == vga_width)
	{
		memset(dest, color, width*height);
	}
	else
	{
		while (height--)
		{
			memset(dest, color, width);
			dest += vga_width;
		}
	}
}

/*
============================================================================

 MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToLatch
=
=================
*/

void VL_MemToLatch (byte far *source, int16_t width, int16_t height, uint16_t dest)
{
	int base_offset = vl_get_offset(dest, 0, 0);

	// TODO: optimize this
	// ... or maybe not, only used by LoadLatchMem at load time
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				vga_memory[base_offset + (h * width) + w] = *source++;
			}
		}
	}
}


//===========================================================================


/*
=================
=
= VL_MemToScreen
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MemToScreen (byte far *source, int16_t width, int16_t height, int16_t x, int16_t y) // PG-13 OK???
{
	// TODO is this actually faster?
	// only used in the menus
	uint8_t *dst_pixels = &vga_memory[vl_get_offset(bufferofs, x, y)];
	for (int j=0; j<height; j++)
	{
		for (int i=0; i<width; i++)
		{
			byte color = source[(j*(width>>2)+(i>>2))+(i&3)*(width>>2)*height];
			//color = rand() & 0xFF;
			*dst_pixels++ = color;
		}
		dst_pixels += (vga_width-width);
	}
	/*
	// TODO: optimize this
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				VL_Plot(x + w, y + h, *source++);
			}
		}
	}
	*/
}


//===========================================================================


/*
=================
=
= VL_MemToScreen
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MaskMemToScreen (byte far *source, int16_t width, int16_t height, int16_t x, int16_t y, byte mask) // ???
{
	// TODO is this actually faster?
	// used in DrawMPic, InputFloor
	uint8_t *dst_pixels = &vga_memory[vl_get_offset(bufferofs, x, y)];
	for (int j=0; j<height; j++)
	{
		for (int i=0; i<width; i++)
		{
			byte color = source[(j*(width>>2)+(i>>2))+(i&3)*(width>>2)*height];
			if (color != mask)
			{
				*dst_pixels = color;
			}
			dst_pixels++;
		}
		dst_pixels += (vga_width-width);
	}

	/*
	// TODO: optimize this
	for (int p = 0; p < 4; ++p) {
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				uint8_t color = *source++;

				if (color != mask)
				{
					VL_Plot(x + w, y + h, color);
				}
			}
		}
	}
	*/
}


//------------------------------------------------------------------------
// VL_ScreenToMem()
//------------------------------------------------------------------------
void VL_ScreenToMem(byte far *dest, int16_t width, int16_t height, int16_t x, int16_t y)
{
	// TODO: optimize this
	// only used by SaveOverheadChunk
	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				*dest++ = vl_get_pixel(bufferofs, x + w, y + h);
			}
		}
	}
}


//==========================================================================

/*
=================
=
= VL_LatchToScreen
=
=================
*/

void VL_LatchToScreen (uint16_t source, int16_t width, int16_t height, int16_t x, int16_t y)// intro ok
{
	int src_pitch = /*vga_scale **/ 4 * width;
	int dst_pitch = vga_width-src_pitch;
	//int src_offset = vl_get_offset(source, 0, 0);
	//int dst_offset = vl_get_offset(bufferofs, x, y);

	uint8_t *src_pixels = &vga_memory[vl_get_offset(source, 0, 0)];
	uint8_t *dst_pixels = &vga_memory[vl_get_offset(bufferofs, x, y)];

	for (int h = 0; h < height; ++h)
	{
		for (int j = 0; j < src_pitch; ++j)
		{
			//vga_memory[dst_offset + j] = vga_memory[src_offset + j];
			*dst_pixels++ = *src_pixels++;
		}

		dst_pixels += dst_pitch;
		/*
		src_offset += src_pitch;
		dst_offset += vga_width;
		*/
	}
}


//===========================================================================

/*
=================
=
= VL_ScreenToScreen
=
=================
*/

void VL_ScreenToScreen (uint16_t source, uint16_t dest,int16_t width, int16_t height) // ???
{
	uint8_t *src_pixels = &vga_memory[vl_get_offset(source, 0, 0)];
	uint8_t *dst_pixels = &vga_memory[vl_get_offset(dest, 0, 0)];

	// TODO: is this correct?
	// apparently yes, used by the end game screen
	for (int h = 0; h < height; ++h)
	{
		for (int j = 0; j < width; ++j)
		{
			*dst_pixels++ = *src_pixels++;
		}
	}
}

void JM_VGALinearFill(int32_t start,int32_t length, int8_t fill)
{
	memset(&vga_memory[vl_get_offset(start, 0, 0)], fill, 4 * length);
}

void VL_RefreshScreen(void)
{
	uint8_t *chunky = &vga_memory[vl_get_offset(displayofs, 0, 0)];
#ifdef KALMS_C2P
	if (contiguous)
	{
#ifdef USE_DOUBLEBUFFER
		currentBitMap ^= 1;
#endif
		struct BitMap *bm = sbuf[currentBitMap]->sb_BitMap;
		if (SysBase->AttnFlags & AFF_68040)
		{
			c2p1x1_8_c5_040(chunky, bm->Planes[0]);
		}
		else
		{
			c2p1x1_8_c5_030(chunky, bm->Planes[0]);
			//c2p1x1_8_c3b1(chunky, bm->Planes[0]);
		}
#ifdef USE_DOUBLEBUFFER
		ChangeScreenBuffer(screen, sbuf[currentBitMap]);
#endif
	}
	else
#endif
	{
		//WaitTOF();
		struct BitMap *bm = sbuf[currentBitMap]->sb_BitMap;
		struct RastPort rp;
		InitRastPort(&rp);
		rp.BitMap = bm;
		WritePixelArray8(&rp, 0, 0, vga_width-1, vga_height-1, chunky, NULL);
		//WriteChunkyPixels(&rp, 0, 0, vga_width-1, vga_height-1, chunky, vga_width);
	}
}

void VH_UpdateScreen(void)
{
	if (displayofs != bufferofs)
	{
		memcpy(&vga_memory[vl_get_offset(displayofs, 0, 0)], &vga_memory[vl_get_offset(bufferofs, 0, 0)], vga_width * vga_height);
	}

	VL_RefreshScreen();
}

#include <stdarg.h>

void BE_ST_DebugText(int x, int y, const char *fmt, ...)
{
	UBYTE buffer[256];
	struct IntuiText WinText = {BLOCKPEN, DETAILPEN, JAM1 /*| INVERSVID*/, 0, 0, NULL, NULL, NULL};
	va_list ap;

	WinText.IText = buffer;

	va_start(ap, fmt); 
	vsnprintf((char *)buffer, sizeof(buffer), fmt, (char *)ap);
	va_end(ap);

	PrintIText(&screen->RastPort, &WinText, x, y);
}


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


//#include <stdexcept>
//#include <vector>
/*#include <cybergraphx/cybergraphics.h>
#include <proto/intuition.h>
#include <proto/cybergraphics.h>
#include <proto/exec.h>
#define IPTR ULONG
#define Point BSPoint*/
#include <SDL.h>
#include "id_heads.h"
//#include "bstone_ogl_api.h"


uint16_t bufferofs;
uint16_t displayofs;

//uint16_t* ylookup = NULL;
uint16_t	ylookup[MAXSCANLINES];

boolean screenfaded;

uint8_t palette1[256][3];
uint8_t palette2[256][3];


// BBi
//namespace {


//
// Common stuff
//

/*static struct Screen *screen = NULL;
static struct Window *window = NULL;
static UWORD *pointermem = NULL;
struct Library *CyberGfxBase = NULL;*/
static SDL_Surface	*screen;

/*const int default_window_width = 640;
const int default_window_height = 480;

int window_width = 0;
int window_height = 0;*/


//uint8_t* vga_palette = NULL;
uint8_t vga_palette1[3 * 256];
uint8_t vga_palette[3 * 256];
uint8_t vga_palette2[3 * 256];

//extern byte vgapal[768];

uint8_t vga_memory[256 * 1024];
/*
int vga_scale = 0;
int vga_width = 0;
int vga_height = 0;
int vga_area = 0;// width*height

int screen_x = 0;
int screen_y = 0;

int screen_width = 0;
int screen_height = 0;
*/
// BBi


// ===========================================================================

// asm

void VL_WaitVBL(int16_t vbls)
{
	while (vbls-- > 0)
		//WaitTOF();
		//SDL_Delay(20);
		BE_ST_ShortSleep();
}

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
	/*free(vga_memory);
	vga_memory = NULL;*/
	/*free(ylookup);
	ylookup = NULL;*/

	/*
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
	*/
	if (screen)
	{
		SDL_FreeSurface(screen);
		screen = NULL;
	}
}

/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/
int initflag = 0;

void VL_SetVGAPlaneMode(void)
{
	/*const int vga_size = vga_scale * vga_scale * vga_ref_size;
	vga_memory = calloc(1, vga_size);
	if (!vga_memory)
	{
		return;
	}*/
	if (!initflag)
	{
		initflag = 1;
		if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO/*SDL_INIT_EVERYTHING*/) < 0)
			printf("VID: Couldn't load SDL: %s", SDL_GetError());
		atexit(SDL_Quit);
	}

	memset(vga_palette, 0, sizeof(vga_palette));

	//vid_refresh_screen();
	//in_handle_events();

	/*
	ULONG modeid = INVALID_ID;

	if (!CyberGfxBase)
		CyberGfxBase = OpenLibrary("cybergraphics.library", 0);

	if (CyberGfxBase)
	{
		modeid = BestCModeIDTags(
			CYBRBIDTG_NominalWidth, vga_width,
			CYBRBIDTG_NominalHeight, vga_height,
			CYBRBIDTG_Depth, 8,
			TAG_DONE);
	}

	if (modeid == INVALID_ID)
	{
		modeid = BestModeID(
			BIDTAG_NominalWidth, vga_width,
			BIDTAG_NominalHeight, vga_height,
			BIDTAG_Depth, 8,
			//BIDTAG_MonitorID, DEFAULT_MONITOR_ID,
			TAG_DONE);
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
		if ((window = OpenWindowTags(NULL,
			WA_Flags, WFLG_BACKDROP | WFLG_REPORTMOUSE | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP,
			WA_InnerWidth, vga_width,
			WA_InnerHeight, vga_height,
			WA_CustomScreen, (IPTR)screen,
			TAG_DONE)))
		{
			pointermem = (UWORD *)AllocVec(16 * 16, MEMF_CLEAR | MEMF_CHIP);
			SetPointer(window, pointermem, 16, 16, 0, 0);
			return;
		}
	}*/
	
	screen = SDL_SetVideoMode(vga_width, vga_height, 8, SDL_SWSURFACE);
	if (screen)	return;


	printf("VID: Couldn't open screen: %s", SDL_GetError());
	VL_Shutdown();
	// painful death awaits
	Quit("It's over man");
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
	/*free(ylookup);
	ylookup = calloc(vga_height, sizeof(int));*/
	int16_t i,offset;

//
// set up lookup tables
//
	offset = 0;

	for (i=0;i<MAXSCANLINES;i++)
	{
		ylookup[i]=offset;
		offset += width;
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
	for (int i = 0; i < 256; ++i) {
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
{//printf("%s(%u,%u,%p)\n", __FUNCTION__, firstreg, numregs, palette);
	int entry;
	SDL_Color colors[256];
	/*
	for (entry = firstreg; entry < numregs; ++entry)
	{
		vga_palette[entry * 3 + 0] = *palette++;
		vga_palette[entry * 3 + 1] = *palette++;
		vga_palette[entry * 3 + 2] = *palette++;
	}
	*/
	int offset = 3 * firstreg;
	int size = 3 * numregs;
	memcpy(&vga_palette[offset], palette, size);
	for (entry = 0; entry < 256; entry++)
	{
		colors[entry].r = vga_palette[entry * 3 + 0] << 2;
		colors[entry].g = vga_palette[entry * 3 + 1] << 2;
		colors[entry].b = vga_palette[entry * 3 + 2] << 2;
	}
	/*
	palette32[entry * 3 + 1] = 0;
	LoadRGB32(&screen->ViewPort, palette32);
	*/
	SDL_SetColors(screen, colors, 0, 256);
	SDL_UpdateRect(screen, 0, 0, vga_width, vga_height);
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
	int offset = 3 * firstreg;
	int size = 3 * numregs;
	memcpy(palette, &vga_palette[offset], size);
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
	VL_Bar(x, y, width, 1, color);
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
	VL_Bar(x, y, 1, height, color);
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
	/*width *= vga_scale;
	height *= vga_scale;*/

	int offset = vl_get_offset(bufferofs, x, y);

	if (x == 0 && width == vga_width)
	{
		int count = height * vga_width;
		for (int j = 0; j < count; ++j)
		{
			vga_memory[offset + j] = color;
		}
	}
	else
	{
		for (int i = 0; i < height; ++i)
		{
			for (int j = 0; j < width; ++j)
			{
				vga_memory[offset + j] = color;
			}
			offset += vga_width;
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

	for (int p = 0; p < 4; ++p)
	{
		for (int h = 0; h < height; ++h)
		{
			for (int w = p; w < width; w += 4)
			{
				uint8_t pixel = *source++;
				int offset = base_offset + (h * width) + w;
				vga_memory[offset] = pixel;
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

void VL_MemToScreen (byte far *source, int16_t width, int16_t height, int16_t x, int16_t y)
{
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

void VL_MaskMemToScreen (byte far *source, int16_t width, int16_t height, int16_t x, int16_t y, byte mask)
{
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
}


//------------------------------------------------------------------------
// VL_ScreenToMem()
//------------------------------------------------------------------------
void VL_ScreenToMem(byte far *dest, int16_t width, int16_t height, int16_t x, int16_t y)
{
	// TODO: optimize this
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

void VL_LatchToScreen (uint16_t source, int16_t width, int16_t height, int16_t x, int16_t y)
{
	int src_pitch = /*vga_scale **/ 4 * width;
	int src_offset = vl_get_offset(source, 0, 0);
	int dst_offset = vl_get_offset(bufferofs, x, y);

	for (int h = 0; h < height; ++h)
	{
		for (int j = 0; j < src_pitch; ++j)
		{
			vga_memory[dst_offset + j] = vga_memory[src_offset + j];
		}

		src_offset += src_pitch;
		dst_offset += vga_width;
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

void VL_ScreenToScreen (uint16_t source, uint16_t dest,int16_t width, int16_t height)
{
	/*
	source *= 4 * vga_scale * vga_scale;
	dest *= 4 * vga_scale * vga_scale;
	width *= 4 * vga_scale;
	*/
	//height *= vga_scale;

	uint8_t *src_pixels = &vga_memory[source];
	uint8_t *dst_pixels = &vga_memory[dest];

	for (int h = 0; h < height; ++h)
	{
		for (int j = 0; j < width; ++j)
		{
			dst_pixels[j] = src_pixels[j];
		}

		src_pixels += vga_width;
		dst_pixels += vga_width;
	}
}

void JM_VGALinearFill(int32_t start,int32_t length, int8_t fill)
{
	memset(&vga_memory[vl_get_offset(start, 0, 0)], fill, 4 * length);
}

void VL_RefreshScreen(/*int x, int y, int width, int height*/void)
{
	uint8_t *src;
	uint8_t *dst;
//printf("%s(%d,%d,%d,%d)\n", __FUNCTION__, x, y, width, height);
	if ( SDL_MUSTLOCK(screen) ) {
		if ( SDL_LockSurface(screen) < 0 )
			return;
	}
int x = 0, y = 0, width = vga_width, height = vga_height;
	src = &vga_memory[vl_get_offset(displayofs, x, y)];
	dst = screen->pixels + y*screen->pitch + x;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			*dst++ = *src++;
		}
		src += (vga_width - width);
		dst += (screen->pitch - width);
	}

	if ( SDL_MUSTLOCK(screen) ) {
		SDL_UnlockSurface(screen);
	}
	SDL_UpdateRect(screen, 0, 0, vga_width, vga_height);
}

void VH_UpdateScreen(void)
{
	if (displayofs != bufferofs)
	{
		int src_offset = vl_get_offset(bufferofs, 0, 0);
		int dst_offset = vl_get_offset(displayofs, 0, 0);

		memcpy(&vga_memory[dst_offset], &vga_memory[src_offset], vga_width * vga_height);
	}

	VL_RefreshScreen(/*0, 0, vga_width, vga_height*/);
}

void BE_ST_DebugText(int x, int y, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	/*
	vprintf(fmt, ap);
	printf("\n");
	*/
	va_end(ap);
}

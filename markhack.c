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


//
// Former MARKHACK.ASM
//


#include "3d_def.h"


extern int16_t viewwidth;
extern int16_t viewheight;
extern uint16_t bufferofs;
extern uint16_t centery;
extern uint16_t postheight;
extern const uint8_t* shadingtable;


typedef enum DrawMode {
    DRAW_DEFAULT,
    DRAW_LIGHTED
} DrawMode;


/*static void R_CopyColumns(uint8_t *destBuffer)
{
	if (postwidth > 1)
	{
		uint8_t *destPtr = destBuffer;
		for (int j = 0; j < height; j++)
		{
			for (int i = 1; i < postwidth; i++)
			{
				destPtr[i] = destPtr[0];
			}
			destPtr += vga_width;
		}
	}
}*/

static void generic_draw_post(DrawMode draw_mode)
{
	int height = postheight*2;
	int toppix;
	fixed fracstep = (64<<16) / (fixed)height;
	fixed frac;

	if (height > viewheight)
	{
		frac = (height-viewheight)/2*fracstep;
		height = viewheight;
		toppix = 0;
	}
	else
	{
		toppix = (viewheight-height)/2;
		frac = 0;
	}

	uint8_t *destBuffer = &vga_memory[vl_get_offset(bufferofs, postx, toppix)];
	uint8_t *destPtr = destBuffer;
	int count = height;

	if (draw_mode == DRAW_LIGHTED)
	{
		do
		{
			*destPtr = shadingtable[postsource[frac>>16]];
			destPtr += vga_width;
			frac += fracstep;
		} while (--count);
	}
	else
	{
		do 
		{
			*destPtr = postsource[frac>>16];
			destPtr += vga_width;
			frac += fracstep;
		} while (--count);
	}

	//R_CopyColumns(destBuffer);
	if (postwidth > 1)
	{
		destPtr = destBuffer;
		for (int j = 0; j < height; j++)
		{
			for (int i = 1; i < postwidth; i++)
			{
				destPtr[i] = destPtr[0];
			}
			destPtr += vga_width;
		}
	}
}


//
// Draws an unmasked post centered in the viewport
//

void DrawPost()
{
	generic_draw_post(DRAW_DEFAULT);
}


//
// Draws an unmasked light sourced post centered in the viewport
//

void DrawLSPost()
{
	generic_draw_post(DRAW_LIGHTED);
}

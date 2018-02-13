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
// Former SCALE.ASM
//


#include "3d_def.h"


extern longword dc_iscale;
extern longword dc_frac;
extern uint16_t dc_source;
extern uint8_t *dc_seg;
extern int dc_length;
extern int dc_dest;
/*extern int dc_x;
extern int dc_y;
extern int dc_dy;*/
extern int dc_width;

extern const uint8_t *shadingtable;

static void R_CopyColumns(void)
{
	if (dc_width > 1)
	{
		uint8_t *destPtr = &vga_memory[dc_dest];
		for (int j = 0; j < dc_length; j++)
		{
			for (int i = 1; i < dc_width; i++)
			{
				destPtr[i] = destPtr[0];
			}
			destPtr += vga_width;
		}
	}
}

void R_DrawColumn(void)
{
	fixed frac = dc_frac;

	uint8_t *source = dc_seg + dc_source;
	uint8_t *destPtr = &vga_memory[dc_dest];

	for (int i = 0; i < dc_length; ++i)
	{
		*destPtr = source[frac >> 16];
		destPtr += vga_width;
		frac += dc_iscale;
	}
	R_CopyColumns();
}

void R_DrawSLSColumn(void)
{
	uint8_t *source = &vga_memory[dc_dest & ~(int)3];
	uint8_t *destPtr = &vga_memory[dc_dest];

	for (int i = 0; i < dc_length; ++i)
	{
		//*destPtr = shadingtable[0x1000 | *destPtr];
		//*destPtr = shadingtable[0x1000 | *source];
		*destPtr = shadingtable[*source];
		source += vga_width;
		destPtr += vga_width;
	}
}

void R_DrawLSColumn(void)
{
	fixed frac = dc_frac;

	uint8_t *source = dc_seg + dc_source;
	uint8_t *destPtr = &vga_memory[dc_dest];

	for (int i = 0; i < dc_length; ++i)
	{
		*destPtr = shadingtable[source[frac >> 16]];
		destPtr += vga_width;
		frac += dc_iscale;
	}
	R_CopyColumns();
}

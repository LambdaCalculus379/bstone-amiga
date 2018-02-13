/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)
Copyright (c) 2017 Szilard Biro

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
// Former D3_DASM2.ASM.
//


#include <stdint.h>

extern int mr_rowofs;
extern int16_t mr_count;
extern int16_t mr_xstep;
extern int16_t mr_ystep;
extern int16_t mr_xfrac;
extern int16_t mr_yfrac;
extern int mr_dest;

extern const uint8_t* shadingtable;
//extern uint8_t* vga_memory;
extern uint8_t vga_memory[256 * 1024];


uint8_t planepics[8192]; // 4k of ceiling, 4k of floor


void MapLSRow()
{
	int xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	int xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	uint8_t *destPtrCeil = &vga_memory[mr_dest];
	uint8_t *destPtrFloor = &vga_memory[mr_dest + mr_rowofs];

	for (int i = 0; i < mr_count; ++i)
	{
		uint8_t *srcPtr = &planepics[(((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E)) & 0xFFFF];
		*destPtrCeil++ = shadingtable[*srcPtr++];
		*destPtrFloor++ = shadingtable[*srcPtr];
		xy_frac += xy_step;
	}
}

void F_MapLSRow()
{
	int xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	int xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	uint8_t *destPtrFloor = &vga_memory[mr_dest + mr_rowofs];

	for (int i = 0; i < mr_count; ++i)
	{
		uint8_t *srcPtr = &planepics[(((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E)) & 0xFFFF];
		*destPtrFloor++ = shadingtable[*srcPtr];
		xy_frac += xy_step;
	}
}

void C_MapLSRow()
{
    //generic_map_row(DO_CEILING, SO_DEFAULT);
	int xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	int xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	uint8_t *destPtrCeil = &vga_memory[mr_dest];

	for (int i = 0; i < mr_count; ++i)
	{
		uint8_t *srcPtr = &planepics[(((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E)) & 0xFFFF];
		*destPtrCeil++ = shadingtable[*srcPtr];
		xy_frac += xy_step;
	}
}

void MapRow()
{
    //generic_map_row(DO_CEILING_AND_FLOORING, SO_NONE);
	int xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	int xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	uint8_t *destPtrCeil = &vga_memory[mr_dest];
	uint8_t *destPtrFloor = &vga_memory[mr_dest + mr_rowofs];

	for (int i = 0; i < mr_count; ++i)
	{
		uint8_t *srcPtr = &planepics[(((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E)) & 0xFFFF];
		*destPtrCeil++ = *srcPtr++;
		*destPtrFloor++ = *srcPtr;
		xy_frac += xy_step;
	}
}

void F_MapRow()
{
    //generic_map_row(DO_FLOORING, SO_NONE);
	int xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	int xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	uint8_t *destPtrFloor = &vga_memory[mr_dest + mr_rowofs];

	for (int i = 0; i < mr_count; ++i)
	{
		uint8_t *srcPtr = &planepics[(((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E)) & 0xFFFF];
		*destPtrFloor++ = *srcPtr;
		xy_frac += xy_step;
	}
}

void C_MapRow()
{
    //generic_map_row(DO_CEILING, SO_NONE);
	int xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	int xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	uint8_t *destPtrCeil = &vga_memory[mr_dest];

	for (int i = 0; i < mr_count; ++i)
	{
		uint8_t *srcPtr = &planepics[(((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E)) & 0xFFFF];
		*destPtrCeil++ = *srcPtr;
		xy_frac += xy_step;
	}
}

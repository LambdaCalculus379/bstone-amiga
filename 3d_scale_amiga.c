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


#include "3d_def.h"


typedef enum ShapeDrawMode {
	e_sdm_simple,
	e_sdm_shaded
} ShapeDrawMode;


extern const uint8_t *shadingtable;
extern const uint8_t *lightsource;

void R_DrawSLSColumn();
void R_DrawLSColumn();
void R_DrawColumn();


#ifdef GAMEVER_RESTORATION_AOG
#define CLOAKED_SHAPES (0)
#else
#define CLOAKED_SHAPES (1)
#endif

/*
=============================================================================

 GLOBALS

=============================================================================
*/

int16_t maxscale;
int16_t maxscaleshl2;
uint16_t centery;

int16_t normalshade;
#ifdef GAMEVER_RESTORATION_AOG
int16_t normalshade_div = SHADE_DIV;
int16_t shade_max = SHADE_MAX;
#else
int16_t normalshade_div = 1;
int16_t shade_max = 1;
#endif

int16_t nsd_table[] = { 1, 6, 3, 4, 1, 2 };
int16_t sm_table[] = { 36, 51, 62, 63, 18, 52 };
uint16_t *linecmds;


void SetupScaling(int16_t maxscaleheight)
{
	maxscaleheight /= 2; // one scaler every two pixels

	maxscale = maxscaleheight - 1;
	maxscaleshl2 = maxscale * 4;
	normalshade = (3 * maxscale) / (4 * normalshade_div);
	centery = viewheight / 2;
}


// Draw Column vars

int32_t dc_iscale;
int32_t dc_frac;
uint16_t dc_source;
uint8_t *dc_seg;
int dc_length;
int dc_dest;
int dc_x; // TODO remove? would break wide scales
int dc_y; // TODO remove, refactor MegaSimpleScaleShape
//int dc_dy;
int dc_width;


#define SFRACUNIT (0x10000)

extern boolean useBounceOffset;

fixed bounceOffset = 0;

void generic_scale_masked_post(int height, ShapeDrawMode draw_mode)
{
#ifndef GAMEVER_RESTORATION_AOG
	fixed bounce;

	if (useBounceOffset)
		bounce = bounceOffset;
	else
		bounce = 0;
#endif

	const uint16_t *srcpost = linecmds;
	dc_iscale = (64 * 65536) / height;
	longword screenstep = height << 10;

#ifdef GAMEVER_RESTORATION_AOG
	int32_t sprtopoffset = (viewheight << 15) - (height << 15);
#else
	int32_t sprtopoffset = (viewheight << 15) - (height << 15) + (bounce >> 1);
#endif

	uint16_t end = (*srcpost) / 2;
	srcpost++;

	int buf = vl_get_offset(bufferofs, dc_x, dc_y);

	while (end != 0)
	{
		dc_source = *srcpost;
		srcpost++;

		uint16_t start = *srcpost / 2;
		srcpost++;

		dc_source += start;

		int16_t length = end - start;
		int32_t topscreen = sprtopoffset + (screenstep * start);
		int32_t bottomscreen = topscreen + (screenstep * length);

		int32_t dc_yl = (topscreen + SFRACUNIT - 1) >> 16;
		int32_t dc_yh = (bottomscreen - 1) >> 16;

		if (dc_yh >= viewheight)
			dc_yh = viewheight - 1;

		if (dc_yl < 0)
		{
			dc_frac = dc_iscale * (-dc_yl);
			dc_yl = 0;
		}
		else 
		{
			dc_frac = 0;
		}

		if (dc_yl <= dc_yh)
		{
			//dc_dy = dc_yl;
			//dc_dest = vl_get_offset(bufferofs, dc_x, dc_y + dc_yl);
			//dc_dest = buf + ylookup[dc_yl];
			dc_dest = buf + vga_width*dc_yl;
			dc_length = dc_yh - dc_yl + 1;
			if (draw_mode == e_sdm_shaded)
			{
#if CLOAKED_SHAPES
				if (cloaked_shape)
					R_DrawSLSColumn();
				else
#endif
					R_DrawLSColumn();
			}
			else 
			{
				R_DrawColumn();
			}
		}

		end = *srcpost / 2;
		srcpost++;
	}
}

void generic_scale_shape(int16_t xcenter, int16_t shapenum, uint16_t height, int8_t lighting, ShapeDrawMode draw_mode)
{
	if ((height / 2) > maxscaleshl2 || ((height / 2) == 0))
		return;

	t_compshape *shape = (t_compshape *)PM_GetSpritePage(shapenum);

	dc_seg = (uint8_t *)(shape);

	int32_t xscale = (int32_t)height << 12;

	int32_t xcent = ((int32_t)xcenter << 20) - ((int32_t)height << 17) + 0x80000;

	//
	// calculate edges of the shape
	//
	//int x1 = (int)((xcent + (shape->leftpix * xscale)) >> 20);
	int16_t x1 = (int16_t)((int32_t)(xcent+((int32_t)shape->leftpix*xscale))>>20);

	if (x1 >= viewwidth)
		return; // off the right side

	//int x2 = (int)((xcent + (shape->rightpix * xscale)) >> 20);
	int16_t x2 = (int16_t)((int32_t)(xcent+((int32_t)shape->rightpix*xscale))>>20);

	if (x2 < 0)
		return; // off the left side

	longword screenscale = (256 << 20) / height;

	//
	// store information in a vissprite
	//
	longword frac;

	if (x1 < 0)
	{
		frac = (-x1) * screenscale;
		x1 = 0;
	}
	else
	{
		frac = screenscale / 2;
	}

	if (x2 >= viewwidth)
		x2 = viewwidth - 1;
//cloaked_shape = 1; // hack
	if (draw_mode == e_sdm_shaded)
	{
		int16_t i = shade_max - (63 * height / (normalshade * 8)) + lighting;

		if (i < 0)
			i = 0;
		else if (i > 63)
			i = 63;

#if CLOAKED_SHAPES
		if (cloaked_shape)
		{
			// this roughly matches what scale.asm does
			i += 10;
			if (i > 62)
				i = 62;
		}
#endif

		shadingtable = &lightsource[i * 256];
	}

	dc_y = 0;
	uint16_t swidth = shape->rightpix - shape->leftpix;

#if CLOAKED_SHAPES
	if (height>256 && !cloaked_shape)
#else
	if (height>256)
#endif
	{
		int32_t lastcolumn = -1;

		dc_width = 1;
		dc_x = 0;

		for (; x1 <= x2; ++x1, frac += screenscale)
		{
			if (wallheight[x1] > height)
			{
				if (lastcolumn >= 0)
				{
					linecmds = (uint16_t *)(&dc_seg[shape->dataofs[lastcolumn]]);
					generic_scale_masked_post(height/4, draw_mode);
					dc_width = 1;
					lastcolumn = -1;
				}
				continue;
			}

			int32_t texturecolumn = frac >> 20;

			if (texturecolumn > swidth)
				texturecolumn = swidth;

			if (texturecolumn == lastcolumn)
			{
				dc_width++;
				continue;
			}
			else
			{
				if (lastcolumn >= 0)
				{
					linecmds = (uint16_t *)(&dc_seg[shape->dataofs[lastcolumn]]);
					generic_scale_masked_post(height/4, draw_mode);
					dc_width = 1;
					dc_x = x1;
					lastcolumn = texturecolumn;
				}
				else
				{
					dc_x = x1;
					dc_width = 1;
					lastcolumn = texturecolumn;
				}
			}
		}
		if (lastcolumn != -1)
		{
			linecmds = (uint16_t *)(&dc_seg[shape->dataofs[lastcolumn]]);
			generic_scale_masked_post(height/4, draw_mode);
		}
	}
	else
	{
		dc_width = 1;

		for (; x1 <= x2; ++x1, frac += screenscale)
		{
			if (wallheight[x1] > height)
				continue;

			dc_x = x1;

			int32_t texturecolumn = frac >> 20;

			if (texturecolumn > swidth)
				texturecolumn = swidth;

			linecmds = (uint16_t *)(&dc_seg[shape->dataofs[texturecolumn]]);

			generic_scale_masked_post(height / 4, draw_mode);
		}
	}
}

/*
=======================
=
= ScaleLSShape with Light sourcing
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
=       end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
=       top of virtual line with segment in proper place
=       start of segment pixel*2, used to jsl into compiled scaler
=       <repeat>
=
= Setup for call
= --------------
= GC_MODE read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX pointing at GC_BITMASK
=
=======================
*/
void ScaleLSShape(int16_t xcenter, int16_t shapenum, uint16_t height, int8_t lighting)
{
	generic_scale_shape(xcenter, shapenum, height, lighting, e_sdm_shaded);
}

/*
=======================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
=       end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
=       top of virtual line with segment in proper place
=       start of segment pixel*2, used to jsl into compiled scaler
=       <repeat>
=
= Setup for call
= --------------
= GC_MODE read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX pointing at GC_BITMASK
=
=======================
*/
void ScaleShape(int16_t xcenter, int16_t shapenum, uint16_t height)
{
	generic_scale_shape(xcenter, shapenum, height, 0, e_sdm_simple);
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
=       end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
=       top of virtual line with segment in proper place
=       start of segment pixel*2, used to jsl into compiled scaler
=       <repeat>
=
= Setup for call
= --------------
= GC_MODE read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX pointing at GC_BITMASK
=
=======================
*/
void SimpleScaleShape(int16_t xcenter, int16_t shapenum, uint16_t height)
{
	dc_y = 0;

	t_compshape *shape = (t_compshape *)PM_GetSpritePage(shapenum);

	dc_seg = (uint8_t *)(shape);

	int32_t xscale = (int32_t)(height) << 10;

	int32_t xcent = ((int32_t)(xcenter) << 16) - ((int32_t)(height) << 15) + 0x8000;


	//
	// calculate edges of the shape
	//
	//int x1 = (int)((xcent + (shape->leftpix * xscale)) >> 16);
	int16_t x1 = (int16_t)((int32_t)(xcent+((int32_t)shape->leftpix*xscale))>>16);

	if (x1 >= viewwidth)
		return; // off the right side

	//int x2 = (int)((xcent + (shape->rightpix * xscale)) >> 16);
	int16_t x2 = (int16_t)((int32_t)(xcent+((int32_t)shape->rightpix*xscale))>>16);

	if (x2 < 0)
		return; // off the left side

	longword screenscale = (64 * 65536) / height;

	//
	// store information in a vissprite
	//
	longword frac;

	if (x1 < 0)
	{
		frac = screenscale * (-x1);
		x1 = 0;
	}
	else
	{
		frac = screenscale / 2;
	}

	if (x2 >= viewwidth)
		x2 = viewwidth - 1;

	if (height>64)
	{
		int32_t lastcolumn = -1;

		dc_x = 0;
		dc_width = 1;

		for (; x1 <= x2; ++x1, frac += screenscale)
		{
			int32_t texturecolumn = frac >> 16;

			if (texturecolumn == lastcolumn)
			{
				dc_width++;
				continue;
			}
			else
			{
				if (lastcolumn >= 0)
				{
					linecmds = (uint16_t *)(&dc_seg[shape->dataofs[texturecolumn]]);

					generic_scale_masked_post(height, e_sdm_simple);

					dc_width = 1;
					dc_x = x1;
					lastcolumn = texturecolumn;
				}
				else
				{
					dc_x = x1;
					lastcolumn = texturecolumn;
				}
			}
		}
		if (lastcolumn != -1)
		{
			linecmds = (uint16_t *)(&dc_seg[shape->dataofs[lastcolumn]]);
			generic_scale_masked_post(height, e_sdm_simple);
		}
	}
	else
	{
		uint16_t  swidth = shape->rightpix - shape->leftpix;
		dc_width = 1;

		for (; x1 <= x2; ++x1, frac += screenscale)
		{
			dc_x = x1;

			int32_t texturecolumn = frac >> 16;

			if (texturecolumn > swidth)
				texturecolumn = swidth;

			linecmds = (uint16_t *)(&dc_seg[shape->dataofs[texturecolumn]]);

			generic_scale_masked_post(height, e_sdm_simple);
		}
	}
}

// -------------------------------------------------------------------------
// MegaSimpleScaleShape()
//
// NOTE: Parameter SHADE determines which Shade palette to use on the shape.
//       0 == NO Shading
//       63 == Max Shade (BLACK or near)
// -------------------------------------------------------------------------
#ifdef GAMEVER_RESTORATION_AOG
void MegaSimpleScaleShape (int16_t xcenter, int16_t ycenter, int16_t shapenum, uint16_t height)
#else
void MegaSimpleScaleShape (int16_t xcenter, int16_t ycenter, int16_t shapenum, uint16_t height, uint16_t shade)
#endif
{

	dc_y = 0;
	dc_y -= (viewheight - 64) / 2;
	dc_y += ycenter - 34;

	t_compshape *shape = (t_compshape *)PM_GetSpritePage(shapenum);

	dc_seg = (uint8_t *)(shape);

	int32_t xscale = (int32_t)(height) << 14;

	int32_t xcent = ((int32_t)(xcenter) << 20) - ((int32_t)(height) << 19) + 0x80000;

	//
	// calculate edges of the shape
	//
	//int x1 = (int)((xcent + (shape->leftpix * xscale)) >> 20);
	int16_t x1 = (int16_t)((int32_t)(xcent+((int32_t)shape->leftpix*xscale))>>20);

	if (x1 >= viewwidth)
		return; // off the right side

	//int x2 = (int)((xcent + (shape->rightpix * xscale)) >> 20);
	int16_t x2 = (int16_t)((int32_t)(xcent+((int32_t)shape->rightpix*xscale))>>20);

	if (x2 < 0)
		return; // off the left side

	longword screenscale = (64 << 20) / height;

#ifndef GAMEVER_RESTORATION_AOG
	//
	// Choose shade table.
	//
	shadingtable = &lightsource[shade * 256];
#endif

	//
	// store information in a vissprite
	//
	longword frac;

	if (x1 < 0)
	{
		frac = screenscale * (-x1);
		x1 = 0;
	}
	else
	{
		frac = screenscale / 2;
	}

	if (x2 >= viewwidth)
		x2 = viewwidth - 1;

	/*
	uint16_t old_bufferofs;
	old_bufferofs = bufferofs;
	ycenter -= 34;
	bufferofs -= ((viewheight-64)>>1)*SCREENBWIDE;
	bufferofs += SCREENBWIDE*ycenter;
	*/

	if (height>64)
	{
		int32_t lastcolumn = -1;

		dc_x = 0;
		dc_width = 1;

		for (; x1 <= x2; ++x1, frac += screenscale)
		{
			int32_t texturecolumn = frac >> 20;

			if (texturecolumn == lastcolumn)
			{
				dc_width++;
				continue;
			}
			else
			{
				if (lastcolumn >= 0)
				{
					linecmds = (uint16_t *)(&dc_seg[shape->dataofs[texturecolumn]]);

#ifdef GAMEVER_RESTORATION_AOG
					generic_scale_masked_post(height, e_sdm_simple);
#else
					generic_scale_masked_post(height, e_sdm_shaded);
#endif
					dc_width = 1;
					dc_x = x1;
					lastcolumn = texturecolumn;
				}
				else
				{
					dc_x = x1;
					lastcolumn = texturecolumn;
				}
			}
		}
		if (lastcolumn != -1)
		{
			linecmds = (uint16_t *)(&dc_seg[shape->dataofs[lastcolumn]]);
#ifdef GAMEVER_RESTORATION_AOG
			generic_scale_masked_post(height, e_sdm_simple);
#else
			generic_scale_masked_post(height, e_sdm_shaded);
#endif
		}
	}
	else
	{
		uint16_t swidth = shape->rightpix - shape->leftpix;
		dc_width = 1;

		for (; x1 <= x2; ++x1, frac += screenscale)
		{
			dc_x = x1;

			int32_t texturecolumn = frac >> 20;

			if (texturecolumn > swidth)
				texturecolumn = swidth;

			linecmds = (uint16_t *)(&dc_seg[shape->dataofs[texturecolumn]]);

#ifdef GAMEVER_RESTORATION_AOG
			generic_scale_masked_post(height, e_sdm_simple);
#else
			generic_scale_masked_post(height, e_sdm_shaded);
#endif
		}
	}
	//bufferofs = old_bufferofs;
}

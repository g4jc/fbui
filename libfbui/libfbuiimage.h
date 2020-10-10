
/*=========================================================================
 *
 * libfbuiimage, imaging routines for libfbui.
 * Copyright (C) 2004-2005 Zachary Smith, fbui@comcast.net
 *
 * This module is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this module; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * (See the file COPYING in the main directory of this archive for
 * more details.)
 *
 *=======================================================================*/


#ifndef LIBFBUI_IMAGING
#define LIBFBUI_IMAGING

#include <libfbui.h>

enum {
	IMAGE_RGB=0,
	IMAGE_GREYSCALE=1,
	IMAGE_RGB_TRANSPARENCY=2
};

#define IMAGE_MAXBPP 48

enum {
	IMAGE_FORCE_GREYSCALE=1,
	IMAGE_JPEGBUG=64,
};	

typedef struct 
{
	char type;
	char components;	// 1 => greyscale, 3 => RGB, 4 => transparency
	char bytes_per_pixel;
	char bpp;
	char flags;
	char _filler;

	short	width;
	short	height;

	unsigned char*	data;
} Image;

extern Image *Image_new (char type, short w, short h, char bpp);
extern Image *Image_read_jpeg (char *path);
extern Image *Image_read_tiff (char *path);
extern Image *Image_resize (Image *im, short w, short h);
extern Image *Image_rotate (Image *im, int angle);
extern void Image_draw (Image *, Display *, Window *, short x, short y);
extern void Image_draw_partial (Image *, Display *, Window *, short x, short y,
		short,short,short,short);
extern void Image_delete (Image*);
extern void Image_fill_rect (Image *im, short x0, short y0, short x1, short y1, unsigned long );
extern int Image_draw_string (Image *im, Font *, short x, short y, char *, unsigned long );
extern void Image_copy_area (Image *im, short xsrc,short ysrc,short w, short h,
	short xdest, short ydest);
#endif

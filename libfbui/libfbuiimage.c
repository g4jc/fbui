
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

/* Changes:
 *
 * 27 Sep 2005, fbui@comcast.net: fixed byte order for TIFFs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/fb.h>
#include <libfbui.h>
#include <libfbuiimage.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <tiffio.h>

typedef struct
{
        int dx, dy;
        int e;
        int j;
        int sum;
}
BresenhamInfo;



/* Globals for JPEG & TIFF readers */
static Image *jpegImage = NULL;
static unsigned char * image_buffer = NULL;
static int image_height = 0;
static int image_width = 0;
static int image_ncomponents = 0;



Image *Image_new (char type, short w, short h, char bpp)
{
	Image *nu;

	if (w <= 0 || h <= 0 || bpp <= 0 || bpp > IMAGE_MAXBPP)
		return NULL;
	if (type != IMAGE_GREYSCALE && type != IMAGE_RGB && 
	    type != IMAGE_RGB_TRANSPARENCY)
		return NULL;
	/*----------*/

	nu = (Image*) malloc (sizeof(Image));
	if (!nu)
		return NULL;

	memset (nu, 0, sizeof(Image));
	nu->type = type;
	switch (type) {
	case IMAGE_RGB_TRANSPARENCY:	
		nu->components = 4; 
		break;
	case IMAGE_GREYSCALE:	
		nu->components = 1; 
		break;
	default:
	case IMAGE_RGB:	
		nu->components = 3; 
		break;
	}
	nu->width = w;
	nu->height = h;
	nu->bpp = bpp;
	nu->bytes_per_pixel = (7+bpp) >> 3;
	return nu;
}


char read_JPEG_file (char * filename);

Image *Image_read_jpeg (char *path)
{
	if (!path)
		return NULL;
	/*----------*/

	jpegImage = NULL;
	image_buffer = NULL;
	image_height = 0;
	image_width = 0;
	image_ncomponents = 0;

	if (!read_JPEG_file (path)) {
		if (jpegImage) {
			Image_delete (jpegImage);
			jpegImage = NULL;
		}
		if (image_buffer) {
			free (image_buffer);
			image_buffer = NULL;
		}
		image_height = 0;
		image_width = 0;
		image_ncomponents = 0;
		return NULL;
	}

	Image *result = jpegImage;
	jpegImage = NULL;
	image_height = 0;
	image_width = 0;
	image_ncomponents = 0;

	if (result)
		result->flags |= IMAGE_JPEGBUG; // assume the JPEG double-free bug is there

	return result;
}


void Image_delete (Image *im)
{
	if (!im) return;
	/*----------*/

	if (im->data && !(im->flags & IMAGE_JPEGBUG))
		free (im->data);
	free (im);
}


void
Image_draw_partial (Image *im, Display *dpy, Window *win, short x, short y,
	short x0, short y0, short x1, short y1)
{
	short x2, y2;

	if (!im || !dpy || !win)
		return;
	if (!im->data)
		return;
	x2 = im->width + x - 1;
	y2 = im->height + y - 1;
	if (x2<0 || y2<0)
		return;
	if (x >= dpy->width || y >= dpy->height) 
		return;
	if (im->type != IMAGE_GREYSCALE && im->type != IMAGE_RGB && 
	    im->type != IMAGE_RGB_TRANSPARENCY)
		return;
	if (im->type == IMAGE_GREYSCALE && (im->components != 1 || im->bpp != 8))
		return;
	if (im->type == IMAGE_RGB && (im->components != 3 || im->bpp != 24))
		return;
	if (im->type == IMAGE_RGB_TRANSPARENCY && (im->components != 4 || im->bytes_per_pixel != 4))
		return;
	/*----------*/

	short w = im->width;
	short h = im->height;
	char bytes = im->bytes_per_pixel;

	unsigned char *p = im->data;

	switch (im->type) {
	case IMAGE_GREYSCALE:
		fbui_put_image_partial (dpy, win, FB_IMAGETYPE_GREY, x, y, w, h,
			x0,y0,x1,y1, p);
		break;

	case IMAGE_RGB:
		if (bytes == 3) {
			fbui_put_image_partial (dpy, win, FB_IMAGETYPE_RGB3, 
				x, y, w, h,
				x0,y0,x1,y1, p);
		} else
		if (bytes == 4) {
			fbui_put_image_partial (dpy, win, FB_IMAGETYPE_RGB4, 
				x, y, w, h,
				x0,y0,x1,y1, p);
		}
		break;

	case IMAGE_RGB_TRANSPARENCY:
		fbui_put_image_partial (dpy, win, FB_IMAGETYPE_RGBA, x, y, w, h,
			x0,y0,x1,y1, p);
		break;
	}

	fbui_flush(dpy,win);
}

void
Image_draw (Image *im, Display *dpy, Window *win, short x, short y)
{
	// XX use fbui_put_image
	Image_draw_partial (im, dpy, win, x, y, 0, 0, im->width-1, im->height-1);
}

Image *Image_read_tiff (char *path)
{
	if (!path)
		return NULL;
	/*----------*/

	jpegImage = NULL;
	image_buffer = NULL;
	image_height = 0;
	image_width = 0;
	image_ncomponents = 0;

	TIFF *tiff = TIFFOpen (path, "r");
	if (!tiff)
		return NULL;

	unsigned long w, h;
	unsigned short d, samples;
	char ok = 1;

	if (!TIFFGetField (tiff, TIFFTAG_IMAGEWIDTH, &w)) 
		ok = 0;
	if (!TIFFGetField (tiff, TIFFTAG_IMAGELENGTH, &h)) 
		ok = 0;
	if (!TIFFGetField (tiff, TIFFTAG_BITSPERSAMPLE, &d)) 
		ok = 0;
	if (TIFFGetField (tiff, TIFFTAG_SAMPLESPERPIXEL, &samples))
		d *= samples;

	if (!ok) {
		TIFFClose (tiff);
		fprintf (stderr, "TIFF %s: missing dimensions or depth\n", path);
		return NULL;
	}

	unsigned char * buf = (unsigned char*) malloc (w * (1+h) * 4 + /*padding*/4);
	if (!buf) {
		TIFFClose (tiff);
		FATAL ("out of memory");
	}

	if (!TIFFReadRGBAImage (tiff, w, h, (unsigned long*) buf, 1)) {
		TIFFClose (tiff);
		WARNING ("unable to open TIFF file");
		free (buf);
		return NULL;
	}

	Image *result = NULL;

	// Note! The reader always produces a 32-bit per pixel image,
	// even if the input is greyscale.

	result = Image_new (IMAGE_RGB, w, h, d);
	if (!result)
		FATAL ("cannot allocate Image");

	result->bytes_per_pixel = 4;
	result->data = buf;

	/* TIFFs are upside down, so now we must flip the TIFF.
	 */
	short y=0;
	short lim = h/2;
	unsigned long rasterlen = 4 * w;
	unsigned char *buf2 = (unsigned char*) malloc (rasterlen);
	while (y < lim) {
		short y2 = h - 1 - y;
		unsigned char *src, *dest;
		src = & buf [ rasterlen * y ];
		dest = & buf [ rasterlen * y2 ];

		memmove (buf2, src, rasterlen);
		memmove (src, dest, rasterlen);
		memmove (dest, buf2, rasterlen);

		// Swap red and blue since FBUI is B-G-R-A.
		int i;
		for (i=0; i<rasterlen; i+=4) {
			unsigned char tmp = src[0];
			src[0] = src[2];
			src[2] = tmp;
			src += 4;

			tmp = dest[0];
			dest[0] = dest[2];
			dest[2] = tmp;
			dest += 4;

		}
		y++;
	}

	TIFFClose (tiff);
	return result;
}


void
bresenham_init (BresenhamInfo *bi, int target_size, int original_size)
{
	if (!bi) 
		FATAL("null ptr param");
	if (!target_size || !original_size)
		FATAL("zero numeric param");
	if (target_size > original_size)
		FATAL("expansion requested");

	bi->dx = original_size;
	bi->dy = target_size;
	bi->e = (bi->dy<<1) - bi->dx;
	bi->j = 0;
	bi->sum = 0; // diag
}


int
bresenham_get (BresenhamInfo *bi)
{
	char done = 0;

	if (!bi) 
		FATAL("null ptr param");

	done = false;

	int initial_j = bi->j;

	while (bi->j <= bi->dx && !done)
	{
		if (bi->e >= 0)
		{
			done = true;
			bi->e -= (bi->dx<<1);
		}
		bi->e += (bi->dy<<1);
		++bi->j;
	}

	int count = bi->j - initial_j;
	bi->sum += count;
	if (bi->sum > bi->dx)
	{
		int *i=0;
		*i++;
	}
	return count;
}


Image *Image_rotate (Image *im, int angle)
{
	int i, j;

	if (!im)
		return NULL;
	if (angle != 90)
		return NULL;
	if (im->type != IMAGE_RGB || im->bytes_per_pixel != 3) 
		return NULL;
	/*----------*/

	unsigned long size = im->bytes_per_pixel * im->width * im->height;
	unsigned char *buf = (unsigned char*) malloc (size);
	if (!buf)
		FATAL ("cannot allocate rotate buffer");

	for (i=0; i<size; i+=3) {
		buf[i] = 0xff;	// all red for testing purposes
		buf[i+1] = 0;
		buf[i+2] = 0;
	}

	short w = im->width;
	short h = im->height;

	for (j=0; j < h; j++) {
		for (i=0; i<w; i++) {
			unsigned char r,g,b;
			unsigned long src_ix, dest_ix;

			src_ix = 3 * (w * j + i);
			short x, y;
			x = h - 1 - j;
			y = i;
			dest_ix = 3 * (h * y + x);
			r = im->data [src_ix++];
			g = im->data [src_ix++];
			b = im->data [src_ix];
			buf [dest_ix++] = r;
			buf [dest_ix++] = g;
			buf [dest_ix] = b;
		}
	}

	Image *nu = Image_new (im->type, h, w, im->bpp);
	nu->flags = im->flags;
	nu->data = buf;

	return nu;
}


Image *Image_resize (Image *im, short target_w, short target_h)
{
	int n=0, i, j, inc;
	BresenhamInfo b;
	Image *nu;

	if (!im)
		return NULL;
	if (target_w <= 0 || target_h <= 0)
		return NULL;
	if (target_w > im->width || target_h > im->height)
		return NULL;
	if (im->type == IMAGE_GREYSCALE && im->bytes_per_pixel != 1)
		return NULL;
	if (im->type == IMAGE_RGB && im->bytes_per_pixel < 3)
		return NULL;
	if (im->type == IMAGE_RGB_TRANSPARENCY && im->bytes_per_pixel != 4)
		return NULL;
	/*--------*/

	unsigned char *bres_x_ary;
	unsigned char *bres_y_ary;

	short w = im->width;
	short h = im->height;

	bres_x_ary = (unsigned char*) malloc (w + 1);
	bres_y_ary = (unsigned char*) malloc (h + 1);

	bresenham_init (&b, target_w, w);
	for (i=0; i<w; i+=inc) {
		inc = bres_x_ary[n++] = bresenham_get(&b);
	}
	bresenham_init (&b, target_h, h);
	n=0;
	for (i=0; i<h; i+=inc) {
		inc = bres_y_ary[n++] = bresenham_get(&b);
	}

	unsigned char *shrunken_image_buffer = (unsigned char*) malloc (target_w * target_h * 3);
	if (!shrunken_image_buffer)
		FATAL("out of memory");

	/* shrink */
	int yinc = 0;
	int x, y;
	for (y=0, j=0; j<h; j+=yinc, y++) 
	{
		yinc = bres_y_ary [y];

		int xinc = 0;

		for(x=0, i=0; i<w; i+=xinc, x++) 
		{
			xinc = bres_x_ary [x];

			unsigned long r,g,b;
			r=g=b=0;

			/* collect a pixel average */
			int k,l;
			for (k=0; k<xinc; k++) {
				for (l=0; l<yinc; l++) {
					unsigned char *p = NULL;

					p = im->data + im->bytes_per_pixel * (i+k + (j+l)*w);
					unsigned char pix=0;
					if (im->type == IMAGE_GREYSCALE) {
						pix = *p;
						r += pix;			
						g += pix;			
						b += pix;			
					} else 
					if (im->type == IMAGE_RGB || im->type == IMAGE_RGB_TRANSPARENCY) {
						/* 3 */
						r += *p++;
						g += *p++;
						b += *p;
					}
				}
			}
			short factor = xinc * yinc;
			r /= factor;
			g /= factor;
			b /= factor;

			unsigned long ix = 3 * (x + y * target_w);

			if (im->flags & IMAGE_FORCE_GREYSCALE) {
				short n = (r + g + b) / 3;
				r = g = b = n;
			}

			shrunken_image_buffer [ix++] = r;
			shrunken_image_buffer [ix++] = g;
			shrunken_image_buffer [ix] = b;
		} 
	}

	free (bres_x_ary);
	free (bres_y_ary);

	nu = Image_new (IMAGE_RGB, target_w, target_h, 24);
	if (!nu) {
		free (shrunken_image_buffer);
		FATAL("out of memory");
	}

	nu->data = shrunken_image_buffer;
	nu->flags = im->flags;

	return nu;
}

void Image_copy_area (Image *im, short xsrc,short ysrc,short w, short h,
	short xdest, short ydest)
{
	unsigned char *buf;
	short bytes;

	if (!im)
		return;
	bytes = im->bytes_per_pixel;
	if (xsrc < 0) {
		w += xsrc;
		xdest -= xsrc;
		xsrc=0;
	}
	if (ysrc < 0) {
		h += ysrc;
		ydest -= ysrc;
		ysrc=0;
	}
	if (xsrc+w-1 >= im->width) {
		short diff = xsrc+w - im->width;
		w -= diff;
	}
	if (ysrc+h-1 >= im->height) {
		short diff = ysrc+h - im->height;
		h -= diff;
	}
	buf = (unsigned char*) malloc (w * 4);
	if (!buf)
		FATAL("cannot allocate buffer in Image_copy_area");
	/*----------*/

	int j;
	for (j=0; j<h; j++) {
		memcpy (buf, &im->data[bytes*(im->width*(ysrc+j)+xsrc)],
			w*bytes);
		memcpy (&im->data[bytes*(im->width*(ydest+j)+xdest)],
			buf, w*bytes);
	}
}

void Image_fill_rect (Image *im, short x0, short y0, short x1, short y1, unsigned long color)
{
	int i,j;

	if (!im)
		return;
	if (x0 > x1) { short tmp = x0; x0 = x1; x1 = tmp; }
	if (y0 > y1) { short tmp = y0; y0 = y1; y1 = tmp; }
	if (x0 >= im->width || x1 < 0 || y0 >= im->height || y1 < 0)
		return;
	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 >= im->width)
		x1 = im->width-1;
	if (y1 >= im->height)
		y1 = im->height-1;
	/*----------*/

	if (!im->data) {
		unsigned long size = im->bytes_per_pixel * im->width * im->height;
		im->data = (unsigned char*) malloc (size);
		if (!im->data)
			FATAL ("cannot allocate rotate buffer");
		memset (im->data, 0, size);
	}

	for (j=y0; j <= y1; j++) {
		for (i=x0; i<=x1; i++) {
			unsigned long ix = i + j*im->width;
			unsigned long p = color;
			char bytes = im->bytes_per_pixel;

			switch (im->type) {
			case IMAGE_GREYSCALE:
				im->data[ix] = p;
				break;

			case IMAGE_RGB:
				if (bytes == 3) {
					im->data[ix*3] = p; p >>= 8;
					im->data[ix*3+1] = p; p >>= 8;
					im->data[ix*3+2] = p; p >>= 8;
				}
				else
				if (bytes == 4) {
					unsigned long *ptr = (unsigned long*) &im->data[ix<<2];
					*ptr = p;
				}
				break;

			case IMAGE_RGB_TRANSPARENCY: {
				unsigned long *ptr = (unsigned long*) &im->data[ix<<2];
				*ptr = p;
				break;
				}
			}
		}
	}
}


static unsigned char leftshift[4] = {
        24, 16, 8, 0
};
int
Image_draw_string (Image *im, Font *font,
	short x0, short y, char *str_, 
	unsigned long color)
{
	unsigned char *str = (unsigned char*) str_;
	short x= x0;
	unsigned char *bitmap, bitwidth, bitheight;
	short ytop, total_width = 0;
	short i, j;

	if (!im || !str)
		return -1;
	if (y >= im->height || x >= im->width)
		return 0;
	if (y + (font->ascent + font->descent) <= 0)
		return 0;
	/*---------------*/

	while (x < im->width)
	{
		char left, descent;
		unsigned char ch;
		unsigned char width;

		ch = *str++;
		if (!ch)
			break;

		if (ch < font->first_char || ch > font->last_char)
			continue;

		ch -= font->first_char;
		if (ch >= 128 && ch < 160) {	
			/* our PCF Fonts move [128,159] to end */
			ch &= 31;
			ch |= 240;
		} 
		else if (ch >= 160)
			ch -= 32;

		bitmap = font->bitmaps[ch];
		bitwidth = font->bitwidths[ch];
		bitheight = font->heights[ch];
		width = font->widths[ch];
		left = font->lefts[ch];
		descent = font->descents[ch];

		if (!bitmap || !bitwidth || !bitheight || !width)
			continue;

		ytop = font->ascent + descent;
		ytop -= bitheight;

		/* Draw loop */
		j = 0;

		while (j < bitheight) 
		{
			unsigned long bits;
			short current_x;
			short current_y = y + j + ytop;
			unsigned char bytes_per_row;

			bytes_per_row = (bitwidth+7)/8;
			if (current_y < 0) {
				bitmap += bytes_per_row;
				goto ds_inner_skip;
			}
			if (current_y >= im->height)
				break;

			current_x = x + left;
			bits = *(unsigned long*)bitmap;
			bitmap += bytes_per_row;

			// XX could be sped up.
			i = 0;
			while (bits && i<width) {
				if (bits & 1) {
					Image_fill_rect (im, x+i, y+ytop+j, x+i, y+ytop+j, color);
				}
				i++;
				bits >>= 1;
			}

ds_inner_skip:
			j++;
		}

		x += width;
		total_width += width;

	} /* char loop */

	return 0;
}

/*------------------------------------------------------------------------*/

/* Below is the JPEG reading code, derived heavily from the jpeglib sample code.
 * The code below is not copyrighted by me except put_scanline_someplace().
 */

static int y=0;
int put_scanline_someplace(JSAMPLE* data, int nbytes)
{
	if (!jpegImage) {
		image_buffer = (JSAMPLE*) malloc (nbytes * image_height * 3);
		if (!image_buffer) {
			printf ("failed to malloc image buffer\n");
			exit(2);
		}

		char type;
		switch (image_ncomponents){
		case 1:
			type = IMAGE_GREYSCALE;
			break;
		case 3:
			type = IMAGE_RGB;
			break;
		case 4:
			type = IMAGE_RGB_TRANSPARENCY;
			break;
		default:
			FATAL("invalid number of jpeg color components");
		}

		jpegImage = Image_new (type, image_width, image_height, image_ncomponents*8);
		if (!jpegImage)
			return 0;
		jpegImage->data = image_buffer;
	}

	unsigned char *tmp = (unsigned char*) data;
	unsigned char *tmp2 = (unsigned char*) (image_buffer + y * nbytes);
	int i;

	switch (image_ncomponents){
	case 1:
		memcpy (tmp2,tmp,nbytes);
		break;
	case 3:
		for (i = 0; i < nbytes; i+=3) {
			tmp2[0] = tmp[2];
			tmp2[1] = tmp[1]; /* jpeglib writes R-G-B-R-G-B...*/
			tmp2[2] = tmp[0];
			tmp += 3;
			tmp2 += 3;
		}
		break;
	case 4:
		for (i = 0; i < nbytes; i+=4) {
			tmp2[0] = tmp[2];
			tmp2[1] = tmp[1];
			tmp2[2] = tmp[0];
			tmp2[3] = 0;
			tmp += 4;
			tmp2 += 4;
		}
		break;
	}

	y++;
	return 1;
}

struct my_error_mgr {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

void
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

char
read_JPEG_file (char * filename)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FILE * infile;		/* source file */
	JSAMPARRAY buffer;	/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return 0;
	}

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return 0;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);

	cinfo.out_color_space = JCS_RGB;
	cinfo.scale_num = 1;
	cinfo.scale_denom = 0;
	cinfo.output_gamma = 1.0;
	cinfo.dct_method = JDCT_FLOAT;
	cinfo.do_fancy_upsampling = 1;
	cinfo.do_block_smoothing = 1;
	cinfo.buffered_image = 0;
	cinfo.raw_data_out = 0;
	cinfo.quantize_colors = 0;
	cinfo.dither_mode = JDITHER_NONE;
	cinfo.two_pass_quantize = 0;

	jpeg_start_decompress(&cinfo);

	image_height = cinfo.output_height;
	image_width = cinfo.output_width;
	row_stride = cinfo.output_width * cinfo.output_components;
	image_ncomponents = cinfo.output_components;

	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		put_scanline_someplace(buffer[0], row_stride);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	y=0;
	return 1;
}



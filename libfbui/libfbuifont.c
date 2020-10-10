
/*=========================================================================
 *
 * libfbuifont, PCF font reading and drawing routines for libfbui.
 * Copyright (C) 2003-2005 Zachary Smith, fbui@comcast.net
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


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>

#include "libfbui.h"
#include "libfbuifont.h"

int
fbui_draw_string (Display *dpy, Window *win, Font *font,
	short x0, short y, char *str_,
	unsigned long color)
{
        unsigned long n;
        unsigned long n2;
	unsigned char *str = (unsigned char*) str_;
	short x= x0;
	unsigned char *bitmap, bitwidth, bitheight;
	short ytop, total_width = 0;
	short i, j;
	unsigned short pixel_ix;

	if (!dpy || !win || !str)
		return -1;
	if (win->deleted) 
		return -1;

	if (y + (font->ascent + font->descent) <= 0)
		return 0;
	/*---------------*/

	while (x < win->width)
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
		fbui_put_image_mono (dpy, win, x, y+ytop, 
			32, bitheight, (unsigned char*) bitmap, color);

		x += width;
		total_width += width;

	} /* char loop */

	return 0;
}


Font *
Font_new (void)
{
	Font *nu;

	if (!dpi)
		FATAL ("display dpi has not been established");

	nu = (Font*) malloc(sizeof (Font));
	if (!nu)
		FATAL("out of memory")
	else
	{
		memset ((void*) nu, 0, sizeof (Font));
		nu->bitmap_buffer = NULL;
		// nu->dpi = dpi;
		nu->ascent = 0;
		nu->descent = 0;
	}
	return nu;
}

void
Font_free (Font* font)
{
	int i;
	if (!font)
		return; // error

	if (font->lefts) 
		free ((void*) font->lefts);
	if (font->descents) 
		free ((void*) font->descents);
	if (font->widths) 
		free ((void*) font->widths);
	if (font->heights) 
		free ((void*) font->heights);
	if (font->bitmaps) 
		free ((void*) font->bitmaps);

	free ((void*)font);
}


void 
Font_char_dims (Font *font, uchar ch, short *w, short *asc, short *desc)
{
	if (!font || !w || !asc || !desc)
		return; // error

	ch -= font->first_char;
	*w = font->widths[ch];
	*asc = font->ascent;
	*desc = font->descent;
}


void
Font_string_dims (Font *font, unsigned char *str, short *w, short *a, short *d)
{
	if (!font || !str || !w || !a || !d) {
		if (w)
			*w = 0;
		if (a)
			*a = 0;
		if (d)
			*d = 0;
		return; // error
	}

	short w0 = 0;
	int ch;

	while ((ch = *str++)) {
		ch -= font->first_char;
		if (ch >= 0)
			w0 += font->widths [ch];
	}

	*w = w0;
	*a = font->ascent;
	*d = font->descent;
}



static uchar bit_reversal_array [256] =
{
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0, 0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8, 0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4, 0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc, 0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2, 0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa, 0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6, 0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1, 0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9, 0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 
0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5, 0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd, 0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3, 0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb, 0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7, 0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff, 
};


#define ZZ(a,b,c,d) ((((unsigned long)d)<<24)|(((unsigned long)c)<<16)|(((unsigned short)b)<<8)|a)


inline __attribute__((always_inline)) unsigned long ULONG(char endian, unsigned char* pp) 
{
	if (endian) 
		return (((unsigned long)pp[0]) << 24) | (((unsigned long)pp[1]) << 16) | (((unsigned short)pp[2]) << 8) | pp[3];
	else
		return (((unsigned long)pp[3]) << 24) | (((unsigned long)pp[2]) << 16) | (((unsigned short)pp[1]) << 8) | pp[0];
}

inline __attribute__((always_inline)) unsigned short USHORT(char endian, unsigned char* pp)
{
	unsigned short i;

	if (endian) 
		i = (((unsigned short)pp[0]) << 8) | pp[1];
	else
		i = (((unsigned short)pp[1]) << 8) | pp[0];

	return i;
}
// TODO: Rather than inline these, we could make them defines...
//#define USHORT(endian, pp) ((endian) ? ((((unsigned long)(pp)[0]) << 24) | (((unsigned long)(pp)[1]) << 16) | (((unsigned short)(pp)[2]) << 8) | (pp)[3]) : ((((unsigned long)(pp)[3]) << 24) | (((unsigned long)(pp)[2]) << 16) | (((unsigned short)(pp)[1]) << 8) | (pp)[0]) )

// A pcf that is "compressed" has smaller metrics info
// but the bitmap data is the same.

static unsigned char *read_buffer = NULL;

char
pcf_read_encodings (Font* pcf, unsigned char* orig)
{
	unsigned char *ptr = orig;
	unsigned long format;
	char endian;

	format = ULONG(0,ptr); ptr += 4;
	endian = (format & PCF_BIG_ENDIAN) ? true : false;

	ptr += 2; // skip: first column
	ptr += 2; // skip: last column

	ptr += 2; // skip: first row
	ptr += 2; // skip: last row

	// read default char

	unsigned short defaultChar = USHORT(endian,ptr); ptr += 2;
	unsigned short numChars = USHORT(endian,ptr); ptr += 2;

#if 0
	printf ("pcf_read_encodings(): default char = %d, num chars = %d\n",
		pcf->first_char, numChars);
#endif

	/* kludge */
	pcf->first_char = 31;
	pcf->last_char = 255;

	return true;
}


char
pcf_read_bitmaps (Font* pcf, unsigned char* orig)
{
	unsigned char *ptr = orig;
	unsigned long format;
	char endian;
	char compressed;
	unsigned long nChars;
	int i, j;
	unsigned char *offsets;

	format = ULONG(0,ptr); ptr += 4;
	endian = (format & PCF_BIG_ENDIAN) ? true : false;
	compressed = (format & PCF_COMPRESSED) ? true : false;

	int storage_unit = (format >> 4) & 3;
	int row_unit = format & 3;
	int bit_order = format & 8;

	nChars = ULONG(endian,ptr); ptr += 4;
	if (pcf->nchars)
	{
		if (pcf->nchars != nChars)
			FATAL ("#metrics != #chars");
	}
	else
	{
		pcf->nchars = nChars;
	}

	offsets = ptr;
	ptr += 4 * nChars;

	unsigned char *ptr2 = ptr + 4 * (3 & format);
	unsigned long data_size = ULONG(endian,ptr2);

	ptr += 16;

	pcf->bitmap_buffer = malloc (data_size);
	if (!pcf->widths)
	{
		pcf->lefts = malloc (nChars);
		pcf->widths = malloc (nChars);
		pcf->bitwidths = malloc (nChars);
		pcf->descents = malloc (nChars);
		pcf->heights = malloc (nChars);

		if (!pcf->lefts || !pcf->heights)
			FATAL ("out of memory");
	}

	pcf->bitmaps = malloc (nChars * sizeof(char*));

	if (!pcf->bitmap_buffer || !pcf->lefts || !pcf->widths || !pcf->heights 
	    || !pcf->bitmaps)
		FATAL ("unable to allocate font data");

	// Copy over bitmap data.
	for (i=0; i < data_size; i++)
	{
		pcf->bitmap_buffer[i] = ptr[i];
	}

	// Need to ensure that the leftmost font bit is bit 0.
	//
	if (bit_order)
	{
		unsigned char *p = pcf->bitmap_buffer;
		i = data_size;
		while (i--) {
			*p++ = bit_reversal_array[*p];
		}
	}

	// Need to re-order the bytes to make them
	// big-endian, if they were written in big-endian format.
	//
	if (!endian) {
		unsigned char *p;
		switch (row_unit) {
		default:
		case 0: // row = 1 byte
			break;

		case 1: // row = 1 short
			if (storage_unit == 0) {
				unsigned char *p = pcf->bitmap_buffer;
				i = nChars / 2;
				while (i)
				{
					int tmp = *p;
					*p = *(p+1);
					*++p = tmp;
					i--;
				}
			}
			break;

		case 2: // row = 1 long
			switch (storage_unit) {
			case 0:
				p = pcf->bitmap_buffer;
				i = nChars / 4;
				while (i)
				{
					int tmp = *p;
					*p = *(p+3);
					*(p+3) = tmp;

					p++;
					tmp = *p;
					*p = *(p+1);
					*(p+1) = tmp;

					p += 3;

					i--;
				}
				break;
			case 1:
				p = pcf->bitmap_buffer;
				i = nChars / 4;
				while (i)
				{
					int tmp = *p;
					int tmp2 = *(p+1);
					*p = *(p+2);
					*(p+1) = *(p+3);
					*(p+2) = tmp;
					*(p+3) = tmp2;

					p += 4;

					i--;
				}
				break;
			default:
				break;
			}
			break;
		}
	}

	// Now generate pointers for character data.
	//
	j = 8 << row_unit;

	for (i=0; i < nChars; i++)
	{
		unsigned long offset = ULONG(endian,offsets); offsets += 4;
		pcf->bitmaps [i] = pcf->bitmap_buffer + offset;
		pcf->bitwidths [i] = j;
	}

	return true;
}


char
pcf_read_metrics (Font* pcf, unsigned char* orig)
{
	unsigned char *ptr = orig;
	unsigned long format;
	char endian;
	char compressed;
	unsigned long nMetrics;
	int i;

	format = ULONG(0,ptr); ptr += 4;
	endian = (format & PCF_BIG_ENDIAN) ? true : false;
	compressed = (format & PCF_COMPRESSED) ? true : false;

	if (compressed)
	{
		nMetrics = USHORT(endian,ptr);
		ptr += 2;
	} 
	else
	{
		nMetrics = ULONG(endian,ptr);
		ptr += 4;
	}

	if (!pcf->nchars && !pcf->widths)
	{
		pcf->lefts = malloc (nMetrics);
		pcf->widths = malloc (nMetrics);
		pcf->bitwidths = malloc (nMetrics);
		pcf->descents = malloc (nMetrics);
		pcf->heights = malloc (nMetrics);

		if (!pcf->lefts || !pcf->heights)
			FATAL ("out of memory");
	}

	if (compressed)
	{
		for (i=0; i < nMetrics; i++)
		{
			pcf->lefts[i] = ((short)*ptr++) - 0x80; 
			ptr++; // skip right-bearing
			pcf->widths[i] = ((short)*ptr++) - 0x80;
			pcf->heights [i] = ((short)*ptr++) - 0x80; // get ascent
			pcf->descents[i] = ((short)*ptr++) - 0x80;
			pcf->heights [i] += pcf->descents[i]; // height=ascent+descent
			// There is no attr byte in compressed version.

			pcf->bitwidths[i] = pcf->widths[i];

#if 0
printf ("char %c(%d): w=%d ht=%d desc=%d\n", i+32,i+32, pcf->widths[i], pcf->heights[i], pcf->descents[i]);
#endif
		}
	}
	else
	{
		for (i=0; i < nMetrics; i++)
		{
			pcf->lefts [i] = USHORT (endian,ptr); ptr += 2;
			ptr += 2;
			pcf->widths[i] = USHORT (endian,ptr); ptr += 2;
			pcf->heights [i] = USHORT (endian,ptr); ptr += 2;
			pcf->descents[i] = USHORT (endian,ptr); ptr += 2;
			ptr += 2; // skip attr word

			pcf->bitwidths[i] = pcf->widths[i];
		}
	}

	return true;
}

char
pcf_read_accelerator (Font* pcf, unsigned char* orig)
{
	unsigned char *ptr = orig;
	unsigned long format;
	char endian;
	int i;

	format = ULONG(0,ptr); ptr += 4;
	endian= (format & PCF_BIG_ENDIAN) ? 1 : 0;

	ptr += 8;

	pcf->ascent = ULONG(endian,ptr); ptr += 4;
	pcf->descent = ULONG(endian,ptr); ptr += 4;

// printf ("ascent = %u , descent = %d\n", ascent, descent);

	return true;
}



char
pcf_read_properties (Font* pcf, unsigned char* orig)
{
	unsigned char *ptr = orig;
	unsigned long format, nprops;
	char endian;
	int i;

	format = ULONG(0,ptr); ptr += 4;
	endian = (format & PCF_BIG_ENDIAN) ? 1 : 0;
	nprops = ULONG(endian,ptr); ptr += 4;

	unsigned char* str_buffer = ptr + nprops*9 + 4;

// printf ("format=%08lx\n", format);
// printf ("nprops=%08lx\n", nprops);

	i = ((unsigned long)str_buffer) & 3;
	if (i) str_buffer += (4-i);

	for (i = 0; i < nprops; i++)
	{
		unsigned long name = ULONG(endian,ptr); ptr += 4;
		unsigned char is_string = *ptr++;
		unsigned long value = ULONG(endian,ptr); ptr += 4;

		char *str = ((char*)str_buffer) + name;
		if (is_string)
		{
			char *str2 = ((char*)str_buffer) + value;

			switch (*str) {

			case 'F':

			if (!strcmp ("FAMILY_NAME", str)) 
			{
//printf ("family-name=%s\n", str2);
#if 0
				if (strlen (str2) < 63)
					strcpy (pcf->family, str2);
				else
					strncpy (pcf->family, str2, 63);
#endif
			} 
			else
			if (!strcmp ("FULL_NAME", str)) 
			{
//printf ("full-name=%s\n", str2);
#if 0
				if (strlen (str2) < 63)
					strcpy (pcf->fullname, str2);
				else
					strncpy (pcf->fullname, str2, 63);
#endif
			} 
			break;

			case 'C':

			if (!strcmp ("CHARSET_REGISTRY", str)) 
			{
// printf ("charset registry=%s\n", str2);
#if 0
				if (strcmp ("ISO8859", str2))
					return false;
#endif
			}
			break;

			case 'S':

			if (!strcmp ("SLANT", str)) 
			{
//printf ("slant=%s\n", str2);
#if 0
				char angle=*str2;
				if (angle=='R')
					pcf->italic=0;
				else
					pcf->italic=1;
#endif
			} 
			break;
			
			case 'W':

			if (!strcmp ("WEIGHT_NAME", str)) 
			{
//prf ("weight-name=%s\n", str2);
#if 0
				if (!strcmp(str2, "Bold")) {
					pcf->weight = FONT_WEIGHT_BOLD;
				}
				else
				if (!strcmp(str2, "Medium")) {
					pcf->weight = FONT_WEIGHT_MEDIUM;
				}
				else
					pcf->weight = FONT_WEIGHT_MEDIUM;
#endif
			}
			break;
			
			default:
				break;

			//printf ("\tunused property name(%ld) %s value(%ld) %s\n",
			 //	name, str, value, str2);
			}
		}
		else
		{
			switch (*str) {

			case 'P':

			if (!strcmp ("POINT_SIZE", str)) 
			{
//printf ("point size=%lu\n", value);
#if 0
				double sz = value;
				pcf->size = sz / 10.0;
#endif
			}
			break;

			case 'R':

			if (!strcmp ("RESOLUTION_X", str)) 
			{
//printf ("resolution x=%lu\n", value);
#if 0
				pcf->dpi = value;
#endif
			} 
			break;

			case 'C':

			// We're using only iso8859-1 fonts.
			//
			if (!strcmp ("CHARSET_ENCODING", str))
			{
// printf ("encoding=%d\n", value);
				if (value != 1)
					return false;
			}
			default:
				break;

			// printf ("\tunused property name %s value %lu\n", str, value);
			}
		}
	}
	return true;
}


char
pcf_read (Font* pcf, char *path)
{
	int i;
	FILE *f;
	static char *line;
	char *param;
	char *param_end;
	short param_length;
	unsigned char *us;
	char *s;
	char *s2;
	char *s3;
	int char_num = 0;
	static char got_start = false;
	static char got_encoding = false;
	static char got_end = false;
	static char got_bitmap = false;
	struct stat statbuf;
	char path2[PATH_MAX];

	if (*path != '/') {
		char *dir = getenv("PCFFONTDIR");
		if (dir) {
			strcpy (path2, dir);
			if ('/' != dir[strlen(dir)-1])
				strcat (path2, "/");
		} else
			sprintf (path2, "/usr/X11R6/lib/fonts/%d%s", dpi, "dpi/");

		strcat (path2, path);
	}
	else
		strcpy (path2, path);

//printf ("path='%s'\n", path2);
	if (stat (path2, &statbuf)) {
		strcat (path2, ".gz");
		if (stat (path2, &statbuf)) {
			return false;
		} else {
			FATAL("a font file was found to be gzipped however libfbuifont doesn't currently uncompress gzipped fonts");
		}
	}

	unsigned long size = statbuf.st_size;

#if 0
	pcf->fullname[0]=0;
	pcf->family[0]=0;
#endif

	f = fopen (path2, "rb");
	if (!f)
		return false;

        read_buffer = malloc (size);
        if (!read_buffer)
		FATAL ("unable to allocate font read buffer!");
        i = fread (read_buffer,1,size,f);
        if (i != size)
		FATAL ("cannot read entire font file");
        fclose (f);
        f = NULL;
        uchar *ptr = read_buffer;

	// Read the TOC
	unsigned long toc_type;
	unsigned long toc_total;

	toc_type = ULONG(0,ptr); ptr += 4; 
	toc_total = ULONG(0,ptr); ptr += 4;
	if (toc_type != ZZ(1, 'f','c','p'))
		FATAL ("pcf file has bad header");

	for (i=0; i < toc_total; i++)
	{
		unsigned long j;
		unsigned short type;
		unsigned char *table;

// printf ("toc entry %d:\n", i);
		
		type = ULONG(0,ptr); ptr += 4;
//		printf ("  type=%u, ", type);

		j = ULONG(0,ptr); ptr += 4;
		// printf (" format=%lu, ", j);

		j = ULONG(0,ptr); ptr += 4;
		// printf (" size=%lu, ", j);

		j = ULONG(0,ptr); ptr += 4;
   //		printf (" offset=%lu\n", j);

		table = read_buffer + j;

		switch (type)
		{
		case PCF_PROPS:
			if (!pcf_read_properties (pcf, table))
				return false;
			break;

		case PCF_BDF_ENCODINGS :
			if (!pcf_read_encodings (pcf, table))
				return false;
			break;

		case PCF_ACCEL:
			if (!pcf_read_accelerator (pcf, table))
				return false;
			break;

		case PCF_METRICS:
			if (!pcf_read_metrics (pcf, table))
				return false;
			break;

		case PCF_BITMAPS:
			if (!pcf_read_bitmaps (pcf, table))
				return false;
			break;

		default:
			;
		}
	}

	if (read_buffer)
	{
		free (read_buffer);
		read_buffer = NULL;
	}

	return true;
}



/*=========================================================================
 *
 * libfbui, a library for accessing FBUI (in-kernel framebuffer GUI).
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



#ifndef FONT_H
#define FONT_H

typedef struct pcf_font {
        unsigned char ascent;
        unsigned char descent;
        unsigned char first_char;
        unsigned char last_char;
        unsigned char nchars;
        unsigned char *lefts;
        unsigned char *heights;
        unsigned char *widths;  /* # of bits used for pixels */
        unsigned char *bitwidths; /* # bits actually used.. e.g. 32 */
        unsigned char *descents;
        unsigned char *bitmap_buffer;
        unsigned char **bitmaps;
} Font;

extern int fbui_draw_string (Display *dpy, Window *win, Font *font, short x0, short y, char *str_, unsigned long color);

extern Font * Font_new (void);
extern void Font_free (Font* font);
extern void Font_char_dims (Font *font, uchar ch, short *w, short *asc, short *desc);
extern void Font_string_dims (Font *font, unsigned char *str, short *w, short *a, short *d);

extern char pcf_read (Font* pcf, char *path);

enum {
	PCF_PROPS = 1,
	PCF_ACCEL = 2,
	PCF_METRICS = 4,
	PCF_BITMAPS = 8,
	PCF_INK_METRICS = 16,
	PCF_BDF_ENCODINGS = 32,
	PCF_SWIDTHS = 64,
	PCF_GLYPH_NAMES = 128,
	PCF_BDF_ACCEL = 256,

};

enum {
	PCF_BIG_ENDIAN = (1<<2),
	PCF_COMPRESSED = 256,	
};


#endif


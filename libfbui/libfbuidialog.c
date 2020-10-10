
/*=========================================================================
 *
 * libfbuidialog, software to create and manage a dialog box using FBUI.
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

#include "libfbui.h"
#include "libfbuifont.h"


void
fbui_draw_dialog (Display *dpy, Window *win, Font *font, char *expr)
{
	short w,a,d;
	short win_w, win_h;
	
	if (!dpy || !font || !win || !expr)
		return;
	if (win->deleted) 
		return;
	/*----------------*/

	Font_string_dims (font, expr, &w, &a, &d);

	if (fbui_get_dims (dpy, win, &win_w, &win_h))
		return;

	short x, y;
	short border = 15;
	short dialog_w = w + border*2;
	short dialog_h = a+d + border*2;
	x = (win_w - dialog_w) / 2;
	y = (win_h - dialog_h) / 2;
	short x1, y1;
	x1 = x + dialog_w - 1;
	y1 = y + dialog_h - 1;

	fbui_fill_rect (dpy, win, x, y, x1, y1, 0xC0C0C0);
	fbui_draw_hline (dpy, win, x, x1, y, 0xF0F0F0);
	fbui_draw_hline (dpy, win, x, x1, y+1, 0xF0F0F0);
	fbui_draw_hline (dpy, win, x, x1, y1, 0x909090);
	fbui_draw_hline (dpy, win, x, x1, y1-1, 0x909090);
	fbui_draw_vline (dpy, win, x, y, y1, 0xF0F0F0);
	fbui_draw_vline (dpy, win, x+1, y+1, y1, 0xE8E8E8);
	fbui_draw_vline (dpy, win, x1, y, y1, 0x989898);
	fbui_draw_vline (dpy, win, x1-1, y+1, y1-1, 0x989898);
	fbui_draw_string (dpy, win, font, x+border, y+border, expr, RGB_BLUE);
}

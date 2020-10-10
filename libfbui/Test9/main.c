
/*=========================================================================
 *
 * fbtest9, test of beep function.
 * Copyright (C) 2005 Zachary Smith, fbui@comcast.net
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


typedef unsigned long u32;
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include <linux/vt.h>
#include <linux/input.h>

#include "libfbui.h"
#include "libfbuifont.h"


int
main(int argc, char** argv)
{
	Display *dpy;
	Window *win;
	int i;
	int vc=-1;

	dpy = fbui_display_open ();
	if (!dpy) 
		FATAL ("cannot open display");

	RGB fg = RGB_WHITE;
	RGB bg = RGB_BLUE;

	short win_w = 200;
	short win_h = 200;

	short x = rand(NULL) % dpy->width;
	short y = rand(NULL) % dpy->height;

	if (x+win_w > dpy->width)
		x = dpy->width - win_w;
	if (y+win_h > dpy->height)
		y = dpy->height - win_h;

	win = fbui_window_open (dpy, win_w, win_h, 
		&win_w, &win_h, 999,999,
		(int) x, (int) y,
		&fg, &bg, 
		"fbtest5", "-",
		FBUI_PROGTYPE_APP, 
		false,false, vc,
		true, 
		false, 
		false,
		NULL,
		0,NULL);

	if (!win) {
		FATAL ("cannot open window");
	}

	fbui_draw_line (dpy, win, 0,0,100,100, RGB_YELLOW);

	int countdown=5;
	while (countdown--) {
		int total;
		int need=0;
		Event ev;
		int type;
		int err;

		err = fbui_beep (dpy, 440, 100);
		if (err) {
			fbui_print_error (err);
			break;
		}

		sleep (1);
	}

	fbui_display_close (dpy);
	return 0;
}


/*=========================================================================
 *
 * fbtest2, a multiple-window-per-application test program for FBUI
 * Copyright (C) 2004 Zachary Smith, fbui@comcast.net
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


struct mywindows {
	int inside;
	Window *win;
};

#define NUMWINS 24
static struct mywindows mywins [NUMWINS];


int
main(int argc, char** argv)
{
	Display *dpy;
	Font *pcf;
	u32 fg,bg;
	int i;
	int vc=-1;

	for (i=0; i<NUMWINS; i++)
		mywins[i].win = NULL;

	i=1;
	while(i<argc) {
		if (!strncmp("-c",argv[i],2)) {
			if (argv[i][2])
				vc = atoi (2 + argv[i]);
		}
		i++;
	}

	fg = RGB_YELLOW;

	short win_w, win_h;
	dpy = fbui_display_open ();
	if (!dpy) 
		FATAL ("cannot open display");

	pcf = Font_new ();
	if (!pcf_read (pcf, "timR12.pcf")) {
		Font_free (pcf);
		pcf = NULL;
		FATAL ("cannot load font");
	}
	short line_height = pcf->ascent + pcf->descent;

	srand (time(NULL));

	for (i=0; i< NUMWINS; i++) {
		Window *win;
		char subtitle[20];

		sprintf (subtitle, "%d", i);

		bg = 0xa0 | (i << 12);

		do {
			short win_w = 150;
			short win_h = 150;
			int x = rand () % (dpy->width - win_w);
			int y = rand () & (dpy->height - win_h);
			win = fbui_window_open (dpy, win_w, win_h, 
				&win_w, &win_h, 999,999,
				x, y,
				&fg, &bg, 
				"fbmtest", subtitle, 
				FBUI_PROGTYPE_APP, 
				false,false, vc,
				false, 
				false, 
				i == 3 ? true : false, // hide the 4th window
				NULL, /* no mask */
				0,NULL);
		} while (!win);

		mywins[i].win = win;
		mywins[i].inside = 0;
	}

	printf ("MultiTest: All windows created.\n");

	while (1) {
		int total;
		int need=0;
		Event ev;
		Window *win;
		int x=0, y=0;
		int type;
		int err;

		/* Get any event for ANY of our windows */

		if (err = fbui_wait_event (dpy, &ev, FBUI_EVENTMASK_ALL))
		{
			fbui_print_error (err);
			continue;
		}

		win = ev.win;
		if (!win) {
			printf ("window id %d\n", ev.id);
			FATAL("null window");
		}

		type = ev.type;

		i=0;
		int in = 0;
		while(i < NUMWINS) {
			if (mywins[i].win->id == win->id) {
				if (type == FBUI_EVENT_ENTER || type == FBUI_EVENT_LEAVE)
					in = mywins[i].inside = type==FBUI_EVENT_ENTER ? 1 : 0;
				else
					in = mywins[i].inside;
				break;
			}
			i++;
		}

		fbui_clear (dpy, win);

		char expr[100];
		sprintf (expr, "This is window %d", win->id);

		if (in)
			fbui_draw_rect (dpy, win, 0, 0, 149,149, RGB_YELLOW);

		fbui_draw_string (dpy, win, pcf, 6,6, expr, fg);

		if (type == FBUI_EVENT_MOTION) {
			fbui_draw_line (dpy, win, 0,0, ev.x, ev.y, RGB_WHITE);
			fbui_flush (dpy, win);
		}
	}

	fbui_display_close (dpy);
}

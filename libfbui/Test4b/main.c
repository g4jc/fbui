
/*=========================================================================
 *
 * fbtest4b, an overlapping window test
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


#define NUMWINS 27

int
main(int argc, char** argv)
{
	Display *dpy;
	Window *windows[NUMWINS];
	int inside[NUMWINS];
	u32 fg,bg;
	int i;
	int vc=-1;

	i=1;
	while(i<argc) {
		if (!strncmp("-c",argv[i],2)) {
			if (argv[i][2])
				vc = atoi (2 + argv[i]);
		}
		i++;
	}

	fg = RGB_YELLOW;
	bg = RGB_BROWN;

	struct windata {
		short x;
		short y;
		short w;
		short h;
	};
	struct windata xy[NUMWINS] = {
		{ 0,20, 150, 100 }, // green
		{ 20,0, 100, 100 }, // red

		{ 220,0, 100, 100 },
		{ 200,20, 150, 100 },

		{ 400,0, 150, 100 },
		{ 420,20, 100, 100 },

		{ 620,20, 100, 100 },
		{ 600,0, 150, 100 },

		{ 0, 200, 150, 100 },
		{ 20, 200, 100, 100 },

		{ 220, 200, 100, 100 },
		{ 200, 200, 150, 100 },

		{ 400,200, 100, 150 },
		{ 400,220, 100, 100 },

		{ 600,220, 100, 100 },
		{ 600,200, 100, 150 },

		{ 800,200, 150,150 },
		{ 820,220, 100,100 },

		{ 820,20, 100,100 },
		{ 800,0, 150,150 },

		{ 0,420, 150,100 },
		{ 20,400, 100,150 },

		{ 220,400, 100,150 },
		{ 200,420, 150,100 },

		{ 400,400, 100, 100 },
		{ 420,420, 100, 150 },
		{ 440,440, 100,100 },
	};

	short win_w, win_h;
	dpy = fbui_display_open ();
	if (!dpy) 
		FATAL ("cannot open display");

	Font *pcf = Font_new ();
	if (!pcf_read (pcf, "timR12.pcf")) {
		Font_free (pcf);
		pcf = NULL;
		FATAL ("cannot load font");
	}
	short line_height = pcf->ascent + pcf->descent;

	srand (time(NULL));

	vc = dpy->console;

	win_w = 100;
	win_h = 100;

	for (i=0; i< NUMWINS; i++) {
		Window *win;
		char subtitle[10];

		inside[i] = 0;

		sprintf (subtitle, "%d", i);

		short x,y;

		x = xy[i].x;
		y = xy[i].y;
		win_w = xy[i].w;
		win_h = xy[i].h;
		bg = (i&1) ? RGB_RED: RGB_GREEN;

		win = fbui_window_open (dpy, win_w, win_h, 
			&win_w, &win_h, 999,999,
			x, y,
			&fg, &bg, 
			"fbtest4b", subtitle, 
			FBUI_PROGTYPE_APP, 
			false,false, vc,
			false, 
			false, 
			false, // i == 3 ? true : false, // hide the 4th window
			NULL, /* no mask */
			0,NULL);

		if (!win) {
			FATAL ("cannot open window");
		}

		windows[i] = win;
	}

	printf ("OverlapTest: All windows created.\n");

	int counter=0;
	int counter2=0;
	Window *overlap_win=NULL;

	while (1) {
		int total;
		int need=0;
		Event ev;
		Window *win;
		int x=0, y=0;
		int type;
		int err;

		counter++;
		if ((counter%9999)==1) {
			bg = RGB_CYAN;
			win_w = 700,
			win_h = 600,
			overlap_win = fbui_window_open (dpy, win_w, win_h, 
				&win_w, &win_h, 999,999,
				100,100,
				&fg, &bg, 
				"fbtest4b", "",
				FBUI_PROGTYPE_APP, 
				false,false, vc,
				false, 
				false, 
				false, 
				NULL, 0,NULL);
			if (!overlap_win) {
				fbui_update_error_loc (dpy);
				break;
			}
		}
		if ((counter%9999)==5000) {
			fbui_window_close (dpy, overlap_win);
			counter2++;
			if (counter2==3)
				break;
		}

		for (i = 0; i<NUMWINS; i++) {
			unsigned long color = (i&1) ? RGB_RED : RGB_GREEN;
			color &= rand();
			short x0 = rand() % 1000;
			short y0 = rand() % 1000;
			short x1 = rand() % 1000;
			short y1 = rand() % 1000;
			x0 -= 200;
			y0 -= 200;
			x1 -= 200;
			y1 -= 200;

			int err = fbui_draw_line (dpy, windows[i], x0,y0,x1,y1, color);
			if (err) {
				puts (fbui_error_name (err));
				break;
			}
			err = fbui_flush (dpy, windows[i]);
			if (err) {
				puts (fbui_error_name (err));
				break;
			}
		}

		if (fbui_poll_event (dpy, &ev, FBUI_EVENTMASK_ALL)) {
			if (ev.type == FBUI_EVENT_KEY) {
				printf ("got key\n");
				break;
			}
		}
	}

	fbui_display_close (dpy);
	return 0;
}


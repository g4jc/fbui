
/*=========================================================================
 *
 * fbtest5, a moving overlapping window test.
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


#define NUMWINS 4


void redraw (Display *dpy, Window *win)
{
	fbui_draw_rect (dpy, win, 0,0, win->width-1, win->height-1, RGB_BLUE);
	fbui_draw_line (dpy, win, 0,0, win->width-1, win->height-1, RGB_YELLOW);
	fbui_draw_line (dpy, win, 0,win->height-1,win->width-1, 0,  RGB_WHITE);
}

void redraw_rects (Display *dpy, Window *win, Event *ev)
{
	if (!dpy || !win || !ev)
		FATAL ("null ptr")

	int ix=0;
	//printf ("total rects = %d\n", ev->rects.total);

	redraw (dpy,win);
	return;

#if 0
	while (ix < (ev->rects.total << 2)) {
		short x0, x1, y0, y1;
		x0 = ev->rects.c[ix++];
		y0 = ev->rects.c[ix++];
		x1 = ev->rects.c[ix++];
		y1 = ev->rects.c[ix++];
// printf ("rect %d,%d--%d,%d\n", x0,y0,x1,y1);
		fbui_fill_rect (dpy, win, x0,y0,x1,y1, RGB_WHITE);
	}
	fbui_flush (dpy, win);
#endif
}

int
main(int argc, char** argv)
{
	Display *dpy;
	Window *windows[NUMWINS];
	int inside[NUMWINS];
	u32 fg,bg;
	int i;
	int vc=-1;

	double speed = 8.0;

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

	unsigned long colors[7] = { 0xff0000, 0xff00, 0xff, 0xff00ff, 0xffff, 0xffff00 , 0xc0c0c0 };

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

	int trajs[28] = { 
10,20,30,40,50,60,70,
35, 140, 60, 100, 250, 9 , 190,
235, 240, 260, 200, 150, 29 , 290,
95, 190, 90, 194, 290, 99 , 199,

 };

	srand (time(NULL));

	double x,y;

	for (i=0; i< NUMWINS; i++) {
		Window *win;
		char subtitle[10];

		inside[i] = 0;

		sprintf (subtitle, "%d", i);

		bg = colors[i % 7];

		short win_w = 200;
		short win_h = 200;

		x = rand(NULL) % dpy->width;
		y = rand(NULL) % dpy->height;

		if (x+win_w > dpy->width)
			x = dpy->width - win_w;
		if (y+win_h > dpy->height)
			y = dpy->height - win_h;

		win = fbui_window_open (dpy, win_w, win_h, 
			&win_w, &win_h, 999,999,
			(int) x, (int) y,
			&fg, &bg, 
			"fbtest5", subtitle, 
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

		windows[i] = win;
	}

	printf ("OverlapTest: All windows created.\n");

	int counter=0;

	int countdown=500;
	while (countdown--) {
		int total;
		int need=0;
		Event ev;
		Window *win;
		int type;
		int err;

		for (i=NUMWINS-1; i>= 0; i--) 
		{
			win = windows[i];

			double trajectory = trajs[i];

			double dx = speed * sin(2.0 * 3.14159 * ((double) trajectory)/360.0);
			double dy = speed * cos(2.0 * 3.14159 * ((double) trajectory)/360.0);

			short x0, y0;
			if (0 > fbui_get_position (dpy,win,&x0,&y0))
				break;

			x = x0;
			y = y0;

			x += (int)dx;
			y += (int)dy;

			// window hit left side
			if (x <= 0) {
				x = 0;
				trajectory = 360-trajectory;
			} else
			// window hit right side
			if ((x+win->width) >= dpy->width) {
				x = dpy->width - win->width;
				trajectory = 360-trajectory;
			} else
			// window hit top
			if (y <= 0) {
				y = 0;
				if (trajectory < 180)
					trajectory = 180 - trajectory;
				else
					trajectory = 540 - trajectory;
			} else
			// window hit bottom
			if ((y+win->height) >= dpy->height) {
				y = dpy->height - win->height;
				if (trajectory < 180)
					trajectory = 180 - trajectory;
				else
					trajectory = 540 - trajectory;
			}

			trajs[i] = trajectory;
			
			// Move one window
			//
			fbui_move_resize (dpy, NULL, win->id, (short)x, (short)y, win->width, win->height);

			// Process any events caused by this window move.
			//
			// Must process events as soon as possible
			// since the queue has a finite length.
			//

			while (!(err = fbui_poll_event (dpy, &ev, FBUI_EVENTMASK_ALL))) {

				if (ev.type == FBUI_EVENT_EXPOSE) {
					if (ev.has_rects) {
						//printf ("got Expose for window %d, with RECTS\n", ev.win->id);
						redraw_rects (dpy, ev.win, &ev);
					} else {
						//printf ("got Expose for window %d, WITHOUT rects\n", ev.win->id);
						redraw (dpy, ev.win);
					}
				}
				else
				if (ev.type == FBUI_EVENT_MOVERESIZE) {
					if (ev.key)
						redraw (dpy, ev.win);
				}
				else
					printf ("got event type %d for window %d\n", ev.type,ev.win->id);
			}
		}
	}

	fbui_display_close (dpy);
	return 0;
}

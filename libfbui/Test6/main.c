
/*=========================================================================
 *
 * fbtest6, a moving overlapping window test
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


#define NUMWINS 3

#define MASKWIDTH 400
#define MASKHEIGHT 300
#define MASKSIZE ((MASKWIDTH+7)/8 * MASKHEIGHT)
static unsigned char mask [MASKSIZE];


void maskplot (int x, int y)
{
	int line_len = (MASKWIDTH+7)/8;
	if (x<0) x=0;
	if (y<0) y=0;
	if (x>=MASKWIDTH)
		x = MASKWIDTH-1;
	if (y>=MASKHEIGHT)
		y = MASKHEIGHT-1;
	int ix = line_len * y + (x/8);
	mask[ix] |= 1 << (x & 7);
}

void drawmask ()
{
	double angle;
	memset (mask, 0, MASKSIZE);

	for (angle = 0.0; angle < 180.0; angle += 0.24) {
		double x, y;
		double rad = 3.14159265 * 2.0 * angle / 360.0;
		x = MASKWIDTH/2 * sin (rad);
		y = MASKHEIGHT/2 * cos (rad);

		int i,j;
		i = MASKWIDTH/2 + x;
		j = MASKHEIGHT/2 + y;
		
		int k;
		for (k = MASKWIDTH - i; k <= i; k++) {
			maskplot (k, j);
		}
	}
}

void redraw (Display *dpy, Window *win)
{
	fbui_draw_rect (dpy, win, 0,0, win->width-1, win->height-1, RGB_BLUE);
	fbui_draw_line (dpy, win, 0,0, win->width-1, win->height-1, RGB_YELLOW);
	fbui_draw_line (dpy, win, 0,win->height-1,win->width-1, 0,  RGB_WHITE);
	fbui_flush (dpy, win);
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

	double speed = 50.0;

	drawmask();

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

	short x,y;

	for (i=0; i< NUMWINS; i++) {
		Window *win;
		char subtitle[10];

		inside[i] = 0;

		sprintf (subtitle, "%d", i);

		bg = colors[i];

		short win_w = MASKWIDTH;
		short win_h = MASKHEIGHT;

		x = 200.0 + i * 100.0;
		y = x;

		if (i) x+=15;
		win = fbui_window_open (dpy, win_w, win_h, 
			&win_w, &win_h, 999,999,
			x, y,
			&fg, &bg, 
			"fbtest5", subtitle, 
			FBUI_PROGTYPE_APP, 
			false,false, vc,
			true, 
			false, 
			false,
			NULL, //mask,
			0,NULL);

		if (!win) {
			FATAL ("cannot open window");
		}

		windows[i] = win;

		redraw (dpy, win);
	}

	printf ("OverlapTest: All windows created.\n");

	sleep(2);

	Window *win = windows[1];
	fbui_move_resize (dpy, NULL, win->id, 360, 320, win->width, win->height);

	int err;
	Event ev;
	while (!(err = fbui_poll_event (dpy, &ev, FBUI_EVENTMASK_ALL))) {
		if (ev.type == FBUI_EVENT_EXPOSE || (ev.type==FBUI_EVENT_MOVERESIZE && (ev.key & FBUI_CONTENTS_CHANGED)))
		redraw (dpy, ev.win);
	}

sleep (2);

	win = windows[0];
	fbui_move_resize (dpy, NULL, win->id, 170, 140, win->width, win->height);

	while (!(err = fbui_poll_event (dpy, &ev, FBUI_EVENTMASK_ALL))) {
		if (ev.type == FBUI_EVENT_EXPOSE || (ev.type==FBUI_EVENT_MOVERESIZE && (ev.key & FBUI_CONTENTS_CHANGED)))
		redraw (dpy, ev.win);
	}

sleep (2);
	win = windows[1];
	fbui_move_resize (dpy, NULL, win->id, 50, 100, win->width, win->height);

	while (!(err = fbui_poll_event (dpy, &ev, FBUI_EVENTMASK_ALL))) {
		if (ev.type == FBUI_EVENT_EXPOSE || (ev.type==FBUI_EVENT_MOVERESIZE && (ev.key & FBUI_CONTENTS_CHANGED)))
		redraw (dpy, ev.win);
	}

sleep (2);
	win = windows[1];
	fbui_move_resize (dpy, NULL, win->id, 500, 500, win->width, win->height);

	while (!(err = fbui_poll_event (dpy, &ev, FBUI_EVENTMASK_ALL))) {
		if (ev.type == FBUI_EVENT_EXPOSE || (ev.type==FBUI_EVENT_MOVERESIZE && (ev.key & FBUI_CONTENTS_CHANGED)))
		redraw (dpy, ev.win);
	}

sleep (2);

	fbui_display_close (dpy);
	return 0;
}

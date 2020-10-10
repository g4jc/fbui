
/*=========================================================================
 *
 * fbload, a load monitor for FBUI (in-kernel framebuffer UI)
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


/* Changes
 *
 * 26 Sep 2004: responds to geo changes.
 * 07 Sep 2005: added icon.
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "libfbui.h"


#define BK 0
#define LL RGB_YELLOW
#define w_ RGB_GREEN
static unsigned long icon [FBUI_ICON_WIDTH * FBUI_ICON_HEIGHT] = {
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,w_,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,w_,w_,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,w_,w_,w_,w_,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,w_,w_,w_,w_,w_,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,BK,BK,
BK,BK,BK,BK,BK,BK,w_,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,BK,
LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,LL,
BK,BK,BK,BK,BK,w_,w_,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,
BK,BK,BK,BK,w_,w_,w_,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,
BK,BK,BK,BK,w_,w_,w_,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,
BK,BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,
BK,BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,BK,
BK,BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,
BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,
BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,
BK,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,
w_,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,BK,
w_,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,
w_,BK,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,
w_,w_,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,BK,
w_,w_,BK,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,BK,
w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,
w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,w_,
};

float getload ()
{
	float a,b,c;
	FILE *f = fopen ("/proc/loadavg", "r");
	if (3 != fscanf (f, "%f %f %f", &a, &b, &c))
		a = 0.0;
	fclose (f);
	return a;
}


int
main(int argc, char** argv)
{
	int i;
	Display *dpy;
	Window *win;
	int seconds_per = 5;

	if (argc >= 2 && isdigit(*argv[1]) && '0'!=isdigit(*argv[1]))
		seconds_per = atoi (argv[1]);

	short w = 50;
	short h = 120;
	float *values;
	unsigned long fgcolor = RGB_GREEN;
	unsigned long bgcolor = 0x2000;

	dpy = fbui_display_open ();
	if (!dpy)
		FATAL ("cannot open display");

	win = fbui_window_open (dpy, w,h, &w,&h, 9999,9999, -1, -1, 
		&fgcolor, &bgcolor, "fbload", "", 
		FBUI_PROGTYPE_TOOL,false,false, -1,false, 
		false, false, NULL, argc,argv);
	if (!win) 
		FATAL ("cannot create window");

	fbui_set_icon (dpy,win,icon);

	int last_x = w - 1;
	int last_y = h - 1;

	values = (float*) malloc (w * sizeof(float));
	if (!values)
		FATAL("out of memory");
	memset (values, 0, w * sizeof(float));

	unsigned long barcolor = fgcolor;

	int need_shift = true;
	int need_redraw = true;

	int range=0;
	time_t t0 = time(NULL);
	while(1) {
		Event ev;
		time_t t;
		usleep (250000);

		t = time(NULL);
		int tdiff = t - t0;
		if (tdiff >= seconds_per) {
			need_shift = true;
			t0 = t;
		}

		if (!need_shift) {
			int err;
			if (err = fbui_poll_event (dpy, &ev,
					     FBUI_EVENTMASK_ALL & ~FBUI_EVENTMASK_KEY))
			{
				if (err != FBUI_ERR_NOEVENT)
					fbui_print_error (err);
				continue;
			}
printf ("%s got event %s\n", argv[0], fbui_get_event_name (ev.type));

			int num = ev.type;
			char key = ev.key;

			if (ev.win != win)
				FATAL ("event for wrong window");

			switch (num) {
			case FBUI_EVENT_EXPOSE:
				need_redraw = true;
				break;

			case FBUI_EVENT_MOVERESIZE:
				if (ev.key & FBUI_SIZE_CHANGED) {
					int count = sizeof(float) * ev.width;
					float *values2 = (float*) malloc(count);
					if (!values2)
						FATAL("out of memory");
					memset (values, 0, count);

					int k = 0;
					while (k < w && k < ev.width) {
						values2 [ev.width - 1 - k] = values [w - 1 - k];
						k++;
					}
					free (values);
					values = values2;

					w = ev.width;
					h = ev.height;
				}
				if (ev.key & FBUI_CONTENTS_CHANGED)
					need_redraw = true;
				break;
			}
		}

		if (need_shift) {

			/* shift values to left */
			for (i=1; i<w; i++)
				values[i-1] = values[i];

			int nu = values[w-1] = getload();
			nu++;
			if (nu > range)
				need_redraw = 1;

			/* get max value */
			float max = 0;
			for (i=0; i<w; i++) {
				if (max < values[i])
					max = values[i];
			}

			range = max;
			range++;
			if (!range)
				range = 1;

			need_redraw = true;
		}

		if (need_redraw) {
			fbui_clear (dpy, win);

			for (i=0; i<w; i++)
				fbui_draw_vline (dpy, win, i, 
					h - ((float)h/range) * values[i], h, barcolor);

			if (range > 1) {
				for (i=0; i<range; i++) {
					int y = h/range;
					y *= i;
					y = h - y;
					fbui_draw_hline (dpy, win, 0, last_x, y, RGB_YELLOW);
				}
			}
			fbui_flush(dpy, win);
		}

		need_redraw = false;
		need_shift = false;
	}

	fbui_window_close (dpy, win);
	fbui_display_close (dpy);
	return 0;
}

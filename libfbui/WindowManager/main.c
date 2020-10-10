
/*=========================================================================
 *
 * fbwm, a window manager for FBUI (in-kernel framebuffer UI)
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

/*
 * Changes:
 *
 * 19 Oct 2004, fbui@comcast.net: got rectangle subtraction working
 * 19 Oct 2004, fbui@comcast.net: got background image drawing working
 * 31 Dec 2004, fbui@comcast.net: moved rectangle code to separate file
 * 07 Sep 2005, fbui@comcast.net: icons now displayed at top
 * 12 Sep 2005, fbui@comcast.net: icons may now be clicked to set keyfocus
 * 14 Sep 2005, fbui@comcast.net: implemented Alt-M based window move.
 * 27 Sep 2005, fbui@comcast.net: improvements to icon drawing.
 */



#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <jpeglib.h>
#include <signal.h>


#include "libfbui.h"
#include "libfbuifont.h"


short win_w, win_h;
#define STEELBLUE 0x4682B4


struct fbui_wininfo info[200];
struct fbui_wininfo prev[200];
int window_count;


int focus_id = -1;


int icon_ids[100];
int total_icons_drawn =0;


void
update_focusgraphics (Display *dpy, Window *self, bool draw)
{
	int i=0;
	int icon_x = 0;
	total_icons_drawn = 0;

	/* Clear the area where the icons are displayed.
	 */
	fbui_clear_area (dpy, self, 0,0, dpy->width-1, FBUI_ICON_HEIGHT+3);

	bool found_focus_icon = false;
	int focus_x = -1;

	/* determine which windows have disappeared */
	for (i=0; i<window_count; i++) 
	{
		/* draw any icon in the lower right */
		short iw = FBUI_ICON_WIDTH;
		short ih = FBUI_ICON_HEIGHT;
		RGB icon [iw * ih];
		bool got_icon = false;
		int id = info[i].id;

		if (!fbui_get_icon (dpy, self, id, icon)) {
			int i,j;
			got_icon = true;
			icon_ids[total_icons_drawn++] = id;

			if (id == focus_id) {
				found_focus_icon = true;
				focus_x = icon_x;
			}

			for (i=0; i<iw; i++) {
				for (j=0; j<ih; j++) {
					unsigned long pix = icon[iw*j+i];
					fbui_draw_point (dpy,self,icon_x + i,j,pix);
				}
			}

			icon_x += 2 + iw;
		} 
	}

	fbui_clear_area (dpy,self,0,FBUI_ICON_HEIGHT+2, total_icons_drawn * FBUI_ICON_WIDTH-1, FBUI_ICON_HEIGHT+3);

	if (found_focus_icon) {
		fbui_fill_rect (dpy,self,
			focus_x, FBUI_ICON_HEIGHT+2, focus_x+FBUI_ICON_WIDTH-1, 
			FBUI_ICON_HEIGHT+3, RGB_GREEN);
	}
	fbui_flush (dpy, self);
}


void
update_focus (Display *dpy, Window *self)
{
	int i;
	for (i=0; i<window_count; i++) {
		if (info[i].program_type == FBUI_PROGTYPE_APP) {
			int id = info[i].id;
			fbui_assign_keyfocus (dpy, self, id);
			focus_id = id;
			update_focusgraphics (dpy, self, 1);
			return;
		}
	}
}


void
process_window_move (Display *dpy, Window *self)
{
	fbui_assign_ptrfocus (dpy, self, self->id);

	/* Wait for click */
	Event ev;
	int id=-1;
	int err;
	while (!(err = fbui_wait_event (dpy, &ev, FBUI_EVENTMASK_ALL)))
	{
		if (ev.type == FBUI_EVENT_BUTTON && 
		    (ev.key == FBUI_BUTTON_LEFT+FBUI_BUTTON_DOWN)) {
			id = fbui_get_window_at_xy (dpy,self, ev.x, ev.y);
			printf ("MOVE id = %d\n", id);
			break;
		} 
	}

	if (id >= 0) {
		short win_x, win_y;
		short win_w, win_h;
		bool found=false;
		int i=0;
		while (i < window_count) {
			if (info[i].id == id) {
				found=true;
				win_x = info[i].x;
				win_y = info[i].y;
				win_w = info[i].width;
				win_h = info[i].height;
				break;
			}
			i++;
		}
		if (!found)
			return; /* background was clicked */

		short first_x = ev.x;
		short first_y = ev.y;

		while (!(err = fbui_wait_event (dpy, &ev, FBUI_EVENTMASK_ALL)))
		{
			if (ev.type == FBUI_EVENT_BUTTON && 
			    ev.key == FBUI_BUTTON_LEFT) {
				break;
			} 
			else if (ev.type == FBUI_EVENT_MOTION) {
				short dx = ev.x - first_x;
				short dy = ev.y - first_y;
				short x = dx + win_x;
				short y = dy + win_y;
				if (x < 0)
					x = 0;
				if (y < 0)
					y = 0;
				if (x+win_w-1 >= dpy->width)
					x = dpy->width - win_w;
				if (y+win_h-1 >= dpy->height)
					y = dpy->height - win_h;
				fbui_move_resize (dpy, self, id, x, y, win_w, win_h);
			}
		}
	}

	if (id >= 0)
		fbui_redraw (dpy, self, id);

	window_count = fbui_window_info (dpy, self, &info[0], 200);
	fbui_assign_ptrfocus (dpy, self, -1);
}


#if 0
	int i;
	for (i=0; i<window_count; i++) {
		short x0 = info[i].x;
		short y0 = info[i].y;
		short x1 = x0 + info[i].width - 1;
		short y1 = x0 + info[i].width - 1;
		if (x >= info[i].x  && x <= info[i].x1) 
			fbui_assign_keyfocus (dpy, self, info[i].id);
	}
#endif


void
process_click_on_icon (Display *dpy, Window *self, short x, short y)
{
	/* Determine which icon was clicked */

	if (y > FBUI_ICON_HEIGHT)
		return;

	int i = (x-2) / (FBUI_ICON_WIDTH+2);
	if (i < 0)
		i = 0;
	if (i > total_icons_drawn)
		return;

	int id = icon_ids[i];

	/* Determine whether window can take keyfocus */
	bool can_take =false;
	i = 0;
	while (i < window_count) {
		if (info[i].id == id) {
			can_take = info[i].need_keys;
			break;
		}
		i++;
	}

	if (can_take) {
		fbui_assign_keyfocus (dpy, self, id);
		update_focusgraphics (dpy, self, 0);
		focus_id = id;
		update_focusgraphics (dpy, self, 1);
	}
}


void
gradient (Display *dpy, Window *wm, short x0, short y0, short x1, short y1)
{
	int i;
	for (i=x0; i<=x1; i++) {
		fbui_draw_vline (dpy, wm, i, y0, y1, i & 255);
	}
	fbui_flush (dpy, wm);
}


int
main(int argc, char** argv)
{
	int mypid,i,j;
	Display *dpy;
        Font *pcf;
	Window *self;
	short line_height;

	dpy = fbui_display_open ();
	if (!dpy)
		FATAL ("request for control denied");

        pcf = Font_new ();
	if (!pcf)
		FATAL ("out o' memory");

        if (!pcf_read (pcf, "timR12.pcf")) {
                Font_free (pcf);
        	FATAL ("cannot read Times 12");
        }

	line_height = pcf->ascent + pcf->descent;

	long fg,bg=0x108020;

	self = fbui_window_open (dpy, 1,1, &win_w, &win_h, 9999,9999,
		0, 0, 
		&fg, &bg, 
		"fbwm", "", 
		FBUI_PROGTYPE_WM,
		true,  // we are a window manager
		false, // but we are not autopositioning anything
		-1,
		false, // we don't need keys
		false, // don't need all motion
		false, // not hidden
		NULL, /* no mask */
		argc,argv);
        if (!self)
                FATAL ("cannot open manager window");

	/* Create the greyscale background pattern.
	 */
#if 0
	unsigned long bgsize = dpy->width * dpy->height * 4;
	unsigned char *background_image = malloc (bgsize);
	if (background_image) {
		unsigned char *p = background_image;
		unsigned char q = 0;
		while (bgsize--) 
			*p++ = q++;
		if (fbui_set_bgimage (dpy, self, background_image)) 
			FATAL ("failed to set background image");
	} else
		printf ("fbwm: cannot allocate %d bytes for greyscale background image\n", bgsize);
#endif

	/* Check for a background image */
#if 0
	image_width=0;
	image_height=0;
	image_ncomponents=0;
	image_buffer=NULL;
#endif

	char *image_path=NULL;
	i=1;
	while (i<argc) {
		char *s = argv[i];
		int ch = s ? s[0] : 0;
		if (ch && '-' != ch) {
			image_path = argv[i];
			break;
		}
		i++;
	}
	if (image_path) {
		int result;
#if 0
		result = read_JPEG_file (image_path);
                if (image_ncomponents != 3)
			result=0;
		if (!result)
			FATAL ("cannot read background image");
#endif
	}

	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '1', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '2', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '3', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '4', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '5', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '6', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '7', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '8', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '9', 1))
		FATAL ("cannot register accelerator");
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '0', 1))
		FATAL ("cannot register accelerator");

	/* Window move */
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, 'm', 1))
		FATAL ("cannot register accelerator");

	/* Alt-tab switches between apps
	 */
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '\t', 1))
		FATAL ("cannot register accelerator");

	/* Alt-backsp shuts down FBUI on the current VC
	 */
	if (FBUI_SUCCESS != fbui_accelerator (dpy, self, '\b', 1))
		FATAL ("cannot register accelerator");

	int need_list = 1;
	int need_redraw = 0;

	while(true) {
		Event ev;
		int err;
		if (err = fbui_wait_event (dpy, &ev, FBUI_EVENTMASK_ALL))
		{
			fbui_print_error (err);
			continue;
		}
		int type = ev.type;
		if (ev.win != self) {
			FATAL ("got event for another win");
		}

		switch (type) {
		case FBUI_EVENT_EXPOSE:
			need_redraw = 1;
			break;
		
		case FBUI_EVENT_WINCHANGE:
			need_list = 1;
printf ("GOT WINCHANGE\n");
			break;

		case FBUI_EVENT_MOTION:
			break;

		case FBUI_EVENT_BUTTON:
			process_click_on_icon (dpy, self, ev.x, ev.y);
			break;

		case FBUI_EVENT_ACCEL: {
			short key = ev.key;

			switch (key) {
			case '\b':
				goto done;

			case '\t':
				continue;
			
			case 'm':
				process_window_move (dpy, self);
				;
			}
			break;
		 }

		default:
			printf ("fbwm: event type %d\n", type);
		}

		if (need_list) {
			int i;
			int prev_count = window_count;

			need_redraw = 1;

			for (i=0; i<window_count; i++)
				prev[i] = info[i];

			window_count = fbui_window_info (dpy, self, &info[0], 200);
printf ("fbwm: found %d windows\n", window_count);

			/* determine which windows have disappeared */
			for (i=0; i<window_count; i++) {
				int id = info[i].id;
				int missing=1;
				int j=0;

printf ("%d: window id %d, %s width %d height %d \n", 
	i, info[i].id, info[i].name, info[i].width, info[i].height);
				while (j < window_count) {
					if (prev[i].pid == info[j].pid) {
						missing=0;
						break;
					}
					j++;
				}
				if (missing) {
#if 0
					draw_image (dpy, self,
					  prev[i].x,
					  prev[i].y - line_height,
					  prev[i].x + prev[i].width - 1,
					  prev[i].y - 1);
					fbui_flush(dpy, self);
#endif
				}
			}

			update_focus (dpy, self);

			need_list=0;
		} 
		
		if (need_redraw) {
			int i;

			need_redraw = 0;

			update_focusgraphics (dpy,self,true);

#if 0
			/* draw the background gradient */
			if (ev.has_rects) {
				int ix=0;
				while ((ix>>2) < ev.rects.total) {
					short x0 = ev.rects.c[ix++];
					short y0 = ev.rects.c[ix++];
					short x1 = ev.rects.c[ix++];
					short y1 = ev.rects.c[ix++];

					gradient (dpy, self, x0,y0,x1,y1);
				}
			} else {
				gradient (dpy, self, 0,0, dpy->width-1, dpy->height-1);
			}
#endif

			i=0; 
			while (i < window_count) {
				char tmp[100];
				struct fbui_wininfo *wi = &info[i];
				int x,y;

				// locate window
				x = wi->x;
				y = wi->y;

#if 0
				// draw gradient behind text
				int j=0;
				while (j < wi->width) {
					unsigned long r,g,b;
					int k = (255 - j/2) > 1 ? (255 - j/2) : 0;
					r = 30;
					g = 32 + (7*k)/8;
					b = k/3;
					r <<= 16;
					b <<= 8;
					fbui_draw_vline (dpy, self, x+j, y-1, y-line_height,
						r|g|b);
					j++;
				}

				// draw program name/pid
				sprintf(tmp,"%s (pid=%d)", wi->name,wi->pid);
				fbui_draw_string (dpy, self, pcf,x,y-line_height,tmp, RGB_YELLOW);
#endif

				i++;
			}
		}
	}

done:
	mypid = getpid();
	for (i=0; i<window_count; i++) {
		if (info[i].pid != mypid) {
			int pid = info[i].pid;
			printf ("fbwm: sending SIGTERM to pid %d\n", pid);
			kill (pid, SIGTERM);
		}
	}

	fbui_display_close (dpy);
	return 0;
}

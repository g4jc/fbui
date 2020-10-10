
/*=========================================================================
 *
 * fbdirectory, a directory-contents viewer for FBUI
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


/* Changes
 *
 */

#include <stdio.h>
#include <sys/param.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h> // dirent
#include <dirent.h> // opendir
#include <unistd.h> // getcwd
#include <sys/stat.h>


#include "libfbui.h"
#include "libfbuifont.h"


#define VERSION "0.1"


// Window dimensions
static short w = 300;
static short h = 400;


static int pos_in_list = 0;


Font *titlefont, *listfont, *boldfont;
short titlefontheight, listfontheight;


short total_items;
typedef struct item {
	char *s1, *s2;
	unsigned int is_dir : 1;
	struct item *next;
} Item;

Item *items;

Item *Item_new (char *s1, char *s2, char flag) 
{
	if (!s1 || !s2) return NULL;

	struct item *nu = (struct item*)malloc(sizeof(struct item));
	if (!nu)
		FATAL("cannot allocate list item");

	nu->next=NULL;
	nu->s1 = strdup(s1);
	nu->s2 = strdup(s2);
	nu->is_dir = flag;
	if (!nu->s1 || !nu->s2)
		FATAL("cannot allocate string for item");
	return nu;
}

Item *Item_append_pair (char *s1, char *s2, char flag) 
{
	struct item *nu = Item_new (s1,s2, flag);
	if (!nu) 
		FATAL ("out of memory");

	struct item *prev = NULL;
	struct item *im = items;

	while (im && strcmp (s1, im->s1) > 0) {
		prev = im;
		im = im->next;
	}

	if (!prev) {
		nu->next = items;
		items = nu;
	} else {
		prev->next = nu;
		nu->next = im;
	}

	++total_items;

	return nu;
}


void get_dir_list (char *path)
{
	DIR *dir = opendir (path);
	struct dirent *de;
	char str [MAXPATHLEN];

	while ((de = readdir (dir))) {
			
		strcpy (str, path);
		strcat (str, de->d_name);

		struct stat st;
		stat (str, &st);

		char d = S_ISDIR (st.st_mode);
		
		Item *im = Item_append_pair (de->d_name, "foo", d);
	}

	closedir (dir);
}


char free_items ()
{
	Item *im = items;
	while (im) {
		Item *next = im->next;
		free (im->s1);
		free (im->s2);
		free (im);
		im = next;
	}
	items = NULL;
	total_items = 0;
	pos_in_list = 0;
}

Item *Item_lookup_n (int n)
{
	Item *im = items;

	while (im && n--)
		im = im->next;

	return im;
}


int
main(int argc, char** argv)
{
	int i;
	Display *dpy;
	Window *win;
	char path[MAXPATHLEN];
	struct stat st;
	FILE *f;
	int highlight_which = 0;

	total_items = 0;
	items=NULL;

	dpy = fbui_display_open ();
	if (!dpy)
		FATAL("cannot open display");

        titlefont = Font_new ();
        listfont = Font_new ();
        boldfont = Font_new ();

	if (!titlefont || !listfont)
		FATAL ("cannot allocate Font");

        if (!pcf_read (titlefont, "helvB14.pcf"))
		FATAL ("cannot read Helv bold 12 point");
        if (!pcf_read (listfont, "timR12.pcf"))
		FATAL ("cannot read Times 12 point");
        if (!pcf_read (boldfont, "timB12.pcf"))
		FATAL ("cannot read Times Bold 12 point");

	short indent,a,d;
	Font_string_dims (listfont, "8888. ", &indent, &a, &d);

	listfontheight = listfont->ascent + listfont->descent;
	titlefontheight = titlefont->ascent + titlefont->descent;

	unsigned long xcolor = RGB_RED;
	unsigned long arrowcolor = RGB_BLUE;
	unsigned long titlecolor = RGB_GREY;
	unsigned long titlefontcolor = RGB_BLACK;
	unsigned long listcolor = RGB_WHITE;
	unsigned long listfontcolor = RGB_BLACK;
	unsigned long sepcolor = 0xe0e0e0;
	unsigned long listhighlightcolor = 0xc0c0c0;

	win = fbui_window_open (dpy, w,h, &w, &h, 9999,9999, 0, -50, 
		&listfontcolor, 
		&listcolor,
		"fbdirectory", VERSION, 
		FBUI_PROGTYPE_APP, 
		false,false, 
		-1, 
                true, // need keys
		false,
		false, 
		NULL,
		argc,argv);
	if (!win) 
		FATAL ("cannot create window");

	// Determine the path of the directory whose contents 
	// we will display.
	//
	char gotpath = 0;
	i = 1;
	while (i < argc) {
		char ch = argv[i][0];
		if (ch && ch != '-') {
			if (ch != '/') {
				getcwd (path, MAXPATHLEN-1);
			}
			if (path [strlen(path)-1] != '/')
				strcat (path, "/");
			strcpy (path, argv[i]);
			gotpath = 1;
			break;
		}
		i++;
	}
	if (!gotpath) {
		strcpy (path, getenv ("HOME"));
		if (path [strlen(path)-1] != '/')
			strcat (path, "/");
	}

	int rows_available = h - titlefontheight - 1;
	rows_available /= listfontheight;

	char needdraw=0;
	char fulldraw=0;

	char done=0;
	while(!done) {
		// Read the contents of the directory
		//
		get_dir_list (path);

		short x=-1, y=-1;
		short whichlaunch = -1;
		short oldwhich = -1;
		short which = -1;

		while (!done) {
			Event ev;
			int err;

			if (!needdraw && !fulldraw) {

				if (err = fbui_wait_event (dpy, &ev, FBUI_EVENTMASK_ALL)) {
					fbui_print_error (err);
					continue;
				}

				int type = ev.type;
				Window *win2 = ev.win;

				if (win2 != win)
					FATAL ("event's window is not ours");

				switch(type) {
				case FBUI_EVENT_MOTION: 
					x = ev.x;
					y = ev.y;

					if (y >= (titlefontheight + 1)) {
						oldwhich = which;
						which = (y - titlefontheight - 1) / listfontheight;
						if (which > total_items)
							which = total_items;
						if (which != highlight_which) {
							highlight_which = which;
							needdraw = 1;
						} else
							highlight_which = -1;
					}
					break;
			
				case FBUI_EVENT_EXPOSE:
					needdraw=1;
					fulldraw=1;
					break;

				case FBUI_EVENT_ENTER:
					break;

				case FBUI_EVENT_BUTTON:
					if (y < titlefontheight) {
						if (x >= (w - 10)) {
							free_items ();
							done=true;
						}
						continue;
					}

					if (ev.key & FBUI_BUTTON_LEFT) {
						i = (y - titlefontheight) / listfontheight;

						if (i == 0) {
							--pos_in_list;
							if (pos_in_list < 0)
								pos_in_list = 0;
							else
								needdraw = fulldraw = 1;
						}
						else
						if (i == rows_available-1) {
							++pos_in_list;
							if (pos_in_list >= total_items)
								pos_in_list = total_items-1;
							else
								needdraw = fulldraw = 1;
						}
						else 
						if (ev.key & 1) {
							whichlaunch = i-1 + pos_in_list;
							done=true;
							continue;
						}
					}
					break;

				case FBUI_EVENT_LEAVE:
					if (highlight_which != -1) {
						highlight_which = -1;
						needdraw=1;
					}
					break;

				case FBUI_EVENT_MOVERESIZE:
printf ("got move-resize, dims are now %d x %d\n", w,h);
					w = ev.width;
					h = ev.height;
					rows_available = h - titlefontheight - 1;
					rows_available /= listfontheight;
					needdraw = fulldraw = true;
					break;
				}
			}
			
			if (!needdraw) 
				continue;

			if (fulldraw) {
				char title[200];
				sprintf (title, "fbdirectory: %s", path);

				short wid,a,d;
				Font_string_dims (titlefont, title, &wid,&a,&d);
				short title_pos = 5;

				fbui_fill_rect (dpy, win, 0, 0, w-1, titlefontheight, titlecolor);
				fbui_draw_hline (dpy, win, 0, w-1, titlefontheight, sepcolor);
				fbui_draw_string (dpy, win, titlefont, title_pos, 0, 
					title, titlefontcolor);

				fbui_fill_rect (dpy, win, 0, titlefontheight+1, w-1, h, listcolor);

				// Draw the X
				fbui_draw_line (dpy, win, w-10, 2, w-3, 9, xcolor);
				fbui_draw_line (dpy, win, w-10, 9, w-3, 2, xcolor);
			} 

			struct item *im = items;
			i = pos_in_list;
			while (i>0 && im) {
				im = im->next;
				i--;
			}

			short y2;
			int ix=0;
			while (im && ix < (rows_available-2)) {
				char numstr[5];
				sprintf (numstr, "%d.", ix+1+pos_in_list);

				y2 = titlefontheight+1+(1+ix)*listfontheight;

				if (fulldraw ||
				    (which != -1 && which == (1+ix)) ||
				    (oldwhich != -1 && oldwhich == (1+ix)))
				{
					if (1+ix == highlight_which) 
						fbui_fill_rect (dpy, win, 0, y2, w, y2 + listfontheight-1,
							listhighlightcolor);
					else
						fbui_fill_rect (dpy, win, 0, y2, w, y2 + listfontheight-1,
							 listcolor);

					fbui_draw_string (dpy, win, listfont, 
						0, y2, numstr, listfontcolor);
			
					fbui_draw_string (dpy, win, im->is_dir ? boldfont : listfont, 
						indent, y2, im->s1, listfontcolor);
				}

				im = im->next;
				ix++;
			}

			// Draw arrows
			//
			y2 = titlefontheight+1;
			fbui_draw_vline (dpy, win, w/2, y2, y2 + listfontheight-1, arrowcolor);
			fbui_draw_line (dpy, win, w/2, y2, w/2-4, y2 + listfontheight/2, arrowcolor);
			fbui_draw_line (dpy, win, w/2, y2, w/2+4, y2 + listfontheight/2, arrowcolor);

			y2 = titlefontheight+1 + listfontheight*(rows_available-1);
			fbui_draw_vline (dpy, win, w/2, y2, y2 + listfontheight-1, arrowcolor);
			fbui_draw_line (dpy, win, w/2, y2+listfontheight-1, w/2-4, y2 + listfontheight/2, arrowcolor);
			fbui_draw_line (dpy, win, w/2, y2+listfontheight-1, w/2+4, y2 + listfontheight/2, arrowcolor);

			fbui_flush (dpy, win);

			needdraw=0;
			fulldraw=0;
		}

		if (-1 != whichlaunch) {
			Item *launch = Item_lookup_n (whichlaunch);

			if (launch->is_dir) {
				if (!strcmp (".", launch->s1)) {
					// path doesn't change
				} else 
				if (!strcmp ("..", launch->s1)) {
					int i = strlen (path) - 1;
					path[i--] = 0;
					while (i >= 0 && path[i] != '/')
						path[i--] = 0;
				} else {
					strcat (path, launch->s1);
					strcat (path, "/");
				}

				free_items ();
				items = NULL;
				needdraw = fulldraw = 1;
				done=false;
				continue;
			}

			fbui_window_close (dpy, win);
			fbui_display_close (dpy);

			if (launch)
				system (launch->s2);

			exit(0);
		}
	}

	fbui_window_close (dpy, win);
	fbui_display_close (dpy);

	return 0;
}

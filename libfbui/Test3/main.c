
/*=========================================================================
 *
 * fbtest3, a test for FBUI which deliberately does things wrongly.
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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <linux/vt.h>
#include <linux/input.h>


#include "libfbui.h"
#include "libfbuifont.h"

#define TESTFAIL(BB) { fbui_update_error_loc (dpy); fprintf(stderr, "fbtest3 test failed, reason: %s, location %d\n", BB,fbui_errloc); goto quit; }

int
main(int argc, char** argv)
{
	int i;
	int counter;
	int vc=-1;

	Display *dpy;
	Window *win;

	short width, height;
	short maxwidth, maxheight;
	short x,y;
	long fg,bg;
	char name[20];
	char subtitle[20];
	char progtype;

	dpy = fbui_display_open ();
	if (!dpy) 
		FATAL ("cannot open display");

	Font *pcf = Font_new ();
	if (!pcf_read (pcf, "timR12.pcf")) {
		Font_free (pcf);
		pcf = NULL;
		FATAL ("cannot load font");
	}

	/*----------------------------------------------------------
	 * LIBFBUI tests 
	 *----------------------------------------------------------*/

	/* Test A: check zero & negative width/height
	 */
	width = 0;
	height = 1;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_YELLOW;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height, 
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("zero width accepted by fbui_window_open")

	width = 1;
	height = 0;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_GREEN;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height, 
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("zero height accepted by fbui_window_open")

	width = -1;
	height = 1;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_BLUE;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height, 
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("negative width accepted by fbui_window_open")

	width = 1;
	height = -1;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_ORANGE;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height, 
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("negative height accepted by fbui_window_open")

	/* Test B: null pointers
	 */
	width = 100;
	height = 100;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_WHITE;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, NULL, &height, 
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("null ptr accepted by fbui_window_open")

	width = 100;
	height = 100;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_BROWN;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, NULL,
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("null ptr accepted by fbui_window_open")

	width = 100;
	height = 100;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_YELLOW;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height,
		maxwidth, maxheight, x, y, NULL, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 	
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("null ptr accepted by fbui_window_open")

	width = 100;
	height = 100;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_YELLOW;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height,
		maxwidth, maxheight, x, y, &fg, NULL, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("null ptr accepted by fbui_window_open")

	width = 100;
	height = 100;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_YELLOW;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height,
		maxwidth, maxheight, x, y, &fg, &bg, NULL, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("null ptr accepted by fbui_window_open")

	width = 100;
	height = 100;
	maxwidth = 1000;
	maxheight = 1000;
	fg = RGB_RED;
	bg = RGB_YELLOW;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height,
		maxwidth, maxheight, x, y, &fg, &bg, name, NULL, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (win)
		TESTFAIL("null ptr accepted by fbui_window_open")

	/* Test B: large numbers
	 */
	width = 32767;
	height = 1;
	maxwidth = 32767;
	maxheight = 32767;
	fg = RGB_RED;
	bg = RGB_YELLOW;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height,
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (!win)
		TESTFAIL("large width rejected by fbui_window_open")
	fbui_window_close (dpy, win);

	width = 1;
	height = 32767;
	maxwidth = 32767;
	maxheight = 32767;
	fg = RGB_RED;
	bg = RGB_YELLOW;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height,
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (!win)
		TESTFAIL("large height rejected by fbui_window_open")
	fbui_window_close (dpy, win);

	/*----------------------------------------------------------
	 * IOCTL tests 
	 *----------------------------------------------------------*/
	width = 600;
	height = 400;
	maxwidth = 999;
	maxheight = 999;
	fg = RGB_RED;
	bg = RGB_YELLOW;
	strcpy (name,"fbtest3");
	strcpy (subtitle,"foo");
	x=y=0;
	progtype=0;
	win = fbui_window_open (dpy, width,height, &width, &height,
		maxwidth, maxheight, x, y, &fg, &bg, name, subtitle, 
		progtype, 0, 0, -1, 0, 0, 0, 
		NULL, /* no mask */
		argc,argv);
	if (!win)
		TESTFAIL("legitimate window params rejected by fbui_window_open")

	/* Test A: all zero data
	 */
	for (counter=0; counter<1000; counter++) {
		fbui_errno = FBUI_SUCCESS;
		win->command[0] = win->id;
		win->command[1] = LIBFBUI_COMMANDBUFLEN-2;
		for (i=2; i<LIBFBUI_COMMANDBUFLEN; i++)
			win->command[i] = 0;
		if (ioctl (dpy->fd, FBIO_UI_EXEC, (void*) win->command) < 0) {
			fbui_errno = -errno;
			TESTFAIL("all zero commands rejected")
		}
	}

	/* Test B: all 0xff data
	 */
	for (counter=0; counter<1000; counter++) {
		fbui_errno = FBUI_SUCCESS;
		win->command[0] = win->id;
		win->command[1] = LIBFBUI_COMMANDBUFLEN-2;
		for (i=2; i<LIBFBUI_COMMANDBUFLEN; i++)
			win->command[i] = 0xff;
		if (!ioctl (dpy->fd, FBIO_UI_EXEC, (void*) win->command)) {
			fbui_errno = -errno;
			TESTFAIL("all 0xff commands accepted")
		}
	}

	/* Test C: all random data
	 */
	for (counter=0; counter<1000; counter++) {
		fbui_errno = FBUI_SUCCESS;
		win->command[0] = win->id;
		win->command[1] = LIBFBUI_COMMANDBUFLEN-2;
		for (i=2; i<LIBFBUI_COMMANDBUFLEN; i++)
			win->command[i] = rand();
		ioctl (dpy->fd, FBIO_UI_EXEC, (void*) win->command);
	}

	/* Test D: real drawing commands with random params
	 */
	for (counter=0; counter<10000; counter++) {
		fbui_errno = FBUI_SUCCESS;
		win->command[0] = win->id;
		win->command[1] = LIBFBUI_COMMANDBUFLEN-2;
		win->command[2] = counter & 31;
		for (i=3; i<LIBFBUI_COMMANDBUFLEN; i++)
			win->command[i] = rand();
		ioctl (dpy->fd, FBIO_UI_EXEC, (void*) win->command);
	}

	/* Test E: real drawing commands & random command lengths
	 */
	for (counter=0; counter<1000; counter++) {
		fbui_errno = FBUI_SUCCESS;
		win->command[0] = win->id;
		win->command[1] = rand();
		win->command[2] = counter & 31;
		for (i=3; i<LIBFBUI_COMMANDBUFLEN; i++)
			win->command[i] = rand();
		ioctl (dpy->fd, FBIO_UI_EXEC, (void*) win->command);
	}

	/* Test F: real drawing commands & random window ids
	 */
	for (counter=0; counter<1000; counter++) {
		fbui_errno = FBUI_SUCCESS;
		win->command[0] = win->id;
		win->command[1] = rand();
		win->command[2] = counter & 31;
		for (i=3; i<LIBFBUI_COMMANDBUFLEN; i++)
			win->command[i] = rand();
		ioctl (dpy->fd, FBIO_UI_EXEC, (void*) win->command);
	}

	/* Test G: zero'd control operations
	 */
	for (counter=0; counter<1000; counter++) {
		struct fbui_ctrl ctl;
		memset (&ctl, 0, sizeof (struct fbui_ctrl));

		if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0)
			TESTFAIL("all zeroes control operation rejected")
	}

	/* Test H: 0xff'd control operations
	 */
	for (counter=0; counter<1000; counter++) {
		struct fbui_ctrl ctl;
		memset (&ctl, 0xff, sizeof (struct fbui_ctrl));

		if (!ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl))
			TESTFAIL("all 0xff's control operation accepted")
	}

	/* Test I: random control operations
	 */
	for (counter=0; counter<1000; counter++) {
		struct fbui_ctrl ctl;
		memset (&ctl, 0xff, sizeof (struct fbui_ctrl));
		char *ptr = (char*) &ctl;

		for (i=0; i<sizeof (struct fbui_ctrl); i++)
			*ptr++ = rand();

		ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl);
	}

	/*---------------*/
	printf ("OK\n");

quit:
	fbui_display_close (dpy);
}

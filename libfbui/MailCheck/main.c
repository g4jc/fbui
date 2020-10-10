
/*=========================================================================
 *
 * fbcheck, a POP3 mail checker for FBUI (in-kernel framebuffer UI)
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
 * 03 Sep 05: lack of config file now doesn't cause program to quit
 */

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

/* XPM */
static char * mail_xpm[] = {
"24 32 40 1",
" 	c #FFFFFF",
".	c #EAEAEA",
"+	c #C9C9C9",
"@	c #9E9E9E",
"#	c #000000",
"$	c #C3C3C3",
"%	c #D7D7D7",
"&	c #F4F4F4",
"*	c #C30808",
"=	c #F1ABAB",
"-	c #A8A8A8",
";	c #F9D8D8",
">	c #FCEBEB",
",	c #E87474",
"'	c #E35252",
")	c #E97979",
"!	c #E86D6D",
"~	c #F7CBCB",
"{	c #848484",
"]	c #ECECEC",
"^	c #F3B6B6",
"/	c #E4E4E4",
"(	c #BABABA",
"_	c #FBE5E5",
":	c #D1D1D1",
"<	c #909090",
"[	c #4736CE",
"}	c #BDB6ED",
"|	c #EDECFA",
"1	c #6C5DD8",
"2	c #5645D2",
"3	c #1E08C3",
"4	c #E5E3F8",
"5	c #B0A9EA",
"6	c #4E3CD0",
"7	c #BCBAC9",
"8	c #E7E7E7",
"9	c #B1B1B1",
"0	c #D9D9D9",
"a	c #FCFCFC",
"                        ",
"                        ",
"                .+@@    ",
"               #######. ",
"             .#@     .#+",
"            $#        +#",
"           %#&         #",
"           #        *= #",
"          -#       **; #",
"         +#      >**,> #",
"        +#       >*'   #",
"       .#        >*)   #",
"       #         >*)   #",
"      #.         >*)   #",
"     -#          >*)   #",
"    +#.          >*!   #",
"   +#######      ~*!   #",
"  .#+.  %+{#]    ^*    #",
" /#.       (#/   __   #.",
" #:         <#       #+ ",
" #          {#      +#  ",
" #          +#     .#   ",
" #          +#     #    ",
" # [}[|1 22 +#    #.    ",
" # 23233422 +#   +#     ",
" # 2 23532637#  +#      ",
".#          +# .#       ",
".#          +# #        ",
".#          +#-#        ",
".#8.........9##         ",
"0#############          ",
"a]                      "};

int
main(int argc, char** argv)
{
	Display *dpy;
	Window *win;
	u32 fg,bg;
	char *server, *user, *pass;

	char read_failed = 0;

	char tmp[300];
	sprintf (tmp, "%s/.fbcheckrc", getenv("HOME"));
	FILE *f = fopen (tmp,"r");
	if (!f)
		read_failed = 1;
	else
	if (!fgets (tmp, 300, f))
		read_failed = 1;
	else {
		server = strdup(tmp);
		if (!fgets (tmp, 300, f))
			read_failed = 1;
		else {
			user = strdup(tmp);
			if (!fgets (tmp, 300, f))
				read_failed = 1;
			else
				pass = strdup(tmp);
		}
	}

	fg = RGB_YELLOW;
	bg = RGB_BROWN;

	short win_w, win_h;
	dpy = fbui_display_open ();
	if (!dpy) 
		FATAL ("cannot open display");

	Font *pcf = Font_new ();
	if (!pcf_read (pcf, "timR10.pcf")) {
		Font_free (pcf);
		FATAL ("cannot load font");
	}
	short line_height = pcf->ascent + pcf->descent;

	win = fbui_window_open (dpy, 150,50, &win_w, &win_h, 250, 50,
		300, -1, 
		&fg, &bg, 
		"fbcheck", "", 
		FBUI_PROGTYPE_LAUNCHER, false,false, -1,false, 
		false, false, NULL, argc,argv);
	if (!win) 
		FATAL ("cannot create window");

	if (!fbui_xpm_to_icon (dpy, win, mail_xpm))
		printf ("fbcheck: failed to set icon\n");

	time_t t0 = time(NULL);
	time_t t = t0 + 999;

	bool need_redraw = true;

	while (1) {
		int total;
		Event ev;

		usleep(250000);

		// Check mail only every 5 minutes.
		if ((t - t0) >= 300) {
			if (!read_failed)
				total = checkmail (server,user,pass);
			t0 = t = time(NULL);
			need_redraw=true;
		}

		int err;
		if (err = fbui_poll_event (dpy, &ev,
                                FBUI_EVENTMASK_ALL - FBUI_EVENTMASK_KEY))
		{
			if (err != FBUI_ERR_NOEVENT)
				fbui_print_error (err);
			continue;
		}
//printf ("%s got event %d (%s)\n", argv[0], ev.type,fbui_get_event_name (ev.type));

		int num = ev.type;

		switch (num) {
		case FBUI_EVENT_EXPOSE:
			need_redraw = true;
			break;
		
		case FBUI_EVENT_MOVERESIZE:
			if (ev.key & FBUI_SIZE_CHANGED) {
				win_w = ev.width;
				win_h = ev.height;
				need_redraw=true;
			}
			if (ev.key & FBUI_CONTENTS_CHANGED)
				need_redraw=true;
			break;
		
		}

		if (!need_redraw)
			continue;

		int underline = 0;
		char text[200];
		if (total < 0) {
			sprintf (text, "Mailserver unreachable");
		} else if (total > 0) {
			sprintf (text, "Messages: <<%d>>", total);
			underline = 1;
		} else {
			sprintf (text, "~ No messages ~");
		}

		short w,a,d;
		Font_string_dims(pcf,text,&w,&a,&d);

		int x, y;
		x = (win_w - w) / 2;
		if (d < 0)
			d = -d;
		y = (win_h - (a+d)) / 2;

		fbui_clear (dpy, win);
		fbui_draw_string (dpy, win, pcf, x,y, text, fg);
		if (underline)
			fbui_draw_hline (dpy, win, x, x+w-1, 2 + y + pcf->ascent, fg);
		fbui_flush (dpy, win);
	}

	fbui_window_close(dpy, win);
	fbui_display_close (dpy);

	free(server);
	free(user);
	free(pass);
}

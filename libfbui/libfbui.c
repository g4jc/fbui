
/*=========================================================================
 *
 * libfbui, a library for accessing FBUI (in-kernel framebuffer GUI).
 * Copyright (C) 2003-2005 Zachary Smith, fbui@comcast.net
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#define timespec linux_timespec
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/vt.h>
#include <signal.h>
#define sigset_t linux_sigset_t
#define timeval linux_timeval
#define itimerval linux_itimerval
#include <linux/input.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/param.h>
#include <asm/errno.h> // EFAULT etc


#include "libfbui.h"


static struct fb_fix_screeninfo fi;
static struct fb_var_screeninfo vi;
static struct fbui_open oi;

#define ERRLOG "/tmp/fbui.log"


int dpi;
int fbui_errno;
int fbui_errloc;


/*-----------------------------------------------------------------------------
 * Changes:
 *
 * 23 Sep 04 ZS: added SIGTERM handler
 * 24 Sep 04 ZS: added commandline processing of console# (-c)
 * 25 Sep 04 ZS: added read_point
 * 25 Sep 04 ZS: added errlog
 * 26 Sep 04 ZS: added commandline processing of geometry (-geo)
 * 26 Sep 04 ZS: more signals supported
 * 01 Oct 04 ZS: updated for new use of control ioctl
 * 14 Oct 04 ZS: added support for fbui-input events 
 * 14 Oct 04 ZS: added keycode->char conversion
 * 22 Oct 04 ZS: fbui_open now ensures dims are available
 * 26 Oct 04 ZS: added -fg and -bg
 * 18 Jun 05 ZS: added fbui_errlog
 * 18 Jul 05 ZS: fixed fbui_get_position bug
 * 25 Jul 05 ZS: added fbui_draw_dialog (from fbview project)
 * 04 Sep 05 ZS: added support for more signal types.
 * 04 Sep 05 ZS: added support for image-draw of several pixel formats.
 * 07 Sep 05 ZS: added support for set-get icon.
 * 07 Sep 05 ZS: added fbui_xpm_to_icon
 * 24 Sep 05 ZS: added support for SIGINT.
 *
 *----------------------------------------------------------------------------*/


Display *my_dpy = NULL;
int display_fd;


static void
errlog (char *s, char *s2)
{
	FILE *f=fopen(ERRLOG,"a"); 
	if (!f) 
		return;
	fprintf(f, "%s(): %s\n", s,s2); 
	//fprintf(f, "\tsystem error(%d): %s\n",  errno, strerror(errno));
	fclose(f);
}




static char errname[24];
char *fbui_error_name (int value)
{
	char *s = NULL;

	if (value >= 0)
		value = -value;

	switch (value) {
	case FBUI_ERR_BADADDR: s = "bad address"; break;
	case FBUI_ERR_NULLPTR: s = "null pointer"; break;
	case FBUI_ERR_OFFSCREEN: s = "offscreen"; break;
	case FBUI_ERR_NOTRUNNING: s = "not running"; break;
	case FBUI_ERR_WRONGVISUAL: s = "wrong visual"; break;
	case FBUI_ERR_NOTPLACED: s = "not placed"; break;
	case FBUI_ERR_BIGENDIAN: s = "big endian"; break;
	case FBUI_ERR_INVALIDCMD: s = "invalid command"; break;
	case FBUI_ERR_BADPID: s = "bad process id"; break;
	case FBUI_ERR_ACCELBUSY: s = "accelerator key in use"; break;
	case FBUI_ERR_NOMEM: s = "out of memory"; break;
	case FBUI_ERR_NOTOPEN: s = "not open"; break;
	case FBUI_ERR_OVERLAP: s = "window overlap"; break;
	case FBUI_ERR_ALREADYOPEN: s = "already open"; break;
	case FBUI_ERR_MISSINGWIN: s = "missing window"; break;
	case FBUI_ERR_NOTWM: s = "not a window manager"; break;
	case FBUI_ERR_WRONGWM: s = "wrong window manager"; break;
	case FBUI_ERR_HAVEWM: s = "already have window manager"; break;
	case FBUI_ERR_KEYFOCUSDENIED: s = "key focus request denied"; break;
	case FBUI_ERR_KEYFOCUSERR: s = "key focus error"; break;
	case FBUI_ERR_BADPARAM: s = "bad parameter"; break;
	case FBUI_ERR_NOMOUSE: s = "no mouse"; break;
	case FBUI_ERR_MOUSEREAD: s = "mouse read error"; break;
	case FBUI_ERR_OVERLARGECUT: s = "overlarge cut"; break;
	case FBUI_ERR_BADWIN: s = "bad window"; break;
	case FBUI_ERR_PASTEFAIL: s = "paste failed"; break;
	case FBUI_ERR_CUTFAIL: s = "cut failed"; break;
	case FBUI_ERR_NOEVENT: s = "no events"; break;
	case FBUI_ERR_DRAWING: s = "busy drawing"; break;
	case FBUI_ERR_MISSINGPROCENT: s = "missing process entry"; break;
	case FBUI_ERR_BADVC: s = "bad virtual console number"; break;
	case FBUI_ERR_INTERNAL: s = "internal error"; break;
	case FBUI_ERR_WINDELETED: s = "window deleted by window manager"; break;
	case FBUI_ERR_BADCMD: s = "bad command number"; break;
	case -ENOMEM: s = "out of memory"; break;
	case -EFAULT: s = "segmentation fault"; break;
	default: s = "(other)"; 
	}

	if (!s) {
		sprintf (errname, "#%d", value);
		s = errname;
	}
	return s;
}

static void
fbui_errlog (char *s)
{
	FILE *f=fopen(ERRLOG,"a"); 
	if (!f) 
		return;
	char *s2 = fbui_error_name (fbui_errno);
	fprintf(f, "%s(): %s (kernel module location id %d)\n", s,s2, fbui_errloc); 
	fclose(f);
}
/*----------------------------------------------------------------------------*/


void 
fbui_update_error_loc (Display *dpy)
{
	static struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_ERRNO;
	ctl.id = dpy ? (dpy->list ? dpy->list->id : -1) : -1;
	int tmp = ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl);
	fbui_errno =tmp & 255;
	fbui_errloc = tmp >> 8;
}

void fbui_print_error (int value)
{
	printf ("FBUI error: %s (kernel module location %d)\n", fbui_error_name(value), fbui_errloc);
}

char fbui_get_active_console (Display *dpy)
{
	if (!dpy)
		return -1;

	static struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_GETCONSOLE;
	ctl.id = -1;
	return ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl);
}

char fbui_get_console (Display *dpy)
{
	if (!dpy)
		return -1;

	if (dpy->console < 0) 
		dpy->console = fbui_get_active_console(dpy);

	return dpy->console;
}


int
fbui_flush (Display *dpy, Window *win)
{
	fbui_errno = FBUI_SUCCESS;
	fbui_errloc = 0;

	if (!dpy || !win) 
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/

	if (win->command_ix <= 2)
		return 0;

	win->command[0] = win->id;
	win->command[1] = win->command_ix-2;

	if (win->command[1]) {
		if (ioctl (dpy->fd, FBIO_UI_EXEC, (void*) win->command) < 0) {
			//fbui_errno = -errno;
			fbui_update_error_loc (dpy);
		}
	}

	if (fbui_errno == FBUI_ERR_WINDELETED)
		win->deleted = true;

	win->command[0] = win->id;
	win->command[1] = 0;
	win->command_ix = 2;
	return fbui_errno;
}

static int
check_flush (Display *dpy, Window *win, int need)
{
	unsigned short nwords;

	if (!dpy || !win) 
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	nwords = win->command_ix-2;
	if (nwords + need >= LIBFBUI_COMMANDBUFLEN) {
		fbui_flush (dpy,win);
	}
	return 0;
}



/* called only by wm */
int 
fbui_window_info (Display* dpy, Window *wm, struct fbui_wininfo* info, int ninfo)
{
	int count;

	if (!dpy || !wm || !info || ninfo<=0) 
		return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;
	fbui_errloc = 0;

	static struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_WININFO;
	ctl.id = wm->id;
	ctl.info = info;
	ctl.ninfo = ninfo;

	count = ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl);
	if (count < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return count < 0 ? fbui_errno : count;
}

int 
fbui_accelerator (Display* dpy, Window *wm, short key, short op)
{
	if (!dpy || !wm)  
		return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_ACCEL;
	ctl.id = wm->id;
	ctl.x = key;
	ctl.y = op;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}



int 
fbui_set_icon (Display* dpy, Window *win, unsigned long *data)
{
	if (!dpy || !win || !data)
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_ICON;
	ctl.id = win->id;
	ctl.pointer = (void*) data;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}


int 
fbui_get_icon (Display* dpy, Window *win, int id, unsigned long *data)
{
	if (!dpy || !win || !data)
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_GETICON;
	ctl.id = win->id;
	ctl.id2 = id;
	ctl.pointer = (void*) data;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}

int 
fbui_cut (Display* dpy, Window *win, unsigned char *data, unsigned long length)
{
	if (!dpy || !win || !data || !length) 
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_CUT;
	ctl.id = win->id;
	ctl.pointer = data;
	ctl.cutpaste_length = length;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}


int 
fbui_paste (Display* dpy, Window *win, unsigned char *data, unsigned long max)
{
	if (!dpy || !win || !data || !max) 
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_PASTE;
	ctl.id = win->id;
	ctl.pointer = data;
	ctl.cutpaste_length = max;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}


long
fbui_cut_length (Display* dpy, Window *win)
{
	if (!dpy || !win)
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_CUTLENGTH;
	ctl.id = win->id;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}


/* 1 => force all windows to be auto placed
 */
/* called only by wm */
int 
fbui_placement (Display* dpy, Window *wm, int yes)
{
	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_PLACEMENT;
	ctl.id = wm->id;
	ctl.x = yes;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}


/* called only by wm */
int
fbui_redraw (Display *dpy, Window *wm, short win)
{
	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_REDRAW;
	ctl.id = wm->id;
	ctl.id2 = win;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}



/* wm param may be NULL */
int
fbui_move_resize (Display *dpy, Window *wm, short id, short x, short y, short w, short h)
{
	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_MOVERESIZE;
	ctl.id = wm ? wm->id : id;
	ctl.id2 = wm ? id : -1;
	ctl.x = x;
	ctl.y = y;
	ctl.width = w;
	ctl.height = h;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}


/* wm param may be NULL */
int
fbui_raise (Display *dpy, Window *wm, short id)
{
	Window *win;

	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	win = dpy->list;
	while (win && win->id != id)
		win = win->next;

	if (!win) 
		FATAL("invalid window id");

	fbui_flush (dpy,win);

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_RAISE;
	ctl.id = wm ? wm->id : id;
	ctl.id2 = wm ? id : -1;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}

/* wm param may be NULL */
int
fbui_lower (Display *dpy, Window *wm, short id)
{
	Window *win;

	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	win = dpy->list;
	while (win && win->id != id)
		win = win->next;

	if (!win) 
		FATAL("invalid window id");

	fbui_flush (dpy,win);

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_LOWER;
	ctl.id = wm ? wm->id : id;
	ctl.id2 = wm ? id : -1;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}

/* called only by wm */
int
fbui_delete (Display *dpy, Window *wm, short id)
{
	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_DELETE;
	ctl.id = wm->id;
	ctl.id2 = id;

	int rv=1;
	do {
		rv = ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long)&ctl);
/*		if (rv < 0 && -errno != FBUI_ERR_DRAWING) {
			fbui_errno = -errno;
			fbui_update_error_loc (dpy);
			break;
		}
*/
	}
	while (rv);

	return fbui_errno;
}

/* called only by wm */
int
fbui_assign_keyfocus (Display *dpy, Window *wm, short id)
{
	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_ASSIGN_KEYFOCUS;
	ctl.id = wm->id;
	ctl.id2 = id;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long)&ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}

/* called only by wm */
int
fbui_assign_ptrfocus (Display *dpy, Window *wm, short id)
{
	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_ASSIGN_PTRFOCUS;
	ctl.id = wm->id;
	ctl.id2 = id;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long)&ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}

/* called only by wm */
int
fbui_hide (Display *dpy, Window *wm, short id)
{
	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_HIDE;
	ctl.id = wm ? wm->id : id;
	ctl.id2 = wm ? id : -1;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long)&ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}

/* called only by wm */
int
fbui_unhide (Display *dpy, Window *wm, short id)
{
	if (!dpy) return -1;
	/*---------------*/

	fbui_errno = FBUI_SUCCESS;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_UNHIDE;
	ctl.id = wm ? wm->id : id;
	ctl.id2 = wm ? id : -1;

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long)&ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}


int
fbui_draw_point (Display *dpy, Window *win, short x, short y, unsigned long color)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,5))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_POINT;
	win->command [win->command_ix++] = x;
	win->command [win->command_ix++] = y;
	win->command [win->command_ix++] = color;
	win->command [win->command_ix++] = color>>16;

	return 0;
}

unsigned long
fbui_read_point (Display *dpy, Window *win, short x, short y)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,3))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_READPOINT;
	win->command [win->command_ix++] = x;
	win->command [win->command_ix++] = y;

	return fbui_flush (dpy, win);
}

int
fbui_draw_vline (Display *dpy, Window *win, short x, short y0, short y1, unsigned long color)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,6))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_VLINE;
	win->command [win->command_ix++] = x;
	win->command [win->command_ix++] = y0;
	win->command [win->command_ix++] = color;
	win->command [win->command_ix++] = color>>16;
	win->command [win->command_ix++] = y1;

	return 0;
}


int
fbui_draw_hline (Display *dpy, Window *win, short x0, short x1, short y, unsigned long color)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,6))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_HLINE;
	win->command [win->command_ix++] = x0;
	win->command [win->command_ix++] = y;
	win->command [win->command_ix++] = color;
	win->command [win->command_ix++] = color>>16;
	win->command [win->command_ix++] = x1;

	return 0;
}


int
fbui_fill_triangle (Display *dpy, Window *win, 
		    short x0, short y0, 
		    short x1, short y1, 
		    short x2, short y2, 
		    unsigned long color)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,9))
		return fbui_errno;
	win->command [win->command_ix++] = FBUI_TRIANGLE;
	win->command [win->command_ix++] = x0;
	win->command [win->command_ix++] = y0;
	win->command [win->command_ix++] = x1;
	win->command [win->command_ix++] = y1;
	win->command [win->command_ix++] = color;
	win->command [win->command_ix++] = color>>16;
	win->command [win->command_ix++] = x2;
	win->command [win->command_ix++] = y2;
	return 0;
}


int
fbui_draw_line (Display *dpy, Window *win, short x0, short y0, short x1, short y1, unsigned long color)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,7))
		return fbui_errno;
	win->command [win->command_ix++] = FBUI_LINE;
	win->command [win->command_ix++] = x0;
	win->command [win->command_ix++] = y0;
	win->command [win->command_ix++] = x1;
	win->command [win->command_ix++] = y1;
	win->command [win->command_ix++] = color;
	win->command [win->command_ix++] = color>>16;
	return 0;
}


int 
fbui_set_subtitle (Display *dpy, Window *win, char *str)
{
	struct fbui_ctrl ctl;

	if (!dpy || !win || !str)
		return FBUI_ERR_NULLPTR;
	if (win->deleted) 
		return -1;

	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_SUBTITLE;
	ctl.id = win->id;
	strncpy (ctl.string, str, FBUI_NAMELEN);

	if (ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl) < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
	}

	return fbui_errno;
}


int
fbui_clear (Display *dpy, Window *win)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,1))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_CLEAR;

	return 0;
}

int
fbui_draw_rect (Display *dpy, Window *win, short x0, short y0, short x1, short y1, unsigned long color)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,7))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_RECT;
	win->command [win->command_ix++] = x0;
	win->command [win->command_ix++] = y0;
	win->command [win->command_ix++] = x1;
	win->command [win->command_ix++] = y1;
	win->command [win->command_ix++] = color;
	win->command [win->command_ix++] = color>>16;

	return 0;
}

int
fbui_fill_rect (Display *dpy, Window *win, short x0, short y0, short x1, short y1, unsigned long color)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,7))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_FILLRECT;
	win->command [win->command_ix++] = x0;
	win->command [win->command_ix++] = y0;
	win->command [win->command_ix++] = x1;
	win->command [win->command_ix++] = y1;
	win->command [win->command_ix++] = color;
	win->command [win->command_ix++] = color>>16;

	return 0;
}

int
fbui_clear_area (Display *dpy, Window *win, short x0, short y0, short x1, short y1)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,5))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_CLEARAREA;
	win->command [win->command_ix++] = x0;
	win->command [win->command_ix++] = y0;
	win->command [win->command_ix++] = x1;
	win->command [win->command_ix++] = y1;

	return 0;
}


int
fbui_copy_area (Display *dpy, Window *win, short xsrc, short ysrc, short xdest, short ydest, short w, short h)
{
	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,7))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_COPYAREA;
	win->command [win->command_ix++] = xsrc;
	win->command [win->command_ix++] = ysrc;
	win->command [win->command_ix++] = xsrc + w - 1;
	win->command [win->command_ix++] = ysrc + h - 1;
	win->command [win->command_ix++] = xdest;
	win->command [win->command_ix++] = ydest;

	return 0;
}


int
fbui_put_image (Display *dpy, Window *win, char type, short x, short y, 
                     short wid, short ht, unsigned char *p)
{
	if (!dpy || !win || !p) 
		return -1;
	if (win->deleted) 
		return -1;
	if (type < 0 || type > FB_IMAGETYPE_GREY)
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,8))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_FULLIMAGE;
	win->command [win->command_ix++] = x;
	win->command [win->command_ix++] = y;
	win->command [win->command_ix++] = (unsigned long)p;
	win->command [win->command_ix++] = ((unsigned long)p)>>16;
	win->command [win->command_ix++] = type;
	win->command [win->command_ix++] = wid;
	win->command [win->command_ix++] = ht;
	fbui_flush (dpy, win); // the caller's pointer may not last forever

	return 0;
}


int
fbui_put_image_mono (Display *dpy, Window *win, short x, short y, 
                     short wid, short ht, unsigned char *p, unsigned long color)
{
	if (!dpy || !win || !p) 
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,9))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_MONOIMAGE;
	win->command [win->command_ix++] = x;
	win->command [win->command_ix++] = y;
	win->command [win->command_ix++] = (unsigned long)p;
	win->command [win->command_ix++] = ((unsigned long)p)>>16;
	win->command [win->command_ix++] = wid;
	win->command [win->command_ix++] = ht;
	win->command [win->command_ix++] = color;
	win->command [win->command_ix++] = color>>16;

	return 0;
}


int
fbui_put_image_partial (Display *dpy, Window *win, char type, short x, short y, 
                short wid, short ht, 
		short xstart, short ystart, short xend, short yend,
		unsigned char *p)
{
	if (!dpy || !win || !p) 
		return -1;
	if (win->deleted) 
		return -1;
	if (type < 0 || type > FB_IMAGETYPE_GREY)
		return -1;
	/*---------------*/
	if (check_flush (dpy, win,12))
		return fbui_errno;

	win->command [win->command_ix++] = FBUI_IMAGE;
	win->command [win->command_ix++] = x;
	win->command [win->command_ix++] = y;
	win->command [win->command_ix++] = (unsigned long)p;
	win->command [win->command_ix++] = ((unsigned long)p)>>16;
	win->command [win->command_ix++] = type;
	win->command [win->command_ix++] = wid;
	win->command [win->command_ix++] = ht;
	win->command [win->command_ix++] = xstart;
	win->command [win->command_ix++] = ystart;
	win->command [win->command_ix++] = xend;
	win->command [win->command_ix++] = yend;

	fbui_flush (dpy, win); // the caller's pointer may not last forever

	return 0;
}


int
fbui_window_close (Display *dpy, Window *win)
{
	int r;

	if (!dpy || !win) return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/
	// fbui_flush (dpy, win);

	r = ioctl (dpy->fd, FBIO_UI_CLOSE, win->id);

	Window *prev = NULL;
	Window *ptr = dpy->list;
	while (ptr) {
		if (win == ptr)
			break;
		prev = ptr;
		ptr = ptr->next;
	}
	if (ptr) {
		if (!prev)
			dpy->list = win->next;
		else
			prev->next = win->next;
	}

	free (win);
	return r;
}



// key info that comes from FBUI : bits 0-1= state 2-15=keycode
int
fbui_convert_key (Display *dpy, long keyinfo)
{
	int key,state;
	unsigned short ch=0;

	if (!dpy) return -1;
	/*---------------*/

	state = 3 & keyinfo;
	key = keyinfo >> 2;

	switch (key) {
	case KEY_RIGHTSHIFT:
	case KEY_LEFTSHIFT: 
		dpy->shift = state>0;
		return 0;

	case KEY_RIGHTCTRL:
	case KEY_LEFTCTRL: 
		dpy->ctrl = state>0;
		return 0;

	case KEY_RIGHTALT:
	case KEY_LEFTALT: 
		dpy->alt = state>0;
		return 0;
	}

	if (state==0)
		return 0;

	switch (key) {
	case KEY_ENTER: ch = '\n'; break;
	case KEY_ESC: ch = 27; break;
	case KEY_SPACE: ch = ' '; break;
	case KEY_MINUS: ch = !dpy->shift ? '-' : '_'; break;
	case KEY_EQUAL: ch = !dpy->shift ? '=' : '+'; break;
	case KEY_BACKSPACE: ch = 8; break;
	case KEY_SEMICOLON: ch = !dpy->shift ? ';' : ':'; break;
	case KEY_COMMA: ch = !dpy->shift ? ',' : '<'; break;
	case KEY_DOT: ch = !dpy->shift ? '.' : '>'; break;
	case KEY_GRAVE: ch = !dpy->shift ? '`' : '~'; break;
	case KEY_BACKSLASH: ch = !dpy->shift ? '\\' : '|'; break;
	case KEY_LEFTBRACE: ch = !dpy->shift ? '[' : '{'; break;
	case KEY_RIGHTBRACE: ch = !dpy->shift ? ']' : '}'; break;
	case KEY_APOSTROPHE: ch = !dpy->shift ? '\'' : '"'; break;
	case KEY_SLASH: ch = !dpy->shift ? '/' : '?'; break;
	case KEY_TAB: ch = !dpy->shift ? '\t' : FBUI_LEFTTAB; break;
	case KEY_A: ch = 'a'; break;
	case KEY_B: ch = 'b'; break;
	case KEY_C: ch = 'c'; break;
	case KEY_D: ch = 'd'; break;
	case KEY_E: ch = 'e'; break;
	case KEY_F: ch = 'f'; break;
	case KEY_G: ch = 'g'; break;
	case KEY_H: ch = 'h'; break;
	case KEY_I: ch = 'i'; break;
	case KEY_J: ch = 'j'; break;
	case KEY_K: ch = 'k'; break;
	case KEY_L: ch = 'l'; break;
	case KEY_M: ch = 'm'; break;
	case KEY_N: ch = 'n'; break;
	case KEY_O: ch = 'o'; break;
	case KEY_P: ch = 'p'; break;
	case KEY_Q: ch = 'q'; break;
	case KEY_R: ch = 'r'; break;
	case KEY_S: ch = 's'; break;
	case KEY_T: ch = 't'; break;
	case KEY_U: ch = 'u'; break;
	case KEY_V: ch = 'v'; break;
	case KEY_W: ch = 'w'; break;
	case KEY_X: ch = 'x'; break;
	case KEY_Y: ch = 'y'; break;
	case KEY_Z: ch = 'z'; break;
	case KEY_0: ch = '0'; break;
	case KEY_1: ch = '1'; break;
	case KEY_2: ch = '2'; break;
	case KEY_3: ch = '3'; break;
	case KEY_4: ch = '4'; break;
	case KEY_5: ch = '5'; break;
	case KEY_6: ch = '6'; break;
	case KEY_7: ch = '7'; break;
	case KEY_8: ch = '8'; break;
	case KEY_9: ch = '9'; break;

	/* special chars section */
	case KEY_UP: ch = FBUI_UP; break;
	case KEY_DOWN: ch = FBUI_DOWN; break;
	case KEY_LEFT: ch = FBUI_LEFT; break;
	case KEY_RIGHT: ch = FBUI_RIGHT; break;

	case KEY_INSERT: ch = FBUI_INS; break;
	case KEY_DELETE: ch = FBUI_DEL; break;
	case KEY_HOME: ch = FBUI_HOME; break;
	case KEY_END: ch = FBUI_END; break;
	case KEY_PAGEUP: ch = FBUI_PGUP; break;
	case KEY_PAGEDOWN: ch = FBUI_PGDN; break;
	case KEY_SCROLLLOCK: ch = FBUI_SCRLK; break;
	case KEY_NUMLOCK: ch = FBUI_NUMLK; break;
	case KEY_CAPSLOCK: ch = FBUI_CAPSLK; break;

	case KEY_PRINT: ch = FBUI_PRTSC; break;

	case KEY_F1: ch = FBUI_F1; break;
	case KEY_F2: ch = FBUI_F2; break;
	case KEY_F3: ch = FBUI_F3; break;
	case KEY_F4: ch = FBUI_F4; break;
	case KEY_F5: ch = FBUI_F5; break;
	case KEY_F6: ch = FBUI_F6; break;
	case KEY_F7: ch = FBUI_F7; break;
	case KEY_F8: ch = FBUI_F8; break;
	case KEY_F9: ch = FBUI_F9; break;
	case KEY_F10: ch = FBUI_F10; break;
	case KEY_F11: ch = FBUI_F11; break;
	case KEY_F12: ch = FBUI_F12; break;

	/* mouse buttons */
	case BTN_LEFT: ch = FBUI_BUTTON_LEFT; break;
	case BTN_RIGHT: ch = FBUI_BUTTON_RIGHT; break;
	case BTN_MIDDLE: ch = FBUI_BUTTON_MIDDLE; break;
	}

	if (dpy->ctrl && isalpha(ch))
		ch &= 31;
	else
	if (dpy->shift && isalpha(ch))
		ch = toupper(ch);

	if (dpy->shift && isdigit(ch)) {
		switch (ch) {
		case '0': ch = ')'; break;
		case '1': ch = '!'; break;
		case '2': ch = '@'; break;
		case '3': ch = '#'; break;
		case '4': ch = '$'; break;
		case '5': ch = '%'; break;
		case '6': ch = '^'; break;
		case '7': ch = '&'; break;
		case '8': ch = '*'; break;
		case '9': ch = '('; break;
		}
	}

	return ch;
}


int
fbui_poll_event (Display *dpy, Event *e, unsigned short mask)
{
	Window *win;
	short win_id;
	long retval;

	if (!dpy || !e) 
		return -1;
	if (!dpy->list) {
		WARNING ("program tried to receive events without first creating window(s)");
		return -1;
	}
	/*---------------*/

	win = dpy->list;
	while (win) {
		fbui_flush (dpy, win);
		win = win->next;
	}

	memset (e, 0, sizeof(Event));

	struct fbui_event event;
	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_POLLEVENT;
	ctl.id = dpy->list->id;
	ctl.x = (short)mask;
	ctl.event = &event;
	ctl.rects = &e->rects;
	retval = ioctl (dpy->fd, FBIO_UI_CONTROL, &ctl);

/*	if (retval < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
		//return -errno;
	}
*/
	e->has_rects = event.has_rects;
	e->type = event.type;
	e->id = win_id = event.id;
	e->key = event.key;
	e->x = event.x;
	e->y = event.y;
	e->width = event.width;
	e->height = event.height;

	win = dpy->list;
	while (win) {
		if (win->id == win_id)
			break;
		win = win->next;
	}
	e->win = win;

	if (!win)
		fprintf(stderr, "libfbui: cannot identify window for id %d\n", win_id);
	else
	if (event.type == FBUI_EVENT_MOVERESIZE) {
		win->width = event.width;
		win->height = event.height;
	}

	return 0;
}

int
fbui_wait_event (Display *dpy, Event *e, unsigned short mask)
{
	Window *win;

	if (!dpy || !e) 
		return -1;
	/*---------------*/

	win = dpy->list;
	while (win) {
		fbui_flush (dpy, win);
		win = win->next;
	}

	memset (e, 0, sizeof(Event));

	struct fbui_event event;
	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_WAITEVENT;
	ctl.id = dpy->list ? dpy->list->id : -1; /* window id not required but faster */
	ctl.x = (short)mask;
	ctl.event = &event;
	ctl.rects = &e->rects;
	int retval = ioctl (dpy->fd, FBIO_UI_CONTROL, &ctl);

/*	if (retval < 0) {
		fbui_errno = -errno;
		if (errno == FBUI_ERR_NOEVENT)
			fbui_errno = errno = 0;
		else
		fbui_update_error_loc (dpy);
		return -errno;
	}
*/
	e->has_rects = event.has_rects;
	e->type = event.type;
	short win_id = event.id;
	e->id = win_id;
	e->key = event.key;
	e->x = event.x;
	e->y = event.y;
	e->width = event.width;
	e->height = event.height;

	win = dpy->list;
	while (win) {
		if (win->id == win_id)
			break;
		win = win->next;
	}
	e->win = win;

	if (!win)
		fprintf(stderr, "libfbui: cannot identify window for id %d\n", win_id);
	else
	if (event.type == FBUI_EVENT_MOVERESIZE) {
		win->width = event.width;
		win->height = event.height;
	}

	return 0;
}

int
fbui_get_dims (Display *dpy, Window *win, short *width, short *height)
{
	long result;

	if (!dpy || !win || !width || !height) 
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/

	fbui_flush (dpy, win);

	*width = 0;
	*height = 0;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_GETDIMS;
	ctl.id = win->id;

	result = ioctl (dpy->fd, FBIO_UI_CONTROL, &ctl);
//printf ("getdims retval %d (%d)\n",result,-errno);
	if (result < 0) {
		//fbui_errno = -errno;
		fbui_update_error_loc (dpy);
		return fbui_errno;
	}

	*width = result >> 16;
	*height = result & 0xffff;

	win->width = *width;
	win->height = *height;

	return 0;
}


/* note: mouse buttons appear as keystrokes */
int
fbui_read_mouse (Display *dpy, Window *win, short *x, short *y)
{
	long result;

	if (!dpy || !win || !x || !y) 
		return -1;
	if (win->deleted) 
		return -1;
	/*---------------*/

	fbui_flush (dpy, win);

	*x = 0;
	*y = 0;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_READMOUSE;
	ctl.id = win->id;

	result = ioctl (dpy->fd, FBIO_UI_CONTROL, &ctl);
	if (result < 0) 
		return result;

	*x = result >> 16;
	*y = result & 0xffff;

	return 0;
}


int
fbui_get_position (Display *dpy, Window *win, short *x, short *y)
{
	unsigned long result;

	if (!dpy || !win || !x || !y) 
		return -1;
	if (win->deleted) 
		return 0;
	/*---------------*/

	fbui_flush (dpy, win);

	*x = -1;
	*y = -1;

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_GETPOSN;
	ctl.id = win->id;

	result = ioctl (dpy->fd, FBIO_UI_CONTROL, &ctl);
	if (result < 0) {
		fbui_errno = -result;
		fbui_update_error_loc (dpy);
		fbui_errlog(__FUNCTION__);
		return 0;
	}

	*x = result >> 16;
	*y = result & 0xffff;

	return 1;
}

int
fbui_get_window_at_xy (Display *dpy, Window *win, short x, short y)
{
	unsigned long result;

	if (!dpy || !win)
		return -1;
	if (win->deleted) 
		return 0;
	/*---------------*/

	struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_XYTOWINDOW;
	ctl.id = win->id;
	ctl.x = x;
	ctl.y = y;

	result = ioctl (dpy->fd, FBIO_UI_CONTROL, &ctl);
	if (result < 0 && result != -1) {
		fbui_errno = -result;
		fbui_update_error_loc (dpy);
		fbui_errlog(__FUNCTION__);
	}
	return result;
}

static void kill_handler (int type)
{
	if (my_dpy) {
		Window *win = my_dpy->list;
		while (win) {
			Window *next = win->next;
			fbui_window_close (my_dpy, win);
			win = next;
		}
		my_dpy->list = NULL;
		fbui_display_close (my_dpy);
	}

	char *s = NULL;
	switch (type) {
	case SIGTERM:	s = "terminated"; break;
	case SIGSEGV:	s = "segmentation fault"; break;
	case SIGFPE:	s = "floating-point exception"; break;
	case SIGBUS:	s = "bus error"; break;
	case SIGKILL:	s = "killed"; break;
	case SIGILL:	s = "illegal instruction"; break;
	case SIGSTKFLT:	s = "coprocessor stack fault"; break;
	}

	exit(-1);
}

static int hex_to_int (char h)
{
	h = tolower(h);
	if (h >= '0' && h <= '9')
		return h - '0';
	else
	if (h >= 'a' && h <= 'f')
		return 10 + h - 'a';
	else
		return 0;
}

static long parse_rgb (char *s)
{
	int len;
	long c;
	if (!s) return -1;
	len = strlen (s);
	if (len == 3) {
		c = hex_to_int (*s++); 
		c <<= 8;
		c |= hex_to_int (*s++); 
		c <<= 8;
		c |= hex_to_int (*s++); 
		c <<= 4;
		return c;
	}
	if (len == 6) {
		c = hex_to_int (*s++); 
		c <<= 4;
		c |= hex_to_int (*s++); 
		c <<= 4;
		c |= hex_to_int (*s++); 
		c <<= 4;
		c |= hex_to_int (*s++); 
		c <<= 4;
		c |= hex_to_int (*s++); 
		c <<= 4;
		c |= hex_to_int (*s++); 
		return c;
	}
	return -1;
}


static int
getline2 (FILE *f, char *buf, int buflen)
{
        int got;
        char ch;
        int ix=0;

        buf[0] = 0;

        while (ix < (buflen-1)) {
                got = fread (&ch, 1, 1, f);

                if (!got)
                        break;

                if (ch == '\n')
                        break;
                if (ch == '\r')
                        continue;

                buf [ix++] = ch;
        }

        buf [ix] = 0;
        return ix;
}

long
parse_colorname (char *s)
{
	if (!strcmp(s, "red"))
		return RGB_RED;
	if (!strcmp(s, "green"))
		return RGB_GREEN;
	if (!strcmp(s, "blue"))
		return RGB_BLUE;
	if (!strcmp(s, "black"))
		return 0;
	if (!strcmp(s, "white"))
		return 0xffffff;
	if (!strcmp(s, "steelblue"))
		return RGB_STEELBLUE;
	if (!strcmp(s, "sienna"))
		return RGB_SIENNA;
	if (!strcmp(s, "cyan"))
		return RGB_CYAN;
	if (!strcmp(s, "orange"))
		return RGB_ORANGE;
	if (!strcmp(s, "yellow"))
		return RGB_YELLOW;
	if (!strcmp(s, "magenta"))
		return RGB_MAGENTA;
	if (!strcmp(s, "purple"))
		return RGB_PURPLE;
	if (!strcmp(s, "brown"))
		return RGB_BROWN;
	if (!strcmp(s, "gray"))
		return RGB_GRAY;

	FILE *f = fopen ("/usr/X11/lib/X11/rgb.txt","r");
	char buffer [200];
	if (!f) return -1;
	while (getline2 (f, buffer, 199)) {
		int r,g,b;
		char name[100];
		if (4 == sscanf(buffer,"%u %u %u %s", &r, &g, &b, name)) {
			if (!strcmp (s, name)) {
				long color;
				r &= 0xff;
				g &= 0xff;
				b &= 0xff;
				color = r;
				color <<= 8;
				color |= g;
				color <<= 8;
				color |= b;
				fclose(f);
				return color;
			}
		}
	}
	fclose(f);

	return -1;
}


/* not static since it's used by fbterm */

int fbui_parse_geom (char *s1, short *w, short *h, short *xr, short *yr)
{
	short width=0,height=0,xrel=-1,yrel=-1;
	char *s2 = NULL;
	char *s3 = NULL;
	char *s4 = NULL;

	if (!s1 || !w || !h || !xr || !yr)
		return 0;
	/*---------------*/

	if (*s1 && isdigit(*s1)) {
		s2 = strchr (s1, 'x');
		if (!s2)
			return 0;
		*s2++ = 0;

		s3 = s2;
		while (*s3 && isdigit(*s3))
			s3++;

		if (*s3) {
			xrel = *s3=='-' ? -1 : 1;
			*s3++ = 0;

			s4 = s3;
			while (*s4 && isdigit(*s4))
				s4++;

			if (s4) {
				yrel = *s4=='-' ? -1 : 1;
				*s4++ = 0;
			}
		} else {
			xrel = 9999; /* no position given */
			s3 = NULL;
		}

		width = atoi(s1);
		height = atoi(s2);

		if (s3 && s4) {
			int a = atoi(s3);
			int b = atoi(s4);
			if (xrel == 1) {
				xrel = a;
			} else {
				xrel = -a - 1;
			}
			if (yrel == 1) {
				yrel = b;
			} else {
				yrel = -b - 1;
			}
		} else {
			xrel = 9999;
		}
		*w = width;
		*h = height;
		*xr = xrel;
		*yr = yrel;
		return 1;
	} else
		return 0;
}


void
fbui_display_close (Display* dpy)
{
	if (dpy) {
		Window *win = dpy->list;
		while (win) {
			fbui_window_close (dpy, win);
			win = win->next;
		}
		close (dpy->fd);
		free (dpy);
	}
}

Display*
fbui_display_open ()
{
	Display *dpy = NULL;
	int success=0;
	int fd;

	fbui_errno = 0;

	dpi = 75;

	if (my_dpy)
		FATAL ("attempt to re-init");

	fd = open ("/dev/fb0", O_RDWR );
	if (fd < 0)
		fd = open ("/dev/fb/0", O_RDWR );
	display_fd = fd;
	if (fd < 0)
	{
		success= 0;
		perror("open");
		errlog(__FUNCTION__, "Open failed");
	}
	else
	{
		if (ioctl (fd, FBIOGET_FSCREENINFO, &fi))
		{
			perror("ioctl");
			success= 0;
			errlog(__FUNCTION__, "ioctl1 failed");
		}
		else
		if (ioctl (fd, FBIOGET_VSCREENINFO, &vi))
		{
			perror("ioctl");
			success= 0;
			errlog(__FUNCTION__, "ioctl2 failed");
		}
		else
		{
			printf ("Framebuffer resolution: %dx%d, %dbpp\n", vi.xres, vi.yres, vi.bits_per_pixel);
			success = 1;
		}
	}

	if (success) {
		dpy = (Display*) malloc (sizeof(Display));
		if (!dpy) {
			fprintf (stderr, "out of memory\n");
			return NULL;
		} 

		my_dpy = dpy;
		memset ((void*)dpy, 0, sizeof(Display));
		dpy->fd = fd;

		dpy->width = vi.xres;
		dpy->height = vi.yres;
		dpy->depth = vi.bits_per_pixel;
		dpy->bytes_per_pixel = (7 + dpy->depth) >> 3;

		dpy->red_offset = vi.red.offset;
		dpy->green_offset = vi.green.offset;
		dpy->blue_offset = vi.blue.offset;
		dpy->red_length = vi.red.length;
		dpy->green_length = vi.green.length;
		dpy->blue_length = vi.blue.length;

		dpy->console = fbui_get_active_console(dpy);

		signal (SIGKILL, kill_handler);
		signal (SIGTERM, kill_handler);
		signal (SIGINT, kill_handler);
		signal (SIGSEGV, kill_handler);
		signal (SIGFPE, kill_handler);
		signal (SIGILL, kill_handler);
		signal (SIGBUS, kill_handler);
		signal (SIGSTKFLT, kill_handler);
	}
	return dpy;
}

Window*
fbui_window_open (Display *dpy,
	   short width, short height, 
	   short *width_return, short *height_return,
	   short max_width, short max_height,
	   short xrel, short yrel, 
	   unsigned long *fgcolor_inout, 
	   unsigned long *bgcolor_inout, 
	   char *name, char *subtitle, 
	   char program_type, 
	   char request_control, 
	   char doing_autopositioning,
	   char vc_, 
	   char need_keys,
	   char receive_all_motion,
	   char initially_hidden,
	   unsigned char *mask,
	   int argc, char **argv)
{
	int i;
	char vc=-1;
	long fgcolor;
	long bgcolor;
	Window *win = NULL;
	char force_type = -1;

	if (!dpy || !name || !subtitle || !width_return || !height_return || 
	    !fgcolor_inout || !bgcolor_inout)
	{
		WARNING ("null param");
		return NULL;
	}
	/*---------------*/
	fgcolor = *fgcolor_inout;
	bgcolor = *bgcolor_inout;

        i=1;
	if (argv)
        while(i < argc) {
		char *str = argv[i];
                if (!strncmp(str, "-type=",6)) {
			char *tstr = str+6;
			if (!strcmp (tstr, "app"))
				force_type = FBUI_PROGTYPE_APP;
			else if (strcmp (tstr, "launcher"))
				force_type = FBUI_PROGTYPE_LAUNCHER;
			else if (strcmp (tstr, "tool"))
				force_type = FBUI_PROGTYPE_TOOL;
			else if (strcmp (tstr, "emphemeral"))
				force_type = FBUI_PROGTYPE_EPHEMERAL;
			else if (strcmp (tstr, "none"))
				force_type = FBUI_PROGTYPE_NONE;
		}
                else if (!strncmp(str, "-geo",4)) {
			int ch = str[4];
			char *values=NULL;
			*str = 0;
			if (ch && isdigit(ch)) {
				values = str+4;
			} else {
				if (i != (argc-1)) {
					values = argv[++i];
					argv[i][0] = 0;
				}
			}
				
			if (values) {
				short w,h,xr,yr;
				if (fbui_parse_geom (values,&w,&h,&xr,&yr)) {
					width = w;
					height = h;
					if (xr != 9999) {
						xrel = xr;
						yrel = yr;
					}
				}
                        }
		}
		else
                if (!strncmp (str, "-bg", 3) || !strncmp(str, "-fg",3)) {
			char *s = 3 + str;
			char which = str[1];
			char ch = *s;
			long color = -1;
			*str = 0;
			if (ch == '=')
				ch = *++s;
			if (!ch) {
				s = argv[++i];
				ch = s ? *s : 0;
			}
			if (ch == '#') {
				color = parse_rgb (s+1);
			} 
			else if (ch) {
				color = parse_colorname (s);
			}
			if (color != -1) {
				if (which == 'f')
					fgcolor = color;
				else
					bgcolor = color;
			}
			argv[i][0] = 0;
		}
		else
                if (!strncmp(str, "-c",2)) {
                        char *s = 2 + str;
                        if (*s && isdigit(*s)) {
                                vc = atoi(s);
				*str = 0;
                        } else {
				*str = 0;
                                if (i != (argc-1)) {
                                        vc = atoi(argv[++i]);
					*argv[i] = 0;
                                } else { 
					fprintf(stderr, "Usage: %s [-cCONSOLE#] [-fgCOLOR] [-bgCOLOR]\n", argv[0]); 
				}
                        }
                }
                i++;
        }
	if (vc_ != -1 && vc == -1)
		vc = vc_;

	if (width > max_width)
		width = max_width;
	if (height > max_height)
		height = max_height;
	if (width < 1)
		return NULL;
	if (height < 1)
		return NULL;

	short x, y;
	x = xrel >= 0 ? xrel : vi.xres + xrel + 1 - width;
	y = yrel >= 0 ? yrel : vi.yres + yrel + 1 - height;

	if (x >= dpy->width || y >= dpy->height || x < 0 || y < 0)
		FATAL("window position is partly or fully off-screen");

	oi.req_control = request_control ? 1 : 0;
	oi.x = x;
	oi.y = y;
	oi.width = width;
	oi.height = height;
	oi.max_width = 	max_width;
	oi.max_height = max_height;
	oi.bgcolor =	bgcolor;
	oi.initially_hidden = initially_hidden;
	*fgcolor_inout = fgcolor;
	*bgcolor_inout = bgcolor;
	oi.desired_vc =	vc;
	oi.doing_autoposition =	doing_autopositioning ? 1 : 0;
	oi.need_keys =	need_keys ? 1 : 0;
	oi.receive_all_motion =	receive_all_motion ? 1 : 0;
	oi.program_type = force_type >= 0 ? force_type : program_type;
	oi.usermask = (__u32) mask;
	oi.maskwidth = width;
	oi.maskheight = height;
	strncpy (oi.name, name, FBUI_NAMELEN);
	strncpy (oi.subtitle, subtitle, FBUI_NAMELEN);

	int result = ioctl (dpy->fd, FBIO_UI_OPEN, &oi);
	if (result < 0)
	{
		fbui_errno = -result;
		fbui_update_error_loc (dpy);
		fbui_errlog(__FUNCTION__);
		return NULL;
	}

	win = (Window*) malloc (sizeof(Window));
	if (!win)
		FATAL ("out of memory");
	memset (win, 0, sizeof (Window));

	win->id = result;
	win->command_ix = 2;
	win->next = dpy->list;
	dpy->list = win;

	short w,h;
	int total_delay = 0;

#define PATIENCE_MAX 7
	while (fbui_get_dims (dpy, win, &w, &h)) {
		printf ("%s: waiting for window dimensions\n", __FUNCTION__);
		sleep (1);
		total_delay++;
		if (total_delay == PATIENCE_MAX) {
			WARNING ("ran out of patience waiting for window dimensions, quitting...");
			fbui_window_close (dpy, win);
			*width_return = 0;
			*height_return = 0;
			return NULL;
		}
	}

	*width_return = w;
	*height_return = h;

	return win;
}


int fbui_beep (Display *dpy, short pitch, short duration)
{
	if (!dpy || pitch <= 0 || duration <= 0)
		return -1;

	static struct fbui_ctrl ctl;
	memset (&ctl, 0, sizeof (struct fbui_ctrl));
	ctl.op = FBUI_BEEP;
	ctl.id = -1;
	ctl.x = pitch;
	ctl.y = duration;
	return ioctl (dpy->fd, FBIO_UI_CONTROL, (unsigned long) &ctl);
}


char *fbui_get_event_name (int type)
{
	char *s="(unknown)";
	switch (type) {
	case FBUI_EVENT_NONE: s="(none)"; break;
	case FBUI_EVENT_EXPOSE: s="Expose"; break;
	case FBUI_EVENT_HIDE: s="Hide"; break;
	case FBUI_EVENT_UNHIDE: s="Unhide"; break;
	case FBUI_EVENT_ENTER: s="Enter"; break;
	case FBUI_EVENT_LEAVE: s="Leave"; break;
	case FBUI_EVENT_KEY: s="Key"; break;
	case FBUI_EVENT_MOVERESIZE: s="MoveResize"; break;
	case FBUI_EVENT_ACCEL: s="Accel"; break;
	case FBUI_EVENT_WINCHANGE: s="WinChange"; break;
	case FBUI_EVENT_NAMECHANGE: s="NameChange"; break;
	case FBUI_EVENT_MOTION: s="Motion"; break;
	case FBUI_EVENT_BUTTON: s="Button"; break;
	case FBUI_EVENT_RAISE: s="Raise"; break;
	case FBUI_EVENT_LOWER: s="Lower"; break;
	}
	return s;
}



bool fbui_xpm_to_icon (Display *dpy, Window *win, char **xpm)
{
	unsigned long icon [FBUI_ICON_WIDTH * FBUI_ICON_HEIGHT];
	int i, j, k;
	char str[3];
	int w, h, ncolors, chars_per;

	if (4 != sscanf (xpm[0], "%d %d %d %d", &w, &h, &ncolors, &chars_per))
		return false;

	if (w != FBUI_ICON_WIDTH || h != FBUI_ICON_HEIGHT)
		return false;

	for (i=0; i<FBUI_ICON_WIDTH; i++) {
		for (j=0; j<FBUI_ICON_HEIGHT; j++) {
			char *pixels = xpm[1+ncolors+j];
			k = chars_per * i;
			str[0] = pixels[k];
			if (chars_per > 1)
				str[1] = pixels[k+1];
			else
				str[1] = 0;
			str[2] = 0;

			int l=1;
			while (l < 1+ncolors) {
				char str2[3];
				str2[0] = xpm[l][0];
				if (chars_per > 1)
					str2[1] = xpm[l][1];
				else
					str2[1] = 0;
				str2[2] = 0;
				if (!strcmp (str,str2)) {
					char *s = strchr (xpm[l]+3, '#');
					if (s) {
						unsigned long rgb;
						if (1 == sscanf(1+s, "%06lx", &rgb)) {
							icon[i+j*FBUI_ICON_WIDTH] = rgb;
							break;
						}
						else
							FATAL("bad xpm 1");
					}
				}
				else
					l++;
			}
			if (l == 1+ncolors)
				FATAL("bad xpm 2");
		}
	}
	fbui_set_icon (dpy,win,icon);

	return true;
}



/*=========================================================================
 *
 * Module name:		FBUI (FrameBufferUI)
 * Module purpose:	Provides in-kernel windowing system for use by
 *			applications, supporting 16/24/32 bpp displays.
 * Module originator:	Zachary Smith (fbui@comcast.net)
 *
 *=========================================================================
 *
 * FBUI, an in-kernel windowing system for Linux.
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
 *=========================================================================
 *
 * Changes:
 *
 * 12 Sep 2004, fbui@comcast.net: added fbui_copy_area
 * 13 Sep 2004, fbui@comcast.net: added fbui_draw_string
 * 13 Sep 2004, fbui@comcast.net: added sharing of fbui between processes
 * 14 Sep 2004, fbui@comcast.net: added fbui_switch
 * 16 Sep 2004, fbui@comcast.net: added fbui_open/close
 * 16 Sep 2004, fbui@comcast.net: began use of KD_GRAPHICS
 * 16 Sep 2004, fbui@comcast.net: return to KD_TEXT after all closed
 * 18 Sep 2004, fbui@comcast.net: added event types
 * 20 Sep 2004, fbui@comcast.net: added window-data semaphore
 * 21 Sep 2004, fbui@comcast.net: added keyboard focus concept
 * 22 Sep 2004, fbui@comcast.net: keyboard reading via tty->read_buf
 * 22 Sep 2004, fbui@comcast.net: keyboard mode set to XLATE
 * 22 Sep 2004, fbui@comcast.net: simple keyboard behavior achieved
 * 23 Sep 2004, fbui@comcast.net: added implicit keyfocus when only 1 win
 * 24 Sep 2004, fbui@comcast.net: fbui_open() now accepts target console#
 * 25 Sep 2004, fbui@comcast.net: added read_point
 * 25 Sep 2004, fbui@comcast.net: expanded role of window manager
 * 25 Sep 2004, fbui@comcast.net: added fbui_put_rgb3
 * 25 Sep 2004, fbui@comcast.net: coordinates now truly window-relative
 * 26 Sep 2004, fbui@comcast.net: added per-console winlists
 * 27 Sep 2004, fbui@comcast.net: keyfocus_stack
 * 28 Sep 2004, fbui@comcast.net: control ioctl
 * 29 Sep 2004, fbui@comcast.net: window semaphore; window resize, move
 * 01 Oct 2004, fbui@comcast.net: all drawing commands now atomic
 * 01 Oct 2004, fbui@comcast.net: fixed put_rgb bug
 * 02 Oct 2004, fbui@comcast.net: added fbui_window_info()
 * 04 Oct 2004, fbui@comcast.net: structural changes
 * 05 Oct 2004, fbui@comcast.net: keyboard focus working
 * 05 Oct 2004, fbui@comcast.net: added fbui_wininfo_change()
 * 05 Oct 2004, fbui@comcast.net: added keyboard accelerators
 * 06 Oct 2004, fbui@comcast.net: moved code to pixel_from_rgb
 * 06 Oct 2004, fbui@comcast.net: removed read_point (mmap replaces it)
 * 06 Oct 2004, fbui@comcast.net: created vesa_hline
 * 06 Oct 2004, fbui@comcast.net: vesa_clear now supports info->bgcolor
 * 10 Oct 2004, fbui@comcast.net: added support for retrieving mouse info
 * 12 Oct 2004, fbui@comcast.net: resolved bug to do with multiple down's
 * 12 Oct 2004, fbui@comcast.net: switched to rw_semaphores
 * 14 Oct 2004, fbui@comcast.net: got fbui_input module working
 * 14 Oct 2004, fbui@comcast.net: mouse data now available as events
 * 14 Oct 2004, fbui@comcast.net: key data now available as events
 * 15 Oct 2004, fbui@comcast.net: put in basic mouse-pointer logic
 * 16 Oct 2004, fbui@comcast.net: fbui-input now calls fbui to provide events
 * 17 Oct 2004, fbui@comcast.net: perfected software mouse-pointer logic
 * 17 Oct 2004, fbui@comcast.net: restructuring: now only fb_* in fbops.
 * 17 Oct 2004, fbui@comcast.net: added window-moved event.
 * 18 Oct 2004, fbui@comcast.net: fixed keyboard accelerators post inp-hand
 * 20 Oct 2004, fbui@comcast.net: wait-for-event working
 * 20 Oct 2004, fbui@comcast.net: accelerators are now for chars [32,128)
 * 21 Oct 2004, fbui@comcast.net: revisions to support panel manager
 * 22 Oct 2004, fbui@comcast.net: removed keyfocus stack
 * 23 Oct 2004, fbui@comcast.net: fixed tty: NULL is now used
 * 25 Oct 2004, fbui@comcast.net: key queue is now per-console
 * 26 Oct 2004, fbui@comcast.net: putpixels now takes in-kernel data
 * 26 Oct 2004, fbui@comcast.net: putpixels_rgb takes 0% & 100% transparency
 * 26 Oct 2004, fbui@comcast.net: added vesa_getpixels_rgb
 * 26 Oct 2004, fbui@comcast.net: mouse pointer now drawn using putpixels
 * 26 Oct 2004, fbui@comcast.net: conversion to multiple windows per process
 * 26 Oct 2004, fbui@comcast.net: added cut/paste logic
 * 11 Nov 2004, fbui@comcast.net: left-bearing for drawn characters!
 * 18 Nov 2004, fbui@comcast.net: added mouse-motion event
 * 24 Nov 2004, fbui@comcast.net: added fbui-specific logfile
 * 29 Nov 2004, fbui@comcast.net: added proper event queues for windows.
 * 12 Dec 2004, fbui@comcast.net: added ability to open hidden window
 * 13 Dec 2004, fbui@comcast.net: event queues are now per-process
 * 13 Dec 2004, fbui@comcast.net: Enter event now generated after switch
 * 13 Dec 2004, fbui@comcast.net: added fbui_tinyblit
 * 14 Dec 2004, fbui@comcast.net: Enter/Leave after ptr hide/unhide
 * 15 Dec 2004, fbui@comcast.net: wait-for-event checks all pid's windows
 * 16 Dec 2004, fbui@comcast.net: processentry nwindows now correct
 * 18 Dec 2004, fbui@comcast.net: processentry now has event mask
 * 21 Dec 2004, fbui@comcast.net: optimization for tinyblit
 * 28 Dec 2004, fbui@comcast.net: added fbui_clean to remove zombie data
 * 31 Dec 2004, fbui@comcast.net: added process_exists
 * 01 Jan 2005, fbui@comcast.net: added PrtSc accelerator
 * 01 Jan 2005, fbui@comcast.net: added SUBTITLE and SETFONT cmds
 * 02 Jan 2005, fbui@comcast.net: unhide now performs overlap check
 * 03 Jan 2005, fbui@comcast.net: added mode24 for slight speedup
 * 03 Jan 2005, fbui@comcast.net: added pointerfocus and receive_all_motion
 * 04 Jan 2005, fbui@comcast.net: fixed tinyblit bug
 * 05 Jan 2005, fbui@comcast.net: separated button events from keys
 * 13 Jan 2005, fbui@comcast.net: minor tweaks
 * 27 Feb 2005, fbui@comcast.net: experimenting with cfb fillrect
 * 01 Apr 2005, fbui@comcast.net: removed use of fillrect due to crashing
 * 10 Jun 2005, fbui@comcast.net: added CONFIG_FB_UI_*BPP.
 * 10 Jun 2005, fbui@comcast.net: added check of whether bpp is supported.
 * 05 Jul 2005, fbui@comcast.net: added fbui_overlap_window
 * 06 Jul 2005, fbui@comcast.net: added fbui_winsort
 * 12 Jul 2005, fbui@comcast.net: updated fbui_switch to use fbui_winsort.
 * 18 Jul 2005, fbui@comcast.net: updated winsort to permit removing window.
 * 27 Jul 2005, fbui@comcast.net: basic rectangle lists working.
 *		=> 0.10.0
 * 02 Aug 2005, fbui@comcast.net: rectangle subraction working.
 * 02 Aug 2005, fbui@comcast.net: fixed winsort removal bug.
 * 02 Aug 2005, fbui@comcast.net: removed window undraw working.
 * 02 Aug 2005, fbui@comcast.net: fbui_overlap_window draws/undraws pointer
 * 02 Aug 2005, fbui@comcast.net: fbui_remove_window draws/undraws pointer
 * 03 Aug 2005, fbui@comcast.net: fixed another winsort removal bug.
 *		=> 0.10.1
 * 04 Aug 2005, fbui@comcast.net: FBUI now kmallocs much of its fb_info data.
 * 04 Aug 2005, fbui@comcast.net: added FBUI kmalloc/kfree accounting.
 * 05 Aug 2005, fbui@comcast.net: added memory log, fixed memory leak.
 * 07 Aug 2005, fbui@comcast.net: expose events now include rects.
 * 08 Aug 2005, fbui@comcast.net: rectangle-based fbui_moveresize working.
 * 08 Aug 2005, fbui@comcast.net: fixed moveresize memory leak.
 * 08 Aug 2005, fbui@comcast.net: added raise+lower to winsort.
 * 08 Aug 2005, fbui@comcast.net: added fbui_raise/lower_window() & cmds.
 *		=> 0.10.2
 * 08 Aug 2005, fbui@comcast.net: added hide_unhide to redo.
 * 09 Aug 2005, fbui@comcast.net: updated fbui_hide/unhide_window().
 * 19 Aug 2005, fbui@comcast.net: wm may no longer draw, nor receives Expose
 * 21 Aug 2005, fbui@comcast.net: better handling of deleted windows.
 *		=> 0.10.3
 * 21 Aug 2005, fbui@comcast.net: fb_putpixels_rgb now handles transparency.
 * 21 Aug 2005, fbui@comcast.net: added cutesy shadow next to mouse pointer.
 * 21 Aug 2005, fbui@comcast.net: added some simple logic for overlay window.
 * 21 Aug 2005, fbui@comcast.net: removed tinyblit.
 * 21 Aug 2005, fbui@comcast.net: moved fbui_draw_string to libfbui
 * 24 Aug 2005, fbui@comcast.net: fb_putpixels_rgb becomes vesa_putimage_rgb
 * 24 Aug 2005, fbui@comcast.net: fb_putpixels_native -> vesa_putimage_native
 * 24 Aug 2005, fbui@comcast.net: fb_putpixels_rgb3 -> vesa_putimage_rgb3
 * 24 Aug 2005, fbui@comcast.net: updated vesa_point & fbui_draw_hline for 
 *				   transparency.
 * 30 Aug 2005, fbui@comcast.net: moved all VESA-specific code to vesafb.c
 * 30 Aug 2005, fbui@comcast.net: updated code to use fb_clip struct.
 * 30 Aug 2005, fbui@comcast.net: removed invert-line.
 * 30 Aug 2005, fbui@comcast.net: background color now filled in on wm open.
 * 30 Aug 2005, fbui@comcast.net: added pointer_hide_if_touching.
 * 30 Aug 2005, fbui@comcast.net: added pointer_hide_if_rects.
 *		=> 0.10.4
 * 06 Sep 2005, fbui@comcast.net: various minor bugfixes.
 * 07 Sep 2005, fbui@comcast.net: added set/get icon.
 * 07 Sep 2005, fbui@comcast.net: added greyscale & alpha-only putimage.
 *		=> 0.10.5
 * 10 Sep 2005, fbui@comcast.net: merged fbui_draw_point/hline/vline/fillarea
 * 10 Sep 2005, fbui@comcast.net: added lineto.
 * 10 Sep 2005, fbui@comcast.net: merged 5 routines into fbui_put_image.
 * 11 Sep 2005, fbui@comcast.net: added putimage support for 16-bit(565) RGB.
 * 11 Sep 2005, fbui@comcast.net: most routines now take params in structs.
 *		=> 0.10.6
 * 12 Sep 2005, fbui@comcast.net: added FBUI_XYTOWINDOW command.
 * 12 Sep 2005, fbui@comcast.net: vesa optimizations for 32bpp.
 * 13 Sep 2005, fbui@comcast.net: added mode565.
 * 14 Sep 2005, fbui@comcast.net: ptrfocus now makes sense and works.
 * 15 Sep 2005, fbui@comcast.net: added window obscured flag.
 * 15 Sep 2005, fbui@comcast.net: MOVERESIZE event now returns flags.
 * 15 Sep 2005, fbui@comcast.net: added mode555 (unused).
 * 16 Sep 2005, fbui@comcast.net: separated FBUI_IMAGE & FBUI_FULLIMAGE.
 *		=> 0.10.7
 * 18 Sep 2005, fbui@comcast.net: VESA driver optimizations.
 * 19 Sep 2005, fbui@comcast.net: moved generic fillrect/putimage to fbui.c.
 * 20 Sep 2005, fbui@comcast.net: added FB_IMAGETYPE_MONO.
 *		=> 0.10.8
 * 22 Sep 2005, fbui@comcast.net: generic_filltriangle & fbui_fill_triangle.
 * 23 Sep 2005, fbui@comcast.net: fixed altdown getting cleared problem.
 * 23 Sep 2005, fbui@comcast.net: VESA speedups.
 * 24 Sep 2005, fbui@comcast.net: Kconfig FB...BPP are now per-driver.
 * 24 Sep 2005, fbui@comcast.net: fixed processentry bug.
 * 24 Sep 2005, fbui@comcast.net: fixed altdown bug.
 * 27 Sep 2005, fbui@comcast.net: FBUI_EVENT_NAMECHANGE & fbui_wininfo_change.
 * 27 Sep 2005, fbui@comcast.net: hide_if_touching and hidden windows.
 * 27 Sep 2005, fbui@comcast.net: added better reporting of discarded events.
 *		=> 0.11.0
 * 02 Oct 2005, fbui@comcast.net: 24bpp improvements and speed-ups.
 * 17 Oct 2005, fbui@comcast.net: bugfix re Expose getting lost when queue full.
 * 17 Oct 2005, fbui@comcast.net: genericized getpixels & read_point.
 *		=> 0.11.1
 * 19 Oct 2005, fbui@comcast.net: source code compression.
 * 20 Oct 2005, fbui@comcast.net: added FBUI_BEEP, not working yet.
 *		=> 0.11.2
 *============================================================================
 */

#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fb.h>
#include <asm/types.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/console.h>
#include <linux/kbd_kern.h>
#include <linux/vt_kern.h>
#include <linux/ctype.h>
#include <linux/input.h>	/* struct input_event */
#include <linux/sem.h>
#include <linux/delay.h>
#include <linux/pid.h>	/* find_pid */


/* Variables for input_handler */
static char fbui_handler_regd = 0;
static char got_rel_x = 0;
static char got_rel_y = 0;
static short incoming_x = 0;
static short incoming_y = 0;
static char altdown = 0;
static char intercepting_accel = 0;

void generic_filltriangle (struct fb_info *info, struct fb_draw *p);

extern void fbui_mksound(int,int);


#define FBUI_VERSION "0.11.2"


/* #define FBUI_ERROR_LOC(NN) { }  */
#define FBUI_ERROR_LOC(NN) { info->fbui_errloc = NN; } 



/* Mouse-pointer */
#define PTRWID 16
#define PTRHT 17
static unsigned long pointer_saveunder[PTRWID*PTRHT];


#if 0
static char *event_names[] = {
	"(none)",
	"Expose",
	"Hide",
	"Unhide",
	"Enter",
	"Leave",
	"Key",
	"MoveResize",
	"Accel",
	"WinChange",
	"Motion",
	"Button",
	"Raise",
	"Lower",
	"ClickArea",
	"NameChange"
};
#endif


static void fbui_enable_pointer (struct fb_info *info);

static int fbui_clear (struct fb_info *info, struct fbui_win *win);
static int fbui_fill_rect (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *);
static int fbui_draw_line (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *);
static int fbui_fill_triangle (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *);
static int fbui_draw_rect (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *);
static int fbui_put_image (struct fb_info *info, struct fbui_win *win, 
	struct fb_put *);
static int fbui_copy_area (struct fb_info *info, struct fbui_win *win,
	struct fb_draw *);
static struct fbui_processentry *alloc_processentry (struct fb_info *info, 
	int pid, int cons);
static void free_processentry(struct fb_info *info, struct fbui_processentry *,
		char);
static struct fbui_win *get_window_at_xy (struct fb_info *info, short,short);
static struct fbui_win *
   accelerator_test (struct fb_info *info, int cons, unsigned char);
static int fbui_clean (struct fb_info *info, int cons);
static int fbui_destroy_win (struct fb_info *info, short win_id, int);
static struct fbui_rects* rects_new (u8,u8);
static struct fbui_rects *rects_add (struct fbui_rects *r, 
				short x0, short y0, short x1, short y1, u8);
static void rects_delete (struct fbui_rects *r);
static void rects_shift (struct fbui_rects *r, short dx, short dy);


/* #define MEMORYLOG */
static u32 fbui_total_allocated = 0;

#ifdef MEMORYLOG

#else
void *myalloc (u8 foo, u8 bar, u32 size) 
{
	return kmalloc (size, GFP_KERNEL);
}
void myfree (u16 foo, void *addr)
{
	if (addr)
		kfree (addr);
	else
		printk (KERN_INFO "fbui/myfree: attempt to free NULL pointer, code=%u\n",foo);
}
void myalloc_dump (void)
{
}
#endif

/* tasklist is no more as of c59923a15c12d2b3597af913bf234a0ef264a38b
static int process_exists (int pid)
{
	struct pid *pidptr;
	read_lock_irq(&tasklist_lock);
	//pidptr = find_pid (PIDTYPE_PID, pid);
	pidptr = pid_task(pid, PIDTYPE_PID);
	read_unlock_irq(&tasklist_lock);
	return pidptr != NULL;
}
*/


void rects_compress (struct fbui_rects *r)
{
	short min_x, max_x, min_y, max_y;
	short ix, lim;

	if (!r)
		return;
	/*---------*/

	min_x = min_y = 32767;
	max_x = max_y = -32768;
	ix=0;
	lim = r->total << 2;

	while (ix < lim) {
		short x,y;
		x = r->c[ix++];
		y = r->c[ix++];
		if (x < min_x) 
			min_x = x;
		if (y < min_y) 
			min_y = y;
		x = r->c[ix++];
		y = r->c[ix++];
		if (x > max_x) 
			max_x = x;
		if (y > max_y) 
			max_y = y;
	}
	r->total = 1;
	r->c[0] = min_x;
	r->c[1] = min_y;
	r->c[2] = max_x;
	r->c[3] = max_y;
}

static void fbui_enqueue_event (struct fb_info *info, struct fbui_win *win, 
                           	struct fbui_event *ev, int inside_IH)
{
	DEFINE_SPINLOCK(mylock);
	struct fbui_processentry *pre;
	unsigned long flags = 0;
	short head;

	if (!info || !win || !ev)
		return;

	pre = win->processentry;
	if (!pre) {
		printk (KERN_INFO "fbui_enqueue_event: no processentry\n");
		return;
	}
	if (!pre->in_use) {
		printk (KERN_INFO "fbui_enqueue_event: processentry not in use\n");
		return;
	}
	/*----------*/

	if (ev->rects)
		rects_shift (ev->rects, -win->x0, -win->y0);

	if (!inside_IH) {
		down (&pre->queuesem);
		spin_lock_irqsave(&mylock, flags); 
	}

	if (pre->events_pending >= FBUI_MAXEVENTSPERPROCESS) {
		/* Rule:
		 * If the event queue is full and the event is an Expose,
		 * and if the previous event was an Expose, then we merge them.
		 *
		 * If Raise or Lower, we set a flag to recover that type of event 
		 * later.
		 */
		switch (ev->type) {
		case FBUI_EVENT_EXPOSE: {
			struct fbui_rects *r;
			short prev = pre->events_head - 1;
			short tr;
			struct fbui_event *ev2;
			if (prev < 0)
				prev = FBUI_MAXEVENTSPERPROCESS - 1;
			ev2 = &pre->events[prev];
			if (ev2->type != FBUI_EVENT_EXPOSE || ev2->id != win->id) {
				win->expose_event = 1;
				if (ev->rects) {
					rects_delete (ev->rects);
					ev->rects = NULL;
				}
				break;
			}
			if (!ev2->rects) {
				if (ev->rects) {
					rects_delete (ev->rects);
					ev->rects = NULL;
				}
				break;
			}
			if (!ev->rects) {
				rects_delete (ev2->rects);
				ev2->rects = NULL;
				break;
			}
			rects_shift (ev->rects, -win->x0, -win->y0);
			tr = ev->rects->total + ev2->rects->total;
			if (tr <= FBUI_RECT_ARYSIZE) {
				short min_x, max_x, min_y, max_y;

				r = ev->rects;
				rects_compress (r);
				min_x = r->c[0];
				min_y = r->c[1];
				max_x = r->c[2];
				max_y = r->c[3];
				rects_delete (r);
				ev->rects = NULL;

				r = ev2->rects;
				rects_compress (r);
				r->total = 1;
				if (r->c[0] > min_x)
					r->c[0] = min_x;
				if (r->c[1] > min_y)
					r->c[1] = min_y;
				if (r->c[2] > max_x)
					r->c[2] = max_x;
				if (r->c[3] > max_y)
					r->c[3] = max_y;
			} else {
				rects_delete (ev2->rects);
				ev2->rects = NULL;
				break;
			}

			win->expose_event = 1;
			if ((r = ev->rects)) {
				rects_delete (r);
				ev->rects = NULL;
			 }
			}
			break;

		case FBUI_EVENT_RAISE:
			win->raise_event = 1;
			break;
		
		case FBUI_EVENT_LOWER:
			win->lower_event = 1;
			break;

#if 0
		default: {
			char *s = "?";
			if (ev->type >=0 && ev->type <= FBUI_EVENT_LAST)
				s = event_names [(int)ev->type];
			printk (KERN_INFO "fbui_enqueue_event: window %d's event queue full, discarding %s\n",
				win->id, s);
			break;
		 }
#endif
		}
	} else {
		ev->id = win->id;
		ev->pid = win->pid;
		head = pre->events_head;
		memcpy (&pre->events[head], ev, sizeof (struct fbui_event));
		pre->events_head = (head + 1) % FBUI_MAXEVENTSPERPROCESS;
		pre->events_pending++;
	}

/*printk(KERN_INFO "enqueue: window %d event %d pending=%d, enqueue at head %d\n", win->id, ev->type, pre->events_pending, head);*/

	if (!inside_IH) {
		spin_unlock_irqrestore(&mylock, flags); 
		up (&pre->queuesem);
	}

	if (pre->waiting) {
		pre->waiting = 0;
		wake_up_interruptible (&pre->waitqueue);
	}
}


static int fbui_dequeue_event (struct fb_info *info, 
			       struct fbui_processentry *pre,
                               struct fbui_event *ev)
{
	short tail;
	unsigned long flags;

	if (!info || !pre || !ev)
		return 0;

	if (pre->events_pending <= 0) {
		struct rw_semaphore *sem = &info->winptrSem;
		int cons = pre->console;
		int i = cons * FBUI_MAXWINDOWSPERVC;
		int lim = i + FBUI_MAXWINDOWSPERVC;
		char got_event=0;

		down_read (sem);
		ev->pid = pre->pid;

		while (i < lim) {
			struct fbui_win *w = info->windows[i++];
			if (!w)
				continue;

			if (w->processentry == pre) {
				if (w->expose_event) {
					ev->type = FBUI_EVENT_EXPOSE;
					got_event=1;
					w->expose_event = 0;
					ev->has_rects = 0;
					ev->rects = NULL;
					ev->id = w->id;
					break;
				} else
				if (w->raise_event) {
					ev->type = FBUI_EVENT_RAISE;
					got_event=1;
					w->raise_event = 0;
					ev->id = w->id;
					break;
				} else
				if (w->lower_event) {
					ev->type = FBUI_EVENT_LOWER;
					got_event=1;
					w->lower_event = 0;
					ev->id = w->id;
					break;
				} 
			}
		}
		up_read (sem);
/*if (got_event) printk (KERN_INFO "dequeue: ev { type %d key %d pid %d id %d } current->pid %d pre { pid %d ix %d}\n",ev->type,ev->key,ev->pid,ev->id,current->pid,pre->pid,pre->index); */
		return got_event;
	}
	/*----------*/

	/* Remove an event from the event queue for this process,
	 * locking out input_handler which writes to the queue,
	 * and locking out via a semaphore any other process
	 * either reading or writing the queue.
	 */
	down (&pre->queuesem);
	spin_lock_irqsave(&pre->queuelock, flags); 

	tail = pre->events_tail;
	memcpy (ev, &pre->events[tail], sizeof (struct fbui_event));
	pre->events_tail = (tail + 1) % FBUI_MAXEVENTSPERPROCESS;
	--pre->events_pending;

	spin_unlock_irqrestore (&pre->queuelock, flags);
	up (&pre->queuesem);

/*printk (KERN_INFO "dequeue: ev { type %d key %d pid %d id %d } current->pid %d pre { pid %d ix %d}\n",ev->type,ev->key,ev->pid,ev->id,current->pid,pre->pid,pre->index); */

	return 1;
}


u32 pixel_from_rgb (struct fb_info *info, u32 value)
{
	u32 r,g,b;
	unsigned char tmp = 0xff;
	unsigned long pixel;

	value &= 0xffffff;
	if (info->mode24)
		return value;
	if (info->mode565) {
		return  ((value >> 8) & 0xf800) |
			((value >> 5) & 0x7e0) |
			((value >> 3) & 31);
	} 
	if (info->mode555) {
		return  ((value >> 9) & 0x7c00) |
			((value >> 6) & 0x3e0) |
			((value >> 3) & 31);
	} 

	b = tmp & value;
	value >>= 8;
	g = tmp & value;
	value >>= 8;
	r = tmp & value;
	tmp = 8;
	r >>= (tmp - info->redsize);
	g >>= (tmp - info->greensize);
	b >>= (tmp - info->bluesize);
	r <<= info->redshift;
	g <<= info->greenshift;
	b <<= info->blueshift;
	pixel = r | g | b;
/* printk (KERN_INFO "pixel=%08lx\n", pixel); */
	return pixel;
}


u32 pixel_to_rgb (struct fb_info *info, u32 value)
{
	u32 r,g,b;
	unsigned char tmp = 8;

	if (info->mode24)
		return value & 0xffffff;
	if (info->mode565) {
		return  ((value << 8) & 0xf80000) |
			((value << 5) & 0xfc00) |
			((value << 3) & 0xf8);
	}
	if (info->mode555) {
		return  ((value << 9) & 0xf80000) |
			((value << 6) & 0xf800) |
			((value << 3) & 0xf8);
	}
	
	r = value >> info->redshift;
	g = value >> info->greenshift;
	b = value >> info->blueshift;
	r &= (1 << info->redsize) - 1;
	b &= (1 << info->bluesize) - 1;
	g &= (1 << info->greensize) - 1;
	r <<= (tmp - info->redsize);
	g <<= (tmp - info->greensize);
	b <<= (tmp - info->bluesize);
	r <<= 16;
	g <<= 8;
	return r | g | b;
}

/* This routine combines two pixels based on a transparency
 * value in [0,255].
 */
u32 combine_rgb_pixels (u32 orig_value, u32 value, u32 transp)
{
	u32 r,g,b;
	u32 t;
	register u32 m = 255;
	transp &= 255;
	t = m - transp;
	r = transp * (m & (orig_value>>16)) + t * (m & (value>>16));
	g = transp * (m & (orig_value>>8)) + t * (m & (value>>8));
	b = transp * (m & orig_value) + t * (m & value);
	r >>= 8;
	g >>= 8;
	b >>= 8;
	value = (r<<16) | (g<<8) | b;
	return value;
}



static void local_clear (struct fb_info *info, u32 color)
{
	struct fb_draw params;

	if (!info) 
		return;
	/*----------*/

#if 0
	/* This is crashing */
	struct fb_fillrect fr;
	fr.dx = 0;
	fr.dy = 0;
	fr.width = info->var.xres;
	fr.height = info->var.yres;
	fr.color = color_;
	fr.rop = ROP_COPY;

	info->fbops->fb_fillrect (info, &fr);
#endif

	params.x0 = 0;
	params.x1 = info->var.xres - 1;
	params.y0 = 0;
	params.y1 = info->var.yres - 1;
	params.color = color & 0xffffff;
	params.clip_valid = 0;

	info->fbops->fb_fillrect2 (info, &params);
}


static char rects_have_point (struct fbui_rects *r, short x, short y)
{
	u16 ix, lim;

	if (!r || x < 0 || y < 0)
		return 0;
	/*----------*/

	ix = 0;
	lim = r->total << 2;
	while (r && ix < lim) {
		short rx0,ry0,rx1,ry1;
		rx0 = r->c[ix++];
		ry0 = r->c[ix++];
		rx1 = r->c[ix++];
		ry1 = r->c[ix++];

		if (x >= rx0 && x <= rx1 && y >= ry0 && y <= ry1)
			return 1;

		if (ix >= lim) {
			ix = 0;
			r = r->next;
			if (r)
				lim = r->total << 2;
		}
	}
	return 0;
}


static void rects_fill (struct fb_info *info, struct fbui_rects *r, 
		u32 color)
{
	u16 ix, lim;
	struct fb_draw params;

	if (!info || !r)
		return;
	if (!info->fbops->fb_fillrect2)
		return;
	/*----------*/

	color &= 0xffffff; /* no transparency */

	ix = 0;
	lim = r->total << 2;
	while (r && ix < lim) {
		params.x0 = r->c[ix++];
		params.y0 = r->c[ix++];
		params.x1 = r->c[ix++];
		params.y1 = r->c[ix++];
		params.clip_valid = 0;
		params.color = color;

		info->fbops->fb_fillrect2 (info, &params);

		if (ix >= lim) {
			ix = 0;
			r = r->next;
			if (r)
				lim = r->total << 2;
		}
	}
}


void background_fill (struct fb_info *info, struct fbui_rects *r, u32 color)
{
	rects_fill (info, r, color);

	/* Future: a background image */
}


static short fbui_winsort (struct fb_info *info, int cons, short *result,
			struct fbui_win **wm_return,
			struct fbui_win *insert_me,
			struct fbui_win *remove_me,
			struct fbui_win *raise_me,
			struct fbui_win *lower_me);

/* Parameter cons is the VC we are switching to;
 * fg_console is the VC we are switching from.
 */
int fbui_switch (struct fb_info *info, int cons)
{
	struct fbui_win *enter_win;
	struct fbui_win *leave_win=NULL;
	struct rw_semaphore *sem;
	struct fbui_event ev;
	static short sorted_windows [FBUI_MAXWINDOWSPERVC];
	int i,lim;

	if (!info)
		return 0;
	if (cons < 0 || cons >= FBUI_MAXCONSOLES || !vc_cons_allocated (cons)) {
		FBUI_ERROR_LOC(1);
		return FBUI_ERR_BADVC;
	}
	/*----------*/

	info->pointer_active = 0;
	intercepting_accel = 0;

	/* Clean up the new console
	 */
	fbui_clean (info, cons);

	/* Find out which window in previous console gets the Leave event.
	 */
	sem = &info->winptrSem;
	down_write (sem);
	leave_win = info->pointer_window [fg_console];
	info->pointer_window [fg_console] = NULL;
	up_write (sem);

	/* Set the new console# */
	fg_console = cons;

	/* This routine is called whenever there's a switch,
	 * not just when we're switching to a graphics console.
	 */
	if (vc_cons[fg_console].d->vc_mode != KD_GRAPHICS)
		return 1;

	/* Former pointer window no longer valid */
	down_write (sem);
	info->pointer_window [cons] = NULL;
	up_write (sem);

	/* Clear the visible parts of the background */
	background_fill (info, info->bg_rects[cons], info->bgcolor[cons]);

	/* Perform the clears
	 */
	i = 0;
	down_read (sem);
	lim = fbui_winsort (info, cons, sorted_windows, NULL, 
		NULL, NULL, NULL, NULL);
	if (lim >= 0) {
		for (i = lim; i >= 0; i--) {
			short ix = sorted_windows[i];
			struct fbui_win *p;

			if (ix < 0 || ix >= 
			     (FBUI_MAXWINDOWSPERVC * FBUI_MAXCONSOLES)) {
				FBUI_ERROR_LOC(162);
				return FBUI_ERR_INTERNAL;
			}

			p = info->windows [ix];

			if (p && !p->is_wm && !p->is_hidden)
				rects_fill (info, p->rects, 
						 p->bgcolor);
		}
	} else
	if (lim == -2)
		printk (KERN_INFO "fbui_switch: winsort failed\n");
	up_read (sem);

	/* Send Leave event to whichever window had gotten an Enter.
	 */
	if (leave_win) {
		memset (&ev, 0, sizeof (struct fbui_event));
		ev.type = FBUI_EVENT_LEAVE;

		leave_win->pointer_inside = 0;

		fbui_enqueue_event (info, leave_win, &ev, 0);
	}

	/* Send the expose events
	 */
	i = cons * FBUI_MAXWINDOWSPERVC;
	lim = i + FBUI_MAXWINDOWSPERVC;
	down_read (sem);
	for (; i < lim; i++) {
		struct fbui_win *p = info->windows [i];

		if (p && !p->is_hidden) {
			memset (&ev, 0, sizeof (struct fbui_event));
			ev.type = FBUI_EVENT_EXPOSE;

			p->pointer_inside = 0;

			/* No need to expose if hidden */
			if (!p->is_hidden)
				fbui_enqueue_event (info, p, &ev, 0);
		}
	}
	up_read (sem);

	info->pointer_active = 0;
	fbui_enable_pointer (info);

	/* If the pointer is on top of a window, 
	 * send that window an Enter event.
	 */
	down_read (sem);
	enter_win= get_window_at_xy(info,info->curr_mouse_x,info->curr_mouse_y);
	up_read (sem);
	if (enter_win) {
		struct fbui_event ev;
		memset (&ev, 0, sizeof (struct fbui_event));
		ev.type = FBUI_EVENT_ENTER;
		fbui_enqueue_event (info, enter_win, &ev, 0);
	}

	return 1;
}



static int pointer_in_window (struct fb_info *info, struct fbui_win *win, 
			      char just_tip)
{
	register short mx,my,mx1,my1;
	short x0,y0,x1,y1;

	if (!info || !win)
		return 0;
	if (!info->pointer_active || info->pointer_hidden)
		return 0;
	/*----------*/

	if (win->console != fg_console)
		return 0;
	if (win->is_hidden)
		return 0;
	mx = info->curr_mouse_x;
	my = info->curr_mouse_y;
	mx1 = mx + PTRWID - 1;
	my1 = my + PTRHT - 1;
	x0 = win->x0;
	y0 = win->y0;
	x1 = win->x1;
	y1 = win->y1;
	if (!just_tip) {
		if ((mx >= x0 && mx <= x1) || (mx1 >= x0 && mx1 <= x1))
			if ((my >= y0 && my <= y1) || (my1 >= y0 && my1 <= y1))
				return 1;
	} else {
		if ((mx >= x0 && mx <= x1) && (my >= y0 && my <= y1))
				return 1;
	}

	return 0;
}


/* XX -- Currently the mouse pointer is a fixed pattern */
static unsigned long ptrpixels [] = {
#define T___ 0xff000000
#define TA__ 0xf8000000
#define T9__ 0xf0000000
#define T8__ 0xe2000000
#define T7__ 0xd4000000
#define BORD RGB_BLACK
#define X___ RGB_WHITE
BORD,BORD,T___,T___,T___,T___,T___,T___,T___,T___,T___,T___,T___,T___,T___,T___,
BORD,X___,BORD,T___,T___,T___,TA__,TA__,T___,T___,T___,T___,T___,T___,T___,T___,
BORD,X___,X___,BORD,TA__,TA__,T8__,T9__,TA__,T___,T___,T___,T___,T___,T___,T___,
BORD,X___,X___,X___,BORD,T9__,T7__,T8__,T9__,TA__,T___,T___,T___,T___,T___,T___,
BORD,X___,X___,X___,X___,BORD,T7__,T7__,T8__,T9__,TA__,T___,T___,T___,T___,T___,
BORD,X___,X___,X___,X___,X___,BORD,T7__,T7__,T8__,T8__,TA__,T___,T___,T___,T___,
BORD,X___,X___,X___,X___,X___,X___,BORD,T7__,T7__,T8__,T9__,TA__,T___,T___,T___,
BORD,X___,X___,X___,X___,X___,X___,X___,BORD,T7__,T7__,T8__,T9__,TA__,T___,T___,
BORD,X___,X___,X___,X___,X___,X___,X___,X___,BORD,T7__,T8__,T8__,T9__,TA__,T___,
BORD,X___,X___,X___,X___,X___,BORD,BORD,BORD,BORD,T8__,T9__,T9__,T8__,T9__,TA__,
BORD,X___,X___,BORD,X___,X___,BORD,T7__,T7__,T8__,T9__,T9__,T9__,T9__,T9__,TA__,
BORD,X___,BORD,T___,BORD,X___,X___,BORD,T9__,T7__,T8__,TA__,T___,T___,T___,T___,
BORD,BORD,T___,T___,BORD,X___,X___,BORD,T9__,T7__,T8__,T9__,TA__,T___,T___,T___,
T___,T___,T___,T___,T___,BORD,X___,X___,BORD,T9__,T7__,T8__,T9__,T___,T___,T___,
T___,T___,T___,T___,T___,BORD,X___,X___,BORD,T9__,T7__,T8__,T9__,TA__,T___,T___,
T___,T___,T___,T___,T___,T___,BORD,BORD,TA__,T9__,T7__,T8__,T9__,TA__,T___,T___,
T___,T___,T___,T___,T___,T___,T___,T___,T___,TA__,T9__,T9__,TA__,T___,T___,T___,
};


static void fbui_pointer_save (struct fb_info *info)
{
	short j,x,y;
	unsigned long *p = pointer_saveunder;
	struct fb_put put;

	if (!info)
		return;
	if (!info->fbops->fb_getpixels_rgb) 
		return;
	/*----------*/

	x = info->mouse_x0;
	y = info->mouse_y0;
	j = PTRHT;
	while (j--) {
		put.x0 = x;
		put.y0 = put.y0 = y++;
		put.width = PTRWID;
		put.height = 1;
		put.x1 = x + PTRWID - 1;
		put.pixels = (unsigned char*) p;
		put.location = FB_LOCATION_KERNEL;
		put.clip_valid = 0;

		info->fbops->fb_getpixels_rgb (info, &put);
		p += PTRWID;
	}
}

static void fbui_pointer_restore (struct fb_info *info)
{
	struct fb_put p;

	if (!info)
		return;
	if (!info->fbops->fb_putimage) 
		return;
	/*----------*/

	p.x0 = info->mouse_x0;
	p.y0 = info->mouse_y0;
	p.x1 = info->mouse_x0 + PTRWID - 1;
	p.y1 = info->mouse_y0 + PTRHT - 1;
	p.width = PTRWID;
	p.height = PTRHT;
	p.xstart = 0;
	p.ystart = 0;
	p.yend = PTRHT-1;
	p.xend = PTRWID-1;
	p.pixels = (unsigned char*) pointer_saveunder;
	p.type = FB_IMAGETYPE_RGBA;
	p.location = FB_LOCATION_KERNEL;
	p.clip_valid = 0;

	info->fbops->fb_putimage (info, &p);
}

static void fbui_pointer_draw (struct fb_info *info)
{
	struct fb_put p;

	if (!info)
		return;
	if (!info->fbops->fb_putimage) 
		return;
	/*----------*/

	p.x0 = info->mouse_x0;
	p.y0 = info->mouse_y0;
	p.x1 = info->mouse_x0 + PTRWID - 1;
	p.y1 = info->mouse_y0 + PTRHT - 1;
	p.width = PTRWID;
	p.height = PTRHT;
	p.xstart = 0;
	p.ystart = 0;
	p.yend = PTRHT-1;
	p.xend = PTRWID-1;
	p.pixels = (unsigned char*) ptrpixels;
	p.type = FB_IMAGETYPE_RGBA;
	p.location = FB_LOCATION_KERNEL;
	p.clip_valid = 0;

	info->fbops->fb_putimage (info, &p);
}

static void fbui_enable_pointer (struct fb_info *info)
{
	DEFINE_SPINLOCK(lock);
	unsigned long flags;

	if (!info) 
		return;
	if (info->pointer_active)
		return;
	/*----------*/

	spin_lock_irqsave (&lock, flags); 

	fbui_pointer_save (info);
	fbui_pointer_draw (info);

	info->pointer_active = 1;
	info->pointer_hidden = 0;

	spin_unlock_irqrestore (&lock, flags); 
}

#if 0
static void fbui_disable_pointer (struct fb_info *info)
{
	if (!info) 
		return;
	if (!info->pointer_active)
		return;
	/*----------*/

	/* fbui_pointer_restore (info, win); */
	info->pointer_active = 0;
	info->pointer_hidden = 0;
}
#endif

static void fbui_hide_pointer (struct fb_info *info)
{
	DEFINE_SPINLOCK(lock);
	unsigned long flags;

	if (!info) 
		return;
	if (!info->pointer_active)
		return;
	if (info->pointer_hidden)
		return;
	/*----------*/

	down (&info->pointerSems[fg_console]);
	spin_lock_irqsave (&lock, flags); 

	if (!info->have_hardware_pointer)
		fbui_pointer_restore (info);
	else {
		/* XX update hardware */
	}

	info->pointer_hidden = 1;

	spin_unlock_irqrestore(&lock, flags); 
	up (&info->pointerSems[fg_console]);
}


static void fbui_unhide_pointer (struct fb_info *info)
{
	DEFINE_SPINLOCK(lock);
	unsigned long flags;

	if (!info) 
		return;
	if (!info->pointer_active)
		return;
	if (!info->pointer_hidden)
		return;
	/*----------*/

	down (&info->pointerSems[fg_console]);
	spin_lock_irqsave(&lock, flags); 

	if (!info->have_hardware_pointer) {
		fbui_pointer_save (info);
		fbui_pointer_draw (info);
	} else {
		/* XX update hardware */
	}
	info->pointer_hidden = 0;

	spin_unlock_irqrestore(&lock, flags); 
	up (&info->pointerSems[fg_console]);
}


static char pointer_hide_if_touching (struct fb_info *info,
	short x0, short y0, short x1, short y1)
{
	DEFINE_SPINLOCK(lock);
	unsigned long flags;
	short mx = info->mouse_x0;
	short my = info->mouse_y0;
	short mx1 = info->mouse_x1;
	short my1 = info->mouse_y1;
	char touching = 1;

	if (!info || !info->pointer_active || info->pointer_hidden)
		return 0;
	if (x0 > x1) {
		short tmp=x0;
		x0=x1;
		x1=tmp;
	}
	if (y0 > y1) {
		short tmp=y0;
		y0=y1;
		y1=tmp;
	}
	/*----------*/

	down (&info->pointerSems[fg_console]);
	spin_lock_irqsave (&lock, flags); 

	if (mx > x1 || mx1 < x0)
		touching = 0;
	if (my > y1 || my1 < y0)
		touching = 0;

	if (touching) {
		if (!info->have_hardware_pointer)
			fbui_pointer_restore (info);
		else {
			/* XX talk to hardware */
		}
		info->pointer_hidden = 1;
	}

	spin_unlock_irqrestore(&lock, flags); 
	up (&info->pointerSems[fg_console]);
	return touching;
}

static char pointer_hide_if_rects (struct fb_info *info,
	struct fbui_win *win)
{
	DEFINE_SPINLOCK(lock);
	unsigned long flags;
	short mx = info->mouse_x0;
	short my = info->mouse_y0;
	short mx1 = info->mouse_x1;
	short my1 = info->mouse_y1;
	char touching;
	unsigned short ix=0;
	struct fbui_rects *r;

	if (!info || !win || !info->pointer_active || info->pointer_hidden)
		return 0;
	/*----------*/

	down (&info->pointerSems[fg_console]);
	spin_lock_irqsave (&lock, flags); 

	touching = 0;

	if (win->is_hidden) {
		touching = 1;
		if (mx > win->x1 || mx1 < win->x0)
			touching = 0;
		if (my > win->y1 || my1 < win->y0)
			touching = 0;
	} else {
		r = win->is_wm ? info->bg_rects[win->console] : win->rects;
		while (r && r->total) {
			short x0,y0,x1,y1;

			touching = 1;

			x0 = r->c[ix++];
			y0 = r->c[ix++];
			x1 = r->c[ix++];
			y1 = r->c[ix++];

			if (mx > x1 || mx1 < x0)
				touching = 0;
			if (my > y1 || my1 < y0)
				touching = 0;

			if (touching)
				break;

			if (ix >= (r->total << 2)) {
				ix = 0;
				r = r->next;
			}
		}
	}

	if (touching) {
		if (!info->have_hardware_pointer)
			fbui_pointer_restore (info);
		else {
			/* XX talk to hardware */
		}
		info->pointer_hidden = 1;
	}

	spin_unlock_irqrestore(&lock, flags); 
	up (&info->pointerSems[fg_console]);
	return touching;
}

static struct fbui_win *get_window_at_xy (struct fb_info *info, short x, short y)
{
	struct fbui_win *win=NULL;
	int i,lim;
	int cons;

	if (!info || !info->pointer_active || info->pointer_hidden)
		return NULL;
	/*----------*/

	cons = fg_console;
	i = cons * FBUI_MAXWINDOWSPERVC;
	lim = i + FBUI_MAXWINDOWSPERVC;
	while (i < lim) {
		win = info->windows [i];
		if (win && !win->is_hidden) {
			struct fbui_rects *r;

			if (win->is_wm)
				r = info->bg_rects[cons];
			else
				r = win->rects;

		    	if (rects_have_point (r, x, y))
				break;
		}
		i++;
	}
	if (i == lim)
		return NULL;
	return win;
}



static struct fbui_win *fbui_lookup_wm (struct fb_info *info, int cons)
{
	struct fbui_win *p;
        struct rw_semaphore *sem;

	if (!info)
		return NULL;
	if (cons < 0 || cons >= FBUI_MAXCONSOLES)
		return NULL;
	/*----------*/

	sem = &info->winptrSem;
	down_read (sem);
	p = info->window_managers [cons];
	up_read (sem);

	/* Since the wm is a critical window, let's
	 * just make sure that its process still exists.
	 */
	 /*
	if (p) {
		if (!process_exists (p->pid)) {
			fbui_destroy_win (info, p->id, 0);
			down_write (sem);
			p = info->window_managers [cons];
			up_write (sem);
		}
	}
	*/

	return p;
}


/* ISR */
void input_handler (u32 param, struct input_event *ev)
{
	struct fb_info *info;
	int type, code, value;
	short xlim, ylim;
	int cons;
	char event_is_altkey=0;

	if (!fbui_handler_regd)
		return;

	info = (struct fb_info*) param;
	if (!info || !ev)
		return;

	cons = fg_console;
	if (cons < 0 || cons >= FBUI_MAXCONSOLES)
		return;
	/*----------*/

	type = ev->type;
	code = ev->code;
	value = ev->value;

	if (type == EV_KEY && (code == KEY_LEFTALT || code == KEY_RIGHTALT)) {
		altdown = value > 0;
		event_is_altkey=1;
	}

	if (vc_cons[fg_console].d->vc_mode != KD_GRAPHICS)
		return;

	switch (type) {
	case EV_SYN:
		return;

	case EV_KEY:
		if (altdown && !event_is_altkey) {
			if (value==1) {
				unsigned char ia = 0;
				struct fbui_win *match;

				switch (code) {
				case KEY_1: ia = '1'; break;
				case KEY_2: ia = '2'; break;
				case KEY_3: ia = '3'; break;
				case KEY_4: ia = '4'; break;
				case KEY_5: ia = '5'; break;
				case KEY_6: ia = '6'; break;
				case KEY_7: ia = '7'; break;
				case KEY_8: ia = '8'; break;
				case KEY_9: ia = '9'; break;
				case KEY_0: ia = '0'; break;
				case KEY_TAB:	ia = '\t'; break;
				case KEY_ENTER:	ia = '\n'; break;
				case KEY_SYSRQ:	ia = FBUI_ACCEL_PRTSC; break;
				case KEY_HOME:	ia = FBUI_ACCEL_HOME; break;
				case KEY_END:	ia = FBUI_ACCEL_END; break;
				case KEY_PAGEUP:ia = FBUI_ACCEL_PGUP; break;
				case KEY_PAGEDOWN:ia = FBUI_ACCEL_PGDN; break;
				case KEY_A: ia = 'a'; break;
				case KEY_B: ia = 'b'; break;
				case KEY_C: ia = 'c'; break;
				case KEY_D: ia = 'd'; break;
				case KEY_E: ia = 'e'; break;
				case KEY_F: ia = 'f'; break;
				case KEY_G: ia = 'g'; break; 
				case KEY_H: ia = 'h'; break;
				case KEY_I: ia = 'i'; break;
				case KEY_J: ia = 'j'; break; 
				case KEY_K: ia = 'k'; break; 
				case KEY_L: ia = 'l'; break;
				case KEY_M: ia = 'm'; break; 
				case KEY_N: ia = 'n'; break;
				case KEY_O: ia = 'o'; break; 
				case KEY_P: ia = 'p'; break;
				case KEY_Q: ia = 'q'; break;
				case KEY_R: ia = 'r'; break;
				case KEY_S: ia = 's'; break; 
				case KEY_T: ia = 't'; break;
				case KEY_U: ia = 'u'; break; 
				case KEY_V: ia = 'v'; break;
				case KEY_W: ia = 'w'; break; 
				case KEY_X: ia = 'x'; break;
				case KEY_Y: ia = 'y'; break;
				case KEY_Z: ia = 'z'; break;
				case KEY_BACKSPACE: ia = '\b'; break;
				}

				match = accelerator_test (info, fg_console,
					ia);
				if (match) {
					struct fbui_event ev;
					memset (&ev, 0, 
						sizeof (struct fbui_event));
					ev.type = FBUI_EVENT_ACCEL;
					ev.key = ia;
					fbui_enqueue_event (info,match, &ev, 1);
				}

				intercepting_accel = 1;
				return;
			}
		}

		if (intercepting_accel && code!=KEY_LEFTALT && 
		    code!=KEY_RIGHTALT) {
			intercepting_accel = 0;
			return;
		}

		if (!intercepting_accel) {
			if (down_read_trylock (&info->winptrSem)) {

				if ((code & 0xfff0) == BTN_MOUSE) {
					struct fbui_win *win;

					win = info->pointerfocus_window[cons];
					if (!win) {
						win = get_window_at_xy(info,
							info->curr_mouse_x,
							info->curr_mouse_y);
					}
					up_read (&info->winptrSem);

					if (win) {
						struct fbui_event ev;
						short tmp=0;

						memset (&ev, 0, 
						  sizeof (struct fbui_event));
						ev.type = FBUI_EVENT_BUTTON;
						ev.x = info->mouse_x0;
						ev.y = info->mouse_y0;
						ev.key = value ? 1 : 0;

						switch (code) {
						case BTN_LEFT:
							tmp = FBUI_BUTTON_LEFT;
							break;
						case BTN_MIDDLE:
							tmp= FBUI_BUTTON_MIDDLE;
							break;
						case BTN_RIGHT:
							tmp = FBUI_BUTTON_RIGHT;
							break;
						}
						ev.key |= tmp;
						fbui_enqueue_event (info, win, 
							&ev, 1);
					}
				}
				else
				{
					struct fbui_win *recipient =
						info->keyfocus_window [cons];
					up_read (&info->winptrSem);

					if (recipient) {
						struct fbui_event ev;
						memset (&ev, 0, 
						  sizeof (struct fbui_event));

						ev.type = FBUI_EVENT_KEY;
						ev.key = (code<<2) | (value&3);
						fbui_enqueue_event (info, 
							recipient, &ev, 1);
					} 
#if 0
					else
						printk (KERN_INFO "No keyfocus on console %d, so key %d,%d discarded\n",cons,code,value);
#endif
				}
			} else {
				/* XX need workaround */
				printk (KERN_INFO "key %d,%d inadvertently discarded\n",code,value);
			}
		}
		break;

	case EV_REL:
		/* We don't receive both x and y at once;
		 * we must wait for both to arrive.
		 */
		xlim = info->var.xres;
		ylim = info->var.yres;
		if (!got_rel_x && !got_rel_y) {
			incoming_x = info->curr_mouse_x;
			incoming_y = info->curr_mouse_y;
		}
		if (code == REL_X) {
			short x = incoming_x;
			if (got_rel_x && !got_rel_y)
				got_rel_y = 1;
			xlim--;
			x += value;
			if (x<0)
				x = 0;
			if (x>xlim)
				x = xlim;
			incoming_x = x;
			got_rel_x = 1;
		} else
		if (code == REL_Y) {
			short y = incoming_y;
			if (got_rel_y && !got_rel_x)
				got_rel_x = 1;
			ylim--;
			y += value;
			if (y<0)
				y = 0;
			if (y>ylim)
				y = ylim;
			incoming_y = y;
			got_rel_y = 1;
		}

		if (got_rel_x && got_rel_y) {
			int cons = fg_console;

			if (!info->pointer_active || info->pointer_hidden)
				return;

			/* Even if the new c cannot affect the
			 * display due to drawing that is happening, 
			 * we at least need to record them.
			 */
			info->curr_mouse_x = incoming_x;
			info->curr_mouse_y = incoming_y;

			if (down_read_trylock (&info->winptrSem)) {
				struct fbui_win *win = NULL;
				/*struct fbui_win *wm = NULL;*/
				struct fbui_win *pf = NULL;
				struct fbui_win *oldwin = 
					info->pointer_window [cons];
				struct fbui_processentry *pre = NULL;
				struct fbui_processentry *oldpre = NULL;
				char drawing=0;
				char focus=0;

				win = get_window_at_xy(info,
					info->curr_mouse_x,
					info->curr_mouse_y);
				if (win) {
					drawing = win->drawing;
					pre = win->processentry;
				}

				if (oldwin)
					oldpre = oldwin->processentry;

				/* Generate Motion for the window that has 
				 * pointer focus. Pointer focus is exclusive.
				 */
				pf = info->pointerfocus_window[cons];
				if (pf && pf->processentry) {
					struct fbui_event ev;
					focus = 1;
					memset (&ev, 0, 
						sizeof (struct fbui_event));
					ev.type = FBUI_EVENT_MOTION;
					ev.x = info->mouse_x0 - pf->x0;
					ev.y = info->mouse_y0 - pf->y0;
					fbui_enqueue_event (info, pf, &ev, 1);
				}
				
				/* generate Leave */
				if (oldwin && oldpre && win != oldwin &&
				   (oldpre->wait_event_mask & 
					FBUI_EVENTMASK_LEAVE)) {
					struct fbui_event ev;
					memset (&ev, 0, 
						sizeof (struct fbui_event));
					ev.type = FBUI_EVENT_LEAVE;

					if (!focus || oldwin==pf)
					 fbui_enqueue_event(info,oldwin,&ev, 1);

					oldwin->pointer_inside = 0;
					info->pointer_window [cons] = NULL;
				}

				/* generate Enter */
				if (win && pre && !win->pointer_inside
				    && (pre->wait_event_mask & 
				    FBUI_EVENTMASK_ENTER)){
					struct fbui_event ev;
					memset (&ev, 0, 
						sizeof (struct fbui_event));
					ev.type = FBUI_EVENT_ENTER;

					if (!focus || win==pf)
					 fbui_enqueue_event (info, win, &ev, 1);

					win->pointer_inside = 1;
					info->pointer_window [cons] = win;
				}

				/* If possible draw the pointer */
				if (!win || !drawing) {
					fbui_pointer_restore (info);
					info->mouse_x0 = incoming_x;
					info->mouse_y0 = incoming_y;
					info->mouse_x1 = info->mouse_x0 + 
						PTRWID - 1;
					info->mouse_y1 = info->mouse_y0 + 
						PTRHT - 1;
					fbui_pointer_save (info);
					fbui_pointer_draw (info);
				}

				/* Generate Motion for appropriate window */
				if (win && pre &&
				   (pre->wait_event_mask&FBUI_EVENTMASK_MOTION))
				{
					struct fbui_event ev;
					memset (&ev, 0, 
						sizeof (struct fbui_event));
					ev.type = FBUI_EVENT_MOTION;
					ev.x = info->mouse_x0 - win->x0;
					ev.y = info->mouse_y0 - win->y0;

					if (!focus || win==pf)
					 fbui_enqueue_event (info, win, &ev, 1);
				}

#if 0
/* XX for "xeyes" type of app, not just wm */
				wm = info->window_managers [cons];
				if (wm && wm->receive_all_motion) {
					struct fbui_event ev;
					memset (&ev, 0, 
						sizeof (struct fbui_event));
					ev.type = FBUI_EVENT_MOTION;
					ev.x = info->mouse_x0;
					ev.y = info->mouse_y0;
					fbui_enqueue_event (info, wm, &ev, 1);
				}
#endif
				up_read (&info->winptrSem);
			} 

			got_rel_x = 0;
			got_rel_y = 0;
		}
		break;
	}
}


int fbui_init (struct fb_info *info)
{
	int i,j;
	int bpp;
	char supported;
	short xres, yres;

	if (!info) 
		return 0;
	/*----------*/

	info->fbui_errno = 0;
	FBUI_ERROR_LOC(0);

	xres = info->var.xres;
	yres = info->var.yres;
	bpp = info->var.bits_per_pixel;

	supported = 0;

	if (bpp <= 4) {
#ifdef CONFIG_FB_UI_4BPP
		supported = 1;
#endif
	} else if (bpp <= 8) {
#ifdef CONFIG_FB_UI_8BPP
		supported = 1;
#endif
	} else if (bpp <= 16) {
#ifdef CONFIG_FB_UI_16BPP
		supported = 1;
#endif
	} else if (bpp <= 24) {
#ifdef CONFIG_FB_UI_24BPP
		supported = 1;
#endif
	} else {
#ifdef CONFIG_FB_UI_32BPP
		supported = 1;
#endif
	}

	i = FBUI_MAXCONSOLES * FBUI_MAXWINDOWSPERVC;
	j = FBUI_TOTALACCELS * FBUI_MAXCONSOLES;
	info->windows = myalloc (1, 63, 4 * i);
	info->accelerators = myalloc (1, 63, 4 * j);
	info->processentries = 
	  myalloc (1, 63, sizeof(struct fbui_processentry) * i);
	info->windowSems = myalloc (1, 63, sizeof(struct semaphore) * i);
	info->pointerSems = myalloc (1, 63, sizeof(struct semaphore) * 
		FBUI_MAXCONSOLES);
	fbui_total_allocated += 4 * i + 4 * j + 
	  sizeof(struct fbui_processentry) * i + sizeof(struct semaphore) * i;

	if (!info->windows || !info->accelerators || !info->processentries ||
	    !info->windowSems || !info->pointerSems) {
		if (info->windows)
			myfree (1, info->windows);
		if (info->processentries)
			myfree (1, info->processentries);
		if (info->accelerators)
			myfree (1, info->accelerators);
		if (info->windowSems)
			myfree (1, info->windowSems);
		fbui_total_allocated -= 4 * i + 4 * j + 
			sizeof(struct fbui_processentry) * i + 
			sizeof(struct semaphore) * i;

		printk(KERN_INFO "FrameBuffer UI (%s) by Zack T Smith: NOT operational due to lack of memory on fb%d\n",
			FBUI_VERSION, info->node);
		info->fbui = 0;
		return 0;
	}

	memset (info->windows, 0, 4*i);
	memset (info->accelerators, 0, 4*j);
	memset (info->processentries, 0, i*sizeof(struct fbui_processentry));
	memset (info->windowSems, 0, i*sizeof(struct semaphore));
	memset (info->pointerSems, 0,FBUI_MAXCONSOLES*sizeof(struct semaphore));

	info->fbui = supported;
	printk (KERN_INFO "FrameBuffer UI (%s) by Zack T Smith: ",FBUI_VERSION);
	if (!supported) {
		printk(KERN_INFO "unsupported depth of %dbpp on fb%d\n",
			bpp, info->node);
		return 0;
	} else {
		printk(KERN_INFO "operational on fb%d\n",
			info->node);
	}

	printk(KERN_INFO "   resolution %dx%d, %dbpp\n", xres, yres, bpp);

	for (i=0; i < FBUI_TOTALACCELS * FBUI_MAXCONSOLES; i++)
		info->accelerators [i] = NULL;

	for (i=0; i < (FBUI_MAXCONSOLES * FBUI_MAXWINDOWSPERVC); i++) {
		info->windows [i] = NULL;
		sema_init (&info->windowSems [i],1);
	}

	sema_init (&info->preSem,1);

	for (i=0; i<FBUI_MAXCONSOLES; i++) {
		info->force_placement [i] = 0;
		info->bgcolor[i] = RGB_STEELBLUE;
		info->pointer_window [i] = NULL;
		info->keyfocus_window [i] = NULL;
		info->pointerfocus_window [i] = NULL;
		info->window_managers [i] = NULL;
		info->bg_rects [i] = NULL;
		sema_init (&info->pointerSems[i],1);
	}

	init_rwsem (&info->winptrSem);
	info->mouse_x0 = xres >> 1;
	info->mouse_y0 = yres >> 1;
	info->curr_mouse_x = info->mouse_x0;
	info->curr_mouse_y = info->mouse_y0;
	info->mouse_x1 = info->mouse_x0 + PTRWID - 1;
	info->mouse_y1 = info->mouse_y0 + PTRHT - 1;
	info->pointer_active = 0;

	info->have_hardware_pointer = 0;

	init_rwsem (&info->cutpaste_sem);
	info->cutpaste_buffer = NULL;

	info->redsize = info->var.red.length;
	info->greensize = info->var.green.length;
	info->bluesize = info->var.blue.length;
	info->redshift = info->var.red.offset;
	info->greenshift = info->var.green.offset;
	info->blueshift = info->var.blue.offset;

	info->mode24 = (info->redsize == 8 && info->greensize == 8 &&
	    info->bluesize == 8 && info->redshift == 16 &&
	    info->greenshift == 8 && !info->blueshift);

	info->mode565 = (info->redsize == 5 && info->greensize == 6 &&
	    info->bluesize == 5 && info->redshift == 11 &&
	    info->greenshift == 5 && !info->blueshift);

	info->mode555 = (info->redsize == 5 && info->greensize == 5 &&
	    info->bluesize == 5 && info->redshift == 10 &&
	    info->greenshift == 5 && !info->blueshift);

	intercepting_accel = 0;
	altdown = 0;

#if 0
	printk(KERN_INFO "sizeof (struct fbui_win)=%d\n", sizeof (struct fbui_win));
	printk(KERN_INFO "sizeof (struct fbui_processentry)=%d\n",sizeof(struct fbui_processentry));
	printk(KERN_INFO "sizeof (struct fbui_rects)=%d\n", sizeof (struct fbui_rects));
#endif

	return 1;
}


/* overlap check
 * values are assumed to be sorted e.g. x0 < x1.
 */
static char overlap_check (short x0, short y0, short x1, short y1,
                           short x2, short y2, short x3, short y3)
{
	char a,b,c,d;
	char e,f,g,h;

	/*----------*/

	a = (x2 >= x0 && x2 <= x1);
	b = (x3 >= x0 && x3 <= x1);
	c = (x0 >= x2 && x0 <= x3);
	d = (x1 >= x2 && x1 <= x3);
	if (!(a || b || c || d))
		return 0;

	e = (y2 >= y0 && y2 <= y1);
	f = (y3 >= y0 && y3 <= y1);
	g = (y0 >= y2 && y0 <= y3);
	h = (y1 >= y2 && y1 <= y3);
	if (!(e || f || g || h))
		return 0;

#if 0
printk (KERN_INFO "OVERLAP %d,%d--%d,%d   %d,%d--%d,%d\n", x0,y0,x1,y1,x2,y2,x3,y3);
printk (KERN_INFO "a%d b%d c%d d%d e%d f%d g%d h%d\n",a,b,c,d,e,f,g,h);
#endif

	if (a && b && e && f)
		return 2; /* x2y2x3y3 is totally overlapped by x0y0x1y1 */
	if (c && d && g && h)
		return 3; /* x0y0x1y1 is totally overlapped by x2y2x3y3 */

	return 1; /* partial overlap */
}



static inline struct fbui_win *fbui_lookup_win (struct fb_info *info, 
		int win_id)
{
	struct fbui_win *win;

	if (!info) 
		return NULL;
	if (win_id < 0 || win_id >= (FBUI_MAXWINDOWSPERVC * FBUI_MAXCONSOLES))
		return NULL;
	/*----------*/

	down_read (&info->winptrSem);
	win = info->windows [win_id];
	up_read (&info->winptrSem);

	return win;
}


static struct fbui_rects* rects_new (u8 creator, u8 id)
{
	struct fbui_rects* nu;
	u32 size = sizeof (struct fbui_rects);

	nu = (struct fbui_rects*) myalloc (creator, id, size);
	if (nu) {
		memset (nu, 0, size);
		fbui_total_allocated += size;
#if 0
printk (KERN_INFO "+++ FBUI total allocated = %8lu\n", fbui_total_allocated);
#endif
	}

	return nu;
}

static void rects_delete (struct fbui_rects *r)
{
	while (r) {
		struct fbui_rects* next = r->next;
		myfree (2, r);
		fbui_total_allocated -= sizeof (struct fbui_rects);
		r = next;
	}
#if 0
printk (KERN_INFO "--- FBUI total allocated = %8lu\n", fbui_total_allocated);
#endif
}

static struct fbui_rects *rects_add (struct fbui_rects *r, 
				short x0, short y0, short x1, short y1, u8 id)
{
	struct fbui_rects *prev = r;
	u16 ix;

	if (!r)
		return NULL;

	if (x0 > x1) {
		short tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (y0 > y1) {
		short tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	/* Move to the next rectangle array, if one exists.
	 */
	while (r && r->total >= FBUI_RECT_ARYSIZE) {
		prev = r;
		r = r->next;
	}

	if (!r) {
		if (!prev) {
			printk (KERN_INFO "rects_add: prev=NULL!\n");
			return NULL;
		}

		r = prev->next = rects_new(2,id);
	}

	ix = r->total * 4;
	r->c [ix++] = x0;
	r->c [ix++] = y0;
	r->c [ix++] = x1;
	r->c [ix++] = y1;
	r->total++;

	return r;
}

static void rects_append (struct fbui_rects *dest, 
			       struct fbui_rects *src, u8 id)
{
	short x0,y0,x1,y1;
	int ix = 0;

	if (!dest || !src)
		return;

	while (src && src->total) {
		x0 = src->c[ix++];
		y0 = src->c[ix++];
		x1 = src->c[ix++];
		y1 = src->c[ix++];

		rects_add (dest, x0,y0,x1,y1, id);

		if (ix >= (src->total << 2)) {
			ix = 0;
			src = src->next;
		}
	}
}

static void rects_shift (struct fbui_rects *r, short dx, short dy)
{
	int ix = 0;

	if (!r || (!dx && !dy))
		return;

	while (r && r->total) {
		r->c[ix++] += dx;
		r->c[ix++] += dy;
		r->c[ix++] += dx;
		r->c[ix++] += dy;

		if (ix >= (r->total << 2)) {
			ix = 0;
			r = r->next;
		}
	}
}

static int rect_and (short ax0, short ay0, short ax1, short ay1,
		short bx0, short by0, short bx1, short by1,
		struct fbui_rects *result, u8 id)
{
	short x0,y0,x1,y1;

#if 0
printk (KERN_INFO "Entered rect_and, a= %d,%d,%d,%d b= %d,%d,%d,%d\n",
ax0,ay0,ax1,ay1,
bx0,by0,bx1,by1);
#endif
	if (!result) 
		return 0;
	if (ax0 > bx1 || bx0 > ax1 || ay0 > by1 || by0 > ay1)
		return 0;
	/*----------*/

	x0 = ax0 > bx0 ? ax0 : bx0;
	x1 = ax1 < bx1 ? ax1 : bx1;
	y0 = ay0 > by0 ? ay0 : by0;
	y1 = ay1 < by1 ? ay1 : by1;

	rects_add (result, x0,y0,x1,y1, id);
	return 1;
}

static void rects_clear_flags (struct fbui_rects *r)
{
	while (r) {
		r->flags = 0;
		r = r->next;
	}
}

/* Returns -1 if there is no intersection
 */
static int rect_subtract (struct fbui_rects *a, short aix,
		struct fbui_rects *b, short bix,
		struct fbui_rects *result, u8 id)
{
	int count=0;
	short ax0,ax1,ay0,ay1,bx0,by0,bx1,by1;

	if (!a || !b || !result || aix < 0 || bix < 0)
		return 0;
	if (aix >= 4*FBUI_RECT_ARYSIZE || bix >= 4*FBUI_RECT_ARYSIZE)
		return 0;
	/*----------*/

	ax0 = a->c[aix++];
	ay0 = a->c[aix++];
	ax1 = a->c[aix++];
	ay1 = a->c[aix];

	bx0 = b->c[bix++];
	by0 = b->c[bix++];
	bx1 = b->c[bix++];
	by1 = b->c[bix];

/* printk (KERN_INFO "rect_subtract: a=%d,%d--%d,%d  b=%d,%d--%d,%d\n",ax0,ay0,ax1,ay1,bx0,by0,bx1,by1); */

	/* No overlap */
	if (ax0 > bx1 || ax1 < bx0 || ay0 > by1 || ay1 < by0)
		return -1;

	/* B obscures A */
	if (ax0>=bx0 && ax1<=bx1 && ay0>=by0 && ay1<=by1)
		return 0;

#define between(N,M,O) ((N>M) && (N<O))

#if 0
printk (KERN_INFO "rect_subtract: a=%d,%d-%d,%d \n",ax0,ay0,ax1,ay1);
printk (KERN_INFO "               b=%d,%d-%d,%d\n",bx0,by0,bx1,by1);
#endif
	int i=-1, j=-1;
	if (bx0 <= ax0 && bx1 >= ax0 && bx1 < ax1) 
		i=1;
	else if (bx1 >= ax1 && bx0 > ax0 && bx0 <= ax1)
		i=3;
	else if (bx0>ax0 && bx0<ax1 && bx1>ax0 && bx1<ax1)
		i=2;
	else if (bx0 <= ax0 && bx1 >= ax1)
		i=5;
	else {
		printk (KERN_INFO "rect_subtract: i = -1\n");
		return 0;
	}

	if (by0 <= ay0 && by1 >= ay0 && by1 < ay1) 
		j=1;
	else if (by1 >= ay1 && by0 > ay0 && by0 <= ay1)
		j=3;
	else if (by0>ay0 && by0<ay1 && by1>ay0 && by1<ay1)
		j=2;
	else if (by0 <= ay0 && by1 >= ay1)
		j=5;
	else {
		printk (KERN_INFO "rect_subtract: j = -1\n");
		return 0;
	}

	switch (i) {
	case 1:
		switch (j) {
		case 1:
			count=2;
			rects_add (result, bx1+1,ay0,ax1,ay1, id); /*rt */
			rects_add (result, ax0,by1+1,bx1,ay1, id); /*btml*/
			break;
		case 2:
			count=3;
			rects_add (result, ax0,ay0,bx1,by0-1, id); /*topl*/
			rects_add (result, ax0,by1+1,bx1,ay1, id); /*btml*/
			rects_add (result, bx1+1,ay0,ax1,ay1, id); /*rt */
			break;
		case 3:
			count=2;
			rects_add (result, bx1+1,ay0,ax1,ay1, id); /*rt */
			rects_add (result, ax0,ay0,bx1,by0-1, id); /*topl*/
			break;
		case 5:
			count=1;
			rects_add (result, bx1+1,ay0,ax1,ay1, id);
			break;
		default:
			printk (KERN_INFO "combo %d %d\n",i,j);
		}
		break;

	case 2:
		switch (j) {
		case 1:
			count=3;
			rects_add (result, ax0,ay0,bx0-1,by1, id); /*topl*/
			rects_add (result, bx1+1,ay0,ax1,by1, id); /*topr*/
			rects_add (result, ax0,by1+1,ax1,ay1, id); /*btm */
			break;
		case 2:
			count=4;
			rects_add (result, ax0,ay0,bx0-1,ay1, id); /*lft*/
			rects_add (result, bx0,ay0,bx1,by0-1, id); /*top */
			rects_add (result, bx0,by1+1,bx1,ay1, id); /*btm */
			rects_add (result, bx1+1,ay0,ax1,ay1, id); /*rt */
			break;
		case 3:
			count=3;
			rects_add (result, ax0,ay0,ax1,by0-1, id); /*top */
			rects_add (result, ax0,by0,bx0-1,ay1, id); /*btml*/
			rects_add (result, bx1+1,by0,ax1,ay1, id); /*btmr*/
			break;
		case 5:
			count=2;
			rects_add (result, ax0,ay0,bx0-1,ay1, id);
			rects_add (result, bx1+1,ay0,ax1,ay1, id);
			break;
		default:
			printk (KERN_INFO "combo %d %d\n",i,j);
		}
		break;
	
	case 3:
		switch (j) {
		case 1:
			count=2;
			rects_add (result, ax0,ay0,bx0-1,ay1, id); /*lt*/
			rects_add (result, bx0,by1+1,ax1,ay1, id); /*btm */
			break;
		case 2:
			count=3;
			rects_add (result, ax0,ay0,ax1,by0-1, id); /*top */
			rects_add (result, ax0,by0,bx0-1,by1, id); /*mdl */
			rects_add (result, ax0,by1+1,ax1,ay1, id); /*btm */
			break;
		case 3:
			count=2;
			rects_add (result, ax0,ay0,bx0-1,ay1, id); /*lt*/
			rects_add (result, bx0,ay0,ax1,by0-1, id); /*topr*/
			break;
		case 5:
			count=1;
			rects_add (result, ax0,ay0,bx0-1,ay1, id);
			break;
		default:
			printk (KERN_INFO "combo %d %d\n",i,j);
		}
		break;

	case 5:
		switch (j) {
		case 1:
			count=1;
			rects_add (result, ax0,by1+1,ax1,ay1, id);
			break;
		case 2:
			count=2;
			/* split into two separate parts */
			rects_add (result, ax0,ay0,ax1,by0-1, id);
			rects_add (result, ax0,by1+1,ax1,ay1, id);
			break;
		case 3:
			count=1;
			rects_add (result, ax0,ay0,ax1,by0-1, id);
			break;
		default:
			printk (KERN_INFO "combo %d %d\n",i,j);
		}
		break;

	default:
		printk (KERN_INFO "combo %d %d\n",i,j);
	}

#if 0
	printk (KERN_INFO "==============================================\n");
	printk (KERN_INFO "rect_subtract: i=%d,j=%d begets %d\n", i,j,count);
	j=0;
	i = initial_ix ;
	while (i<result->total) {
		short x0,y0,x1,y1;
		x0=result->c[j++];
		y0=result->c[j++];
		x1=result->c[j++];
		y1=result->c[j++];
		i++;
		printk(KERN_INFO "            rect %d,%d-%d,%d\n", x0,y0,x1,y1);
	}
#endif
	return count;
}


static struct fbui_rects * rects_subtract (struct fbui_rects *a, 
						     struct fbui_rects *b,
						     u8 id)
{
	struct fbui_rects *src;
	u16 aix=0, bix;
	char any_overlap=0;

	if (!a || !b)
		return NULL;

	src = rects_new(3,id);
	rects_append (src, a, id);

	/* Subtract each of the b rects from all of the rects in 
	 * the source list, forming a result which will become the new
	 * source list in every next outer list cycle.
	 */
	bix = 0;
	while (b) {
		struct fbui_rects *new_a = rects_new(4,id);
		struct fbui_rects *orig_src;
		char overlap=0;

		rects_clear_flags (src);
		
		aix = 0;
		orig_src = src;
		while (src) {
			int n = rect_subtract (src, aix, b, bix, new_a, id);
			if (n >= 0) {
				overlap=1;
				any_overlap=1;

				/* Set the flag for this rect, indicating we 
				 * have found an overlap for it.
				 */
				src->flags |= 1 << (aix >> 2);
			} 

			aix += 4;
			if (aix >= (src->total << 2)) {
				aix = 0;
				src = src->next;
			}
		}
		src = orig_src;

		/* Now, all the rects in src that were not
		 * overlapped at all by the current b rectangle
		 * must be added to new_a.
		 */
		if (overlap) {
			while (src) {
				int i=0;
				for (i=0; i<src->total; i++) {
					if (!((1 << i) & src->flags)) {
						short sx0, sy0, sx1, sy1;
						aix = i<<2;
						sx0 = src->c[aix++];
						sy0 = src->c[aix++];
						sx1 = src->c[aix++];
						sy1 = src->c[aix];
						rects_add (new_a, 
							sx0,sy0,sx1,sy1, id);
					}
				}
				src = src->next;
			}

			rects_delete (orig_src);
			src = new_a;
		} else {
			/* If there was zero overlap, we can just keep
			 * the current source list that we have.
			 */
			rects_delete (new_a);
		}

		bix += 4;
		if (bix >= (b->total << 2)) {
			bix = 0;
			b = b->next;
		}
	}

	if (!any_overlap) {
		rects_delete(src);
		src=NULL;
		/* printk (KERN_INFO "rects_subtract: no overlap whatsoever\n"); */
	}
#if 0
	else
		printk (KERN_INFO "rects_subtract: result has %d rects\n", src->total);
#endif

	return src;
}


static struct fbui_rects * rects_and (struct fbui_rects *a, 
					        struct fbui_rects *b,
						u8 id)
{
	struct fbui_rects *b2;
	struct fbui_rects *result;
	u16 aix=0, bix;
	int count=0;

	if (!a || !b)
		return NULL;

	result = rects_new(5,id);

	while (a && aix < (a->total << 2)) {
		short ax0, ay0, ax1, ay1;

		b2 = b;
		bix = 0;

		ax0 = a->c[aix++];
		ay0 = a->c[aix++];
		ax1 = a->c[aix++];
		ay1 = a->c[aix++];

		while (b2 && bix < (b2->total << 2)) {
			short bx0, by0, bx1, by1;

			bx0 = b2->c[bix++];
			by0 = b2->c[bix++];
			bx1 = b2->c[bix++];
			by1 = b2->c[bix++];

			count += rect_and (ax0,ay0,ax1,ay1,bx0,by0,bx1,by1,
				result, id);

/*printk (KERN_INFO "a %d,%d,%d,%d & b %d,%d,%d,%d => %d, result has %d\n", ax0,ay0,ax1,ay1,bx0,by0,bx1,by1,count,result->total);*/

			if (bix >= (b2->total << 2)) {
				b2 = b2->next;
				bix = 0;
			}
		}

		if (aix >= (a->total << 2)) {
			a = a->next;
			aix = 0;
		}
	}

	return result;
}



static short fbui_winsort (struct fb_info *info, int cons, short *result,
			struct fbui_win **wm_return,
			struct fbui_win *insert_me,
			struct fbui_win *remove_me,
			struct fbui_win *raise_me,
			struct fbui_win *lower_me)
{
	int i, lim;
	struct fbui_win *wm=NULL;
	struct fbui_win *overlay=NULL;
	short max_level;
	short offset = insert_me && !insert_me->is_wm && !insert_me->is_overlay ?
			1 : 0;
	char howmany=0;

	if (!info || !result)
		return -2;
	if (cons < 0 || cons >= FBUI_MAXCONSOLES)
		return -2;
	if (insert_me)
		howmany++;
	if (remove_me)
		howmany++;
	if (raise_me)
		howmany++;
	if (lower_me)
		howmany++;
	if (howmany>1)
		return -2;
	if (remove_me)
		offset = 0;
	/* wm_return can be NULL */
	/*----------*/

	memset (result, 0xffff, (sizeof(short) * FBUI_MAXWINDOWSPERVC));
	if (offset) {
		result[0] = insert_me->id;
		insert_me->level = 0;
	}

	/* Process any raise */
	if (raise_me) {
		i = cons * FBUI_MAXWINDOWSPERVC;
		lim = i + FBUI_MAXWINDOWSPERVC;
		while (i < lim) {
			struct fbui_win *p = info->windows [i++];
			if (!p || p->is_wm || p->is_overlay)
				continue;
			if (p->level < raise_me->level)
				++p->level;
		}
		raise_me->level = 0;
	}

	/* Process any lower */
	if (lower_me) {
		max_level = 0;
		i = cons * FBUI_MAXWINDOWSPERVC;
		lim = i + FBUI_MAXWINDOWSPERVC;
		while (i < lim) {
			struct fbui_win *p = info->windows [i++];
			if (!p || p->is_wm || p->is_overlay)
				continue;
			if (p->level > max_level)
				max_level = p->level;
			if (p->level > lower_me->level)
				p->level--;
		}
		lower_me->level = max_level;
	}

	max_level = insert_me ? 0 : -1;
	wm = NULL;
	i = cons * FBUI_MAXWINDOWSPERVC;
	lim = i + FBUI_MAXWINDOWSPERVC;
	while (i < lim) {
		struct fbui_win *p = info->windows [i++];
		short lev;

		if (!p || p == insert_me)
			continue;

		if (p->is_wm) {
			wm = p;
			p->level = -1;
			continue;
		}
		if (p->is_overlay) {
			overlay = p;
			p->unobscured = 1;
			p->level = -2;
			continue;
		}

		p->level += offset;
		lev = p->level;

		if (lev >= FBUI_MAXWINDOWSPERVC) {
			printk (KERN_INFO "fbui_winsort: too many windows! lev=%d\n",lev);
			break;
		}

		if (-1 != result [(unsigned short)lev]) {
			short ii = result [(unsigned short)lev];
			printk (KERN_INFO "fbui_winsort: array item %d already has %d in it : therefore clobbered!\n", lev,ii);
			break;
		}

		result [(unsigned short)lev] = p->id;

		if (lev > max_level)
			max_level = lev;
	}

	if (remove_me && remove_me != wm && remove_me != overlay) {
		short target = remove_me->level;
		i = cons * FBUI_MAXWINDOWSPERVC;
		lim = i + FBUI_MAXWINDOWSPERVC;

		while (i < lim) {
			struct fbui_win *p = info->windows [i++];
			if (!p || p->is_wm || p->is_overlay)
				continue;

			if (p->level > target)
				p->level--;
		}

		i = remove_me->level;
		while (i < max_level) {
			result [i] = result[i+1];
			i++;
		}
		max_level--;
	}

	if (wm_return)
		*wm_return = wm;

	if (wm)
		wm->level = max_level+1;

	return max_level;
}


static int fbui_reset_rects (struct fb_info *info, int cons, int limited,
				  u8 id)
{
	struct fbui_rects *r;
	int i, lim;
	char any=0;

	if (!info) {
		FBUI_ERROR_LOC(2);
		return FBUI_ERR_NULLPTR;
	}
	/*----------*/

	i = cons * FBUI_MAXWINDOWSPERVC;
	lim = i + FBUI_MAXWINDOWSPERVC;

	/* Reset all rects lists.
	 */
	while (i < lim) {
		struct fbui_win *p = info->windows [i++];
		if (!p || p->is_wm)
			continue;

		if (limited && !p->rectangle_work)
			continue;

		p->unobscured = 1;
		
		any = 1;
		r = p->rects;
		while (r) {
			r->total = 0;
			r = r->next;
		}

		r = p->rects;
		rects_add (r, p->x0, p->y0, p->x1, p->y1, id);
	}

	/* If no rects were needing work,
	 * then we can avoid working on the background
	 * as well.
	 */
	if (limited && !any)
		return FBUI_SUCCESS;

	/* XX need semaphore */
	r = info->bg_rects[cons];
	if (!r) {
		FBUI_ERROR_LOC(3);
		return FBUI_ERR_INTERNAL;
	}
	while (r) {
		r->total = 0;
		r = r->next;
	}
	r = info->bg_rects[cons];
	rects_add (r, 0, 0, info->var.xres-1, info->var.yres-1, id);

	return FBUI_SUCCESS;
}


/* Perform a redo of rectangle lists.
 */
static int fbui_redo_overlap (struct fb_info *info, int cons, 
	struct fbui_win *insert, 
	struct fbui_win *remove,
	struct fbui_win *raise,
	struct fbui_win *lower,
	struct fbui_win *hide,
	struct fbui_win *unhide)
{
	struct fbui_win *wm = NULL;
	short sorted_windows [FBUI_MAXWINDOWSPERVC];
	struct fbui_rects *bg = NULL;
	short max_level;
	int i, j;
	char do_all_windows=0;
	char howmany=0;
	struct fbui_win *w;
	short orig_level = -1;
	struct fbui_rects *orig_rects = NULL; /* hide / lower / remove */

#if 0
char *s=NULL;
w = insert?insert : (remove?remove: (raise?raise: (lower?lower: (hide?hide: (unhide?unhide: NULL)))));
s = insert?"insert" : (remove?"remove": (raise?"raise": (lower?"lower": (hide?"hide": (unhide?"unhide": "-")))));
printk(KERN_INFO "Entered fbui_redo_overlap: cons=%d, window=%d, function=%s\n",
	cons,w? w->id : -1, s);
#endif

	if (!info) {
		FBUI_ERROR_LOC(4);
		return FBUI_ERR_NULLPTR;
	}

	if (insert)
		howmany++;
	if (remove)
		howmany++;
	if (raise)
		howmany++;
	if (lower)
		howmany++;
	if (hide)
		howmany++;
	if (unhide)
		howmany++;
	if (howmany>1) {
		FBUI_ERROR_LOC(5);
		return FBUI_ERR_BADPARAM;
	}
	if (cons < 0 || cons >= FBUI_MAXCONSOLES) {
		FBUI_ERROR_LOC(6);
		return FBUI_ERR_BADPARAM;
	}
	/*----------*/

/* printk (KERN_INFO "fbui_redo_overlap-------------------begin\n"); */

	w = insert? insert : (remove? remove : (raise ? raise : 
		(lower? lower : (hide? hide : (unhide? unhide : NULL)))));
	if (!w)
		do_all_windows = 1;
	else
		orig_level = w->level; /* needed for lower */

	if (lower || remove || hide) {
		orig_rects = rects_new (0,0);
		rects_append (orig_rects, w->rects, 0);
	}

	max_level = fbui_winsort (info, cons, sorted_windows, &wm, 
				  insert, remove, raise, lower);
	if (max_level == -2) {
		FBUI_ERROR_LOC(7);
		return FBUI_ERR_INTERNAL;
	}

	if (hide)
		w->is_hidden = 1;
	else
	if (unhide)
		w->is_hidden = 0;

	/* In most cases a hidden window lets us avoid much work. 
	 */ 
	if ((insert || remove || raise || lower) && w->is_hidden)
		return FBUI_SUCCESS;

	/* Determine which windows need new rectangle lists.
	 */
	if (max_level >= 0 && !do_all_windows) {
		int lim;
		short x0,y0,x1,y1;

		x0 = w->x0;
		y0 = w->y0;
		x1 = w->x1;
		y1 = w->y1;

		i = FBUI_MAXWINDOWSPERVC * cons;
		lim = i + FBUI_MAXWINDOWSPERVC;
		while (i < lim) {
			struct fbui_win *p = info->windows[i++];
			if (!p)
				continue;

			p->rectangle_work = 0;

			if (p->is_hidden || p->is_wm || p->is_overlay || 
			    p == w) 
				continue;

			/* Windows above ours don't need new rect lists, unless
			 * we've just lowered the window & set the level to the
			 * max_level.
			 */
			if ((lower||hide||remove) && p->level < orig_level)
				continue;

			if (overlap_check (x0,y0,x1,y1,
			    p->x0,p->y0,p->x1,p->y1))
			{
/* if (lower) printk (KERN_INFO "window %d will need rectangle work due to lowering of window %d\n", p->id,w->id); */
				p->rectangle_work = 1;
				p->unobscured = 0;
			}
		}
	}

	if (w)
		w->rectangle_work = 1;

	i = fbui_reset_rects (info, cons, !do_all_windows, 57);
	if (i) {
		return i;
	}

	/* If we are not raising or lowering, 
	 * generate the background rectangle list.
	 */
	if (!raise && !lower) {
		bg = info->bg_rects[cons];
		if (!bg) {
			FBUI_ERROR_LOC(150);
			return FBUI_ERR_INTERNAL;
		}
		i=0;
		while (i <= max_level) {
			struct fbui_win *p;
			struct fbui_rects *r;
			int id;

			id = sorted_windows [i++];
			if (id < 0 || id >= (FBUI_MAXWINDOWSPERVC * FBUI_MAXCONSOLES)) {
				FBUI_ERROR_LOC(9);
				return FBUI_ERR_INTERNAL;
			}

			p = info->windows [id];
			if (!p) {
				FBUI_ERROR_LOC(10);
				return FBUI_ERR_INTERNAL;
			}
			if (p->is_wm || p->is_hidden || p->is_overlay ||
			    p == remove)
				continue;

			if (!p->rects) {
				FBUI_ERROR_LOC(11);
				return FBUI_ERR_INTERNAL;
			}

			r = rects_subtract (bg, p->rects, 52);
			if (r) {
				rects_delete (bg);
				bg = r;
			}
		}
		if (!bg) {
			FBUI_ERROR_LOC(8);
			return FBUI_ERR_INTERNAL;
		}
		info->bg_rects[cons] = bg;
	}

	/* For each window, subtract the rectangles of windows above it. 
	 */
	i=1;
	while (i <= max_level) {
		struct fbui_win *p;
		struct fbui_rects *r;
		int id;

		id = sorted_windows [i];
		if (id < 0) {
			FBUI_ERROR_LOC(12);
			return FBUI_ERR_INTERNAL;
		}

		p = info->windows [id];
		if (!p) {
			FBUI_ERROR_LOC(13);
			return FBUI_ERR_INTERNAL;
		}
		if (p->is_wm || p->is_hidden || p==remove || 
		    p->is_overlay ||
		     (!do_all_windows && !p->rectangle_work)) {
			i++;
			continue;
		}

		r = p->rects;
		if (!r) {
			FBUI_ERROR_LOC(151);
			return FBUI_ERR_INTERNAL;
		}
		j=0;
		while (j < i) {
			struct fbui_win *p2;
			struct fbui_rects *r2, *r3;
			int id2 = sorted_windows [j];
			if (id2 < 0) {
				FBUI_ERROR_LOC(14);
				return FBUI_ERR_INTERNAL;
			}

			p2 = info->windows [id2];
			if (!p2) {
				FBUI_ERROR_LOC(15);
				return FBUI_ERR_INTERNAL;
			}

			if (p2==p || p2->is_hidden || p2==remove) {
				j++;
				continue;
			}

			r3 = p2->rects;
			if (!r3) {
				FBUI_ERROR_LOC(16);
				return FBUI_ERR_INTERNAL;
			}

			r2 = rects_subtract (r, r3, p->id);
			if (r2) {
				rects_delete (r);
				r = p->rects = r2;
				p->unobscured = 0;
			}

			j++;
		}

		i++;
	}

	/* If we are doing an unhide, clear its background.
	 */
	if (raise || unhide) {
		if (cons == fg_console) {
			rects_fill (info, w->rects, w->bgcolor);
		}
	}

	if (raise) {
		struct fbui_event ev;
		memset (&ev, 0, sizeof (struct fbui_event));
		ev.type = FBUI_EVENT_RAISE;
		ev.id = raise->id;
		fbui_enqueue_event (info, raise, &ev, 0);
	}
	else
	if (lower) {
		struct fbui_event ev;
		memset (&ev, 0, sizeof (struct fbui_event));
		ev.type = FBUI_EVENT_LOWER;
		ev.id = lower->id;
		fbui_enqueue_event (info, lower, &ev, 0);
	}

	/* If we are doing a remove, hide or lower, clear the parts of windows
	 * that will be exposed, plus the background.
	 */
	if (remove || lower || hide) {
		struct fbui_rects *to_expose;
		int lim;

		i = FBUI_MAXWINDOWSPERVC * cons;
		lim = i + FBUI_MAXWINDOWSPERVC;

		while (i < lim) {
			struct fbui_win *p = info->windows[i++];

			if (!p || p == w || p->is_wm || p->is_hidden ||
		            p->is_overlay || !p->rectangle_work)
				continue;

			to_expose = rects_and (
				orig_rects, p->rects, p->id);

			if (!to_expose)
				continue;
			
			if (p->to_expose) {
				rects_append (p->to_expose, to_expose, 
					p->id);
				rects_delete (to_expose);
			} else
				p->to_expose = to_expose;

			if (p->to_expose) {
				struct fbui_event ev;
				if (cons == fg_console)
					rects_fill (info, p->to_expose,
							 p->bgcolor);

				memset (&ev, 0, sizeof (struct fbui_event));
				ev.type = FBUI_EVENT_EXPOSE;
				ev.id = p->id;
				ev.rects = p->to_expose;
				p->to_expose = NULL;
				fbui_enqueue_event (info, p, &ev, 0);
			}
		}

		if (!lower && cons == fg_console) {
			struct fbui_rects *rect;
			rect = rects_new (58,w->id);
			rects_add (rect, w->x0, w->y0, w->x1, w->y1, 58);

			to_expose = rects_and (rect,
				info->bg_rects[cons], 58);
			rects_delete (rect);

			background_fill (info, to_expose, info->bgcolor[cons]);

			rects_delete (to_expose);
		}
	}

	if (orig_rects)
		rects_delete (orig_rects);

#if 0
	printk (KERN_INFO "-------------------- window list ------------------\n");
	i=0;
	while (i <= max_level) {
		struct fbui_rects *r;
		struct fbui_win *p = info->windows 
						[(short) sorted_windows [i]];
		printk (KERN_INFO "level %2d = window id %d which has %d rects  %s\n",
				i, p->id, p->rects->total,
				p->is_hidden ? "HIDDEN" : "");
		i++;
	}
	printk (KERN_INFO "---------------------------------------------------\n");
#endif

	return FBUI_SUCCESS;
}


/* Put the given window on top of all other windows. Revise the masks.
 */
static int fbui_overlap_window (struct fb_info *info, struct fbui_win *win)
{
	int cons=-1, err=0;
	char hid=0;
	struct rw_semaphore *sem;

	if (!info || !win) {
		FBUI_ERROR_LOC(17);
		return FBUI_ERR_NULLPTR;
	}
	/*----------*/

	cons = win->console;

	if (win->console == fg_console)
		hid = pointer_hide_if_rects (info, win);

	sem = &info->winptrSem;
	down_write (sem);

	down (&info->windowSems [win->id]);

	err = fbui_redo_overlap (info, cons, win, NULL, NULL, NULL,
		NULL, NULL);
	
	up (&info->windowSems [win->id]);

	up_write (sem);

	if (hid)
		fbui_unhide_pointer (info);

	return err;
}


static int fbui_remove_window (struct fb_info *info, struct fbui_win *win)
{
	int cons=-1, err=0;
	struct rw_semaphore *sem;
	char hid=0;

	if (!info || !win) {
		FBUI_ERROR_LOC(17);
		return FBUI_ERR_NULLPTR;
	}
	/*----------*/

	cons = win->console;

	if (cons == fg_console)
		hid = pointer_hide_if_rects (info, win);

	sem = &info->winptrSem;
	down_write (sem);

	down (&info->windowSems [win->id]);

	err = fbui_redo_overlap (info, cons, NULL, win, NULL, NULL,
		NULL, NULL);
	
	up (&info->windowSems [win->id]);

	int i = cons * FBUI_MAXWINDOWSPERVC;
	int lim = i + FBUI_MAXWINDOWSPERVC;
	printk (KERN_INFO "After remove:\n");
	while (i<lim) {
		printk (KERN_INFO "ix %d: ", i);
		struct fbui_win * w = info->windows[i];
		if (!w) {
			printk (KERN_INFO "(no win)\n");
		} else {
			printk (KERN_INFO "id %d, level %d\n", w->id,w->level);
		}
		i++;
	}

	up_write (sem);

	if (hid)
		fbui_unhide_pointer (info);

	return err;
}



/* Move/resize a window, updating whichever masks are affected.
 */
static int fbui_moveresize_window (struct fb_info *info, 
	struct fbui_win *win,
	short nu_x0, short nu_y0, short nu_w, short nu_h)
{
	int i=0, lim=0, cons=-1;
	struct rw_semaphore *sem;
	short xres, yres;
	short old_x0, old_y0, old_x1, old_y1;
	char did_copyarea = 0;
	short nu_x1 = nu_x0 + nu_w - 1;
	short nu_y1 = nu_y0 + nu_h - 1;
	struct fbui_rects *area_to_clear = NULL;
	char hid_mouseptr = 0;

/*u32 ii = fbui_total_allocated;*/

	if (!info || !win) {
		FBUI_ERROR_LOC(18);
		return FBUI_ERR_NULLPTR;
	}
	if (nu_x1 < 0 || nu_y1 < 0) {
		FBUI_ERROR_LOC(19);
		return FBUI_ERR_BADPARAM;
	}
	xres = info->var.xres;
	yres = info->var.yres;
	if (nu_x0 >= xres || nu_y0 >= yres) {
		FBUI_ERROR_LOC(20);
		return FBUI_ERR_BADPARAM;
	}
	if (nu_x0 < 0)
		nu_x0 = 0;
	if (nu_y0 < 0)
		nu_y0 = 0;
	if (nu_x1 >= xres)
		nu_x1 = xres - 1;
	if (nu_y1 >= yres)
		nu_y1 = yres - 1;
	/*----------*/

	if (win->x0 == nu_x0 && win->x1 == nu_x1 &&
	    win->y0 == nu_y0 && win->y1 == nu_y1)
		return FBUI_SUCCESS;

	if (win->is_hidden || win->is_overlay) {
		struct fbui_event ev;
		short old_width = win->width;
		short old_height = win->height;
		
		win->x0 = nu_x0;
		win->y0 = nu_y0;
		win->x1 = nu_x1;
		win->y1 = nu_y1;
		win->width = nu_x1 - nu_x0 + 1;
		win->height = nu_y1 - nu_y0 + 1;
		win->need_placement = 0;

		fbui_clear (info, win);

		memset (&ev, 0, sizeof (struct fbui_event));
		ev.x = nu_x0;
		ev.y = nu_y0;
		ev.width = nu_x1-nu_x0+1;
		ev.height = nu_y1-nu_y0+1;
		ev.key = 0;

		if (ev.width != old_width || ev.height != old_height)
			ev.key = FBUI_SIZE_CHANGED;
		if (nu_x0 != win->x0 || nu_y0 != win->y0)
			ev.key = FBUI_POSITION_CHANGED;

		ev.type = FBUI_EVENT_MOVERESIZE;
		fbui_enqueue_event (info, win, &ev, 0);

		/* XX for an overlay window, we must set the
		 * hardware position registers.
		 */
		return FBUI_SUCCESS;
	}

	cons = win->console;

	if (cons == fg_console && !info->have_hardware_pointer && 
	    info->pointer_active && !info->pointer_hidden) 
	{
		hid_mouseptr = pointer_hide_if_rects (info, win);

		if (!hid_mouseptr) {
			hid_mouseptr = pointer_hide_if_touching (info,
				nu_x0, nu_y0, nu_x1, nu_y1);
		}
	}

	sem = &info->winptrSem;
	down_write (sem);

	if (cons == fg_console) {
		struct fbui_rects *tmp;
		struct fbui_rects *shifted = NULL;

		/* Firstly, determine whether the window contents can be copied
		 * in one go, and if so, do it.
		 */
		if (win->unobscured) {
			char unobscured_at_dest = 1;

			i = FBUI_MAXWINDOWSPERVC * cons;
			lim = i + FBUI_MAXWINDOWSPERVC;
			while (i < lim) {
				struct fbui_win *p = info->windows[i++];
				if (p && p->level < win->level &&
					!p->is_overlay && !p->is_wm &&
					!p->is_hidden &&
				    overlap_check (nu_x0,nu_y0,nu_x1,nu_y1,
					   p->x0,p->y0,p->x1,p->y1)) {

					/* XX This is only an approximation.
					 * Really there is need for a 
					 * rects_intersect() function.
					 */
					unobscured_at_dest = 0;
					break;
				}
			}

			/* Just because the window is unobscured where it starts
			 * doesn't mean it will be unobscured where it ends up.
			 */
			if (unobscured_at_dest && info->fbops->fb_copyarea2) {
				struct fb_draw params;
				short dest_width = nu_x1 - nu_x0 + 1;
				short dest_height = nu_x1 - nu_x0 + 1;

				params.x0 = win->x0;
				params.y0 = win->y0;
				params.x1 = win->x1;
				params.y1 = win->y1;
				params.x2 = nu_x0;
				params.y2 = nu_y0;
				params.clip_valid = 0;

				/* If window will shrink, copy less. */
				if (dest_width < win->width)
					params.x1 = win->x0 + dest_width - 1;
				if (dest_height < win->height)
					params.y1 = win->y0 + dest_height - 1;

				info->fbops->fb_copyarea2 (info, 
					&params);
				did_copyarea = 1;
			}
		}

		area_to_clear = rects_new (6,55);
		shifted = rects_new (7,56);
		if (!area_to_clear || !shifted) {
			up_write (sem);
			if (hid_mouseptr)
				fbui_unhide_pointer (info);

			if (area_to_clear)
				rects_delete (area_to_clear);
			if (shifted)
				rects_delete (shifted);

			FBUI_ERROR_LOC(21);
			return FBUI_ERR_NOMEM;
		}
		rects_append (area_to_clear, win->rects, 60);
		rects_append (shifted, win->rects, 59);
		rects_shift (shifted, nu_x0 - win->x0, nu_y0 - win->y0);
		tmp = rects_subtract (area_to_clear, shifted, win->id);
		rects_delete (shifted);
		if (tmp) {
			rects_delete (area_to_clear);
			area_to_clear = tmp;
		} 
	}

	old_x0 = win->x0;
	old_x1 = win->x1;
	old_y0 = win->y0;
	old_y1 = win->y1;

	win->x0 = nu_x0;
	win->y0 = nu_y0;
	win->x1 = nu_x1;
	win->y1 = nu_y1;

	i = fbui_redo_overlap (info, cons, NULL, NULL, NULL, NULL,
		NULL, NULL);
	if (i) {
		up_write (sem);
		if (hid_mouseptr)
			fbui_unhide_pointer (info);
		return i;
	}

	if (cons == fg_console) {
		struct fbui_rects *to_clear;

		/* Clear the parts of underlying windows that become
		 * exposed by our moving the present window.
		 */
		i = FBUI_MAXWINDOWSPERVC * cons;
		lim = i + FBUI_MAXWINDOWSPERVC;
		while (i < lim) {
			struct fbui_win *p = info->windows[i++];

			if (p)
				p->rectangle_work = 0;

			if (!p || p->is_hidden || p->is_wm || 
			    p->is_overlay)
				continue;
			if (p == win)
				continue;
			if (win->level > p->level)
				continue;
			if (p->y1 < old_y0 || p->y0 > old_y1)
				continue;
			if (p->x1 < old_x0 || p->x0 > old_x1)
				continue;

			to_clear = rects_and (area_to_clear,
						   p->rects,p->id);
			if (!to_clear)
				continue;
			if (!to_clear->total) {
				rects_delete (to_clear);
				continue;
			}

			rects_fill (info, to_clear, p->bgcolor);
			p->rectangle_work = 1;

			if (!p->to_expose)
				p->to_expose = to_clear;
			else {
				if (p->to_expose->total) {
					rects_delete (p->to_expose);
					p->to_expose = NULL;
					rects_delete (to_clear);
				} else {
					rects_delete (p->to_expose);
					p->to_expose = to_clear;
				}
			}
		}

		/* Clear the part of the background that will be
		 * uncovered.
		 */
		to_clear = rects_and (area_to_clear,
					   info->bg_rects[cons],
					   50);
		if (to_clear && to_clear->total)
		{
			if (cons == fg_console)
				background_fill (info, to_clear, 
					info->bgcolor[cons]);

			rects_delete (to_clear);
			to_clear = NULL;
		}

		i = FBUI_MAXWINDOWSPERVC * cons;
		lim = i + FBUI_MAXWINDOWSPERVC;
		while (i < lim) {
			struct fbui_win *p = info->windows[i++];

			if (p && p->rectangle_work && !p->is_wm &&
			    !p->is_overlay) {
				struct fbui_event ev;
				memset (&ev, 0, sizeof (struct fbui_event));
				ev.type = FBUI_EVENT_EXPOSE;
				ev.id = p->id;
				ev.rects = p->to_expose;
				p->to_expose = NULL;
				fbui_enqueue_event (info, p, &ev, 0);
			}
		}
	}

	up_write (sem);

	rects_delete (area_to_clear);

	win->need_placement = 0;

	if (!did_copyarea) {
		struct fbui_event ev;
		short old_width = old_x1 - old_x0;
		short old_height = old_y1 - old_y0;

		fbui_clear (info, win);

		memset (&ev, 0, sizeof (struct fbui_event));
		ev.x = nu_x0;
		ev.y = nu_y0;
		ev.width = nu_x1-nu_x0+1;
		ev.height = nu_y1-nu_y0+1;
		ev.key = FBUI_CONTENTS_CHANGED;

		if (ev.width != old_width || ev.height != old_height)
			ev.key |= FBUI_SIZE_CHANGED;
		if (nu_x0 != old_x0 || nu_y0 != old_y0)
			ev.key |= FBUI_POSITION_CHANGED;

		ev.type = FBUI_EVENT_MOVERESIZE; /* => redraw */
		fbui_enqueue_event (info, win, &ev, 0);

		/*ev.type = FBUI_EVENT_EXPOSE;
		fbui_enqueue_event (info, win, &ev, 0);
		*/

	} else {
		struct fbui_event ev;
		short old_width = old_x1 - old_x0;
		short old_height = old_y1 - old_y0;

		memset (&ev, 0, sizeof (struct fbui_event));

		if (ev.width != old_width && ev.height != old_height)
			ev.key = FBUI_SIZE_CHANGED;
		if (nu_x0 != old_x0 || nu_y0 != old_y0)
			ev.key |= FBUI_POSITION_CHANGED;

		/* Rule: If a window is moved using copyarea,
		 * and its size remains the same, therefore all of
		 * the window's contents are unaffected, don't report
		 * a position YY
		 */

		ev.x = nu_x0;
		ev.y = nu_y0;
		ev.width = nu_x1-nu_x0+1;
		ev.height = nu_y1-nu_y0+1;
		ev.type = FBUI_EVENT_MOVERESIZE;
		fbui_enqueue_event (info, win, &ev, 0);
	}

	if (hid_mouseptr) {
		fbui_unhide_pointer (info);
	}
/*printk (KERN_INFO ">>> FBUI memory diff = %ld\n", fbui_total_allocated-ii); */

	return FBUI_SUCCESS;
}


static void restore_vc (struct fb_info *info, int cons)
{
	struct fbui_rects *r;

	if (cons < 0 || cons >= FBUI_MAXCONSOLES)
		return;
	/*----------*/

	if ((r = info->bg_rects[cons])) {
		rects_delete (r);
		info->bg_rects[cons] = NULL;
	}

	if (cons != fg_console) {
		vc_cons[cons].d->port.tty = info->ttysave[cons];
		vc_cons[fg_console].d->vc_mode = KD_TEXT;
	} else {
		console_lock();
		vc_cons[cons].d->port.tty = info->ttysave[cons];
		vc_cons[fg_console].d->vc_mode = KD_TEXT;
		do_unblank_screen(1);
		redraw_screen (cons,0);
		console_unlock();
	}

	printk(KERN_INFO "FBUI: restored console %d to text.\n", cons);
}




static void fbui_wininfo_change (struct fb_info *info, int cons, int what)
{
	struct fbui_win *p;

	if (!info) 
		return;
	if (cons < 0 || cons >= FBUI_MAXCONSOLES)
		return;
	/*----------*/

	p = fbui_lookup_wm (info, cons);
	if (p) {
		struct fbui_event ev;
		memset (&ev, 0, sizeof (struct fbui_event));
		ev.type = what? FBUI_EVENT_NAMECHANGE : FBUI_EVENT_WINCHANGE;
		fbui_enqueue_event (info, p, &ev, 0);
	}
}


static int get_total_windows (struct fb_info *info, int cons)
{
	int i,lim,total;

	if (!info)
		return 0;

	total=0;
	i = FBUI_MAXWINDOWSPERVC * cons;
	lim = i + FBUI_MAXWINDOWSPERVC;
	down_read (&info->winptrSem);
	for ( ; i < lim; i++) {
		if (NULL != info->windows [i])
			total++;
	}
	up_read (&info->winptrSem);
	return total;
}


/* force => do it even if we are not wm or window owner.
 */
static int fbui_destroy_win (struct fb_info *info, short win_id, int force)
{
	struct fbui_win *win;
	struct fbui_processentry *pre;
	struct rw_semaphore *sem;
	int cons = 0;
	int i, lim;

	if (!info) {
		FBUI_ERROR_LOC(23);
		return FBUI_ERR_NULLPTR;
	}
	if (win_id < 0 || win_id >= (FBUI_MAXWINDOWSPERVC * FBUI_MAXCONSOLES)) {
		FBUI_ERROR_LOC(24);
		return FBUI_ERR_BADWIN;
	}
	/*----------*/

	sem = &info->winptrSem;
	down_read (sem);
	win = info->windows [win_id];
	up_read (sem);

	if (!win) {
		FBUI_ERROR_LOC(25);
		return FBUI_ERR_BADWIN;
	}

	cons = win->console;

	/* Verify that we're allowed to remove this window */
	if (!force && current->pid != win->pid) {
		struct fbui_win *wm = fbui_lookup_wm (info, win->console);
		if (!wm) {
			FBUI_ERROR_LOC(26);
			return FBUI_ERR_BADWIN;
		}
		if (current->pid != wm->pid) {
			FBUI_ERROR_LOC(27);
			return FBUI_ERR_BADWIN;
		}
	}

	/* Remove the window from the pile.
	 */
	fbui_remove_window (info, win);

	if (win->icon)
		kfree (win->icon);

	/* XX */
	if (win->drawing)
		printk(KERN_INFO "DRAWING DURING REMOVEWIN\n");

	down_write (sem);

	if (win == info->windows[win_id]) 
		info->windows [win_id] = NULL;
	else {
		printk (KERN_INFO "fbui_destroy_win: window[win->id] != win\n");
	}

	if (win->is_wm) 
		info->window_managers [cons] = NULL;

	if (info->pointerfocus_window[cons] == win)
		info->pointerfocus_window[cons] = NULL;

	/* XX need to search window list for another window that wants
	 * key focus, or reimplement a focus stack.
	 */
	if (info->keyfocus_window[cons] == win)
		info->keyfocus_window[cons] = NULL;

	if (info->pointer_window [cons] == win)
		info->pointer_window[cons] = NULL;

	/* Clear any accelerators tied to this window */
	i = FBUI_TOTALACCELS * cons;
	lim = i + FBUI_TOTALACCELS;
	for ( ; i < lim; i++) {
		if (info->accelerators[i] == win)
			info->accelerators[i] = NULL;
	}
	up_write (sem);

	/* Reduce the window count for this pid */
	pre = win->processentry;
	if (!pre)
		printk (KERN_INFO "fbui_destroy_win: window is missing processentry\n");
	else {
		if (pre->nwindows <= 0) {
			printk (KERN_INFO "fbui_destroy_win: processentry has invalid #windows=%d\n",
				pre->nwindows);
		} else
			pre->nwindows--;

		if (pre->nwindows <= 0)
			free_processentry (info,pre,1);
		else
			printk(KERN_INFO "fbui_destroy_win: processentry %d has #wins=%d\n",pre->index,
				pre->nwindows);
	}

	if (win->to_expose) {
printk(KERN_INFO"---------window still has rects to expose at destroywin---\n");
		rects_delete (win->to_expose);
	}

	if (!win->is_wm && win->rects)
		rects_delete (win->rects);

	myfree(3, win);
	fbui_total_allocated -= sizeof (struct fbui_win);

	/* Console empty? If so, restore textmode
	 */
	if (!get_total_windows (info, cons)) {
		printk (KERN_INFO "fbui: restoring to text mode on VC %d\n", cons);
		restore_vc (info,cons);
	} else
	if (!force) {
		fbui_wininfo_change (info,cons,0);
	}

	return FBUI_SUCCESS;
}


static inline struct fbui_win *
		fbui_create_win (struct fb_info *info, 
                              int cons, 
                              char autoplacement,
                              char hidden,
			      char is_wm)
{
	struct fbui_win *nu = NULL;
	struct fbui_processentry *pre = NULL;
	struct semaphore *sem = NULL;
	int pid = current->pid;
	short win_id;
	short i, lim;

	if (!info)
		return NULL;
	if (!info->fbui)
		return NULL;
	if (cons < 0 || cons >= FBUI_MAXCONSOLES)
		return NULL;
	/*----------*/

	/* Find an empty window array entry
	 */
	down_write (&info->winptrSem);
	i = cons * FBUI_MAXWINDOWSPERVC;
	lim = i + FBUI_MAXWINDOWSPERVC;
	win_id = -1;
	while (i < lim) {
		if (!info->windows [i]) {
			nu = myalloc (3, i, sizeof(struct fbui_win));
			if (!nu) {
				/* XX kprint ... */
				up_write (&info->winptrSem);
				return NULL;
			}
			fbui_total_allocated += sizeof(struct fbui_win);
			win_id = i;
			info->windows [i] = nu;
			break;
		}
		i++;
	}
	up_write (&info->winptrSem);

	if (i >= lim)
		return NULL;

	memset ((void*) nu, 0, sizeof(struct fbui_win));

	nu->id = win_id;

	nu->rects = rects_new(8,win_id);

	nu->level = 0;
	nu->unobscured = 1;

	nu->pid = pid;
	nu->console = cons;

	/* Lookup the appropriate processentry, or allocate one
	 * if necessary.
	 */
	sem = &info->preSem;
	down (sem);
	pre = NULL;
	i=0;
	while (i < FBUI_MAXCONSOLES * FBUI_MAXWINDOWSPERVC) {
		pre = &info->processentries[i];
		if (pre->in_use && pre->pid == pid) {
			nu->processentry = pre;

			/* Rule: each process' windows are limited to one VC
			 */
			if (pre->console != cons)
				nu->console = cons = pre->console;
			break;
		}
		pre = NULL;
		i++;
	}
	up (sem);

	if (!pre) {
		pre = alloc_processentry (info, pid, cons);
		if (pre)
			nu->processentry = pre;
	}
	/* If *still* null */
	if (!pre) {
		printk (KERN_INFO "fbui_create_win: cannot even allocate a processentry\n");
		info->windows [nu->id] = NULL;
		rects_delete (nu->rects);
		myfree (3, nu);
		fbui_total_allocated -= sizeof (struct fbui_win);
		return NULL;
	}
	else
		++pre->nwindows;

	nu->is_hidden = hidden;
	nu->need_placement = autoplacement;

	return nu;
}


static inline void
backup_vc (struct fb_info *info, int cons)
{
	struct tty_struct *tty;
	struct fbui_rects *r;

	if (!info || cons < 0 || cons >= FBUI_MAXCONSOLES)
		return;
	/*----------*/

	console_lock();
	vc_cons[fg_console].d->vc_mode = KD_GRAPHICS;
	tty = vc_cons[cons].d->port.tty;
	info->ttysave[cons] = tty;
	vc_cons[cons].d->port.tty = NULL;
	//info->cursor.enable = 0;
	do_blank_screen(1);
	console_unlock();

	if ((r = info->bg_rects[cons]))
		rects_delete (r);

	info->bg_rects[cons] = rects_new (11,51);
	rects_add (info->bg_rects[cons], 0,0, 
		info->var.xres-1, info->var.yres-1, 61);

	if (cons == fg_console) {
		local_clear (info, info->bgcolor[cons]);
		info->pointer_active = 0;
		fbui_enable_pointer(info);
	}
}


static int fbui_set_geometry (struct fb_info *info, struct fbui_win *win,
                              short x0, short y0, short x1, short y1);


/* Open a window
 */
int fbui_open (struct fb_info *info, struct fbui_open *p)
{
	struct fbui_win *win;
	struct fbui_win *wm;
	short cons=0;
	char auto_placed=0;
	char initially_hidden;
	short tmp1, tmp2;
	short xres, yres;

	if (!info || !p) {
		FBUI_ERROR_LOC(28);
		return FBUI_ERR_NULLPTR;
	}
	if (!info->fbui) {
		FBUI_ERROR_LOC(29);
		return FBUI_ERR_NOT_OPERATIONAL;
	}
	xres = info->var.xres;
	yres = info->var.yres;
	if (p->need_keys & 0xfe) {
		FBUI_ERROR_LOC(30);
		return FBUI_ERR_BADPARAM;
	}
	if (p->desired_vc < -1 || p->desired_vc >= FBUI_MAXCONSOLES) {
		FBUI_ERROR_LOC(32);
		return FBUI_ERR_BADPARAM;
	}
	if (p->req_control & 0xfe) {
		FBUI_ERROR_LOC(33);
		return FBUI_ERR_BADPARAM;
	}
	if (p->need_keys & 0xfe) {
		FBUI_ERROR_LOC(33);
		return FBUI_ERR_BADPARAM;
	}
	if (p->receive_all_motion & 0xfe) {
		FBUI_ERROR_LOC(34);
		return FBUI_ERR_BADPARAM;
	}
	if (p->doing_autoposition & 0xfe) {
		FBUI_ERROR_LOC(35);
		return FBUI_ERR_BADPARAM;
	}
	if (p->initially_hidden & 0xfe) {
		FBUI_ERROR_LOC(36);
		return FBUI_ERR_BADPARAM;
	}
	if (p->usermask) {
		FBUI_ERROR_LOC(37);
		return FBUI_ERR_INTERNAL;
	}
	tmp1 = p->program_type;
	if (tmp1 < 0 || tmp1 >= FBUI_PROGTYPE_LAST) {
		FBUI_ERROR_LOC(38);
		return FBUI_ERR_BADPARAM;
	}
/* printk (KERN_INFO "@ open, x0,y0=%d %d; x1,y1=%d %d\n", p->x0,p->y0,p->x1,p->y1); */
	tmp1 = p->width;
	tmp2 = p->height;
	if (tmp1 <= 0 || tmp2 <= 0) {
		FBUI_ERROR_LOC(39);
		return FBUI_ERR_BADPARAM;
	}
	if (tmp1 > p->max_width)
		p->width = p->max_width;
	if (tmp2 > p->max_width)
		p->height = p->max_height;
	/* Rule (temporary): Limit the window to being onscreen 
	 */
	tmp1 = p->x;
	tmp2 = p->y;
	if (tmp1 >= xres || tmp2 >= yres) {
		FBUI_ERROR_LOC(154);
		return FBUI_ERR_BADPARAM;
	}
	if (tmp1 + p->width > xres)
		p->width = xres - tmp1;
	if (tmp2 + p->height > yres)
		p->height = yres - tmp2;
	/*----------*/

	if (!fbui_handler_regd) {
		fbui_input_register_handler (&input_handler, (u32) info);
		fbui_handler_regd = 1;
	}

	initially_hidden = p->initially_hidden;

	cons = p->desired_vc;
	if (cons >= FBUI_MAXCONSOLES) {
		FBUI_ERROR_LOC(40);
		return FBUI_ERR_BADVC;
	}
	if (cons < 0) {
		cons = fg_console;
		if (cons < 0) {
			printk (KERN_INFO "currcon is negative\n");
		}
	}
	if (!vc_cons_allocated (cons)) {
printk(KERN_INFO "VC %d NOT ALLOCated, oddly enough\n", cons);
#if 0
		if (!vc_allocate (cons)) {
			FBUI_ERROR_LOC(41);
			return FBUI_ERR_BADVC;
		}
#endif
	}

	if (vc_cons[fg_console].d->vc_mode != KD_GRAPHICS) {
		backup_vc (info, cons);
		intercepting_accel = 0;
	}

	wm = fbui_lookup_wm (info, cons);

	if (p->req_control) {
		if (wm) {
			FBUI_ERROR_LOC(42);
			return FBUI_ERR_HAVEWM;
		}

		/* the window-manager process gets the full screen area */
		p->x = 0;
		p->y = 0;
		p->width = info->var.xres;
		p->height = info->var.yres;
		info->bgcolor[cons] = p->bgcolor;
	} else {
		if (p->program_type)
			auto_placed = 1;
		if (info->force_placement[cons])
			auto_placed = 1;
		if (!wm)
			auto_placed = 0;
		if (wm && !wm->doing_autopos)
			auto_placed = 0;
	} 

	win = fbui_create_win (info, cons, auto_placed, 
			       initially_hidden | auto_placed,
			       p->req_control);
	if (!win) {
		FBUI_ERROR_LOC(43);
		return FBUI_ERR_NOMEM;
	}

	win->need_keys = p->need_keys;
	win->receive_all_motion = p->receive_all_motion;
	win->need_placement = auto_placed;

	strncpy (win->name, p->name, FBUI_NAMELEN);
	strncpy (win->subtitle, p->subtitle, FBUI_NAMELEN);

/* printk(KERN_INFO "fbui_open: window %d successfully added, location %d %d, %dx%d\n", win->id,win->x0,win->y0,win->width,win->height);  */
	win->program_type = p->program_type;
	win->max_width = p->max_width;
	win->max_height = p->max_height;

	if (p->req_control) {
		win->is_wm = 1;
		if (p->doing_autoposition)
			win->doing_autopos = 1;

		down_write (&info->winptrSem);
		info->window_managers [cons] = win;
		up_write (&info->winptrSem);
	}

	if (!auto_placed && !initially_hidden && !p->req_control)
		fbui_set_geometry (info, win, p->x,p->y,p->x+p->width-1,
			p->y+p->height-1);
	else {
		/* Copy the numbers over verbatim since the wm may find them useful*/
		win->x0 = p->x;
		win->y0 = p->y;
		win->x1 = p->x + p->width - 1;
		win->y1 = p->y + p->height - 1;
		win->width = p->width;
		win->height = p->height;
	}

	if (!win->is_wm) {
		if (!win->rects) 
			win->rects = rects_new(9,win->id);
		rects_add (win->rects, win->x0, win->y0, win->x1, 
			win->y1, win->id);
	}

	/* Update window levels.
	 * If the new window isn't hidden, update all the windows' masks.
	 */
	if ((tmp1 = fbui_overlap_window (info, win)))
		printk (KERN_INFO "!!! overlap returned %d, loc = %d\n", tmp1, info->fbui_errloc);

	win->bgcolor = p->bgcolor;

	if (!win->is_wm && !auto_placed && !initially_hidden)
		fbui_clear (info, win);
	if (win->is_wm && cons == fg_console) {
		fbui_hide_pointer (info);
		background_fill (info, info->bg_rects[cons], win->bgcolor);
		fbui_unhide_pointer (info);
	}

	down_write (&info->winptrSem);
/* printk(KERN_INFO "fbui_open: cons=%d needkeys=%d keyfocus=%08lx %s %d\n", cons, win->need_keys, (unsigned long)p, p?p->name:"(none)", p?p->id:-1); */
	if (!info->keyfocus_window[cons] && win->need_keys)
		info->keyfocus_window[cons] = win;
	up_write (&info->winptrSem);

	/* Finally, when the window has been created,
	 * its dimensions are set and it has a mask,
	 * we can create events for it.
	 */
	if (!auto_placed && !initially_hidden) {
		struct fbui_event ev;
		memset (&ev, 0, sizeof (struct fbui_event));
		ev.type = FBUI_EVENT_EXPOSE;
		fbui_enqueue_event (info, win, &ev, 0);
	}

	fbui_wininfo_change (info,cons,0);

	return win->id;
}


int fbui_close (struct fb_info *info, short id)
{
	struct fbui_win *win;
	int i;

	if (!info) {
		FBUI_ERROR_LOC(44);
		return FBUI_ERR_NULLPTR;
	}
	if (id < 0 || id >= (FBUI_MAXWINDOWSPERVC * FBUI_MAXCONSOLES)) {
		FBUI_ERROR_LOC(45);
		return FBUI_ERR_BADWIN;
	}
	/*----------*/

	win = fbui_lookup_win (info, id);
	if (!win) {
		FBUI_ERROR_LOC(46);
		return FBUI_ERR_NOTOPEN;
	}

	i = fbui_destroy_win (info, id, 0);

	return i;
}


/* "redraw" forced by window-manager process
 */
int fbui_redraw (struct fb_info *info, struct fbui_win *self, 
		 struct fbui_win *win)
{
	struct fbui_event ev;

	if (!info || !self || !win) {
		FBUI_ERROR_LOC(47);
		return FBUI_ERR_NULLPTR;
	}
	if (!self->is_wm) {
		FBUI_ERROR_LOC(48);
		return FBUI_ERR_NOTWM;
	}
	/*----------*/

	fbui_clear (info, win);

	memset (&ev, 0, sizeof (struct fbui_event));
	ev.type = FBUI_EVENT_EXPOSE;
	fbui_enqueue_event (info, win, &ev, 0);

	return FBUI_SUCCESS;
}


/* Verify that the process entry array doesn't contain killed processes,
 *   remove any such entries.
 * Verify that the windows on the system are for processes that exist.
 *   remove any such windows.
 */
static int fbui_clean (struct fb_info *info, int cons)
{
	struct semaphore *sem;
        struct rw_semaphore *rwsem;
	int i, lim;

	if (!fbui_handler_regd)
		return FBUI_SUCCESS;
	if (!info) {
		FBUI_ERROR_LOC(49);
		return FBUI_ERR_NULLPTR;
	}
	if (cons < 0 || cons >= FBUI_MAXCONSOLES) {
		FBUI_ERROR_LOC(50);
		return FBUI_ERR_BADPARAM;
	}
	/*----------*/
/*
Process entry check
	sem = &info->preSem;
	down(sem);
	i=0;
	while (i < FBUI_MAXCONSOLES * FBUI_MAXWINDOWSPERVC) {
		struct fbui_processentry *pre = &info->processentries[i];

		if (pre->in_use) {
			if (!process_exists (pre->pid)) {
				free_processentry (info, pre, 0);
			}
		}
		i++;
	}
	up(sem);
/*
* Window struct check
	rwsem = &info->winptrSem;
	i = cons * FBUI_MAXWINDOWSPERVC;
	lim = i + FBUI_MAXWINDOWSPERVC;
	while (i < lim) {
		struct fbui_win *win;

		down_read (rwsem);
		win = info->windows [i];
		up_read (rwsem);

		if (win) {
			if (!process_exists (win->pid)) {
				short id = win->id;

				if (id == i) {
					printk (KERN_INFO "fbui_clean: removing zombie window %d\n", i);
					fbui_destroy_win (info, i, 1);
				} else
					printk (KERN_INFO "fbui_clean: invalid window entry for window %d\n", id);
			}
		}
		i++;
	}
*/
	return FBUI_SUCCESS;
}


/* Return the process ids of all windows for this console
 * npids initially tells the maximum we can return.
 */
int fbui_window_info (struct fb_info *info, int cons, 
	/*user*/ struct fbui_wininfo *ary, int ninfo)
{
	struct fbui_win *win;
	int i, j, lim;

	if (!info || !ary || ninfo <= 0) {
		FBUI_ERROR_LOC(51);
		return FBUI_ERR_NULLPTR;
	}
	if (!access_ok ((void*)ary, 
	    ninfo * sizeof(struct fbui_wininfo))) {
		FBUI_ERROR_LOC(52);
		return FBUI_ERR_BADADDR;
	}
	if (cons < 0 || cons >= FBUI_MAXCONSOLES) {
		FBUI_ERROR_LOC(53);
		return FBUI_ERR_BADPARAM;
	}
	/*----------*/

	/* Clean up the console */
	fbui_clean (info, cons);

	i = 0;
	j = cons * FBUI_MAXWINDOWSPERVC;
	lim = j + FBUI_MAXWINDOWSPERVC;
	down_read (&info->winptrSem);
	while (i < ninfo && j < lim) {
		win = info->windows [j];
		if (win && !win->is_wm) {
			struct fbui_wininfo wi;
			char *ptr;
#if 0
printk(KERN_INFO "            : window %d/%s ", win->id,win->name);
printk(KERN_INFO "            : %s %s type=%d\n", 
	win->is_hidden?"hidden":"", 
	win->need_keys?"need_keys":"",
	win->program_type);
#endif
			wi.id = win->id;
			wi.pid = win->pid;
			wi.program_type = win->program_type;
			wi.hidden = win->is_hidden;
			wi.need_placement = win->need_placement;
			wi.need_keys = win->need_keys;
			wi.x = win->x0;
			wi.y = win->y0;
			wi.width = win->width;
			wi.height = win->height;
			wi.max_width = win->max_width;
			wi.max_height = win->max_height;
/* printk(KERN_INFO "inside wininfo: x %d y %d w %d h %d\n",(int)wi.x,(int)wi.y,(int)wi.width, (int)wi.height);  */
			strcpy (wi.name, win->name);
			strcpy (wi.subtitle, win->subtitle);

			ptr = (char*) &ary[i];

			if (copy_to_user (ptr, &wi, 
				sizeof(struct fbui_wininfo))) {
			    up_read (&info->winptrSem);
				FBUI_ERROR_LOC(54);
				return FBUI_ERR_BADADDR;
			}

			i++;
		}
		j++;
	}
	up_read (&info->winptrSem);

	return i;
}


/* Delete a window.
 */
int fbui_delete_window (struct fb_info *info, struct fbui_win *self, 
		 struct fbui_win *win)
{
	int rv;
	short id;
	char hid=0;
	struct semaphore *sem;

	if (!info || !self || !win) {
		FBUI_ERROR_LOC(55);
		return FBUI_ERR_NULLPTR;
	}
	if (!self->is_wm) {
		FBUI_ERROR_LOC(156);
		return FBUI_ERR_NOTWM;
	}
	/*----------*/

	id = win->id;
	sem = &info->windowSems [id];

	if (win->console == fg_console)
		hid = pointer_hide_if_rects (info, win);

	/* XX Need to send event to owner.
	 */
	down (sem);
	rv = fbui_destroy_win (info, win->id, 0);
	up (sem);
	
	if (hid)
		fbui_unhide_pointer (info);

	return FBUI_SUCCESS;
}


/* Raise a window to the top of the pile.
 */
int fbui_raise_window (struct fb_info *info, struct fbui_win *win)
{
	short id;
	struct semaphore *winsem;
	char hid=0;
	int err;

	if (!info || !win) {
		FBUI_ERROR_LOC(55);
		return FBUI_ERR_NULLPTR;
	}
	if (win->is_wm) {
		FBUI_ERROR_LOC(155);
		return FBUI_ERR_BADPARAM;
	}
	/*----------*/

	id = win->id;

	if (win->console == fg_console)
		hid = pointer_hide_if_touching (info, win->x0, win->y0,
						win->x1, win->y1);

	winsem = &info->windowSems [id];
	down (winsem);

	err = fbui_redo_overlap (info, win->console, NULL, NULL, win, NULL,
		NULL, NULL);

	up (winsem);
	
	if (hid)
		fbui_unhide_pointer (info);

	return err;
}


/* Lower a window to the top of the pile.
 */
int fbui_lower_window (struct fb_info *info, struct fbui_win *win)
{
	short id;
	struct semaphore *winsem;
	int err;
	char hid=0;

	if (!info || !win) {
		FBUI_ERROR_LOC(55);
		return FBUI_ERR_NULLPTR;
	}
	if (win->is_wm) {
		FBUI_ERROR_LOC(155);
		return FBUI_ERR_BADPARAM;
	}
	/*----------*/

	id = win->id;

	if (win->console == fg_console)
		hid = pointer_hide_if_rects (info, win);

	winsem = &info->windowSems [id];
	down (winsem);

	err = fbui_redo_overlap (info, win->console, NULL, NULL, NULL, win,
		NULL, NULL);

	up (winsem);

	if (hid)
		fbui_unhide_pointer (info);

	return err;
}



static struct fbui_win *
   accelerator_test (struct fb_info *info, int cons, unsigned char ch)
{
	if (!info) {
		FBUI_ERROR_LOC(57);
		return NULL;
	}
	if (ch >= FBUI_TOTALACCELS) {
		FBUI_ERROR_LOC(58);
		return NULL;
	}
	if (cons < 0 || cons >= FBUI_MAXCONSOLES) {
		FBUI_ERROR_LOC(59);
		return NULL;
	}
	/*----------*/

	return info->accelerators [ch + FBUI_TOTALACCELS*cons];
}


/* op==1 to register an accelerator (Alt-key sequence), 0 to unregister
 */
static int fbui_accel (struct fb_info *info, struct fbui_win *win,
	short which, short op)
{
	int ix;

	if (!info || !win) {
		FBUI_ERROR_LOC(60);
		return FBUI_ERR_NULLPTR;
	}
	if (which < 0 || which > 127 || op < 0 || op > 1) {
		FBUI_ERROR_LOC(61);
		return FBUI_ERR_BADPARAM;
	}
	/*----------*/

	ix = which + FBUI_TOTALACCELS * win->console;

	if (op==1) {
		if (accelerator_test (info, win->console, which)) {
			FBUI_ERROR_LOC(62);
			return FBUI_ERR_ACCELBUSY;
		}

		info->accelerators[ix] = win;
	} else
		info->accelerators[ix] = NULL;

	return FBUI_SUCCESS;
}


/* Does not generate WinChange event */
int fbui_hide_window (struct fb_info *info, struct fbui_win *win)
{
	struct fbui_event ev;
	struct semaphore *winsem;
	struct fbui_win *win2;
	char hid=0;
	int err;

	if (!info || !win) {
		FBUI_ERROR_LOC(63);
		return FBUI_ERR_NULLPTR;
	}
	if (win->is_hidden) {
		return FBUI_SUCCESS;
	}
	/*----------*/

	if (win->console == fg_console)
		hid = pointer_hide_if_rects (info, win);

	down_write (&info->winptrSem);

	winsem = &info->windowSems [win->id];
	down (winsem);

	err = fbui_redo_overlap (info, win->console, NULL, NULL, NULL, NULL,
		win, NULL);
	
	up (winsem);

	up_write (&info->winptrSem);

	memset (&ev, 0, sizeof (struct fbui_event));
	ev.type = FBUI_EVENT_HIDE;
	fbui_enqueue_event (info, win, &ev, 0);

	down_read (&info->winptrSem);
	win2 = info->pointer_window [fg_console];
	up_read (&info->winptrSem);

	if (hid)
		fbui_unhide_pointer (info);

	if (win2 == win) {
		memset (&ev, 0, sizeof (struct fbui_event));
		ev.type = FBUI_EVENT_LEAVE;
		fbui_enqueue_event (info, win, &ev, 0);
	}

	/* XX need to check for Enter */

	return FBUI_SUCCESS;
}


/* Does not generate WinChange event */
int fbui_unhide_window (struct fb_info *info, struct fbui_win *win)
{
	struct fbui_event ev;
	struct fbui_win *win2;
	struct rw_semaphore *sem;
	char hid=0;
	int err;

	if (!info || !win) {
		FBUI_ERROR_LOC(67);
		return FBUI_ERR_NULLPTR;
	}
	if (!win->is_hidden) {
		return FBUI_SUCCESS;
	}
	/*----------*/

	if (win->console == fg_console)
		hid = pointer_hide_if_rects (info, win);

	/* Rule: Don't allow unhide if the window will overlap.
	 */
	sem = &info->winptrSem;
	down_write (sem);

	down (&info->windowSems [win->id]);

	err = fbui_redo_overlap (info, win->console, NULL, NULL, NULL, NULL,
		NULL, win);

	up (&info->windowSems [win->id]);
	
	up_write (sem);

#if 0
	sem = &info->windowSems [win->id];
	down (sem);
	win->is_hidden = 0;
	fbui_clear (info, win);
	up (sem);
#endif

	if (hid)
		fbui_unhide_pointer (info);

	memset (&ev, 0, sizeof (struct fbui_event));
	ev.type = FBUI_EVENT_UNHIDE;
	fbui_enqueue_event (info, win, &ev, 0);
	ev.type = FBUI_EVENT_EXPOSE;
	fbui_enqueue_event (info, win, &ev, 0);
	
	down_read (&info->winptrSem);
	win2 = get_window_at_xy(info,
		info->curr_mouse_x,
		info->curr_mouse_y);
	up_read (&info->winptrSem);
	if (win2 == win) {
		memset (&ev, 0, sizeof (struct fbui_event));
		ev.type = FBUI_EVENT_ENTER;
		fbui_enqueue_event (info, win, &ev, 0);
	}

	return FBUI_SUCCESS;
}


static struct fbui_processentry *alloc_processentry (struct fb_info *info, 
						     int pid, int cons)
{
	int i;
	struct fbui_processentry *pre;
	struct semaphore *sem;
	DEFINE_SPINLOCK(queuelock);

	if (!info)
		return NULL;
	/*----------*/

	sem = &info->preSem;
	down(sem);
	i=0;
	while (i < FBUI_MAXCONSOLES * FBUI_MAXWINDOWSPERVC) {
		pre = &info->processentries[i];
		if (!pre->in_use) {
			pre->index = i;
			pre->waiting = 0;
			pre->pid = pid;
			pre->console = cons;
			pre->nwindows = 0;
			init_waitqueue_head(&pre->waitqueue);
			pre->window_num = -1;
			pre->queuelock = queuelock;
			sema_init (&pre->queuesem,1);
			pre->in_use = 1;
			pre->events = myalloc (9, 63,
			    sizeof (struct fbui_event) * 
				FBUI_MAXEVENTSPERPROCESS);
			pre->events_head = 0;
			pre->events_tail = 0;
			pre->events_pending = 0;
			up (sem);
			return pre;
		}
		i++;
	}
	up (sem);
	return NULL;
}

static void free_processentry (struct fb_info *info, 
			       struct fbui_processentry *pre,
			       char get_sem)
{
	struct semaphore *sem=NULL;
	struct fbui_event ev;

	if (!pre)
		return;
	/*----------*/

	while (fbui_dequeue_event (info, pre, &ev)) {
		if (ev.rects)
			rects_delete (ev.rects);
	}
	if (get_sem) {
		sem = &info->preSem;
		down (sem);
	}
	pre->in_use = 0;
	if (pre->events)
		myfree (0, pre->events);
	pre->events = NULL;
	pre->waiting = 0;
	pre->pid = 0;
	pre->nwindows = 0;
	pre->events_head = 0;
	pre->events_tail = 0;
	pre->events_pending = 0;
	if (get_sem)
		up (sem);

	myalloc_dump ();
}


int fbui_set_icon (struct fb_info *info, struct fbui_win *win, u32 *pixels)
{
	u32 *p;
	u32 size= FBUI_ICON_WIDTH * FBUI_ICON_HEIGHT * ((FBUI_ICON_DEPTH+7)>>3);

	if (!info || !win || !pixels) {
		FBUI_ERROR_LOC(163);
		return FBUI_ERR_NULLPTR;
	}
	if (!access_ok ((void*)pixels, size))
		return -EFAULT;
	/*----------*/

	if (!(p = win->icon)) {
		p = win->icon = myalloc (0,0, size);
		if (!p)
			return FBUI_ERR_NOMEM;
	}

	if (copy_from_user (p, pixels, size))
		return -EFAULT;
	
	return FBUI_SUCCESS;
}


int fbui_get_icon (struct fb_info *info, struct fbui_win *win, u32 *pixels)
{
	u32 size= FBUI_ICON_WIDTH * FBUI_ICON_HEIGHT * ((FBUI_ICON_DEPTH+7)>>3);

	if (!info || !win || !pixels) {
		FBUI_ERROR_LOC(163);
		return FBUI_ERR_NULLPTR;
	}
	if (!win->icon)
		return FBUI_ERR_NOICON;
	if (!access_ok ((void*)pixels, size))
		return -EFAULT;
	/*----------*/

	if (copy_to_user (pixels, win->icon, size))
		return -EFAULT;
	
	return FBUI_SUCCESS;
}


int fbui_control (struct fb_info *info, struct fbui_ctrl *ctl)
{
	struct fbui_win *self=NULL;
	struct fbui_win *win=NULL;
	struct fbui_processentry *pre=NULL;
	int cons = -1;
	char cmd;
	short id;
	short width, height;
	struct fbui_event *event;
	unsigned char *pointer;
	u32 cutlen;

	if (!info || !ctl) {
		FBUI_ERROR_LOC(71);
		return FBUI_ERR_NULLPTR;
	}
	if (!info->fbui) {
		FBUI_ERROR_LOC(72);
		return FBUI_ERR_NOT_OPERATIONAL;
	}
	/*----------*/

	cmd = ctl->op;
	if (cmd == FBUI_NOOP)
		return FBUI_SUCCESS;

	if (cmd < 0 || (cmd > FBUI_BEEP && cmd < FBUI_WM_ONLY) || 
	    cmd > FBUI_XYTOWINDOW)
		return FBUI_ERR_BADCMD;

	if (cmd == FBUI_ERRNO) {
		u16 value = 0xff & -info->fbui_errno;
		value |= ((u16)info->fbui_errloc) << 8;
		info->fbui_errno = 0;
		FBUI_ERROR_LOC(0);
		return (int) value;
	}

	if (cmd == FBUI_BEEP) {
		fbui_mksound (ctl->x, ctl->y);
		return 0;
	}

	id = ctl->id;
	width = ctl->width;
	height = ctl->height;
	event = ctl->event;
	pointer = ctl->pointer;
	cutlen = ctl->cutpaste_length;

	/* Rule: If the app wants the current console #,
	 * it must pass in a negative window #.
	 */
	if (cmd == FBUI_GETCONSOLE && id < 0)
		return fg_console;

	if (id < 0) {
		FBUI_ERROR_LOC(73);
		return FBUI_ERR_BADPARAM;
	}

	self = fbui_lookup_win (info, id);
	if (!self) {
		FBUI_ERROR_LOC(74);
		return FBUI_ERR_MISSINGWIN;
	}

	pre = self->processentry;
	if (!pre) {
		FBUI_ERROR_LOC(76);
		return FBUI_ERR_MISSINGPROCENT;
	}
	if (pre->pid != current->pid) {
		FBUI_ERROR_LOC(75);
		return FBUI_ERR_BADPID;
	}

	if (self->is_wm && cmd == FBUI_XYTOWINDOW) {
		win = get_window_at_xy (info, ctl->x, ctl->y);
		return win ? win->id : -1;
	}

	cons = pre->console;

	/* Rule: If the app wants the console that this process is 
	 * actually using, which may not be the current console, 
	 * then it passes in a valid window #.
	 */
	if (cmd == FBUI_GETCONSOLE) {
		return cons;
	}

	if (cons < 0 || cons >= FBUI_MAXCONSOLES) {
		FBUI_ERROR_LOC(77);
		return FBUI_ERR_BADVC;
	}
	if (cons != id / FBUI_MAXWINDOWSPERVC) {
		FBUI_ERROR_LOC(78);
		return FBUI_ERR_BADVC;
	}

	/* Moveresize, raise, and lower can either be done by the wm, or by 
	 * the app.
	 */
        if (cmd == FBUI_MOVERESIZE || cmd == FBUI_RAISE || cmd == FBUI_LOWER ||
            cmd == FBUI_HIDE || cmd == FBUI_UNHIDE) {
		if (self->is_wm) {
			win = fbui_lookup_win (info, ctl->id2);
			if (!win) {
				FBUI_ERROR_LOC(79);
				return FBUI_ERR_BADPID;
			}
			if (win->console != cons) {
				FBUI_ERROR_LOC(80);
				return FBUI_ERR_WRONGWM;
			}
		} 
		else
			win = self;
	} 

	/* Commands that take a window parameter are
	 * used only by the window manager.
	 */
	if (cmd >= FBUI_WM_ONLY) {
		if (!self->is_wm) {
			FBUI_ERROR_LOC(81);
			return FBUI_ERR_NOTWM;
		}

		if ((cmd == FBUI_ASSIGN_PTRFOCUS || cmd == FBUI_ASSIGN_KEYFOCUS)
		    && ctl->id2 < 0) {
			win = NULL;
		} else 
		if (!(win = fbui_lookup_win (info, ctl->id2))) {
			FBUI_ERROR_LOC(82);
			return FBUI_ERR_BADPID;
		}

		if (win && win->console != cons) {
			FBUI_ERROR_LOC(83);
			return FBUI_ERR_WRONGWM;
		}
	} 

	switch (cmd) {
	case FBUI_REDRAW:
		return fbui_redraw (info, self, win);

	case FBUI_DELETE:
		return fbui_delete_window (info, self, win);

	case FBUI_MOVERESIZE:
		return fbui_moveresize_window (info, self->is_wm ? win : self, 
			ctl->x, ctl->y, width, height);

	case FBUI_RAISE:
		return fbui_raise_window (info, self->is_wm ? win : self);

	case FBUI_LOWER:
		return fbui_lower_window (info, self->is_wm ? win : self);

	case FBUI_HIDE:
		return fbui_hide_window (info, self->is_wm ? win : self);

	case FBUI_UNHIDE:
		return fbui_unhide_window (info, self->is_wm ? win : self);

	case FBUI_WININFO:
		return fbui_window_info (info, cons, ctl->info, ctl->ninfo);

	case FBUI_ACCEL:
		return fbui_accel (info, self, ctl->x, ctl->y);

	case FBUI_ICON:
		return fbui_set_icon (info, self, (u32*) pointer);

	case FBUI_GETICON:
		return fbui_get_icon (info, win, (u32*) pointer);

	case FBUI_GETDIMS: {
		long value;

		down (&info->windowSems [self->id]);

		if (self->need_placement)
			value = FBUI_ERR_NOTPLACED;
		else {
			value = self->width;
			value <<= 16;
			value |= self->height;
		}
		
		up (&info->windowSems [self->id]);
		return value;
	}

	case FBUI_READMOUSE: {
		long value = 0;
#if 0
		value = self->mouse_x;
		value <<= 16;
		value |= self->mouse_y;
#endif
		return value;
	}

	case FBUI_GETPOSN: {
		u32 value;

		down (&info->windowSems [self->id]);
		if (self->need_placement) {
			value = FBUI_ERR_NOTPLACED;
		} else {
			value = self->x0;
			value <<= 16;
			value |= self->y0;
		}

		up (&info->windowSems [self->id]);
		return value;
	}

	case FBUI_ASSIGN_KEYFOCUS:
		if (!win || (!win->is_hidden && win->need_keys && !win->is_wm)){
			down_write (&info->winptrSem);
			info->keyfocus_window [cons] = win;
			up_write (&info->winptrSem);
		}
		return FBUI_SUCCESS;

	case FBUI_ASSIGN_PTRFOCUS:
		if (!win || !win->is_hidden) {
			down_write (&info->winptrSem);
			info->pointerfocus_window [cons] = win;
			up_write (&info->winptrSem);
		}
		return FBUI_SUCCESS;

	case FBUI_SUBTITLE:
		strncpy (self->subtitle, ctl->string, FBUI_NAMELEN);
		fbui_wininfo_change (info, cons, 1);
		return FBUI_SUCCESS;

	case FBUI_POLLEVENT:
	case FBUI_WAITEVENT: {
		struct fbui_event ev;
		struct fbui_rects *ru = ctl->rects;
		struct fbui_rects *rk;

		if (!event) {
			FBUI_ERROR_LOC(86);
			return FBUI_ERR_NULLPTR;
		}

		if (!access_ok ((void*)event, 
				sizeof(struct fbui_event))) {
			FBUI_ERROR_LOC(87);
			return -EFAULT;
		}
		if (ru) {
			if (!access_ok ((void*)ru, 
					sizeof(struct fbui_rects))) {
				FBUI_ERROR_LOC(157);
				return -EFAULT;
			}
		}

		/* Rule: all windows for a process use the same event mask */
		pre->wait_event_mask = ctl->x;

		memset (&ev, 0, sizeof (struct fbui_event));

		if (fbui_dequeue_event (info, pre, &ev)) {
			if ((rk = ev.rects)) {
				if (ru) {
					u32 size = sizeof (struct 
						fbui_rects);

					/* Rule: If rects existed
					 * but user doesn't want them,
					 * that's OK, just delete them.
					 */
					if (copy_to_user (ru, rk, size)) {
						FBUI_ERROR_LOC(158);
						return -EFAULT;
					}
					ev.has_rects = 1;
					ev.rects = NULL;
				}
				rects_delete(rk);
			}

			if (copy_to_user (event, &ev, 
					  sizeof(struct fbui_event))) {
				FBUI_ERROR_LOC(159);
				return -EFAULT;
			}

			return FBUI_SUCCESS;
		}

		/* No event & polling means exit here */
		if (cmd == FBUI_POLLEVENT) {
			pre->waiting = 0;
			FBUI_ERROR_LOC(88);
			return FBUI_ERR_NOEVENT;
		}

		pre->waiting = 1;
		wait_event_interruptible (pre->waitqueue, 
					  pre->events_pending > 0);

		if (fbui_dequeue_event (info, pre, &ev)) {
			struct fbui_rects *rk;

			if ((rk = ev.rects)) {
				if (ru) {
					u32 size = sizeof (struct 
						fbui_rects);

					/* Rule: If rects existed
					 * but user doesn't want them,
					 * that's OK, just delete them.
					 */
					if (copy_to_user (ru, rk, size)) {
						FBUI_ERROR_LOC(160);
						return -EFAULT;
					}
					ev.has_rects = 1;
					ev.rects = NULL;
				}
				rects_delete(rk);
			}

			if (copy_to_user (event, &ev, 
					  sizeof(struct fbui_event))) {
				FBUI_ERROR_LOC(90);
				return -EFAULT;
			} 
			else
				return FBUI_SUCCESS;
		} else {
			/* Process woken up but no event pending. Caused by kill? */
			FBUI_ERROR_LOC(89);
			return FBUI_ERR_NOEVENT;
		}
	}

	case FBUI_READPOINT:
		if (!self->is_wm)
			return RGB_NOCOLOR;
		else
			return info->fbops->fb_read_point ? 
				info->fbops->fb_read_point (info, ctl->x, ctl->y) :
				RGB_NOCOLOR;

	case FBUI_PLACEMENT:
		if (self->is_wm) {
			info->force_placement[cons] = ctl->x?1:0;
			return FBUI_SUCCESS;
		} else {
			FBUI_ERROR_LOC(91);
			return FBUI_ERR_NOTWM;
		}

	case FBUI_CUT: { /* write to kernel buf */
		unsigned char *ptr;
		char err=0;
		if (!pointer) {
			FBUI_ERROR_LOC(92);
			return FBUI_ERR_NULLPTR;
		}

		if (cutlen <= 0) {
			FBUI_ERROR_LOC(93);
			return FBUI_ERR_BADPARAM;
		}

		if (!access_ok (pointer, cutlen)) {
			FBUI_ERROR_LOC(94);
			return FBUI_ERR_BADADDR;
		}

		down_write (&info->cutpaste_sem);
		ptr = myalloc (4, 63, cutlen);
		if (!ptr)
			err = 1;
		else {
			fbui_total_allocated += cutlen;

			if (copy_from_user (ptr, pointer, cutlen)) {
				err = 1;
				myfree (4, ptr);
				fbui_total_allocated -= cutlen;
			} else {
				/* success */
				if (info->cutpaste_buffer) {
					myfree (4, info->cutpaste_buffer);
					fbui_total_allocated -= 
						info->cutpaste_length;
				}
				info->cutpaste_buffer = ptr;
				info->cutpaste_length = cutlen;
			}
		}
		up_write (&info->cutpaste_sem);
		if (err) {
			FBUI_ERROR_LOC(95);
			return FBUI_ERR_CUTFAIL;
		} else
			return FBUI_SUCCESS;
	}

	case FBUI_CUTLENGTH:
		return info->cutpaste_length;

	case FBUI_PASTE: { /* read from kernel buf */
		unsigned char *ptr;
		char err = 0;

		if (!pointer) {
			FBUI_ERROR_LOC(96);
			return FBUI_ERR_NULLPTR;
		}

		if (cutlen <= 0) {
			FBUI_ERROR_LOC(97);
			return FBUI_ERR_BADPARAM;
		}

		if (!access_ok (pointer, cutlen))
			FBUI_ERROR_LOC(98);
			return FBUI_ERR_BADADDR;

		if (info->cutpaste_buffer) {
			myfree (4, info->cutpaste_buffer);
			fbui_total_allocated -= info->cutpaste_length;
		}
		down_read (&info->cutpaste_sem);
		ptr = info->cutpaste_buffer;
		if (ptr) {
			u32 len = sizeof(struct fbui_wininfo);
			if (cutlen < len)
				len = cutlen;
			if (copy_to_user (pointer, ptr, len))
				err = 1;
		} else 
			err = 1;
		up_read (&info->cutpaste_sem);
		if (err) {
			FBUI_ERROR_LOC(99);
			return FBUI_ERR_PASTEFAIL;
		}

		/* return byte count */
		if (!ptr) 
			return 0;
		else
			return cutlen;
	 } /* paste */

	default:
		FBUI_ERROR_LOC(100);
		return FBUI_ERR_INVALIDCMD;
	}
}


static char cmdinfo[] = 
{
        0,      /* none */
192+    6,      /* copy area    x0,y0,x1,y1,w,h*/
32+128+ 4,      /* point        x,y,color lo,hi*/
32+192+ 6,      /* line         x0,y0,x1,y1,color lo,hi*/
32+128+ 4,      /* lineto    x1,y1,color lo,hi*/
32+128+ 5,      /* hline        x0,y0,color lo,hi,x1*/
32+128+ 5,      /* vline        x0,y0,color lo,hi,y1*/
32+192+ 6,      /* rect         x0,y0,x1,y1,color lo,hi*/
32+192+ 6,      /* fill_rect    x0,y0,x1,y1,color lo,hi*/
        0,      /* clear */
192+    4,      /* clear rect	x0,y0,x1,y1 */
32+128+ 11,     /* put image 	x,y,ptr lo,hi, mode, width, height, xstart, ystart, xend, yend */
32+128+ 7,	/* put image full	x,y,ptr lo,hi, mode, width, height */
32+128+ 8,	/* put image full mono	x,y,ptr lo,hi, width, height, color lo,hi */
32+192+ 8,      /* fill_triangle	x0,y0,x1,y1,color lo,hi, x2,y2 */
};


/* This routine executes commands which can be 
 * safely ignored when a window is hidden, suspended, or
 * not in the foreground console.
 */
int fbui_exec (struct fb_info *info, short win_id, short n, unsigned char *arg)
{
	struct fbui_win *win=NULL;
	unsigned char *argmax = arg + n * 2;
	int result = FBUI_SUCCESS;
	char initially_hidden=0;

	if (!info || !arg) {
		FBUI_ERROR_LOC(101);
		return FBUI_ERR_NULLPTR;
	}
	if (!info->fbui) {
		FBUI_ERROR_LOC(102);
		return FBUI_ERR_NOT_OPERATIONAL;
	}
	if (win_id < 0 || win_id >= (FBUI_MAXWINDOWSPERVC * FBUI_MAXCONSOLES)) {
		FBUI_ERROR_LOC(103);
		return FBUI_ERR_BADWIN;
	}
	if (n < 0) {
		FBUI_ERROR_LOC(104);
		return FBUI_ERR_INVALIDCMD; /* XX */
	}
	/*----------*/

	if (!(win = fbui_lookup_win (info, win_id))) {
		FBUI_ERROR_LOC(105);
		return FBUI_ERR_BADWIN;
	}
	if (win->pid != current->pid) {
		FBUI_ERROR_LOC(106);
                return FBUI_ERR_BADWIN;
	}
	if (win->console != fg_console || win->is_hidden)
		return FBUI_SUCCESS;

	down (&info->windowSems [win->id]);
	win->drawing = 1;

	/* At this point, we have permission to use the window,
	 * but there is a chance that the wm has just deleted it.
	 */
	if (!fbui_lookup_win (info, win_id)) {
		FBUI_ERROR_LOC(161);
		return FBUI_ERR_WINDELETED;
	}

	initially_hidden = info->pointer_hidden;

	while (FBUI_SUCCESS == result && arg < argmax && !win->is_hidden)
	{
		struct fb_draw draw;
		struct fb_put put;
		unsigned short ary [20];
		short len;
		unsigned short cmd;
		unsigned char flags;
		unsigned short ix;

		put.location = FB_LOCATION_USER;

		if (get_user (cmd, arg)) {
			result = FBUI_ERR_BADADDR;
			break;
		}
		arg += 2;

		if (cmd > sizeof (cmdinfo)) {
			result = FBUI_ERR_INVALIDCMD;
			break;
		}

		flags = cmdinfo[cmd];
		if ((len = 2 * (flags & 31))) {
			if (copy_from_user (ary, arg, len)) {
				result = FBUI_ERR_BADADDR;
				break;
			}
		}
		arg += len;

		ix = 0;
		if (flags & 128) {
			put.x0 = draw.x0 = ary[ix++];
			put.y0 = draw.y0 = ary[ix++];
		}
		if (flags & 64) {
			draw.x1 = ary[ix++];
			draw.y1 = ary[ix++];
		}
		if (flags & 32) {
			u32 param32 = ary[ix+1];
			param32 <<= 16;
			param32 |= ary[ix];
			draw.color = param32;
			put.pixels = (unsigned char*) param32;
			ix+=2;
		}

		if (cmd == FBUI_LINETO) {
			draw.x1 = draw.x0;
			draw.y1 = draw.y0;
			draw.x0 = win->draw_x;
			draw.y0 = win->draw_y;
			cmd = FBUI_LINE;
		}

		switch(cmd) {
		case FBUI_CLEAR:
			result = fbui_clear (info,win);
			if (result)
				goto finished;
			break;

		case FBUI_POINT: 
			draw.x1 = draw.x0;
			draw.y1 = draw.y0;
			result = fbui_fill_rect (info,win,&draw);
			if (result)
				goto finished;
			break;

		case FBUI_TRIANGLE:
			draw.x2 = ary[ix++];
			draw.y2 = ary[ix++];
			result = fbui_fill_triangle (info,win,&draw);
			if (result)
				goto finished;
			break;

		case FBUI_LINE: 
			result = fbui_draw_line (info,win, &draw);
			if (result)
				goto finished;
			break;

		case FBUI_HLINE: 
			draw.x1 = ary[ix++];
			draw.y1 = draw.y0;
			result = fbui_fill_rect (info,win,&draw);
			if (result)
				goto finished;
			break;

		case FBUI_VLINE: 
			draw.y1 = ary[ix++];
			draw.x1 = draw.x0;
			result = fbui_fill_rect (info,win,&draw);
			if (result)
				goto finished;
			break;

		case FBUI_RECT: 
			result = fbui_draw_rect (info,win,&draw);
			if (result)
				goto finished;
			break;

		case FBUI_FILLRECT: 
			result = fbui_fill_rect (info,win,&draw);
			if (result)
				goto finished;
			break;

		case FBUI_CLEARAREA:
			draw.color = win->bgcolor;
			result = fbui_fill_rect (info,win,&draw);
			if (result)
				goto finished;
			break;

		case FBUI_IMAGE:
			put.type = ary[ix++];
			put.width = ary[ix++];
			put.height = ary[ix++];
			put.x1 = put.x0 + put.width - 1;
			put.y1 = put.y0 + put.height - 1;
			put.xstart = ary[ix++];
			put.ystart = ary[ix++];
			put.xend = ary[ix++];
			put.yend = ary[ix++];
			if (put.type < 0 || put.type > FB_IMAGETYPE_GREY) {
				result = FBUI_ERR_BADPARAM;
				goto finished;
			}
			result = fbui_put_image (info,win, &put);
			if (result)
				goto finished;
			break;

		case FBUI_FULLIMAGE:
			put.type = ary[ix++];
			put.width = ary[ix++];
			put.height = ary[ix++];
			put.x1 = put.x0 + put.width - 1;
			put.y1 = put.y0 + put.height - 1;
			put.xstart = 0;
			put.ystart = 0;
			put.xend = put.width - 1;
			put.yend = put.height - 1;
			if (put.type < 0 || put.type > FB_IMAGETYPE_GREY) {
				result = FBUI_ERR_BADPARAM;
				goto finished;
			}
			result = fbui_put_image (info,win, &put);
/* printk (KERN_INFO "put info: xy %d %d, ptr %08lx, wh %d %d, color %08lx\n", put.x0, put.y0, (unsigned long)put.pixels,put.width,put.height,(unsigned long)put.color); */
			if (result)
				goto finished;
			break;

		case FBUI_MONOIMAGE: /* Monochrome is a full-image put. */
			put.width = ary[ix++];
			put.height = ary[ix++];
			put.x1 = put.x0 + put.width - 1;
			put.y1 = put.y0 + put.height - 1;
			put.xstart = 0;
			put.ystart = 0;
			put.xend = put.width - 1;
			put.yend = put.height - 1;
			put.color = ary[ix+1];
			put.color <<= 16;
			put.color |= ary[ix];
			put.type = FB_IMAGETYPE_MONO;
			ix += 2;
			result = fbui_put_image (info,win, &put);
			if (result)
				goto finished;
			break;

		case FBUI_COPYAREA:
			draw.x2 = ary[ix++];
			draw.y2 = ary[ix++];
			result = fbui_copy_area (info,win, &draw);
			if (result)
				goto finished;
			break;

		} /* switch */

		win->draw_x = draw.x1;
		win->draw_y = draw.y1;
	}
finished:
	if (!initially_hidden && info->pointer_hidden)
		fbui_unhide_pointer (info);

	win->drawing = 0;
	up (&info->windowSems [win->id]);
	return result;
}




static int fbui_draw_line (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *p)
{
	unsigned int ix, lim;
	struct fbui_rects *r;
	short j;

	if (!info || !win || !p) {
		FBUI_ERROR_LOC(110);
		return FBUI_ERR_NULLPTR;
	}
	if (win->console != fg_console)
		return FBUI_SUCCESS;
	if (win->is_hidden)
		return FBUI_SUCCESS;
	if ((p->x0 < 0 && p->x1 < 0) ||
	    (p->y0 < 0 && p->y1 < 0) )
		return 0;
	j = win->width;
	if (p->x0 >= j && p->x1 >= j)
		return 0;
	j = win->height;
	if (p->y0 >= j && p->y1 >= j)
		return 0;
	/*----------*/

	p->x0 += win->x0;
	p->x1 += win->x0;
	p->y0 += win->y0;
	p->y1 += win->y0;

	if (!info->have_hardware_pointer && info->pointer_active && 
	    !info->pointer_hidden) {
		short x2 = p->x0;
		short x3 = p->x1;
		short y2 = p->y0;
		short y3 = p->y1;
		if (x2 > x3) {
			short tmp=x2;
			x2=x3;
			x3=tmp;
		}
		if (y2 > y3) {
			short tmp=y2;
			y2=y3;
			y3=tmp;
		}
		pointer_hide_if_touching (info, 
			x2 < win->x0 ? win->x0 : x2,
			y2 < win->y0 ? win->y0 : y2,
			x3 > win->x1 ? win->x1 : x3,
			y3 > win->y1 ? win->y1 : y3);
	}

	p->clip_valid = 1;

	r = win->is_wm ? info->bg_rects[fg_console] : win->rects;
	ix = 0;
	lim = r->total << 2;
	while (r && ix < lim) {
		p->clip_x0 = r->c[ix++];
		p->clip_y0 = r->c[ix++];
		p->clip_x1 = r->c[ix++];
		p->clip_y1 = r->c[ix++];

		info->fbops->fb_line (info, p);

		if (ix >= lim) {
			ix = 0;
			r = r->next;
		}
	}

	return FBUI_SUCCESS;
}


static int fbui_fill_triangle (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *p)
{
	unsigned int ix, lim;
	struct fbui_rects *r;
	short j;

	if (!info || !win || !p) {
		FBUI_ERROR_LOC(110);
		return FBUI_ERR_NULLPTR;
	}
	if (win->console != fg_console)
		return FBUI_SUCCESS;
	if (!info->fbops->fb_filltriangle)
		return FBUI_SUCCESS;
	if (win->is_hidden)
		return FBUI_SUCCESS;
	if ((p->x0 < 0 && p->x1 < 0 && p->x2 < 0) ||
	    (p->y0 < 0 && p->y1 < 0 && p->y2 < 0) )
		return 0;
	j = win->width;
	if (p->x0 >= j && p->x1 >= j && p->x2 >= j)
		return 0;
	j = win->height;
	if (p->y0 >= j && p->y1 >= j && p->y2 >= j)
		return 0;
	/*----------*/

	p->x0 += win->x0;
	p->y0 += win->y0;
	p->x1 += win->x0;
	p->y1 += win->y0;
	p->x2 += win->x0;
	p->y2 += win->y0;

	if (!info->have_hardware_pointer && info->pointer_active && 
	    !info->pointer_hidden) {
		short xmin = 32767, ymin = 32767;
		short xmax = -32768, ymax = -32768;
		if (p->x0 < xmin)
			xmin = p->x0;
		if (p->x1 < xmin)
			xmin = p->x1;
		if (p->x2 < xmin)
			xmin = p->x2;
		if (p->y0 < ymin)
			ymin = p->y0;
		if (p->y1 < ymin)
			ymin = p->y1;
		if (p->y2 < ymin)
			ymin = p->y2;
		if (p->x0 > xmax)
			xmax = p->x0;
		if (p->x1 > xmax)
			xmax = p->x1;
		if (p->x2 > xmax)
			xmax = p->x2;
		if (p->y0 > ymax)
			ymax = p->y0;
		if (p->y1 > ymax)
			ymax = p->y1;
		if (p->y2 > ymax)
			ymax = p->y2;
		pointer_hide_if_touching (info, 
			xmin, ymin, xmax, ymax);
	}

	p->clip_valid = 1;

	r = win->is_wm ? info->bg_rects[fg_console] : win->rects;
	ix = 0;
	lim = r->total << 2;
	while (r && ix < lim) {
		p->clip_x0 = r->c[ix++];
		p->clip_y0 = r->c[ix++];
		p->clip_x1 = r->c[ix++];
		p->clip_y1 = r->c[ix++];

		info->fbops->fb_filltriangle (info, p);

		if (ix >= lim) {
			ix = 0;
			r = r->next;
		}
	}

	return FBUI_SUCCESS;
}



static int fbui_draw_rect (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *p)
{
	int retval;
	struct fb_draw p2;

	if (!info || !win || !p) {
		FBUI_ERROR_LOC(117);
		return FBUI_ERR_NULLPTR;
	}
	if (win->console != fg_console)
		return FBUI_SUCCESS;
	if (win->is_hidden)
		return FBUI_SUCCESS;
	/*----------*/

	p2.x0 = p->x0;
	p2.y0 = p->y0;
	p2.x1 = p->x1;
	p2.y1 = p->y0;
	p2.color = p->color;
	if ((retval = fbui_fill_rect (info, win, &p2)))
		return retval;

	p2.x0 = p->x0;
	p2.y0 = p->y0;
	p2.x1 = p->x0;
	p2.y1 = p->y1;
	p2.color = p->color;
	if ((retval = fbui_fill_rect (info, win, &p2)))
		return retval;

	p2.x0 = p->x1;
	p2.y0 = p->y0;
	p2.x1 = p->x1;
	p2.y1 = p->y1;
	p2.color = p->color;
	if ((retval = fbui_fill_rect (info, win, &p2)))
		return retval;

	p2.x0 = p->x0;
	p2.y0 = p->y1;
	p2.x1 = p->x1;
	p2.y1 = p->y1;
	p2.color = p->color;
	if ((retval = fbui_fill_rect (info, win, &p2)))
		return retval;

	return FBUI_SUCCESS;
}


static int fbui_fill_rect (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *p)
{
	struct fbui_rects *r;
	struct fb_draw p2;
	int ix, lim;

	if (!info || !win || !p) {
		FBUI_ERROR_LOC(120);
		return FBUI_ERR_NULLPTR;
	}
	if (win->console != fg_console)
		return FBUI_SUCCESS;
	if (win->is_hidden)
		return FBUI_SUCCESS;
	if (p->x0 > p->x1) { 
		short tmp=p->x0; 
		p->x0=p->x1; 
		p->x1=tmp; 
	}
	if (p->y0 > p->y1) { 
		short tmp=p->y0; 
		p->y0 = p->y1; 
		p->y1 = tmp; 
	}
	if (p->x1 < 0 || p->y1 < 0 || p->x0 >= win->width || 
	    p->y0 >= win->height)
		return 0;
	if (!info->fbops->fb_fillrect2)
		return 0;
	/*----------*/

	p->x0 += win->x0;
	p->y0 += win->y0;
	p->x1 += win->x0;
	p->y1 += win->y0;

	if (!info->have_hardware_pointer && info->pointer_active && 
	    !info->pointer_hidden) {
		short x2 = p->x0;
		short x3 = p->x1;
		short y2 = p->y0;
		short y3 = p->y1;
		if (x2 > x3) {
			short tmp=x2;
			x2=x3;
			x3=tmp;
		}
		if (y2 > y3) {
			short tmp=y2;
			y2=y3;
			y3=tmp;
		}
		pointer_hide_if_touching (info, 
			x2 < win->x0 ? win->x0 : x2,
			y2 < win->y0 ? win->y0 : y2,
			x3 > win->x1 ? win->x1 : x3,
			y3 > win->y1 ? win->y1 : y3);
	}

	r = win->is_wm ? info->bg_rects[fg_console] : win->rects;
	ix = 0;
	lim = r->total << 2;
	while (r && ix < lim) {
		p2.x0 = p->x0;
		p2.y0 = p->y0;
		p2.x1 = p->x1;
		p2.y1 = p->y1;
		p2.clip_x0 = r->c[ix++];
		p2.clip_y0 = r->c[ix++];
		p2.clip_x1 = r->c[ix++];
		p2.clip_y1 = r->c[ix++];
		p2.clip_valid = 1;
		p2.color = p->color;

		info->fbops->fb_fillrect2 (info, &p2);

		if (ix >= lim) {
			ix = 0;
			r = r->next;
		}
	}

	return FBUI_SUCCESS;
}


int fbui_clear (struct fb_info *info, struct fbui_win *win)
{
	if (!info || !win) {
		FBUI_ERROR_LOC(122);
		return FBUI_ERR_NULLPTR;
	}
	if (win->console != fg_console) {
		return FBUI_SUCCESS;
	}
	if (win->is_hidden || win->is_wm)
		return FBUI_SUCCESS;
	/*----------*/

	if (!info->have_hardware_pointer && info->pointer_active && 
	    !info->pointer_hidden)
		pointer_hide_if_rects (info, win);

	rects_fill (info, win->rects, win->bgcolor);

	/* Following is needed since fbui_clear is called from many places,
	 * not just fbui_exec which does its own unhide
	 */
	fbui_unhide_pointer (info);

	return FBUI_SUCCESS;
}



static int fbui_set_geometry (struct fb_info *info, struct fbui_win *win,
                              short x0, short y0, short x1, short y1)
{
	short xres, yres;
	short w,h;

	if (!info || !win) {
		FBUI_ERROR_LOC(132);
		return FBUI_ERR_NULLPTR;
	}
	xres=info->var.xres;
	yres=info->var.yres;
	if (x0>x1) { 
		int tmp=x0; x0=x1; x1=tmp; 
	}
	if (y0>y1) { 
		int tmp=y0; y0=y1; y1=tmp; 
	}
	if (x1<0 || x0>=xres) {
		FBUI_ERROR_LOC(133);
		return FBUI_ERR_OFFSCREEN;
	}
	if (y1<0 || y0>=yres) {
		FBUI_ERROR_LOC(134);
		return FBUI_ERR_OFFSCREEN;
	}
	if (x0<0) 
		x0=0;
	if (y0<0) 
		y0=0;
	if (x1>=xres) 
		x1=xres-1;
	if (y1>=yres) 
		y1=yres-1;
	/*----------*/

	w = x1-x0+1;
	h = y1-y0+1;
	if (win->max_width && w > win->max_width)
		w = win->max_width;
	if (win->max_height && h > win->max_height)
		h = win->max_height;

	win->x0 = x0;
	win->y0 = y0;
	win->x1 = x0 + w - 1;
	win->y1 = y0 + h - 1;
	win->width = w;
	win->height = h;
/* printk (KERN_INFO "fbui_set_geometry: %s (id=%d) is now at %d %d %d %d , wh = %d %d\n",win->name,win->id,x0,y0,x0+w-1,y0+h-1,w,h); */

	return FBUI_SUCCESS;
}



static int fbui_put_image (struct fb_info *info, struct fbui_win *win, 
	struct fb_put *p)
{
	struct fbui_rects *r;
	u16 ix, lim, xres, yres;
	u32 length;
	int bytes_per_pixel;

	if (!info || !win || !p) {
		FBUI_ERROR_LOC(135);
		return FBUI_ERR_NULLPTR;
	}
	if (win->console != fg_console)
		return FBUI_SUCCESS;
	if (win->is_hidden)
		return FBUI_SUCCESS;
	if (!p->pixels)
		return FBUI_ERR_NULLPTR;
	switch (p->type) {
	case FB_IMAGETYPE_RGB4:
	case FB_IMAGETYPE_RGBA:
        	bytes_per_pixel = 4;
		break;
	case FB_IMAGETYPE_RGB3:
        	bytes_per_pixel = 3;
		break;
	case FB_IMAGETYPE_RGB2:
        	bytes_per_pixel = 2;
		break;
	case FB_IMAGETYPE_GREY:
        	bytes_per_pixel = 1;
		break;
	case FB_IMAGETYPE_MONO:
        	bytes_per_pixel = 0;
		break;
	default:
		return FBUI_ERR_BADPARAM;
	}

	if (!info->fbops->fb_putimage)
		return FBUI_SUCCESS;

	length = bytes_per_pixel ? 
		bytes_per_pixel * p->width * p->height :
		(p->width * p->height + 7) >> 3;
	if (!access_ok ((void*)p->pixels, length)) {
		FBUI_ERROR_LOC(137);
		return FBUI_ERR_BADADDR;
	}
	xres = info->var.xres;
	yres = info->var.yres;
	if (p->x0 >= xres || p->x1 < 0 ||
	    p->y0 >= yres || p->y1 < 0)
		return 0;
	if (p->xstart > p->xend) {
		short tmp = p->xstart;
		p->xstart = p->xend;
		p->xend = tmp;
	}
	if (p->ystart > p->yend) {
		short tmp = p->ystart;
		p->ystart = p->yend;
		p->yend = tmp;
	}
	if (p->xend < 0 || p->yend < 0 || p->xstart >= p->width || p->ystart >= p->height)
		return 0;
	/*----------*/

	p->x0 += win->x0;
	p->y0 += win->y0;
	p->x1 += win->x0;
	p->y1 += win->y0;

	if (!info->have_hardware_pointer && info->pointer_active && 
	    !info->pointer_hidden) {
		pointer_hide_if_touching (info, 
			p->x0 < win->x0 ? win->x0 : p->x0, 
			p->y0 < win->y0 ? win->y0 : p->y0,
			p->x1 > win->x1 ? win->x1 : p->x1, 
			p->y1 > win->y1 ? win->y1 : p->y1);
	}

	r = win->is_wm ? info->bg_rects[fg_console] : win->rects;
	ix = 0;
	lim = r->total << 2;
	while (r && ix < lim) {
		p->clip_x0 = r->c[ix++];
		p->clip_y0 = r->c[ix++];
		p->clip_x1 = r->c[ix++];
		p->clip_y1 = r->c[ix++];
		p->clip_valid = 1;

		info->fbops->fb_putimage (info, p);

		if (ix >= lim) {
			ix = 0;
			r = r->next;
		}
	}
	
	return FBUI_SUCCESS;
}


int fbui_copy_area (struct fb_info *info, struct fbui_win *win, 
	struct fb_draw *p)
{
	if (!info || !win || !p) {
		FBUI_ERROR_LOC(148);
		return FBUI_ERR_NULLPTR;
	}
	if (!win->unobscured)
		return FBUI_ERR_OBSCURED; /* <- app must expose itself */
	if (win->console != fg_console)
		return FBUI_SUCCESS;
	if (win->is_hidden)
		return FBUI_SUCCESS;
	if (p->x0 > p->x1) {
		short tmp = p->x0;
		p->x0 = p->x1;
		p->x1 = tmp;
	}
	if (p->y0 > p->y1) {
		short tmp = p->y0;
		p->y0 = p->y1;
		p->y1 = tmp;
	}
	/*----------*/

	p->x0 += win->x0;
	p->y0 += win->y0;
	p->x1 += win->x0;
	p->y1 += win->y0;
	p->x2 += win->x0;
	p->y2 += win->y0;

	if (!info->have_hardware_pointer && info->pointer_active && 
	    !info->pointer_hidden && pointer_in_window (info, win, 0)) {
		short width = p->x1 - p->x0;
		short height = p->y1 - p->y0;
		pointer_hide_if_touching (info, p->x0, p->y0,
			p->x1, p->y1);
		pointer_hide_if_touching (info, p->x2, p->y2,
			p->x2 + width - 1, p->y2 + height - 1);
	}

	p->clip_valid = 1;
	p->clip_x0 = win->x0;
	p->clip_y0 = win->y0;
	p->clip_x1 = win->x1;
	p->clip_y1 = win->y1;

	if (info->fbops->fb_copyarea2)
		info->fbops->fb_copyarea2 (info, p);

	return FBUI_SUCCESS;
}


int fbui_release (struct fb_info *info, int user)
{
	return 0;
}



void generic_fillrect (struct fb_info *info, struct fb_draw *p)
{
	u32 pixel, bytes_per_pixel, offset;
	short xres, yres;
	unsigned char *ptr, *ptr_save;
	u32 transp;
	short x0,x1,y0,y1;
	int i, j;

	if (!info || !p)
		return;
	x0 = p->x0;
	y0 = p->y0;
	x1 = p->x1;
	y1 = p->y1;
	transp = p->color >> 24;
	if (transp == 255)
		return;
	if (x0 > x1) {
		short tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (y0 > y1) {
		short tmp = y0;
		y0 = y1;
		y1 = tmp;
	}
	if (y1 < 0 || x1 < 0)
		return;
	xres = info->var.xres;
	yres = info->var.yres;
	if (x0 >= xres || y0 >= yres)
		return;
	if (x0 < 0)
		x0 = 0;
	if (x1 >= xres)
		x1 = xres-1;
	if (y0 < 0)
		y0 = 0;
	if (y1 >= yres)
		y1 = yres-1;
	if (p->clip_valid) {
		if (y1 < p->clip_y0 || y0 > p->clip_y1 ||
		    x1 < p->clip_x0 || x0 > p->clip_x1)
			return;

		if (x0 < p->clip_x0)
			x0 = p->clip_x0;
		if (x1 > p->clip_x1)
			x1 = p->clip_x1;
		if (y0 < p->clip_y0)
			y0 = p->clip_y0;
		if (y1 > p->clip_y1)
			y1 = p->clip_y1;
	}
	/*----------*/

	bytes_per_pixel = (info->var.bits_per_pixel + 7) >> 3;
	offset = y0 * info->fix.line_length + x0 * bytes_per_pixel;
	ptr = ((unsigned char*)info->screen_base);
	ptr += offset;
	ptr_save = ptr;
	pixel = pixel_from_rgb (info, p->color);

	j = y1 - y0 + 1;
	while (j--) {
		ptr = ptr_save;

		i = x1 - x0 + 1;

		while (i--) {
			if (transp) {
				u32 orig_value = 0;
				u32 value = p->color;

				switch (bytes_per_pixel) {
#ifdef CONFIG_FB_UI_32BPP
				case 4:
					orig_value = fb_readl (ptr);
					break;
#endif

#ifdef CONFIG_FB_UI_24BPP
				case 3: {
					u32 tmp;
					orig_value = 0xff & fb_readb (ptr); 
					ptr++;
					tmp = 0xff & fb_readb (ptr); 
					ptr++;
					tmp <<= 8;
					orig_value |= tmp;
					tmp = 0xff & fb_readb (ptr);
					tmp <<= 16;
					orig_value |= tmp;
					ptr -= 2;
					break;
				}
#endif

#ifdef CONFIG_FB_UI_16BPP
				case 2:
					orig_value = fb_readw (ptr);
					break;
#endif

				}

				orig_value = pixel_to_rgb (info, orig_value);
				value = combine_rgb_pixels (orig_value, value, transp);
				pixel = pixel_from_rgb (info, value);
			}

			switch (bytes_per_pixel) {
#ifdef CONFIG_FB_UI_32BPP
			case 4: 
				fb_writel (pixel, ptr);
				ptr += 4;
				break;
#endif

#ifdef CONFIG_FB_UI_24BPP
			case 3: {
				register u32 c = pixel;
				fb_writeb (c, ptr); ptr++; c >>= 8;
				fb_writeb (c, ptr); ptr++; c >>= 8;
				fb_writeb (c, ptr); ptr++;
				break;
			}
#endif

#ifdef CONFIG_FB_UI_16BPP
			case 2: 
				fb_writew (pixel, ptr);
				ptr += 2;
				break;
#endif
			}/*switch*/
		}

		ptr_save += info->fix.line_length;
	}
}


/* generic_putimage
 *
 * Supports source images of:
 *	. native depth 
 *	. 24 bits per pixel
 *	. 32 bits per pixel with transparency
 *	. 8 bits grey
 *
 * Re transparency:
 * 0 is opaque, 255 is 100% transparent. 
 */

void generic_putimage (struct fb_info *info, struct fb_put *p)
{
	int i, j;
        int offset;
        u16 bytes_per_pixel;
        unsigned char *dest;
        unsigned char *dest_save;
	unsigned char *src, *src_save;
	short xres, yres;
	short width, height;
	short x0, y0;
	short x1, y1;
	u32 src_bytes_per_pixel;
	short stride;

	if (!info || !p)
		return;
	if (p->type == FB_IMAGETYPE_MONO)
		return;
	if (p->xend < 0 || p->yend < 0 || p->xstart >= p->width || p->ystart >= p->height)
		return;
	if (p->xstart > p->xend) {
		short tmp = p->xstart;
		p->xstart = p->xend;
		p->xend = tmp;
	}
	if (p->ystart > p->yend) {
		short tmp = p->ystart;
		p->ystart = p->yend;
		p->yend = tmp;
	}
	if (p->xstart < 0)
		p->xstart = 0;
	if (p->ystart < 0)
		p->ystart = 0;
	if (p->xend >= p->width)
		p->xend = p->width - 1;
	if (p->yend >= p->height)
		p->yend = p->height - 1;
	src = p->pixels;
	xres = info->var.xres;
	yres = info->var.yres;
	width = p->xend - p->xstart + 1;
	height = p->yend - p->ystart + 1;
	x0 = p->x0;
	y0 = p->y0;
	x1 = p->x1;
	y1 = p->y1;
	stride = p->width;

	switch (p->type) {
	case FB_IMAGETYPE_GREY: 
		src_bytes_per_pixel = 1;
		break;
	case FB_IMAGETYPE_RGB2: 
		src_bytes_per_pixel = 2;
		break;
	case FB_IMAGETYPE_RGB3: 
		src_bytes_per_pixel = 3;
		break;
	case FB_IMAGETYPE_RGB4: 
	case FB_IMAGETYPE_RGBA: 
		src_bytes_per_pixel = 4;
		break;
	default:
		return;
	}

	if (!src || width <= 0 || height <= 0 ||
	    x1 < 0 || y1 < 0 || x0 >= xres || y0 >= yres ||
	    stride <= 0)
		return;
	src += src_bytes_per_pixel * (p->width * p->ystart + p->xstart);
	if (x0 < 0) {
		x0 = -x0;
		width -= x0;
		src += x0 * src_bytes_per_pixel;
		x0 = 0;
	}
	if (x1 >= xres) {
		short diff = x1 - xres + 1;
		width -= diff;
		x1 = xres - 1;
	}
	if (y0 < 0) {
		y0 = -y0;
		height -= y0;
		src += y0 * stride * src_bytes_per_pixel;
		y0 = 0;
	}
	if (y1 >= yres) {
		short diff = y1 - yres + 1;
		height -= diff;
		y1 = yres - 1;
	}

	if (p->clip_valid) {
		if (y0 > p->clip_y1 || x0 > p->clip_x1 ||
		    y1 < p->clip_y0 || x1 < p->clip_x0)
			return;
		if (x0 < p->clip_x0) {
			short diff = p->clip_x0 - x0;
			width -= diff;
			src += diff * src_bytes_per_pixel;
			x0 = p->clip_x0;
		}
		if (x1 > p->clip_x1) {
			short diff = x1 - p->clip_x1;
			width -= diff;
		}
		if (y0 < p->clip_y0) {
			short diff = p->clip_y0 - y0;
			height -= diff;
			src += diff * stride * src_bytes_per_pixel;
			y0 = p->clip_y0;
		}
		if (y1 > p->clip_y1) {
			short diff = y1 - p->clip_y1;
			height -= diff;
		}
	}
	/*----------*/

        bytes_per_pixel = (info->var.bits_per_pixel + 7) >> 3;
        dest = info->screen_base;
	offset = (y0 + p->ystart) * info->fix.line_length + 
		 (x0 + p->xstart) * bytes_per_pixel;
        dest += offset;
	dest_save = dest;
	src_save = src;
	
	j = height;
	while (j-- > 0) {
		i = width;
		while (i-- > 0) {
			u32 value=0;
			u32 transp=0;
			u32 orig_value;

			switch (p->location) {
			case FB_LOCATION_KERNEL:
				switch (src_bytes_per_pixel) {
				case 1:
					value = *src++;
					break;
				case 2:
					value = *(unsigned short*) src;
					src += 2;
					break;
				case 3: {
					value = src[2];
					value <<= 8;
					value |= src[1];
					value <<= 8;
					value |= *src++;
					src += 2;
					break;
				 }
				case 4:
					value = *(unsigned long*) src;
					src += 4;
				}
				break;

			case FB_LOCATION_USER:
				switch (src_bytes_per_pixel) {
				case 1: {
					u8 tmp;
					if (get_user(tmp, src)) 
						return;
					src++;
					value = tmp;
					break;
				 }
				case 2: {
					u16 tmp, *s=(u16*)src;
					if (get_user(tmp, s)) 
						return;
					src+=2;
					value = tmp;
					break;
				 }
				case 3: {
					u8 tmp;
					src += 2;
					if (get_user(tmp, src)) 
						return;
					value = tmp;
					value <<= 8;
					src--;
					if (get_user(tmp, src)) 
						return;
					value |= tmp;
					value <<= 8;
					src--;
					if (get_user(tmp, src)) 
						return;
					value |= tmp;
					src += 3;
					break;
				 }
				case 4:  {
					u32 *s=(u32*)src;
					if (get_user(value, s)) 
						return;
					src+=4;
					break;
				 }
				}
				break;

			case FB_LOCATION_VRAM:
				value = 0;
				break;

			default:
				return;
			}

			switch (p->type) {
			case FB_IMAGETYPE_RGBA:
				transp = value >> 24;
				break;
			case FB_IMAGETYPE_RGB2: {
					u32 r,g,b;
					r = (value >> 11) & 31;
					g = (value >> 5) & 63;
					b = value & 31;
					r <<= 19;
					g <<= 10;
					b <<= 3;
					value = r | g | b;
				}
				break;
			case FB_IMAGETYPE_GREY:
				value |= (value<<8) | (value<<16);
			}

			if (transp > 0 && transp < 255) {
				orig_value = 0;
				switch (bytes_per_pixel) {
#ifdef CONFIG_FB_UI_32BPP
				case 4:
					orig_value = fb_readl (dest);
					break;
#endif

#ifdef CONFIG_FB_UI_24BPP
				case 3: {
					u32 tmp;
					orig_value = 0xff & fb_readb (dest); 
					dest++;
					tmp = 0xff & fb_readb (dest); 
					dest++;
					tmp <<= 8;
					orig_value |= tmp;
					tmp = 0xff & fb_readb (dest);
					tmp <<= 16;
					orig_value |= tmp;
					dest -= 2;
					break;
				}
#endif

#ifdef CONFIG_FB_UI_16BPP
				case 2:
					orig_value = fb_readw (dest);
					break;
#endif

				}

				orig_value = pixel_to_rgb (info, orig_value);
				value = combine_rgb_pixels (orig_value, value, transp);
			}

			if (transp == 255)
				dest += bytes_per_pixel;
			else {
				value = pixel_from_rgb (info, value);

				switch (bytes_per_pixel)
				{
#ifdef CONFIG_FB_UI_32BPP
				case 4:
					fb_writel (value, dest);
					dest += 4;
					break;
#endif

#ifdef CONFIG_FB_UI_24BPP
				case 3:
					fb_writeb (value, dest); dest++; value >>= 8;
					fb_writeb (value, dest); dest++; value >>= 8;
					fb_writeb (value, dest); dest++;
					break;
#endif

#ifdef CONFIG_FB_UI_16BPP
				case 2:
					fb_writew (value, dest); 
					dest += 2;
					break;
#endif

				default:
					break;
				}
			} 
		}
		src_save += stride * src_bytes_per_pixel;
		src = src_save;
		dest_save += info->fix.line_length;
		dest = dest_save;
	}
}

void init_bresenham (struct fb_dda *p, short x0, short y0, short x1, short y1)
{
	if (!p)
		return;

	p->x = p->xprev = x0;
	p->y = p->yprev = y0;
	p->s1 = 1;
	p->s2 = 1;

	p->dx = x1 - x0;
	if (p->dx < 0) {
		p->dx = -p->dx;
		p->s1 = -1;
	}

	p->dy = y1 - y0;
	if (p->dy < 0) {
		p->dy = -p->dy;
		p->s2 = -1;
	}

	p->xchange = 0;

	if (p->dy > p->dx) {
		int tmp = p->dx;
		p->dx = p->dy;
		p->dy = tmp;
		p->xchange = 1;
	}

	p->e = (p->dy<<1) - p->dx;
	p->j = 0;
}


void generic_line (struct fb_info *info, struct fb_draw *p)
{
	struct fb_dda dda;
	short x0_save, x1_save, y0_save, y1_save;
	u8 use_clip=0;
	short xres, yres;

	if (!info || !p)
		return;
	if (!info->fbops->fb_fillrect2)
		return;
	if ((p->x0 < 0 && p->x1 < 0) || (p->y0 < 0 && p->y1 < 0))
		return;
	xres = info->var.xres;
	yres = info->var.yres;
	if ((p->x0 >= xres && p->x1 >= xres) || 
	    (p->y0 >= yres && p->y1 >= yres))
		return;
	/*----------*/

	init_bresenham (&dda, p->x0, p->y0, p->x1, p->y1);

	x0_save = p->x0;
	x1_save = p->x1;
	y0_save = p->y0;
	y1_save = p->y1;
	use_clip = p->clip_valid;
	p->clip_valid = 0;

	while (dda.j <= dda.dx) {
		dda.j++;

		if (!use_clip || 
		    (dda.x >= p->clip_x0 && 
		     dda.x <= p->clip_x1 && 
		     dda.y >= p->clip_y0 && 
		     dda.y <= p->clip_y1)) {
			p->x0 = p->x1 = dda.x;
			p->y0 = p->y1 = dda.y;
			info->fbops->fb_fillrect2 (info, p);
		}

		if (dda.e >= 0) {
			if (dda.xchange)
				dda.x += dda.s1;
			else
				dda.y += dda.s2;
			dda.e -= (dda.dx << 1);
		}
		if (dda.xchange) 
			dda.y += dda.s2;
		else
			dda.x += dda.s1;
		dda.e += (dda.dy << 1);
	}

	p->x0 = x0_save;
	p->x1 = x1_save;
	p->y0 = y0_save;
	p->y1 = y1_save;
	p->clip_valid = use_clip;
}


void generic_filltriangle (struct fb_info *info, struct fb_draw *p)
{
	struct fb_dda left;
	struct fb_dda right;
	struct fb_draw p2;
	short xres, yres;

	if (!info || !p)
		return;
	yres = info->var.yres;
	if (p->x0 < 0 && p->x1 < 0 && p->x2 < 0)
		return;
	if (p->y0 < 0 && p->y1 < 0 && p->y2 < 0)
		return;
	if (p->y0 > p->y1) {
		short tmp = p->x0; p->x0 = p->x1; p->x1 = tmp;
		tmp = p->y0; p->y0 = p->y1; p->y1 = tmp;
	}
	if (p->y1 > p->y2) {
		short tmp = p->x2; p->x2 = p->x1; p->x1 = tmp;
		tmp = p->y2; p->y2 = p->y1; p->y1 = tmp;
	}
	if (p->y0 > p->y1) {
		short tmp = p->x0; p->x0 = p->x1; p->x1 = tmp;
		tmp = p->y0; p->y0 = p->y1; p->y1 = tmp;
	}
	if (p->y1 > p->y2) {
		short tmp = p->x2; p->x2 = p->x1; p->x1 = tmp;
		tmp = p->y2; p->y2 = p->y1; p->y1 = tmp;
	}
	xres = info->var.xres;
	yres = info->var.yres;
	if ((p->x0 >= xres && p->x1 >= xres && p->x2 >= xres) || 
	    (p->y0 >= yres && p->y1 >= yres && p->y2 >= yres))
		return;
	/*----------*/

	init_bresenham (&left, p->x0, p->y0, p->x1, p->y1);
	init_bresenham (&right, p->x0, p->y0, p->x2, p->y2);

	if ((p2.clip_valid = p->clip_valid)) {
		p2.clip_x0 = p->clip_x0;
		p2.clip_y0 = p->clip_y0;
		p2.clip_x1 = p->clip_x1;
		p2.clip_y1 = p->clip_y1;
	}

	while (1) {
		if (left.xprev < right.xprev) {
			p2.x0 = left.xprev;
			p2.x1 = right.xprev;
		} else {
			p2.x1 = left.xprev;
			p2.x0 = right.xprev;
		}
		p2.y0 = p2.y1 = left.yprev;
		p2.color = p->color;
		info->fbops->fb_fillrect2 (info, &p2);

		char got_ychange = 0;

		/* Advance the left line */
		left.xprev = left.x;
		left.yprev = left.y;
		while (!got_ychange) {
			if (left.j == left.dx) {
				if (left.y == p->y2)
					return;
				init_bresenham (&left, p->x1, p->y1, p->x2, p->y2);
			}
			left.j++;
			if (left.e >= 0) {
				if (left.xchange)
					left.x += left.s1;
				else {
					got_ychange = 1;
					left.y += left.s2;
				}
				left.e -= (left.dx << 1);
			}
			if (left.xchange) {
				got_ychange = 1;
				left.y += left.s2;
			} else
				left.x += left.s1;
			left.e += (left.dy << 1);
		}

		right.xprev = right.x;
		right.yprev = right.y;
		got_ychange = 0;

		while (!got_ychange) {
			if (right.j == right.dx)
				return;
		
			right.j++;
			if (right.e >= 0) {
				if (right.xchange)
					right.x += right.s1;
				else {
					got_ychange = 1;
					right.y += right.s2;
				}
				right.e -= (right.dx << 1);
			}
			if (right.xchange) {
				got_ychange = 1;
				right.y += right.s2;
			} else
				right.x += right.s1;
			right.e += (right.dy << 1);
		}
	}
}


/* "generic" read point routine, in the sense that
 * it will deal with any linear packed pixel framebuffer
 * that is 15/16/24/32 bpp.
 */
u32 generic_read_point (struct fb_info *info, short x, short y)
{
	u32 bytes_per_pixel, offset, value;
	unsigned char *ptr;

	if (!info || info->state != FBINFO_STATE_RUNNING) 
		return RGB_NOCOLOR;
	if (x < 0 || y < 0 || x >= info->var.xres || y >= info->var.yres)
		return RGB_NOCOLOR;
	/*----------*/

        bytes_per_pixel = (info->var.bits_per_pixel + 7) >> 3;
        offset = y * info->fix.line_length + x * bytes_per_pixel;
        ptr = offset + ((unsigned char*)info->screen_base);

	value = 0;
	switch (bytes_per_pixel) {
	case 4:
		value = fb_readl (ptr);
		break;

	case 3: {
		u32 tmp;
		value = 0xff & fb_readb (ptr); 
		ptr++;
		tmp = 0xff & fb_readb (ptr); 
		ptr++;
		tmp <<= 8;
		value |= tmp;
		tmp = 0xff & fb_readb (ptr);
		tmp <<= 16;
		value |= tmp;
		break;
	}

	case 2:
		value = fb_readw (ptr);
		break;

	default:
		value = 0;
	}

	return pixel_to_rgb (info, value);
}

/* This routine is used only for the software pointer.
 */
u32 generic_getpixels_rgb (struct fb_info *info, struct fb_put *p)
{
	short xres, yres;
	u32 *ptr;
	short i;

	if (!info || !p || !p->pixels)
		return 0;
	if (!info->fbops->fb_read_point)
		return 0;
	if (p->width <= 0 || p->x0 < 0 || p->y0 < 0)
		return 0;
	if (p->location != FB_LOCATION_KERNEL)
		return 0;
	ptr = (u32*) p->pixels;
	xres = info->var.xres;
	yres = info->var.yres;
	if (p->x0 >= xres)
		return 0;
	if (p->y0 >= yres)
		return 0;
	if (p->x0 + p->width >= xres)
		p->width = xres - p->x0;
	/*----------*/

	i = p->width;
	while (i > 0) {
		u32 rgb = info->fbops->fb_read_point (info, p->x0, p->y0);
		*ptr++ = rgb;
		p->x0++;
		i--;
	}
	return p->width;
}


EXPORT_SYMBOL(fbui_init);
EXPORT_SYMBOL(fbui_release);
EXPORT_SYMBOL(fbui_control);
EXPORT_SYMBOL(fbui_open);
EXPORT_SYMBOL(fbui_switch);
EXPORT_SYMBOL(fbui_close);
EXPORT_SYMBOL(fbui_exec);

EXPORT_SYMBOL(generic_fillrect);
EXPORT_SYMBOL(generic_putimage);
EXPORT_SYMBOL(generic_line);
EXPORT_SYMBOL(generic_filltriangle);
EXPORT_SYMBOL(generic_read_point);
EXPORT_SYMBOL(generic_getpixels_rgb);

EXPORT_SYMBOL(combine_rgb_pixels);
EXPORT_SYMBOL(pixel_from_rgb);
EXPORT_SYMBOL(pixel_to_rgb);

MODULE_AUTHOR("Zachary Smith <fbui@comcast.net>");
MODULE_DESCRIPTION("In-kernel graphical user interface for applications");
MODULE_LICENSE("GPL");


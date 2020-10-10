
/*=========================================================================
 *
 * fbmark, a benchmark test for FBUI (in-kernel framebuffer UI)
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



static short win_w, win_h;


#define CHECK { if ((count % 3)==0) { t = time(NULL); if (t-t0 >= duration) break; } count++; }
#define CHECKNOW { t = time(NULL); if (t-t0 >= duration) break; count++; }
#define REPORT(NN) { double d = count; d/=duration; if(NN) { d *= NN; printf ("\t-> pixels per second = %11.2f/sec\n", d); } else printf ("\t-> rate %11.2f/sec\n", d); count=0; t0 = time(NULL); }




int
main (int argc, char** argv)
{
	int vc=-1;

	Display *dpy;
	Window *win;

	long fg,bg;
	bg=0x303030;

	dpy = fbui_display_open ();
	if (!dpy) 
		FATAL ("cannot open display");

	Font *pcf = Font_new ();
	if (!pcf_read (pcf, "timR12.pcf")) {
		Font_free (pcf);
		pcf = NULL;
		FATAL ("cannot load font");
	}

	win = fbui_window_open (dpy, 775,580, &win_w, &win_h, 775,580, 5, 5, 
		&fg, &bg, "fbmark", "", 
		FBUI_PROGTYPE_APP, false,false, vc,
		false, false,false,
		NULL,
		argc,argv);
	if (!win) 
		FATAL ("cannot create window");

	int whichtest = argc >= 2 ? atoi(argv[1]) : 0;

	sleep(2);

	double rate;
	time_t t, t0, tstart;
	tstart = t0 = time(NULL);
	int duration = 60;
	int count = 0;

	if (!whichtest || whichtest==1)
	{
		printf ("opaque lines\n");

		while (true)
		{
			short x0,y0,x1,y1;
			unsigned long color;

			x0 = count % (2*win_w);
			if (x0 >= win_w) 
				x0 = 2*win_w - x0;
			y0 = (count & 1) ? 0 : win_h-1;
			x1 = (count & 1) ? win_w-1 : 0;
			y1 = count % (2*win_h);
			if (y1 >= win_w) 
				y1 = 2*win_h - y1;
			color = count & 0xff00ff;

			fbui_draw_line (dpy, win, x0,y0,x1,y1,color);

			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(0)

	if (!whichtest || whichtest==2)
	{
		printf ("opaque hlines\n");

		while(true)
		{
			short x0,y,x1;
			unsigned long color;

			color = count & 0xff;

			x0 = count % 255;
			x1 = count % win_w;
			y = count % win_h;

			fbui_draw_hline (dpy, win, x0,x1,y,color);

			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(0)

	if (!whichtest || whichtest==3)
	{
		printf ("opaque text\n");
                if(pcf)
                {
			fbui_draw_line(dpy, win, 0,0,win_w,win_h,RGB_RED);
			fbui_draw_line(dpy, win, win_w,0,0,win_h,RGB_RED);

			int line_height = pcf->ascent;
			if (pcf->descent > 0)
				line_height += pcf->descent;
			else
				line_height -= pcf->descent;

			while (true) {
                        	int y = (count % win_h);
				y /= line_height;
				y *= line_height;
				unsigned long color = ((count & 255) << 16) | (200<<8);

#define TESTSTR "01234 ? ABC_EFGHI ~` JKLMNOPQRSTItest m,yYqpaafgjklz<>?!@#$%^&*()~ this is a test ######### !!!!!!!!!!! AAAAAAAAAAA"
				int rv = fbui_draw_string (dpy, win, pcf,
					-100,y, TESTSTR, color);

				CHECK
			}

                	Font_free (pcf);
                }
		fbui_flush (dpy, win);

	}

	REPORT(0)

	if (!whichtest || whichtest==4) 
	{
		printf ("random opaque rectangles\n");
		while (true)
		{
			short x0,y0,x1,y1;
			unsigned long color;

			x0 = count % win_w;
			y0 = count % win_h;
			x1 = win_w - x0 - 1;
			y1 = win_h - (count % 100) - 1;
			color = count & 0xff00ff;

			fbui_draw_rect (dpy,win,x0,y0,x1,y1,0xffffff & rand());

			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(0)

	if (!whichtest || whichtest==5)
	{
		printf ("filled transparent rectangles\n");
		while(true)
		{
			short x0,y0,x1,y1;
			unsigned long color;

			x0 = count % (win_w-300);
			y0 = count % (win_h-300);
			x1 = x0 + 299;
			y1 = y0 + 299;
			color = rand();

			fbui_fill_rect (dpy,win, x0,y0,x1,y1,color);

			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(300*300)

	if (!whichtest || whichtest==6)
	{
		printf ("filled opaque rectangles\n");
		while (true)
		{
			short x0,y0,x1,y1;
			unsigned long color;

			x0 = count % (win_w-300);
			y0 = count % (win_h-300);
			x1 = x0 + 299;
			y1 = y0 + 299;
			color = count & 0xff00ff;

			fbui_fill_rect (dpy,win, x0,y0,x1,y1,color);

			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(300*300)

	if (!whichtest || whichtest==8)
	{
		printf ("fullscreen filled rectangles\n");
		while(true)
		{
			fbui_fill_rect (dpy, win, -20,-20,1000,999,
				count & 0xff);
			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(win_w*win_h)

	if (!whichtest || whichtest==9)
	{
		printf ("opaque points\n");

		fbui_draw_point (dpy, win, win_w-1, 0, RGB_RED);

		while (true)
		{
			short row, col;

			unsigned long j = count % (win_w * win_h);
			row = j / win_w;
			col = j - (row * win_w);

			fbui_draw_point (dpy, win, col, row, count & 0xff);
			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(0)

	if (!whichtest || whichtest==10)
	{
		static unsigned char drawthis[256*16*3];
		int j;
		int i=0;
		int b = 3;
		for (j=0; j<4096; j++) {
			drawthis[j*b] = 0;
			drawthis[j*b+1] = 255-i;
			drawthis[j*b+2] = i/2;
			i++;
		}

		printf ("256x16 put-rgb3 operations\n");

		while(true)
		{
			short x,y;

			x = rand() % (win_w - 256);
			y = rand() % (win_h - 16);

			int tmp = fbui_put_image (dpy, win, FB_IMAGETYPE_RGB3, x,y,256,16, (unsigned char*) drawthis );

			if (tmp != FBUI_SUCCESS) {
				fbui_window_close(dpy, win);
				printf ("error %d from fbui_put\n", tmp);
				exit(0);
			}
			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(256*16)

	if (!whichtest || whichtest==11)
	{
		static unsigned long drawthis[16*256];
		int i=0, j;
		for (j=0; j<4096; j++) {
			drawthis[i] = i;
			i++;
		}

		printf ("256x16 put-rgb opaque operations\n");

		while(true)
		{
			short x,y;

			x = rand() % (win_w - 256);
			y = rand() % (win_h - 16);

			int tmp = fbui_put_image (dpy, win, FB_IMAGETYPE_RGB4, x,y,256, 16, (unsigned char*) drawthis );

			if (tmp != FBUI_SUCCESS) {
				fbui_window_close(dpy, win);
				printf ("error %d from fbui_put\n", tmp);
				exit(0);
			}
			CHECK
		}
		fbui_flush (dpy, win);
	}

	REPORT(256*16)

	if (!whichtest || whichtest==12) 
	{
		fbui_draw_line (dpy, win, 0,0,799,599, RGB_BLUE);
		fbui_draw_line (dpy, win, 799,0,0,599, RGB_GREEN);

		printf ("copyareas upward\n");

		while(true) {
			fbui_copy_area (dpy, win, 0,1,0,0,win_w,win_h);
			CHECKNOW
		}
		fbui_flush (dpy, win);
	}

	REPORT(0)

	if (!whichtest || whichtest==13) 
	{
		fbui_draw_line (dpy, win, 0,0,799,599, RGB_BLUE);
		fbui_draw_line (dpy, win, 799,0,0,599, RGB_BLUE);

		printf ("copyareas downard\n");

		while(true) {
			fbui_copy_area (dpy, win, 0,0,0,1,win_w,win_h);
			CHECKNOW
		}
		fbui_flush (dpy, win);
	}

	REPORT(0)

	if (!whichtest || whichtest==14) 
	{
		fbui_draw_line (dpy, win, 0,0,799,599, RGB_YELLOW);
		fbui_draw_line (dpy, win, 799,0,0,599, RGB_ORANGE);

		printf ("copyareas rightward\n");

		while(true) {
			fbui_copy_area (dpy, win, 0,0,1,0,win_w, win_h);
			CHECKNOW
		}
		fbui_flush (dpy, win);
	}

	REPORT(0)

	if (!whichtest || whichtest==15) 
	{
		fbui_draw_line (dpy, win, 0,0,799,599, RGB_BLUE);
		fbui_draw_line (dpy, win, 799,0,0,599, RGB_BLUE);

		printf ("copyareas leftward\n");

		while(true) {
			fbui_copy_area (dpy, win, 1,0,0,0,win_w, win_h);
			CHECKNOW
		}
		fbui_flush (dpy, win);
	}

	REPORT(0)

	if (!whichtest || whichtest==18)
	{
		unsigned long drawthis[640*480];
		int i;
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			drawthis[i] = j + (k << 8);
		}

		printf ("640x480 rgb4 put image\n");
		time_t t0 = time(NULL);

		while(true)
		{
			CHECK

			fbui_put_image (dpy, win, FB_IMAGETYPE_RGB4, 3,3,640,480, (unsigned char*) drawthis );
		}
	}

	REPORT(0)
	
	if (!whichtest || whichtest==19)
	{
		printf ("640x480 rgb4 fillrect \n");
		time_t t0 = time(NULL);

		while(true)
		{
			CHECK

			fbui_fill_rect (dpy, win, 1,1,640,480, count&1 ? RGB_GREEN : RGB_BLUE);
			fbui_flush (dpy, win);
		}
	}

	REPORT(0)

	if (!whichtest || whichtest==20)
	{
		unsigned char drawthis[640*480*3];
		int i;
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			int ix = 3*i;
			drawthis[ix] = j;
			drawthis[ix+1] = 0;
			drawthis[ix+2] = k;
		}

		printf ("640x480 rgb3 put image\n");
		time_t t0 = time(NULL);

		while(true)
		{
			CHECK

			fbui_put_image (dpy, win, FB_IMAGETYPE_RGB3, 3,3,640,480, drawthis );
		}
	}

	REPORT(0)
	
	if (!whichtest || whichtest==21)
	{
		unsigned short drawthis[640*480];
		int i;
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			j >>= 3;
			k >>= 3;
			drawthis[i] = (j << 11) | k;
		}

		printf ("640x480 rgb2 put image\n");
		time_t t0 = time(NULL);

		while(true)
		{
			CHECK

			fbui_put_image (dpy, win, FB_IMAGETYPE_RGB2, 3,3,640,480, (unsigned char*) drawthis );
		}
	}

	REPORT(0)

	if (!whichtest || whichtest==22)
	{
		unsigned char drawthis[640*480];
		int i;
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			drawthis[i] = (j+k)/2;
		}

		printf ("640x480 grey put image\n");
		time_t t0 = time(NULL);

		while(true) {
			CHECK
			fbui_put_image (dpy, win, FB_IMAGETYPE_GREY, 
				3,3,640,480, drawthis );
		}
	}

	REPORT(0)

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==23)
	{
		unsigned char drawthis[80*480];
		memset (drawthis,0,80*480);

		int L=15;
		while (L--) {
			int i;
			for (i=0; i<80*480; i++) {
				int b;
				for (b = 0; b<8; b++) {
					int j = (i % 80) * 80 + b;
					int k = i / 80;
					double dist = sqrt (j*j + k*k) / 800.0;
					unsigned char b2 = 1<<b;
					int n = rand() % 800;
					if (dist > n)
						drawthis[i] |= b2;
				}
			}
		}

		printf ("640x480 mono image writes per second\n");
		time_t t0 = time(NULL);

		while(true) {
			CHECK
			fbui_put_image_mono (dpy, win, 3,3,640,480, 
				drawthis, RGB_RED );
		}
	}

	REPORT(0)

	long dur = t0 - tstart;
	printf ("The entire test took %ld seconds to complete.\n", dur);

	fbui_window_close(dpy, win);
	fbui_display_close (dpy);

	return 0;
}

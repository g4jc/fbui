
/*=========================================================================
 *
 * fbtest, a test for FBUI (in-kernel framebuffer UI)
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




void
random_rect_test(Display* dpy, Window *win)
{
	int i=0;		
	srand(1243+time(NULL));

	printf ("50000 random opaque rectangles\n");
	i = 0;
	while (i++ < 50000)
	{
		int x0 = rand();
		int y0 = rand();
		int x1 = rand();
		int y1= rand();
		x0 &= 2047;
		x1 &= 2047;
		y0 &= 2047;
		y1 &= 2047;
		x0 -= 800;
		y0 -= 800;
		x1 -= 800;
		y1 -= 800;
		fbui_draw_rect (dpy,win,x0,y0,x1,y1,0xffffff & rand());
	}
}



int
main(int argc, char** argv)
{
	int i;
	int vc=-1;

	Display *dpy;
	Window *win;

	srand (time(NULL));

	short win_w, win_h;
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

	win = fbui_window_open (dpy, 700,500, &win_w, &win_h, 9999,9999, 40, 15, 
		&fg, &bg, "fbtest", "", 
		FBUI_PROGTYPE_APP, false,false, vc,
		false, false,false,
		NULL,
		argc,argv);
	if (!win) 
		FATAL ("cannot create window");

	int whichtest = argc >= 2 ? atoi(argv[argc-1]) : 0;

	if (!whichtest || whichtest==1)
	{
		printf ("100000 lines\n");
		for (i=0;i<100000; i++)
		{
			short x0 = (rand() & 2047) - 800;
			short y0 = (rand() & 2047) - 800;
			short x1 = (rand() & 2047) - 800;
			short y1 = (rand() & 2047) - 800;
			fbui_draw_line (dpy, win, x0,y0,x1,y1,rand());
		}
		fbui_flush (dpy, win);
	}

	if (!whichtest || whichtest==2)
	{
		printf ("500000 hlines\n");
		int j;
		for(j=0; j<1000; j++)
		{
			i=0;
			unsigned long color=rand() & 0xffffff;
			while(i < 500)
			{
				int x1 = (rand() % 2000) - 400;
				int x2 = (rand() % 2000) - 400;
				int y = (rand() % 2000) - 400;
				fbui_draw_hline (dpy, win, x1,x2,y,color);
				i++;
			}
			fbui_flush (dpy, win);
			i=-20;
			while(i < 1000)
			{
				color=rand();
				fbui_draw_hline (dpy, win, -50, i, i,color); 
				i++;
			}
			fbui_flush (dpy, win);
		}
	}


	if (!whichtest || whichtest==3)
	{
                if(pcf)
                {
			fbui_draw_line(dpy, win, 0,0,799,599,RGB_RED);
			fbui_draw_line(dpy, win, 799,0,0,599,RGB_RED);

			int n=0;
			while(n<200) {
                        	int y = -5;

				RGB color = rand() & 0xffffff;

				while (y<610) {
#define TESTSTR "01234 ? ABC_EFGHI ~` JKLMNOPQRSTItest m,yYqpaafgjklz<>?!@#$%^&*()~"
					int rv = fbui_draw_string (dpy, win, pcf,
						-100,y, TESTSTR, color);

					y += pcf->ascent + pcf->descent;
				}
				n++;
			}

                	Font_free (pcf);
                }
	}

	if (!whichtest || whichtest==4) 
	{
		printf ("rects\n");
		random_rect_test(dpy, win);
	}

	if (!whichtest || whichtest==5)
	{
		printf ("1000 random filled transparent rectangles\n");
		for (i=0; i<1000; i++)
		{
			fbui_fill_rect (dpy,win, 
				(rand() % 1000) -200,
				(rand() % 1000) -200,
				(rand() % 1000) -200,
				(rand() % 1000) -200,
				rand());
		}
	}

	if (!whichtest || whichtest==7)
	{
		printf ("500 fullscreen filled rectangles\n");
		for (i=0; i<500; i++)
		{
			int tmp = fbui_fill_rect (dpy, win, -20,-20,1000,999, 0xffffff & rand());
			if (tmp) {
				fbui_display_close(dpy);
				printf ("error %d from fbui_fill_rect\n", tmp);
				exit(0);
			}
		}
		fbui_flush (dpy,win);
	}

	if (!whichtest || whichtest==8)
	{
		printf ("couple million opaque points\n");

		// draw 2,000,000 pixels
		i=0;
		while (i < 2000000)
		{
			short x = (rand() % 1000) - 200;
			short y = (rand() % 1000) - 200;
			fbui_draw_point (dpy, win, x,y, i & 0xff);
			i++;
		}
		fbui_flush (dpy, win);
	}

	if (!whichtest || whichtest==9)
	{
	}

	if (!whichtest || whichtest==10)
	{
		unsigned long drawthis[256*16];
		for (i=0; i<256*16; i++) {
			int j = i & 255;
			unsigned long value = j/2;
			value <<= 8;
			value |= 255-j;
			value <<= 8;
			value |= 192;
			drawthis[i] = value;
		}

		printf ("50000 256x16 put-rgb operations\n");

		i=0;
		while(i<100000)
		{
			int j=0;
			short x = (rand() % 1000) - 200;
			short y = (rand() % 1000) - 200;
			fbui_put_image (dpy, win, FB_IMAGETYPE_RGB4, x,y+j,256, 16, (unsigned char*)drawthis );
			i++;
		}
	}

	if (!whichtest || whichtest==11) 
	{
		fbui_draw_line (dpy, win, 0,0,799,599, RGB_BLUE);
		fbui_draw_line (dpy, win, 799,0,0,599, RGB_GREEN);

		for (i=0; i<100; i++)
			fbui_copy_area (dpy, win, 0,20,0,0,800,600);
		fbui_flush (dpy, win);
	}

	if (!whichtest || whichtest==12) 
	{
		fbui_draw_line (dpy, win, 0,0,799,599, RGB_BLUE);
		fbui_draw_line (dpy, win, 799,0,0,599, RGB_BLUE);

		for (i=0; i<100; i++)
			fbui_copy_area (dpy, win, 0,0,0,22,800,800);
		fbui_flush (dpy, win);
	}

	if (!whichtest || whichtest==13) 
	{
		fbui_draw_line (dpy, win, 0,0,799,599, RGB_YELLOW);
		fbui_draw_line (dpy, win, 799,0,0,599, RGB_ORANGE);

		for (i=0; i<100; i++)
			fbui_copy_area (dpy, win, 0,0,20,0,800,800);
		fbui_flush (dpy, win);
	}

	if (!whichtest || whichtest==14) 
	{
		fbui_draw_line (dpy, win, 0,0,799,599, RGB_BLUE);
		fbui_draw_line (dpy, win, 799,0,0,599, RGB_BLUE);

		for (i=0; i<100; i++)
			fbui_copy_area (dpy, win, 20,0,0,0,800,800);
		fbui_flush (dpy, win);
	}

	if (!whichtest || whichtest==15)
	{
		static unsigned char drawthis[768*4];
		for (i=0; i<768; ) {
			int j=0;
			while(j<4) {
				drawthis[768*j + i] = i/6;
				drawthis[768*j + i+1] = 255-i/3;
				drawthis[768*j + i+2] = 0xff;
				j++;
			}
			i+=3;
		}

		printf ("50000 256x16 put-rgb operations\n");

		i=0;
		while(i<50000)
		{
			int j=0;
			short x = (rand() % 1000) - 200;
			short y = (rand() % 1000) - 200;
			while(j<16) {
				fbui_put_image (dpy, win, FB_IMAGETYPE_RGB3, x,y+j,256, 4, drawthis );
				j+=4;
			}
			i++;
		}
	}

	if (!whichtest || whichtest==16)
	{
		int j=256;

		printf ("vlines\n");

#if 1
		for (j=0; j<256; j++) {
			i=0;
			unsigned long color = (rand() & 0xffff00) | (j & 255);
			while(i<5000)
			{
				fbui_draw_vline (dpy, win, i-1000,i-2000,i-1000,color);
				i++;
			}
			fbui_flush (dpy, win);
		}
#endif
		fbui_clear (dpy, win);
		fbui_flush (dpy, win);

		for (j=0 ; j<640; j++)
		{
			double r;
			int factor = j / 160;
			i=0;
			unsigned long color=rand() & 0xffffff; // no transparency
			while(i<win_w)
			{
				r = i;
				r /= win_w/4;
				r *= 3.1415926536;
				r *= factor;
				r = sin(r);
				int y = r*(win_h/2) + (win_h/2);
				fbui_draw_vline (dpy, win, i, (j&1) ? -44 : win_h+44, y,color);
				i++;
			}
			fbui_flush (dpy, win);
		}
		fbui_draw_vline (dpy, win, 100, 0, 100, RGB_WHITE);
		fbui_draw_vline (dpy, win, 110, 100, 0, RGB_WHITE);
		fbui_flush (dpy, win);
	}

	if (!whichtest || whichtest==17)
	{
		unsigned short drawthis[256*16];
		for (i=0; i<256*16; i++) {
			int j = (i & 255) / 4;
			unsigned short value = 20 << 11; // red
			value |= j << 5; // green
			value |= ((63-j)/2); // blue
			drawthis[i] = value;
		}

		printf ("50000 256x16 put-rgb2 operations\n");

		i=0;
		while(i<100000)
		{
			short x = (rand() & 2047) - 800;
			short y = (rand() & 2047) - 800;
			fbui_put_image (dpy, win, FB_IMAGETYPE_RGB2, x,y,256, 16, (unsigned char*)drawthis );
			i++;
		}
	}

	int duration = 60;
	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==18)
	{
		unsigned long drawthis[640*480];
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			drawthis[i] = j + (k << 8);
		}

		sleep(2);

		printf ("640x480 rgb4 image writes per second\n");
		time_t t0 = time(NULL);

		i=0;
		while(true)
		{
			if ((i & 7) == 0) {
				time_t t = time(NULL)-t0;
				if (t >= duration)
					break;
			}

			fbui_put_image (dpy, win, FB_IMAGETYPE_RGB4, 3,3,640,480, (unsigned char*)drawthis );
			i++;
		}

		double n = i;
		n /= duration;
		printf ("Total images written per second = %g\n", n);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==19)
	{
		sleep(2);

		printf ("640x480 rgb4 fillrects writes per second\n");
		time_t t0 = time(NULL);

		i=0;
		while(true)
		{
			if ((i & 7) == 0) {
				time_t t = time(NULL)-t0;
				if (t >= duration)
					break;
			}

			fbui_fill_rect (dpy, win, 1,1,640,480, i&1 ? RGB_GREEN : RGB_BLUE);
			fbui_flush (dpy, win);
			i++;
		}

		double n = i;
		n /= duration;
		printf ("Total images written per second = %g\n", n);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==20)
	{
		unsigned char drawthis[640*480*3];
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			int ix = 3*i;
			drawthis[ix] = j;
			drawthis[ix+1] = 0;
			drawthis[ix+2] = k;
		}

		sleep(2);

		printf ("640x480 rgb3 image writes per second\n");
		time_t t0 = time(NULL);

		i=0;
		while(true)
		{
			if ((i & 7) == 0) {
				time_t t = time(NULL)-t0;
				if (t >= duration)
					break;
			}

			fbui_put_image (dpy, win, FB_IMAGETYPE_RGB3, 3,3,640,480, drawthis );
			i++;
		}

		double n = i;
		n /= duration;
		printf ("Total images written per second = %g\n", n);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==21)
	{
		unsigned short drawthis[640*480];
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			j >>= 3;
			k >>= 3;
			drawthis[i] = (j << 11) | k;
		}

		sleep(2);

		printf ("640x480 rgb2 image writes per second\n");
		time_t t0 = time(NULL);

		i=0;
		while(true)
		{
			if ((i & 7) == 0) {
				time_t t = time(NULL)-t0;
				if (t >= duration)
					break;
			}

			fbui_put_image (dpy, win, FB_IMAGETYPE_RGB2, 3,3,640,480, (unsigned char*)drawthis );
			i++;
		}

		double n = i;
		n /= duration;
		printf ("Total images written per second = %g\n", n);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==22)
	{
		unsigned char drawthis[80*480];
		memset (drawthis,0,80*480);

		int L=30;
		while (L--) {
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

		sleep(2);
		putchar (7);

		printf ("640x480 mono image writes per second\n");
		time_t t0 = time(NULL);

		i=0;
		while(true)
		{
			if ((i & 7) == 0) {
				time_t t = time(NULL)-t0;
				if (t >= duration)
					break;
			}

			fbui_put_image_mono (dpy, win, 3,3,640,480, drawthis, RGB_RED );
			i++;
		}

		double n = i;
		n /= duration;
		printf ("Total images written per second = %g\n", n);
	}

	if (!whichtest || whichtest==23)
	{
		printf ("10000 random filled opaque rectangles\n");
		for (i=0; i<10000; i++)
		{
			fbui_fill_rect (dpy,win, 
				(rand() % 1000) -200,
				(rand() % 1000) -200,
				(rand() % 1000) -200,
				(rand() % 1000) -200,
				0xffffff & rand());
		}
		fbui_flush (dpy, win);
		sleep(2);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==24)
	{
		sleep(2);

		printf ("640x480 grey fillrects writes per second\n");
		time_t t0 = time(NULL);

		i=0;
		while(true)
		{
			if ((i & 7) == 0) {
				time_t t = time(NULL)-t0;
				if (t >= duration)
					break;
			}

			fbui_fill_rect (dpy, win, 1,1,640,480, 0xEFEFEF);
			fbui_flush (dpy, win);
			i++;
		}

		double n = i;
		n /= duration;
		printf ("Total images written per second = %g\n", n);
	}

	if (!whichtest || whichtest==25)
	{
		printf ("triangles\n");
		for (i=0;i<40; i++)
		{
			short x0 = (rand() & 850)  - 25;
			short y0 = (rand() & 650)  - 25;
			short x1 = (rand() & 850)  - 25;
			short y1 = (rand() & 650)  - 25;
			short x2 = (rand() & 850)  - 25;
			short y2 = (rand() & 650)  - 25;
			fbui_fill_triangle (dpy, win, x0,y0,x1,y1,x2,y2,rand());
		}
		fbui_flush (dpy, win);
		sleep(1);
	}

	if (!whichtest || whichtest==26)
	{
		printf ("triangles\n");
		for (i=0;i<10000; i++)
		{
			short x0 = (rand() & 850)  - 25;
			short y0 = (rand() & 650)  - 25;
			short x1 = (rand() & 850)  - 25;
			short y1 = (rand() & 650)  - 25;
			short x2 = (rand() & 850)  - 25;
			short y2 = (rand() & 650)  - 25;
			RGB color = rand() & 0xffffff;
			fbui_fill_triangle (dpy, win, x0,y0,x1,y1,x2,y2,color);
		}
		fbui_flush (dpy, win);
		sleep(1);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==27)
	{
		sleep(2);

		unsigned short drawthis[640*480];
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			j >>= 3;
			k >>= 3;
			drawthis[i] = (j << 11) | k;
		}

		printf ("640x480 rgb2 partial putimage test\n");
		time_t t0 = time(NULL);

		i=0;
		int count=1;
		while(count--)
		{
			int x, y;
			for (y=0; y<480; y++) {
				for (x=0; x<640; x++) {
					fbui_put_image_partial (dpy, win, 
						FB_IMAGETYPE_RGB2,
						3,3,640,480, 
						x,y,x,y,
						(unsigned char*)drawthis);
					i++;
				}
			}
		}
		fbui_flush (dpy, win);

		int duration = time(NULL) - t0;
		double n = i;
		n /= duration;
		printf ("Single-pixel partial putimage 16bpp per second = %g\n", n);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==28)
	{
		unsigned long drawthis[640*480];
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			drawthis[i] = j + (k << 8);
		}

		printf ("640x480 rgb4 partial putimage test\n");
		time_t t0 = time(NULL);

		i=0;
		int count=1;
		while(count--)
		{
			int x, y;
			for (y=0; y<480; y++) {
				for (x=0; x<640; x++) {
					fbui_put_image_partial (dpy, win, 
						FB_IMAGETYPE_RGB4,
						3,3,640,480, 
						x,y,x,y,
						(unsigned char*)drawthis);
					i++;
				}
			}
		}
		fbui_flush (dpy, win);

		int duration = time(NULL) - t0;
		double n = i;
		n /= duration;
		printf ("Single-pixel partial putimage 32bpp per second = %g\n", n);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==29)
	{
		unsigned long drawthis[640*480];
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			drawthis[i] = j + (k << 8);
		}

		printf ("640x480 rgb4 partial putimage test\n");
		time_t t0 = time(NULL);

		i=0;
		int count=1;
		while(count--)
		{
			int x, y;
			for (y=0; y<480; y+=12) {
				for (x=0; x<640; x+=7) {
					fbui_put_image_partial (dpy, win, 
						FB_IMAGETYPE_RGB4,
						3,3,640,480, 
						x,y,x+6,y+11,
						(unsigned char*)drawthis);
					i++;
				}
			}
		}
		fbui_flush (dpy, win);

		int duration = time(NULL) - t0;
		double n = i;
		n /= duration;
		n *= 12*7;
		printf ("Multi-pixel 7x12 partial putimage 32bpp per second = %g\n", n);

		sleep(1);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==30)
	{
		unsigned long drawthis[640*480];
		for (i=0; i<640*480; i++) {
			int j = (256 * (i % 640)) / 640;
			int k = (256 * (i / 480)) / 640;
			drawthis[i] = j + (k << 8);
		}

		printf ("640x480 rgb4 partial putimage test\n");
		time_t t0 = time(NULL);

		i=0;
		int count=5000;
		while(count--)
		{
			int x, y;
			for (y=0; y<480; y+=99) {
				for (x=0; x<640; x+=66) {
					fbui_put_image_partial (dpy, win, 
						FB_IMAGETYPE_RGB4,
						3,3,640,480, 
						x,y,x+66,y+99,
						(unsigned char*)drawthis);
					i++;
				}
			}
		}
		fbui_flush (dpy, win);

		int duration = time(NULL) - t0;
		double n = i;
		n /= duration;
		n *= 66;
		n *= 99;
		n /= 5000.0;
		printf ("Multi-pixel 66x99 partial putimage 32bpp per second = %g\n", n);

		sleep(1);
	}

	/* ---------------- Timed Test ----------------- */
	if (!whichtest || whichtest==31)
	{
		printf ("640x480 rgb4 greyscale fillrects writes per second (test of memset_io)\n");
sleep(2);
		time_t t0 = time(NULL);

		i=0;
		while(true)
		{
			if ((i & 7) == 0) {
				time_t t = time(NULL)-t0;
				if (t >= duration)
					break;
			}

			unsigned long grey = (i&1) ? 0: 0xff;
			grey |= (grey << 8) | (grey << 16);

			fbui_fill_rect (dpy, win, 1,1,640,480, grey);
			fbui_flush (dpy, win);
			i++;
		}

		double n = i;
		n /= duration;
		printf ("Total images written per second = %g\n", n);
	}

	fbui_window_close(dpy, win);
	fbui_display_close (dpy);

	return 0;
}


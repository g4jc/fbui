/*
 * framebuffer driver for VBE 2.0 compliant graphic boards
 *
 * switching to graphics mode happens at boot time (while
 * running in real mode, see arch/i386/boot/video.S).
 *
 * (c) 1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
 *
 *----------------------------------------------------------------------------
 * Additional functions and optimizations for FBUI:
 * Copyright (C) 2004-2005, by Zachary Smith, fbui@comcast.net.
 * Covered under the GNU Public License.
 * See change log below.
 *----------------------------------------------------------------------------
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/ioport.h>
#include <linux/init.h>
#ifdef __i386__
#include <video/edid.h>
#endif
#include <asm/io.h>
#include <asm/mtrr.h>
#include <asm/uaccess.h>

#define dac_reg	(0x3c8)
#define dac_val	(0x3c9)

/* --------------------------------------------------------------------- */

static struct fb_var_screeninfo vesafb_defined __initdata = {
	.activate	= FB_ACTIVATE_NOW,
	.height		= -1,
	.width		= -1,
	.right_margin	= 32,
	.upper_margin	= 16,
	.lower_margin	= 4,
	.vsync_len	= 4,
	.vmode		= FB_VMODE_NONINTERLACED,
};

static struct fb_fix_screeninfo vesafb_fix __initdata = {
	.id	= "VESA VGA",
	.type	= FB_TYPE_PACKED_PIXELS,
	.accel	= FB_ACCEL_NONE,
};

static int             inverse   = 0;
static int             mtrr      = 1;
static int	       vram __initdata = 0; /* Set amount of memory to be used */
static int             pmi_setpal = 0;	/* pmi for palette changes ??? */
static int             ypan       = 0;  /* 0..nothing, 1..ypan, 2..ywrap */
static unsigned short  *pmi_base  = NULL;
static void            (*pmi_start)(void);
static void            (*pmi_pal)(void);


/*-----------------------------------------------------------------------------
 * VESA driver code for the FBUI project
 *-----------------------------------------------------------------------------
 * Change log:
 * 05 Sep 2005, fbui@comcast.net: removed 8bpp code.
 * 05 Sep 2005, fbui@comcast.net: transparent hline speedup.
 * 10 Sep 2005, fbui@comcast.net: merged point/hline/vline/fillrect.
 * 12 Sep 2005, fbui@comcast.net: optimized right/leftward copyarea.
 * 12 Sep 2005, fbui@comcast.net: optimized 32bpp image draw RGB4.
 * 12 Sep 2005, fbui@comcast.net: optimized 32bpp image draw RGB3.
 * 12 Sep 2005, fbui@comcast.net: optimized 32bpp image draw RGB2.
 * 12 Sep 2005, fbui@comcast.net: optimized 32bpp image draw GREY.
 * 12 Sep 2005, fbui@comcast.net: optimized 32bpp opaque fillrect.
 * 12 Sep 2005, fbui@comcast.net: optimized 24bpp opaque fillrect.
 * 13 Sep 2005, fbui@comcast.net: unrolled loops for further speed-up.
 * 17 Sep 2005, fbui@comcast.net: putimage optimizations using memcpy_toio.
 * 18 Sep 2005, fbui@comcast.net: copyarea optimizations using memcpy_to/fromio.
 * 20 Sep 2005, fbui@comcast.net: added 1bpp monochrome putimage.
 * 22 Sep 2005, fbui@comcast.net: reimplemented vesa_line for speedup.
 * 22 Sep 2005, fbui@comcast.net: speed up of mono putimage by 52%.
 * 02 Oct 2005, fbui@comcast.net: 24bpp improvements & speedups.
 *-----------------------------------------------------------------------------
 */
#ifdef CONFIG_FB_UI

/* Optimized function to put opaque GREY, RGB2, RGB3, & RGB4 data to a
 * 32bpp display.
 *
 * Benchmark results for writing 640x480 images (run libfbui/Test1/fbtest)
 * Maximal numbers only.
 *
 * CPU:		Cel. M	P3
 * Speed(MHz):	1500	700E
 * VRAM:	Shared	AGP
 * ----------------------------------------------------------------------------
 * RGB4		594/sec	?
 * RGB3		165	?
 * RGB2		412	?
 * GREY		560	?
 *
 * There is another function for 16bpp.
 */
#ifdef CONFIG_FB_UI_32BPP
static void vesa_putimage_opaque32 (struct fb_info *info, struct fb_put *p)
{
	int i, j;
        u32 offset;
	short xres, yres;
	short width, height;
	short x0, y0;
	u8 src_bytes_per_pixel;
	u32 *dest, *dest_save;
	u8 *src0;
	short stride;
	short x1, y1;
	short xstart,xend,ystart,yend;

	if (!info || !p)
		return;

	xstart = p->xstart;
	xend = p->xend;
	if (xstart > xend) {
		short tmp = xstart;
		xstart = xend;
		xend = tmp;
	}
	ystart = p->ystart;
	yend = p->yend;
	if (ystart > yend) {
		short tmp = ystart;
		ystart = yend;
		yend = tmp;
	}
	if (xend < 0 || yend < 0 || xstart >= p->width || ystart >= p->height)
		return;
	if (xstart < 0)
		xstart = 0;
	if (ystart < 0)
		ystart = 0;
	if (xend >= p->width)
		xend = p->width - 1;
	if (yend >= p->height)
		yend = p->height - 1;

	src0 = p->pixels;
	xres = info->var.xres;
	yres = info->var.yres;
	width = p->xend - p->xstart + 1;
	height = p->yend - p->ystart + 1;
	x0 = p->x0 + xstart;
	y0 = p->y0 + ystart;
	x1 = p->x0 + xend;
	y1 = p->y0 + yend;
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
		src_bytes_per_pixel = 4;
		break;
	default:
		return;
	}

	if (!src0 || width <= 0 || height <= 0 ||
	    x1 < 0 || y1 < 0 || x0 >= xres || y0 >= yres ||
	    stride <= 0)
		return;
	src0 += src_bytes_per_pixel * (stride * ystart + xstart);
	if (x0 < 0) {
		x0 = -x0;
		width -= x0;
		src0 += x0 * src_bytes_per_pixel;
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
		src0 += y0 * stride * src_bytes_per_pixel;
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
			src0 += diff * src_bytes_per_pixel;
			x0 = p->clip_x0;
		}
		if (x1 > p->clip_x1) {
			short diff = x1 - p->clip_x1;
			width -= diff;
		}
		if (y0 < p->clip_y0) {
			short diff = p->clip_y0 - y0;
			height -= diff;
			src0 += diff * stride * src_bytes_per_pixel;
			y0 = p->clip_y0;
		}
		if (y1 > p->clip_y1) {
			short diff = y1 - p->clip_y1;
			height -= diff;
		}
	}
	/*----------*/

	dest = (u32*) info->screen_base;
	offset = y0 * (info->fix.line_length>>2) + x0;
	dest += offset;
	dest_save = dest;

	switch (p->type) {
	case FB_IMAGETYPE_RGB4: {
		u32 *src, *src_save;
		u32 *buffer;
#define BUFSIZE32 4096

		/* What is needed is a function to transfer from memory chunks
		 * directly from userspace to memory mapped I/O. Without that,
		 * gotta kmalloc.
		 */
		buffer = kmalloc(BUFSIZE32, GFP_KERNEL);
		if (!buffer) {
			printk(KERN_INFO "vesa_putimage_opaque32: no memory\n");
			return;
		}

		src = (u32*) src0;
		src_save = src;
		
		j = height;
		while (j-- > 0) {
			i = width;
			while (i > 0) {
				u32 pixels = i > (BUFSIZE32/4) ? (BUFSIZE32/4) : i;
				u32 bytes = pixels << 2;

				/* Data -> L2 cache */
				if (copy_from_user((u8*)buffer,(u8*)src, bytes))
					goto dealloc;
				src += pixels;

				/* L2 cache -> VRAM */
				memcpy_toio ((u8*)dest, (u8*)buffer, bytes);
				dest += pixels;
				i -= pixels;
			}
			src_save += stride;
			src = src_save;
			dest_save += (info->fix.line_length>>2);
			dest = dest_save;
		 }

dealloc:
		kfree (buffer);
	  	}
		break;

	case FB_IMAGETYPE_RGB3: {
		u8 *src = (u8*) src0, *src_save;
		src_save = src;

		j = height;
		while (j-- > 0) {
			u32 value;
			i = width;
			while (i > 0) {
#if 0
				if (!(3 & (u32)src)) {
					if (get_user(value, ((u32*)src)))
						return;
					value &= 0xffffff;
				} else {
#endif
					u8 tmp;
					src += 2;
					if (get_user(tmp, src)) /* red << 16 */
						return;
					value = tmp;
					value <<= 8;
					src--;
					if (get_user(tmp, src)) /* green << 8 */
						return;
					value |= tmp;
					value <<= 8;
					src--;
					if (get_user(tmp, src)) /* blue << 0 */
						return;
					value |= tmp;
				//}
				src += 3;

				fb_writel (value, dest);
				dest++;
				i--;
			}
			src_save += stride * 3;
			src = src_save;
			dest_save += (info->fix.line_length>>2);
			dest = dest_save;
		}
	  	}
		break;

	case FB_IMAGETYPE_RGB2: {
		u16 *src, *src_save;
		src = (u16*) src0;
		src_save = src;

		j = height;
		while (j-- > 0) {
			u32 value,value2;
			i = width;
			while (i > 0) {
				u32 tmp;
				if (i >= 2) {
					if (get_user(tmp, ((u32*)src))) 
						return;
					value = ((tmp << 8) & 0xf80000) |
						((tmp << 5) & 0xfc00) |
						((tmp << 3) & 0xf8);
					value2 = ((tmp >> 8) & 0xf80000) |
						((tmp >> 11) & 0xfc00) |
						((tmp >> 13) & 0xf8);
					src += 2;
					fb_writel (value, dest); dest++;
					fb_writel (value2, dest); dest++;
					i -= 2;
				} else {
					if (get_user(tmp, src)) 
						return;
					value = ((tmp << 8) & 0xf80000) |
						((tmp << 5) & 0xfc00) |
						((tmp << 3) & 0xf8);
					src++;
					fb_writel (value, dest);
					dest++;
					i--;
				}
			}
			src_save += stride;
			src = src_save;
			dest_save += (info->fix.line_length>>2);
			dest = dest_save;
		}
	  	}
		break;

	case FB_IMAGETYPE_GREY: {
		u8 *src, *src_save;
		src = src0;
		src_save = src;

		j = height;
		while (j-- > 0) {
			i = width;
			while (i > 0) {
				if (!(3 & (u32)src) && i >= 4) {
					u32 *s = (u32*) src;
					u32 value, value2;
					if (get_user(value, s)) 
						return;
					src += 4;
					i -= 4;

					value2 = value & 0xff;
					value2 |= (value2<<8) | (value2<<16);
					value >>= 8;
					fb_writel (value2, dest);
					dest++;

					value2 = value & 0xff;
					value2 |= (value2<<8) | (value2<<16);
					value >>= 8;
					fb_writel (value2, dest);
					dest++;

					value2 = value & 0xff;
					value2 |= (value2<<8) | (value2<<16);
					value >>= 8;
					fb_writel (value2, dest);
					dest++;

					value2 = value & 0xff;
					value2 |= (value2<<8) | (value2<<16);
					fb_writel (value2, dest);
					dest++;
				} else {
					u32 value;
					if (get_user(value, src)) 
						return;
					value &= 0xff;
					value |= (value<<8) | (value<<16);
					src++;
					fb_writel (value, dest);
					dest++;
					i--;
				}
			}
			src_save += stride;
			src = src_save;
			dest_save += (info->fix.line_length>>2);
			dest = dest_save;
		}
	  	}
		break;
	}

}
#endif



/* Optimized function to put monochrome image.
 * 1's put color.
 * 0's put nothing.
 */
static void vesa_putimage_mono (struct fb_info *info, struct fb_put *p)
{
	int i, j;
        u32 offset;
	short xres, yres;
	short width, height;
	short x0, y0;
	u8 *src0;
	u8 *src, *src_save;
	short stride;
	short x1, y1;
	short xstart, xend, ystart, yend;
	u8 ix0;

	if (!info || !p)
		return;
	if (p->type != FB_IMAGETYPE_MONO)
		return;
	xstart = p->xstart;
	xend = p->xend;
	if (xstart > xend) {
		short tmp = xstart;
		xstart = xend;
		xend = tmp;
	}
	ystart = p->ystart;
	yend = p->yend;
	if (ystart > yend) {
		short tmp = ystart;
		ystart = yend;
		yend = tmp;
	}
	if (xend < 0 || yend < 0 || xstart >= p->width || ystart >= p->height)
		return;
	if (xstart < 0)
		xstart = 0;
	if (ystart < 0)
		ystart = 0;
	if (xend >= p->width)
		xend = p->width - 1;
	if (yend >= p->height)
		yend = p->height - 1;
	xres = info->var.xres;
	yres = info->var.yres;
	width = xend - xstart + 1;
	height = yend - ystart + 1;
	x0 = p->x0 + xstart;
	y0 = p->y0 + ystart;
	x1 = p->x0 + xend;
	y1 = p->y0 + yend;
	stride = p->width;
	src0 = p->pixels;

	if (!src0 || width <= 0 || height <= 0 ||
	    x1 < 0 || y1 < 0 || x0 >= xres || y0 >= yres ||
	    stride <= 0)
		return;
	if (x0 < 0) {
		x0 = -x0;
		width -= x0;
		xstart += x0;
		x0 = 0;
	}
	if (x1 >= xres) {
		short diff = x1 - xres + 1;
		width -= diff;
		x1 = xres - 1;
		xend -= diff;
	}
	if (y0 < 0) {
		y0 = -y0;
		height -= y0;
		ystart += y0;
		y0 = 0;
	}
	if (y1 >= yres) {
		short diff = y1 - yres + 1;
		height -= diff;
		y1 = yres - 1;
		yend -= diff;
	}

	if (p->clip_valid) {
		if (y0 > p->clip_y1 || x0 > p->clip_x1 ||
		    y1 < p->clip_y0 || x1 < p->clip_x0)
			return;
		if (x0 < p->clip_x0) {
			short diff = p->clip_x0 - x0;
			width -= diff;
			x0 = p->clip_x0;
			xstart += diff;
		}
		if (x1 > p->clip_x1) {
			short diff = x1 - p->clip_x1;
			width -= diff;
			xend -= diff;
		}
		if (y0 < p->clip_y0) {
			short diff = p->clip_y0 - y0;
			height -= diff;
			y0 = p->clip_y0;
			ystart += diff;
		}
		if (y1 > p->clip_y1) {
			short diff = y1 - p->clip_y1;
			height -= diff;
			yend -= diff;
		}
	}
	/*----------*/

	switch ((info->var.bits_per_pixel + 7) >> 3) {
#ifdef CONFIG_FB_UI_32BPP
	case 4: {
		u32 *dest, *dest_save;
		dest = (u32*) info->screen_base;
		offset = y0 * (info->fix.line_length>>2) + x0;
		dest += offset;
		dest_save = dest;

		src0 += ((stride+7)>>3) * ystart + (xstart>>3);
		ix0 = xstart & 7;

		src = src0;
		src_save = src;

		j = height;
		while (j-- > 0) {
			u8 k = 1 << ix0;
			i = width;

			while (i > 0) {
				if (i >= 32 && !(3 & (u32)src)) {
					/* 52% faster using this code.
					 */
					register u32 k2, value, *s=(u32*)src;
					register u32 *d = dest;
					register u32 c = p->color;
					if (get_user(value, s)) 
						return;
					src += 4;

					k2 = k;
					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					if (k2 & value) fb_writel (c, d);
					k2 <<= 1; d++;

					dest = d;
					k = 1;
					i-=32;
				} else {
					u8 value;
					u32 c = p->color;
					if (get_user(value, s)) 
						return;
					src++;

					while (i > 0 && k) {
						if (k & value)
							fb_writel (c, dest);
						dest++;
						k <<= 1;
						i--;
					}

					k = 1;
				}
			}
			src_save += (stride+7)>>3;
			src = src_save;
			dest_save += (info->fix.line_length>>2);
			dest = dest_save;
		}
		break;
	  }
#endif

#ifdef CONFIG_FB_UI_24BPP
	case 3: {
		u8 *dest, *dest_save;
		dest = info->screen_base;
		offset = y0 * info->fix.line_length + x0*3;
		dest += offset;
		dest_save = dest;

		src0 += ((stride+7)>>3) * ystart + (xstart>>3);
		ix0 = xstart & 7;

		src = src0;
		src_save = src;

		j = height;
		while (j-- > 0) {
			u8 k = 1 << ix0;
			u32 c = p->color;

			i = width;
			while (i > 0) {
				if (i >= 32 && k == 1) {
					register u32 k2 = k;
					register u32 value, *s;
					register u8 *d2 = dest;
					register u8 r, g;
					g = c>>8;
					r = c>>16;
					s = (u32*) src;

					if (get_user(value, s)) 
						return;
					src += 4;

					while (k2) {
						if (k2 & value) {
							fb_writeb (c, d2);
							fb_writeb (g, (1+d2));
							fb_writeb (r, (2+d2));
						}
						d2 += 3;
						k2 <<= 1;
					}

					dest += 96;
					i -= 32;
				} else {
					u8 value;
					u32 c = p->color;
					register u8 r, g;
					g = c>>8;
					r = c>>16;

					if (get_user(value, src)) 
						return;
					src++;

					while (i > 0 && k) {
						if (k & value) {
							fb_writeb (c, dest);
							fb_writeb (g, (1+dest));
							fb_writeb (r, (2+dest));
						}
						dest += 3;
						k <<= 1;
						i--;
					}
				}
				k = 1;
			}

			src_save += (stride+7)>>3;
			src = src_save;
			dest_save += info->fix.line_length;
			dest = dest_save;
		}
		break;
	  }
#endif

#ifdef CONFIG_FB_UI_16BPP
	case 2: {
		u16 *dest, *dest_save;
		u16 pixel;
		dest = (u16*) info->screen_base;
		offset = y0 * (info->fix.line_length>>1) + x0;
		dest += offset;
		dest_save = dest;

		src0 += ((stride+7)>>3) * ystart + (xstart>>3);
		ix0 = xstart & 7;

		pixel = pixel_from_rgb (info, p->color);

		src = src0;
		src_save = src;

		j = height;
		while (j-- > 0) {
			u8 k = 1 << ix0;
			i = width;

			while (i > 0) {
				if (i >= 16 && !(3 & (u32)src)) {
					register u16 k2, value, *s=(u16*)src;
					register u16 *d = dest;
					register u16 c = pixel;
					if (get_user(value, s)) 
						return;
					src += 2;
					k2 = k;

					if (k2 & value) fb_writew (c, d);
					k2 <<= 1;

					if (k2 & value) fb_writew (c, (d+1));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+2));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+3));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+4));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+5));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+6));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+7));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+8));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+9));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+10));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+11));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+12));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+13));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+14));
					k2 <<= 1; 

					if (k2 & value) fb_writew (c, (d+15));

					dest += 16;
					k = 1;
					i -= 16;
				} else {
					u8 value;
					if (get_user(value, src)) 
						return;
					src++;

					while (i > 0 && k) {
						if (k & value)
							fb_writew (pixel, dest);
						
						dest++;
						k <<= 1;
						i--;
					}

					k = 1;
				}
			}
			src_save += (stride+7)>>3;
			src = src_save;
			dest_save += (info->fix.line_length>>1);
			dest = dest_save;
		}
	  }
#endif
	}
}


/* Optimized function to put opaque GREY, RGB2, RGB3, & RGB4 data to a
 * 16bpp display.
 *
 * Benchmark results for writing 640x480 images:
 * Maximal values only.
 *
 * CPU:		Cel. M	P3 	P2	P1
 * Speed(MHz):	1500	700E	233	200
 * VRAM:	Shared	AGP	PCI	PCI
 * ----------------------------------------------------------------------------
 * RGB4		299/sec	?	31	12
 * RGB3		140	?	20	8.3
 * RGB2		1180	?	105	13
 * GREY		490	?	67	14
 *
 */
#ifdef CONFIG_FB_UI_16BPP
static void vesa_putimage_opaque16 (struct fb_info *info, struct fb_put *p)
{
	int i, j;
        u32 offset;
	short xres, yres;
	short width, height;
	short x0, y0;
	u8 *src0;
	u8 src_bytes_per_pixel;
	u16 *dest, *dest_save;
	short stride;
	short x1, y1;
	short xstart, xend, ystart, yend;

	if (!info || !p)
		return;

	xstart = p->xstart;
	xend = p->xend;
	if (xstart > xend) {
		short tmp = xstart;
		xstart = xend;
		xend = tmp;
	}
	ystart = p->ystart;
	yend = p->yend;
	if (ystart > yend) {
		short tmp = ystart;
		ystart = yend;
		yend = tmp;
	}
	if (xend < 0 || yend < 0 || xstart >= p->width || ystart >= p->height)
		return;
	if (xstart < 0)
		xstart = 0;
	if (ystart < 0)
		ystart = 0;
	if (xend >= p->width)
		xend = p->width - 1;
	if (yend >= p->height)
		yend = p->height - 1;

	src0 = p->pixels;
	xres = info->var.xres;
	yres = info->var.yres;
	width = p->xend - p->xstart + 1;
	height = p->yend - p->ystart + 1;
	x0 = p->x0 + xstart;
	y0 = p->y0 + ystart;
	x1 = p->x0 + xend;
	y1 = p->y0 + yend;
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
		src_bytes_per_pixel = 4;
		break;
	default:
		return;
	}

	if (!src0 || width <= 0 || height <= 0 ||
	    x1 < 0 || y1 < 0 || x0 >= xres || y0 >= yres ||
	    stride <= 0)
		return;
	src0 += src_bytes_per_pixel * (stride * ystart + xstart);
	if (x0 < 0) {
		x0 = -x0;
		width -= x0;
		src0 += x0 * src_bytes_per_pixel;
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
		src0 += y0 * stride * src_bytes_per_pixel;
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
			src0 += diff * src_bytes_per_pixel;
			x0 = p->clip_x0;
		}
		if (x1 > p->clip_x1) {
			short diff = x1 - p->clip_x1;
			width -= diff;
			/* x1 = p->clip_x1; */
		}
		if (y0 < p->clip_y0) {
			short diff = p->clip_y0 - y0;
			height -= diff;
			src0 += diff * stride * src_bytes_per_pixel;
			y0 = p->clip_y0;
		}
		if (y1 > p->clip_y1) {
			short diff = y1 - p->clip_y1;
			height -= diff;
			/* y1 = p->clip_y1; */
		}
	}
	/*----------*/

	dest = (u16*) info->screen_base;
	offset = y0 * (info->fix.line_length >> 1) + x0;
	dest += offset;
	dest_save = dest;

	switch (p->type) {
	case FB_IMAGETYPE_RGB4: {
		u32 *src, *src_save;
		src = (u32*) src0;
		src_save = src;
		
		j = height;
		while (j-- > 0) {
			u32 value;
			i = width;
			while (i-- > 0) {
				if (get_user(value, src)) 
					return;
				src++;
				value = ((value >> 8) & 0xf800) |
					((value >> 5) & 0x7e0) |
					((value >> 3) & 31);
				fb_writew (value, dest);
				dest++;
			}
			src_save += stride;
			src = src_save;
			dest_save += (info->fix.line_length>>1);
			dest = dest_save;
		}
	  	}
		break;

	case FB_IMAGETYPE_RGB3: {
		u8 *src = (u8*) src0, *src_save;
		src_save = src;

		j = height;
		while (j-- > 0) {
			u32 value;
			i = width;
			while (i-- > 0) {
				if (!(3 & (u32)src)) {
					if (get_user(value, ((u32*)src)))
						return;
					value &= 0xffffff;
				} else {
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
				}
				src += 3;

				value = ((value >> 8) & 0xf800) |
				        ((value >> 5) & 0x7e0) |
				        ((value >> 3) & 31);
				fb_writew (value, dest);
				dest++;
			}
			src_save += stride * 3;
			src = src_save;
			dest_save += (info->fix.line_length>>1);
			dest = dest_save;
		}
	  	}
		break;

	case FB_IMAGETYPE_RGB2: {
		u16 *src, *src_save;
		src = (u16*) src0;
		src_save = src;
		u8* buffer;

		/* What is needed is a function to transfer from memory chunks
		 * directly from userspace to memory mapped I/O. Without that,
		 * gotta kmalloc.
		 */
#define BUFSIZE16 2040
		buffer = kmalloc(BUFSIZE16, GFP_KERNEL);
		if (!buffer) {
			printk(KERN_INFO "vesa_putimage_opaque16: no memory\n");
			return;
		}

		j = height;
		while (j-- > 0) {
			i = width;
			while (i > 0) {
				u32 pixels = i > (BUFSIZE16/2) ? (BUFSIZE16/2) : i;
				u32 bytes = pixels << 1;

				/* Userspace -> L2 cache */
				if (copy_from_user((u8*)buffer,(u8*)src, bytes))
					goto dealloc2;
				src += pixels;

				/* L2 cache -> VRAM */
				memcpy_toio ((u8*)dest, (u8*)buffer, bytes);
				dest += pixels;
				i -= pixels;
#if 0

				/* This runs at one half the speed of the
				 * above buffer-based code.
				 */
				if (!(3 & (u32)src) && i >= 4) {
					u32 a, b, *s=(u32*)src;
					if (get_user(a, s)) 
						return;
					s++;
					if (get_user(b, s)) 
						return;
					src+=4;

					fb_writew (a, dest);
					dest++;
					fb_writew (a>>16, dest);
					dest++;
					fb_writew (b, dest);
					dest++;
					fb_writew (b>>16, dest);
					dest++;
					i-=4;
				} else {
					u32 tmp;
					if (get_user(tmp, src)) 
						return;
					tmp &= 0xffff;
					src++;

					fb_writew (tmp, dest);
					dest++;
					i--;
				}
#endif
			}
			src_save += stride;
			src = src_save;
			dest_save += (info->fix.line_length>>1);
			dest = dest_save;
		}

dealloc2:
		kfree (buffer);
	  	}
		break;

	case FB_IMAGETYPE_GREY: {
		u8 *src, *src_save;
		src = src0;
		src_save = src;

		j = height;
		while (j-- > 0) {
			i = width;
			while (i > 0) {
				if (!(3 & (u32)src) && i >= 4) {
					u32 value, value2, *s=(u32*)src;
					if (get_user(value, s)) 
						return;
					src += 4;
					i -= 4;

					value2 = value & 0xff;
					value >>= 8;
					value2 = ((value2 << 8) & 0xf800) |
						((value2 << 3) & 0x7e0) |
						((value2 >> 3) & 31);
					fb_writew (value2, dest);
					dest++;

					value2 = value & 0xff;
					value >>= 8;
					value2 = ((value2 << 8) & 0xf800) |
						((value2 << 3) & 0x7e0) |
						((value2 >> 3) & 31);
					fb_writew (value2, dest);
					dest++;

					value2 = value & 0xff;
					value >>= 8;
					value2 = ((value2 << 8) & 0xf800) |
						((value2 << 3) & 0x7e0) |
						((value2 >> 3) & 31);
					fb_writew (value2, dest);
					dest++;

					value2 = value & 0xff;
					value >>= 8;
					value2 = ((value2 << 8) & 0xf800) |
						((value2 << 3) & 0x7e0) |
						((value2 >> 3) & 31);
					fb_writew (value2, dest);
					dest++;
				} else {
					u32 value;
					if (get_user(value, src)) 
						return;
					value &= 0xff;
					value = ((value << 8) & 0xf800) |
						((value << 3) & 0x7e0) |
						((value >> 3) & 31);
					src++;

					fb_writew (value, dest);
					dest++;
					i--;
				}
			}
			src_save += stride;
			src = src_save;
			dest_save += (info->fix.line_length>>1);
			dest = dest_save;
		}
	  	}
		break;
	}
}
#endif



/* Optimized function to put opaque GREY, RGB2, RGB3, & RGB4 data to a
 * 24bpp display.
 *
 * Benchmark results for writing 640x480 images:
 * Maximal values only.
 *
 * CPU:		P3 	P2	P1
 * Speed(MHz):	700E	233	200
 * VRAM:	AGP	PCI	PCI
 * ----------------------------------------------------------------------------
 * RGB4		?	21
 * RGB3		?	47
 * RGB2		?	20
 * GREY		?	28
 *
 */
static void vesa_putimage_opaque24 (struct fb_info *info, struct fb_put *p)
{
	int i, j;
        u32 offset;
	short xres, yres;
	short width, height;
	short x0, y0;
	u8 *src0;
	u8 src_bytes_per_pixel;
	u8 *dest, *dest_save;
	short stride;
	short x1, y1;
	short xstart, xend, ystart, yend;

	if (!info || !p)
		return;

	xstart = p->xstart;
	xend = p->xend;
	if (xstart > xend) {
		short tmp = xstart;
		xstart = xend;
		xend = tmp;
	}
	ystart = p->ystart;
	yend = p->yend;
	if (ystart > yend) {
		short tmp = ystart;
		ystart = yend;
		yend = tmp;
	}
	if (xend < 0 || yend < 0 || xstart >= p->width || ystart >= p->height)
		return;
	if (xstart < 0)
		xstart = 0;
	if (ystart < 0)
		ystart = 0;
	if (xend >= p->width)
		xend = p->width - 1;
	if (yend >= p->height)
		yend = p->height - 1;

	src0 = p->pixels;
	xres = info->var.xres;
	yres = info->var.yres;
	width = p->xend - p->xstart + 1;
	height = p->yend - p->ystart + 1;
	x0 = p->x0 + xstart;
	y0 = p->y0 + ystart;
	x1 = p->x0 + xend;
	y1 = p->y0 + yend;
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
		src_bytes_per_pixel = 4;
		break;
	default:
		return;
	}

	if (!src0 || width <= 0 || height <= 0 ||
	    x1 < 0 || y1 < 0 || x0 >= xres || y0 >= yres ||
	    stride <= 0)
		return;
	src0 += src_bytes_per_pixel * (stride * ystart + xstart);
	if (x0 < 0) {
		x0 = -x0;
		width -= x0;
		src0 += x0 * src_bytes_per_pixel;
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
		src0 += y0 * stride * src_bytes_per_pixel;
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
			src0 += diff * src_bytes_per_pixel;
			x0 = p->clip_x0;
		}
		if (x1 > p->clip_x1) {
			short diff = x1 - p->clip_x1;
			width -= diff;
			/* x1 = p->clip_x1; */
		}
		if (y0 < p->clip_y0) {
			short diff = p->clip_y0 - y0;
			height -= diff;
			src0 += diff * stride * src_bytes_per_pixel;
			y0 = p->clip_y0;
		}
		if (y1 > p->clip_y1) {
			short diff = y1 - p->clip_y1;
			height -= diff;
			/* y1 = p->clip_y1; */
		}
	}
	/*----------*/

	dest = (u8*) info->screen_base;
	offset = y0 * info->fix.line_length + 3 * x0;
	dest += offset;
	dest_save = dest;

	switch (p->type) {
	case FB_IMAGETYPE_RGB4: {
		u32 *src, *src_save;
		src = (u32*) src0;
		src_save = src;
		
		j = height;
		while (j-- > 0) {
			u32 value;
			i = width;
			while (i-- > 0) {
				if (get_user(value, src)) 
					return;
				src++;

				fb_writeb (value, dest); dest++;
				fb_writeb (value>>8, dest); dest++;
				fb_writeb (value>>16, dest); dest++;
			}
			src_save += stride;
			src = src_save;
			dest_save += info->fix.line_length;
			dest = dest_save;
		 }
	  	}
		break;

	case FB_IMAGETYPE_RGB3: {
		u8 *src = (u8*) src0, *src_save;
		u8 *buffer;

		/* What is needed is a function to transfer from memory chunks
		 * directly from userspace to memory mapped I/O. Without that,
		 * gotta kmalloc.
		 */
#define BUFSIZE24 3072
		buffer = kmalloc(BUFSIZE24, GFP_KERNEL);
		if (!buffer) {
			printk(KERN_INFO "vesa_putimage_opaque16: no memory\n");
			return;
		}

		src_save = src;

		j = height;
		while (j-- > 0) {
			i = width;
			while (i > 0) {
				u32 pixels = i > (BUFSIZE24/3) ? (BUFSIZE24/3) : i;
				u32 bytes = pixels * 3;

				/* Userspace -> L2 cache */
				if (copy_from_user((u8*)buffer,(u8*)src, bytes))
					goto dealloc3;
				src += pixels;

				/* L2 cache -> VRAM */
				memcpy_toio ((u8*)dest, (u8*)buffer, bytes);
				dest += pixels;
				i -= pixels;

#if 0
		j = height;
		while (j-- > 0) {
			u32 value;
			i = width;
			while (i-- > 0) {
				u8 a,b,c;

				if (get_user(a, src)) 
					return;
				src++;
				if (get_user(b, src)) 
					return;
				src++;
				if (get_user(c, src)) 
					return;
				src++;

				fb_writeb (a, dest); dest++; 
				fb_writeb (b, dest); dest++;
				fb_writeb (c, dest); dest++;
#endif
			}
			src_save += stride * 3;
			src = src_save;
			dest_save += info->fix.line_length;
			dest = dest_save;
		}
dealloc3:
		kfree (buffer);
	  	}
		break;

	case FB_IMAGETYPE_RGB2: {
		u16 *src, *src_save;
		src = (u16*) src0;
		src_save = src;

		j = height;
		while (j-- > 0) {
			i = width;
			while (i > 0) {
				u32 value;
				u32 tmp;

				if (get_user(tmp, src)) 
					return;
				src++;

				value = ((tmp << 8) & 0xf80000) |
					((tmp << 5) & 0xfc00) |
					((tmp << 3) & 0xf8);
				fb_writeb (value, dest); dest++; value >>= 8;
				fb_writeb (value, dest); dest++; value >>= 8;
				fb_writeb (value, dest); dest++;
				i--;
			}
			src_save += stride;
			src = src_save;
			dest_save += info->fix.line_length;
			dest = dest_save;
		 }
	  	}
		break;

	case FB_IMAGETYPE_GREY: {
		u8 *src, *src_save;
		src = src0;
		src_save = src;

		j = height;
		while (j-- > 0) {
			i = width;
			while (i > 0) {
				u8 value;
				if (get_user(value, src)) 
					return;
				src++;

				fb_writeb (value, dest); dest++;
				fb_writeb (value, dest); dest++;
				fb_writeb (value, dest); dest++;
				i--;
			}
			src_save += stride;
			src = src_save;
			dest_save += info->fix.line_length;
			dest = dest_save;
		 }
	  	}
		break;
	}
}



void vesa_putimage (struct fb_info *info, struct fb_put *p)
{
	u8 bytes_per_pixel;

	if (!info || !p)
		return;
	/*----------*/

	bytes_per_pixel = (info->var.bits_per_pixel + 7) >> 3;

	if (p->type == FB_IMAGETYPE_MONO) {
		vesa_putimage_mono (info, p);
		return;
	}
	else
	if (p->type != FB_IMAGETYPE_RGBA) {
		if (bytes_per_pixel==2 && info->mode565) {
#ifdef CONFIG_FB_UI_16BPP
			vesa_putimage_opaque16 (info, p);
			return;
#endif
		} else
		if (info->mode24) {
			if (bytes_per_pixel==4) {
#ifdef CONFIG_FB_UI_32BPP
				vesa_putimage_opaque32 (info, p);
#endif
			} else {
#ifdef CONFIG_FB_UI_24BPP
				vesa_putimage_opaque24 (info, p);
#endif
			}
			return;
		}
	}

	generic_putimage (info, p);
}
	

/* Benchmark results for optimized fillrect code below:
 *
 * CPU:		Cel. M	P3 	P2	P1
 * Speed(MHz):	1500	700E	233	200
 * VRAM:	Shared	AGP	PCI	PCI
 * ----------------------------------------------------------------------------
 * 16bpp	2441/s	?	146	32
 * 24bpp	-	?	70	-
 * 32bpp	1418	?	-	-
 *
 * The theoretical limit for 32bpp 33 MHz PCI in burst mode should be:
 *    33 * 1000000 / (640*480) = 107 per seconds 32bpp
 *    33 * 1000000 / (640*480) = 160 per seconds 24bpp
 *    33 * 1000000 / (640*480) = 214 per seconds 16bpp
 * Note, code below does not use PCI Burst mode.
 */
static void vesa_fillrect (struct fb_info *info, struct fb_draw *p)
{
	u32 offset;
	short xres, yres;
	short x0,x1,y0,y1;
	int i, j;

	if (!info || !p)
		return;
	x0 = p->x0;
	y0 = p->y0;
	x1 = p->x1;
	y1 = p->y1;
	if (p->color >> 24) {
		generic_fillrect (info, p);
		return;
	}

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

#if 0
	/* This crashes the console: */
	struct fb_fillrect fr;
	fr.dx = x0;
	fr.dy = y0;
	fr.width = x1 - x0;
	fr.height = y1 - y0;
	fr.color = p->color;
	fr.rop = ROP_COPY;

	cfb_fillrect (info, &fr);
	return;
#endif

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

	switch (((info->var.bits_per_pixel + 7) >> 3)) {
	case 4: {
#ifdef CONFIG_FB_UI_32BPP
		u32 *ptr, *ptr_save;
		short w = x1 - x0 + 1;

		ptr = ((u32*)info->screen_base);
		offset = y0 * (info->fix.line_length>>2) + x0;
		ptr += offset;
		ptr_save = ptr;

		j = y1 - y0 + 1;
		while (j-- > 0) {
			register u32 px = p->color;
			i = w;

			while (i > 0) {
				if (i >= 8) {
					register u32 *p = ptr;
					fb_writel (px, p);
					fb_writel (px, (p+1));
					fb_writel (px, (p+2));
					fb_writel (px, (p+3));
					fb_writel (px, (p+4));
					fb_writel (px, (p+5));
					fb_writel (px, (p+6));
					fb_writel (px, (p+7));
					ptr += 8;
					i -= 8;
				} else {
					fb_writel (px, ptr); ptr++;
					i--;
				}
			}

			ptr_save += info->fix.line_length>>2;
			ptr = ptr_save;
		}
#endif
		}
		break;

	case 3: {
#ifdef CONFIG_FB_UI_24BPP
		u8 *ptr, *ptr_save;
		u8 r,g,b;
		short w = x1 - x0 + 1;
		union {
			u32 l[3];
			u8 c[12];
		} stripe;
		r = p->color >> 16;
		g = p->color >> 8;
		b = p->color;
		for (i=0; i<=9; i+=3) {
			stripe.c[i] = b;
			stripe.c[i+1] = g;
			stripe.c[i+2] = r;
		}

		ptr = ((u8*)info->screen_base);
		offset = y0 * info->fix.line_length + x0*3;
		ptr += offset;
		ptr_save = ptr;

		j = y1 - y0 + 1;
		while (j-- > 0) {
			i = w;

			while (i > 0) {
				if (i>=12 && !(3 & (u32)ptr)) {
					register u32 a,b,c;
					register u32 *p = (u32*)ptr;
					a = stripe.l[0];
					b = stripe.l[1];
					c = stripe.l[2];
					fb_writel (a, p); p++;
					fb_writel (b, p); p++;
					fb_writel (c, p); p++;
					fb_writel (a, p); p++;
					fb_writel (b, p); p++;
					fb_writel (c, p); p++;
					fb_writel (a, p); p++;
					fb_writel (b, p); p++;
					fb_writel (c, p); p++;
					ptr += 36;
					i -= 12;
				} else {
					fb_writeb (b, ptr); ptr++;
					fb_writeb (g, ptr); ptr++;
					fb_writeb (r, ptr); ptr++;
					i--;
				}
			}

			ptr_save += info->fix.line_length;
			ptr = ptr_save;
		 }
#endif
		}
		break;

	case 2: {
#ifdef CONFIG_FB_UI_16BPP
		u16 *ptr, *ptr_save;
		u16 pix; 
		u32 pix2;
		short w = x1 - x0 + 1;

		pix = pixel_from_rgb (info,p->color);
		pix2 = pix;
		pix2 <<= 16;
		pix2 |= pix;
		ptr = ((u16*)info->screen_base);
		offset = y0 * (info->fix.line_length >> 1) + x0;
		ptr += offset;
		ptr_save = ptr;

		j = y1 - y0 + 1;
		while (j-- > 0) {
			i = w;

			while (i > 0) {
				if (!(3 & (u32)ptr) && i >= 8) {
					register u32 *p = (u32*) ptr;
					fb_writel (pix2, p);
					fb_writel (pix2, (1+p));
					fb_writel (pix2, (2+p));
					fb_writel (pix2, (3+p));
					ptr += 8;
					i -= 8;
				} else {
					fb_writew (pix, ptr); 
					ptr++;
					i--;
				}
			}

			ptr_save += info->fix.line_length>>1;
			ptr = ptr_save;
		}
#endif
		}
		break;
	}
}


static void vesa_copy_within (unsigned char *dest, unsigned char *src, u32 n)
{
	int dif = dest < src ? src - dest : dest - src;
	while (n) {
		if (dif >= 4 && n >= 16 && !(3 & (u32)src) && !(3 & (u32)dest)){
			/* This is most effective on pre-Pentium CPUs.
			 * and maybe could be shrunk down for >= Pentium.
			 */
			fb_writel (fb_readl (src), dest); src+=4; dest+=4;
			fb_writel (fb_readl (src), dest); src+=4; dest+=4;
			fb_writel (fb_readl (src), dest); src+=4; dest+=4;
			fb_writel (fb_readl (src), dest); src+=4; dest+=4;
			n -= 16;
		} else {
			fb_writeb (fb_readb (src), dest); src++; dest++;
			n--;
		}
	}
}


/* I wrote this routine because I need a robust copyarea,
 * supporting 16/24/32bpp, shifting up/down/left/right.
 */
void vesa_copyarea (struct fb_info *info, struct fb_draw *p)
{
        u32 bytes_per_pixel, offset, rasterlen;
        unsigned char *src;
        unsigned char *dest;
	short w, h;
	short xres, yres;
	u32 stride;
	short x0,y0,x1,y1,x2,y2;

	if (!info || !p)
		return;
	x0 = p->x0;
	y0 = p->y0;
	x1 = p->x1;
	y1 = p->y1;
	x2 = p->x2;
	y2 = p->y2;
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
	if (x1 < 0 || y1 < 0)
		return;
	xres = info->var.xres;
	yres = info->var.yres;
	if (x0 >= xres || y0 >= yres || x2 >= xres || y2 >= yres) 
		return;
	w = x1 - x0 + 1;
	h = y1 - y0 + 1;
	if (w<=0 || h<=0)
		return;
	if (x2+w-1 < 0 || y2+h-1 < 0) 
		return;
	if (x0 < 0) { 
		x2 -= x0; 
		w += x0; 
		x0=0;
	}
	if (y0 < 0) { 
		y2 -= y0;
		h += y0;
		y0=0;
	}
	if (x1 >= xres) { 
		short diff = x1 - xres + 1;
		w -= diff; 
		x1 = xres-1;
	}
	if (y1 >= yres) { 
		short diff = y1 - yres + 1;
		h -= diff; 
		y1 = yres-1;
	}
	if (x2+w-1 >= xres) { 
		short diff = x2 + w - xres;
		w -= diff; 
	}
	if (y2+h-1 >= yres) { 
		short diff = y2+h - yres;
		h -= diff; 
	}
	if (p->clip_valid) {
		if (x1 < p->clip_x0 || y1 < p->clip_y0)
			return;
		if (x2+w-1 < p->clip_x0 || y2+h-1 < p->clip_y0) 
			return;
		if (x0 > p->clip_x1 || y0 > p->clip_y1)
			return;
		if (x2 > p->clip_x1 || y2 > p->clip_y1)
			return;
		if (x0 < p->clip_x0) { 
			short diff = p->clip_x0 - x0;
			x2 += diff;
			w -= diff; 
			x0 = p->clip_x0;
		}
		if (y0 < p->clip_y0) { 
			short diff = p->clip_y0 - y0;
			y2 += diff;
			h -= diff; 
			y0 = p->clip_y0;
		}
		if (x1 > p->clip_x1) { 
			short diff = x1 - p->clip_x1;
			w -= diff; 
			x1 = p->clip_x1;
		}
		if (y1 > p->clip_y1) { 
			short diff = y1 - p->clip_y1;
			h -= diff; 
			y1 = p->clip_y1;
		}
		if (x2 < p->clip_x0) {
			short diff = p->clip_x0 - x2;
			x0 += diff;
			w -= diff;
			x2 = p->clip_x0;
		}
		if (y2 < p->clip_y0) {
			short diff = p->clip_y0 - y2;
			y0 += diff;
			h -= diff;
			y2 = p->clip_y0;
		}
		if (x2+w-1 > p->clip_x1) { 
			short diff = x2+w-1 - p->clip_x1;
			w -= diff; 
		}
		if (y2+h-1 > p->clip_y1) { 
			short diff = y2+h-1 - p->clip_y1;
			h -= diff; 
		}
	}
	/*----------*/

        bytes_per_pixel = (info->var.bits_per_pixel + 7) >> 3;
        rasterlen = info->fix.line_length;
        src = dest = info->screen_base;
	offset = y0 * rasterlen + x0 * bytes_per_pixel;
        src += offset;
	offset = y2 * rasterlen + x2 * bytes_per_pixel;
	dest += offset;
	stride = w * bytes_per_pixel;

	if (y2 < y0) {
#define BUFSIZECOPY 4096
		u8 *buffer = NULL;
		buffer = kmalloc (BUFSIZECOPY, GFP_KERNEL);
		/* moving upward */
		int j=0;
		while (j < h) {
			if (!buffer)
				vesa_copy_within (dest,src,stride);
			else {
				u8 *d = dest;
				u8 *s = src;
				long st = stride;
				while (st > 0) {
					u32 len = st > BUFSIZECOPY ? 
						BUFSIZECOPY : st;
					memcpy_fromio (buffer, s, len);
					memcpy_toio (d, buffer, len);
					st -= len;
					s += len;
					d += len;
				}
			}
			dest += rasterlen;
			src += rasterlen;
			j++;
		}
		kfree (buffer);
	} else
	if (y2 > y0) {
		u8 *buffer = NULL;
		int j=h-1;

		buffer = kmalloc (BUFSIZECOPY, GFP_KERNEL);

		/* moving downward */
		dest += rasterlen * h;
		src += rasterlen * h;
		while (j >= 0) {
			dest -= rasterlen;
			src -= rasterlen;
			if (!buffer)
				vesa_copy_within (dest,src,stride);
			else {
				u8 *d = dest;
				u8 *s = src;
				long st = stride;
				while (st > 0) {
					u32 len = st > BUFSIZECOPY ? 
						BUFSIZECOPY : st;
					memcpy_fromio (buffer, s, len);
					memcpy_toio (d, buffer, len);
					st -= len;
					s += len;
					d += len;
				}
			}
			j--;
		}
		kfree (buffer);
	} else { 
		/* y equal */

		u8 *buffer = NULL;
		buffer = kmalloc (BUFSIZECOPY, GFP_KERNEL);
		/* moving upward */
		int j=0;
		while (j < h) {
			if (!buffer)
				vesa_copy_within (dest,src,stride);
			else {
				u8 *d = dest;
				u8 *s = src;
				long st = stride;
				while (st > 0) {
					u32 len = st > BUFSIZECOPY ? 
						BUFSIZECOPY : st;
					memcpy_fromio (buffer, s, len);
					memcpy_toio (d, buffer, len);
					st -= len;
					s += len;
					d += len;
				}
			}
			dest += rasterlen;
			src += rasterlen;
			j++;
		}
		kfree (buffer);
#if 0
		/* leftward */
		if (x2 < x0) {
			int j=0;
			while (j < h) {
				register unsigned char *d = dest;
				register unsigned char *s = src;
				register int i = w * bytes_per_pixel;
				while (i) {
					if (!(3 & (u32) s) && i>=4) {
						u32 v = fb_readl (s); s+= 4;
						if (!(3 & (u32) d)) {
							u32 *s2 = (u32*)s;
							u32 *d2 = (u32*)d
;
							fb_writel (v, d); 
							d += 4;
							d2++;
							i -= 4;

							while (i >= 16) {
								v=fb_readl (s2);
								s2++;
								fb_writel(v,d2);
								d2++;
								v=fb_readl (s2);
								s2++;
								fb_writel(v,d2);
								d2++;
								v=fb_readl (s2);
								s2++;
								fb_writel(v,d2);
								d2++;
								v=fb_readl (s2);
								s2++;
								fb_writel(v,d2);
								d2++;
								s += 16;
								d += 16;
								i -= 16;
							}
						} else {
							fb_writeb (v, d); 
							d++; v >>= 8;
							fb_writeb (v, d); 
							d++; v >>= 8;
							fb_writeb (v, d); 
							d++; v >>= 8;
							fb_writeb (v, d); 
							d++;
							i -= 4;
						}
					} else {
						fb_writeb (fb_readb (s), d);
						s++;
						d++;
						i--;
					}
				}
				dest += rasterlen;
				src += rasterlen;
				j++;
			}
			
		} else {
		/* rightward */
			int j=0;
			while (j < h) {
				int i = w * bytes_per_pixel;
				unsigned char *d = dest + i - 1;
				unsigned char *s = src + i - 1;
				while (i) {
					if (3==(3 & (u32) s) && i>=4) 
					{
						u32 v;
						s -= 3;
						v = fb_readl (s);
						if (3 == (3 & (u32) d)) {
							d -= 3;
							fb_writel (v, d);
							i -= 4;

							s--;
							d--;
							while (i >= 32) {
								s -= 3;
								d -= 3;
								v = fb_readl(s);
								s -= 4;
								fb_writel(v,d);
								d -= 4;
								v = fb_readl(s);
								s -= 4;
								fb_writel(v,d);
								d -= 4;
								v = fb_readl(s);
								s -= 4;
								fb_writel(v,d);
								d -= 4;
								v = fb_readl(s);
								s -= 4;
								fb_writel(v,d);
								d -= 4;
								v = fb_readl(s);
								s -= 4;
								fb_writel(v,d);
								d -= 4;
								v = fb_readl(s);
								s -= 4;
								fb_writel(v,d);
								d -= 4;
								v = fb_readl(s);
								s -= 4;
								fb_writel(v,d);
								d -= 4;
								v = fb_readl(s);
								s -= 4;
								fb_writel(v,d);
								d -= 4;
								s += 3;
								d += 3;
								i -= 32;
							}
						} else {
							d--;
							fb_writeb (v>>24, d); 
							d--;
							fb_writeb (v>>16, d); 
							d--;
							fb_writeb (v>>8, d); 
							d--;
							fb_writeb (v, d); 
							i -= 4;
						}
					} else {
						fb_writeb (fb_readb (s), d);
						s--;
						d--;
						i--;
					}
				}
				dest += rasterlen;
				src += rasterlen;
				j++;
			}
		}
#endif
	}
}


void vesa_line (struct fb_info *info, struct fb_draw *p)
{
	struct fb_dda dda;
	u8 use_clip=0;
	u8 bytes_per_pixel;
	short xres, yres;
	u32 pixel;

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

	if (p->color >> 24) {
		generic_line (info,p);
		return;
	}

        bytes_per_pixel = (info->var.bits_per_pixel + 7) >> 3;
	pixel = pixel_from_rgb (info, p->color);
	use_clip = p->clip_valid;
	init_bresenham (&dda, p->x0, p->y0, p->x1, p->y1);

	while (dda.j <= dda.dx) {
		dda.j++;

		if (!use_clip || 
		    (dda.x >= p->clip_x0 && 
		     dda.x <= p->clip_x1 && 
		     dda.y >= p->clip_y0 && 
		     dda.y <= p->clip_y1)) {
			u8 *dest = info->screen_base;
			u32 offset = dda.y * info->fix.line_length + 
					dda.x * bytes_per_pixel;
			dest += offset;
			switch (bytes_per_pixel) {
			case 2:
				fb_writew (pixel, (u16*)dest);
				break;
			case 3: {
				u32 p = pixel;
				fb_writeb (p, dest); dest++; p >>= 8;
				fb_writeb (p, dest); dest++; p >>= 8;
				fb_writeb (p, dest);
			 }
				break;
			case 4:
				fb_writel (pixel, (u32*)dest);
				break;
			}
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
}

#endif /* FBUI */

/* --------------------------------------------------------------------- */

static int vesafb_pan_display(struct fb_var_screeninfo *var,
                              struct fb_info *info)
{
#ifdef __i386__
	int offset;

	if (!ypan)
		return -EINVAL;
	if (var->xoffset)
		return -EINVAL;
	if (var->yoffset > var->yres_virtual)
		return -EINVAL;
	if ((ypan==1) && var->yoffset+var->yres > var->yres_virtual)
		return -EINVAL;

	offset = (var->yoffset * info->fix.line_length + var->xoffset) / 4;

        __asm__ __volatile__(
                "call *(%%edi)"
                : /* no return value */
                : "a" (0x4f07),         /* EAX */
                  "b" (0),              /* EBX */
                  "c" (offset),         /* ECX */
                  "d" (offset >> 16),   /* EDX */
                  "D" (&pmi_start));    /* EDI */
#endif
	return 0;
}

static void vesa_setpalette(int regno, unsigned red, unsigned green,
			    unsigned blue, struct fb_var_screeninfo *var)
{
#ifdef __i386__
	struct { u_char blue, green, red, pad; } entry;
	int shift = 16 - var->green.length;

	if (pmi_setpal) {
		entry.red   = red   >> shift;
		entry.green = green >> shift;
		entry.blue  = blue  >> shift;
		entry.pad   = 0;
	        __asm__ __volatile__(
                "call *(%%esi)"
                : /* no return value */
                : "a" (0x4f09),         /* EAX */
                  "b" (0),              /* EBX */
                  "c" (1),              /* ECX */
                  "d" (regno),          /* EDX */
                  "D" (&entry),         /* EDI */
                  "S" (&pmi_pal));      /* ESI */
	} else {
		/* without protected mode interface, try VGA registers... */
		outb_p(regno,       dac_reg);
		outb_p(red   >> shift, dac_val);
		outb_p(green >> shift, dac_val);
		outb_p(blue  >> shift, dac_val);
	}
#endif
}

static int vesafb_setcolreg(unsigned regno, unsigned red, unsigned green,
			    unsigned blue, unsigned transp,
			    struct fb_info *info)
{
	/*
	 *  Set a single color register. The values supplied are
	 *  already rounded down to the hardware's capabilities
	 *  (according to the entries in the `var' structure). Return
	 *  != 0 for invalid regno.
	 */
	
	if (regno >= info->cmap.len)
		return 1;

	switch (info->var.bits_per_pixel) {
	case 8:
		vesa_setpalette(regno,red,green,blue, &info->var);
		break;
	case 16:
		if (info->var.red.offset == 10) {
			/* 1:5:5:5 */
			((u32*) (info->pseudo_palette))[regno] =	
					((red   & 0xf800) >>  1) |
					((green & 0xf800) >>  6) |
					((blue  & 0xf800) >> 11);
		} else {
			/* 0:5:6:5 */
			((u32*) (info->pseudo_palette))[regno] =	
					((red   & 0xf800)      ) |
					((green & 0xfc00) >>  5) |
					((blue  & 0xf800) >> 11);
		}
		break;
	case 24:
		red   >>= 8;
		green >>= 8;
		blue  >>= 8;
		((u32 *)(info->pseudo_palette))[regno] =
			(red   << info->var.red.offset)   |
			(green << info->var.green.offset) |
			(blue  << info->var.blue.offset);
		break;
	case 32:
		red   >>= 8;
		green >>= 8;
		blue  >>= 8;
		((u32 *)(info->pseudo_palette))[regno] =
			(red   << info->var.red.offset)   |
			(green << info->var.green.offset) |
			(blue  << info->var.blue.offset);
		break;
    }
    return 0;
}

static struct fb_ops vesafb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= vesafb_setcolreg,
	.fb_pan_display	= vesafb_pan_display,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_cursor	= soft_cursor,

#ifdef CONFIG_FB_UI
        /* Assign routines from fbui.c */
	.fb_sync		= NULL,
        .fb_release             = fbui_release,
        .fb_line                = vesa_line,
        .fb_filltriangle	= generic_filltriangle,
        .fb_putimage		= vesa_putimage,
        .fb_fillrect2		= vesa_fillrect,
        .fb_copyarea2           = vesa_copyarea,
        .fb_read_point          = generic_read_point,
        .fb_getpixels_rgb       = generic_getpixels_rgb,
#endif
};

int __init vesafb_setup(char *options)
{
	char *this_opt;
	
	if (!options || !*options)
		return 0;
	
	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt) continue;
		
		if (! strcmp(this_opt, "inverse"))
			inverse=1;
		else if (! strcmp(this_opt, "redraw"))
			ypan=0;
		else if (! strcmp(this_opt, "ypan"))
			ypan=1;
		else if (! strcmp(this_opt, "ywrap"))
			ypan=2;
		else if (! strcmp(this_opt, "vgapal"))
			pmi_setpal=0;
		else if (! strcmp(this_opt, "pmipal"))
			pmi_setpal=1;
		else if (! strcmp(this_opt, "mtrr"))
			mtrr=1;
		else if (! strcmp(this_opt, "nomtrr"))
			mtrr=0;
		else if (! strncmp(this_opt, "vram:", 5))
			vram = simple_strtoul(this_opt+5, NULL, 0);
	}
	return 0;
}

static int __init vesafb_probe(struct device *device)
{
	struct platform_device *dev = to_platform_device(device);
	struct fb_info *info;
	int i, err;

	if (screen_info.orig_video_isVGA != VIDEO_TYPE_VLFB)
		return -ENXIO;

	vesafb_fix.smem_start = screen_info.lfb_base;
	vesafb_defined.bits_per_pixel = screen_info.lfb_depth;
	if (15 == vesafb_defined.bits_per_pixel)
		vesafb_defined.bits_per_pixel = 16;
	vesafb_defined.xres = screen_info.lfb_width;
	vesafb_defined.yres = screen_info.lfb_height;
	vesafb_fix.line_length = screen_info.lfb_linelength;

	/* Allocate enough memory for double buffering */
	vesafb_fix.smem_len = screen_info.lfb_width * screen_info.lfb_height * vesafb_defined.bits_per_pixel >> 2;

	/* check that we don't remap more memory than old cards have */
	if (vesafb_fix.smem_len > (screen_info.lfb_size * 65536))
		vesafb_fix.smem_len = screen_info.lfb_size * 65536;

	/* Set video size according to vram boot option */
	if (vram)
		vesafb_fix.smem_len = vram * 1024 * 1024;

	vesafb_fix.visual   = (vesafb_defined.bits_per_pixel == 8) ?
		FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR;

	/* limit framebuffer size to 16 MB.  Otherwise we'll eat tons of
	 * kernel address space for nothing if the gfx card has alot of
	 * memory (>= 128 MB isn't uncommon these days ...) */
	if (vesafb_fix.smem_len > 16 * 1024 * 1024)
		vesafb_fix.smem_len = 16 * 1024 * 1024;

#ifndef __i386__
	screen_info.vesapm_seg = 0;
#endif

	if (!request_mem_region(vesafb_fix.smem_start, vesafb_fix.smem_len, "vesafb")) {
		printk(KERN_WARNING
		       "vesafb: abort, cannot reserve video memory at 0x%lx\n",
			vesafb_fix.smem_start);
		/* We cannot make this fatal. Sometimes this comes from magic
		   spaces our resource handlers simply don't know about */
	}

	info = framebuffer_alloc(sizeof(u32) * 256, &dev->dev);
	if (!info) {
		release_mem_region(vesafb_fix.smem_start, vesafb_fix.smem_len);
		return -ENOMEM;
	}
	info->pseudo_palette = info->par;
	info->par = NULL;

        info->screen_base = ioremap(vesafb_fix.smem_start, vesafb_fix.smem_len);
	if (!info->screen_base) {
		printk(KERN_ERR
		       "vesafb: abort, cannot ioremap video memory 0x%x @ 0x%lx\n",
			vesafb_fix.smem_len, vesafb_fix.smem_start);
		err = -EIO;
		goto err;
	}

	printk(KERN_INFO "vesafb: framebuffer at 0x%lx, mapped to 0x%p, size %dk\n",
	       vesafb_fix.smem_start, info->screen_base, vesafb_fix.smem_len/1024);
	printk(KERN_INFO "vesafb: mode is %dx%dx%d, linelength=%d, pages=%d\n",
	       vesafb_defined.xres, vesafb_defined.yres, vesafb_defined.bits_per_pixel, vesafb_fix.line_length, screen_info.pages);

	if (screen_info.vesapm_seg) {
		printk(KERN_INFO "vesafb: protected mode interface info at %04x:%04x\n",
		       screen_info.vesapm_seg,screen_info.vesapm_off);
	}

	if (screen_info.vesapm_seg < 0xc000)
		ypan = pmi_setpal = 0; /* not available or some DOS TSR ... */

	if (ypan || pmi_setpal) {
		pmi_base  = (unsigned short*)phys_to_virt(((unsigned long)screen_info.vesapm_seg << 4) + screen_info.vesapm_off);
		pmi_start = (void*)((char*)pmi_base + pmi_base[1]);
		pmi_pal   = (void*)((char*)pmi_base + pmi_base[2]);
		printk(KERN_INFO "vesafb: pmi: set display start = %p, set palette = %p\n",pmi_start,pmi_pal);
		if (pmi_base[3]) {
			printk(KERN_INFO "vesafb: pmi: ports = ");
				for (i = pmi_base[3]/2; pmi_base[i] != 0xffff; i++)
					printk("%x ",pmi_base[i]);
			printk("\n");
			if (pmi_base[i] != 0xffff) {
				/*
				 * memory areas not supported (yet?)
				 *
				 * Rules are: we have to set up a descriptor for the requested
				 * memory area and pass it in the ES register to the BIOS function.
				 */
				printk(KERN_INFO "vesafb: can't handle memory requests, pmi disabled\n");
				ypan = pmi_setpal = 0;
			}
		}
	}

	vesafb_defined.xres_virtual = vesafb_defined.xres;
	vesafb_defined.yres_virtual = vesafb_fix.smem_len / vesafb_fix.line_length;
	if (ypan && vesafb_defined.yres_virtual > vesafb_defined.yres) {
		printk(KERN_INFO "vesafb: scrolling: %s using protected mode interface, yres_virtual=%d\n",
		       (ypan > 1) ? "ywrap" : "ypan",vesafb_defined.yres_virtual);
	} else {
		printk(KERN_INFO "vesafb: scrolling: redraw\n");
		vesafb_defined.yres_virtual = vesafb_defined.yres;
		ypan = 0;
	}

	/* some dummy values for timing to make fbset happy */
	vesafb_defined.pixclock     = 10000000 / vesafb_defined.xres * 1000 / vesafb_defined.yres;
	vesafb_defined.left_margin  = (vesafb_defined.xres / 8) & 0xf8;
	vesafb_defined.hsync_len    = (vesafb_defined.xres / 8) & 0xf8;
	
	vesafb_defined.red.offset    = screen_info.red_pos;
	vesafb_defined.red.length    = screen_info.red_size;
	vesafb_defined.green.offset  = screen_info.green_pos;
	vesafb_defined.green.length  = screen_info.green_size;
	vesafb_defined.blue.offset   = screen_info.blue_pos;
	vesafb_defined.blue.length   = screen_info.blue_size;
	vesafb_defined.transp.offset = screen_info.rsvd_pos;
	vesafb_defined.transp.length = screen_info.rsvd_size;
	printk(KERN_INFO "vesafb: %s: "
	       "size=%d:%d:%d:%d, shift=%d:%d:%d:%d\n",
	       (vesafb_defined.bits_per_pixel > 8) ?
	       "Truecolor" : "Pseudocolor",
	       screen_info.rsvd_size,
	       screen_info.red_size,
	       screen_info.green_size,
	       screen_info.blue_size,
	       screen_info.rsvd_pos,
	       screen_info.red_pos,
	       screen_info.green_pos,
	       screen_info.blue_pos);

	vesafb_fix.ypanstep  = ypan     ? 1 : 0;
	vesafb_fix.ywrapstep = (ypan>1) ? 1 : 0;

	/* request failure does not faze us, as vgacon probably has this
	 * region already (FIXME) */
	request_region(0x3c0, 32, "vesafb");

	if (mtrr) {
		int temp_size = vesafb_fix.smem_len;
		/* Find the largest power-of-two */
		while (temp_size & (temp_size - 1))
                	temp_size &= (temp_size - 1);
                        
                /* Try and find a power of two to add */
		while (temp_size && mtrr_add(vesafb_fix.smem_start, temp_size, MTRR_TYPE_WRCOMB, 1)==-EINVAL) {
			temp_size >>= 1;
		}
	}
	
	info->fbops = &vesafb_ops;
	info->var = vesafb_defined;
	info->fix = vesafb_fix;
	info->flags = FBINFO_FLAG_DEFAULT |
		(ypan) ? FBINFO_HWACCEL_YPAN : 0;

	if (fb_alloc_cmap(&info->cmap, 256, 0) < 0) {
		err = -ENXIO;
		goto err;
	}
	if (register_framebuffer(info)<0) {
		err = -EINVAL;
		fb_dealloc_cmap(&info->cmap);
		goto err;
	}
	printk(KERN_INFO "fb%d: %s frame buffer device\n",
	       info->node, info->fix.id);

#ifdef CONFIG_FB_UI
        fbui_init (info);
#endif

	return 0;
err:
	framebuffer_release(info);
	release_mem_region(vesafb_fix.smem_start, vesafb_fix.smem_len);
	return err;
}

static struct device_driver vesafb_driver = {
	.name	= "vesafb",
	.bus	= &platform_bus_type,
	.probe	= vesafb_probe,
};

static struct platform_device vesafb_device = {
	.name	= "vesafb",
};

int __init vesafb_init(void)
{
	int ret;
	char *option = NULL;

	/* ignore error return of fb_get_options */
	fb_get_options("vesafb", &option);
	vesafb_setup(option);
	ret = driver_register(&vesafb_driver);

	if (!ret) {
		ret = platform_device_register(&vesafb_device);
		if (ret)
			driver_unregister(&vesafb_driver);
	}
	return ret;
}
module_init(vesafb_init);

/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */

MODULE_LICENSE("GPL");

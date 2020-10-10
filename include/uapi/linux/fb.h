/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_FB_H
#define _UAPI_LINUX_FB_H

#include <linux/types.h>
#include <linux/i2c.h>

/* Definitions of frame buffers						*/

#define FB_MAX			32	/* sufficient for now */

/* ioctls
   0x46 is 'F'								*/
#define FBIOGET_VSCREENINFO	0x4600
#define FBIOPUT_VSCREENINFO	0x4601
#define FBIOGET_FSCREENINFO	0x4602
#define FBIOGETCMAP		0x4604
#define FBIOPUTCMAP		0x4605
#define FBIOPAN_DISPLAY		0x4606
#ifndef __KERNEL__
#define FBIO_CURSOR            _IOWR('F', 0x08, struct fb_cursor)
#endif
/* 0x4607-0x460B are defined below */
/* #define FBIOGET_MONITORSPEC	0x460C */
/* #define FBIOPUT_MONITORSPEC	0x460D */
/* #define FBIOSWITCH_MONIBIT	0x460E */
#define FBIOGET_CON2FBMAP	0x460F
#define FBIOPUT_CON2FBMAP	0x4610
#define FBIOBLANK		0x4611		/* arg: 0 or vesa level + 1 */
#define FBIOGET_VBLANK		_IOR('F', 0x12, struct fb_vblank)
#define FBIO_ALLOC              0x4613
#define FBIO_FREE               0x4614
#define FBIOGET_GLYPH           0x4615
#define FBIOGET_HWCINFO         0x4616
#define FBIOPUT_MODEINFO        0x4617
#define FBIOGET_DISPINFO        0x4618
#define FBIO_WAITFORVSYNC	_IOW('F', 0x20, __u32)

/* ==========================================================================*/
/* FBUI ioctls */
#define FBIO_UI_OPEN            0x4619  /* arg = ptr to fb_ui_open */
#define FBIO_UI_CLOSE           0x461a  /* no arg */
	/* Exec commands are queued and are not executed if window
	 * is not visible. */
#define FBIO_UI_EXEC            0x461b  /* arg = ptr to array of shorts (1st=count) */
	/* Control commands are _not_ queued and are always executed*/
#define FBIO_UI_CONTROL		0x461c  /* arg = ptr to fbui_ctrl struct */
#define FBUI_NAMELEN 32
typedef unsigned long RGB;

/* FBUI program type hints for fbui_open */
#define FBUI_PROGTYPE_NONE 0
#define FBUI_PROGTYPE_APP 1		/* e.g. takes main area */
#define FBUI_PROGTYPE_LAUNCHER 2	/* e.g. takes bottom row area */
#define FBUI_PROGTYPE_TOOL 3		/* e.g. fbclock: takes right area permanently */
#define FBUI_PROGTYPE_EPHEMERAL 4	/* e.g. calculator */
#define FBUI_PROGTYPE_WM 	5	/* window mgr, panel mgr */
#define FBUI_PROGTYPE_LAST 6 /* not for use */

/* A few accelerator keys encoded into 0..31 */
#define FBUI_ACCEL_PRTSC 1
#define FBUI_ACCEL_HOME 2
#define FBUI_ACCEL_END 3
#define FBUI_ACCEL_PGUP 4
#define FBUI_ACCEL_PGDN 5

#define FBUI_BUTTON_DOWN 1
#define FBUI_BUTTON_LEFT 2
#define FBUI_BUTTON_RIGHT 4
#define FBUI_BUTTON_MIDDLE 8


/* Passed _in_ to FBIO_UI_OPEN ioctl */
struct fbui_open {
	char	desired_vc;	/* -1 = current */
	unsigned char	req_control;   	/* maximum one window manager per vc */
	unsigned char	doing_autoposition;   /* used to differentiate btw fbwm & fbpm */
	unsigned char	program_type;	/* if !0, window is hidden upon creation, wm informed */
	unsigned char	need_keys;	/* key focus */
	unsigned char	receive_all_motion;	/* supported for window manager only */
	unsigned char	initially_hidden;
	short 	x;
	short 	y;
	short 	width;
	short 	height;
	short	max_width;
	short	max_height;
	__u32 	bgcolor;
	char 	name [FBUI_NAMELEN];
	char 	subtitle [FBUI_NAMELEN];
	__u32	usermask;   // not used
	short	maskwidth;
	short	maskheight;
};

/* Data passed _out_ kernel via FBUI_WININFO command */
struct fbui_wininfo {
	short	id;
	int	pid;
	unsigned char	program_type;
	unsigned char	hidden;
	unsigned char	need_placement;
	unsigned char	need_keys;
	short	x, y;
	short	width, height;
	short	max_width;
	short	max_height;
	char 	name [FBUI_NAMELEN];
	char 	subtitle [FBUI_NAMELEN];
};

#define FBUI_RECT_ARYSIZE 14
struct fbui_rects {
	short	c [FBUI_RECT_ARYSIZE * 4]; /* [x0,y0,x1,y1]* */ 
	unsigned short	flags;
	short	total;
	struct fbui_rects *next;
};

/* Data passed _out_ kernel via FBUI_POLLEVENT & FBUI_WAITEVENT commands */
struct fbui_event {
	char	type;
	char	has_rects;	/* returned to userspace */
	short	id;
	int	pid;
	short	x, y;
	short	width, height;
	short	key;
	struct fbui_rects *rects;	/* kernel-only ptr */
};

/* Used by fbui_moveresize_window to tell the app what happened: */
#define FBUI_SIZE_CHANGED 1
#define FBUI_POSITION_CHANGED 2
#define FBUI_CONTENTS_CHANGED 4

/* Passed _in_ to FBIO_UI_CONTROL
 */
struct fbui_ctrl {
	char	op;
	short	id;
	short	id2; /* used by wm */
	short	x,y;
	short 	width,height;
	struct fbui_wininfo	*info;	/* passed out */
	int	ninfo;
	unsigned char	*pointer;
	unsigned long	cutpaste_length;
	struct fbui_event 	*event;	/* passed out */
	char	string [FBUI_NAMELEN];	/* passed in */
	struct fbui_rects	*rects;	/* passed out */
};

#define FB_IMAGETYPE_RGB2	0	/* 16 bpp (5-6-5) */
#define FB_IMAGETYPE_RGB3	1	/* 24 bpp (8-8-8) */
#define FB_IMAGETYPE_RGB4	2	/* 24 bpp (8-8-8) + 8b zeroes */
#define FB_IMAGETYPE_RGBA	3	/* 24 bpp (8-8-8) + 8b transparency */
#define FB_IMAGETYPE_GREY	4	/* 8 bits grey => 24 bits RGB */
#define FB_IMAGETYPE_MONO	5	/* 1 bpp, 1=>color 0=>transparent */

#define FBUI_EVENTMASK_KEY	1
#define FBUI_EVENTMASK_EXPOSE	2
#define FBUI_EVENTMASK_HIDE	4
#define FBUI_EVENTMASK_UNHIDE	8
#define FBUI_EVENTMASK_ENTER	16
#define FBUI_EVENTMASK_LEAVE	32
#define FBUI_EVENTMASK_MR	64
#define FBUI_EVENTMASK_ACCEL	128
#define FBUI_EVENTMASK_WC	256
#define FBUI_EVENTMASK_MOTION	512
#define FBUI_EVENTMASK_BUTTON	1024
#define FBUI_EVENTMASK_RAISE	2048
#define FBUI_EVENTMASK_LOWER	4096
#define FBUI_EVENTMASK_ALL 0x7fff

/* Commands for FBIO_UI_CONTROL ioctl */
#define FBUI_NOOP	0
#define FBUI_POLLEVENT	1
#define FBUI_READMOUSE	2
#define FBUI_READPOINT	3	/* wm only */
#define FBUI_ACCEL	4
#define FBUI_WININFO	5	/* wm only */
#define FBUI_SUSPEND	6	/* wm only */
#define FBUI_RESUME	7
#define FBUI_GETPOSN	8
#define FBUI_WAITEVENT	9
#define FBUI_PLACEMENT	10	/* wm only */
#define FBUI_CUT	11
#define FBUI_PASTE	12
#define FBUI_CUTLENGTH	13
#define FBUI_SUBTITLE	14
#define FBUI_MOVERESIZE	16
#define FBUI_ERRNO	17
#define FBUI_GETCONSOLE	18
#define FBUI_RAISE	19
#define FBUI_LOWER	20
#define FBUI_HIDE 	21
#define FBUI_UNHIDE 	22
#define FBUI_GETDIMS	23
#define FBUI_ICON	24
#define FBUI_BEEP	25

#define FBUI_WM_ONLY 32
#define FBUI_REDRAW	(FBUI_WM_ONLY+0)
#define FBUI_DELETE	(FBUI_WM_ONLY+1)
#define FBUI_ASSIGN_KEYFOCUS	(FBUI_WM_ONLY+2)
#define FBUI_ASSIGN_PTRFOCUS	(FBUI_WM_ONLY+3)
#define FBUI_GETICON	(FBUI_WM_ONLY+4)
#define FBUI_XYTOWINDOW	(FBUI_WM_ONLY+5)

#define FBUI_ICON_WIDTH 24
#define FBUI_ICON_HEIGHT 32
#define FBUI_ICON_DEPTH 32

/* Some useful colors */
#define RGB_NOCOLOR	0xff000000
#define RGB_TRANSPARENT	0xff000000
#define RGB_BLACK 	0
#define RGB_GRAY	0xa0a0a0
#define RGB_GREY	0xa0a0a0
#define RGB_WHITE 	0xffffff
#define RGB_RED 	0xff0000
#define RGB_GREEN	0xff00
#define RGB_DARKGREEN	0xA000
#define RGB_BLUE	0xff
#define RGB_NAVYBLUE	0x80
#define RGB_CYAN	0xffff
#define RGB_YELLOW	0xffff00
#define RGB_MAGENTA	0xff00ff
#define RGB_ORANGE 	0xffa000
#define RGB_PURPLE	0xa030ff
#define RGB_LTBROWN	0xb54c4c
#define RGB_BROWN	0xa52c2c
#define RGB_STEELBLUE 	0x4682B4
#define RGB_SIENNA	0x605230

/* FBUI event types. Events are 31-bit values; type is lower 4 bits */
#define FBUI_EVENT_NONE 	0
#define FBUI_EVENT_EXPOSE 	1	/* expose without rects */
#define FBUI_EVENT_HIDE 	2
#define FBUI_EVENT_UNHIDE 	3
#define FBUI_EVENT_ENTER 	4	/* future... mouse pointer enter */
#define FBUI_EVENT_LEAVE 	5	/* future... mouse pointer leave */
#define FBUI_EVENT_KEY 		6
#define FBUI_EVENT_MOVERESIZE	7	/* moved and resized => need redraw */
#define FBUI_EVENT_ACCEL 	8	/* keyboard accelerator (Alt-) key */
#define FBUI_EVENT_WINCHANGE 	9	/* recv'd only by window manager */
#define FBUI_EVENT_MOTION	10	/* mouse pointer moved */
#define FBUI_EVENT_BUTTON	11	/* mouse button activity */
#define FBUI_EVENT_RAISE	12
#define FBUI_EVENT_LOWER	13
#define FBUI_EVENT_CLICKAREA	14	/* received only by wm */
#define FBUI_EVENT_NAMECHANGE 	15	/* received only by wm */
#define FBUI_EVENT_LAST 15

/* FBUI queued commands: for use with FBIO_UI_EXEC ioctl */
#define FBUI_NONE 	0
#define FBUI_COPYAREA 	1
#define FBUI_POINT 	2
#define FBUI_LINE 	3
#define FBUI_LINETO	4
#define FBUI_HLINE 	5
#define FBUI_VLINE 	6
#define FBUI_RECT 	7
#define FBUI_FILLRECT 	8
#define FBUI_CLEAR 	9
#define FBUI_CLEARAREA	10
#define FBUI_IMAGE	11
#define FBUI_FULLIMAGE	12
#define FBUI_MONOIMAGE	13
#define FBUI_TRIANGLE	14

/* FBUI ioctl return values */
#define FBUI_SUCCESS 0
#define FBUI_ERR_BADADDR -254
#define FBUI_ERR_NULLPTR -253
#define FBUI_ERR_OFFSCREEN -252
#define FBUI_ERR_NOTRUNNING -251
#define FBUI_ERR_WRONGVISUAL -250
#define FBUI_ERR_NOTPLACED -249
#define FBUI_ERR_BIGENDIAN -248
#define FBUI_ERR_INVALIDCMD -247
#define FBUI_ERR_BADPID -246
#define FBUI_ERR_ACCELBUSY -245
#define FBUI_ERR_BADCMD -244
#define FBUI_ERR_NOMEM -243
#define FBUI_ERR_NOTOPEN -242
#define FBUI_ERR_OVERLAP -241
#define FBUI_ERR_ALREADYOPEN -240
#define FBUI_ERR_MISSINGWIN -239
#define FBUI_ERR_NOTWM -238
#define FBUI_ERR_WRONGWM -237
#define FBUI_ERR_HAVEWM -236
#define FBUI_ERR_KEYFOCUSDENIED -235
#define FBUI_ERR_KEYFOCUSERR -234
#define FBUI_ERR_BADPARAM -233
#define FBUI_ERR_NOMOUSE -232
#define FBUI_ERR_MOUSEREAD -231
#define FBUI_ERR_OVERLARGECUT -230
#define FBUI_ERR_BADWIN -229
#define FBUI_ERR_PASTEFAIL -228
#define FBUI_ERR_CUTFAIL -227
#define FBUI_ERR_NOEVENT -226
#define FBUI_ERR_DRAWING -225
#define FBUI_ERR_MISSINGPROCENT -224
#define FBUI_ERR_BADVC -223
#define FBUI_ERR_NOT_OPERATIONAL -222
#define FBUI_ERR_INTERNAL -221
#define FBUI_ERR_WINDELETED -220
#define FBUI_ERR_NOICON -219
#define FBUI_ERR_OBSCURED -218

/* ==========================================================================*/

#define FB_TYPE_PACKED_PIXELS		0	/* Packed Pixels	*/
#define FB_TYPE_PLANES			1	/* Non interleaved planes */
#define FB_TYPE_INTERLEAVED_PLANES	2	/* Interleaved planes	*/
#define FB_TYPE_TEXT			3	/* Text/attributes	*/
#define FB_TYPE_VGA_PLANES		4	/* EGA/VGA planes	*/
#define FB_TYPE_FOURCC			5	/* Type identified by a V4L2 FOURCC */

#define FB_AUX_TEXT_MDA		0	/* Monochrome text */
#define FB_AUX_TEXT_CGA		1	/* CGA/EGA/VGA Color text */
#define FB_AUX_TEXT_S3_MMIO	2	/* S3 MMIO fasttext */
#define FB_AUX_TEXT_MGA_STEP16	3	/* MGA Millenium I: text, attr, 14 reserved bytes */
#define FB_AUX_TEXT_MGA_STEP8	4	/* other MGAs:      text, attr,  6 reserved bytes */
#define FB_AUX_TEXT_SVGA_GROUP	8	/* 8-15: SVGA tileblit compatible modes */
#define FB_AUX_TEXT_SVGA_MASK	7	/* lower three bits says step */
#define FB_AUX_TEXT_SVGA_STEP2	8	/* SVGA text mode:  text, attr */
#define FB_AUX_TEXT_SVGA_STEP4	9	/* SVGA text mode:  text, attr,  2 reserved bytes */
#define FB_AUX_TEXT_SVGA_STEP8	10	/* SVGA text mode:  text, attr,  6 reserved bytes */
#define FB_AUX_TEXT_SVGA_STEP16	11	/* SVGA text mode:  text, attr, 14 reserved bytes */
#define FB_AUX_TEXT_SVGA_LAST	15	/* reserved up to 15 */

#define FB_AUX_VGA_PLANES_VGA4		0	/* 16 color planes (EGA/VGA) */
#define FB_AUX_VGA_PLANES_CFB4		1	/* CFB4 in planes (VGA) */
#define FB_AUX_VGA_PLANES_CFB8		2	/* CFB8 in planes (VGA) */

#define FB_VISUAL_MONO01		0	/* Monochr. 1=Black 0=White */
#define FB_VISUAL_MONO10		1	/* Monochr. 1=White 0=Black */
#define FB_VISUAL_TRUECOLOR		2	/* True color	*/
#define FB_VISUAL_PSEUDOCOLOR		3	/* Pseudo color (like atari) */
#define FB_VISUAL_DIRECTCOLOR		4	/* Direct color */
#define FB_VISUAL_STATIC_PSEUDOCOLOR	5	/* Pseudo color readonly */
#define FB_VISUAL_FOURCC		6	/* Visual identified by a V4L2 FOURCC */

#define FB_ACCEL_NONE		0	/* no hardware accelerator	*/
#define FB_ACCEL_ATARIBLITT	1	/* Atari Blitter		*/
#define FB_ACCEL_AMIGABLITT	2	/* Amiga Blitter                */
#define FB_ACCEL_S3_TRIO64	3	/* Cybervision64 (S3 Trio64)    */
#define FB_ACCEL_NCR_77C32BLT	4	/* RetinaZ3 (NCR 77C32BLT)      */
#define FB_ACCEL_S3_VIRGE	5	/* Cybervision64/3D (S3 ViRGE)	*/
#define FB_ACCEL_ATI_MACH64GX	6	/* ATI Mach 64GX family		*/
#define FB_ACCEL_DEC_TGA	7	/* DEC 21030 TGA		*/
#define FB_ACCEL_ATI_MACH64CT	8	/* ATI Mach 64CT family		*/
#define FB_ACCEL_ATI_MACH64VT	9	/* ATI Mach 64CT family VT class */
#define FB_ACCEL_ATI_MACH64GT	10	/* ATI Mach 64CT family GT class */
#define FB_ACCEL_SUN_CREATOR	11	/* Sun Creator/Creator3D	*/
#define FB_ACCEL_SUN_CGSIX	12	/* Sun cg6			*/
#define FB_ACCEL_SUN_LEO	13	/* Sun leo/zx			*/
#define FB_ACCEL_IMS_TWINTURBO	14	/* IMS Twin Turbo		*/
#define FB_ACCEL_3DLABS_PERMEDIA2 15	/* 3Dlabs Permedia 2		*/
#define FB_ACCEL_MATROX_MGA2064W 16	/* Matrox MGA2064W (Millenium)	*/
#define FB_ACCEL_MATROX_MGA1064SG 17	/* Matrox MGA1064SG (Mystique)	*/
#define FB_ACCEL_MATROX_MGA2164W 18	/* Matrox MGA2164W (Millenium II) */
#define FB_ACCEL_MATROX_MGA2164W_AGP 19	/* Matrox MGA2164W (Millenium II) */
#define FB_ACCEL_MATROX_MGAG100	20	/* Matrox G100 (Productiva G100) */
#define FB_ACCEL_MATROX_MGAG200	21	/* Matrox G200 (Myst, Mill, ...) */
#define FB_ACCEL_SUN_CG14	22	/* Sun cgfourteen		 */
#define FB_ACCEL_SUN_BWTWO	23	/* Sun bwtwo			*/
#define FB_ACCEL_SUN_CGTHREE	24	/* Sun cgthree			*/
#define FB_ACCEL_SUN_TCX	25	/* Sun tcx			*/
#define FB_ACCEL_MATROX_MGAG400	26	/* Matrox G400			*/
#define FB_ACCEL_NV3		27	/* nVidia RIVA 128              */
#define FB_ACCEL_NV4		28	/* nVidia RIVA TNT		*/
#define FB_ACCEL_NV5		29	/* nVidia RIVA TNT2		*/
#define FB_ACCEL_CT_6555x	30	/* C&T 6555x			*/
#define FB_ACCEL_3DFX_BANSHEE	31	/* 3Dfx Banshee			*/
#define FB_ACCEL_ATI_RAGE128	32	/* ATI Rage128 family		*/
#define FB_ACCEL_IGS_CYBER2000	33	/* CyberPro 2000		*/
#define FB_ACCEL_IGS_CYBER2010	34	/* CyberPro 2010		*/
#define FB_ACCEL_IGS_CYBER5000	35	/* CyberPro 5000		*/
#define FB_ACCEL_SIS_GLAMOUR    36	/* SiS 300/630/540              */
#define FB_ACCEL_3DLABS_PERMEDIA3 37	/* 3Dlabs Permedia 3		*/
#define FB_ACCEL_ATI_RADEON	38	/* ATI Radeon family		*/
#define FB_ACCEL_I810           39      /* Intel 810/815                */
#define FB_ACCEL_SIS_GLAMOUR_2  40	/* SiS 315, 650, 740		*/
#define FB_ACCEL_SIS_XABRE      41	/* SiS 330 ("Xabre")		*/
#define FB_ACCEL_I830           42      /* Intel 830M/845G/85x/865G     */
#define FB_ACCEL_NV_10          43      /* nVidia Arch 10               */
#define FB_ACCEL_NV_20          44      /* nVidia Arch 20               */
#define FB_ACCEL_NV_30          45      /* nVidia Arch 30               */
#define FB_ACCEL_NV_40          46      /* nVidia Arch 40               */
#define FB_ACCEL_XGI_VOLARI_V	47	/* XGI Volari V3XT, V5, V8      */
#define FB_ACCEL_XGI_VOLARI_Z	48	/* XGI Volari Z7                */
#define FB_ACCEL_OMAP1610	49	/* TI OMAP16xx                  */
#define FB_ACCEL_TRIDENT_TGUI	50	/* Trident TGUI			*/
#define FB_ACCEL_TRIDENT_3DIMAGE 51	/* Trident 3DImage		*/
#define FB_ACCEL_TRIDENT_BLADE3D 52	/* Trident Blade3D		*/
#define FB_ACCEL_TRIDENT_BLADEXP 53	/* Trident BladeXP		*/
#define FB_ACCEL_CIRRUS_ALPINE   53	/* Cirrus Logic 543x/544x/5480	*/
#define FB_ACCEL_NEOMAGIC_NM2070 90	/* NeoMagic NM2070              */
#define FB_ACCEL_NEOMAGIC_NM2090 91	/* NeoMagic NM2090              */
#define FB_ACCEL_NEOMAGIC_NM2093 92	/* NeoMagic NM2093              */
#define FB_ACCEL_NEOMAGIC_NM2097 93	/* NeoMagic NM2097              */
#define FB_ACCEL_NEOMAGIC_NM2160 94	/* NeoMagic NM2160              */
#define FB_ACCEL_NEOMAGIC_NM2200 95	/* NeoMagic NM2200              */
#define FB_ACCEL_NEOMAGIC_NM2230 96	/* NeoMagic NM2230              */
#define FB_ACCEL_NEOMAGIC_NM2360 97	/* NeoMagic NM2360              */
#define FB_ACCEL_NEOMAGIC_NM2380 98	/* NeoMagic NM2380              */
#define FB_ACCEL_PXA3XX		 99	/* PXA3xx			*/

#define FB_ACCEL_SAVAGE4        0x80	/* S3 Savage4                   */
#define FB_ACCEL_SAVAGE3D       0x81	/* S3 Savage3D                  */
#define FB_ACCEL_SAVAGE3D_MV    0x82	/* S3 Savage3D-MV               */
#define FB_ACCEL_SAVAGE2000     0x83	/* S3 Savage2000                */
#define FB_ACCEL_SAVAGE_MX_MV   0x84	/* S3 Savage/MX-MV              */
#define FB_ACCEL_SAVAGE_MX      0x85	/* S3 Savage/MX                 */
#define FB_ACCEL_SAVAGE_IX_MV   0x86	/* S3 Savage/IX-MV              */
#define FB_ACCEL_SAVAGE_IX      0x87	/* S3 Savage/IX                 */
#define FB_ACCEL_PROSAVAGE_PM   0x88	/* S3 ProSavage PM133           */
#define FB_ACCEL_PROSAVAGE_KM   0x89	/* S3 ProSavage KM133           */
#define FB_ACCEL_S3TWISTER_P    0x8a	/* S3 Twister                   */
#define FB_ACCEL_S3TWISTER_K    0x8b	/* S3 TwisterK                  */
#define FB_ACCEL_SUPERSAVAGE    0x8c    /* S3 Supersavage               */
#define FB_ACCEL_PROSAVAGE_DDR  0x8d	/* S3 ProSavage DDR             */
#define FB_ACCEL_PROSAVAGE_DDRK 0x8e	/* S3 ProSavage DDR-K           */

#define FB_ACCEL_PUV3_UNIGFX	0xa0	/* PKUnity-v3 Unigfx		*/

#define FB_CAP_FOURCC		1	/* Device supports FOURCC-based formats */

struct fb_fix_screeninfo {
	char id[16];			/* identification string eg "TT Builtin" */
	unsigned long smem_start;	/* Start of frame buffer mem */
					/* (physical address) */
	__u32 smem_len;			/* Length of frame buffer mem */
	__u32 type;			/* see FB_TYPE_*		*/
	__u32 type_aux;			/* Interleave for interleaved Planes */
	__u32 visual;			/* see FB_VISUAL_*		*/ 
	__u16 xpanstep;			/* zero if no hardware panning  */
	__u16 ypanstep;			/* zero if no hardware panning  */
	__u16 ywrapstep;		/* zero if no hardware ywrap    */
	__u32 line_length;		/* length of a line in bytes    */
	unsigned long mmio_start;	/* Start of Memory Mapped I/O   */
					/* (physical address) */
	__u32 mmio_len;			/* Length of Memory Mapped I/O  */
	__u32 accel;			/* Indicate to driver which	*/
					/*  specific chip/card we have	*/
	__u16 capabilities;		/* see FB_CAP_*			*/
	__u16 reserved[2];		/* Reserved for future compatibility */
};

/* Interpretation of offset for color fields: All offsets are from the right,
 * inside a "pixel" value, which is exactly 'bits_per_pixel' wide (means: you
 * can use the offset as right argument to <<). A pixel afterwards is a bit
 * stream and is written to video memory as that unmodified.
 *
 * For pseudocolor: offset and length should be the same for all color
 * components. Offset specifies the position of the least significant bit
 * of the pallette index in a pixel value. Length indicates the number
 * of available palette entries (i.e. # of entries = 1 << length).
 */
struct fb_bitfield {
	__u32 offset;			/* beginning of bitfield	*/
	__u32 length;			/* length of bitfield		*/
	__u32 msb_right;		/* != 0 : Most significant bit is */ 
					/* right */ 
};

#define FB_NONSTD_HAM		1	/* Hold-And-Modify (HAM)        */
#define FB_NONSTD_REV_PIX_IN_B	2	/* order of pixels in each byte is reversed */

#define FB_ACTIVATE_NOW		0	/* set values immediately (or vbl)*/
#define FB_ACTIVATE_NXTOPEN	1	/* activate on next open	*/
#define FB_ACTIVATE_TEST	2	/* don't set, round up impossible */
#define FB_ACTIVATE_MASK       15
					/* values			*/
#define FB_ACTIVATE_VBL	       16	/* activate values on next vbl  */
#define FB_CHANGE_CMAP_VBL     32	/* change colormap on vbl	*/
#define FB_ACTIVATE_ALL	       64	/* change all VCs on this fb	*/
#define FB_ACTIVATE_FORCE     128	/* force apply even when no change*/
#define FB_ACTIVATE_INV_MODE  256       /* invalidate videomode */

#define FB_ACCELF_TEXT		1	/* (OBSOLETE) see fb_info.flags and vc_mode */

#define FB_SYNC_HOR_HIGH_ACT	1	/* horizontal sync high active	*/
#define FB_SYNC_VERT_HIGH_ACT	2	/* vertical sync high active	*/
#define FB_SYNC_EXT		4	/* external sync		*/
#define FB_SYNC_COMP_HIGH_ACT	8	/* composite sync high active   */
#define FB_SYNC_BROADCAST	16	/* broadcast video timings      */
					/* vtotal = 144d/288n/576i => PAL  */
					/* vtotal = 121d/242n/484i => NTSC */
#define FB_SYNC_ON_GREEN	32	/* sync on green */

#define FB_VMODE_NONINTERLACED  0	/* non interlaced */
#define FB_VMODE_INTERLACED	1	/* interlaced	*/
#define FB_VMODE_DOUBLE		2	/* double scan */
#define FB_VMODE_ODD_FLD_FIRST	4	/* interlaced: top line first */
#define FB_VMODE_MASK		255

#define FB_VMODE_YWRAP		256	/* ywrap instead of panning     */
#define FB_VMODE_SMOOTH_XPAN	512	/* smooth xpan possible (internally used) */
#define FB_VMODE_CONUPDATE	512	/* don't update x/yoffset	*/

/*
 * Display rotation support
 */
#define FB_ROTATE_UR      0
#define FB_ROTATE_CW      1
#define FB_ROTATE_UD      2
#define FB_ROTATE_CCW     3

#define PICOS2KHZ(a) (1000000000UL/(a))
#define KHZ2PICOS(a) (1000000000UL/(a))

struct fb_var_screeninfo {
	__u32 xres;			/* visible resolution		*/
	__u32 yres;
	__u32 xres_virtual;		/* virtual resolution		*/
	__u32 yres_virtual;
	__u32 xoffset;			/* offset from virtual to visible */
	__u32 yoffset;			/* resolution			*/

	__u32 bits_per_pixel;		/* guess what			*/
	__u32 grayscale;		/* 0 = color, 1 = grayscale,	*/
					/* >1 = FOURCC			*/
	struct fb_bitfield red;		/* bitfield in fb mem if true color, */
	struct fb_bitfield green;	/* else only length is significant */
	struct fb_bitfield blue;
	struct fb_bitfield transp;	/* transparency			*/	

	__u32 nonstd;			/* != 0 Non standard pixel format */

	__u32 activate;			/* see FB_ACTIVATE_*		*/

	__u32 height;			/* height of picture in mm    */
	__u32 width;			/* width of picture in mm     */

	__u32 accel_flags;		/* (OBSOLETE) see fb_info.flags */

	/* Timing: All values in pixclocks, except pixclock (of course) */
	__u32 pixclock;			/* pixel clock in ps (pico seconds) */
	__u32 left_margin;		/* time from sync to picture	*/
	__u32 right_margin;		/* time from picture to sync	*/
	__u32 upper_margin;		/* time from sync to picture	*/
	__u32 lower_margin;
	__u32 hsync_len;		/* length of horizontal sync	*/
	__u32 vsync_len;		/* length of vertical sync	*/
	__u32 sync;			/* see FB_SYNC_*		*/
	__u32 vmode;			/* see FB_VMODE_*		*/
	__u32 rotate;			/* angle we rotate counter clockwise */
	__u32 colorspace;		/* colorspace for FOURCC-based modes */
	__u32 reserved[4];		/* Reserved for future compatibility */
};

struct fb_cmap {
	__u32 start;			/* First entry	*/
	__u32 len;			/* Number of entries */
	__u16 *red;			/* Red values	*/
	__u16 *green;
	__u16 *blue;
	__u16 *transp;			/* transparency, can be NULL */
};

struct fb_con2fbmap {
	__u32 console;
	__u32 framebuffer;
};

/* VESA Blanking Levels */
#define VESA_NO_BLANKING        0
#define VESA_VSYNC_SUSPEND      1
#define VESA_HSYNC_SUSPEND      2
#define VESA_POWERDOWN          3


enum {
	/* screen: unblanked, hsync: on,  vsync: on */
	FB_BLANK_UNBLANK       = VESA_NO_BLANKING,

	/* screen: blanked,   hsync: on,  vsync: on */
	FB_BLANK_NORMAL        = VESA_NO_BLANKING + 1,

	/* screen: blanked,   hsync: on,  vsync: off */
	FB_BLANK_VSYNC_SUSPEND = VESA_VSYNC_SUSPEND + 1,

	/* screen: blanked,   hsync: off, vsync: on */
	FB_BLANK_HSYNC_SUSPEND = VESA_HSYNC_SUSPEND + 1,

	/* screen: blanked,   hsync: off, vsync: off */
	FB_BLANK_POWERDOWN     = VESA_POWERDOWN + 1
};

#define FB_VBLANK_VBLANKING	0x001	/* currently in a vertical blank */
#define FB_VBLANK_HBLANKING	0x002	/* currently in a horizontal blank */
#define FB_VBLANK_HAVE_VBLANK	0x004	/* vertical blanks can be detected */
#define FB_VBLANK_HAVE_HBLANK	0x008	/* horizontal blanks can be detected */
#define FB_VBLANK_HAVE_COUNT	0x010	/* global retrace counter is available */
#define FB_VBLANK_HAVE_VCOUNT	0x020	/* the vcount field is valid */
#define FB_VBLANK_HAVE_HCOUNT	0x040	/* the hcount field is valid */
#define FB_VBLANK_VSYNCING	0x080	/* currently in a vsync */
#define FB_VBLANK_HAVE_VSYNC	0x100	/* verical syncs can be detected */

struct fb_vblank {
	__u32 flags;			/* FB_VBLANK flags */
	__u32 count;			/* counter of retraces since boot */
	__u32 vcount;			/* current scanline position */
	__u32 hcount;			/* current scandot position */
	__u32 reserved[4];		/* reserved for future compatibility */
};

/* Internal HW accel */
#define ROP_COPY 0
#define ROP_XOR  1

struct fb_copyarea {
	__u32 dx;
	__u32 dy;
	__u32 width;
	__u32 height;
	__u32 sx;
	__u32 sy;
};

struct fb_fillrect {
	__u32 dx;	/* screen-relative */
	__u32 dy;
	__u32 width;
	__u32 height;
	__u32 color;
	__u32 rop;
};

struct fb_image {
	__u32 dx;		/* Where to place image */
	__u32 dy;
	__u32 width;		/* Size of image */
	__u32 height;
	__u32 fg_color;		/* Only used when a mono bitmap */
	__u32 bg_color;
	__u8  depth;		/* Depth of the image */
	const char *data;	/* Pointer to image data */
	struct fb_cmap cmap;	/* color map info */
};

/*
 * hardware cursor control
 */

#define FB_CUR_SETIMAGE 0x01
#define FB_CUR_SETPOS   0x02
#define FB_CUR_SETHOT   0x04
#define FB_CUR_SETCMAP  0x08
#define FB_CUR_SETSHAPE 0x10
#define FB_CUR_SETSIZE	0x20
#define FB_CUR_SETALL   0xFF

struct fbcurpos {
	__u16 x, y;
};

struct fb_cursor {
	__u16 set;		/* what to set */
	__u16 enable;		/* cursor on/off */
	__u16 rop;		/* bitop operation */
	const char *mask;	/* cursor mask bits */
	struct fbcurpos hot;	/* cursor hot spot */
	struct fb_image	image;	/* Cursor image */
};

/* Settings for the generic backlight code */
#define FB_BACKLIGHT_LEVELS	128
#define FB_BACKLIGHT_MAX	0xFF


#endif /* _UAPI_LINUX_FB_H */

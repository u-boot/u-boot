/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 * (C) Copyright 2002
 * Wolfgang Denk, wd@denx.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* #define DEBUG */

/************************************************************************/
/* ** HEADER FILES							*/
/************************************************************************/

#include <stdarg.h>
#include <common.h>
#include <config.h>
#include <version.h>
#include <i2c.h>
#include <linux/types.h>
#include <stdio_dev.h>

#ifdef CONFIG_VIDEO

DECLARE_GLOBAL_DATA_PTR;

/************************************************************************/
/* ** DEBUG SETTINGS							*/
/************************************************************************/

#if 0
#define VIDEO_DEBUG_COLORBARS	/* Force colorbars output */
#endif

/************************************************************************/
/* ** VIDEO MODE SETTINGS						*/
/************************************************************************/

#if 0
#define VIDEO_MODE_EXTENDED		/* Allow screen size bigger than visible area */
#define VIDEO_MODE_NTSC
#endif

#define VIDEO_MODE_PAL

#if 0
#define VIDEO_BLINK			/* This enables cursor blinking (under construction) */
#endif

#define VIDEO_INFO			/* Show U-Boot information */
#define VIDEO_INFO_X		VIDEO_LOGO_WIDTH+8
#define VIDEO_INFO_Y		16

/************************************************************************/
/* ** VIDEO ENCODER CONSTANTS						*/
/************************************************************************/

#ifdef CONFIG_VIDEO_ENCODER_AD7176

#include <video_ad7176.h>	/* Sets encoder data, mode, and visible and active area */

#define VIDEO_I2C		1
#define VIDEO_I2C_ADDR		CONFIG_VIDEO_ENCODER_AD7176_ADDR
#endif

#ifdef CONFIG_VIDEO_ENCODER_AD7177

#include <video_ad7177.h>	/* Sets encoder data, mode, and visible and active area */

#define VIDEO_I2C		1
#define VIDEO_I2C_ADDR		CONFIG_VIDEO_ENCODER_AD7177_ADDR
#endif

#ifdef CONFIG_VIDEO_ENCODER_AD7179

#include <video_ad7179.h>	/* Sets encoder data, mode, and visible and active area */

#define VIDEO_I2C		1
#define VIDEO_I2C_ADDR		CONFIG_VIDEO_ENCODER_AD7179_ADDR
#endif

/************************************************************************/
/* ** VIDEO MODE CONSTANTS						*/
/************************************************************************/

#ifdef VIDEO_MODE_EXTENDED
#define VIDEO_COLS	VIDEO_ACTIVE_COLS
#define VIDEO_ROWS	VIDEO_ACTIVE_ROWS
#else
#define VIDEO_COLS	VIDEO_VISIBLE_COLS
#define VIDEO_ROWS	VIDEO_VISIBLE_ROWS
#endif

#define VIDEO_PIXEL_SIZE	(VIDEO_MODE_BPP/8)
#define VIDEO_SIZE		(VIDEO_ROWS*VIDEO_COLS*VIDEO_PIXEL_SIZE)	/* Total size of buffer */
#define VIDEO_PIX_BLOCKS	(VIDEO_SIZE >> 2)	/* Number of ints */
#define VIDEO_LINE_LEN		(VIDEO_COLS*VIDEO_PIXEL_SIZE)	/* Number of bytes per line */
#define VIDEO_BURST_LEN		(VIDEO_COLS/8)

#ifdef VIDEO_MODE_YUYV
#define VIDEO_BG_COL	0x80D880D8	/* Background color in YUYV format */
#else
#define VIDEO_BG_COL	0xF8F8F8F8	/* Background color in RGB format */
#endif

/************************************************************************/
/* ** FONT AND LOGO DATA						*/
/************************************************************************/

#include <video_font.h>			/* Get font data, width and height */

#ifdef CONFIG_VIDEO_LOGO
#include <video_logo.h>			/* Get logo data, width and height */

#define VIDEO_LOGO_WIDTH	DEF_U_BOOT_LOGO_WIDTH
#define VIDEO_LOGO_HEIGHT	DEF_U_BOOT_LOGO_HEIGHT
#define VIDEO_LOGO_ADDR		&u_boot_logo
#endif

/************************************************************************/
/* ** VIDEO CONTROLLER CONSTANTS					*/
/************************************************************************/

/* VCCR - VIDEO CONTROLLER CONFIGURATION REGISTER */

#define VIDEO_VCCR_VON	0		/* Video controller ON */
#define VIDEO_VCCR_CSRC	1		/* Clock source */
#define VIDEO_VCCR_PDF	13		/* Pixel display format */
#define VIDEO_VCCR_IEN	11		/* Interrupt enable */

/* VSR - VIDEO STATUS REGISTER */

#define VIDEO_VSR_CAS	6		/* Active set */
#define VIDEO_VSR_EOF	0		/* End of frame */

/* VCMR - VIDEO COMMAND REGISTER */

#define VIDEO_VCMR_BD	0		/* Blank display */
#define VIDEO_VCMR_ASEL	1		/* Active set selection */

/* VBCB - VIDEO BACKGROUND COLOR BUFFER REGISTER */

#define VIDEO_BCSR4_RESET_BIT	21	/* BCSR4 - Extern video encoder reset */
#define VIDEO_BCSR4_EXTCLK_BIT	22	/* BCSR4 - Extern clock enable */
#define VIDEO_BCSR4_VIDLED_BIT	23	/* BCSR4 - Video led disable */

/************************************************************************/
/* ** CONSOLE CONSTANTS							*/
/************************************************************************/

#ifdef	CONFIG_VIDEO_LOGO
#define CONSOLE_ROWS		((VIDEO_ROWS - VIDEO_LOGO_HEIGHT) / VIDEO_FONT_HEIGHT)
#define VIDEO_LOGO_SKIP		(VIDEO_COLS - VIDEO_LOGO_WIDTH)
#else
#define CONSOLE_ROWS		(VIDEO_ROWS / VIDEO_FONT_HEIGHT)
#endif

#define CONSOLE_COLS		(VIDEO_COLS / VIDEO_FONT_WIDTH)
#define CONSOLE_ROW_SIZE	(VIDEO_FONT_HEIGHT * VIDEO_LINE_LEN)
#define CONSOLE_ROW_FIRST	(video_console_address)
#define CONSOLE_ROW_SECOND	(video_console_address + CONSOLE_ROW_SIZE)
#define CONSOLE_ROW_LAST	(video_console_address + CONSOLE_SIZE - CONSOLE_ROW_SIZE)
#define CONSOLE_SIZE		(CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_SCROLL_SIZE	(CONSOLE_SIZE - CONSOLE_ROW_SIZE)

/*
 * Simple color definitions
 */
#define CONSOLE_COLOR_BLACK	 0
#define CONSOLE_COLOR_RED	 1
#define CONSOLE_COLOR_GREEN	 2
#define CONSOLE_COLOR_YELLOW	 3
#define CONSOLE_COLOR_BLUE	 4
#define CONSOLE_COLOR_MAGENTA	 5
#define CONSOLE_COLOR_CYAN	 6
#define CONSOLE_COLOR_GREY	13
#define CONSOLE_COLOR_GREY2	14
#define CONSOLE_COLOR_WHITE	15	/* Must remain last / highest */

/************************************************************************/
/* ** BITOPS MACROS							*/
/************************************************************************/

#define HISHORT(i)	((i >> 16)&0xffff)
#define LOSHORT(i)	(i & 0xffff)
#define HICHAR(s)	((i >> 8)&0xff)
#define LOCHAR(s)	(i & 0xff)
#define HI(c)		((c >> 4)&0xf)
#define LO(c)		(c & 0xf)
#define SWAPINT(i)	(HISHORT(i) | (LOSHORT(i) << 16))
#define SWAPSHORT(s)	(HICHAR(s) | (LOCHAR(s) << 8))
#define SWAPCHAR(c)	(HI(c) | (LO(c) << 4))
#define BITMASK(b)	(1 << (b))
#define GETBIT(v,b)	(((v) & BITMASK(b)) > 0)
#define SETBIT(v,b,d)	(v = (((d)>0) ? (v) | BITMASK(b): (v) & ~BITMASK(b)))

/************************************************************************/
/* ** STRUCTURES							*/
/************************************************************************/

typedef struct {
	unsigned char V, Y1, U, Y2;
} tYUYV;

/* This structure is based on the Video Ram in the MPC823. */
typedef struct VRAM {
	unsigned	hx:2,		/* Horizontal sync */
			vx:2,		/* Vertical sync */
			fx:2,		/* Frame */
			bx:2,		/* Blank */
			res1:6,		/* Reserved */
			vds:2,		/* Video Data Select */
			inter:1,	/* Interrupt */
			res2:2,		/* Reserved */
			lcyc:11,	/* Loop/video cycles */
			lp:1,		/* Loop start/end */
			lst:1;		/* Last entry */
} VRAM;

/************************************************************************/
/* ** VARIABLES								*/
/************************************************************************/

static int
	video_panning_range_x = 0,	/* Video mode invisible pixels x range */
	video_panning_range_y = 0,	/* Video mode invisible pixels y range */
	video_panning_value_x = 0,	/* Video mode x panning value (absolute) */
	video_panning_value_y = 0,	/* Video mode y panning value (absolute) */
	video_panning_factor_x = 0,	/* Video mode x panning value (-127 +127) */
	video_panning_factor_y = 0,	/* Video mode y panning value (-127 +127) */
	console_col = 0,		/* Cursor col */
	console_row = 0,		/* Cursor row */
	video_palette[16];		/* Our palette */

static const int video_font_draw_table[] =
	{ 0x00000000, 0x0000ffff, 0xffff0000, 0xffffffff };

static char
	video_color_fg = 0,		/* Current fg color index (0-15) */
	video_color_bg = 0,		/* Current bg color index (0-15) */
	video_enable = 0;		/* Video has been initialized? */

static void
	*video_fb_address,		/* Frame buffer address */
	*video_console_address;		/* Console frame buffer start address */

/************************************************************************/
/* ** MEMORY FUNCTIONS (32bit)						*/
/************************************************************************/

static void memsetl (int *p, int c, int v)
{
	while (c--)
		*(p++) = v;
}

static void memcpyl (int *d, int *s, int c)
{
	while (c--)
		*(d++) = *(s++);
}

/************************************************************************/
/* ** VIDEO DRAWING AND COLOR FUNCTIONS					*/
/************************************************************************/

static int video_maprgb (int r, int g, int b)
{
#ifdef VIDEO_MODE_YUYV
	unsigned int pR, pG, pB;
	tYUYV YUYV;
	unsigned int *ret = (unsigned int *) &YUYV;

	/* Transform (0-255) components to (0-100) */

	pR = r * 100 / 255;
	pG = g * 100 / 255;
	pB = b * 100 / 255;

	/* Calculate YUV values (0-255) from RGB beetween 0-100 */

	YUYV.Y1 = YUYV.Y2 = 209 * (pR + pG + pB) / 300 + 16;
	YUYV.U	= pR - (pG * 3 / 4) - (pB / 4) + 128;
	YUYV.V	= pB - (pR / 4) - (pG * 3 / 4) + 128;
	return *ret;
#endif
#ifdef VIDEO_MODE_RGB
	return ((r >> 3) << 11) | ((g > 2) << 6) | (b >> 3);
#endif
}

static void video_setpalette (int color, int r, int g, int b)
{
	color &= 0xf;

	video_palette[color] = video_maprgb (r, g, b);

	/* Swap values if our panning offset is odd */
	if (video_panning_value_x & 1)
		video_palette[color] = SWAPINT (video_palette[color]);
}

static void video_fill (int color)
{
	memsetl (video_fb_address, VIDEO_PIX_BLOCKS, color);
}

static void video_setfgcolor (int i)
{
	video_color_fg = i & 0xf;
}

static void video_setbgcolor (int i)
{
	video_color_bg = i & 0xf;
}

static int video_pickcolor (int i)
{
	return video_palette[i & 0xf];
}

/* Absolute console plotting functions */

#ifdef VIDEO_BLINK
static void video_revchar (int xx, int yy)
{
	int rows;
	u8 *dest;

	dest = video_fb_address + yy * VIDEO_LINE_LEN + xx * 2;

	for (rows = VIDEO_FONT_HEIGHT; rows--; dest += VIDEO_LINE_LEN) {
		switch (VIDEO_FONT_WIDTH) {
		case 16:
			((u32 *) dest)[6] ^= 0xffffffff;
			((u32 *) dest)[7] ^= 0xffffffff;
			/* FALL THROUGH */
		case 12:
			((u32 *) dest)[4] ^= 0xffffffff;
			((u32 *) dest)[5] ^= 0xffffffff;
			/* FALL THROUGH */
		case 8:
			((u32 *) dest)[2] ^= 0xffffffff;
			((u32 *) dest)[3] ^= 0xffffffff;
			/* FALL THROUGH */
		case 4:
			((u32 *) dest)[0] ^= 0xffffffff;
			((u32 *) dest)[1] ^= 0xffffffff;
		}
	}
}
#endif

static void video_drawchars (int xx, int yy, unsigned char *s, int count)
{
	u8 *cdat, *dest, *dest0;
	int rows, offset, c;
	u32 eorx, fgx, bgx;

	offset = yy * VIDEO_LINE_LEN + xx * 2;
	dest0 = video_fb_address + offset;

	fgx = video_pickcolor (video_color_fg);
	bgx = video_pickcolor (video_color_bg);

	if (xx & 1) {
		fgx = SWAPINT (fgx);
		bgx = SWAPINT (bgx);
	}

	eorx = fgx ^ bgx;

	switch (VIDEO_FONT_WIDTH) {
	case 4:
	case 8:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--;
			     dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] =
					(video_font_draw_table[bits >> 6] & eorx) ^ bgx;
				((u32 *) dest)[1] =
					(video_font_draw_table[bits >> 4 & 3] & eorx) ^ bgx;
				if (VIDEO_FONT_WIDTH == 8) {
					((u32 *) dest)[2] =
						(video_font_draw_table[bits >> 2 & 3] & eorx) ^ bgx;
					((u32 *) dest)[3] =
						(video_font_draw_table[bits & 3] & eorx) ^ bgx;
				}
			}
			dest0 += VIDEO_FONT_WIDTH * 2;
			s++;
		}
		break;
	case 12:
	case 16:
		while (count--) {
			cdat = video_fontdata + (*s) * (VIDEO_FONT_HEIGHT << 1);
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0; rows--;
				 dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] =
					(video_font_draw_table[bits >> 6] & eorx) ^ bgx;
				((u32 *) dest)[1] =
					(video_font_draw_table[bits >> 4 & 3] & eorx) ^ bgx;
				((u32 *) dest)[2] =
					(video_font_draw_table[bits >> 2 & 3] & eorx) ^ bgx;
				((u32 *) dest)[3] =
					(video_font_draw_table[bits & 3] & eorx) ^ bgx;
				bits = *cdat++;
				((u32 *) dest)[4] =
					(video_font_draw_table[bits >> 6] & eorx) ^ bgx;
				((u32 *) dest)[5] =
					(video_font_draw_table[bits >> 4 & 3] & eorx) ^ bgx;
				if (VIDEO_FONT_WIDTH == 16) {
					((u32 *) dest)[6] =
						(video_font_draw_table[bits >> 2 & 3] & eorx) ^ bgx;
					((u32 *) dest)[7] =
						(video_font_draw_table[bits & 3] & eorx) ^ bgx;
				}
			}
			s++;
			dest0 += VIDEO_FONT_WIDTH * 2;
		}
		break;
	}
}

static inline void video_drawstring (int xx, int yy, char *s)
{
	video_drawchars (xx, yy, (unsigned char *)s, strlen (s));
}

/* Relative to console plotting functions */

static void video_putchars (int xx, int yy, unsigned char *s, int count)
{
#ifdef CONFIG_VIDEO_LOGO
	video_drawchars (xx, yy + VIDEO_LOGO_HEIGHT, s, count);
#else
	video_drawchars (xx, yy, s, count);
#endif
}

static void video_putchar (int xx, int yy, unsigned char c)
{
#ifdef CONFIG_VIDEO_LOGO
	video_drawchars (xx, yy + VIDEO_LOGO_HEIGHT, &c, 1);
#else
	video_drawchars (xx, yy, &c, 1);
#endif
}

static inline void video_putstring (int xx, int yy, unsigned char *s)
{
	video_putchars (xx, yy, (unsigned char *)s, strlen ((char *)s));
}

/************************************************************************/
/* ** VIDEO CONTROLLER LOW-LEVEL FUNCTIONS				*/
/************************************************************************/

#if !defined(CONFIG_RRVISION)
static void video_mode_dupefield (VRAM * source, VRAM * dest, int entries)
{
	int i;

	for (i = 0; i < entries; i++) {
		dest[i] = source[i];	/* Copy the entire record */
		dest[i].fx = (!dest[i].fx) * 3;	/* Negate field bit */
	}

	dest[0].lcyc++;			/* Add a cycle to the first entry */
	dest[entries - 1].lst = 1;	/* Set end of ram entries */
}
#endif

static void inline video_mode_addentry (VRAM * vr,
	int Hx, int Vx, int Fx, int Bx,
	int VDS, int INT, int LCYC, int LP, int LST)
{
	vr->hx = Hx;
	vr->vx = Vx;
	vr->fx = Fx;
	vr->bx = Bx;
	vr->vds = VDS;
	vr->inter = INT;
	vr->lcyc = LCYC;
	vr->lp = LP;
	vr->lst = LST;
}

#define ADDENTRY(a,b,c,d,e,f,g,h,i)	video_mode_addentry(&vr[entry++],a,b,c,d,e,f,g,h,i)

static int video_mode_generate (void)
{
	immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	VRAM *vr = (VRAM *) (((void *) immap) + 0xb00);	/* Pointer to the VRAM table */
	int DX, X1, X2, DY, Y1, Y2, entry = 0, fifo;

	/* CHECKING PARAMETERS */

	if (video_panning_factor_y < -128)
		video_panning_factor_y = -128;

	if (video_panning_factor_y > 128)
		video_panning_factor_y = 128;

	if (video_panning_factor_x < -128)
		video_panning_factor_x = -128;

	if (video_panning_factor_x > 128)
		video_panning_factor_x = 128;

	/* Setting panning */

	DX = video_panning_range_x = (VIDEO_ACTIVE_COLS - VIDEO_COLS) * 2;
	DY = video_panning_range_y = (VIDEO_ACTIVE_ROWS - VIDEO_ROWS) / 2;

	video_panning_value_x = (video_panning_factor_x + 128) * DX / 256;
	video_panning_value_y = (video_panning_factor_y + 128) * DY / 256;

	/* We assume these are burst units (multiplied by 2, we need it pari) */
	X1 = video_panning_value_x & 0xfffe;
	X2 = DX - X1;

	/* We assume these are field line units (divided by 2, we need it pari) */
	Y1 = video_panning_value_y & 0xfffe;
	Y2 = DY - Y1;

	debug("X1=%d, X2=%d, Y1=%d, Y2=%d, DX=%d, DY=%d VIDEO_COLS=%d \n",
	      X1, X2, Y1, Y2, DX, DY, VIDEO_COLS);

#ifdef VIDEO_MODE_NTSC
/*
 *	     Hx Vx Fx Bx VDS INT LCYC LP LST
 *
 * Retrace blanking
 */
	ADDENTRY (0, 0, 3, 0, 1, 0, 3, 1, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 243, 0, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 1440, 0, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 32, 1, 0);
/*
 * Vertical blanking
 */
	ADDENTRY (0, 0, 0, 0, 1, 0, 18, 1, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 243, 0, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 1440, 0, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 32, 1, 0);
/*
 * Odd field active area (TOP)
 */
	if (Y1 > 0) {
		ADDENTRY (0, 0, 0, 0, 1, 0, Y1, 1, 0);
		ADDENTRY (3, 0, 0, 0, 1, 0, 235, 0, 0);
		ADDENTRY (3, 0, 0, 3, 1, 0, 1448, 0, 0);
		ADDENTRY (3, 0, 0, 0, 1, 0, 32, 1, 0);
	}
/*
 * Odd field active area
 */
	ADDENTRY (0, 0, 0, 0, 1, 0, 240 - DY, 1, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 235, 0, 0);
	ADDENTRY (3, 0, 0, 3, 1, 0, 8 + X1, 0, 0);
	ADDENTRY (3, 0, 0, 3, 0, 0, VIDEO_COLS * 2, 0, 0);

	if (X2 > 0)
		ADDENTRY (3, 0, 0, 3, 1, 0, X2, 0, 0);

	ADDENTRY (3, 0, 0, 0, 1, 0, 32, 1, 0);

/*
 * Odd field active area (BOTTOM)
 */
	if (Y1 > 0) {
		ADDENTRY (0, 0, 0, 0, 1, 0, Y2, 1, 0);
		ADDENTRY (3, 0, 0, 0, 1, 0, 235, 0, 0);
		ADDENTRY (3, 0, 0, 3, 1, 0, 1448, 0, 0);
		ADDENTRY (3, 0, 0, 0, 1, 0, 32, 1, 0);
	}
/*
 * Vertical blanking
 */
	ADDENTRY (0, 0, 0, 0, 1, 0, 4, 1, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 243, 0, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 1440, 0, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 32, 1, 0);
/*
 * Vertical blanking
 */
	ADDENTRY (0, 0, 3, 0, 1, 0, 19, 1, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 243, 0, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 1440, 0, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 32, 1, 0);
/*
 * Even field active area (TOP)
 */
	if (Y1 > 0) {
		ADDENTRY (0, 0, 3, 0, 1, 0, Y1, 1, 0);
		ADDENTRY (3, 0, 3, 0, 1, 0, 235, 0, 0);
		ADDENTRY (3, 0, 3, 3, 1, 0, 1448, 0, 0);
		ADDENTRY (3, 0, 3, 0, 1, 0, 32, 1, 0);
	}
/*
 * Even field active area (CENTER)
 */
	ADDENTRY (0, 0, 3, 0, 1, 0, 240 - DY, 1, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 235, 0, 0);
	ADDENTRY (3, 0, 3, 3, 1, 0, 8 + X1, 0, 0);
	ADDENTRY (3, 0, 3, 3, 0, 0, VIDEO_COLS * 2, 0, 0);

	if (X2 > 0)
		ADDENTRY (3, 0, 3, 3, 1, 0, X2, 0, 0);

	ADDENTRY (3, 0, 3, 0, 1, 0, 32, 1, 0);
/*
 * Even field active area (BOTTOM)
 */
	if (Y1 > 0) {
		ADDENTRY (0, 0, 3, 0, 1, 0, Y2, 1, 0);
		ADDENTRY (3, 0, 3, 0, 1, 0, 235, 0, 0);
		ADDENTRY (3, 0, 3, 3, 1, 0, 1448, 0, 0);
		ADDENTRY (3, 0, 3, 0, 1, 0, 32, 1, 0);
	}
/*
 * Vertical blanking
 */
	ADDENTRY (0, 0, 3, 0, 1, 0, 1, 1, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 243, 0, 0);
	ADDENTRY (3, 0, 3, 0, 1, 0, 1440, 0, 0);
	ADDENTRY (3, 0, 3, 0, 1, 1, 32, 1, 1);
#endif

#ifdef VIDEO_MODE_PAL

#if defined(CONFIG_RRVISION)

#define HPW   160  /* horizontal pulse width (was 139)	*/
#define VPW	2  /* vertical pulse width		*/
#define HBP   104  /* horizontal back porch (was 112)	*/
#define VBP    19  /* vertical back porch (was 19)	*/
#define VID_R 240  /* number of rows			*/

	debug ("[VIDEO CTRL] Starting to add controller entries...");
/*
 * Even field
 */
	ADDENTRY (0, 3, 0, 3, 1, 0, 2, 0, 0);
	ADDENTRY (0, 0, 0, 3, 1, 0, HPW, 0, 0);
	ADDENTRY (3, 0, 0, 3, 1, 0, HBP + (VIDEO_COLS * 2) + 72, 0, 0);

	ADDENTRY (0, 0, 0, 3, 1, 0, VPW, 1, 0);
	ADDENTRY (0, 0, 0, 3, 1, 0, HPW-1, 0, 0);
	ADDENTRY (3, 0, 0, 3, 1, 0, HBP + (VIDEO_COLS * 2) + 72, 1, 0);

	ADDENTRY (0, 3, 0, 3, 1, 0, VBP, 1, 0);
	ADDENTRY (0, 3, 0, 3, 1, 0, HPW-1, 0, 0);
	ADDENTRY (3, 3, 0, 3, 1, 0, HBP + (VIDEO_COLS * 2) + 72, 1, 0);
/*
 * Active area
 */
	ADDENTRY (0, 3, 0, 3, 1, 0, VID_R , 1, 0);
	ADDENTRY (0, 3, 0, 3, 1, 0, HPW-1, 0, 0);
	ADDENTRY (3, 3, 0, 3, 1, 0, HBP, 0, 0);
	ADDENTRY (3, 3, 0, 3, 0, 0, VIDEO_COLS*2, 0, 0);
	ADDENTRY (3, 3, 0, 3, 1, 0, 72, 1, 1);

	ADDENTRY (0, 3, 0, 3, 1, 0, 51, 1, 0);
	ADDENTRY (0, 3, 0, 3, 1, 0, HPW-1, 0, 0);
	ADDENTRY (3, 3, 0, 3, 1, 0, HBP +(VIDEO_COLS * 2) + 72 , 1, 0);
/*
 * Odd field
 */
	ADDENTRY (0, 3, 0, 3, 1, 0, 2, 0, 0);
	ADDENTRY (0, 0, 0, 3, 1, 0, HPW, 0, 0);
	ADDENTRY (3, 0, 0, 3, 1, 0, HBP + (VIDEO_COLS * 2) + 72, 0, 0);

	ADDENTRY (0, 0, 0, 3, 1, 0, VPW+1, 1, 0);
	ADDENTRY (0, 0, 0, 3, 1, 0, HPW-1, 0, 0);
	ADDENTRY (3, 0, 0, 3, 1, 0, HBP + (VIDEO_COLS * 2) + 72, 1, 0);

	ADDENTRY (0, 3, 0, 3, 1, 0, VBP, 1, 0);
	ADDENTRY (0, 3, 0, 3, 1, 0, HPW-1, 0, 0);
	ADDENTRY (3, 3, 0, 3, 1, 0, HBP + (VIDEO_COLS * 2) + 72, 1, 0);
/*
 * Active area
 */
	ADDENTRY (0, 3, 0, 3, 1, 0, VID_R , 1, 0);
	ADDENTRY (0, 3, 0, 3, 1, 0, HPW-1, 0, 0);
	ADDENTRY (3, 3, 0, 3, 1, 0, HBP, 0, 0);
	ADDENTRY (3, 3, 0, 3, 0, 0, VIDEO_COLS*2, 0, 0);
	ADDENTRY (3, 3, 0, 3, 1, 0, 72, 1, 1);

	ADDENTRY (0, 3, 0, 3, 1, 0, 51, 1, 0);
	ADDENTRY (0, 3, 0, 3, 1, 0, HPW-1, 0, 0);
	ADDENTRY (3, 3, 0, 3, 1, 0, HBP +(VIDEO_COLS * 2) + 72 , 1, 0);

	debug ("done\n");

#else  /* !CONFIG_RRVISION */

/*
 *	Hx Vx Fx Bx VDS INT LCYC LP LST
 *
 * vertical; blanking
 */
	ADDENTRY (0, 0, 0, 0, 1, 0, 22, 1, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 263, 0, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 1440, 0, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 24, 1, 0);
/*
 * active area (TOP)
 */
	if (Y1 > 0) {
		ADDENTRY (0, 0, 0, 0, 1, 0, Y1, 1, 0);	/* 11? */
		ADDENTRY (3, 0, 0, 0, 1, 0, 255, 0, 0);
		ADDENTRY (3, 0, 0, 3, 1, 0, 1448, 0, 0);
		ADDENTRY (3, 0, 0, 0, 1, 0, 24, 1, 0);
	}
/*
 * field active area (CENTER)
 */
	ADDENTRY (0, 0, 0, 0, 1, 0, 288 - DY, 1, 0);	/* 265? */
	ADDENTRY (3, 0, 0, 0, 1, 0, 255, 0, 0);
	ADDENTRY (3, 0, 0, 3, 1, 0, 8 + X1, 0, 0);
	ADDENTRY (3, 0, 0, 3, 0, 0, VIDEO_COLS * 2, 0, 0);

	if (X2 > 0)
		ADDENTRY (3, 0, 0, 1, 1, 0, X2, 0, 0);

	ADDENTRY (3, 0, 0, 0, 1, 0, 24, 1, 0);
/*
 * field active area (BOTTOM)
 */
	if (Y2 > 0) {
		ADDENTRY (0, 0, 0, 0, 1, 0, Y2, 1, 0);	/* 12? */
		ADDENTRY (3, 0, 0, 0, 1, 0, 255, 0, 0);
		ADDENTRY (3, 0, 0, 3, 1, 0, 1448, 0, 0);
		ADDENTRY (3, 0, 0, 0, 1, 0, 24, 1, 0);
	}
/*
 * field vertical; blanking
 */
	ADDENTRY (0, 0, 0, 0, 1, 0, 2, 1, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 263, 0, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 1440, 0, 0);
	ADDENTRY (3, 0, 0, 0, 1, 0, 24, 1, 0);
/*
 * Create the other field (like this, but whit other field selected,
 * one more cycle loop and a last identifier)
 */
	video_mode_dupefield (vr, &vr[entry], entry);
#endif /* CONFIG_RRVISION */

#endif /* VIDEO_MODE_PAL */

	/* See what FIFO are we using */
	fifo = GETBIT (immap->im_vid.vid_vsr, VIDEO_VSR_CAS);

	/* Set number of lines and burst (only one frame for now) */
	if (fifo) {
		immap->im_vid.vid_vfcr0 = VIDEO_BURST_LEN |
			(VIDEO_BURST_LEN << 8) | ((VIDEO_ROWS / 2) << 19);
	} else {
		immap->im_vid.vid_vfcr1 = VIDEO_BURST_LEN |
			(VIDEO_BURST_LEN << 8) | ((VIDEO_ROWS / 2) << 19);
	}

	SETBIT (immap->im_vid.vid_vcmr, VIDEO_VCMR_ASEL, !fifo);

/*
 * Wait until changes are applied (not done)
 * while (GETBIT(immap->im_vid.vid_vsr, VIDEO_VSR_CAS) == fifo) ;
 */

	/* Return number of VRAM entries */
	return entry * 2;
}

static void video_encoder_init (void)
{
#ifdef VIDEO_I2C
	int rc;

	/* Initialize the I2C */
	debug ("[VIDEO ENCODER] Initializing I2C bus...\n");
#ifdef CONFIG_SYS_I2C
	i2c_init_all();
#else
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

	/* Send configuration */
#ifdef DEBUG
	{
		int i;

		puts ("[VIDEO ENCODER] Configuring the encoder...\n");

		printf ("Sending %zu bytes (@ %08lX) to I2C 0x%lX:\n   ",
			sizeof(video_encoder_data),
			(ulong)video_encoder_data,
			(ulong)VIDEO_I2C_ADDR);
		for (i=0; i<sizeof(video_encoder_data); ++i) {
			printf(" %02X", video_encoder_data[i]);
		}
		putc ('\n');
	}
#endif	/* DEBUG */

	if ((rc = i2c_write (VIDEO_I2C_ADDR, 0, 1,
			 video_encoder_data,
			 sizeof(video_encoder_data))) != 0) {
		printf ("i2c_send error: rc=%d\n", rc);
		return;
	}
#endif	/* VIDEO_I2C */
	return;
}

static void video_ctrl_init (void *memptr)
{
	immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	video_fb_address = memptr;

	/* Set background */
	debug ("[VIDEO CTRL] Setting background color...\n");
	immap->im_vid.vid_vbcb = VIDEO_BG_COL;

	/* Show the background */
	debug ("[VIDEO CTRL] Forcing background...\n");
	SETBIT (immap->im_vid.vid_vcmr, VIDEO_VCMR_BD, 1);

	/* Turn off video controller */
	debug ("[VIDEO CTRL] Turning off video controller...\n");
	SETBIT (immap->im_vid.vid_vccr, VIDEO_VCCR_VON, 0);

	/* Generate and make active a new video mode */
	debug ("[VIDEO CTRL] Generating video mode...\n");
	video_mode_generate ();

	/* Start of frame buffer (even and odd frame, to make it working with */
	/* any selected active set) */
	debug ("[VIDEO CTRL] Setting frame buffer address...\n");
	immap->im_vid.vid_vfaa1 =
		immap->im_vid.vid_vfaa0 = (u32) video_fb_address;
	immap->im_vid.vid_vfba1 =
	immap->im_vid.vid_vfba0 =
		(u32) video_fb_address + VIDEO_LINE_LEN;

	/* YUV, Big endian, SHIFT/CLK/CLK input (BEFORE ENABLING 27MHZ EXT CLOCK) */
	debug ("[VIDEO CTRL] Setting pixel mode and clocks...\n");
	immap->im_vid.vid_vccr = 0x2042;

	/* Configure port pins */
	debug ("[VIDEO CTRL] Configuring input/output pins...\n");
	immap->im_ioport.iop_pdpar = 0x1fff;
	immap->im_ioport.iop_pddir = 0x0000;

#ifdef CONFIG_RRVISION
	debug ("PC5->Output(1): enable PAL clock");
	immap->im_ioport.iop_pcpar &= ~(0x0400);
	immap->im_ioport.iop_pcdir |=   0x0400 ;
	immap->im_ioport.iop_pcdat |=   0x0400 ;
	debug ("PDPAR=0x%04X PDDIR=0x%04X PDDAT=0x%04X\n",
	       immap->im_ioport.iop_pdpar,
	       immap->im_ioport.iop_pddir,
	       immap->im_ioport.iop_pddat);
	debug ("PCPAR=0x%04X PCDIR=0x%04X PCDAT=0x%04X\n",
	       immap->im_ioport.iop_pcpar,
	       immap->im_ioport.iop_pcdir,
	       immap->im_ioport.iop_pcdat);
#endif	/* CONFIG_RRVISION */

	/* Blanking the screen. */
	debug ("[VIDEO CTRL] Blanking the screen...\n");
	video_fill (VIDEO_BG_COL);

	/*
	 * Turns on Aggressive Mode. Normally, turning on the caches
	 * will cause the screen to flicker when the caches try to
	 * fill. This gives the FIFO's for the Video Controller
	 * higher priority and prevents flickering because of
	 * underrun. This may still be an issue when using FLASH,
	 * since accessing data from Flash is so slow.
	 */
	debug ("[VIDEO CTRL] Turning on aggressive mode...\n");
	immap->im_siu_conf.sc_sdcr = 0x40;

	/* Turn on video controller */
	debug ("[VIDEO CTRL] Turning on video controller...\n");
	SETBIT (immap->im_vid.vid_vccr, VIDEO_VCCR_VON, 1);

	/* Show the display */
	debug ("[VIDEO CTRL] Enabling the video...\n");
	SETBIT (immap->im_vid.vid_vcmr, VIDEO_VCMR_BD, 0);
}

/************************************************************************/
/* ** CONSOLE FUNCTIONS							*/
/************************************************************************/

static void console_scrollup (void)
{
	/* Copy up rows ignoring the first one */
	memcpyl (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND, CONSOLE_SCROLL_SIZE >> 2);

	/* Clear the last one */
	memsetl (CONSOLE_ROW_LAST, CONSOLE_ROW_SIZE >> 2, VIDEO_BG_COL);
}

static inline void console_back (void)
{
	console_col--;

	if (console_col < 0) {
		console_col = CONSOLE_COLS - 1;
		console_row--;
		if (console_row < 0)
			console_row = 0;
	}

	video_putchar ( console_col * VIDEO_FONT_WIDTH,
			console_row * VIDEO_FONT_HEIGHT, ' ');
}

static inline void console_newline (void)
{
	console_row++;
	console_col = 0;

	/* Check if we need to scroll the terminal */
	if (console_row >= CONSOLE_ROWS) {
		/* Scroll everything up */
		console_scrollup ();

		/* Decrement row number */
		console_row--;
	}
}

void video_putc(struct stdio_dev *dev, const char c)
{
	if (!video_enable) {
		serial_putc (c);
		return;
	}

	switch (c) {
	case 13:			/* Simply ignore this */
		break;

	case '\n':			/* Next line, please */
		console_newline ();
		break;

	case 9:				/* Tab (8 chars alignment) */
		console_col |= 0x0008;	/* Next 8 chars boundary */
		console_col &= ~0x0007;	/* Set this bit to zero */

		if (console_col >= CONSOLE_COLS)
			console_newline ();
		break;

	case 8:				/* Eat last character */
		console_back ();
		break;

	default:			/* Add to the console */
		video_putchar ( console_col * VIDEO_FONT_WIDTH,
				console_row * VIDEO_FONT_HEIGHT, c);
		console_col++;
		/* Check if we need to go to next row */
		if (console_col >= CONSOLE_COLS)
			console_newline ();
	}
}

void video_puts(struct stdio_dev *dev, const char *s)
{
	int count = strlen (s);

	if (!video_enable)
		while (count--)
			serial_putc (*s++);
	else
		while (count--)
			video_putc(dev, *s++);
}

/************************************************************************/
/* ** CURSOR BLINKING FUNCTIONS						*/
/************************************************************************/

#ifdef VIDEO_BLINK

#define BLINK_TIMER_ID		0
#define BLINK_TIMER_HZ		2

static unsigned char blink_enabled = 0;
static timer_t blink_timer;

static void blink_update (void)
{
	static int blink_row = -1, blink_col = -1, blink_old = 0;

	/* Check if we have a new position to invert */
	if ((console_row != blink_row) || (console_col != blink_col)) {
		/* Check if we need to reverse last character */
		if (blink_old)
			video_revchar ( blink_col * VIDEO_FONT_WIDTH,
					(blink_row
#ifdef CONFIG_VIDEO_LOGO
					 + VIDEO_LOGO_HEIGHT
#endif
					) * VIDEO_FONT_HEIGHT);

		/* Update values */
		blink_row = console_row;
		blink_col = console_col;
		blink_old = 0;
	}

/* Reverse this character */
	blink_old = !blink_old;
	video_revchar ( console_col * VIDEO_FONT_WIDTH,
			(console_row
#ifdef CONFIG_VIDEO_LOGO
			+ VIDEO_LOGO_HEIGHT
#endif
			) * VIDEO_FONT_HEIGHT);

}

/*
 * Handler for blinking cursor
 */
static void blink_handler (void *arg)
{
/* Blink */
	blink_update ();
/* Ack the timer */
	timer_ack (&blink_timer);
}

int blink_set (int blink)
{
	int ret = blink_enabled;

	if (blink)
		timer_enable (&blink_timer);
	else
		timer_disable (&blink_timer);

	blink_enabled = blink;

	return ret;
}

static inline void blink_close (void)
{
	timer_close (&blink_timer);
}

static inline void blink_init (void)
{
	timer_init (&blink_timer,
			BLINK_TIMER_ID, BLINK_TIMER_HZ,
			blink_handler);
}
#endif

/************************************************************************/
/* ** LOGO PLOTTING FUNCTIONS						*/
/************************************************************************/

#ifdef CONFIG_VIDEO_LOGO
void easylogo_plot (fastimage_t * image, void *screen, int width, int x,
					int y)
{
	int skip = width - image->width, xcount, ycount = image->height;

#ifdef VIDEO_MODE_YUYV
	ushort *source = (ushort *) image->data;
	ushort *dest   = (ushort *) screen + y * width + x;

	while (ycount--) {
		xcount = image->width;
		while (xcount--)
			*dest++ = *source++;
		dest += skip;
	}
#endif
#ifdef VIDEO_MODE_RGB
	unsigned char
	*source = (unsigned short *) image->data,
			*dest = (unsigned short *) screen + ((y * width) + x) * 3;

	while (ycount--) {
		xcount = image->width * 3;
		memcpy (dest, source, xcount);
		source += xcount;
		dest += ycount;
	}
#endif
}

static void *video_logo (void)
{
	u16 *screen = video_fb_address, width = VIDEO_COLS;
#ifdef VIDEO_INFO
	char temp[32];
	char info[80];
#endif /* VIDEO_INFO */

	easylogo_plot (VIDEO_LOGO_ADDR, screen, width, 0, 0);

#ifdef VIDEO_INFO
	sprintf (info, "%s (%s - %s) ",
		 U_BOOT_VERSION, U_BOOT_DATE, U_BOOT_TIME);
	video_drawstring (VIDEO_INFO_X, VIDEO_INFO_Y, info);

	sprintf (info, "(C) 2002 DENX Software Engineering");
	video_drawstring (VIDEO_INFO_X, VIDEO_INFO_Y + VIDEO_FONT_HEIGHT,
					info);

	sprintf (info, "    Wolfgang DENK, wd@denx.de");
	video_drawstring (VIDEO_INFO_X, VIDEO_INFO_Y + VIDEO_FONT_HEIGHT * 2,
					info);

	/* leave one blank line */

	sprintf(info, "MPC823 CPU at %s MHz, %ld MiB RAM, %ld MiB Flash",
		strmhz(temp, gd->cpu_clk),
		gd->ram_size >> 20,
		gd->bd->bi_flashsize >> 20 );
	video_drawstring (VIDEO_INFO_X, VIDEO_INFO_Y + VIDEO_FONT_HEIGHT * 4,
					info);
#endif

	return video_fb_address + VIDEO_LOGO_HEIGHT * VIDEO_LINE_LEN;
}
#endif

/************************************************************************/
/* ** VIDEO HIGH-LEVEL FUNCTIONS					*/
/************************************************************************/

static int video_init (void *videobase)
{
	/* Initialize the encoder */
	debug ("[VIDEO] Initializing video encoder...\n");
	video_encoder_init ();

	/* Initialize the video controller */
	debug ("[VIDEO] Initializing video controller at %08x...\n",
		   (int) videobase);
	video_ctrl_init (videobase);

	/* Setting the palette */
	video_setpalette  (CONSOLE_COLOR_BLACK,	     0,	   0,	 0);
	video_setpalette  (CONSOLE_COLOR_RED,	  0xFF,	   0,	 0);
	video_setpalette  (CONSOLE_COLOR_GREEN,	     0, 0xFF,	 0);
	video_setpalette  (CONSOLE_COLOR_YELLOW,  0xFF, 0xFF,	 0);
	video_setpalette  (CONSOLE_COLOR_BLUE,	     0,	   0, 0xFF);
	video_setpalette  (CONSOLE_COLOR_MAGENTA, 0xFF,	   0, 0xFF);
	video_setpalette  (CONSOLE_COLOR_CYAN,	     0, 0xFF, 0xFF);
	video_setpalette  (CONSOLE_COLOR_GREY,	  0xAA, 0xAA, 0xAA);
	video_setpalette  (CONSOLE_COLOR_GREY2,	  0xF8, 0xF8, 0xF8);
	video_setpalette  (CONSOLE_COLOR_WHITE,	  0xFF, 0xFF, 0xFF);

#ifndef CONFIG_SYS_WHITE_ON_BLACK
	video_setfgcolor (CONSOLE_COLOR_BLACK);
	video_setbgcolor (CONSOLE_COLOR_GREY2);
#else
	video_setfgcolor (CONSOLE_COLOR_GREY2);
	video_setbgcolor (CONSOLE_COLOR_BLACK);
#endif	/* CONFIG_SYS_WHITE_ON_BLACK */

#ifdef CONFIG_VIDEO_LOGO
	/* Paint the logo and retrieve tv base address */
	debug ("[VIDEO] Drawing the logo...\n");
	video_console_address = video_logo ();
#else
	video_console_address = video_fb_address;
#endif

#ifdef VIDEO_BLINK
	/* Enable the blinking (under construction) */
	blink_init ();
	blink_set (0);				/* To Fix! */
#endif

	/* Initialize the console */
	console_col = 0;
	console_row = 0;
	video_enable = 1;

#ifdef VIDEO_MODE_PAL
# define VIDEO_MODE_TMP1	"PAL"
#endif
#ifdef VIDEO_MODE_NTSC
# define VIDEO_MODE_TMP1	"NTSC"
#endif
#ifdef VIDEO_MODE_YUYV
# define VIDEO_MODE_TMP2	"YCbYCr"
#endif
#ifdef VIDEO_MODE_RGB
# define VIDEO_MODE_TMP2	"RGB"
#endif
	debug ( VIDEO_MODE_TMP1
		" %dx%dx%d (" VIDEO_MODE_TMP2 ") on %s - console %dx%d\n",
			VIDEO_COLS, VIDEO_ROWS, VIDEO_MODE_BPP,
			VIDEO_ENCODER_NAME, CONSOLE_COLS, CONSOLE_ROWS);
	return 0;
}

int drv_video_init (void)
{
	int error, devices = 1;

	struct stdio_dev videodev;

	video_init ((void *)(gd->fb_base));	/* Video initialization */

/* Device initialization */

	memset (&videodev, 0, sizeof (videodev));

	strcpy (videodev.name, "video");
	videodev.ext = DEV_EXT_VIDEO;	/* Video extensions */
	videodev.flags = DEV_FLAGS_OUTPUT;	/* Output only */
	videodev.putc = video_putc;	/* 'putc' function */
	videodev.puts = video_puts;	/* 'puts' function */

	error = stdio_register (&videodev);

	return (error == 0) ? devices : error;
}

/************************************************************************/
/* ** ROM capable initialization part - needed to reserve FB memory	*/
/************************************************************************/

/*
 * This is called early in the system initialization to grab memory
 * for the video controller.
 * Returns new address for monitor, after reserving video buffer memory
 *
 * Note that this is running from ROM, so no write access to global data.
 */
ulong video_setmem (ulong addr)
{
	/* Allocate pages for the frame buffer. */
	addr -= VIDEO_SIZE;

	debug ("Reserving %dk for Video Framebuffer at: %08lx\n",
		VIDEO_SIZE>>10, addr);

	return (addr);
}

#endif

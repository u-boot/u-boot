/*
 * (C) Copyright 2001-2002
 * Wolfgang Denk, DENX Software Engineering -- wd@denx.de
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/************************************************************************/
/* ** HEADER FILES							*/
/************************************************************************/

#include <config.h>
#include <common.h>
#include <watchdog.h>
#include <version.h>
#include <stdarg.h>
#include <lcdvideo.h>
#include <linux/types.h>
#include <devices.h>
#if defined(CONFIG_POST)
#include <post.h>
#endif
#include <lcd.h>

#ifdef CONFIG_LCD

/************************************************************************/
/* ** CONFIG STUFF -- should be moved to board config file		*/
/************************************************************************/
#define CONFIG_LCD_LOGO
#define LCD_INFO		/* Display Logo, (C) and system info	*/

#if defined(CONFIG_V37) || defined(CONFIG_EDT32F10)
#undef CONFIG_LCD_LOGO
#undef LCD_INFO
#endif

/* #define LCD_TEST_PATTERN */	/* color backgnd for frame/color adjust */
/* #define CFG_INVERT_COLORS */	/* Not needed - adjust vl_dp instead 	*/
/************************************************************************/

/************************************************************************/
/* ** BITMAP DISPLAY SUPPORT  -- should probably be moved elsewhere	*/
/************************************************************************/

#if (CONFIG_COMMANDS & CFG_CMD_BMP)
#include <bmp_layout.h>
#endif /* (CONFIG_COMMANDS & CFG_CMD_BMP) */

/************************************************************************/
/* ** FONT AND LOGO DATA						*/
/************************************************************************/

#include <video_font.h>		/* Get font data, width and height	*/

#ifdef CONFIG_LCD_LOGO
# include <bmp_logo.h>		/* Get logo data, width and height	*/
#endif

/************************************************************************/
/************************************************************************/

/*
 *  Information about displays we are using.  This is for configuring
 *  the LCD controller and memory allocation.  Someone has to know what
 *  is connected, as we can't autodetect anything.
 */
#define CFG_HIGH	0	/* Pins are active high */
#define CFG_LOW		1	/* Pins are active low */

typedef struct vidinfo {
    ushort	vl_col;		/* Number of columns (i.e. 640) */
    ushort	vl_row;		/* Number of rows (i.e. 480) */
    ushort	vl_width;	/* Width of display area in millimeters */
    ushort	vl_height;	/* Height of display area in millimeters */

    /* LCD configuration register.
    */
    u_char	vl_clkp;	/* Clock polarity */
    u_char	vl_oep;		/* Output Enable polarity */
    u_char	vl_hsp;		/* Horizontal Sync polarity */
    u_char	vl_vsp;		/* Vertical Sync polarity */
    u_char	vl_dp;		/* Data polarity */
    u_char	vl_bpix;	/* Bits per pixel, 0 = 1, 1 = 2, 2 = 4, 3 = 8 */
    u_char	vl_lbw;		/* LCD Bus width, 0 = 4, 1 = 8 */
    u_char	vl_splt;	/* Split display, 0 = single-scan, 1 = dual-scan */
    u_char	vl_clor;	/* Color, 0 = mono, 1 = color */
    u_char	vl_tft;		/* 0 = passive, 1 = TFT */

    /* Horizontal control register.  Timing from data sheet.
    */
    ushort	vl_wbl;		/* Wait between lines */

    /* Vertical control register.
    */
    u_char	vl_vpw;		/* Vertical sync pulse width */
    u_char	vl_lcdac;	/* LCD AC timing */
    u_char	vl_wbf;		/* Wait between frames */
} vidinfo_t;

#define LCD_MONOCHROME	0
#define LCD_COLOR2	1
#define LCD_COLOR4	2
#define LCD_COLOR8	3

/*----------------------------------------------------------------------*/
#ifdef CONFIG_KYOCERA_KCS057QV1AJ
/*
 *  Kyocera KCS057QV1AJ-G23. Passive, color, single scan.
 */
#define LCD_BPP	LCD_COLOR4

static vidinfo_t panel_info = {
    640, 480, 132, 99, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_HIGH,
    LCD_BPP, 1, 0, 1, 0,  5, 0, 0, 0
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_KYOCERA_KCS057QV1AJ */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
#ifdef CONFIG_HITACHI_SP19X001_Z1A
/*
 *  Hitachi SP19X001-. Active, color, single scan.
 */
static vidinfo_t panel_info = {
    640, 480, 154, 116, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_HIGH,
    LCD_COLOR8, 1, 0, 1, 0, 0, 0, 0, 0
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_HITACHI_SP19X001_Z1A */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
#ifdef CONFIG_NEC_NL6648AC33
/*
 *  NEC NL6648AC33-18. Active, color, single scan.
 */
static vidinfo_t panel_info = {
    640, 480, 132, 99, CFG_HIGH, CFG_HIGH, CFG_LOW, CFG_LOW, CFG_HIGH,
    3, 0, 0, 1, 1, 144, 2, 0, 33
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_NEC_NL6648AC33 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_NEC_NL6648BC20
/*
 *  NEC NL6648BC20-08. 6.5", 640x480. Active, color, single scan.
 */
static vidinfo_t panel_info = {
    640, 480, 132, 99, CFG_HIGH, CFG_HIGH, CFG_LOW, CFG_LOW, CFG_HIGH,
    3, 0, 0, 1, 1, 144, 2, 0, 33
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_NEC_NL6648BC20 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_SHARP_LQ104V7DS01
/*
 *  SHARP LQ104V7DS01. 6.5", 640x480. Active, color, single scan.
 */
static vidinfo_t panel_info = {
    640, 480, 132, 99, CFG_HIGH, CFG_HIGH, CFG_LOW, CFG_LOW, CFG_LOW,
    3, 0, 0, 1, 1, 25, 1, 0, 33
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_SHARP_LQ104V7DS01 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_SHARP_16x9
/*
 * Sharp 320x240. Active, color, single scan.  It isn't 16x9, and I am
 * not sure what it is.......
 */
static vidinfo_t panel_info = {
    320, 240, 0, 0, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_HIGH,
    3, 0, 0, 1, 1, 15, 4, 0, 3
};
#endif /* CONFIG_SHARP_16x9 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_SHARP_LQ057Q3DC02
/*
 * Sharp LQ057Q3DC02 display. Active, color, single scan.
 */
#define LCD_DF 12

static vidinfo_t panel_info = {
    320, 240, 0, 0, CFG_HIGH, CFG_HIGH, CFG_LOW, CFG_LOW, CFG_HIGH,
    3, 0, 0, 1, 1, 15, 4, 0, 3
		/* wbl, vpw, lcdac, wbf */
};
#define LCD_INFO_BELOW_LOGO
#endif /* CONFIG_SHARP_LQ057Q3DC02 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_SHARP_LQ64D341
/*
 * Sharp LQ64D341 display, 640x480. Active, color, single scan.
 */
static vidinfo_t panel_info = {
    640, 480, 0, 0, CFG_HIGH, CFG_HIGH, CFG_LOW, CFG_LOW, CFG_HIGH,
    3, 0, 0, 1, 1, 128, 16, 0, 32
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_SHARP_LQ64D341 */

#ifdef CONFIG_SHARP_LQ084V1DG21
/*
 * Sharp LQ084V1DG21 display, 640x480. Active, color, single scan.
 */
static vidinfo_t panel_info = {
    640, 480, 171, 129, CFG_HIGH, CFG_HIGH, CFG_LOW, CFG_LOW, CFG_LOW,
    3, 0, 0, 1, 1, 160, 3, 0, 48
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_SHARP_LQ084V1DG21 */

/*----------------------------------------------------------------------*/

#ifdef CONFIG_HLD1045
/*
 * HLD1045 display, 640x480. Active, color, single scan.
 */
static vidinfo_t panel_info = {
    640, 480, 0, 0, CFG_HIGH, CFG_HIGH, CFG_LOW, CFG_LOW, CFG_HIGH,
    3, 0, 0, 1, 1, 160, 3, 0, 48
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_HLD1045 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_PRIMEVIEW_V16C6448AC
/*
 * Prime View V16C6448AC
 */
static vidinfo_t panel_info = {
    640, 480, 130, 98, CFG_HIGH, CFG_HIGH, CFG_LOW, CFG_LOW, CFG_HIGH,
    3, 0, 0, 1, 1, 144, 2, 0, 35
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_PRIMEVIEW_V16C6448AC */

/*----------------------------------------------------------------------*/

#ifdef CONFIG_OPTREX_BW
/*
 * Optrex   CBL50840-2 NF-FW 99 22 M5
 * or
 * Hitachi  LMG6912RPFC-00T
 * or
 * Hitachi  SP14Q002
 *
 * 320x240. Black & white.
 */
#define OPTREX_BPP	0	/* 0 - monochrome,     1 bpp */
				/* 1 -  4 grey levels, 2 bpp */
				/* 2 - 16 grey levels, 4 bpp */
static vidinfo_t panel_info = {
    320, 240, 0, 0, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_LOW,
    OPTREX_BPP, 0, 0, 0, 0, 0, 0, 0, 0, 4
};
#endif /* CONFIG_OPTREX_BW */

/*-----------------------------------------------------------------*/
#ifdef CONFIG_EDT32F10
/*
 * Emerging Display Technologies 320x240. Passive, monochrome, single scan.
 */
#define LCD_BPP		LCD_MONOCHROME
#define LCD_DF		10

static vidinfo_t panel_info = {
    320, 240, 0, 0, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_HIGH, CFG_LOW,
    LCD_BPP,  0, 0, 0, 0, 33, 0, 0, 0
};
#endif
/*----------------------------------------------------------------------*/

#if defined(LCD_INFO_BELOW_LOGO)
# define LCD_INFO_X		0
# define LCD_INFO_Y		(BMP_LOGO_HEIGHT + VIDEO_FONT_HEIGHT)
#elif defined(CONFIG_LCD_LOGO)
# define LCD_INFO_X		(BMP_LOGO_WIDTH + 4 * VIDEO_FONT_WIDTH)
# define LCD_INFO_Y		(VIDEO_FONT_HEIGHT)
#else
# define LCD_INFO_X		(VIDEO_FONT_WIDTH)
# define LCD_INFO_Y		(VIDEO_FONT_HEIGHT)
#endif

#ifndef LCD_BPP
#define LCD_BPP			LCD_COLOR8
#endif
#ifndef LCD_DF
#define LCD_DF			1
#endif

#define NBITS(bit_code)		(1 << (bit_code))
#define NCOLORS(bit_code)	(1 << NBITS(bit_code))

static int lcd_line_length;

static int lcd_color_fg;
static int lcd_color_bg;

char lcd_is_enabled = 0;		/* Indicate that LCD is enabled	*/

/*
 * Frame buffer memory information
 */
static void *lcd_base;			/* Start of framebuffer memory	*/
static void *lcd_console_address;	/* Start of console buffer	*/


/************************************************************************/
/* ** CONSOLE CONSTANTS							*/
/************************************************************************/

#if LCD_BPP == LCD_MONOCHROME

/*
 * Simple color definitions
 */
#define CONSOLE_COLOR_BLACK	 0
#define CONSOLE_COLOR_WHITE	 1	/* Must remain last / highest */

#else

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
#define CONSOLE_COLOR_GREY	14
#define CONSOLE_COLOR_WHITE	15	/* Must remain last / highest */

#endif

#if defined(CONFIG_LCD_LOGO) && (CONSOLE_COLOR_WHITE >= BMP_LOGO_OFFSET)
#error Default Color Map overlaps with Logo Color Map
#endif

/************************************************************************/

#ifndef PAGE_SIZE
#define	PAGE_SIZE	4096
#endif


/************************************************************************/
/* ** CONSOLE DEFINITIONS & FUNCTIONS					*/
/************************************************************************/

#if defined(CONFIG_LCD_LOGO) && !defined(LCD_INFO_BELOW_LOGO)
#define CONSOLE_ROWS		((panel_info.vl_row-BMP_LOGO_HEIGHT) \
					/ VIDEO_FONT_HEIGHT)
#else
#define CONSOLE_ROWS		(panel_info.vl_row / VIDEO_FONT_HEIGHT)
#endif
#define CONSOLE_COLS		(panel_info.vl_col / VIDEO_FONT_WIDTH)
#define CONSOLE_ROW_SIZE	(VIDEO_FONT_HEIGHT * lcd_line_length)
#define CONSOLE_ROW_FIRST	(lcd_console_address)
#define CONSOLE_ROW_SECOND	(lcd_console_address + CONSOLE_ROW_SIZE)
#define CONSOLE_ROW_LAST	(lcd_console_address + CONSOLE_SIZE \
					- CONSOLE_ROW_SIZE)
#define CONSOLE_SIZE		(CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_SCROLL_SIZE	(CONSOLE_SIZE - CONSOLE_ROW_SIZE)

#if  LCD_BPP == LCD_MONOCHROME
#define COLOR_MASK(c)		((c)      | (c) << 1 | (c) << 2 | (c) << 3 | \
				 (c) << 4 | (c) << 5 | (c) << 6 | (c) << 7)
#elif LCD_BPP == LCD_COLOR8
#define COLOR_MASK(c)		(c)
#else
#error Unsupported LCD BPP.
#endif

static short console_col;
static short console_row;

/************************************************************************/

ulong	lcd_setmem (ulong addr);

static void	lcd_drawchars  (ushort x, ushort y, uchar *str, int count);
static inline void lcd_puts_xy (ushort x, ushort y, uchar *s);
static inline void lcd_putc_xy (ushort x, ushort y, uchar  c);

int	lcd_init (void *lcdbase);

static void	lcd_ctrl_init (void *lcdbase);
static void	lcd_enable (void);
static void    *lcd_logo (void);
#if LCD_BPP == LCD_COLOR8
static void	lcd_setcolreg (ushort regno,
				ushort red, ushort green, ushort blue);
#endif
#if LCD_BPP == LCD_MONOCHROME
static void	lcd_initcolregs (void);
#endif
static int	lcd_getbgcolor (void);
static void	lcd_setfgcolor (int color);
static void	lcd_setbgcolor (int color);

#if defined(CONFIG_RBC823)
void	lcd_disable (void);
#endif

#ifdef	NOT_USED_SO_FAR
static void	lcd_getcolreg (ushort regno,
				ushort *red, ushort *green, ushort *blue);
static int	lcd_getfgcolor (void);
#endif	/* NOT_USED_SO_FAR */

/************************************************************************/

/*----------------------------------------------------------------------*/

static void console_scrollup (void)
{
#if 1
	/* Copy up rows ignoring the first one */
	memcpy (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND, CONSOLE_SCROLL_SIZE);

	/* Clear the last one */
	memset (CONSOLE_ROW_LAST, COLOR_MASK(lcd_color_bg), CONSOLE_ROW_SIZE);
#else
	/*
	 * Poor attempt to optimize speed by moving "long"s.
	 * But the code is ugly, and not a bit faster :-(
	 */
	ulong *t = (ulong *)CONSOLE_ROW_FIRST;
	ulong *s = (ulong *)CONSOLE_ROW_SECOND;
	ulong    l = CONSOLE_SCROLL_SIZE / sizeof(ulong);
	uchar  c = lcd_color_bg & 0xFF;
	ulong val= (c<<24) | (c<<16) | (c<<8) | c;

	while (l--)
		*t++ = *s++;

	t = (ulong *)CONSOLE_ROW_LAST;
	l = CONSOLE_ROW_SIZE / sizeof(ulong);

	while (l-- > 0)
		*t++ = val;
#endif
}

/*----------------------------------------------------------------------*/

static inline void console_back (void)
{
	if (--console_col < 0) {
		console_col = CONSOLE_COLS-1 ;
		if (--console_row < 0) {
			console_row = 0;
		}
	}

	lcd_putc_xy (console_col * VIDEO_FONT_WIDTH,
		     console_row * VIDEO_FONT_HEIGHT,
		     ' ');
}

/*----------------------------------------------------------------------*/

static inline void console_newline (void)
{
	++console_row;
	console_col = 0;

	/* Check if we need to scroll the terminal */
	if (console_row >= CONSOLE_ROWS) {
		/* Scroll everything up */
		console_scrollup () ;
		--console_row;
	}
}

/*----------------------------------------------------------------------*/

void lcd_putc (const char c)
{
	if (!lcd_is_enabled) {
		serial_putc(c);
		return;
	}

	switch (c) {
	case '\r':	console_col = 0;
			return;

	case '\n':	console_newline();
			return;

	case '\t':	/* Tab (8 chars alignment) */
			console_col |=  8;
			console_col &= ~7;

			if (console_col >= CONSOLE_COLS) {
				console_newline();
			}
			return;

	case '\b':	console_back();
			return;

	default:	lcd_putc_xy (console_col * VIDEO_FONT_WIDTH,
				     console_row * VIDEO_FONT_HEIGHT,
				     c);
			if (++console_col >= CONSOLE_COLS) {
				console_newline();
			}
			return;
	}
	/* NOTREACHED */
}

/*----------------------------------------------------------------------*/

void lcd_puts (const char *s)
{
	if (!lcd_is_enabled) {
		serial_puts (s);
		return;
	}

	while (*s) {
		lcd_putc (*s++);
	}
}

/************************************************************************/
/* ** Low-Level Graphics Routines					*/
/************************************************************************/

static void lcd_drawchars (ushort x, ushort y, uchar *str, int count)
{
	uchar *dest;
	ushort off, row;

	dest = (uchar *)(lcd_base + y * lcd_line_length + x * (1 << LCD_BPP) / 8);
	off  = x * (1 << LCD_BPP) % 8;

	for (row=0;  row < VIDEO_FONT_HEIGHT;  ++row, dest += lcd_line_length)  {
		uchar *s = str;
		uchar *d = dest;
		int i;

#if LCD_BPP == LCD_MONOCHROME
		uchar rest = *d & -(1 << (8-off));
		uchar sym;
#endif
		for (i=0; i<count; ++i) {
			uchar c, bits;

			c = *s++;
			bits = video_fontdata[c * VIDEO_FONT_HEIGHT + row];

#if LCD_BPP == LCD_MONOCHROME
			sym  = (COLOR_MASK(lcd_color_fg) & bits) |
			       (COLOR_MASK(lcd_color_bg) & ~bits);

			*d++ = rest | (sym >> off);
			rest = sym << (8-off);
#elif LCD_BPP == LCD_COLOR8
			for (c=0; c<8; ++c) {
				*d++ = (bits & 0x80) ?
						lcd_color_fg : lcd_color_bg;
				bits <<= 1;
			}
#endif
		}

#if LCD_BPP == LCD_MONOCHROME
		*d  = rest | (*d & ((1 << (8-off)) - 1));
#endif
	}
}

/*----------------------------------------------------------------------*/

static inline void lcd_puts_xy (ushort x, ushort y, uchar *s)
{
#if defined(CONFIG_LCD_LOGO) && !defined(LCD_INFO_BELOW_LOGO)
	lcd_drawchars (x, y+BMP_LOGO_HEIGHT, s, strlen (s));
#else
	lcd_drawchars (x, y, s, strlen (s));
#endif
}

/*----------------------------------------------------------------------*/

static inline void lcd_putc_xy (ushort x, ushort y, uchar c)
{
#if defined(CONFIG_LCD_LOGO) && !defined(LCD_INFO_BELOW_LOGO)
	lcd_drawchars (x, y+BMP_LOGO_HEIGHT, &c, 1);
#else
	lcd_drawchars (x, y, &c, 1);
#endif
}

/************************************************************************/
/**  Small utility to check that you got the colours right		*/
/************************************************************************/
#ifdef LCD_TEST_PATTERN

#define	N_BLK_VERT	2
#define	N_BLK_HOR	3

static int test_colors[N_BLK_HOR*N_BLK_VERT] = {
	CONSOLE_COLOR_RED,	CONSOLE_COLOR_GREEN,	CONSOLE_COLOR_YELLOW,
	CONSOLE_COLOR_BLUE,	CONSOLE_COLOR_MAGENTA,	CONSOLE_COLOR_CYAN,
};

static void test_pattern (void)
{
	ushort v_max  = panel_info.vl_row;
	ushort h_max  = panel_info.vl_col;
	ushort v_step = (v_max + N_BLK_VERT - 1) / N_BLK_VERT;
	ushort h_step = (h_max + N_BLK_HOR  - 1) / N_BLK_HOR;
	ushort v, h;
	uchar *pix = (uchar *)lcd_base;

	printf ("[LCD] Test Pattern: %d x %d [%d x %d]\n",
		h_max, v_max, h_step, v_step);

	/* WARNING: Code silently assumes 8bit/pixel */
	for (v=0; v<v_max; ++v) {
		uchar iy = v / v_step;
		for (h=0; h<h_max; ++h) {
			uchar ix = N_BLK_HOR * iy + (h/h_step);
			*pix++ = test_colors[ix];
		}
	}
}
#endif /* LCD_TEST_PATTERN */


/************************************************************************/
/* ** GENERIC Initialization Routines					*/
/************************************************************************/

int drv_lcd_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	device_t lcddev;
	int rc;

	lcd_base = (void *)(gd->fb_base);

	lcd_line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;

	lcd_init (lcd_base);		/* LCD initialization */

	/* Device initialization */
	memset (&lcddev, 0, sizeof (lcddev));

	strcpy (lcddev.name, "lcd");
	lcddev.ext   = 0;			/* No extensions */
	lcddev.flags = DEV_FLAGS_OUTPUT;	/* Output only */
	lcddev.putc  = lcd_putc;		/* 'putc' function */
	lcddev.puts  = lcd_puts;		/* 'puts' function */

	rc = device_register (&lcddev);

	return (rc == 0) ? 1 : rc;
}

/*----------------------------------------------------------------------*/

int lcd_init (void *lcdbase)
{
	/* Initialize the lcd controller */
	debug ("[LCD] Initializing LCD frambuffer at %p\n", lcdbase);

	lcd_ctrl_init (lcdbase);

#if LCD_BPP == LCD_MONOCHROME
	/* Setting the palette */
	lcd_initcolregs();

#elif LCD_BPP == LCD_COLOR8
	/* Setting the palette */
	lcd_setcolreg  (CONSOLE_COLOR_BLACK,       0,    0,    0);
	lcd_setcolreg  (CONSOLE_COLOR_RED,	0xFF,    0,    0);
	lcd_setcolreg  (CONSOLE_COLOR_GREEN,       0, 0xFF,    0);
	lcd_setcolreg  (CONSOLE_COLOR_YELLOW,	0xFF, 0xFF,    0);
	lcd_setcolreg  (CONSOLE_COLOR_BLUE,        0,    0, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_MAGENTA,	0xFF,    0, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_CYAN,	   0, 0xFF, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_GREY,	0xAA, 0xAA, 0xAA);
	lcd_setcolreg  (CONSOLE_COLOR_WHITE,	0xFF, 0xFF, 0xFF);
#endif

#ifndef CFG_WHITE_ON_BLACK
	lcd_setfgcolor (CONSOLE_COLOR_BLACK);
	lcd_setbgcolor (CONSOLE_COLOR_WHITE);
#else
	lcd_setfgcolor (CONSOLE_COLOR_WHITE);
	lcd_setbgcolor (CONSOLE_COLOR_BLACK);
#endif	/* CFG_WHITE_ON_BLACK */

#ifdef	LCD_TEST_PATTERN
	test_pattern();
#else
	/* set framebuffer to background color */
	memset ((char *)lcd_base,
		COLOR_MASK(lcd_getbgcolor()),
		lcd_line_length*panel_info.vl_row);
#endif

	lcd_enable ();

	/* Paint the logo and retrieve LCD base address */
	debug ("[LCD] Drawing the logo...\n");
	lcd_console_address = lcd_logo ();

	/* Initialize the console */
	console_col = 0;
#ifdef LCD_INFO_BELOW_LOGO
	console_row = 7 + BMP_LOGO_HEIGHT / VIDEO_FONT_HEIGHT;
#else
	console_row = 1;	/* leave 1 blank line below logo */
#endif
	lcd_is_enabled = 1;

	return 0;
}


/************************************************************************/
/* ** ROM capable initialization part - needed to reserve FB memory	*/
/************************************************************************/

/*
 * This is called early in the system initialization to grab memory
 * for the LCD controller.
 * Returns new address for monitor, after reserving LCD buffer memory
 *
 * Note that this is running from ROM, so no write access to global data.
 */
ulong lcd_setmem (ulong addr)
{
	ulong size;
	int line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;

	debug ("LCD panel info: %d x %d, %d bit/pix\n",
		panel_info.vl_col, panel_info.vl_row, NBITS (panel_info.vl_bpix) );

	size = line_length * panel_info.vl_row;

	/* Round up to nearest full page */
	size = (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

	/* Allocate pages for the frame buffer. */
	addr -= size;

	debug ("Reserving %ldk for LCD Framebuffer at: %08lx\n", size>>10, addr);

	return (addr);
}


/************************************************************************/
/* ----------------- chipset specific functions ----------------------- */
/************************************************************************/

static void lcd_ctrl_init (void *lcdbase)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile lcd823_t *lcdp = &immr->im_lcd;

	uint lccrtmp;
	uint lchcr_hpc_tmp;

	/* Initialize the LCD control register according to the LCD
	 * parameters defined.  We do everything here but enable
	 * the controller.
	 */

	lccrtmp  = LCDBIT (LCCR_BNUM_BIT,
		   (((panel_info.vl_row * panel_info.vl_col) * (1 << LCD_BPP)) / 128));

	lccrtmp |= LCDBIT (LCCR_CLKP_BIT, panel_info.vl_clkp)	|
		   LCDBIT (LCCR_OEP_BIT,  panel_info.vl_oep)	|
		   LCDBIT (LCCR_HSP_BIT,  panel_info.vl_hsp)	|
		   LCDBIT (LCCR_VSP_BIT,  panel_info.vl_vsp)	|
		   LCDBIT (LCCR_DP_BIT,   panel_info.vl_dp)	|
		   LCDBIT (LCCR_BPIX_BIT, panel_info.vl_bpix)	|
		   LCDBIT (LCCR_LBW_BIT,  panel_info.vl_lbw)	|
		   LCDBIT (LCCR_SPLT_BIT, panel_info.vl_splt)	|
		   LCDBIT (LCCR_CLOR_BIT, panel_info.vl_clor)	|
		   LCDBIT (LCCR_TFT_BIT,  panel_info.vl_tft);

#if 0
	lccrtmp |= ((SIU_LEVEL5 / 2) << 12);
	lccrtmp |= LCCR_EIEN;
#endif

	lcdp->lcd_lccr = lccrtmp;
	lcdp->lcd_lcsr = 0xFF;		/* Clear pending interrupts */

	/* Initialize LCD controller bus priorities.
	 */
#ifdef CONFIG_RBC823
	immr->im_siu_conf.sc_sdcr = (immr->im_siu_conf.sc_sdcr & ~0x0f) | 1;	/* RAID = 01, LAID = 00 */
#else
	immr->im_siu_conf.sc_sdcr &= ~0x0f;	/* RAID = LAID = 0 */

	/* set SHFT/CLOCK division factor 4
	 * This needs to be set based upon display type and processor
	 * speed.  The TFT displays run about 20 to 30 MHz.
	 * I was running 64 MHz processor speed.
	 * The value for this divider must be chosen so the result is
	 * an integer of the processor speed (i.e., divide by 3 with
	 * 64 MHz would be bad).
	 */
	immr->im_clkrst.car_sccr &= ~0x1F;
	immr->im_clkrst.car_sccr |= LCD_DF;	/* was 8 */

#endif /* CONFIG_RBC823 */

#if defined(CONFIG_RBC823)
	/* Enable LCD on port D.
	 */
	immr->im_ioport.iop_pddat &= 0x0300;
	immr->im_ioport.iop_pdpar |= 0x1CFF;
	immr->im_ioport.iop_pddir |= 0x1CFF;

	/* Configure LCD_ON, VEE_ON, CCFL_ON on port B.
	 */
	immr->im_cpm.cp_pbdat &= ~0x00005001;
	immr->im_cpm.cp_pbpar &= ~0x00005001;
	immr->im_cpm.cp_pbdir |=  0x00005001;
#elif !defined(CONFIG_EDT32F10)
	/* Enable LCD on port D.
	 */
	immr->im_ioport.iop_pdpar |= 0x1FFF;
	immr->im_ioport.iop_pddir |= 0x1FFF;

	/* Enable LCD_A/B/C on port B.
	 */
	immr->im_cpm.cp_pbpar |= 0x00005001;
	immr->im_cpm.cp_pbdir |= 0x00005001;
#else
	/* Enable LCD on port D.
	 */
	immr->im_ioport.iop_pdpar |= 0x1DFF;
	immr->im_ioport.iop_pdpar &= ~0x0200;
	immr->im_ioport.iop_pddir |= 0x1FFF;
	immr->im_ioport.iop_pddat |= 0x0200;
#endif

	/* Load the physical address of the linear frame buffer
	 * into the LCD controller.
	 * BIG NOTE:  This has to be modified to load A and B depending
	 * upon the split mode of the LCD.
	 */
	lcdp->lcd_lcfaa = (ulong)lcd_base;
	lcdp->lcd_lcfba = (ulong)lcd_base;

	/* MORE HACKS...This must be updated according to 823 manual
	 * for different panels.
	 * Udi Finkelstein - done - see below:
	 * Note: You better not try unsupported combinations such as
	 * 4-bit wide passive dual scan LCD at 4/8 Bit color.
	 */
	lchcr_hpc_tmp =
	       	(panel_info.vl_col *
		 (panel_info.vl_tft ? 8 :
			(((2 - panel_info.vl_lbw) << /* 4 bit=2, 8-bit = 1 */
			 /* use << to mult by: single scan = 1, dual scan = 2 */
			  panel_info.vl_splt) *
			 (panel_info.vl_bpix | 1)))) >> 3; /* 2/4 BPP = 1, 8/16 BPP = 3 */

	lcdp->lcd_lchcr = LCHCR_BO |
			  LCDBIT (LCHCR_AT_BIT, 4) |
			  LCDBIT (LCHCR_HPC_BIT, lchcr_hpc_tmp) |
			  panel_info.vl_wbl;

	lcdp->lcd_lcvcr = LCDBIT (LCVCR_VPW_BIT, panel_info.vl_vpw) |
			  LCDBIT (LCVCR_LCD_AC_BIT, panel_info.vl_lcdac) |
			  LCDBIT (LCVCR_VPC_BIT, panel_info.vl_row) |
			  panel_info.vl_wbf;

}

/*----------------------------------------------------------------------*/

#ifdef	NOT_USED_SO_FAR
static void
lcd_getcolreg (ushort regno, ushort *red, ushort *green, ushort *blue)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	unsigned short colreg, *cmap_ptr;

	cmap_ptr = (unsigned short *)&cp->lcd_cmap[regno * 2];

	colreg = *cmap_ptr;
#ifdef	CFG_INVERT_COLORS
	colreg ^= 0x0FFF;
#endif

	*red   = (colreg >> 8) & 0x0F;
	*green = (colreg >> 4) & 0x0F;
	*blue  =  colreg       & 0x0F;
}
#endif	/* NOT_USED_SO_FAR */

/*----------------------------------------------------------------------*/

#if LCD_BPP == LCD_COLOR8
static void
lcd_setcolreg (ushort regno, ushort red, ushort green, ushort blue)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	unsigned short colreg, *cmap_ptr;

	cmap_ptr = (unsigned short *)&cp->lcd_cmap[regno * 2];

	colreg = ((red   & 0x0F) << 8) |
		 ((green & 0x0F) << 4) |
		  (blue  & 0x0F) ;
#ifdef	CFG_INVERT_COLORS
	colreg ^= 0x0FFF;
#endif
	*cmap_ptr = colreg;

	debug ("setcolreg: reg %2d @ %p: R=%02X G=%02X B=%02X => %02X%02X\n",
		regno, &(cp->lcd_cmap[regno * 2]),
		red, green, blue,
                cp->lcd_cmap[ regno * 2 ], cp->lcd_cmap[(regno * 2) + 1]);
}
#endif	/* LCD_COLOR8 */

/*----------------------------------------------------------------------*/

#if LCD_BPP == LCD_MONOCHROME
static
void lcd_initcolregs (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	ushort regno;

	for (regno = 0; regno < 16; regno++) {
		cp->lcd_cmap[regno * 2] = 0;
		cp->lcd_cmap[(regno * 2) + 1] = regno & 0x0f;
	}
}
#endif

/*----------------------------------------------------------------------*/

static void lcd_setfgcolor (int color)
{
	lcd_color_fg = color & 0x0F;
}

/*----------------------------------------------------------------------*/

static void lcd_setbgcolor (int color)
{
	lcd_color_bg = color & 0x0F;
}

/*----------------------------------------------------------------------*/

#ifdef	NOT_USED_SO_FAR
static int lcd_getfgcolor (void)
{
	return lcd_color_fg;
}
#endif	/* NOT_USED_SO_FAR */

/*----------------------------------------------------------------------*/

static int lcd_getbgcolor (void)
{
	return lcd_color_bg;
}

/*----------------------------------------------------------------------*/

static void lcd_enable (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile lcd823_t *lcdp = &immr->im_lcd;

	/* Enable the LCD panel */
#ifndef CONFIG_RBC823
	immr->im_siu_conf.sc_sdcr |= (1 << (31 - 25));		/* LAM = 1 */
#endif
	lcdp->lcd_lccr |= LCCR_PON;

#ifdef CONFIG_V37
	/* Turn on display backlight */
	immr->im_cpm.cp_pbpar |= 0x00008000;
	immr->im_cpm.cp_pbdir |= 0x00008000;
#elif defined(CONFIG_RBC823)
	/* Turn on display backlight */
	immr->im_cpm.cp_pbdat |= 0x00004000;
#endif

#if defined(CONFIG_LWMON)
    {	uchar c = pic_read (0x60);
#if defined(CONFIG_LCD) && defined(CONFIG_LWMON) && (CONFIG_POST & CFG_POST_SYSMON)
    	c |= 0x04;	/* Chip Enable LCD */
#else
    	c |= 0x07;	/* Power on CCFL, Enable CCFL, Chip Enable LCD */
#endif
	pic_write (0x60, c);
    }
#endif /* CONFIG_LWMON */

#if defined(CONFIG_R360MPI)
    {
	extern void r360_i2c_lcd_write (uchar data0, uchar data1);

	r360_i2c_lcd_write(0x10, 0x01);
	r360_i2c_lcd_write(0x20, 0x01);
	r360_i2c_lcd_write(0x3F, 0xFF);
	r360_i2c_lcd_write(0x47, 0xFF);
    }
#endif /* CONFIG_R360MPI */
#ifdef CONFIG_RBC823
	udelay(200000); /* wait 200ms */
	/* Turn VEE_ON first */
	immr->im_cpm.cp_pbdat |= 0x00000001;
	udelay(200000); /* wait 200ms */
	/* Now turn on LCD_ON */
	immr->im_cpm.cp_pbdat |= 0x00001000;
#endif
}

/*----------------------------------------------------------------------*/

#if defined (CONFIG_RBC823)
void lcd_disable (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile lcd823_t *lcdp = &immr->im_lcd;

#if defined(CONFIG_LWMON)
    {	uchar c = pic_read (0x60);
    	c &= ~0x07;	/* Power off CCFL, Disable CCFL, Chip Disable LCD */
	pic_write (0x60, c);
    }
#elif defined(CONFIG_R360MPI)
    {
	extern void r360_i2c_lcd_write (uchar data0, uchar data1);

	r360_i2c_lcd_write(0x10, 0x00);
	r360_i2c_lcd_write(0x20, 0x00);
	r360_i2c_lcd_write(0x30, 0x00);
	r360_i2c_lcd_write(0x40, 0x00);
    }
#endif /* CONFIG_LWMON */
	/* Disable the LCD panel */
	lcdp->lcd_lccr &= ~LCCR_PON;
#ifdef CONFIG_RBC823
	/* Turn off display backlight, VEE and LCD_ON */
	immr->im_cpm.cp_pbdat &= ~0x00005001;
#else
	immr->im_siu_conf.sc_sdcr &= ~(1 << (31 - 25));	/* LAM = 0 */
#endif /* CONFIG_RBC823 */
}
#endif	/* NOT_USED_SO_FAR || CONFIG_RBC823 */


/************************************************************************/
/* ** Chipset depending Bitmap / Logo stuff...				*/
/************************************************************************/


#ifdef CONFIG_LCD_LOGO
static void bitmap_plot (int x, int y)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	ushort *cmap;
	ushort i;
	uchar *bmap;
	uchar *fb;

	debug ("Logo: width %d  height %d  colors %d  cmap %d\n",
		BMP_LOGO_WIDTH, BMP_LOGO_HEIGHT, BMP_LOGO_COLORS,
		sizeof(bmp_logo_palette)/(sizeof(ushort))
	);

	/* Leave room for default color map */
	cmap = (ushort *)&(cp->lcd_cmap[BMP_LOGO_OFFSET*sizeof(ushort)]);

	WATCHDOG_RESET();

	/* Set color map */
	for (i=0; i<(sizeof(bmp_logo_palette)/(sizeof(ushort))); ++i) {
		ushort colreg = bmp_logo_palette[i];
#ifdef	CFG_INVERT_COLORS
		colreg ^= 0xFFF;
#endif
		*cmap++ = colreg;
	}

	bmap = &bmp_logo_bitmap[0];
	fb   = (char *)(lcd_base + y * lcd_line_length + x);

	WATCHDOG_RESET();

	for (i=0; i<BMP_LOGO_HEIGHT; ++i) {
		memcpy (fb, bmap, BMP_LOGO_WIDTH);
		bmap += BMP_LOGO_WIDTH;
		fb   += panel_info.vl_col;
	}

	WATCHDOG_RESET();
}
#endif /* CONFIG_LCD_LOGO */

#if (CONFIG_COMMANDS & CFG_CMD_BMP)
/*
 * Display the BMP file located at address bmp_image.
 * Only uncompressed
 */
int lcd_display_bitmap(ulong bmp_image)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	ushort *cmap;
	ushort i, j;
	uchar *fb;
	bmp_image_t *bmp=(bmp_image_t *)bmp_image;
	uchar *bmap;
	ushort padded_line;
	unsigned long width, height;
	unsigned colors,bpix;
	unsigned long compression;

	WATCHDOG_RESET();

	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M'))) {
		printf ("Error: no valid bmp image at %lx\n", bmp_image);
		return 1;
	}

	width = le32_to_cpu (bmp->header.width);
	height = le32_to_cpu (bmp->header.height);
	colors = 1<<le16_to_cpu (bmp->header.bit_count);
	compression = le32_to_cpu (bmp->header.compression);

	bpix = NBITS(panel_info.vl_bpix);

	if ((bpix != 1) && (bpix != 8)) {
		printf ("Error: %d bit/pixel mode not supported by U-Boot\n",
			bpix);
		return 1;
	}

	if (bpix != le16_to_cpu(bmp->header.bit_count)) {
		printf ("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
			bpix,
			le16_to_cpu(bmp->header.bit_count));
		return 1;
	}

	if (compression!=BMP_BI_RGB) {
		printf ("Error: compression type %ld not supported\n",
			compression);
		return 1;
	}

	debug ("Display-bmp: %d x %d  with %d colors\n",
	       width, height, colors);

	if (bpix==8) {
		/* Fill the entire color map */
		cmap = (ushort *)&(cp->lcd_cmap[255*sizeof(ushort)]);

		/* Set color map */
		for (i = 0; i < colors; ++i) {
			bmp_color_table_entry_t cte = bmp->color_table[i];
			ushort colreg =
				((cte.red>>4)   << 8) |
				((cte.green>>4) << 4) |
				(cte.blue>>4) ;
#ifdef	CFG_INVERT_COLORS
			colreg ^= 0xFFF;
#endif
			*cmap-- = colreg;
		}

		WATCHDOG_RESET();
	}

	padded_line = (width&0x3) ? ((width&~0x3)+4) : (width);
	if (width>panel_info.vl_col)
		width = panel_info.vl_col;
	if (height>panel_info.vl_row)
		height = panel_info.vl_row;

	bmap = (uchar *)bmp + le32_to_cpu (bmp->header.data_offset);
	fb   = (uchar *)
		(lcd_base +
		 (((height>=panel_info.vl_row) ? panel_info.vl_row : height)-1)
		 * lcd_line_length);
	for (i = 0; i < height; ++i) {
		WATCHDOG_RESET();
		for (j = 0; j < width ; j++)
			*(fb++)=255-*(bmap++);
		bmap += (width - padded_line);
		fb   -= (width + lcd_line_length);
	}

	return (0);
}
#endif /* (CONFIG_COMMANDS & CFG_CMD_BMP) */

/*----------------------------------------------------------------------*/

static void *lcd_logo (void)
{
#ifdef LCD_INFO
	DECLARE_GLOBAL_DATA_PTR;

	char info[80];
	char temp[32];
#endif /* LCD_INFO */

#ifdef CONFIG_SPLASH_SCREEN
	char *s;
	ulong addr;

	if ((s = getenv("splashimage")) != NULL) {
		addr = simple_strtoul(s, NULL, 16);

		if (lcd_display_bitmap (addr) == 0) {
			return ((void *)lcd_base);
		}
	}
#endif	/* CONFIG_SPLASH_SCREEN */

#ifdef CONFIG_LCD_LOGO
	bitmap_plot (0, 0);
#endif /* CONFIG_LCD_LOGO */


#ifdef LCD_INFO
	sprintf (info, "%s (%s - %s) ", U_BOOT_VERSION, __DATE__, __TIME__);
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y, info, strlen(info));

	sprintf (info, "(C) 2003 DENX Software Engineering");
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT,
					info, strlen(info));

	sprintf (info, "    Wolfgang DENK, wd@denx.de");
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT * 2,
					info, strlen(info));
#ifdef LCD_INFO_BELOW_LOGO
	sprintf (info, "MPC823 CPU at %s MHz",
		strmhz(temp, gd->cpu_clk));
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT * 3,
					info, strlen(info));
	sprintf (info, "  %ld MB RAM, %ld MB Flash",
		gd->ram_size >> 20,
		gd->bd->bi_flashsize >> 20 );
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT * 4,
					info, strlen(info));
#else
	/* leave one blank line */

	sprintf (info, "MPC823 CPU at %s MHz, %ld MB RAM, %ld MB Flash",
		strmhz(temp, gd->cpu_clk),
		gd->ram_size >> 20,
		gd->bd->bi_flashsize >> 20 );
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT * 4,
					info, strlen(info));
#endif /* LCD_INFO_BELOW_LOGO */
#endif /* LCD_INFO */

#if defined(CONFIG_LCD_LOGO) && !defined(LCD_INFO_BELOW_LOGO)
	return ((void *)((ulong)lcd_base + BMP_LOGO_HEIGHT * lcd_line_length));
#else
	return ((void *)lcd_base);
#endif	/* CONFIG_LCD_LOGO */
}

/************************************************************************/
/************************************************************************/

#endif /* CONFIG_LCD */

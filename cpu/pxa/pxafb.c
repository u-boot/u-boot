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

#define DEBUG 1
#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <lcdvideo.h>
#include <linux/types.h>
#include <devices.h>
#include <asm/arch/pxa-regs.h>

#ifdef CONFIG_LCD

/************************************************************************/
/* ** CONFIG STUFF -- should be moved to board config file		*/
/************************************************************************/
#ifndef CONFIG_EDT32F10
#define CONFIG_LCD_LOGO
#define LCD_INFO		/* Display Logo, (C) and system info	*/
#endif

#ifdef CONFIG_V37
#undef CONFIG_LCD_LOGO
#undef LCD_INFO
#endif

#undef CONFIG_LCD_LOGO

#define LCD_TEST_PATTERN 
/* #define LCD_TEST_PATTERN */	/* color backgnd for frame/color adjust */
/* #define CFG_INVERT_COLORS */	/* Not needed - adjust vl_dp instead 	*/
/************************************************************************/

/************************************************************************/
/* ** FONT AND LOGO DATA						*/
/************************************************************************/

#include <video_font.h>		/* Get font data, width and height	*/

#ifdef CONFIG_LCD_LOGO
# include <bmp_nexus.h>		/* Get logo data, width and height	*/
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

/* PXA LCD DMA descriptor */
struct pxafb_dma_descriptor {
	u_long fdadr;
	u_long fsadr;
	u_long fidr;
	u_long ldcmd;
};

/* PXA LCD info */
struct pxafb_info {

	/* Misc registers */
	u_long reg_lccr3;
	u_long reg_lccr2;
	u_long reg_lccr1;
	u_long reg_lccr0;
	u_long fdadr0;
	u_long fdadr1;

	/* DMA descriptors */
	struct pxafb_dma_descriptor * 	dmadesc_fblow;
	struct pxafb_dma_descriptor * 	dmadesc_fbhigh;
	struct pxafb_dma_descriptor *	dmadesc_palette;

	u_long		screen;		/* physical address of frame buffer */
	u_long		palette;	/* physical address of palette memory */
	u_int		palette_size;
};

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
    u_char	vl_bpix;	/* Bits per pixel, 0 = 1, 1 = 2, 2 = 4, 3 = 8, 4 = 16 */
    u_char	vl_lbw;		/* LCD Bus width, 0 = 4, 1 = 8 */
    u_char	vl_splt;	/* Split display, 0 = single-scan, 1 = dual-scan */
    u_char	vl_clor;	/* Color, 0 = mono, 1 = color */
    u_char	vl_tft;		/* 0 = passive, 1 = TFT */

    /* Horizontal control register.  Timing from data sheet.
    */
    ushort	vl_hpw;		/* Horz sync pulse width */
    u_char	vl_blw;		/* Wait before of line */
    u_char	vl_elw;		/* Wait end of line */

    /* Vertical control register.
    */
    u_char	vl_vpw;		/* Vertical sync pulse width */
    u_char	vl_bfw;		/* Wait before of frame */
    u_char	vl_efw;		/* Wait end of frame */

    u_char	vl_lcdac;	/* LCD AC timing */

    /* PXA LCD controller params
     */
    struct pxafb_info pxa;

} vidinfo_t;

#define LCD_MONOCHROME	0
#define LCD_COLOR2	1
#define LCD_COLOR4	2
#define LCD_COLOR8	3
#define LCD_COLOR16	4

/*----------------------------------------------------------------------*/
#define CONFIG_PXA_VGA
#ifdef CONFIG_PXA_VGA
/*
 *  LCD outputs connected to a video DAC
 */

#define LCD_BPP	LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
#define REG_LCCR0	0x003008f8
#define REG_LCCR3	0x0300FF01 

/* 640x480x16 @ 61 Hz */
static vidinfo_t panel_info = {
	vl_col: 	640, 
	vl_row: 	480,
	vl_width: 	640,
	vl_height: 	480,
	vl_clkp: 	CFG_HIGH,
        vl_oep: 	CFG_HIGH,   
	vl_hsp: 	CFG_HIGH,
    	vl_vsp: 	CFG_HIGH,
    	vl_dp: 		CFG_HIGH,
    	vl_bpix: 	LCD_BPP,
    	vl_lbw: 	0,
	vl_splt: 	0,
    	vl_clor: 	0,
	vl_lcdac: 	0,
    	vl_tft: 	1,
    	vl_hpw: 	40,
    	vl_blw: 	56,
    	vl_elw: 	56,
    	vl_vpw: 	20,
    	vl_bfw: 	8,
    	vl_efw: 	8,
};
#endif /* CONFIG_PXA_VIDEO */

#ifdef CONFIG_SHARP_LM8V31

#define LCD_BPP	LCD_COLOR8
#define LCD_INVERT_COLORS	/* Needed for colors to be correct, but why? */

/* you have to set lccr0 and lccr3 (including pcd) */
#define REG_LCCR0	0x0030087C
#define REG_LCCR3	0x0340FF08 

static vidinfo_t panel_info = {
	vl_col: 	640, 
	vl_row: 	480,
	vl_width: 	157,
	vl_height: 	118,
	vl_clkp: 	CFG_HIGH,
        vl_oep: 	CFG_HIGH,   
	vl_hsp: 	CFG_HIGH,
    	vl_vsp: 	CFG_HIGH,
    	vl_dp: 		CFG_HIGH,
    	vl_bpix: 	LCD_BPP,
    	vl_lbw: 	0,
	vl_splt: 	1,
    	vl_clor: 	1,
	vl_lcdac: 	0,
    	vl_tft: 	0,
    	vl_hpw: 	1,
    	vl_blw: 	3,
    	vl_elw: 	3,
    	vl_vpw: 	1,
    	vl_bfw: 	0,
    	vl_efw: 	0,
};
#endif /* CONFIG_SHARP_LM8V31 */

/*----------------------------------------------------------------------*/

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

static char lcd_is_enabled = 0;		/* Indicate that LCD is enabled	*/

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

#elif 	LCD_BPP == LCD_COLOR8

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

#else /* 16 bit */

#define CONSOLE_COLOR_BLACK	 0x0000
#define CONSOLE_COLOR_WHITE	 0xffff	/* Must remain last / highest */

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
#elif LCD_BPP == LCD_COLOR16
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

static int	lcd_init (void *lcdbase);
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
static void	lcd_setfgcolor (int color);
static void	lcd_setbgcolor (int color);

#ifdef	NOT_USED_SO_FAR
static int	lcd_getbgcolor (void);
static void	lcd_disable (void);
static void	lcd_getcolreg (ushort regno,
				ushort *red, ushort *green, ushort *blue);
static int	lcd_getfgcolor (void);
#endif	/* NOT_USED_SO_FAR */


static int pxafb_init_mem(void *lcdbase, vidinfo_t *vid);
static void pxafb_setup_gpio(vidinfo_t *vid);
static void pxafb_enable_controller(vidinfo_t *vid);
static int pxafb_init(vidinfo_t *vid);

/************************************************************************/

/*----------------------------------------------------------------------*/

static void console_scrollup (void)
{
	/* Copy up rows ignoring the first one */
	memcpy (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND, CONSOLE_SCROLL_SIZE);

	/* Clear the last one */
	memset (CONSOLE_ROW_LAST, COLOR_MASK(lcd_color_bg), CONSOLE_ROW_SIZE);
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
#elif LCD_BPP == LCD_COLOR16
			for (c=0; c<16; ++c) {
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

static int lcd_init (void *lcdbase)
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
	lcd_setcolreg  (CONSOLE_COLOR_BLUE,	0,    0, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_MAGENTA,	0xFF,    0, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_CYAN,	   0, 0xFF, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_GREY,	0xAA, 0xAA, 0xAA);
	lcd_setcolreg  (CONSOLE_COLOR_WHITE,	0xFF, 0xFF, 0xFF);
#endif

#define CFG_WHITE_ON_BLACK
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

	/* extra page for dma descriptors and palette */
	size = line_length * panel_info.vl_row + PAGE_SIZE;

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
   	pxafb_init_mem(lcdbase, &panel_info);
	pxafb_init(&panel_info);
	pxafb_setup_gpio(&panel_info);
	pxafb_enable_controller(&panel_info);
}

/*----------------------------------------------------------------------*/

#ifdef	NOT_USED_SO_FAR
static void
lcd_getcolreg (ushort regno, ushort *red, ushort *green, ushort *blue)
{
}
#endif	/* NOT_USED_SO_FAR */

/*----------------------------------------------------------------------*/

#if LCD_BPP == LCD_COLOR8
static void
lcd_setcolreg (ushort regno, ushort red, ushort green, ushort blue)
{
	struct pxafb_info *fbi = &panel_info.pxa;
	unsigned short *palette = (unsigned short *)fbi->palette;
	u_int val;
   	
	if (regno < fbi->palette_size) {
		val = ((red << 8) & 0xf800);
		val |= ((green << 4) & 0x07e0);
		val |= (blue & 0x001f);

#ifdef LCD_INVERT_COLORS
		palette[regno] = ~val;
#else
		palette[regno] = ~val;
#endif
	}

	debug ("setcolreg: reg %2d @ %p: R=%02X G=%02X B=%02X => %04X\n",
		regno, &palette[regno],
		red, green, blue,
		palette[regno]);
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

#ifdef	NOT_USED_SO_FAR
static int lcd_getbgcolor (void)
{
	return lcd_color_bg;
}
#endif	/* NOT_USED_SO_FAR */

/*----------------------------------------------------------------------*/

static void lcd_enable (void)
{
}

/*----------------------------------------------------------------------*/

#ifdef	NOT_USED_SO_FAR
static void lcd_disable (void)
{
}
#endif	/* NOT_USED_SO_FAR */


/************************************************************************/
/* ** Chipset depending Bitmap / Logo stuff...				*/
/************************************************************************/

#ifdef CONFIG_LCD_LOGO
static void bitmap_plot (int x, int y)
{
	ushort *cmap;
	ushort i;
	uchar *bmap;
	uchar *fb;
	struct pxafb_info *fbi = &panel_info.pxa;

	debug ("Logo: width %d  height %d  colors %d  cmap %d\n",
		BMP_LOGO_WIDTH, BMP_LOGO_HEIGHT, BMP_LOGO_COLORS,
		sizeof(bmp_logo_palette)/(sizeof(ushort))
	);

	/* Leave room for default color map */
	cmap = (ushort *)fbi->palette;

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

	for (i=0; i<BMP_LOGO_HEIGHT; ++i) {
		memcpy (fb, bmap, BMP_LOGO_WIDTH);
		bmap += BMP_LOGO_WIDTH;
		fb   += panel_info.vl_col;
	}
}
#endif /* CONFIG_LCD_LOGO */

/*----------------------------------------------------------------------*/

static void *lcd_logo (void)
{
#ifdef CONFIG_LCD_LOGO
	DECLARE_GLOBAL_DATA_PTR;
	char info[80];
	char temp[32];

	bitmap_plot (0, 0);
#endif /* CONFIG_LCD_LOGO */

#if defined(CONFIG_LCD_LOGO) && !defined(LCD_INFO_BELOW_LOGO)
	return ((void *)((ulong)lcd_base + BMP_LOGO_HEIGHT * lcd_line_length));
#else
	return ((void *)lcd_base);
#endif	/* CONFIG_LCD_LOGO */
}

/************************************************************************
   PXA255 specific routines
************************************************************************/

static int pxafb_init_mem(void *lcdbase, vidinfo_t *vid)
{
	u_long palette_mem_size;
	struct pxafb_info *fbi = &vid->pxa;
	int fb_size = vid->vl_row * (vid->vl_col * NBITS (vid->vl_bpix)) / 8;

	fbi->screen = (u_long)lcdbase;

	fbi->palette_size = NBITS(vid->vl_bpix) == 8 ? 256 : 16;
	palette_mem_size = fbi->palette_size * sizeof(u16);
	debug("palette_mem_size = 0x%08lx\n", (u_long) palette_mem_size);
	/* locate palette and descs at end of page following fb */
	fbi->palette = (u_long)lcdbase + fb_size + 2*PAGE_SIZE - palette_mem_size;

	return 0;
}

static void pxafb_setup_gpio(vidinfo_t *vid)
{
	u_long lccr0;

	/*
	 * setup is based on type of panel supported
	 */

	lccr0 = vid->pxa.reg_lccr0;

	/* 4 bit interface */
	if ((lccr0 & LCCR0_CMS) && (lccr0 & LCCR0_SDS) && !(lccr0 & LCCR0_DPD))
	{
		debug("Setting GPIO for 4 bit data\n");
		/* bits 58-61 */
		GPDR1 |= (0xf << 26);
		GAFR1_U = (GAFR1_U & ~(0xff << 20)) | (0xaa << 20);

		/* bits 74-77 */
		GPDR2 |= (0xf << 10);
		GAFR2_L = (GAFR2_L & ~(0xff << 20)) | (0xaa << 20);
	}

	/* 8 bit interface */
	else if (((lccr0 & LCCR0_CMS) && ((lccr0 & LCCR0_SDS) || (lccr0 & LCCR0_DPD))) ||
	    (!(lccr0 & LCCR0_CMS) && !(lccr0 & LCCR0_PAS) && !(lccr0 & LCCR0_SDS)))
	{
		debug("Setting GPIO for 8 bit data\n");
		/* bits 58-65 */
		GPDR1 |= (0x3f << 26);
		GPDR2 |= (0x3);

		GAFR1_U = (GAFR1_U & ~(0xfff << 20)) | (0xaaa << 20);
		GAFR2_L = (GAFR2_L & ~0xf) | (0xa);

		/* bits 74-77 */
		GPDR2 |= (0xf << 10);
		GAFR2_L = (GAFR2_L & ~(0xff << 20)) | (0xaa << 20);
	}

	/* 16 bit interface */
	else if (!(lccr0 & LCCR0_CMS) && ((lccr0 & LCCR0_SDS) || (lccr0 & LCCR0_PAS)))
	{
		debug("Setting GPIO for 16 bit data\n");
		/* bits 58-77 */
		GPDR1 |= (0x3f << 26);
		GPDR2 |= 0x00003fff;

		GAFR1_U = (GAFR1_U & ~(0xfff << 20)) | (0xaaa << 20);
		GAFR2_L = (GAFR2_L & 0xf0000000) | 0x0aaaaaaa;
	}
	else
	{
		printf("pxafb_setup_gpio: unable to determine bits per pixel\n");
	}
}

static void pxafb_enable_controller(vidinfo_t *vid)
{
	debug("Enabling LCD controller\n");

	/* Sequence from 11.7.10 */
	LCCR3  = vid->pxa.reg_lccr3;
	LCCR2  = vid->pxa.reg_lccr2;
	LCCR1  = vid->pxa.reg_lccr1;
	LCCR0  = vid->pxa.reg_lccr0 & ~LCCR0_ENB;
	FDADR0 = vid->pxa.fdadr0;
	FDADR1 = vid->pxa.fdadr1;
	LCCR0 |= LCCR0_ENB;

	CKEN |= CKEN16_LCD;

	debug("FDADR0 = 0x%08x\n", (unsigned int)FDADR0);
	debug("FDADR1 = 0x%08x\n", (unsigned int)FDADR1);
	debug("LCCR0 = 0x%08x\n", (unsigned int)LCCR0);
	debug("LCCR1 = 0x%08x\n", (unsigned int)LCCR1);
	debug("LCCR2 = 0x%08x\n", (unsigned int)LCCR2);
	debug("LCCR3 = 0x%08x\n", (unsigned int)LCCR3);
}

static int pxafb_init(vidinfo_t *vid)
{
	struct pxafb_info *fbi = &vid->pxa;

	debug("Configuring PXA LCD\n");

	fbi->reg_lccr0 = REG_LCCR0;
	fbi->reg_lccr3 = REG_LCCR3;

	debug("vid: vl_col=%d hslen=%d lm=%d rm=%d\n",
		vid->vl_col, vid->vl_hpw,
		vid->vl_blw, vid->vl_elw);
	debug("vid: vl_row=%d vslen=%d um=%d bm=%d\n",
		vid->vl_row, vid->vl_vpw,
		vid->vl_bfw, vid->vl_efw);

	fbi->reg_lccr1 =
		LCCR1_DisWdth(vid->vl_col) +
		LCCR1_HorSnchWdth(vid->vl_hpw) +
		LCCR1_BegLnDel(vid->vl_blw) +
		LCCR1_EndLnDel(vid->vl_elw);
		
	fbi->reg_lccr2 =
		LCCR2_DisHght(vid->vl_row) +
		LCCR2_VrtSnchWdth(vid->vl_vpw) +
		LCCR2_BegFrmDel(vid->vl_bfw) +
		LCCR2_EndFrmDel(vid->vl_efw);

	fbi->reg_lccr3 = REG_LCCR3 & ~(LCCR3_HSP | LCCR3_VSP); 
	fbi->reg_lccr3 |= 
		  (vid->vl_hsp ? LCCR3_HorSnchL : LCCR3_HorSnchH) 
		| (vid->vl_vsp ? LCCR3_VrtSnchL : LCCR3_VrtSnchH);
		

	/* setup dma descriptors */
	fbi->dmadesc_fblow = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 3*16);
	fbi->dmadesc_fbhigh = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 2*16);
	fbi->dmadesc_palette = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 1*16);

	#define BYTES_PER_PANEL	((fbi->reg_lccr0 & LCCR0_SDS) ? \
	      	(vid->vl_col * vid->vl_row * NBITS(vid->vl_bpix) / 8 / 2) : \
	    	(vid->vl_col * vid->vl_row * NBITS(vid->vl_bpix) / 8))
	
	/* populate descriptors */
	fbi->dmadesc_fblow->fdadr = (u_long)fbi->dmadesc_fblow;
	fbi->dmadesc_fblow->fsadr = fbi->screen + BYTES_PER_PANEL;
	fbi->dmadesc_fblow->fidr  = 0;
	fbi->dmadesc_fblow->ldcmd = BYTES_PER_PANEL;

	fbi->fdadr1 = (u_long)fbi->dmadesc_fblow; /* only used in dual-panel mode */
		
	fbi->dmadesc_fbhigh->fsadr = fbi->screen;
	fbi->dmadesc_fbhigh->fidr = 0;
	fbi->dmadesc_fbhigh->ldcmd = BYTES_PER_PANEL;

	fbi->dmadesc_palette->fsadr = fbi->palette;
	fbi->dmadesc_palette->fidr  = 0;
	fbi->dmadesc_palette->ldcmd = (fbi->palette_size * 2) | LDCMD_PAL;

	if( NBITS(vid->vl_bpix) < 12)
	{
		/* assume any mode with <12 bpp is palette driven */
		fbi->dmadesc_palette->fdadr = (u_long)fbi->dmadesc_fbhigh;
		fbi->dmadesc_fbhigh->fdadr = (u_long)fbi->dmadesc_palette;
		/* flips back and forth between pal and fbhigh */
		fbi->fdadr0 = (u_long)fbi->dmadesc_palette; 
	}
	else
	{
		/* palette shouldn't be loaded in true-color mode */
		fbi->dmadesc_fbhigh->fdadr = (u_long)fbi->dmadesc_fbhigh;
		fbi->fdadr0 = (u_long)fbi->dmadesc_fbhigh; /* no pal just fbhigh */
	}

	debug("fbi->dmadesc_fblow = 0x%lx\n", (u_long)fbi->dmadesc_fblow);
	debug("fbi->dmadesc_fbhigh = 0x%lx\n", (u_long)fbi->dmadesc_fbhigh);
	debug("fbi->dmadesc_palette = 0x%lx\n", (u_long)fbi->dmadesc_palette);

	debug("fbi->dmadesc_fblow->fdadr = 0x%lx\n", fbi->dmadesc_fblow->fdadr);
	debug("fbi->dmadesc_fbhigh->fdadr = 0x%lx\n", fbi->dmadesc_fbhigh->fdadr);
	debug("fbi->dmadesc_palette->fdadr = 0x%lx\n", fbi->dmadesc_palette->fdadr);

	debug("fbi->dmadesc_fblow->fsadr = 0x%lx\n", fbi->dmadesc_fblow->fsadr);
	debug("fbi->dmadesc_fbhigh->fsadr = 0x%lx\n", fbi->dmadesc_fbhigh->fsadr);
	debug("fbi->dmadesc_palette->fsadr = 0x%lx\n", fbi->dmadesc_palette->fsadr);

	debug("fbi->dmadesc_fblow->ldcmd = 0x%lx\n", fbi->dmadesc_fblow->ldcmd);
	debug("fbi->dmadesc_fbhigh->ldcmd = 0x%lx\n", fbi->dmadesc_fbhigh->ldcmd);
	debug("fbi->dmadesc_palette->ldcmd = 0x%lx\n", fbi->dmadesc_palette->ldcmd);
	
	return 0;
}

/************************************************************************/
/************************************************************************/

#endif /* CONFIG_LCD */

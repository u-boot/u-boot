/*
 * (C) Copyright 2001
 * ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * Board specific routines for the miniHiPerCam
 *
 * - initialisation (eeprom)
 * - memory controller
 * - serial io initialisation
 * - ethernet io initialisation
 *
 * -----------------------------------------------------------------
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <linux/ctype.h>
#include <commproc.h>
#include "mpc8xx.h"
#include <video_fb.h>

/* imports from common/main.c */
extern char console_buffer[CFG_CBSIZE];

extern void eeprom_init (void);
extern int eeprom_read (unsigned dev_addr, unsigned offset,
			unsigned char *buffer, unsigned cnt);
extern int eeprom_write (unsigned dev_addr, unsigned offset,
			 unsigned char *buffer, unsigned cnt);

/* globals */
void *video_hw_init (void);
void video_set_lut (unsigned int index,	/* color number */
		    unsigned char r,	/* red */
		    unsigned char g,	/* green */
		    unsigned char b	/* blue */
	);

GraphicDevice gdev;

/* locals */
static void video_circle (char *center, int radius, int color, int pitch);
static void video_test_image (void);
static void video_default_lut (unsigned int clut_type);

/* revision info foer MHPC EEPROM offset 480 */
typedef struct {
	char board[12];		/* 000 - Board Revision information */
	char sensor;		/* 012 - Sensor Type information */
	char serial[8];		/* 013 - Board serial number */
	char etheraddr[6];	/* 021 - Ethernet node addresse */
	char revision[2];	/* 027 - Revision code */
	char option[3];		/* 029 - resevered for options */
} revinfo;

/* ------------------------------------------------------------------------- */

static const unsigned int sdram_table[] = {
	/* read single beat cycle */
	0xef0efc04, 0x0e2dac04, 0x01ba5c04, 0x1ff5fc00,
	0xfffffc05, 0xeffafc34, 0x0ff0bc34, 0x1ff57c35,

	/* read burst cycle */
	0xef0efc04, 0x0e3dac04, 0x10ff5c04, 0xf0fffc00,
	0xf0fffc00, 0xf1fffc00, 0xfffffc00, 0xfffffc05,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* write single beat cycle */
	0xef0efc04, 0x0e29ac00, 0x01b25c04, 0x1ff5fc05,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* write burst cycle */
	0xef0ef804, 0x0e39a000, 0x10f75000, 0xf0fff440,
	0xf0fffc40, 0xf1fffc04, 0xfffffc05, 0xfffffc04,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* periodic timer expired */
	0xeffebc84, 0x1ffd7c04, 0xfffffc04, 0xfffffc84,
	0xeffebc04, 0x1ffd7c04, 0xfffffc04, 0xfffffc05,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* exception */
	0xfffffc04, 0xfffffc05, 0xfffffc04, 0xfffffc04
};

/* ------------------------------------------------------------------------- */

int board_early_init_f (void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(im->im_cpm);
	volatile iop8xx_t *ip = (iop8xx_t *) & (im->im_ioport);

	/* reset the port A s.a. cpm-routines */
	ip->iop_padat = 0x0000;
	ip->iop_papar = 0x0000;
	ip->iop_padir = 0x0800;
	ip->iop_paodr = 0x0000;

	/* reset the port B for digital and LCD output */
	cp->cp_pbdat = 0x0300;
	cp->cp_pbpar = 0x5001;
	cp->cp_pbdir = 0x5301;
	cp->cp_pbodr = 0x0000;

	/* reset the port C configured for SMC1 serial port and aqc. control */
	ip->iop_pcdat = 0x0800;
	ip->iop_pcpar = 0x0000;
	ip->iop_pcdir = 0x0e30;
	ip->iop_pcso = 0x0000;

	/* Config port D for LCD output */
	ip->iop_pdpar = 0x1fff;
	ip->iop_pddir = 0x1fff;

	return (0);
}

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity
 */
int checkboard (void)
{
	puts ("Board: ELTEC miniHiperCam\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

int misc_init_r (void)
{
	revinfo mhpcRevInfo;
	char nid[32];
	char *mhpcSensorTypes[] = { "OMNIVISON OV7610/7620 color",
		"OMNIVISON OV7110 b&w", NULL
	};
	char hex[23] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0,
		0, 0, 0, 0, 10, 11, 12, 13, 14, 15
	};
	int i;

	/* check revision data */
	eeprom_read (CFG_I2C_EEPROM_ADDR, 480, (uchar *) &mhpcRevInfo, 32);

	if (strncmp ((char *) &mhpcRevInfo.board[2], "MHPC", 4) != 0) {
		printf ("Enter revision number (0-9): %c  ",
			mhpcRevInfo.revision[0]);
		if (0 != readline (NULL)) {
			mhpcRevInfo.revision[0] =
				(char) toupper (console_buffer[0]);
		}

		printf ("Enter revision character (A-Z): %c  ",
			mhpcRevInfo.revision[1]);
		if (1 == readline (NULL)) {
			mhpcRevInfo.revision[1] =
				(char) toupper (console_buffer[0]);
		}

		printf ("Enter board name (V-XXXX-XXXX): %s  ",
			(char *) &mhpcRevInfo.board);
		if (11 == readline (NULL)) {
			for (i = 0; i < 11; i++) {
				mhpcRevInfo.board[i] =
					(char) toupper (console_buffer[i]);
				mhpcRevInfo.board[11] = '\0';
			}
		}

		printf ("Supported sensor types:\n");
		i = 0;
		do {
			printf ("\n    \'%d\' : %s\n", i, mhpcSensorTypes[i]);
		} while (mhpcSensorTypes[++i] != NULL);

		do {
			printf ("\nEnter sensor number (0-255): %d  ",
				(int) mhpcRevInfo.sensor);
			if (0 != readline (NULL)) {
				mhpcRevInfo.sensor =
					(unsigned char)
					simple_strtoul (console_buffer, NULL,
							10);
			}
		} while (mhpcRevInfo.sensor >= i);

		printf ("Enter serial number: %s ",
			(char *) &mhpcRevInfo.serial);
		if (6 == readline (NULL)) {
			for (i = 0; i < 6; i++) {
				mhpcRevInfo.serial[i] = console_buffer[i];
			}
			mhpcRevInfo.serial[6] = '\0';
		}

		printf ("Enter ether node ID with leading zero (HEX): %02x%02x%02x%02x%02x%02x  ", mhpcRevInfo.etheraddr[0], mhpcRevInfo.etheraddr[1], mhpcRevInfo.etheraddr[2], mhpcRevInfo.etheraddr[3], mhpcRevInfo.etheraddr[4], mhpcRevInfo.etheraddr[5]);
		if (12 == readline (NULL)) {
			for (i = 0; i < 12; i += 2) {
				mhpcRevInfo.etheraddr[i >> 1] =
					(char) (16 *
						hex[toupper
						    (console_buffer[i]) -
						    '0'] +
						hex[toupper
						    (console_buffer[i + 1]) -
						    '0']);
			}
		}

		/* setup new revision data */
		eeprom_write (CFG_I2C_EEPROM_ADDR, 480, (uchar *) &mhpcRevInfo,
			      32);
	}

	/* set environment */
	sprintf (nid, "%02x:%02x:%02x:%02x:%02x:%02x",
		 mhpcRevInfo.etheraddr[0], mhpcRevInfo.etheraddr[1],
		 mhpcRevInfo.etheraddr[2], mhpcRevInfo.etheraddr[3],
		 mhpcRevInfo.etheraddr[4], mhpcRevInfo.etheraddr[5]);
	setenv ("ethaddr", nid);

	/* print actual board identification */
	printf ("Ident: %s %s Ser %s Rev %c%c\n",
		mhpcRevInfo.board,
		(mhpcRevInfo.sensor == 0 ? "color" : "b&w"),
		(char *) &mhpcRevInfo.serial, mhpcRevInfo.revision[0],
		mhpcRevInfo.revision[1]);

	return (0);
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	memctl->memc_mamr = CFG_MAMR & (~(MAMR_PTAE));	/* no refresh yet */
	memctl->memc_mbmr = MBMR_GPL_B4DIS;	/* should this be mamr? - NTL */
	memctl->memc_mptpr = MPTPR_PTP_DIV64;
	memctl->memc_mar = 0x00008800;

	/*
	 * Map controller SDRAM bank 0
	 */
	memctl->memc_or1 = CFG_OR1_PRELIM;
	memctl->memc_br1 = CFG_BR1_PRELIM;
	udelay (200);

	/*
	 * Map controller SDRAM bank 1
	 */
	memctl->memc_or2 = CFG_OR2;
	memctl->memc_br2 = CFG_BR2;

	/*
	 * Perform SDRAM initializsation sequence
	 */
	memctl->memc_mcr = 0x80002105;	/* SDRAM bank 0 */
	udelay (1);
	memctl->memc_mcr = 0x80002730;	/* SDRAM bank 0 - execute twice */
	udelay (1);
	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (10000);

	/* leave place for framebuffers */
	return (SDRAM_MAX_SIZE - SDRAM_RES_SIZE);
}

/* ------------------------------------------------------------------------- */

static void video_circle (char *center, int radius, int color, int pitch)
{
	int x, y, d, dE, dSE;

	x = 0;
	y = radius;
	d = 1 - radius;
	dE = 3;
	dSE = -2 * radius + 5;

	*(center + x + y * pitch) = color;
	*(center + y + x * pitch) = color;
	*(center + y - x * pitch) = color;
	*(center + x - y * pitch) = color;
	*(center - x - y * pitch) = color;
	*(center - y - x * pitch) = color;
	*(center - y + x * pitch) = color;
	*(center - x + y * pitch) = color;
	while (y > x) {
		if (d < 0) {
			d += dE;
			dE += 2;
			dSE += 2;
			x++;
		} else {
			d += dSE;
			dE += 2;
			dSE += 4;
			x++;
			y--;
		}
		*(center + x + y * pitch) = color;
		*(center + y + x * pitch) = color;
		*(center + y - x * pitch) = color;
		*(center + x - y * pitch) = color;
		*(center - x - y * pitch) = color;
		*(center - y - x * pitch) = color;
		*(center - y + x * pitch) = color;
		*(center - x + y * pitch) = color;
	}
}

/* ------------------------------------------------------------------------- */

static void video_test_image (void)
{
	char *di;
	int i, n;

	/* draw raster */
	for (i = 0; i < LCD_VIDEO_ROWS; i += 32) {
		memset ((char *) (LCD_VIDEO_ADDR + i * LCD_VIDEO_COLS),
			LCD_VIDEO_FG, LCD_VIDEO_COLS);
		for (n = i + 1; n < i + 32; n++)
			memset ((char *) (LCD_VIDEO_ADDR +
					  n * LCD_VIDEO_COLS), LCD_VIDEO_BG,
				LCD_VIDEO_COLS);
	}

	for (i = 0; i < LCD_VIDEO_COLS; i += 32) {
		for (n = 0; n < LCD_VIDEO_ROWS; n++)
			*(char *) (LCD_VIDEO_ADDR + n * LCD_VIDEO_COLS + i) =
				LCD_VIDEO_FG;
	}

	/* draw gray bar */
	di = (char *) (LCD_VIDEO_ADDR + (LCD_VIDEO_COLS - 256) / 64 * 32 +
		       97 * LCD_VIDEO_COLS);
	for (n = 0; n < 63; n++) {
		for (i = 0; i < 256; i++) {
			*di++ = (char) i;
			*(di + LCD_VIDEO_COLS * 64) = (i & 1) * 255;
		}
		di += LCD_VIDEO_COLS - 256;
	}

	video_circle ((char *) LCD_VIDEO_ADDR + LCD_VIDEO_COLS / 2 +
		      LCD_VIDEO_ROWS / 2 * LCD_VIDEO_COLS, LCD_VIDEO_ROWS / 2,
		      LCD_VIDEO_FG, LCD_VIDEO_COLS);
}

/* ------------------------------------------------------------------------- */

static void video_default_lut (unsigned int clut_type)
{
	unsigned int i;
	unsigned char RGB[] = {
		0x00, 0x00, 0x00,	/* black */
		0x80, 0x80, 0x80,	/* gray */
		0xff, 0x00, 0x00,	/* red */
		0x00, 0xff, 0x00,	/* green */
		0x00, 0x00, 0xff,	/* blue */
		0x00, 0xff, 0xff,	/* cyan */
		0xff, 0x00, 0xff,	/* magenta */
		0xff, 0xff, 0x00,	/* yellow */
		0x80, 0x00, 0x00,	/* dark red */
		0x00, 0x80, 0x00,	/* dark green */
		0x00, 0x00, 0x80,	/* dark blue */
		0x00, 0x80, 0x80,	/* dark cyan */
		0x80, 0x00, 0x80,	/* dark magenta */
		0x80, 0x80, 0x00,	/* dark yellow */
		0xc0, 0xc0, 0xc0,	/* light gray */
		0xff, 0xff, 0xff,	/* white */
	};

	switch (clut_type) {
	case 1:
		for (i = 0; i < 240; i++)
			video_set_lut (i, i, i, i);
		for (i = 0; i < 16; i++)
			video_set_lut (i + 240, RGB[i * 3], RGB[i * 3 + 1],
				       RGB[i * 3 + 2]);
		break;
	default:
		for (i = 0; i < 256; i++)
			video_set_lut (i, i, i, i);
	}
}

/* ------------------------------------------------------------------------- */

void *video_hw_init (void)
{
	unsigned int clut = 0;
	unsigned char *penv;
	immap_t *immr = (immap_t *) CFG_IMMR;

	/* enable video only on CLUT value */
	if ((penv = (uchar *)getenv ("clut")) != NULL)
		clut = (u_int) simple_strtoul ((char *)penv, NULL, 10);
	else
		return NULL;

	/* disable graphic before write LCD regs. */
	immr->im_lcd.lcd_lccr = 0x96000866;

	/* config LCD regs. */
	immr->im_lcd.lcd_lcfaa = LCD_VIDEO_ADDR;
	immr->im_lcd.lcd_lchcr = 0x010a0093;
	immr->im_lcd.lcd_lcvcr = 0x900f0024;

	printf ("Video: 640x480 8Bit Index Lut %s\n",
		(clut == 1 ? "240/16 (gray/vga)" : "256(gray)"));

	video_default_lut (clut);

	/* clear framebuffer */
	memset ((char *) (LCD_VIDEO_ADDR), LCD_VIDEO_BG,
		LCD_VIDEO_ROWS * LCD_VIDEO_COLS);

	/* enable graphic */
	immr->im_lcd.lcd_lccr = 0x96000867;

	/* fill in Graphic Device */
	gdev.frameAdrs = LCD_VIDEO_ADDR;
	gdev.winSizeX = LCD_VIDEO_COLS;
	gdev.winSizeY = LCD_VIDEO_ROWS;
	gdev.gdfBytesPP = 1;
	gdev.gdfIndex = GDF__8BIT_INDEX;

	if (clut > 1)
		/* return Graphic Device for console */
		return (void *) &gdev;
	else
		/* just graphic enabled - draw something beautiful */
		video_test_image ();

	return NULL;		/* this disabels cfb - console */
}

/* ------------------------------------------------------------------------- */

void video_set_lut (unsigned int index,
		    unsigned char r, unsigned char g, unsigned char b)
{
	unsigned int lum;
	unsigned short *pLut = (unsigned short *) (CFG_IMMR + 0x0e00);

	/* 16 bit lut values, 12 bit used, xxxx BBGG RRii iiii */
	/* y = 0.299*R + 0.587*G + 0.114*B */
	lum = (2990 * r + 5870 * g + 1140 * b) / 10000;
	pLut[index] =
		((b & 0xc0) << 4) | ((g & 0xc0) << 2) | (r & 0xc0) | (lum &
								      0x3f);
}

/* ------------------------------------------------------------------------- */

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

/* #define DEBUG */

#include <config.h>
#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <version.h>
#include <stdarg.h>
#include <lcdvideo.h>
#include <linux/types.h>
#include <stdio_dev.h>
#if defined(CONFIG_POST)
#include <post.h>
#endif
#include <lcd.h>

#ifdef CONFIG_LCD

/************************************************************************/
/* ** CONFIG STUFF -- should be moved to board config file		*/
/************************************************************************/
#ifndef CONFIG_LCD_INFO
#define CONFIG_LCD_INFO		/* Display Logo, (C) and system info	*/
#endif

#if defined(CONFIG_V37) || defined(CONFIG_EDT32F10)
#undef CONFIG_LCD_LOGO
#undef CONFIG_LCD_INFO
#endif

/*----------------------------------------------------------------------*/
#ifdef CONFIG_KYOCERA_KCS057QV1AJ
/*
 *  Kyocera KCS057QV1AJ-G23. Passive, color, single scan.
 */
#define LCD_BPP	LCD_COLOR4

vidinfo_t panel_info = {
    640, 480, 132, 99, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH,
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
vidinfo_t panel_info = {
    640, 480, 154, 116, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH,
    LCD_COLOR8, 1, 0, 1, 0, 0, 0, 0, 0
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_HITACHI_SP19X001_Z1A */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
#ifdef CONFIG_NEC_NL6448AC33
/*
 *  NEC NL6448AC33-18. Active, color, single scan.
 */
vidinfo_t panel_info = {
    640, 480, 132, 99, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_HIGH,
    3, 0, 0, 1, 1, 144, 2, 0, 33
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_NEC_NL6448AC33 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_NEC_NL6448BC20
/*
 *  NEC NL6448BC20-08.  6.5", 640x480. Active, color, single scan.
 */
vidinfo_t panel_info = {
    640, 480, 132, 99, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_HIGH,
    3, 0, 0, 1, 1, 144, 2, 0, 33
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_NEC_NL6448BC20 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_NEC_NL6448BC33_54
/*
 *  NEC NL6448BC33-54. 10.4", 640x480. Active, color, single scan.
 */
vidinfo_t panel_info = {
    640, 480, 212, 158, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_HIGH,
    3, 0, 0, 1, 1, 144, 2, 0, 33
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_NEC_NL6448BC33_54 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_SHARP_LQ104V7DS01
/*
 *  SHARP LQ104V7DS01. 6.5", 640x480. Active, color, single scan.
 */
vidinfo_t panel_info = {
    640, 480, 132, 99, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_LOW,
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
vidinfo_t panel_info = {
    320, 240, 0, 0, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH,
    3, 0, 0, 1, 1, 15, 4, 0, 3
};
#endif /* CONFIG_SHARP_16x9 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_SHARP_LQ057Q3DC02
/*
 * Sharp LQ057Q3DC02 display. Active, color, single scan.
 */
#undef LCD_DF
#define LCD_DF 12

vidinfo_t panel_info = {
    320, 240, 0, 0, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_HIGH,
    3, 0, 0, 1, 1, 15, 4, 0, 3
		/* wbl, vpw, lcdac, wbf */
};
#define CONFIG_LCD_INFO_BELOW_LOGO
#endif /* CONFIG_SHARP_LQ057Q3DC02 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_SHARP_LQ64D341
/*
 * Sharp LQ64D341 display, 640x480. Active, color, single scan.
 */
vidinfo_t panel_info = {
    640, 480, 0, 0, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_HIGH,
    3, 0, 0, 1, 1, 128, 16, 0, 32
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_SHARP_LQ64D341 */

#ifdef CONFIG_SHARP_LQ065T9DR51U
/*
 * Sharp LQ065T9DR51U display, 400x240. Active, color, single scan.
 */
vidinfo_t panel_info = {
    400, 240, 143, 79, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH,
    3, 0, 0, 1, 1, 248, 4, 0, 35
		/* wbl, vpw, lcdac, wbf */
};
#define CONFIG_LCD_INFO_BELOW_LOGO
#endif /* CONFIG_SHARP_LQ065T9DR51U */

#ifdef CONFIG_SHARP_LQ084V1DG21
/*
 * Sharp LQ084V1DG21 display, 640x480. Active, color, single scan.
 */
vidinfo_t panel_info = {
    640, 480, 171, 129, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_LOW,
    3, 0, 0, 1, 1, 160, 3, 0, 48
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_SHARP_LQ084V1DG21 */

/*----------------------------------------------------------------------*/

#ifdef CONFIG_HLD1045
/*
 * HLD1045 display, 640x480. Active, color, single scan.
 */
vidinfo_t panel_info = {
    640, 480, 0, 0, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_HIGH,
    3, 0, 0, 1, 1, 160, 3, 0, 48
		/* wbl, vpw, lcdac, wbf */
};
#endif /* CONFIG_HLD1045 */
/*----------------------------------------------------------------------*/

#ifdef CONFIG_PRIMEVIEW_V16C6448AC
/*
 * Prime View V16C6448AC
 */
vidinfo_t panel_info = {
    640, 480, 130, 98, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW, CONFIG_SYS_LOW, CONFIG_SYS_HIGH,
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
vidinfo_t panel_info = {
    320, 240, 0, 0, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW,
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

vidinfo_t panel_info = {
    320, 240, 0, 0, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_HIGH, CONFIG_SYS_LOW,
    LCD_BPP,  0, 0, 0, 0, 33, 0, 0, 0
};
#endif
/*----------------------------------------------------------------------*/


int lcd_line_length;

int lcd_color_fg;
int lcd_color_bg;

/*
 * Frame buffer memory information
 */
void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;	/* Start of console buffer	*/

short console_col;
short console_row;

/************************************************************************/

void lcd_ctrl_init (void *lcdbase);
void lcd_enable (void);
#if LCD_BPP == LCD_COLOR8
void lcd_setcolreg (ushort regno,
				ushort red, ushort green, ushort blue);
#endif
#if LCD_BPP == LCD_MONOCHROME
void lcd_initcolregs (void);
#endif

#if defined(CONFIG_RBC823)
void lcd_disable (void);
#endif

/************************************************************************/

/************************************************************************/
/* ----------------- chipset specific functions ----------------------- */
/************************************************************************/

/*
 * Calculate fb size for VIDEOLFB_ATAG.
 */
ulong calc_fbsize (void)
{
	ulong size;
	int line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;

	size = line_length * panel_info.vl_row;

	return size;
}

void lcd_ctrl_init (void *lcdbase)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile lcd823_t *lcdp = &immr->im_lcd;

	uint lccrtmp;
	uint lchcr_hpc_tmp;

	/* Initialize the LCD control register according to the LCD
	 * parameters defined.  We do everything here but enable
	 * the controller.
	 */

#ifdef CONFIG_RPXLITE
	/* This is special for RPXlite_DW Software Development Platform **[Sam]** */
	panel_info.vl_dp = CONFIG_SYS_LOW;
#endif

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
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	unsigned short colreg, *cmap_ptr;

	cmap_ptr = (unsigned short *)&cp->lcd_cmap[regno * 2];

	colreg = *cmap_ptr;
#ifdef	CONFIG_SYS_INVERT_COLORS
	colreg ^= 0x0FFF;
#endif

	*red   = (colreg >> 8) & 0x0F;
	*green = (colreg >> 4) & 0x0F;
	*blue  =  colreg       & 0x0F;
}
#endif	/* NOT_USED_SO_FAR */

/*----------------------------------------------------------------------*/

#if LCD_BPP == LCD_COLOR8
void
lcd_setcolreg (ushort regno, ushort red, ushort green, ushort blue)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	unsigned short colreg, *cmap_ptr;

	cmap_ptr = (unsigned short *)&cp->lcd_cmap[regno * 2];

	colreg = ((red   & 0x0F) << 8) |
		 ((green & 0x0F) << 4) |
		  (blue  & 0x0F) ;
#ifdef	CONFIG_SYS_INVERT_COLORS
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
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	ushort regno;

	for (regno = 0; regno < 16; regno++) {
		cp->lcd_cmap[regno * 2] = 0;
		cp->lcd_cmap[(regno * 2) + 1] = regno & 0x0f;
	}
}
#endif

/*----------------------------------------------------------------------*/

void lcd_enable (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
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
#if defined(CONFIG_LCD) && defined(CONFIG_LWMON) && (CONFIG_POST & CONFIG_SYS_POST_SYSMON)
	/* Enable LCD later in sysmon test, only if temperature is OK */
#else
	c |= 0x07;	/* Power on CCFL, Enable CCFL, Chip Enable LCD */
#endif
	pic_write (0x60, c);
    }
#endif /* CONFIG_LWMON */

#if defined(CONFIG_R360MPI)
    {
	extern void r360_i2c_lcd_write (uchar data0, uchar data1);
	unsigned long bgi, ctr;
	char *p;

	if ((p = getenv("lcdbgi")) != NULL) {
		bgi = simple_strtoul (p, 0, 10) & 0xFFF;
	} else {
		bgi = 0xFFF;
	}

	if ((p = getenv("lcdctr")) != NULL) {
		ctr = simple_strtoul (p, 0, 10) & 0xFFF;
	} else {
		ctr=0x7FF;
	}

	r360_i2c_lcd_write(0x10, 0x01);
	r360_i2c_lcd_write(0x20, 0x01);
	r360_i2c_lcd_write(0x30 | ((bgi>>8) & 0xF), bgi & 0xFF);
	r360_i2c_lcd_write(0x40 | ((ctr>>8) & 0xF), ctr & 0xFF);
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
#ifdef CONFIG_RRVISION
	debug ("PC4->Output(1): enable LVDS\n");
	debug ("PC5->Output(0): disable PAL clock\n");
	immr->im_ioport.iop_pddir |=  0x1000;
	immr->im_ioport.iop_pcpar &= ~(0x0C00);
	immr->im_ioport.iop_pcdir |=   0x0C00 ;
	immr->im_ioport.iop_pcdat |=   0x0800 ;
	immr->im_ioport.iop_pcdat &= ~(0x0400);
	debug ("PDPAR=0x%04X PDDIR=0x%04X PDDAT=0x%04X\n",
	       immr->im_ioport.iop_pdpar,
	       immr->im_ioport.iop_pddir,
	       immr->im_ioport.iop_pddat);
	debug ("PCPAR=0x%04X PCDIR=0x%04X PCDAT=0x%04X\n",
	       immr->im_ioport.iop_pcpar,
	       immr->im_ioport.iop_pcdir,
	       immr->im_ioport.iop_pcdat);
#endif
}

/*----------------------------------------------------------------------*/

#if defined (CONFIG_RBC823)
void lcd_disable (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
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

#endif /* CONFIG_LCD */

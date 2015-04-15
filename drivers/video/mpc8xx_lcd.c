/*
 * (C) Copyright 2001-2002
 * Wolfgang Denk, DENX Software Engineering -- wd@denx.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/************************************************************************/
/* ** HEADER FILES							*/
/************************************************************************/

/* #define DEBUG */

#include <config.h>
#include <common.h>
#include <command.h>
#include <watchdog.h>
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

	/* Enable LCD on port D.
	 */
	immr->im_ioport.iop_pdpar |= 0x1FFF;
	immr->im_ioport.iop_pddir |= 0x1FFF;

	/* Enable LCD_A/B/C on port B.
	 */
	immr->im_cpm.cp_pbpar |= 0x00005001;
	immr->im_cpm.cp_pbdir |= 0x00005001;

	/* Load the physical address of the linear frame buffer
	 * into the LCD controller.
	 * BIG NOTE:  This has to be modified to load A and B depending
	 * upon the split mode of the LCD.
	 */
	lcdp->lcd_lcfaa = (ulong)lcdbase;
	lcdp->lcd_lcfba = (ulong)lcdbase;

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

	*cmap_ptr = colreg;

	debug ("setcolreg: reg %2d @ %p: R=%02X G=%02X B=%02X => %02X%02X\n",
		regno, &(cp->lcd_cmap[regno * 2]),
		red, green, blue,
		cp->lcd_cmap[ regno * 2 ], cp->lcd_cmap[(regno * 2) + 1]);
}
#endif	/* LCD_COLOR8 */

/*----------------------------------------------------------------------*/

ushort *configuration_get_cmap(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	cpm8xx_t *cp = &(immr->im_cpm);
	return (ushort *)&(cp->lcd_cmap[255 * sizeof(ushort)]);
}

#if defined(CONFIG_MPC823)
void fb_put_byte(uchar **fb, uchar **from)
{
	*(*fb)++ = (255 - *(*from)++);
}
#endif

#ifdef CONFIG_LCD_LOGO
#include <bmp_logo.h>
void lcd_logo_set_cmap(void)
{
	int i;
	ushort *cmap;
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	cpm8xx_t *cp = &(immr->im_cpm);
	cmap = (ushort *)&(cp->lcd_cmap[BMP_LOGO_OFFSET * sizeof(ushort)]);

	for (i = 0; i < BMP_LOGO_COLORS; ++i)
		*cmap++ = bmp_logo_palette[i];
}
#endif

void lcd_enable (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile lcd823_t *lcdp = &immr->im_lcd;

	/* Enable the LCD panel */
	immr->im_siu_conf.sc_sdcr |= (1 << (31 - 25));		/* LAM = 1 */
	lcdp->lcd_lccr |= LCCR_PON;
}

/************************************************************************/

#endif /* CONFIG_LCD */

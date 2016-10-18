/* ported from ctfb.c (linux kernel):
 * Created in Jan - July 2000 by Thomas Höhenleitner <th@visuelle-maschinen.de>
 *
 * Ported to U-Boot:
 * (C) Copyright 2002 Denis Peter, MPL AG Switzerland
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#ifdef CONFIG_VIDEO

#include <pci.h>
#include <video_fb.h>
#include "videomodes.h"

/* debug */
#undef VGA_DEBUG
#undef VGA_DUMP_REG
#ifdef VGA_DEBUG
#undef _DEBUG
#define _DEBUG  1
#else
#undef _DEBUG
#define _DEBUG  0
#endif

/* Macros */
#ifndef min
#define min( a, b ) ( ( a ) < ( b ) ) ? ( a ) : ( b )
#endif
#ifndef max
#define max( a, b ) ( ( a ) > ( b ) ) ? ( a ) : ( b )
#endif
#ifdef minmax
#error "term minmax already used."
#endif
#define minmax( a, x, b ) max( ( a ), min( ( x ), ( b ) ) )
#define N_ELTS( x ) ( sizeof( x ) / sizeof( x[ 0 ] ) )

/* CT Register Offsets */
#define CT_AR_O			0x3c0	/* Index and Data write port of the attribute Registers */
#define CT_GR_O			0x3ce	/* Index port of the Graphic Controller Registers */
#define CT_SR_O			0x3c4	/* Index port of the Sequencer Controller */
#define CT_CR_O			0x3d4	/* Index port of the CRT Controller */
#define CT_XR_O			0x3d6	/* Extended Register index */
#define CT_MSR_W_O		0x3c2	/* Misc. Output Register (write only) */
#define CT_LUT_MASK_O		0x3c6	/* Color Palette Mask */
#define CT_LUT_START_O		0x3c8	/* Color Palette Write Mode Index */
#define CT_LUT_RGB_O		0x3c9	/* Color Palette Data Port */
#define CT_STATUS_REG0_O	0x3c2	/* Status Register 0 (read only) */
#define CT_STATUS_REG1_O	0x3da	/* Input Status Register 1 (read only) */

#define CT_FP_O			0x3d0	/* Index port of the Flat panel Registers */
#define CT_MR_O			0x3d2	/* Index Port of the Multimedia Extension */

/* defines for the memory mapped registers */
#define BR00_o		0x400000	/* Source and Destination Span Register */
#define BR01_o		0x400004	/* Pattern/Source Expansion Background Color & Transparency Key Register */
#define BR02_o		0x400008	/* Pattern/Source Expansion Foreground Color Register */
#define BR03_o		0x40000C	/* Monochrome Source Control Register */
#define BR04_o		0x400010	/* BitBLT Control Register */
#define BR05_o		0x400014	/* Pattern Address Registe */
#define BR06_o		0x400018	/* Source Address Register */
#define BR07_o		0x40001C	/* Destination Address Register */
#define BR08_o		0x400020	/* Destination Width & Height Register */
#define BR09_o		0x400024	/* Source Expansion Background Color & Transparency Key Register */
#define BR0A_o		0x400028	/* Source Expansion Foreground Color Register */

#define CURSOR_SIZE	0x1000	/* in KByte for HW Cursor */
#define PATTERN_ADR	(pGD->dprBase + CURSOR_SIZE)	/* pattern Memory after Cursor Memory */
#define PATTERN_SIZE	8*8*4	/* 4 Bytes per Pixel 8 x 8 Pixel */
#define ACCELMEMORY	(CURSOR_SIZE + PATTERN_SIZE)	/* reserved Memory for BITBlt and hw cursor */

/* Some Mode definitions */
#define FB_SYNC_HOR_HIGH_ACT	1	/* horizontal sync high active  */
#define FB_SYNC_VERT_HIGH_ACT	2	/* vertical sync high active    */
#define FB_SYNC_EXT		4	/* external sync                */
#define FB_SYNC_COMP_HIGH_ACT	8	/* composite sync high active   */
#define FB_SYNC_BROADCAST	16	/* broadcast video timings      */
					/* vtotal = 144d/288n/576i => PAL  */
					/* vtotal = 121d/242n/484i => NTSC */
#define FB_SYNC_ON_GREEN	32	/* sync on green */

#define FB_VMODE_NONINTERLACED  0	/* non interlaced */
#define FB_VMODE_INTERLACED	1	/* interlaced   */
#define FB_VMODE_DOUBLE		2	/* double scan */
#define FB_VMODE_MASK		255

#define FB_VMODE_YWRAP		256	/* ywrap instead of panning     */
#define FB_VMODE_SMOOTH_XPAN	512	/* smooth xpan possible (internally used) */
#define FB_VMODE_CONUPDATE	512	/* don't update x/yoffset       */

#define text			0
#define fntwidth		8

/* table for VGA Initialization  */
typedef struct {
	const unsigned char reg;
	const unsigned char val;
} CT_CFG_TABLE;

/* this table provides some basic initialisations such as Memory Clock etc */
static CT_CFG_TABLE xreg[] = {
	{0x09, 0x01},		/* CRT Controller Extensions Enable */
	{0x0A, 0x02},		/* Frame Buffer Mapping */
	{0x0B, 0x01},		/* PCI Write Burst support */
	{0x20, 0x00},		/* BitBLT Configuration */
	{0x40, 0x03},		/* Memory Access Control */
	{0x60, 0x00},		/* Video Pin Control */
	{0x61, 0x00},		/* DPMS Synch control */
	{0x62, 0x00},		/* GPIO Pin Control */
	{0x63, 0xBD},		/* GPIO Pin Data */
	{0x67, 0x00},		/* Pin Tri-State */
	{0x80, 0x80},		/* Pixel Pipeline Config 0 register */
	{0xA0, 0x00},		/* Cursor 1 Control Reg */
	{0xA1, 0x00},		/* Cursor 1 Vertical Extension Reg */
	{0xA2, 0x00},		/* Cursor 1 Base Address Low */
	{0xA3, 0x00},		/* Cursor 1 Base Address High */
	{0xA4, 0x00},		/* Cursor 1 X-Position Low */
	{0xA5, 0x00},		/* Cursor 1 X-Position High */
	{0xA6, 0x00},		/* Cursor 1 Y-Position Low */
	{0xA7, 0x00},		/* Cursor 1 Y-Position High */
	{0xA8, 0x00},		/* Cursor 2 Control Reg */
	{0xA9, 0x00},		/* Cursor 2 Vertical Extension Reg */
	{0xAA, 0x00},		/* Cursor 2 Base Address Low */
	{0xAB, 0x00},		/* Cursor 2 Base Address High */
	{0xAC, 0x00},		/* Cursor 2 X-Position Low */
	{0xAD, 0x00},		/* Cursor 2 X-Position High */
	{0xAE, 0x00},		/* Cursor 2 Y-Position Low */
	{0xAF, 0x00},		/* Cursor 2 Y-Position High */
	{0xC0, 0x7D},		/* Dot Clock 0 VCO M-Divisor */
	{0xC1, 0x07},		/* Dot Clock 0 VCO N-Divisor */
	{0xC3, 0x34},		/* Dot Clock 0 Divisor select */
	{0xC4, 0x55},		/* Dot Clock 1 VCO M-Divisor */
	{0xC5, 0x09},		/* Dot Clock 1 VCO N-Divisor */
	{0xC7, 0x24},		/* Dot Clock 1 Divisor select */
	{0xC8, 0x7D},		/* Dot Clock 2 VCO M-Divisor */
	{0xC9, 0x07},		/* Dot Clock 2 VCO N-Divisor */
	{0xCB, 0x34},		/* Dot Clock 2 Divisor select */
	{0xCC, 0x38},		/* Memory Clock 0 VCO M-Divisor */
	{0xCD, 0x03},		/* Memory Clock 0 VCO N-Divisor */
	{0xCE, 0x90},		/* Memory Clock 0 Divisor select */
	{0xCF, 0x06},		/* Clock Config */
	{0xD0, 0x0F},		/* Power Down */
	{0xD1, 0x01},		/* Power Down BitBLT */
	{0xFF, 0xFF}		/* end of table */
};
/* Clock Config:
 * =============
 *
 * PD Registers:
 * -------------
 * Bit2 and Bit4..6 are used for the Loop Divisor and Post Divisor.
 * They are encoded as follows:
 *
 * +---+--------------+
 * | 2 | Loop Divisor |
 * +---+--------------+
 * | 1 | 1            |
 * +---+--------------+
 * | 0 | 4            |
 * +---+--------------+
 * Note: The Memory Clock does not have a Loop Divisor.
 * +---+---+---+--------------+
 * | 6 | 5 | 4 | Post Divisor |
 * +---+---+---+--------------+
 * | 0 | 0 | 0 | 1            |
 * +---+---+---+--------------+
 * | 0 | 0 | 1 | 2            |
 * +---+---+---+--------------+
 * | 0 | 1 | 0 | 4            |
 * +---+---+---+--------------+
 * | 0 | 1 | 1 | 8            |
 * +---+---+---+--------------+
 * | 1 | 0 | 0 | 16           |
 * +---+---+---+--------------+
 * | 1 | 0 | 1 | 32           |
 * +---+---+---+--------------+
 * | 1 | 1 | X | reserved     |
 * +---+---+---+--------------+
 *
 * All other bits are reserved in these registers.
 *
 * Clock VCO M Registers:
 * ----------------------
 * These Registers contain the M Value -2.
 *
 * Clock VCO N Registers:
 * ----------------------
 * These Registers contain the N Value -2.
 *
 * Formulas:
 * ---------
 * Fvco = (Fref * Loop Divisor * M/N), whereas 100MHz < Fvco < 220MHz
 * Fout = Fvco / Post Divisor
 *
 * Dot Clk0 (default 25MHz):
 * -------------------------
 * Fvco = 14.318 * 127 / 9 = 202.045MHz
 * Fout = 202.045MHz / 8 = 25.25MHz
 * Post Divisor = 8
 * Loop Divisor = 1
 * XRC0 = (M - 2) = 125 = 0x7D
 * XRC1 = (N - 2) = 7   = 0x07
 * XRC3 =                 0x34
 *
 * Dot Clk1 (default 28MHz):
 * -------------------------
 * Fvco = 14.318 * 87 / 11 = 113.24MHz
 * Fout = 113.24MHz / 4 = 28.31MHz
 * Post Divisor = 4
 * Loop Divisor = 1
 * XRC4 = (M - 2) = 85 = 0x55
 * XRC5 = (N - 2) = 9  = 0x09
 * XRC7 =                0x24
 *
 * Dot Clk2 (variable for extended modes set to 25MHz):
 * ----------------------------------------------------
 * Fvco = 14.318 * 127 / 9 = 202.045MHz
 * Fout = 202.045MHz / 8 = 25.25MHz
 * Post Divisor = 8
 * Loop Divisor = 1
 * XRC8 = (M - 2) = 125 = 0x7D
 * XRC9 = (N - 2) = 7   = 0x07
 * XRCB =                 0x34
 *
 * Memory Clk for most modes >50MHz:
 * ----------------------------------
 * Fvco = 14.318 * 58 / 5 = 166MHz
 * Fout = 166MHz / 2      = 83MHz
 * Post Divisor = 2
 * XRCC = (M - 2) = 57  = 0x38
 * XRCD = (N - 2) = 3   = 0x03
 * XRCE =                 0x90
 *
 * Note Bit7 enables the clock source from the VCO
 *
 */

/*******************************************************************
 * Chips struct
 *******************************************************************/
struct ctfb_chips_properties {
	int device_id;		/* PCI Device ID */
	unsigned long max_mem;	/* memory for frame buffer */
	int vld_set;		/* value of VLD if bit2 in clock control is set */
	int vld_not_set;	/* value of VLD if bit2 in clock control is set */
	int mn_diff;		/* difference between M/N Value + mn_diff = M/N Register */
	int mn_min;		/* min value of M/N Value */
	int mn_max;		/* max value of M/N Value */
	int vco_min;		/* VCO Min in MHz */
	int vco_max;		/* VCO Max in MHz */
};

static const struct ctfb_chips_properties chips[] = {
	{PCI_DEVICE_ID_CT_69000, 0x200000, 1, 4, -2, 3, 257, 100, 220},
	{PCI_DEVICE_ID_CT_65555, 0x100000, 16, 4, 0, 1, 255, 48, 220},	/* NOT TESTED */
	{0, 0, 0, 0, 0, 0, 0, 0, 0}	/* Terminator */
};

/*
 * The Graphic Device
 */
GraphicDevice ctfb;

/*******************************************************************************
*
* Low Level Routines
*/

/*******************************************************************************
*
* Read CT ISA register
*/
#ifdef VGA_DEBUG
static unsigned char
ctRead (unsigned short index)
{
	GraphicDevice *pGD = (GraphicDevice *) & ctfb;
	if (index == CT_AR_O)
		/* synch the Flip Flop */
		in8 (pGD->isaBase + CT_STATUS_REG1_O);

	return (in8 (pGD->isaBase + index));
}
#endif
/*******************************************************************************
*
* Write CT ISA register
*/
static void
ctWrite (unsigned short index, unsigned char val)
{
	GraphicDevice *pGD = (GraphicDevice *) & ctfb;

	out8 ((pGD->isaBase + index), val);
}

/*******************************************************************************
*
* Read CT ISA register indexed
*/
static unsigned char
ctRead_i (unsigned short index, char reg)
{
	GraphicDevice *pGD = (GraphicDevice *) & ctfb;
	if (index == CT_AR_O)
		/* synch the Flip Flop */
		in8 (pGD->isaBase + CT_STATUS_REG1_O);
	out8 ((pGD->isaBase + index), reg);
	return (in8 (pGD->isaBase + index + 1));
}

/*******************************************************************************
*
* Write CT ISA register indexed
*/
static void
ctWrite_i (unsigned short index, char reg, char val)
{
	GraphicDevice *pGD = (GraphicDevice *) & ctfb;
	if (index == CT_AR_O) {
		/* synch the Flip Flop */
		in8 (pGD->isaBase + CT_STATUS_REG1_O);
		out8 ((pGD->isaBase + index), reg);
		out8 ((pGD->isaBase + index), val);
	} else {
		out8 ((pGD->isaBase + index), reg);
		out8 ((pGD->isaBase + index + 1), val);
	}
}

/*******************************************************************************
*
* Write a table of CT ISA register
*/
static void
ctLoadRegs (unsigned short index, CT_CFG_TABLE * regTab)
{
	while (regTab->reg != 0xFF) {
		ctWrite_i (index, regTab->reg, regTab->val);
		regTab++;
	}
}

/*****************************************************************************/
static void
SetArRegs (void)
{
	int i, tmp;

	for (i = 0; i < 0x10; i++)
		ctWrite_i (CT_AR_O, i, i);
	if (text)
		tmp = 0x04;
	else
		tmp = 0x41;

	ctWrite_i (CT_AR_O, 0x10, tmp);	/* Mode Control Register */
	ctWrite_i (CT_AR_O, 0x11, 0x00);	/* Overscan Color Register */
	ctWrite_i (CT_AR_O, 0x12, 0x0f);	/* Memory Plane Enable Register */
	if (fntwidth == 9)
		tmp = 0x08;
	else
		tmp = 0x00;
	ctWrite_i (CT_AR_O, 0x13, tmp);	/* Horizontal Pixel Panning */
	ctWrite_i (CT_AR_O, 0x14, 0x00);	/* Color Select Register    */
	ctWrite (CT_AR_O, 0x20);	/* enable video             */
}

/*****************************************************************************/
static void
SetGrRegs (void)
{				/* Set Graphics Mode */
	int i;

	for (i = 0; i < 0x05; i++)
		ctWrite_i (CT_GR_O, i, 0);
	if (text) {
		ctWrite_i (CT_GR_O, 0x05, 0x10);
		ctWrite_i (CT_GR_O, 0x06, 0x02);
	} else {
		ctWrite_i (CT_GR_O, 0x05, 0x40);
		ctWrite_i (CT_GR_O, 0x06, 0x05);
	}
	ctWrite_i (CT_GR_O, 0x07, 0x0f);
	ctWrite_i (CT_GR_O, 0x08, 0xff);
}

/*****************************************************************************/
static void
SetSrRegs (void)
{
	int tmp = 0;

	ctWrite_i (CT_SR_O, 0x00, 0x00);	/* reset */
	/*rr( sr, 0x01, tmp );
	   if( fntwidth == 8 ) tmp |= 0x01; else tmp &= ~0x01;
	   wr( sr, 0x01, tmp );  */
	if (fntwidth == 8)
		ctWrite_i (CT_SR_O, 0x01, 0x01);	/* Clocking Mode Register */
	else
		ctWrite_i (CT_SR_O, 0x01, 0x00);	/* Clocking Mode Register */
	ctWrite_i (CT_SR_O, 0x02, 0x0f);	/* Enable CPU wr access to given memory plane */
	ctWrite_i (CT_SR_O, 0x03, 0x00);	/* Character Map Select Register */
	if (text)
		tmp = 0x02;
	else
		tmp = 0x0e;
	ctWrite_i (CT_SR_O, 0x04, tmp);	/* Enable CPU accesses to the rest of the 256KB
					   total VGA memory beyond the first 64KB and set
					   fb mapping mode. */
	ctWrite_i (CT_SR_O, 0x00, 0x03);	/* enable */
}

/*****************************************************************************/
static void
SetBitsPerPixelIntoXrRegs (int bpp)
{
	unsigned int n = (bpp >> 3), tmp;	/* only for 15, 8, 16, 24 bpp */
	static char md[4] = { 0x04, 0x02, 0x05, 0x06 };	/* DisplayColorMode */
	static char off[4] = { ~0x20, ~0x30, ~0x20, ~0x10 };	/* mask */
	static char on[4] = { 0x10, 0x00, 0x10, 0x20 };	/* mask */
	if (bpp == 15)
		n = 0;
	tmp = ctRead_i (CT_XR_O, 0x20);
	tmp &= off[n];
	tmp |= on[n];
	ctWrite_i (CT_XR_O, 0x20, tmp);	/* BitBLT Configuration */
	ctWrite_i (CT_XR_O, 0x81, md[n]);
}

/*****************************************************************************/
static void
SetCrRegs (struct ctfb_res_modes *var, int bits_per_pixel)
{				/* he -le-   ht|0    hd -ri- hs     -h-      he */
	unsigned char cr[0x7a];
	int i, tmp;
	unsigned int hd, hs, he, ht, hbe;	/* Horizontal.  */
	unsigned int vd, vs, ve, vt;	/* vertical */
	unsigned int bpp, wd, dblscan, interlaced, bcast, CrtHalfLine;
	unsigned int CompSyncCharClkDelay, CompSyncPixelClkDelay;
	unsigned int NTSC_PAL_HorizontalPulseWidth, BlDelayCtrl;
	unsigned int HorizontalEqualizationPulses;
	unsigned int HorizontalSerration1Start, HorizontalSerration2Start;

	const int LineCompare = 0x3ff;
	unsigned int TextScanLines = 1;	/* this is in fact a vertical zoom factor   */
	unsigned int RAMDAC_BlankPedestalEnable = 0;	/* 1=en-, 0=disable, see XR82 */

	hd = (var->xres) / 8;	/* HDisp.  */
	hs = (var->xres + var->right_margin) / 8;	/* HsStrt  */
	he = (var->xres + var->right_margin + var->hsync_len) / 8;	/* HsEnd   */
	ht = (var->left_margin + var->xres + var->right_margin + var->hsync_len) / 8;	/* HTotal  */
	hbe = ht - 1;		/* HBlankEnable todo docu wants ht here, but it does not work */
	/* ve -up-  vt|0    vd -lo- vs     -v-      ve */
	vd = var->yres;		/* VDisplay   */
	vs = var->yres + var->lower_margin;	/* VSyncStart */
	ve = var->yres + var->lower_margin + var->vsync_len;	/* VSyncEnd */
	vt = var->upper_margin + var->yres + var->lower_margin + var->vsync_len;	/* VTotal  */
	bpp = bits_per_pixel;
	dblscan = (var->vmode & FB_VMODE_DOUBLE) ? 1 : 0;
	interlaced = var->vmode & FB_VMODE_INTERLACED;
	bcast = var->sync & FB_SYNC_BROADCAST;
	CrtHalfLine = bcast ? (hd >> 1) : 0;
	BlDelayCtrl = bcast ? 1 : 0;
	CompSyncCharClkDelay = 0;	/* 2 bit */
	CompSyncPixelClkDelay = 0;	/* 3 bit */
	if (bcast) {
		NTSC_PAL_HorizontalPulseWidth = 7;	/*( var->hsync_len >> 1 ) + 1 */
		HorizontalEqualizationPulses = 0;	/* inverse value */
		HorizontalSerration1Start = 31;	/* ( ht >> 1 ) */
		HorizontalSerration2Start = 89;	/* ( ht >> 1 ) */
	} else {
		NTSC_PAL_HorizontalPulseWidth = 0;
		/* 4 bit: hsync pulse width = ( ( CR74[4:0] - CR74[5] )
		 * / 2 ) + 1 --> CR74[4:0] = 2*(hs-1) + CR74[5] */
		HorizontalEqualizationPulses = 1;	/* inverse value */
		HorizontalSerration1Start = 0;	/* ( ht >> 1 ) */
		HorizontalSerration2Start = 0;	/* ( ht >> 1 ) */
	}

	if (bpp == 15)
		bpp = 16;
	wd = var->xres * bpp / 64;	/* double words per line */
	if (interlaced) {	/* we divide all vertical timings, exept vd */
		vs >>= 1;
		ve >>= 1;
		vt >>= 1;
	}
	memset (cr, 0, sizeof (cr));
	cr[0x00] = 0xff & (ht - 5);
	cr[0x01] = hd - 1;	/* soll:4f ist 59 */
	cr[0x02] = hd;
	cr[0x03] = (hbe & 0x1F) | 0x80;	/* hd + ht - hd  */
	cr[0x04] = hs;
	cr[0x05] = ((hbe & 0x20) << 2) | (he & 0x1f);
	cr[0x06] = (vt - 2) & 0xFF;
	cr[0x30] = (vt - 2) >> 8;
	cr[0x07] = ((vt & 0x100) >> 8)
	    | ((vd & 0x100) >> 7)
	    | ((vs & 0x100) >> 6)
	    | ((vs & 0x100) >> 5)
	    | ((LineCompare & 0x100) >> 4)
	    | ((vt & 0x200) >> 4)
	    | ((vd & 0x200) >> 3)
	    | ((vs & 0x200) >> 2);
	cr[0x08] = 0x00;
	cr[0x09] = (dblscan << 7)
	    | ((LineCompare & 0x200) >> 3)
	    | ((vs & 0x200) >> 4)
	    | (TextScanLines - 1);
	cr[0x10] = vs & 0xff;	/* VSyncPulseStart */
	cr[0x32] = (vs & 0xf00) >> 8;	/* VSyncPulseStart */
	cr[0x11] = (ve & 0x0f);	/* | 0x20;      */
	cr[0x12] = (vd - 1) & 0xff;	/* LineCount  */
	cr[0x31] = ((vd - 1) & 0xf00) >> 8;	/* LineCount */
	cr[0x13] = wd & 0xff;
	cr[0x41] = (wd & 0xf00) >> 8;
	cr[0x15] = vs & 0xff;
	cr[0x33] = (vs & 0xf00) >> 8;
	cr[0x38] = (0x100 & (ht - 5)) >> 8;
	cr[0x3C] = 0xc0 & hbe;
	cr[0x16] = (vt - 1) & 0xff;	/* vbe - docu wants vt here, */
	cr[0x17] = 0xe3;	/* but it does not work */
	cr[0x18] = 0xff & LineCompare;
	cr[0x22] = 0xff;	/* todo? */
	cr[0x70] = interlaced ? (0x80 | CrtHalfLine) : 0x00;	/* check:0xa6  */
	cr[0x71] = 0x80 | (RAMDAC_BlankPedestalEnable << 6)
	    | (BlDelayCtrl << 5)
	    | ((0x03 & CompSyncCharClkDelay) << 3)
	    | (0x07 & CompSyncPixelClkDelay);	/* todo: see XR82 */
	cr[0x72] = HorizontalSerration1Start;
	cr[0x73] = HorizontalSerration2Start;
	cr[0x74] = (HorizontalEqualizationPulses << 5)
	    | NTSC_PAL_HorizontalPulseWidth;
	/* todo: ct69000 has also 0x75-79 */
	/* now set the registers */
	for (i = 0; i <= 0x0d; i++) {	/*CR00 .. CR0D */
		ctWrite_i (CT_CR_O, i, cr[i]);
	}
	for (i = 0x10; i <= 0x18; i++) {	/*CR10 .. CR18 */
		ctWrite_i (CT_CR_O, i, cr[i]);
	}
	i = 0x22;		/*CR22 */
	ctWrite_i (CT_CR_O, i, cr[i]);
	for (i = 0x30; i <= 0x33; i++) {	/*CR30 .. CR33 */
		ctWrite_i (CT_CR_O, i, cr[i]);
	}
	i = 0x38;		/*CR38 */
	ctWrite_i (CT_CR_O, i, cr[i]);
	i = 0x3C;		/*CR3C */
	ctWrite_i (CT_CR_O, i, cr[i]);
	for (i = 0x40; i <= 0x41; i++) {	/*CR40 .. CR41 */
		ctWrite_i (CT_CR_O, i, cr[i]);
	}
	for (i = 0x70; i <= 0x74; i++) {	/*CR70 .. CR74 */
		ctWrite_i (CT_CR_O, i, cr[i]);
	}
	tmp = ctRead_i (CT_CR_O, 0x40);
	tmp &= 0x0f;
	tmp |= 0x80;
	ctWrite_i (CT_CR_O, 0x40, tmp);	/* StartAddressEnable */
}

/* pixelclock control */

/*****************************************************************************
 We have a rational number p/q and need an m/n which is very close to p/q
 but has m and n within mnmin and mnmax. We have no floating point in the
 kernel. We can use long long without divide. And we have time to compute...
******************************************************************************/
static unsigned int
FindBestPQFittingMN (unsigned int p, unsigned int q, unsigned int mnmin,
		     unsigned int mnmax, unsigned int *pm, unsigned int *pn)
{
	/* this code is not for general purpose usable but good for our number ranges */
	unsigned int n = mnmin, m = 0;
	long long int L = 0, P = p, Q = q, H = P >> 1;
	long long int D = 0x7ffffffffffffffLL;
	for (n = mnmin; n <= mnmax; n++) {
		m = mnmin;	/* p/q ~ m/n -> p*n ~ m*q -> p*n-x*q ~ 0 */
		L = P * n - m * Q;	/* n * vco - m * fref should be near 0 */
		while (L > 0 && m < mnmax) {
			L -= q;	/* difference is greater as 0 subtract fref */
			m++;	/* and increment m */
		}
		/* difference is less or equal than 0 or m > maximum */
		if (m > mnmax)
			break;	/* no solution: if we increase n we get the same situation */
		/* L is <= 0 now */
		if (-L > H && m > mnmin) {	/* if difference > the half fref */
			L += q;	/* we take the situation before */
			m--;	/* because its closer to 0 */
		}
		L = (L < 0) ? -L : +L;	/* absolute value */
		if (D < L)	/* if last difference was better take next n */
			continue;
		D = L;
		*pm = m;
		*pn = n;	/*  keep improved data */
		if (D == 0)
			break;	/* best result we can get */
	}
	return (unsigned int) (0xffffffff & D);
}

/* that is the hardware < 69000 we have to manage
 +---------+  +-------------------+  +----------------------+  +--+
 | REFCLK  |__|NTSC Divisor Select|__|FVCO Reference Divisor|__|÷N|__
 | 14.3MHz |  |(NTSCDS) (÷1, ÷5)  |  |Select (RDS) (÷1, ÷4) |  |  |  |
 +---------+  +-------------------+  +----------------------+  +--+  |
  ___________________________________________________________________|
 |
 |                                    fvco                      fout
 | +--------+  +------------+  +-----+     +-------------------+   +----+
 +-| Phase  |__|Charge Pump |__| VCO |_____|Post Divisor (PD)  |___|CLK |--->
 +-| Detect |  |& Filter VCO|  |     |  |  |÷1, 2, 4, 8, 16, 32|   |    |
 | +--------+  +------------+  +-----+  |  +-------------------+   +----+
 |                                      |
 |    +--+   +---------------+          |
 |____|÷M|___|VCO Loop Divide|__________|
      |  |   |(VLD)(÷4, ÷16) |
      +--+   +---------------+
****************************************************************************
  that is the hardware >= 69000 we have to manage
 +---------+  +--+
 | REFCLK  |__|÷N|__
 | 14.3MHz |  |  |  |
 +---------+  +--+  |
  __________________|
 |
 |                                    fvco                      fout
 | +--------+  +------------+  +-----+     +-------------------+   +----+
 +-| Phase  |__|Charge Pump |__| VCO |_____|Post Divisor (PD)  |___|CLK |--->
 +-| Detect |  |& Filter VCO|  |     |  |  |÷1, 2, 4, 8, 16, 32|   |    |
 | +--------+  +------------+  +-----+  |  +-------------------+   +----+
 |                                      |
 |    +--+   +---------------+          |
 |____|÷M|___|VCO Loop Divide|__________|
      |  |   |(VLD)(÷1, ÷4)  |
      +--+   +---------------+


*/

#define VIDEO_FREF 14318180;	/* Hz  */
/*****************************************************************************/
static int
ReadPixClckFromXrRegsBack (struct ctfb_chips_properties *param)
{
	unsigned int m, n, vld, pd, PD, fref, xr_cb, i, pixclock;
	i = 0;
	pixclock = -1;
	fref = VIDEO_FREF;
	m = ctRead_i (CT_XR_O, 0xc8);
	n = ctRead_i (CT_XR_O, 0xc9);
	m -= param->mn_diff;
	n -= param->mn_diff;
	xr_cb = ctRead_i (CT_XR_O, 0xcb);
	PD = (0x70 & xr_cb) >> 4;
	pd = 1;
	for (i = 0; i < PD; i++) {
		pd *= 2;
	}
	vld = (0x04 & xr_cb) ? param->vld_set : param->vld_not_set;
	if (n * vld * m) {
		unsigned long long p = 1000000000000LL * pd * n;
		unsigned long long q = (long long) fref * vld * m;
		while ((p > 0xffffffffLL) || (q > 0xffffffffLL)) {
			p >>= 1;	/* can't divide with long long so we scale down */
			q >>= 1;
		}
		pixclock = (unsigned) p / (unsigned) q;
	} else
		printf ("Invalid data in xr regs.\n");
	return pixclock;
}

/*****************************************************************************/
static void
FindAndSetPllParamIntoXrRegs (unsigned int pixelclock,
			      struct ctfb_chips_properties *param)
{
	unsigned int m, n, vld, pd, PD, fref, xr_cb;
	unsigned int fvcomin, fvcomax, pclckmin, pclckmax, pclk;
	unsigned int pfreq, fvco, new_pixclock;
	unsigned int D,nback,mback;

	fref = VIDEO_FREF;
	pd = 1;
	PD = 0;
	fvcomin = param->vco_min;
	fvcomax = param->vco_max;	/* MHz */
	pclckmin = 1000000 / fvcomax + 1;	/*   4546 */
	pclckmax = 32000000 / fvcomin - 1;	/* 666665 */
	pclk = minmax (pclckmin, pixelclock, pclckmax);	/* ps pp */
	pfreq = 250 * (4000000000U / pclk);
	fvco = pfreq;		/* Hz */
	new_pixclock = 0;
	while (fvco < fvcomin * 1000000) {
		/* double VCO starting with the pixelclock frequency
		 * as long as it is lower than the minimal VCO frequency */
		fvco *= 2;
		pd *= 2;
		PD++;
	}
	/* fvco is exactly pd * pixelclock and higher than the ninmal VCO frequency */
	/* first try */
	vld = param->vld_set;
	D=FindBestPQFittingMN (fvco / vld, fref, param->mn_min, param->mn_max, &m, &n); /* rds = 1 */
	mback=m;
	nback=n;
	/* second try */
	vld = param->vld_not_set;
	if(D<FindBestPQFittingMN (fvco / vld, fref, param->mn_min, param->mn_max, &m, &n)) {    /* rds = 1 */
		/* first try was better */
		m=mback;
		n=nback;
		vld = param->vld_set;
	}
	m += param->mn_diff;
	n += param->mn_diff;
	debug("VCO %d, pd %d, m %d n %d vld %d\n", fvco, pd, m, n, vld);
	xr_cb = ((0x7 & PD) << 4) | (vld == param->vld_set ? 0x04 : 0);
	/* All four of the registers used for dot clock 2 (XRC8 - XRCB) must be
	 * written, and in order from XRC8 to XRCB, before the hardware will
	 * update the synthesizer s settings.
	 */
	ctWrite_i (CT_XR_O, 0xc8, m);
	ctWrite_i (CT_XR_O, 0xc9, n);	/* xrca does not exist in CT69000 and CT69030 */
	ctWrite_i (CT_XR_O, 0xca, 0);	/* because of a hw bug I guess, but we write */
	ctWrite_i (CT_XR_O, 0xcb, xr_cb);	/* 0 to it for savety */
	new_pixclock = ReadPixClckFromXrRegsBack (param);
	debug("pixelclock.set = %d, pixelclock.real = %d\n",
		pixelclock, new_pixclock);
}

/*****************************************************************************/
static void
SetMsrRegs (struct ctfb_res_modes *mode)
{
	unsigned char h_synch_high, v_synch_high;

	h_synch_high = (mode->sync & FB_SYNC_HOR_HIGH_ACT) ? 0 : 0x40;	/* horizontal Synch High active */
	v_synch_high = (mode->sync & FB_SYNC_VERT_HIGH_ACT) ? 0 : 0x80;	/* vertical Synch High active */
	ctWrite (CT_MSR_W_O, (h_synch_high | v_synch_high | 0x29));
	/* upper64K==0x20, CLC2select==0x08, RAMenable==0x02!(todo), CGA==0x01
	 * Selects the upper 64KB page.Bit5=1
	 * CLK2 (left reserved in standard VGA) Bit3|2=1|0
	 * Disables CPU access to frame buffer. Bit1=0
	 * Sets the I/O address decode for ST01, FCR, and all CR registers
	 * to the 3Dx I/O address range (CGA emulation). Bit0=1
	 */
}

/************************************************************************************/
#ifdef VGA_DUMP_REG

static void
ctDispRegs (unsigned short index, int from, int to)
{
	unsigned char status;
	int i;

	for (i = from; i < to; i++) {
		status = ctRead_i (index, i);
		printf ("%02X: is %02X\n", i, status);
	}
}

void
video_dump_reg (void)
{
	int i;

	printf ("Extended Regs:\n");
	ctDispRegs (CT_XR_O, 0, 0xC);
	ctDispRegs (CT_XR_O, 0xe, 0xf);
	ctDispRegs (CT_XR_O, 0x20, 0x21);
	ctDispRegs (CT_XR_O, 0x40, 0x50);
	ctDispRegs (CT_XR_O, 0x60, 0x64);
	ctDispRegs (CT_XR_O, 0x67, 0x68);
	ctDispRegs (CT_XR_O, 0x70, 0x72);
	ctDispRegs (CT_XR_O, 0x80, 0x83);
	ctDispRegs (CT_XR_O, 0xA0, 0xB0);
	ctDispRegs (CT_XR_O, 0xC0, 0xD3);
	printf ("Sequencer Regs:\n");
	ctDispRegs (CT_SR_O, 0, 0x8);
	printf ("Graphic Regs:\n");
	ctDispRegs (CT_GR_O, 0, 0x9);
	printf ("CRT Regs:\n");
	ctDispRegs (CT_CR_O, 0, 0x19);
	ctDispRegs (CT_CR_O, 0x22, 0x23);
	ctDispRegs (CT_CR_O, 0x30, 0x34);
	ctDispRegs (CT_CR_O, 0x38, 0x39);
	ctDispRegs (CT_CR_O, 0x3C, 0x3D);
	ctDispRegs (CT_CR_O, 0x40, 0x42);
	ctDispRegs (CT_CR_O, 0x70, 0x80);
	/* don't display the attributes */
}

#endif

/***************************************************************
 * Wait for BitBlt ready
 */
static int
video_wait_bitblt (unsigned long addr)
{
	unsigned long br04;
	int i = 0;
	br04 = in32r (addr);
	while (br04 & 0x80000000) {
		udelay (1);
		br04 = in32r (addr);
		if (i++ > 1000000) {
			printf ("ERROR Timeout %lx\n", br04);
			return 1;
		}
	}
	return 0;
}

/***************************************************************
 * Set up BitBlt Registrs
 */
static void
SetDrawingEngine (int bits_per_pixel)
{
	unsigned long br04, br00;
	unsigned char tmp;

	GraphicDevice *pGD = (GraphicDevice *) & ctfb;

	tmp = ctRead_i (CT_XR_O, 0x20);	/* BitBLT Configuration */
	tmp |= 0x02;		/* reset BitBLT */
	ctWrite_i (CT_XR_O, 0x20, tmp);	/* BitBLT Configuration */
	udelay (10);
	tmp &= 0xfd;		/* release reset BitBLT */
	ctWrite_i (CT_XR_O, 0x20, tmp);	/* BitBLT Configuration */
	video_wait_bitblt (pGD->pciBase + BR04_o);

	/* set pattern Address */
	out32r (pGD->pciBase + BR05_o, PATTERN_ADR & 0x003ffff8);
	br04 = 0;
	if (bits_per_pixel == 1) {
		br04 |= 0x00040000;	/* monochome Pattern */
		br04 |= 0x00001000;	/* monochome source */
	}
	br00 = ((pGD->winSizeX * pGD->gdfBytesPP) << 16) + (pGD->winSizeX * pGD->gdfBytesPP);	/* bytes per scanline */
	out32r (pGD->pciBase + BR00_o, br00);	/* */
	out32r (pGD->pciBase + BR08_o, (10 << 16) + 10);	/* dummy */
	out32r (pGD->pciBase + BR04_o, br04);	/* write all 0 */
	out32r (pGD->pciBase + BR07_o, 0);	/* destination */
	video_wait_bitblt (pGD->pciBase + BR04_o);
}

/****************************************************************************
* supported Video Chips
*/
static struct pci_device_id supported[] = {
	{PCI_VENDOR_ID_CT, PCI_DEVICE_ID_CT_69000},
	{}
};

/*******************************************************************************
*
* Init video chip
*/
void *
video_hw_init (void)
{
	GraphicDevice *pGD = (GraphicDevice *) & ctfb;
	unsigned short device_id;
	pci_dev_t devbusfn;
	int videomode;
	unsigned long t1, hsynch, vsynch;
	unsigned int pci_mem_base, *vm;
	int tmp, i, bits_per_pixel;
	char *penv;
	struct ctfb_res_modes *res_mode;
	struct ctfb_res_modes var_mode;
	struct ctfb_chips_properties *chips_param;
	/* Search for video chip */

	if ((devbusfn = pci_find_devices (supported, 0)) < 0) {
#ifdef CONFIG_VIDEO_ONBOARD
		printf ("Video: Controller not found !\n");
#endif
		return (NULL);
	}

	/* PCI setup */
	pci_write_config_dword (devbusfn, PCI_COMMAND,
				(PCI_COMMAND_MEMORY | PCI_COMMAND_IO));
	pci_read_config_word (devbusfn, PCI_DEVICE_ID, &device_id);
	pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_0, &pci_mem_base);
	pci_mem_base = pci_mem_to_phys (devbusfn, pci_mem_base);

	/* get chips params */
	for (chips_param = (struct ctfb_chips_properties *) &chips[0];
	     chips_param->device_id != 0; chips_param++) {
		if (chips_param->device_id == device_id)
			break;
	}
	if (chips_param->device_id == 0) {
#ifdef CONFIG_VIDEO_ONBOARD
		printf ("Video: controller 0x%X not supported\n", device_id);
#endif
		return NULL;
	}
	/* supported Video controller found */
	printf ("Video: ");

	tmp = 0;
	videomode = 0x301;
	/* get video mode via environment */
	if ((penv = getenv ("videomode")) != NULL) {
		/* deceide if it is a string */
		if (penv[0] <= '9') {
			videomode = (int) simple_strtoul (penv, NULL, 16);
			tmp = 1;
		}
	} else {
		tmp = 1;
	}
	if (tmp) {
		/* parameter are vesa modes */
		/* search params */
		for (i = 0; i < VESA_MODES_COUNT; i++) {
			if (vesa_modes[i].vesanr == videomode)
				break;
		}
		if (i == VESA_MODES_COUNT) {
			printf ("no VESA Mode found, switching to mode 0x301 ");
			i = 0;
		}
		res_mode =
		    (struct ctfb_res_modes *) &res_mode_init[vesa_modes[i].
							     resindex];
		bits_per_pixel = vesa_modes[i].bits_per_pixel;
	} else {

		res_mode = (struct ctfb_res_modes *) &var_mode;
		bits_per_pixel = video_get_params (res_mode, penv);
	}

	/* calculate available color depth for controller memory */
	if (bits_per_pixel == 15)
		tmp = 2;
	else
		tmp = bits_per_pixel >> 3;	/* /8 */
	if (((chips_param->max_mem -
	      ACCELMEMORY) / (res_mode->xres * res_mode->yres)) < tmp) {
		tmp =
		    ((chips_param->max_mem -
		      ACCELMEMORY) / (res_mode->xres * res_mode->yres));
		if (tmp == 0) {
			printf
			    ("No matching videomode found .-> reduce resolution\n");
			return NULL;
		} else {
			printf ("Switching back to %d Bits per Pixel ",
				tmp << 3);
			bits_per_pixel = tmp << 3;
		}
	}

	/* calculate hsynch and vsynch freq (info only) */
	t1 = (res_mode->left_margin + res_mode->xres +
	      res_mode->right_margin + res_mode->hsync_len) / 8;
	t1 *= 8;
	t1 *= res_mode->pixclock;
	t1 /= 1000;
	hsynch = 1000000000L / t1;
	t1 *=
	    (res_mode->upper_margin + res_mode->yres +
	     res_mode->lower_margin + res_mode->vsync_len);
	t1 /= 1000;
	vsynch = 1000000000L / t1;

	/* fill in Graphic device struct */
	sprintf (pGD->modeIdent, "%dx%dx%d %ldkHz %ldHz", res_mode->xres,
		 res_mode->yres, bits_per_pixel, (hsynch / 1000),
		 (vsynch / 1000));
	printf ("%s\n", pGD->modeIdent);
	pGD->winSizeX = res_mode->xres;
	pGD->winSizeY = res_mode->yres;
	pGD->plnSizeX = res_mode->xres;
	pGD->plnSizeY = res_mode->yres;
	switch (bits_per_pixel) {
	case 8:
		pGD->gdfBytesPP = 1;
		pGD->gdfIndex = GDF__8BIT_INDEX;
		break;
	case 15:
		pGD->gdfBytesPP = 2;
		pGD->gdfIndex = GDF_15BIT_555RGB;
		break;
	case 16:
		pGD->gdfBytesPP = 2;
		pGD->gdfIndex = GDF_16BIT_565RGB;
		break;
	case 24:
		pGD->gdfBytesPP = 3;
		pGD->gdfIndex = GDF_24BIT_888RGB;
		break;
	}
	pGD->isaBase = CONFIG_SYS_ISA_IO_BASE_ADDRESS;
	pGD->pciBase = pci_mem_base;
	pGD->frameAdrs = pci_mem_base;
	pGD->memSize = chips_param->max_mem;
	/* Cursor Start Address */
	pGD->dprBase =
	    (pGD->winSizeX * pGD->winSizeY * pGD->gdfBytesPP) + pci_mem_base;
	if ((pGD->dprBase & 0x0fff) != 0) {
		/* allign it */
		pGD->dprBase &= 0xfffff000;
		pGD->dprBase += 0x00001000;
	}
	debug("Cursor Start %x Pattern Start %x\n", pGD->dprBase,
		PATTERN_ADR);
	pGD->vprBase = pci_mem_base;	/* Dummy */
	pGD->cprBase = pci_mem_base;	/* Dummy */
	/* set up Hardware */

	ctWrite (CT_MSR_W_O, 0x01);

	/* set the extended Registers */
	ctLoadRegs (CT_XR_O, xreg);
	/* set atribute registers */
	SetArRegs ();
	/* set Graphics register */
	SetGrRegs ();
	/* set sequencer */
	SetSrRegs ();

	/* set msr */
	SetMsrRegs (res_mode);

	/* set CRT Registers */
	SetCrRegs (res_mode, bits_per_pixel);
	/* set color mode */
	SetBitsPerPixelIntoXrRegs (bits_per_pixel);

	/* set PLL */
	FindAndSetPllParamIntoXrRegs (res_mode->pixclock, chips_param);

	ctWrite_i (CT_SR_O, 0, 0x03);	/* clear synchronous reset */
	/* Clear video memory */
	i = pGD->memSize / 4;
	vm = (unsigned int *) pGD->pciBase;
	while (i--)
		*vm++ = 0;
	SetDrawingEngine (bits_per_pixel);
#ifdef VGA_DUMP_REG
	video_dump_reg ();
#endif

	return ((void *) &ctfb);
}

 /*******************************************************************************
*
* Set a RGB color in the LUT (8 bit index)
*/
void
video_set_lut (unsigned int index,	/* color number */
	       unsigned char r,	/* red */
	       unsigned char g,	/* green */
	       unsigned char b	/* blue */
    )
{

	ctWrite (CT_LUT_MASK_O, 0xff);

	ctWrite (CT_LUT_START_O, (char) index);

	ctWrite (CT_LUT_RGB_O, r);	/* red */
	ctWrite (CT_LUT_RGB_O, g);	/* green */
	ctWrite (CT_LUT_RGB_O, b);	/* blue */
	udelay (1);
	ctWrite (CT_LUT_MASK_O, 0xff);
}

/*******************************************************************************
*
* Drawing engine fill on screen region
*/
void
video_hw_rectfill (unsigned int bpp,	/* bytes per pixel */
		   unsigned int dst_x,	/* dest pos x */
		   unsigned int dst_y,	/* dest pos y */
		   unsigned int dim_x,	/* frame width */
		   unsigned int dim_y,	/* frame height */
		   unsigned int color	/* fill color */
    )
{
	GraphicDevice *pGD = (GraphicDevice *) & ctfb;
	unsigned long *p, br04;

	video_wait_bitblt (pGD->pciBase + BR04_o);

	p = (unsigned long *) PATTERN_ADR;
	dim_x *= bpp;
	if (bpp == 3)
		bpp++;		/* 24Bit needs a 32bit pattern */
	memset (p, color, (bpp * sizeof (unsigned char) * 8 * 8));	/* 8 x 8 pattern data */
	out32r (pGD->pciBase + BR07_o, ((pGD->winSizeX * dst_y) + dst_x) * pGD->gdfBytesPP);	/* destination */
	br04 = in32r (pGD->pciBase + BR04_o) & 0xffffff00;
	br04 |= 0xF0;		/* write Pattern P -> D */
	out32r (pGD->pciBase + BR04_o, br04);	/* */
	out32r (pGD->pciBase + BR08_o, (dim_y << 16) + dim_x);	/* starts the BITBlt */
	video_wait_bitblt (pGD->pciBase + BR04_o);
}

/*******************************************************************************
*
* Drawing engine bitblt with screen region
*/
void
video_hw_bitblt (unsigned int bpp,	/* bytes per pixel */
		 unsigned int src_x,	/* source pos x */
		 unsigned int src_y,	/* source pos y */
		 unsigned int dst_x,	/* dest pos x */
		 unsigned int dst_y,	/* dest pos y */
		 unsigned int dim_x,	/* frame width */
		 unsigned int dim_y	/* frame height */
    )
{
	GraphicDevice *pGD = (GraphicDevice *) & ctfb;
	unsigned long br04;

	br04 = in32r (pGD->pciBase + BR04_o);

	/* to prevent data corruption due to overlap, we have to
	 * find out if, and how the frames overlaps */
	if (src_x < dst_x) {
		/* src is more left than dest
		 * the frame may overlap -> start from right to left */
		br04 |= 0x00000100;	/* set bit 8 */
		src_x += dim_x;
		dst_x += dim_x;
	} else {
		br04 &= 0xfffffeff;	/* clear bit 8 left to right */
	}
	if (src_y < dst_y) {
		/* src is higher than dst
		 * the frame may overlap => start from bottom */
		br04 |= 0x00000200;	/* set bit 9 */
		src_y += dim_y;
		dst_y += dim_y;
	} else {
		br04 &= 0xfffffdff;	/* clear bit 9 top to bottom */
	}
	dim_x *= bpp;
	out32r (pGD->pciBase + BR06_o, ((pGD->winSizeX * src_y) + src_x) * pGD->gdfBytesPP);	/* source */
	out32r (pGD->pciBase + BR07_o, ((pGD->winSizeX * dst_y) + dst_x) * pGD->gdfBytesPP);	/* destination */
	br04 &= 0xffffff00;
	br04 |= 0x000000CC;	/* S -> D */
	out32r (pGD->pciBase + BR04_o, br04);	/* */
	out32r (pGD->pciBase + BR08_o, (dim_y << 16) + dim_x);	/* start the BITBlt */
	video_wait_bitblt (pGD->pciBase + BR04_o);
}
#endif				/* CONFIG_VIDEO */

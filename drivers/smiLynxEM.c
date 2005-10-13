/*
 * (C) Copyright 1997-2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * smiLynxEM.c
 *
 * Silicon Motion graphic interface for sm810/sm710/sm712 accelerator
 *
 * modification history
 * --------------------
 * 04-18-2002 Rewritten for U-Boot <fgottschling@eltec.de>.
 *
 * 18-03-2004 - Unify videomodes handling with the ct69000
 *            - The video output can be set via the variable "videoout"
 *              in the environment.
 *              videoout=1 output on LCD
 *              videoout=2 output on CRT (default value)
 *	                <p.aubert@staubli.com>
 */

#include <common.h>

#if defined(CONFIG_VIDEO_SMI_LYNXEM)

#include <pci.h>
#include <video_fb.h>
#include "videomodes.h"
/*
 * Export Graphic Device
 */
GraphicDevice smi;

/*
 * SMI 710/712 have 4MB internal RAM; SMI 810 2MB internal + 2MB external
 */
#define VIDEO_MEM_SIZE	0x400000


/*
 * ISA mapped regs
 */
#define SMI_INDX_C4		(pGD->isaBase + 0x03c4)	   /* index reg */
#define SMI_DATA_C5		(pGD->isaBase + 0x03c5)	   /* data reg */
#define SMI_INDX_D4		(pGD->isaBase + 0x03d4)	   /* index reg */
#define SMI_DATA_D5		(pGD->isaBase + 0x03d5)	   /* data reg */
#define SMI_ISR1		(pGD->isaBase + 0x03ca)
#define SMI_INDX_CE		(pGD->isaBase + 0x03ce)	   /* index reg */
#define SMI_DATA_CF		(pGD->isaBase + 0x03cf)	   /* data reg */
#define SMI_LOCK_REG		(pGD->isaBase + 0x03c3)	   /* unlock/lock ext crt reg */
#define SMI_MISC_REG		(pGD->isaBase + 0x03c2)	   /* misc reg */
#define SMI_LUT_MASK		(pGD->isaBase + 0x03c6)	   /* lut mask reg */
#define SMI_LUT_START		(pGD->isaBase + 0x03c8)	   /* lut start index */
#define SMI_LUT_RGB		(pGD->isaBase + 0x03c9)	   /* lut colors auto incr.*/
#define SMI_INDX_ATTR		(pGD->isaBase + 0x03c0)	   /* attributes index reg */

/*
 * Video processor control
 */
typedef struct {
	unsigned int   control;
	unsigned int   colorKey;
	unsigned int   colorKeyMask;
	unsigned int   start;
	unsigned short offset;
	unsigned short width;
	unsigned int   fifoPrio;
	unsigned int   fifoERL;
	unsigned int   YUVtoRGB;
} SmiVideoProc;

/*
 * Video window control
 */
typedef struct {
	unsigned short top;
	unsigned short left;
	unsigned short bottom;
	unsigned short right;
	unsigned int   srcStart;
	unsigned short width;
	unsigned short offset;
	unsigned char  hStretch;
	unsigned char  vStretch;
} SmiVideoWin;

/*
 * Capture port control
 */
typedef struct {
	unsigned int   control;
	unsigned short topClip;
	unsigned short leftClip;
	unsigned short srcHeight;
	unsigned short srcWidth;
	unsigned int   srcBufStart1;
	unsigned int   srcBufStart2;
	unsigned short srcOffset;
	unsigned short fifoControl;
} SmiCapturePort;


/*
 * Register values for common video modes
 */
static char SMI_SCR[] = {
	/* all modes */
	0x10, 0xff, 0x11, 0xff, 0x12, 0xff, 0x13, 0xff, 0x15, 0x90,
	0x17, 0x20, 0x18, 0xb1, 0x19, 0x00,
};
static char SMI_EXT_CRT[] = {
	0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00, 0x35, 0x00,
	0x36, 0x00, 0x3b, 0x00, 0x3c, 0x00, 0x3d, 0x00, 0x3e, 0x00, 0x3f, 0x00,
};
static char SMI_ATTR [] = {
	0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05,
	0x06, 0x06, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b,
	0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x41, 0x11, 0x00,
	0x12, 0x0f, 0x13, 0x00, 0x14, 0x00,
};
static char SMI_GCR[18] = {
	0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x40,
	0x06, 0x05, 0x07, 0x0f, 0x08, 0xff,
};
static char SMI_SEQR[] = {
	0x00, 0x00, 0x01, 0x01, 0x02, 0x0f, 0x03, 0x03, 0x04, 0x0e, 0x00, 0x03,
};
static char SMI_PCR [] = {
	0x20, 0x04, 0x21, 0x30, 0x22, 0x00, 0x23, 0x00, 0x24, 0x00,
};
static char SMI_MCR[] = {
	0x60, 0x01, 0x61, 0x00,
#ifdef CONFIG_HMI1001
	0x62, 0x74, /* Memory type is not configured by pins on HMI1001 */
#endif
};

static char SMI_HCR[] = {
	0x80, 0xff, 0x81, 0x07, 0x82, 0x00, 0x83, 0xff, 0x84, 0xff, 0x88, 0x00,
	0x89, 0x02, 0x8a, 0x80, 0x8b, 0x01, 0x8c, 0xff, 0x8d, 0x00,
};


/*******************************************************************************
 *
 * Write SMI ISA register
 */
static void smiWrite (unsigned short index, char reg, char val)
{
	register GraphicDevice *pGD = (GraphicDevice *)&smi;

	out8 ((pGD->isaBase + index), reg);
	out8 ((pGD->isaBase + index + 1), val);
}

/*******************************************************************************
 *
 * Write a table of SMI ISA register
 */
static void smiLoadRegs (
	unsigned int iReg,
	unsigned int dReg,
	char	     *regTab,
	unsigned int tabSize
	)
{
	register GraphicDevice *pGD  = (GraphicDevice *)&smi;
	register int i;

	for (i=0; i<tabSize; i+=2) {
		if (iReg == SMI_INDX_ATTR) {
			/* Reset the Flip Flop */
			in8 (SMI_ISR1);
			out8 (iReg, regTab[i]);
			out8 (iReg, regTab[i+1]);
		} else {
			out8 (iReg, regTab[i]);
			out8 (dReg, regTab[i+1]);
		}
	}
}

/*******************************************************************************
 *
 * Init capture port registers
 */
static void smiInitCapturePort (void)
{
	SmiCapturePort smiCP = { 0x01400600, 0x30, 0x40, 480, 640, 0, 0, 2560, 6 };
	register GraphicDevice *pGD  = (GraphicDevice *)&smi;
	register SmiCapturePort *pCP = (SmiCapturePort *)&smiCP;

	out32r ((pGD->cprBase + 0x0004), ((pCP->topClip<<16)   | pCP->leftClip));
	out32r ((pGD->cprBase + 0x0008), ((pCP->srcHeight<<16) | pCP->srcWidth));
	out32r ((pGD->cprBase + 0x000c), pCP->srcBufStart1/8);
	out32r ((pGD->cprBase + 0x0010), pCP->srcBufStart2/8);
	out32r ((pGD->cprBase + 0x0014), pCP->srcOffset/8);
	out32r ((pGD->cprBase + 0x0018), pCP->fifoControl);
	out32r ((pGD->cprBase + 0x0000), pCP->control);
}


/*******************************************************************************
 *
 * Init video processor registers
 */
static void smiInitVideoProcessor (void)
{
	SmiVideoProc smiVP = { 0x100000, 0, 0, 0, 0, 1600, 0x1200543, 4, 0xededed };
	SmiVideoWin  smiVW = { 0, 0, 599, 799, 0, 1600, 0, 0, 0 };
	register GraphicDevice *pGD = (GraphicDevice *)&smi;
	register SmiVideoProc  *pVP = (SmiVideoProc *)&smiVP;
	register SmiVideoWin *pVWin = (SmiVideoWin *)&smiVW;

	pVP->width    = pGD->plnSizeX * pGD->gdfBytesPP;
	pVP->control |= pGD->gdfIndex << 16;
	pVWin->bottom = pGD->winSizeY - 1;
	pVWin->right  = pGD->winSizeX - 1;
	pVWin->width  = pVP->width;

	/* color key */
	out32r ((pGD->vprBase + 0x0004), pVP->colorKey);

	/* color key mask */
	out32r ((pGD->vprBase + 0x0008), pVP->colorKeyMask);

	/* data src start adrs */
	out32r ((pGD->vprBase + 0x000c), pVP->start / 8);

	/* data width and offset */
	out32r ((pGD->vprBase + 0x0010),
		((pVP->offset	/ 8 * pGD->gdfBytesPP) << 16) |
		(pGD->plnSizeX / 8 * pGD->gdfBytesPP));

	/* video window 1 */
	out32r ((pGD->vprBase + 0x0014),
		((pVWin->top << 16) | pVWin->left));

	out32r ((pGD->vprBase + 0x0018),
		((pVWin->bottom << 16) | pVWin->right));

	out32r ((pGD->vprBase + 0x001c), pVWin->srcStart / 8);

	out32r ((pGD->vprBase + 0x0020),
		(((pVWin->offset / 8) << 16) | (pVWin->width / 8)));

	out32r ((pGD->vprBase + 0x0024),
		(((pVWin->hStretch) << 8) | pVWin->vStretch));

	/* video window 2 */
	out32r ((pGD->vprBase + 0x0028),
		((pVWin->top << 16) | pVWin->left));

	out32r ((pGD->vprBase + 0x002c),
		((pVWin->bottom << 16) | pVWin->right));

	out32r ((pGD->vprBase + 0x0030),
		pVWin->srcStart / 8);

	out32r ((pGD->vprBase + 0x0034),
		(((pVWin->offset / 8) << 16) | (pVWin->width / 8)));

	out32r ((pGD->vprBase + 0x0038),
		(((pVWin->hStretch) << 8) | pVWin->vStretch));

	/* fifo prio control */
	out32r ((pGD->vprBase + 0x0054), pVP->fifoPrio);

	/* fifo empty request levell */
	out32r ((pGD->vprBase + 0x0058), pVP->fifoERL);

	/* conversion constant */
	out32r ((pGD->vprBase + 0x005c), pVP->YUVtoRGB);

	/* vpr control word */
	out32r ((pGD->vprBase + 0x0000), pVP->control);
}

/******************************************************************************
 *
 * Init drawing engine registers
 */
static void smiInitDrawingEngine (void)
{
	GraphicDevice *pGD = (GraphicDevice *)&smi;
	unsigned int val;

	/* don't start now */
	out32r ((pGD->dprBase + 0x000c), 0x000f0000);

	/* set rop2 to copypen */
	val = 0xffff3ff0 & in32r ((pGD->dprBase + 0x000c));
	out32r ((pGD->dprBase + 0x000c), (val | 0x8000 | 0x0c));

	/* set clip rect */
	out32r ((pGD->dprBase + 0x002c), 0);
	out32r ((pGD->dprBase + 0x0030),
		((pGD->winSizeY<<16) | pGD->winSizeX * pGD->gdfBytesPP ));

	/* src row pitch */
	val = 0xffff0000 & (in32r ((pGD->dprBase + 0x0010)));
	out32r ((pGD->dprBase + 0x0010),
		(val | pGD->plnSizeX * pGD->gdfBytesPP));

	/* dst row pitch */
	val = 0x0000ffff & (in32r ((pGD->dprBase + 0x0010)));
	out32r ((pGD->dprBase + 0x0010),
		(((pGD->plnSizeX * pGD->gdfBytesPP)<<16) | val));

	/* window width src/dst */
	out32r ((pGD->dprBase + 0x003c),
		(((pGD->plnSizeX * pGD->gdfBytesPP & 0x0fff)<<16) |
		 (pGD->plnSizeX * pGD->gdfBytesPP & 0x0fff)));
	out16r ((pGD->dprBase + 0x001e), 0x0000);

	/* src base adrs */
	out32r ((pGD->dprBase + 0x0040),
		(((pGD->frameAdrs/8) & 0x000fffff)));

	/* dst base adrs */
	out32r ((pGD->dprBase + 0x0044),
		(((pGD->frameAdrs/8) & 0x000fffff)));

	/* foreground color */
	out32r ((pGD->dprBase + 0x0014), pGD->fg);

	/* background color */
	out32r ((pGD->dprBase + 0x0018), pGD->bg);

	/* xcolor */
	out32r ((pGD->dprBase + 0x0020), 0x00ffffff);

	/* xcolor mask */
	out32r ((pGD->dprBase + 0x0024), 0x00ffffff);

	/* bit mask */
	out32r ((pGD->dprBase + 0x0028), 0x00ffffff);

	/* load mono pattern */
	out32r ((pGD->dprBase + 0x0034), 0);
	out32r ((pGD->dprBase + 0x0038), 0);
}

static struct pci_device_id supported[] = {
	{ PCI_VENDOR_ID_SMI, PCI_DEVICE_ID_SMI_710 },
	{ PCI_VENDOR_ID_SMI, PCI_DEVICE_ID_SMI_712 },
	{ PCI_VENDOR_ID_SMI, PCI_DEVICE_ID_SMI_810 },
	{ }
};

/*****************************************************************************/
static void smiLoadMsr (struct ctfb_res_modes *mode)
{
	unsigned char h_synch_high, v_synch_high;
	register GraphicDevice *pGD  = (GraphicDevice *)&smi;

	h_synch_high = (mode->sync & FB_SYNC_HOR_HIGH_ACT) ? 0 : 0x40;	/* horizontal Synch High active */
	v_synch_high = (mode->sync & FB_SYNC_VERT_HIGH_ACT) ? 0 : 0x80;	/* vertical Synch High active */
	out8 (SMI_MISC_REG, (h_synch_high | v_synch_high | 0x29));
	/* upper64K==0x20, CLC2select==0x08, RAMenable==0x02!(todo), CGA==0x01
	 * Selects the upper 64KB page.Bit5=1
	 * CLK2 (left reserved in standard VGA) Bit3|2=1|0
	 * Disables CPU access to frame buffer. Bit1=0
	 * Sets the I/O address decode for ST01, FCR, and all CR registers
	 * to the 3Dx I/O address range (CGA emulation). Bit0=1
	 */
}
/*****************************************************************************/
static void smiLoadCrt (struct ctfb_res_modes *var, int bits_per_pixel)
{
	unsigned char cr[0x7a];
	int i;
	unsigned int hd, hs, he, ht, hbs, hbe;	/* Horizontal.	*/
	unsigned int vd, vs, ve, vt, vbs, vbe;	/* vertical */
	unsigned int bpp, wd, dblscan, interlaced;

	const int LineCompare = 0x3ff;
	unsigned int TextScanLines = 1;	/* this is in fact a vertical zoom factor   */
	register GraphicDevice *pGD  = (GraphicDevice *)&smi;

	/* Horizontal */
	hd = (var->xres) / 8;	/* HDisp.  */
	hs = (var->xres + var->right_margin) / 8;	/* HsStrt  */
	he = (var->xres + var->right_margin + var->hsync_len) / 8;	/* HsEnd   */
	ht = (var->left_margin + var->xres + var->right_margin + var->hsync_len) / 8;	/* HTotal  */
	/* Blank */
	hbs = hd;
	hbe = 0; /* Blank end at 0 */

	/* Vertical */
	vd = var->yres;		/* VDisplay   */
	vs = var->yres + var->lower_margin;	/* VSyncStart */
	ve = var->yres + var->lower_margin + var->vsync_len;	/* VSyncEnd */
	vt = var->upper_margin + var->yres + var->lower_margin + var->vsync_len;	/* VTotal  */
	vbs = vd;
	vbe = 0;

	bpp = bits_per_pixel;
	dblscan = (var->vmode & FB_VMODE_DOUBLE) ? 1 : 0;
	interlaced = var->vmode & FB_VMODE_INTERLACED;


	if (bpp == 15)
		bpp = 16;
	wd = var->xres * bpp / 64;	/* double words per line */
	if (interlaced) {	/* we divide all vertical timings, exept vd */
		vs >>= 1;
		vbs >>= 1;
		ve >>= 1;
		vt >>= 1;
	}

	memset (cr, 0, sizeof (cr));
	cr[0x00] = ht - 5;
	cr[0x01] = hd - 1;
	cr[0x02] = hbs - 1;
	cr[0x03] = (hbe & 0x1F);
	cr[0x04] = hs;
	cr[0x05] = ((hbe & 0x20) << 2) | (he & 0x1f);

	cr[0x06] = (vt - 2) & 0xFF;
	cr[0x07] = (((vt - 2) & 0x100) >> 8)
		| (((vd - 1) & 0x100) >> 7)
		| ((vs & 0x100) >> 6)
		| (((vbs - 1) & 0x100) >> 5)
		| ((LineCompare & 0x100) >> 4)
		| (((vt - 2) & 0x200) >> 4)
		| (((vd - 1) & 0x200) >> 3)
		| ((vs & 0x200) >> 2);

	cr[0x30] = ((vt - 2) & 0x400) >> 7
		| (((vd - 1) & 0x400) >> 8)
		| (((vbs - 1) & 0x400) >> 9)
		| ((vs & 0x400) >> 10)
		| (interlaced) ? 0x80 : 0;


	cr[0x08] = 0x00;
	cr[0x09] = (dblscan << 7)
		| ((LineCompare & 0x200) >> 3)
		| (((vbs - 1) & 0x200) >> 4)
		| (TextScanLines - 1);

	cr[0x10] = vs & 0xff;	/* VSyncPulseStart */
	cr[0x11] = (ve & 0x0f);
	cr[0x12] = (vd - 1) & 0xff;	/* LineCount  */
	cr[0x13] = wd & 0xff;
	cr[0x14] = 0x40;
	cr[0x15] = (vbs - 1) & 0xff;
	cr[0x16] = vbe & 0xff;
	cr[0x17] = 0xe3;	/* but it does not work */
	cr[0x18] = 0xff & LineCompare;
	cr[0x22] = 0x00;	/* todo? */


	/* now set the registers */
	for (i = 0; i <= 0x18; i++) {	/*CR00 .. CR18 */
		smiWrite (SMI_INDX_D4, i, cr[i]);
	}
	i = 0x22;		/*CR22 */
	smiWrite (SMI_INDX_D4, i, cr[i]);
	i = 0x30;		/*CR30 */
	smiWrite (SMI_INDX_D4, i, cr[i]);
}

/*****************************************************************************/
#define REF_FREQ	14318180
#define PMIN		1
#define PMAX		255
#define QMIN		1
#define QMAX		63

static unsigned int FindPQ (unsigned int freq, unsigned int *pp, unsigned int *pq)
{
	unsigned int n = QMIN, m = 0;
	long long int L = 0, P = freq, Q = REF_FREQ, H = P >> 1;
	long long int D = 0x7ffffffffffffffLL;

	for (n = QMIN; n <= QMAX; n++) {
		m = PMIN;	/* p/q ~ freq/ref -> p*ref-freq*q ~ 0 */
		L = P * n - m * Q;
		while (L > 0 && m < PMAX) {
			L -= REF_FREQ;	/* difference is greater as 0 subtract fref */
			m++;	/* and increment m */
		}
		/* difference is less or equal than 0 or m > maximum */
		if (m > PMAX)
			break;	/* no solution: if we increase n we get the same situation */
		/* L is <= 0 now */
		if (-L > H && m > PMIN) {	/* if difference > the half fref */
			L += REF_FREQ;	/* we take the situation before */
			m--;	/* because its closer to 0 */
		}
		L = (L < 0) ? -L : +L;	/* absolute value */
		if (D < L)	/* if last difference was better take next n */
			continue;
		D = L;
		*pp = m;
		*pq = n;	/*  keep improved data */
		if (D == 0)
			break;	/* best result we can get */
	}
	return (unsigned int) (0xffffffff & D);
}

/*****************************************************************************/
static void smiLoadCcr (struct ctfb_res_modes *var, unsigned short device_id)
{
	unsigned int p = 0;
	unsigned int q = 0;
	long long freq;
	register GraphicDevice *pGD  = (GraphicDevice *)&smi;

	smiWrite (SMI_INDX_C4, 0x65, 0);
	smiWrite (SMI_INDX_C4, 0x66, 0);
	smiWrite (SMI_INDX_C4, 0x68, 0x50);
	if (device_id == PCI_DEVICE_ID_SMI_810) {
		smiWrite (SMI_INDX_C4, 0x69, 0x3);
	} else {
		smiWrite (SMI_INDX_C4, 0x69, 0x0);
	}

	/* Memory clock */
	switch (device_id) {
	case PCI_DEVICE_ID_SMI_710 :
		smiWrite (SMI_INDX_C4, 0x6a, 0x75);
		break;
	case PCI_DEVICE_ID_SMI_712 :
		smiWrite (SMI_INDX_C4, 0x6a, 0x80);
		break;
	default :
		smiWrite (SMI_INDX_C4, 0x6a, 0x53);
		break;
	}
	smiWrite (SMI_INDX_C4, 0x6b, 0x15);

	/* VCLK */
	freq = 1000000000000LL / var -> pixclock;

	FindPQ ((unsigned int)freq, &p, &q);

	smiWrite (SMI_INDX_C4, 0x6c, p);
	smiWrite (SMI_INDX_C4, 0x6d, q);

}

/*******************************************************************************
 *
 * Init video chip with common Linux graphic modes (lilo)
 */
void *video_hw_init (void)
{
	GraphicDevice *pGD = (GraphicDevice *)&smi;
	unsigned short device_id;
	pci_dev_t devbusfn;
	int videomode;
	unsigned long t1, hsynch, vsynch;
	unsigned int pci_mem_base, *vm;
	char *penv;
	int tmp, i, bits_per_pixel;
	struct ctfb_res_modes *res_mode;
	struct ctfb_res_modes var_mode;
	unsigned char videoout;

	/* Search for video chip */
	printf("Video: ");

	if ((devbusfn = pci_find_devices(supported, 0)) < 0)
	{
		printf ("Controller not found !\n");
		return (NULL);
	}

	/* PCI setup */
	pci_write_config_dword (devbusfn, PCI_COMMAND, (PCI_COMMAND_MEMORY | PCI_COMMAND_IO));
	pci_read_config_word (devbusfn, PCI_DEVICE_ID, &device_id);
	pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_0, &pci_mem_base);
	pci_mem_base = pci_mem_to_phys (devbusfn, pci_mem_base);

	tmp = 0;

	videomode = CFG_DEFAULT_VIDEO_MODE;
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
			printf ("no VESA Mode found, switching to mode 0x%x ", CFG_DEFAULT_VIDEO_MODE);
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

	pGD->isaBase = CFG_ISA_IO;
	pGD->pciBase = pci_mem_base;
	pGD->dprBase = (pci_mem_base + 0x400000 + 0x8000);
	pGD->vprBase = (pci_mem_base + 0x400000 + 0xc000);
	pGD->cprBase = (pci_mem_base + 0x400000 + 0xe000);
	pGD->frameAdrs = pci_mem_base;
	pGD->memSize = VIDEO_MEM_SIZE;

	/* Set up hardware : select color mode,
	   set Register base to isa 3dx for 3?x regs*/
	out8 (SMI_MISC_REG, 0x01);

	/* Turn off display */
	smiWrite (SMI_INDX_C4, 0x01, 0x20);

	/* Unlock ext. crt regs */
	out8 (SMI_LOCK_REG, 0x40);

	/* Unlock crt regs 0-7 */
	smiWrite (SMI_INDX_D4, 0x11, 0x0e);

	/* Sytem Control Register */
	smiLoadRegs (SMI_INDX_C4, SMI_DATA_C5, SMI_SCR, sizeof(SMI_SCR));

	/* extented CRT Register */
	smiLoadRegs (SMI_INDX_D4, SMI_DATA_D5, SMI_EXT_CRT, sizeof(SMI_EXT_CRT));

	/* Attributes controller registers */
	smiLoadRegs (SMI_INDX_ATTR, SMI_INDX_ATTR, SMI_ATTR, sizeof(SMI_ATTR));

	/* Graphics Controller Register */
	smiLoadRegs (SMI_INDX_CE, SMI_DATA_CF, SMI_GCR, sizeof(SMI_GCR));

	/* Sequencer Register */
	smiLoadRegs (SMI_INDX_C4, SMI_DATA_C5, SMI_SEQR, sizeof(SMI_SEQR));

	/* Power Control Register */
	smiLoadRegs (SMI_INDX_C4, SMI_DATA_C5, SMI_PCR, sizeof(SMI_PCR));

	/* Memory Control Register */
	/* Register MSR62 is a power on configurable register. We don't */
	/* modify it */
	smiLoadRegs (SMI_INDX_C4, SMI_DATA_C5, SMI_MCR, sizeof(SMI_MCR));

	/* Set misc output register */
	smiLoadMsr (res_mode);

	/* Set CRT and Clock control registers */
	smiLoadCrt (res_mode, bits_per_pixel);

	smiLoadCcr (res_mode, device_id);

	/* Hardware Cusor Register */
	smiLoadRegs (SMI_INDX_C4, SMI_DATA_C5, SMI_HCR, sizeof(SMI_HCR));

	/* Enable  Display  */
	videoout = 2;	    /* Default output is CRT */
	if ((penv = getenv ("videoout")) != NULL) {
		/* deceide if it is a string */
		videoout = (int) simple_strtoul (penv, NULL, 16);
	}
	smiWrite (SMI_INDX_C4, 0x31, videoout);

	/* Video processor default setup */
	smiInitVideoProcessor ();

	/* Capture port default setup */
	smiInitCapturePort ();

	/* Drawing engine default setup */
	smiInitDrawingEngine ();

	/* Turn on display */
	smiWrite (0x3c4, 0x01, 0x01);

	/* Clear video memory */
	i = pGD->memSize/4;
	vm = (unsigned int *)pGD->pciBase;
	while(i--)
		*vm++ = 0;
	return ((void*)&smi);
}

/*******************************************************************************
 *
 * Drawing engine fill on screen region
 */
void video_hw_rectfill (
	unsigned int bpp,	      /* bytes per pixel */
	unsigned int dst_x,	      /* dest pos x */
	unsigned int dst_y,	      /* dest pos y */
	unsigned int dim_x,	      /* frame width */
	unsigned int dim_y,	      /* frame height */
	unsigned int color	      /* fill color */
	)
{
	register GraphicDevice *pGD = (GraphicDevice *)&smi;
	register unsigned int control;

	dim_x *= bpp;

	out32r ((pGD->dprBase + 0x0014), color);
	out32r ((pGD->dprBase + 0x0004), ((dst_x<<16) | dst_y));
	out32r ((pGD->dprBase + 0x0008), ((dim_x<<16) | dim_y));

	control = 0x0000ffff &	in32r ((pGD->dprBase + 0x000c));

	control |= 0x80010000;

	out32r ((pGD->dprBase + 0x000c),  control);

	/* Wait for drawing processor */
	do
	{
		out8 ((pGD->isaBase + 0x3c4), 0x16);
	} while (in8 (pGD->isaBase + 0x3c5) & 0x08);
}

/*******************************************************************************
 *
 * Drawing engine bitblt with screen region
 */
void video_hw_bitblt (
	unsigned int bpp,	      /* bytes per pixel */
	unsigned int src_x,	      /* source pos x */
	unsigned int src_y,	      /* source pos y */
	unsigned int dst_x,	      /* dest pos x */
	unsigned int dst_y,	      /* dest pos y */
	unsigned int dim_x,	      /* frame width */
	unsigned int dim_y	      /* frame height */
	)
{
	register GraphicDevice *pGD = (GraphicDevice *)&smi;
	register unsigned int control;

	dim_x *= bpp;

	if ((src_y<dst_y) || ((src_y==dst_y) && (src_x<dst_x)))
	{
		out32r ((pGD->dprBase + 0x0000), (((src_x+dim_x-1)<<16) | (src_y+dim_y-1)));
		out32r ((pGD->dprBase + 0x0004), (((dst_x+dim_x-1)<<16) | (dst_y+dim_y-1)));
		control = 0x88000000;
	} else {
		out32r ((pGD->dprBase + 0x0000), ((src_x<<16) | src_y));
		out32r ((pGD->dprBase + 0x0004), ((dst_x<<16) | dst_y));
		control = 0x80000000;
	}

	out32r ((pGD->dprBase + 0x0008), ((dim_x<<16) | dim_y));
	control |= (0x0000ffff &  in32r ((pGD->dprBase + 0x000c)));
	out32r ((pGD->dprBase + 0x000c), control);

	/* Wait for drawing processor */
	do
	{
		out8 ((pGD->isaBase + 0x3c4), 0x16);
	} while (in8 (pGD->isaBase + 0x3c5) & 0x08);
}

/*******************************************************************************
 *
 * Set a RGB color in the LUT (8 bit index)
 */
void video_set_lut (
	unsigned int index,	      /* color number */
	unsigned char r,	      /* red */
	unsigned char g,	      /* green */
	unsigned char b		      /* blue */
	)
{
	register GraphicDevice *pGD = (GraphicDevice *)&smi;

	out8 (SMI_LUT_MASK,  0xff);

	out8 (SMI_LUT_START, (char)index);

	out8 (SMI_LUT_RGB, r>>2);    /* red */
	udelay (10);
	out8 (SMI_LUT_RGB, g>>2);    /* green */
	udelay (10);
	out8 (SMI_LUT_RGB, b>>2);    /* blue */
	udelay (10);
}

#endif /* CONFIG_VIDEO_SMI_LYNXEM */

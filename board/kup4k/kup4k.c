/*
 * (C) Copyright 2000, 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mpc8xx.h>
#ifdef CONFIG_KUP4K_LOGO
   #include "s1d13706.h"
#endif


typedef struct
{
    volatile unsigned char *VmemAddr;
    volatile unsigned char *RegAddr;
}FB_INFO_S1D13xxx;

/* ------------------------------------------------------------------------- */

#if 0
static long int dram_size (long int, long int *, long int);
#endif

#ifdef CONFIG_KUP4K_LOGO
   void lcd_logo(bd_t *bd);
#endif

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] =
{
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x11ADFC04, 0xEFBBBC00,
	0x1FF77C47, /* last */

	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
		    0x1FF77C35, 0xEFEABC34, 0x1FB57C35, /* last */

	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x10ADFC04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBBBC00, 0x1FF77C47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1F27FC04, 0xEEAEBC00, 0x01B93C04, 0x1FF77C47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEBC00, 0x10AD7C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BBBC04, 0x1FF77C47, /* last */
					    _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FF5FC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, /* last */
				_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7FFFFC07, /* last */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 */

int checkboard (void)
{

	printf ("### No HW ID - assuming KUP4K-Color\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0 = 0;
	long int size_b1 = 0;
	long int size_b2 = 0;

	upmconfig (UPMA, (uint *) sdram_table,
			 sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CFG_MPTPR;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller banks 1 and 2 to the SDRAM banks 2 and 3 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */
/*	memctl->memc_or1 = CFG_OR1_PRELIM;	*/
/*	memctl->memc_br1 = CFG_BR1_PRELIM;	*/

/*	memctl->memc_or2 = CFG_OR2_PRELIM;	*/
/*	memctl->memc_br2 = CFG_BR2_PRELIM;	*/


	memctl->memc_mamr = CFG_MAMR & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80002105;	/* SDRAM bank 0 */
	udelay (1);
	memctl->memc_mcr = 0x80002830;	/* SDRAM bank 0 - execute twice */
	udelay (1);
	memctl->memc_mcr = 0x80002106;	/* SDRAM bank 0 - RUN MRS Pattern from loc 6 */
	udelay (1);

	memctl->memc_mcr = 0x80004105;	/* SDRAM bank 1 */
	udelay (1);
	memctl->memc_mcr = 0x80004830;	/* SDRAM bank 1 - execute twice */
	udelay (1);
	memctl->memc_mcr = 0x80004106;	/* SDRAM bank 1 - RUN MRS Pattern from loc 6 */
	udelay (1);

	memctl->memc_mcr = 0x80006105;	/* SDRAM bank 2 */
	udelay (1);
	memctl->memc_mcr = 0x80006830;	/* SDRAM bank 2 - execute twice */
	udelay (1);
	memctl->memc_mcr = 0x80006106;	/* SDRAM bank 2 - RUN MRS Pattern from loc 6 */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */
	udelay (1000);

#if 0							/* 3 x 8MB */
	size_b0 = 0x00800000;
	size_b1 = 0x00800000;
	size_b2 = 0x00800000;
	memctl->memc_mptpr = CFG_MPTPR;
	udelay (1000);
	memctl->memc_or1 = 0xFF800A00;
	memctl->memc_br1 = 0x00000081;
	memctl->memc_or2 = 0xFF000A00;
	memctl->memc_br2 = 0x00800081;
	memctl->memc_or3 = 0xFE000A00;
	memctl->memc_br3 = 0x01000081;
#else							/* 3 x 16 MB */
	size_b0 = 0x01000000;
	size_b1 = 0x01000000;
	size_b2 = 0x01000000;
	memctl->memc_mptpr = CFG_MPTPR;
	udelay (1000);
	memctl->memc_or1 = 0xFF000A00;
	memctl->memc_br1 = 0x00000081;
	memctl->memc_or2 = 0xFE000A00;
	memctl->memc_br2 = 0x01000081;
	memctl->memc_or3 = 0xFC000A00;
	memctl->memc_br3 = 0x02000081;
#endif

	udelay (10000);

	return (size_b0 + size_b1 + size_b2);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */
#if 0
static long int dram_size (long int mamr_value, long int *base,
						   long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	volatile long int *addr;
	ulong cnt, val;
	ulong save[32];				/* to make test non-destructive */
	unsigned char i = 0;

	memctl->memc_mamr = mamr_value;

	for (cnt = maxsize / sizeof (long); cnt > 0; cnt >>= 1) {
		addr = base + cnt;		/* pointer arith! */

		save[i++] = *addr;
		*addr = ~cnt;
	}

	/* write 0 to base address */
	addr = base;
	save[i] = *addr;
	*addr = 0;

	/* check at base address */
	if ((val = *addr) != 0) {
		*addr = save[i];
		return (0);
	}

	for (cnt = 1; cnt <= maxsize / sizeof (long); cnt <<= 1) {
		addr = base + cnt;		/* pointer arith! */

		val = *addr;
		*addr = save[--i];

		if (val != (~cnt)) {
			return (cnt * sizeof (long));
		}
	}
	return (maxsize);
}
#endif

int misc_init_r (void)
{
	DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_STATUS_LED
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
#endif
#ifdef CONFIG_KUP4K_LOGO
	bd_t *bd = gd->bd;


	lcd_logo (bd);
#endif							/* CONFIG_KUP4K_LOGO */
#ifdef CONFIG_IDE_LED
	/* Configure PA8 as output port */
	immap->im_ioport.iop_padir |= 0x80;
	immap->im_ioport.iop_paodr |= 0x80;
	immap->im_ioport.iop_papar &= ~0x80;
	immap->im_ioport.iop_padat |= 0x80;	/* turn it off */
#endif
	return (0);
}

#ifdef CONFIG_KUP4K_LOGO


#define PB_LCD_PWM	((uint)0x00004000)	/* PB 17 */

void lcd_logo (bd_t * bd)
{
	FB_INFO_S1D13xxx fb_info;
	S1D_INDEX s1dReg;
	S1D_VALUE s1dValue;
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl;
	ushort i;
	uchar *fb;
	int rs, gs, bs;
	int r = 8, g = 8, b = 4;
	int r1, g1, b1;

	immr->im_cpm.cp_pbpar &= ~PB_LCD_PWM;
	immr->im_cpm.cp_pbodr &= ~PB_LCD_PWM;
	immr->im_cpm.cp_pbdat &= ~PB_LCD_PWM;	/* set to 0 = enabled */
	immr->im_cpm.cp_pbdir |= PB_LCD_PWM;


/*----------------------------------------------------------------------------- */
	 /**/
/* Initialize the chip and the frame buffer driver. */
			 /**/
/*----------------------------------------------------------------------------- */
			memctl = &immr->im_memctl;
/*	memctl->memc_or5 = 0xFFC007F0;    / * 4 MB  17 WS or externel TA */
/*	memctl->memc_br5 = 0x80000801;    / * Start at 0x80000000 */

	memctl->memc_or5 = 0xFFC00708;	/* 4 MB  17 WS or externel TA */
	memctl->memc_br5 = 0x80080801;	/* Start at 0x80080000 */





	fb_info.VmemAddr = (unsigned char *) (S1D_PHYSICAL_VMEM_ADDR);
	fb_info.RegAddr = (unsigned char *) (S1D_PHYSICAL_REG_ADDR);

	if ((((S1D_VALUE *) fb_info.RegAddr)[0] != 0x28)
		|| (((S1D_VALUE *) fb_info.RegAddr)[1] != 0x14)) {
		printf ("Warning:LCD Controller S1D13706 not found\n");
		return;
	}

	/* init controller */
	for (i = 0; i < sizeof (aS1DRegs) / sizeof (aS1DRegs[0]); i++) {
		s1dReg = aS1DRegs[i].Index;
		s1dValue = aS1DRegs[i].Value;
/*      printf("sid1 Index: %02x Register: %02x Wert: %02x\n",i, aS1DRegs[i].Index, aS1DRegs[i].Value); */
		((S1D_VALUE *) fb_info.RegAddr)[s1dReg / sizeof (S1D_VALUE)] =
				s1dValue;
	}

#undef MONOCHROME
#ifdef MONOCHROME
	switch (bd->bi_busfreq) {
#if 0
	case 24000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x32;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x28;
		break;
	case 32000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x32;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x33;
		break;
#endif
	case 40000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x32;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x40;
		break;
	case 48000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x32;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x4C;
		break;
	default:
		printf ("KUP4K S1D1: unknown busfrequency: %ld assuming 64 MHz\n",
				bd->bi_busfreq);
	case 64000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x32;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x69;
		break;
	}
	((S1D_VALUE *) fb_info.RegAddr)[0x10] = 0x00;
#else
	switch (bd->bi_busfreq) {
#if 0
	case 24000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x22;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x34;
		break;
	case 32000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x32;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x34;
		break;
#endif
	case 40000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x32;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x41;
		break;
	case 48000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x22;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x34;
		break;
	default:
		printf ("KUP4K S1D1: unknown busfrequency: %ld assuming 64 MHz\n",
				bd->bi_busfreq);
	case 64000000:
		((S1D_VALUE *) fb_info.RegAddr)[0x05] = 0x32;
		((S1D_VALUE *) fb_info.RegAddr)[0x12] = 0x66;
		break;
	}
#endif


	/* create and set colormap */
	rs = 256 / (r - 1);
	gs = 256 / (g - 1);
	bs = 256 / (b - 1);
	for (i = 0; i < 256; i++) {
		r1 = (rs * ((i / (g * b)) % r)) * 255;
		g1 = (gs * ((i / b) % g)) * 255;
		b1 = (bs * ((i) % b)) * 255;
/*     printf("%d %04x %04x %04x\n",i,r1>>4,g1>>4,b1>>4); */
		S1D_WRITE_PALETTE (fb_info.RegAddr, i, (r1 >> 4), (g1 >> 4),
						   (b1 >> 4));
	}

	/* copy bitmap */
	fb = (char *) (fb_info.VmemAddr);
	memcpy (fb, (uchar *) CONFIG_KUP4K_LOGO, 320 * 240);
}
#endif							/* CONFIG_KUP4K_LOGO */

#ifdef CONFIG_IDE_LED
void ide_led (uchar led, uchar status)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;

	/* We have one led for both pcmcia slots */
	if (status) {				/* led on */
		immap->im_ioport.iop_padat &= ~0x80;
	} else {
		immap->im_ioport.iop_padat |= 0x80;
	}
}
#endif

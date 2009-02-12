/*
 * (C) Copyright 2000-2004
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
#include <post.h>
#include "../common/kup.h"
#ifdef CONFIG_KUP4K_LOGO
/* #include "s1d13706.h" */
#endif

#define KUP4X_USB


typedef struct {
	volatile unsigned char *VmemAddr;
	volatile unsigned char *RegAddr;
} FB_INFO_S1D13xxx;

/* ------------------------------------------------------------------------- */

int usb_init_kup4x (void);


#ifdef CONFIG_KUP4K_LOGO
void lcd_logo (bd_t * bd);
#endif

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x11ADFC04, 0xEFBBBC00,
	0x1FF77C47,		/* last */

	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1FF77C35, 0xEFEABC34, 0x1FB57C35,	/* last */

	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x10ADFC04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBBBC00, 0x1FF77C47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1F27FC04, 0xEEAEBC00, 0x01B93C04, 0x1FF77C47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEBC00, 0x10AD7C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BBBC04, 0x1FF77C47,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FF5FC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07,	/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7FFFFC07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	volatile uchar *latch;
	uchar rev, mod;

	/*
	 * Init ChipSelect #4 (CAN + HW-Latch)
	 */
	memctl->memc_or4 = 0xFFFF8926;
	memctl->memc_br4 = 0x90000401;
	__asm__ ("eieio");
	latch = (volatile uchar *) 0x90000200;
	rev = (*latch & 0xF8) >> 3;
	mod = (*latch & 0x03);
	printf ("Board: KUP4X Rev %d.%d\n",rev,mod);
	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0 = 0;
	long int size_b1 = 0;
	long int size_b2 = 0;
	long int size_b3 = 0;

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));
	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller banks 1 and 2 to the SDRAM banks 2 and 3 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */
/*	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;	*/
/*	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;	*/

/*	memctl->memc_or2 = CONFIG_SYS_OR2_PRELIM;	*/
/*	memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;	*/

	memctl->memc_mamr = CONFIG_SYS_MAMR & (~(MAMR_PTAE));	/* no refresh yet */

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

	memctl->memc_mcr = 0x8000C105;	/* SDRAM bank 2 */
	udelay (1);
	memctl->memc_mcr = 0x8000C830;	/* SDRAM bank 2 - execute twice */
	udelay (1);
	memctl->memc_mcr = 0x8000C106;	/* SDRAM bank 2 - RUN MRS Pattern from loc 6 */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */
	udelay (1000);
#if 0				/* 4 x 8MB */
	size_b0 = 0x00800000;
	size_b1 = 0x00800000;
	size_b2 = 0x00800000;
	size_b3 = 0x00800000;
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;
	udelay (1000);
	memctl->memc_or1 = 0xFF800A00;
	memctl->memc_br1 = 0x00000081;
	memctl->memc_or2 = 0xFF000A00;
	memctl->memc_br2 = 0x00800081;
	memctl->memc_or3 = 0xFE000A00;
	memctl->memc_br3 = 0x01000081;
	memctl->memc_or6 = 0xFE000A00;
	memctl->memc_br6 = 0x01800081;
#else  /* 4 x 16 MB */
	size_b0 = 0x01000000;
	size_b1 = 0x01000000;
	size_b2 = 0x01000000;
	size_b3 = 0x01000000;
	memctl->memc_mptpr = CONFIG_SYS_MPTPR;
	udelay (1000);
	memctl->memc_or1 = 0xFF000A00;
	memctl->memc_br1 = 0x00000081;
	memctl->memc_or2 = 0xFE000A00;
	memctl->memc_br2 = 0x01000081;
	memctl->memc_or3 = 0xFD000A00;
	memctl->memc_br3 = 0x02000081;
	memctl->memc_or6 = 0xFC000A00;
	memctl->memc_br6 = 0x03000081;
#endif
	udelay (10000);

	return (size_b0 + size_b1 + size_b2 + size_b3);
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
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	volatile long int *addr;
	ulong cnt, val;
	ulong save[32];		/* to make test non-destructive */
	unsigned char i = 0;

	memctl->memc_mamr = mamr_value;

	for (cnt = maxsize / sizeof (long); cnt > 0; cnt >>= 1) {
		addr = base + cnt;	/* pointer arith! */

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
		addr = base + cnt;	/* pointer arith! */

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
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
#ifdef CONFIG_IDE_LED
	/* Configure PA8 as output port */
	immap->im_ioport.iop_padir |= 0x80;
	immap->im_ioport.iop_paodr |= 0x80;
	immap->im_ioport.iop_papar &= ~0x80;
	immap->im_ioport.iop_padat |= 0x80;	/* turn it off */
#endif
#ifdef KUP4X_USB
	usb_init_kup4x ();
#endif
	load_sernum_ethaddr();
	setenv ("hw", "4x");
	poweron_key ();
	return (0);
}

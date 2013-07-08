/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>
#include <post.h>
#include "../common/kup.h"
#include <asm/io.h>


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


/*
 * Check Board Identity:
 */

int checkboard(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	uchar latch, rev, mod;

	/*
	 * Init ChipSelect #4 (CAN + HW-Latch)
	 */
	out_be32(&memctl->memc_or4, 0xFFFF8926);
	out_be32(&memctl->memc_br4, 0x90000401);

	latch = in_8( (unsigned char *) LATCH_ADDR);
	rev = (latch & 0xF8) >> 3;
	mod = (latch & 0x03);

	printf("Board: KUP4X Rev %d.%d\n", rev, mod);

	return 0;
}


phys_size_t initdram(int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	upmconfig(UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	out_be16(&memctl->memc_mptpr, CONFIG_SYS_MPTPR);

	out_be32(&memctl->memc_mar, 0x00000088);

	out_be32(&memctl->memc_mamr,
		 CONFIG_SYS_MAMR & (~(MAMR_PTAE))); /* no refresh yet */

	udelay(200);

	/* perform SDRAM initializsation sequence */

	/* SDRAM bank 0 */
	out_be32(&memctl->memc_mcr, 0x80002105);
	udelay(1);
	out_be32(&memctl->memc_mcr, 0x80002830); /* execute twice */
	udelay(1);
	out_be32(&memctl->memc_mcr, 0x80002106); /* RUN MRS Pattern from loc 6 */
	udelay(1);

	/* SDRAM bank 1 */
	out_be32(&memctl->memc_mcr, 0x80004105);
	udelay(1);
	out_be32(&memctl->memc_mcr, 0x80004830); /* execute twice */
	udelay(1);
	out_be32(&memctl->memc_mcr, 0x80004106); /* RUN MRS Pattern from loc 6 */
	udelay(1);

	/* SDRAM bank 2 */
	out_be32(&memctl->memc_mcr, 0x80006105);
	udelay(1);
	out_be32(&memctl->memc_mcr, 0x80006830); /* execute twice */
	udelay(1);
	out_be32(&memctl->memc_mcr, 0x80006106); /* RUN MRS Pattern from loc 6 */
	udelay(1);

	/* SDRAM bank 3 */
	out_be32(&memctl->memc_mcr, 0x8000C105);
	udelay(1);
	out_be32(&memctl->memc_mcr, 0x8000C830); /* execute twice */
	udelay(1);
	out_be32(&memctl->memc_mcr, 0x8000C106); /* RUN MRS Pattern from loc 6 */
	udelay(1);

	setbits_be32(&memctl->memc_mamr, MAMR_PTAE); /* enable refresh */

	udelay(1000);
	/* 4 x 16 MB */
	out_be16(&memctl->memc_mptpr, CONFIG_SYS_MPTPR);
	udelay(1000);
	out_be32(&memctl->memc_or1, 0xFF000A00);
	out_be32(&memctl->memc_br1, 0x00000081);
	out_be32(&memctl->memc_or2, 0xFE000A00);
	out_be32(&memctl->memc_br2, 0x01000081);
	out_be32(&memctl->memc_or3, 0xFD000A00);
	out_be32(&memctl->memc_br3, 0x02000081);
	out_be32(&memctl->memc_or6, 0xFC000A00);
	out_be32(&memctl->memc_br6, 0x03000081);
	udelay(10000);

	return (4 * 16 * 1024 * 1024);
}

int misc_init_r(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

#ifdef CONFIG_IDE_LED
	/* Configure PA8 as output port */
	setbits_be16(&immap->im_ioport.iop_padir, PA_8);
	setbits_be16(&immap->im_ioport.iop_paodr, PA_8);
	clrbits_be16(&immap->im_ioport.iop_papar, PA_8);
	setbits_be16(&immap->im_ioport.iop_padat, PA_8); /* turn it off */
#endif
	load_sernum_ethaddr();
	setenv("hw", "4x");
	poweron_key();
	return 0;
}

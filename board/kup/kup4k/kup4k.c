/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <libfdt.h>
#include <mpc8xx.h>
#include <hwconfig.h>
#include <i2c.h>
#include "../common/kup.h"
#include <asm/io.h>

static unsigned char swapbyte(unsigned char c);
static int read_diag(void);

DECLARE_GLOBAL_DATA_PTR;

/* ----------------------------------------------------------------------- */

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

/* ----------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	uchar rev,mod,tmp,pcf,ak_rev,ak_mod;

	/*
	 * Init ChipSelect #4 (CAN + HW-Latch)
	 */
	out_be32(&immap->im_memctl.memc_or4, CONFIG_SYS_OR4);
	out_be32(&immap->im_memctl.memc_br4, CONFIG_SYS_BR4);

	/*
	 * Init ChipSelect #5 (S1D13768)
	 */
	out_be32(&immap->im_memctl.memc_or5, CONFIG_SYS_OR5);
	out_be32(&immap->im_memctl.memc_br5, CONFIG_SYS_BR5);

	tmp = swapbyte(in_8((unsigned char*) LATCH_ADDR));
	rev = (tmp & 0xF8) >> 3;
	mod = (tmp & 0x07);

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	if (read_diag())
		gd->flags &= ~GD_FLG_SILENT;

	printf("Board: KUP4K Rev %d.%d AK:",rev,mod);
	/*
	 * TI Application report: Before using the IO as an input,
	 * a high must be written to the IO first
	 */
	pcf = 0xFF;
	i2c_write(0x21, 0, 0 , &pcf, 1);
	if (i2c_read(0x21, 0, 0, &pcf, 1)) {
		puts("n/a\n");
	} else {
		ak_rev = (pcf & 0xF8) >> 3;
		ak_mod = (pcf & 0x07);
		printf("%d.%d\n", ak_rev, ak_mod);
	}
	return 0;
}

/* ----------------------------------------------------------------------- */


phys_size_t initdram(int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size = 0;
	uchar *latch, rev, tmp;

	/*
	 * Init ChipSelect #4 (CAN + HW-Latch) to determine Hardware Revision
	 * Rev 1..6 -> 48 MB RAM;   Rev >= 7 -> 96 MB
	 */
	out_be32(&immap->im_memctl.memc_or4, CONFIG_SYS_OR4);
	out_be32(&immap->im_memctl.memc_br4, CONFIG_SYS_BR4);

	latch = (uchar *)0x90000200;
	tmp = swapbyte(*latch);
	rev = (tmp & 0xF8) >> 3;

	upmconfig(UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	out_be16(&memctl->memc_mptpr, CONFIG_SYS_MPTPR);

	out_be32(&memctl->memc_mar, 0x00000088);
	/* no refresh yet */
	if(rev >= 7) {
		out_be32(&memctl->memc_mamr,
				 CONFIG_SYS_MAMR_9COL & (~(MAMR_PTAE)));
	} else {
		out_be32(&memctl->memc_mamr,
				 CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE)));
	}

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

	setbits_be32(&memctl->memc_mamr, MAMR_PTAE); /* enable refresh */
	udelay(1000);

	out_be16(&memctl->memc_mptpr, CONFIG_SYS_MPTPR);
	udelay(1000);
	if(rev >= 7) {
		size = 32 * 3 * 1024 * 1024;
		out_be32(&memctl->memc_or1, CONFIG_SYS_OR1_9COL);
		out_be32(&memctl->memc_br1, CONFIG_SYS_BR1_9COL);
		out_be32(&memctl->memc_or2, CONFIG_SYS_OR2_9COL);
		out_be32(&memctl->memc_br2, CONFIG_SYS_BR2_9COL);
		out_be32(&memctl->memc_or3, CONFIG_SYS_OR3_9COL);
		out_be32(&memctl->memc_br3, CONFIG_SYS_BR3_9COL);
	} else {
		size = 16 * 3 * 1024 * 1024;
		out_be32(&memctl->memc_or1, CONFIG_SYS_OR1_8COL);
		out_be32(&memctl->memc_br1, CONFIG_SYS_BR1_8COL);
		out_be32(&memctl->memc_or2, CONFIG_SYS_OR2_8COL);
		out_be32(&memctl->memc_br2, CONFIG_SYS_BR2_8COL);
		out_be32(&memctl->memc_or3, CONFIG_SYS_OR3_8COL);
		out_be32(&memctl->memc_br3, CONFIG_SYS_BR3_8COL);
	}
	return (size);
}

/* ----------------------------------------------------------------------- */


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
	setenv("hw","4k");
	poweron_key();
	return (0);
}


static int read_diag(void)
{
	int diag;
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	clrbits_be16(&immr->im_ioport.iop_pcdir, PC_4);	/* input */
	clrbits_be16(&immr->im_ioport.iop_pcpar, PC_4);	/* gpio */
	setbits_be16(&immr->im_ioport.iop_pcdir, PC_5);	/* output */
	clrbits_be16(&immr->im_ioport.iop_pcpar, PC_4);	/* gpio */
	setbits_be16(&immr->im_ioport.iop_pcdat, PC_5);	/* 1 */
	udelay(500);
	if (in_be16(&immr->im_ioport.iop_pcdat) & PC_4) {
		clrbits_be16(&immr->im_ioport.iop_pcdat, PC_5);/* 0 */
		udelay(500);
		if(in_be16(&immr->im_ioport.iop_pcdat) & PC_4)
			diag = 0;
		else
			diag = 1;
	} else {
		diag = 0;
	}
	clrbits_be16(&immr->im_ioport.iop_pcdir, PC_5);	/* input */
	return (diag);
}

static unsigned char swapbyte(unsigned char c)
{
	unsigned char result = 0;
	int i = 0;

	for(i = 0; i < 8; ++i) {
		result = result << 1;
		result |= (c & 1);
		c = c >> 1;
	}
	return result;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

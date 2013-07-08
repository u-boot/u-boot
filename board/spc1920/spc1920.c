/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Modified by, Yuli Barcohen, Arabella Software Ltd., yuli@arabellasw.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <mpc8xx.h>
#include "pld.h"
#include "hpi.h"

#define	_NOT_USED_	0xFFFFFFFF

static long int dram_size (long int, long int *, long int);

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMB RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x11ADFC04, 0xEFBBBC00,
	0x1FF77C47, /* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMB RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1FF77C34, 0xEFEABC34, 0x1FB57C35, /* last */
	/*
	 * Burst Read. (Offset 8 in UPMB RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x10ADFC04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBBBC00, 0x1FF77C47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMB RAM)
	 */
	0x1F07FC04, 0xEEAEBC00, 0x01B93C04, 0x1FF77C47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMB RAM)
	 */
	0x1F07FC04, 0xEEAEBC00, 0x10AD7C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BBBC04, 0x1FF77C47, /* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMB RAM)
	 */
	0x1FF5FC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, /* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMB RAM)
	 */
	0x7FFFFC07, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};

phys_size_t initdram (int board_type)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;
	/* volatile spc1920_pld_t *pld = (spc1920_pld_t *) CONFIG_SYS_SPC1920_PLD_BASE; */

	long int size_b0;
	long int size8, size9;
	int i;

	/*
	 * Configure UPMB for SDRAM
	 */
	upmconfig (UPMB, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

	udelay(100);

	memctl->memc_mptpr = CONFIG_SYS_MPTPR;

	/* burst length=4, burst type=sequential, CAS latency=2 */
	memctl->memc_mar = CONFIG_SYS_MAR;

	/*
	 * Map controller bank 1 to the SDRAM bank at preliminary address.
	 */
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;

	/* initialize memory address register */
	memctl->memc_mbmr = CONFIG_SYS_MBMR_8COL; /* refresh not enabled yet */

	/* mode initialization (offset 5) */
	udelay (200);				/* 0x80006105 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_B | MCR_MB_CS1 | MCR_MLCF (1) | MCR_MAD (0x05);

	/* run 2 refresh sequence with 4-beat refresh burst (offset 0x30) */
	udelay (1);				/* 0x80006130 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_B | MCR_MB_CS1 | MCR_MLCF (1) | MCR_MAD (0x30);
	udelay (1);				/* 0x80006130 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_B | MCR_MB_CS1 | MCR_MLCF (1) | MCR_MAD (0x30);
	udelay (1);				/* 0x80006106 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_B | MCR_MB_CS1 | MCR_MLCF (1) | MCR_MAD (0x06);

	memctl->memc_mbmr |= MBMR_PTBE;	/* refresh enabled */

	udelay (200);

	/* Need at least 10 DRAM accesses to stabilize */
	for (i = 0; i < 10; ++i) {
		volatile unsigned long *addr =
			(volatile unsigned long *) CONFIG_SYS_SDRAM_BASE;
		unsigned long val;

		val = *(addr + i);
		*(addr + i) = val;
	}

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CONFIG_SYS_MBMR_8COL, (long *)CONFIG_SYS_SDRAM_BASE, SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MBMR_9COL, (long *)CONFIG_SYS_SDRAM_BASE, SDRAM_MAX_SIZE);

	if (size8 < size9) {		/* leave configuration at 9 columns */
		size_b0 = size9;
		memctl->memc_mbmr = CONFIG_SYS_MBMR_9COL | MBMR_PTBE;
		udelay (500);
	} else {			/* back to 8 columns            */
		size_b0 = size8;
		memctl->memc_mbmr = CONFIG_SYS_MBMR_8COL | MBMR_PTBE;
		udelay (500);
	}

	/*
	 * Final mapping:
	 */

	memctl->memc_or1 = ((-size_b0) & 0xFFFF0000) |
			OR_CSNT_SAM | OR_G5LS | SDRAM_TIMING;
	memctl->memc_br1 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMB | BR_V;
	udelay (1000);

	/* initalize the DSP Host Port Interface */
	hpi_init();

	/* FRAM Setup */
	memctl->memc_or4 = CONFIG_SYS_OR4;
	memctl->memc_br4 = CONFIG_SYS_BR4;
	udelay(1000);

	return (size_b0);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */
static long int dram_size (long int mbmr_value, long int *base,
			   long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mbmr = mbmr_value;

	return (get_ram_size (base, maxsize));
}


/************* other stuff ******************/


int board_early_init_f(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	/* Set Go/NoGo led (PA15) to color red */
	immap->im_ioport.iop_papar &= ~0x1;
	immap->im_ioport.iop_paodr &= ~0x1;
	immap->im_ioport.iop_padir |= 0x1;
	immap->im_ioport.iop_padat |= 0x1;

#if 0
	/* Turn on LED PD9 */
	immap->im_ioport.iop_pdpar &= ~(0x0040);
	immap->im_ioport.iop_pddir |= 0x0040;
	immap->im_ioport.iop_pddat |= 0x0040;
#endif

	/*
	 * Enable console on SMC1. This requires turning on
	 * the com2_en signal and SMC1_DISABLE
	 */

	/* SMC1_DISABLE: PB17 */
	immap->im_cpm.cp_pbodr &= ~0x4000;
	immap->im_cpm.cp_pbpar &= ~0x4000;
	immap->im_cpm.cp_pbdir |= 0x4000;
	immap->im_cpm.cp_pbdat &= ~0x4000;

	/* COM2_EN: PD10 */
	immap->im_ioport.iop_pdpar &= ~0x0020;
	immap->im_ioport.iop_pddir &= ~0x4000;
	immap->im_ioport.iop_pddir |= 0x0020;
	immap->im_ioport.iop_pddat |= 0x0020;


#ifdef CONFIG_SYS_SMC1_PLD_CLK4 /* SMC1 uses CLK4 from PLD */
	immap->im_cpm.cp_simode |= 0x7000;
	immap->im_cpm.cp_simode &= ~(0x8000);
#endif

	return 0;
}

int last_stage_init(void)
{
#ifdef CONFIG_SPC1920_HPI_TEST
	printf("CMB1920 Host Port Interface Test: %s\n",
	       hpi_test() ? "Failed!" : "OK");
#endif
	return 0;
}

int checkboard (void)
{
	puts("Board: SPC1920\n");
	return 0;
}

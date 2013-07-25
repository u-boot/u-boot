/*
 *    Copyright (c) 2008 Nuovation System Designs, LLC
 *	Grant Erickson <gerickson@nuovations.com>
 *
 *    Copyright (c) 2007-2009 DENX Software Engineering, GmbH
 *	Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 *    Description:
 *	This file implements ECC initialization for PowerPC processors
 *	using the IBM SDRAM DDR1 & DDR2 controller.
 */

#ifndef _ECC_H_
#define _ECC_H_

/*
 * Since the IBM DDR controller used on 440GP/GX/EP/GR is not register
 * compatible to the IBM DDR/2 controller used on 405EX/440SP/SPe/460EX/GT
 * we need to make some processor dependant defines used later on by the
 * driver.
 */

/* For 440GP/GX/EP/GR */
#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR)
#define SDRAM_MCOPT1		SDRAM_CFG0
#define SDRAM_MCOPT1_MCHK_MASK	SDRAM_CFG0_MCHK_MASK
#define SDRAM_MCOPT1_MCHK_NON	SDRAM_CFG0_MCHK_NON
#define SDRAM_MCOPT1_MCHK_GEN	SDRAM_CFG0_MCHK_GEN
#define SDRAM_MCOPT1_MCHK_CHK	SDRAM_CFG0_MCHK_CHK
#define SDRAM_MCOPT1_MCHK_CHK_REP SDRAM_CFG0_MCHK_CHK
#define SDRAM_MCOPT1_DMWD_MASK	SDRAM_CFG0_DMWD_MASK
#define SDRAM_MCOPT1_DMWD_32	SDRAM_CFG0_DMWD_32

#define SDRAM_MCSTAT		SDRAM0_MCSTS
#define SDRAM_MCSTAT_IDLE_MASK	SDRAM_MCSTS_CIS
#define SDRAM_MCSTAT_IDLE_NOT	SDRAM_MCSTS_IDLE_NOT

#define SDRAM_ECCES		SDRAM0_ECCESR
#endif

void ecc_init(unsigned long * const start, unsigned long size);
void do_program_ecc(unsigned long tlb_word2_i_value);

static void inline blank_string(int size)
{
	int i;

	for (i = 0; i < size; i++)
		putc('\b');
	for (i = 0; i < size; i++)
		putc(' ');
	for (i = 0; i < size; i++)
		putc('\b');
}

#endif /* _ECC_H_ */

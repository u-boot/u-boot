/*
 * (C) Copyright 2017
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/fmc.h>
#include <asm/arch/stm32.h>

static inline u32 _ns2clk(u32 ns, u32 freq)
{
	u32 tmp = freq/1000000;
	return (tmp * ns) / 1000;
}

#define NS2CLK(ns) (_ns2clk(ns, freq))

/*
 * Following are timings for IS42S16400J, from corresponding datasheet
 */
#define SDRAM_CAS	3	/* 3 cycles */
#define SDRAM_NB	1	/* Number of banks */
#define SDRAM_MWID	1	/* 16 bit memory */

#define SDRAM_NR	0x1	/* 12-bit row */
#define SDRAM_NC	0x0	/* 8-bit col */
#define SDRAM_RBURST	0x1	/* Single read requests always as bursts */
#define SDRAM_RPIPE	0x0	/* No HCLK clock cycle delay */

#define SDRAM_TRRD	NS2CLK(12)
#define SDRAM_TRCD	NS2CLK(18)
#define SDRAM_TRP	NS2CLK(18)
#define SDRAM_TRAS	NS2CLK(42)
#define SDRAM_TRC	NS2CLK(60)
#define SDRAM_TRFC	NS2CLK(60)
#define SDRAM_TCDL	(1 - 1)
#define SDRAM_TRDL	NS2CLK(12)
#define SDRAM_TBDL	(1 - 1)
#define SDRAM_TREF	(NS2CLK(64000000 / 8192) - 20)
#define SDRAM_TCCD	(1 - 1)

#define SDRAM_TXSR	SDRAM_TRFC	/* Row cycle time after precharge */
#define SDRAM_TMRD	1		/* Page 10, Mode Register Set */


/* Last data in to row precharge, need also comply ineq on page 1648 */
#define SDRAM_TWR	max(\
		(int)max((int)SDRAM_TRDL, (int)(SDRAM_TRAS - SDRAM_TRCD)), \
		(int)(SDRAM_TRC - SDRAM_TRCD - SDRAM_TRP)\
		)


#define SDRAM_MODE_BL_SHIFT	0
#define SDRAM_MODE_CAS_SHIFT	4
#define SDRAM_MODE_BL		0
#define SDRAM_MODE_CAS		SDRAM_CAS

int stm32_sdram_init(void)
{
	u32 freq;

	/*
	 * Get frequency for NS2CLK calculation.
	 */
	freq = clock_get(CLOCK_AHB) / CONFIG_SYS_RAM_FREQ_DIV;

	writel(CONFIG_SYS_RAM_FREQ_DIV << FMC_SDCR_SDCLK_SHIFT
			| SDRAM_CAS << FMC_SDCR_CAS_SHIFT
			| SDRAM_NB << FMC_SDCR_NB_SHIFT
			| SDRAM_MWID << FMC_SDCR_MWID_SHIFT
			| SDRAM_NR << FMC_SDCR_NR_SHIFT
			| SDRAM_NC << FMC_SDCR_NC_SHIFT
			| SDRAM_RPIPE << FMC_SDCR_RPIPE_SHIFT
			| SDRAM_RBURST << FMC_SDCR_RBURST_SHIFT,
			&STM32_SDRAM_FMC->sdcr1);

	writel(SDRAM_TRCD << FMC_SDTR_TRCD_SHIFT
			| SDRAM_TRP << FMC_SDTR_TRP_SHIFT
			| SDRAM_TWR << FMC_SDTR_TWR_SHIFT
			| SDRAM_TRC << FMC_SDTR_TRC_SHIFT
			| SDRAM_TRAS << FMC_SDTR_TRAS_SHIFT
			| SDRAM_TXSR << FMC_SDTR_TXSR_SHIFT
			| SDRAM_TMRD << FMC_SDTR_TMRD_SHIFT,
			&STM32_SDRAM_FMC->sdtr1);

	writel(FMC_SDCMR_BANK_1 | FMC_SDCMR_MODE_START_CLOCK,
	       &STM32_SDRAM_FMC->sdcmr);
	udelay(200);	/* 200 us delay, page 10, "Power-Up" */
	FMC_BUSY_WAIT();

	writel(FMC_SDCMR_BANK_1 | FMC_SDCMR_MODE_PRECHARGE,
	       &STM32_SDRAM_FMC->sdcmr);
	udelay(100);
	FMC_BUSY_WAIT();

	writel((FMC_SDCMR_BANK_1 | FMC_SDCMR_MODE_AUTOREFRESH
		| 7 << FMC_SDCMR_NRFS_SHIFT), &STM32_SDRAM_FMC->sdcmr);
	udelay(100);
	FMC_BUSY_WAIT();

	writel(FMC_SDCMR_BANK_1 | (SDRAM_MODE_BL << SDRAM_MODE_BL_SHIFT
	       | SDRAM_MODE_CAS << SDRAM_MODE_CAS_SHIFT)
	       << FMC_SDCMR_MODE_REGISTER_SHIFT | FMC_SDCMR_MODE_WRITE_MODE,
	       &STM32_SDRAM_FMC->sdcmr);
	udelay(100);
	FMC_BUSY_WAIT();

	writel(FMC_SDCMR_BANK_1 | FMC_SDCMR_MODE_NORMAL,
	       &STM32_SDRAM_FMC->sdcmr);
	FMC_BUSY_WAIT();

	/* Refresh timer */
	writel(SDRAM_TREF, &STM32_SDRAM_FMC->sdrtr);

	return 0;
}

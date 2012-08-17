/*
 * Jz4740 common routines
 * Copyright (c) 2006 Ingenic Semiconductor, <jlwei@ingenic.cn>
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

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/jz4740.h>

void enable_interrupts(void)
{
}

int disable_interrupts(void)
{
	return 0;
}

/*
 * PLL output clock = EXTAL * NF / (NR * NO)
 * NF = FD + 2, NR = RD + 2
 * NO = 1 (if OD = 0), NO = 2 (if OD = 1 or 2), NO = 4 (if OD = 3)
 */
void pll_init(void)
{
	struct jz4740_cpm *cpm = (struct jz4740_cpm *)JZ4740_CPM_BASE;

	register unsigned int cfcr, plcr1;
	int n2FR[33] = {
		0, 0, 1, 2, 3, 0, 4, 0, 5, 0, 0, 0, 6, 0, 0, 0,
		7, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0,
		9
	};
	int div[5] = {1, 3, 3, 3, 3}; /* divisors of I:S:P:L:M */
	int nf, pllout2;

	cfcr =	CPM_CPCCR_CLKOEN |
		CPM_CPCCR_PCS |
		(n2FR[div[0]] << CPM_CPCCR_CDIV_BIT) |
		(n2FR[div[1]] << CPM_CPCCR_HDIV_BIT) |
		(n2FR[div[2]] << CPM_CPCCR_PDIV_BIT) |
		(n2FR[div[3]] << CPM_CPCCR_MDIV_BIT) |
		(n2FR[div[4]] << CPM_CPCCR_LDIV_BIT);

	pllout2 = (cfcr & CPM_CPCCR_PCS) ?
		CONFIG_SYS_CPU_SPEED : (CONFIG_SYS_CPU_SPEED / 2);

	/* Init USB Host clock, pllout2 must be n*48MHz */
	writel(pllout2 / 48000000 - 1, &cpm->uhccdr);

	nf = CONFIG_SYS_CPU_SPEED * 2 / CONFIG_SYS_EXTAL;
	plcr1 = ((nf - 2) << CPM_CPPCR_PLLM_BIT) | /* FD */
		(0 << CPM_CPPCR_PLLN_BIT) |	/* RD=0, NR=2 */
		(0 << CPM_CPPCR_PLLOD_BIT) |	/* OD=0, NO=1 */
		(0x20 << CPM_CPPCR_PLLST_BIT) |	/* PLL stable time */
		CPM_CPPCR_PLLEN;		/* enable PLL */

	/* init PLL */
	writel(cfcr, &cpm->cpccr);
	writel(plcr1, &cpm->cppcr);
}

void sdram_init(void)
{
	struct jz4740_emc *emc = (struct jz4740_emc *)JZ4740_EMC_BASE;

	register unsigned int dmcr0, dmcr, sdmode, tmp, cpu_clk, mem_clk, ns;

	unsigned int cas_latency_sdmr[2] = {
		EMC_SDMR_CAS_2,
		EMC_SDMR_CAS_3,
	};

	unsigned int cas_latency_dmcr[2] = {
		1 << EMC_DMCR_TCL_BIT,	/* CAS latency is 2 */
		2 << EMC_DMCR_TCL_BIT	/* CAS latency is 3 */
	};

	int div[] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};

	cpu_clk = CONFIG_SYS_CPU_SPEED;
	mem_clk = cpu_clk * div[__cpm_get_cdiv()] / div[__cpm_get_mdiv()];

	writel(0, &emc->bcr);	/* Disable bus release */
	writew(0, &emc->rtcsr);	/* Disable clock for counting */

	/* Fault DMCR value for mode register setting*/
#define SDRAM_ROW0	11
#define SDRAM_COL0	8
#define SDRAM_BANK40	0

	dmcr0 = ((SDRAM_ROW0 - 11) << EMC_DMCR_RA_BIT) |
		((SDRAM_COL0 - 8) << EMC_DMCR_CA_BIT) |
		(SDRAM_BANK40 << EMC_DMCR_BA_BIT) |
		(SDRAM_BW16 << EMC_DMCR_BW_BIT) |
		EMC_DMCR_EPIN |
		cas_latency_dmcr[((SDRAM_CASL == 3) ? 1 : 0)];

	/* Basic DMCR value */
	dmcr = ((SDRAM_ROW - 11) << EMC_DMCR_RA_BIT) |
		((SDRAM_COL - 8) << EMC_DMCR_CA_BIT) |
		(SDRAM_BANK4 << EMC_DMCR_BA_BIT) |
		(SDRAM_BW16 << EMC_DMCR_BW_BIT) |
		EMC_DMCR_EPIN |
		cas_latency_dmcr[((SDRAM_CASL == 3) ? 1 : 0)];

	/* SDRAM timimg */
	ns = 1000000000 / mem_clk;
	tmp = SDRAM_TRAS / ns;
	if (tmp < 4)
		tmp = 4;
	if (tmp > 11)
		tmp = 11;
	dmcr |= (tmp - 4) << EMC_DMCR_TRAS_BIT;
	tmp = SDRAM_RCD / ns;

	if (tmp > 3)
		tmp = 3;
	dmcr |= tmp << EMC_DMCR_RCD_BIT;
	tmp = SDRAM_TPC / ns;

	if (tmp > 7)
		tmp = 7;
	dmcr |= tmp << EMC_DMCR_TPC_BIT;
	tmp = SDRAM_TRWL / ns;

	if (tmp > 3)
		tmp = 3;
	dmcr |= tmp << EMC_DMCR_TRWL_BIT;
	tmp = (SDRAM_TRAS + SDRAM_TPC) / ns;

	if (tmp > 14)
		tmp = 14;
	dmcr |= ((tmp + 1) >> 1) << EMC_DMCR_TRC_BIT;

	/* SDRAM mode value */
	sdmode = EMC_SDMR_BT_SEQ |
		 EMC_SDMR_OM_NORMAL |
		 EMC_SDMR_BL_4 |
		 cas_latency_sdmr[((SDRAM_CASL == 3) ? 1 : 0)];

	/* Stage 1. Precharge all banks by writing SDMR with DMCR.MRSET=0 */
	writel(dmcr, &emc->dmcr);
	writeb(0, JZ4740_EMC_SDMR0 | sdmode);

	/* Wait for precharge, > 200us */
	tmp = (cpu_clk / 1000000) * 1000;
	while (tmp--)
		;

	/* Stage 2. Enable auto-refresh */
	writel(dmcr | EMC_DMCR_RFSH, &emc->dmcr);

	tmp = SDRAM_TREF / ns;
	tmp = tmp / 64 + 1;
	if (tmp > 0xff)
		tmp = 0xff;
	writew(tmp, &emc->rtcor);
	writew(0, &emc->rtcnt);
	/* Divisor is 64, CKO/64 */
	writew(EMC_RTCSR_CKS_64, &emc->rtcsr);

	/* Wait for number of auto-refresh cycles */
	tmp = (cpu_clk / 1000000) * 1000;
	while (tmp--)
		;

	/* Stage 3. Mode Register Set */
	writel(dmcr0 | EMC_DMCR_RFSH | EMC_DMCR_MRSET, &emc->dmcr);
	writeb(0, JZ4740_EMC_SDMR0 | sdmode);

	/* Set back to basic DMCR value */
	writel(dmcr | EMC_DMCR_RFSH | EMC_DMCR_MRSET, &emc->dmcr);

	/* everything is ok now */
}

DECLARE_GLOBAL_DATA_PTR;

void calc_clocks(void)
{
	unsigned int pllout;
	unsigned int div[10] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};

	pllout = __cpm_get_pllout();

	gd->cpu_clk = pllout / div[__cpm_get_cdiv()];
	gd->sys_clk = pllout / div[__cpm_get_hdiv()];
	gd->per_clk = pllout / div[__cpm_get_pdiv()];
	gd->mem_clk = pllout / div[__cpm_get_mdiv()];
	gd->dev_clk = CONFIG_SYS_EXTAL;
}

void rtc_init(void)
{
	struct jz4740_rtc *rtc = (struct jz4740_rtc *)JZ4740_RTC_BASE;

	while (!(readl(&rtc->rcr) & RTC_RCR_WRDY))
		;
	writel(readl(&rtc->rcr) | RTC_RCR_AE, &rtc->rcr); /* enable alarm */

	while (!(readl(&rtc->rcr) & RTC_RCR_WRDY))
		;
	writel(0x00007fff, &rtc->rgr); /* type value */

	while (!(readl(&rtc->rcr) & RTC_RCR_WRDY))
		;
	writel(0x0000ffe0, &rtc->hwfcr); /* Power on delay 2s */

	while (!(readl(&rtc->rcr) & RTC_RCR_WRDY))
		;
	writel(0x00000fe0, &rtc->hrcr); /* reset delay 125ms */
}

/* U-Boot common routines */
phys_size_t initdram(int board_type)
{
	struct jz4740_emc *emc = (struct jz4740_emc *)JZ4740_EMC_BASE;
	u32 dmcr;
	u32 rows, cols, dw, banks;
	ulong size;

	dmcr = readl(&emc->dmcr);
	rows = 11 + ((dmcr & EMC_DMCR_RA_MASK) >> EMC_DMCR_RA_BIT);
	cols = 8 + ((dmcr & EMC_DMCR_CA_MASK) >> EMC_DMCR_CA_BIT);
	dw = (dmcr & EMC_DMCR_BW) ? 2 : 4;
	banks = (dmcr & EMC_DMCR_BA) ? 4 : 2;

	size = (1 << (rows + cols)) * dw * banks;

	return size;
}

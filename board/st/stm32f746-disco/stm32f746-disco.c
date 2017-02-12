/*
 * (C) Copyright 2016
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7m.h>
#include <asm/arch/stm32.h>
#include <asm/arch/gpio.h>
#include <asm/arch/fmc.h>
#include <dm/platdata.h>
#include <dm/platform_data/serial_stm32x7.h>
#include <asm/arch/stm32_periph.h>
#include <asm/arch/stm32_defs.h>
#include <asm/arch/syscfg.h>

DECLARE_GLOBAL_DATA_PTR;

const struct stm32_gpio_ctl gpio_ctl_gpout = {
	.mode = STM32_GPIO_MODE_OUT,
	.otype = STM32_GPIO_OTYPE_PP,
	.speed = STM32_GPIO_SPEED_50M,
	.pupd = STM32_GPIO_PUPD_NO,
	.af = STM32_GPIO_AF0
};

const struct stm32_gpio_ctl gpio_ctl_fmc = {
	.mode = STM32_GPIO_MODE_AF,
	.otype = STM32_GPIO_OTYPE_PP,
	.speed = STM32_GPIO_SPEED_100M,
	.pupd = STM32_GPIO_PUPD_NO,
	.af = STM32_GPIO_AF12
};

static const struct stm32_gpio_dsc ext_ram_fmc_gpio[] = {
	/* Chip is LQFP144, see DM00077036.pdf for details */
	{STM32_GPIO_PORT_D, STM32_GPIO_PIN_10},	/* 79, FMC_D15 */
	{STM32_GPIO_PORT_D, STM32_GPIO_PIN_9},	/* 78, FMC_D14 */
	{STM32_GPIO_PORT_D, STM32_GPIO_PIN_8},	/* 77, FMC_D13 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_15},	/* 68, FMC_D12 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_14},	/* 67, FMC_D11 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_13},	/* 66, FMC_D10 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_12},	/* 65, FMC_D9 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_11},	/* 64, FMC_D8 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_10},	/* 63, FMC_D7 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_9},	/* 60, FMC_D6 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_8},	/* 59, FMC_D5 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_7},	/* 58, FMC_D4 */
	{STM32_GPIO_PORT_D, STM32_GPIO_PIN_1},	/* 115, FMC_D3 */
	{STM32_GPIO_PORT_D, STM32_GPIO_PIN_0},	/* 114, FMC_D2 */
	{STM32_GPIO_PORT_D, STM32_GPIO_PIN_15},	/* 86, FMC_D1 */
	{STM32_GPIO_PORT_D, STM32_GPIO_PIN_14},	/* 85, FMC_D0 */

	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_1},	/* 142, FMC_NBL1 */
	{STM32_GPIO_PORT_E, STM32_GPIO_PIN_0},	/* 141, FMC_NBL0 */

	{STM32_GPIO_PORT_G, STM32_GPIO_PIN_5},	/* 90, FMC_A15, BA1 */
	{STM32_GPIO_PORT_G, STM32_GPIO_PIN_4},	/* 89, FMC_A14, BA0 */

	{STM32_GPIO_PORT_G, STM32_GPIO_PIN_1},	/* 57, FMC_A11 */
	{STM32_GPIO_PORT_G, STM32_GPIO_PIN_0},	/* 56, FMC_A10 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_15},	/* 55, FMC_A9 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_14},	/* 54, FMC_A8 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_13},	/* 53, FMC_A7 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_12},	/* 50, FMC_A6 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_5},	/* 15, FMC_A5 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_4},	/* 14, FMC_A4 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_3},	/* 13, FMC_A3 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_2},	/* 12, FMC_A2 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_1},	/* 11, FMC_A1 */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_0},	/* 10, FMC_A0 */

	{STM32_GPIO_PORT_H, STM32_GPIO_PIN_3},	/* 136, SDRAM_NE */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_11},	/* 49, SDRAM_NRAS */
	{STM32_GPIO_PORT_G, STM32_GPIO_PIN_15},	/* 132, SDRAM_NCAS */
	{STM32_GPIO_PORT_H, STM32_GPIO_PIN_5},	/* 26, SDRAM_NWE */
	{STM32_GPIO_PORT_C, STM32_GPIO_PIN_3},	/* 135, SDRAM_CKE */

	{STM32_GPIO_PORT_G, STM32_GPIO_PIN_8},	/* 93, SDRAM_CLK */
};

static int fmc_setup_gpio(void)
{
	int rv = 0;
	int i;

	clock_setup(GPIO_B_CLOCK_CFG);
	clock_setup(GPIO_C_CLOCK_CFG);
	clock_setup(GPIO_D_CLOCK_CFG);
	clock_setup(GPIO_E_CLOCK_CFG);
	clock_setup(GPIO_F_CLOCK_CFG);
	clock_setup(GPIO_G_CLOCK_CFG);
	clock_setup(GPIO_H_CLOCK_CFG);

	for (i = 0; i < ARRAY_SIZE(ext_ram_fmc_gpio); i++) {
		rv = stm32_gpio_config(&ext_ram_fmc_gpio[i],
				&gpio_ctl_fmc);
		if (rv)
			goto out;
	}

out:
	return rv;
}

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

int dram_init(void)
{
	u32 freq;
	int rv;

	rv = fmc_setup_gpio();
	if (rv)
		return rv;

	clock_setup(FMC_CLOCK_CFG);

	/*
	 * Get frequency for NS2CLK calculation.
	 */
	freq = clock_get(CLOCK_AHB) / CONFIG_SYS_RAM_FREQ_DIV;

	writel(
		CONFIG_SYS_RAM_FREQ_DIV << FMC_SDCR_SDCLK_SHIFT
		| SDRAM_CAS << FMC_SDCR_CAS_SHIFT
		| SDRAM_NB << FMC_SDCR_NB_SHIFT
		| SDRAM_MWID << FMC_SDCR_MWID_SHIFT
		| SDRAM_NR << FMC_SDCR_NR_SHIFT
		| SDRAM_NC << FMC_SDCR_NC_SHIFT
		| SDRAM_RPIPE << FMC_SDCR_RPIPE_SHIFT
		| SDRAM_RBURST << FMC_SDCR_RBURST_SHIFT,
		&STM32_SDRAM_FMC->sdcr1);

	writel(
		SDRAM_TRCD << FMC_SDTR_TRCD_SHIFT
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

	/*
	 * Fill in global info with description of SRAM configuration
	 */
	gd->bd->bi_dram[0].start = CONFIG_SYS_RAM_BASE;
	gd->bd->bi_dram[0].size  = CONFIG_SYS_RAM_SIZE;

	gd->ram_size = CONFIG_SYS_RAM_SIZE;

	return rv;
}

int uart_setup_gpio(void)
{
	clock_setup(GPIO_A_CLOCK_CFG);
	clock_setup(GPIO_B_CLOCK_CFG);
	return 0;
}

#ifdef CONFIG_ETH_DESIGNWARE

static int stmmac_setup(void)
{
	clock_setup(SYSCFG_CLOCK_CFG);
	/* Set >RMII mode */
	STM32_SYSCFG->pmc |= SYSCFG_PMC_MII_RMII_SEL;

	clock_setup(GPIO_A_CLOCK_CFG);
	clock_setup(GPIO_C_CLOCK_CFG);
	clock_setup(GPIO_G_CLOCK_CFG);
	clock_setup(STMMAC_CLOCK_CFG);

	return 0;
}
#endif

#ifdef CONFIG_STM32_QSPI

static int qspi_setup(void)
{
	clock_setup(GPIO_B_CLOCK_CFG);
	clock_setup(GPIO_D_CLOCK_CFG);
	clock_setup(GPIO_E_CLOCK_CFG);
	return 0;
}
#endif

u32 get_board_rev(void)
{
	return 0;
}

int board_early_init_f(void)
{
	int res;

	res = uart_setup_gpio();
	if (res)
		return res;

#ifdef CONFIG_ETH_DESIGNWARE
	res = stmmac_setup();
	if (res)
		return res;
#endif

#ifdef CONFIG_STM32_QSPI
	res = qspi_setup();
	if (res)
		return res;
#endif

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

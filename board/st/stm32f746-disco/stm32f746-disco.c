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

int dram_init(void)
{
	int rv;

	rv = fmc_setup_gpio();
	if (rv)
		return rv;

	clock_setup(FMC_CLOCK_CFG);
	stm32_sdram_init();

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

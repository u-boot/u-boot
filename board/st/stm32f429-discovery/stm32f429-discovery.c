/*
 * (C) Copyright 2011, 2012, 2013
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 * Alexander Potashev, Emcraft Systems, aspotashev@emcraft.com
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
 * Pavel Boldin, Emcraft Systems, paboldin@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <stm32_rcc.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/gpio.h>
#include <dm/platform_data/serial_stm32.h>
#include <asm/arch/stm32_periph.h>
#include <asm/arch/stm32_defs.h>

DECLARE_GLOBAL_DATA_PTR;

const struct stm32_gpio_ctl gpio_ctl_gpout = {
	.mode = STM32_GPIO_MODE_OUT,
	.otype = STM32_GPIO_OTYPE_PP,
	.speed = STM32_GPIO_SPEED_50M,
	.pupd = STM32_GPIO_PUPD_NO,
	.af = STM32_GPIO_AF0
};

const struct stm32_gpio_ctl gpio_ctl_usart = {
	.mode = STM32_GPIO_MODE_AF,
	.otype = STM32_GPIO_OTYPE_PP,
	.speed = STM32_GPIO_SPEED_50M,
	.pupd = STM32_GPIO_PUPD_UP,
	.af = STM32_GPIO_USART
};

static const struct stm32_gpio_dsc usart_gpio[] = {
	{STM32_GPIO_PORT_X, STM32_GPIO_PIN_TX},	/* TX */
	{STM32_GPIO_PORT_X, STM32_GPIO_PIN_RX},	/* RX */
};

int uart_setup_gpio(void)
{
	int i;
	int rv = 0;

	clock_setup(GPIO_A_CLOCK_CFG);
	for (i = 0; i < ARRAY_SIZE(usart_gpio); i++) {
		rv = stm32_gpio_config(&usart_gpio[i], &gpio_ctl_usart);
		if (rv)
			goto out;
	}

out:
	return rv;
}

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
	{STM32_GPIO_PORT_B, STM32_GPIO_PIN_6},	/* 136, SDRAM_NE */
	{STM32_GPIO_PORT_F, STM32_GPIO_PIN_11},	/* 49, SDRAM_NRAS */
	{STM32_GPIO_PORT_G, STM32_GPIO_PIN_15},	/* 132, SDRAM_NCAS */
	{STM32_GPIO_PORT_C, STM32_GPIO_PIN_0},	/* 26, SDRAM_NWE */
	{STM32_GPIO_PORT_B, STM32_GPIO_PIN_5},	/* 135, SDRAM_CKE */
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

	for (i = 0; i < ARRAY_SIZE(ext_ram_fmc_gpio); i++) {
		rv = stm32_gpio_config(&ext_ram_fmc_gpio[i],
				&gpio_ctl_fmc);
		if (rv)
			goto out;
	}

out:
	return rv;
}

/*
 * STM32 RCC FMC specific definitions
 */
#define STM32_RCC_ENR_FMC	(1 << 0)	/* FMC module clock  */

int dram_init(void)
{
	int rv;
	struct udevice *dev;

	rv = fmc_setup_gpio();
	if (rv)
		return rv;

	setbits_le32(&STM32_RCC->ahb3enr, STM32_RCC_ENR_FMC);

	rv = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (rv) {
		debug("DRAM init failed: %d\n", rv);
		return rv;
	}

	if (fdtdec_setup_memory_size() != 0)
		rv = -EINVAL;

	return rv;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

static const struct stm32_serial_platdata serial_platdata = {
	.base = (struct stm32_usart *)STM32_USART1_BASE,
};

U_BOOT_DEVICE(stm32_serials) = {
	.name = "serial_stm32",
	.platdata = &serial_platdata,
};

u32 get_board_rev(void)
{
	return 0;
}

int board_early_init_f(void)
{
	int res;

	configure_clocks();

	res = uart_setup_gpio();
	if (res)
		return res;
	clock_setup(USART1_CLOCK_CFG);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	char serialno[25];
	uint32_t u_id_low, u_id_mid, u_id_high;

	if (!env_get("serial#")) {
		u_id_low  = readl(&STM32_U_ID->u_id_low);
		u_id_mid  = readl(&STM32_U_ID->u_id_mid);
		u_id_high = readl(&STM32_U_ID->u_id_high);
		sprintf(serialno, "%08x%08x%08x",
			u_id_high, u_id_mid, u_id_low);
		env_set("serial#", serialno);
	}

	return 0;
}
#endif

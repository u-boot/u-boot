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
	.af = STM32_GPIO_AF7
};

static const struct stm32_gpio_dsc usart_gpio[] = {
	{STM32_GPIO_PORT_A, STM32_GPIO_PIN_9},	/* TX */
	{STM32_GPIO_PORT_B, STM32_GPIO_PIN_7},	/* RX */
};

int uart_setup_gpio(void)
{
	int i;
	int rv = 0;

	clock_setup(GPIO_A_CLOCK_CFG);
	clock_setup(GPIO_B_CLOCK_CFG);
	for (i = 0; i < ARRAY_SIZE(usart_gpio); i++) {
		rv = stm32_gpio_config(&usart_gpio[i], &gpio_ctl_usart);
		if (rv)
			goto out;
	}

out:
	return rv;
}

static const struct stm32x7_serial_platdata serial_platdata = {
	.base = (struct stm32_usart *)USART1_BASE,
	.clock = CONFIG_SYS_CLK_FREQ,
};

U_BOOT_DEVICE(stm32x7_serials) = {
	.name = "serial_stm32x7",
	.platdata = &serial_platdata,
};

u32 get_board_rev(void)
{
	return 0;
}

int board_early_init_f(void)
{
	int res;

	res = uart_setup_gpio();
	clock_setup(USART1_CLOCK_CFG);
	if (res)
		return res;

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_RAM_BASE;
	gd->bd->bi_dram[0].size  = CONFIG_SYS_RAM_SIZE;

	gd->ram_size = CONFIG_SYS_RAM_SIZE;
	return 0;
}

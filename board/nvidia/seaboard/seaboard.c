/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/pinmux.h>
#include <asm/gpio.h>
#ifdef CONFIG_TEGRA2_MMC
#include <mmc.h>
#endif
#include "../common/board.h"

/*
 * Routine: gpio_config_uart
 * Description: Force GPIO_PI3 low on Seaboard so UART4 works.
 */
void gpio_config_uart(void)
{
	int gp = GPIO_PI3;
	struct gpio_ctlr *gpio = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &gpio->gpio_bank[GPIO_BANK(gp)];
	u32 val;

	/* Enable UART via GPIO_PI3 (port 8, bit 3) so serial console works */
	val = readl(&bank->gpio_config[GPIO_PORT(gp)]);
	val |= 1 << GPIO_BIT(gp);
	writel(val, &bank->gpio_config[GPIO_PORT(gp)]);

	val = readl(&bank->gpio_out[GPIO_PORT(gp)]);
	val &= ~(1 << GPIO_BIT(gp));
	writel(val, &bank->gpio_out[GPIO_PORT(gp)]);

	val = readl(&bank->gpio_dir_out[GPIO_PORT(gp)]);
	val |= 1 << GPIO_BIT(gp);
	writel(val, &bank->gpio_dir_out[GPIO_PORT(gp)]);
}

#ifdef CONFIG_TEGRA2_MMC
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
static void pin_mux_mmc(void)
{
	/* SDMMC4: config 3, x8 on 2nd set of pins */
	pinmux_set_func(PINGRP_ATB, PMUX_FUNC_SDIO4);
	pinmux_set_func(PINGRP_GMA, PMUX_FUNC_SDIO4);
	pinmux_set_func(PINGRP_GME, PMUX_FUNC_SDIO4);

	pinmux_tristate_disable(PINGRP_ATB);
	pinmux_tristate_disable(PINGRP_GMA);
	pinmux_tristate_disable(PINGRP_GME);

	/* SDMMC3: SDIO3_CLK, SDIO3_CMD, SDIO3_DAT[3:0] */
	pinmux_set_func(PINGRP_SDB, PMUX_FUNC_SDIO3);
	pinmux_set_func(PINGRP_SDC, PMUX_FUNC_SDIO3);
	pinmux_set_func(PINGRP_SDD, PMUX_FUNC_SDIO3);

	pinmux_tristate_disable(PINGRP_SDC);
	pinmux_tristate_disable(PINGRP_SDD);
	pinmux_tristate_disable(PINGRP_SDB);

	/* For power GPIO PI6 */
	pinmux_tristate_disable(PINGRP_ATA);
	/* For CD GPIO PI5 */
	pinmux_tristate_disable(PINGRP_ATC);
}

/*
 * Routine: gpio_config_mmc
 * Description: Set GPIOs for SDMMC3 SDIO slot.
 */
void gpio_config_mmc(void)
{
	/* Set EN_VDDIO_SD (GPIO I6) */
	gpio_direction_output(GPIO_PI6, 1);

	/* Config pin as GPI for Card Detect (GPIO I5) */
	gpio_direction_input(GPIO_PI5);
}

/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	debug("board_mmc_init called\n");

	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();
	gpio_config_mmc();

	debug("board_mmc_init: init eMMC\n");
	/* init dev 0, eMMC chip, with 4-bit bus */
	/* The board has an 8-bit bus, but 8-bit doesn't work yet */
	tegra2_mmc_init(0, 4);

	debug("board_mmc_init: init SD slot\n");
	/* init dev 1, SD slot, with 4-bit bus */
	tegra2_mmc_init(1, 4);

	return 0;
}

/* this is a weak define that we are overriding */
int board_mmc_getcd(u8 *cd, struct mmc *mmc)
{
	debug("board_mmc_getcd called\n");
	*cd = 1;			/* Assume card is inserted, or eMMC */

	if (IS_SD(mmc)) {
		/* Seaboard SDMMC3 = SDIO3_CD = GPIO_PI5 */
		if (gpio_get_value(GPIO_PI5))
			*cd = 0;
	}

	return 0;
}
#endif

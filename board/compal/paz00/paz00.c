/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/mmc.h>
#include <asm/gpio.h>
#ifdef CONFIG_TEGRA2_MMC
#include <mmc.h>
#endif

/*
 * Routine: gpio_config_uart
 * Description: Does nothing on Paz00 - no conflict w/SPI.
 */
void gpio_config_uart(void)
{
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

	/* SDMMC1: SDIO1_CLK, SDIO1_CMD, SDIO1_DAT[3:0] */
	pinmux_set_func(PINGRP_SDMMC1, PMUX_FUNC_SDIO1);

	pinmux_tristate_disable(PINGRP_SDMMC1);

	/* For power GPIO PV1 */
	pinmux_tristate_disable(PINGRP_UAC);
	/* For CD GPIO PI5 */
	pinmux_tristate_disable(PINGRP_ATC);
}

/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	debug("board_mmc_init called\n");

	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();

	debug("board_mmc_init: init eMMC\n");
	/* init dev 0, eMMC chip, with 4-bit bus */
	/* The board has an 8-bit bus, but 8-bit doesn't work yet */
	tegra2_mmc_init(0, 4, -1, -1);

	debug("board_mmc_init: init SD slot\n");
	/* init dev 3, SD slot, with 4-bit bus */
	tegra2_mmc_init(3, 4, GPIO_PV1, GPIO_PI5);

	return 0;
}
#endif

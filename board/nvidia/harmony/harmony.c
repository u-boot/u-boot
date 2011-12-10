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
 * Description: Does nothing on Harmony - no conflict w/SPI.
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

	/* For power GPIO PI6 */
	pinmux_tristate_disable(PINGRP_ATA);
	/* For CD GPIO PH2 */
	pinmux_tristate_disable(PINGRP_ATD);

	/* SDMMC2: SDIO2_CLK, SDIO2_CMD, SDIO2_DAT[7:0] */
	pinmux_set_func(PINGRP_DTA, PMUX_FUNC_SDIO2);
	pinmux_set_func(PINGRP_DTD, PMUX_FUNC_SDIO2);

	pinmux_tristate_disable(PINGRP_DTA);
	pinmux_tristate_disable(PINGRP_DTD);

	/* For power GPIO PT3 */
	pinmux_tristate_disable(PINGRP_DTB);
	/* For CD GPIO PI5 */
	pinmux_tristate_disable(PINGRP_ATC);
}

/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	debug("board_mmc_init called\n");

	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();

	debug("board_mmc_init: init SD slot J26\n");
	/* init dev 0, SD slot J26, with 4-bit bus */
	/* The board has an 8-bit bus, but 8-bit doesn't work yet */
	tegra2_mmc_init(0, 4, GPIO_PI6, GPIO_PH2);

	debug("board_mmc_init: init SD slot J5\n");
	/* init dev 2, SD slot J5, with 4-bit bus */
	tegra2_mmc_init(2, 4, GPIO_PT3, GPIO_PI5);

	return 0;
}
#endif

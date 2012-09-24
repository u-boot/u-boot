/*
 *  (C) Copyright 2010-2012
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
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/mmc.h>
#include <asm/gpio.h>
#ifdef CONFIG_TEGRA_MMC
#include <mmc.h>
#endif

/*
 * Routine: gpio_config_uart
 * Description: Does nothing on TrimSlice - no UART-related GPIOs.
 */
void gpio_config_uart(void)
{
}

void pin_mux_spi(void)
{
	funcmux_select(PERIPH_ID_SPI1, FUNCMUX_SPI1_GMC_GMD);
}

/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
static void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC1, FUNCMUX_SDMMC1_SDIO1_4BIT);
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_4_BIT);

	/* For CD GPIO PP1 */
	pinmux_tristate_disable(PINGRP_DAP3);
}

/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	debug("board_mmc_init called\n");

	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();

	/* init dev 0 (SDMMC4), (micro-SD slot) with 4-bit bus */
	tegra2_mmc_init(0, 4, -1, GPIO_PP1);

	/* init dev 3 (SDMMC1), (SD slot) with 4-bit bus */
	tegra2_mmc_init(3, 4, -1, -1);

	return 0;
}

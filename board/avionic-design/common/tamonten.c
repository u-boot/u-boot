/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *  (C) Copyright 2011-2012
 *  Avionic Design GmbH <www.avionic-design.de>
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
#include <ns16550.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/board.h>
#include <asm/arch/tegra20.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/uart.h>
#include <asm/arch/mmc.h>

#ifdef CONFIG_TEGRA_MMC
#include <mmc.h>
#endif

/*
 * Routine: gpio_config_uart
 * Description: Does nothing on Tamonten - no conflict w/SPI.
 */
void gpio_config_uart(void)
{
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
void gpio_early_init(void)
{
	gpio_request(GPIO_PI4, NULL);
	gpio_direction_output(GPIO_PI4, 1);
}
#endif

#ifdef CONFIG_TEGRA_MMC
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
static void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_GME_8_BIT);
	/* for write-protect GPIO PI6 */
	pinmux_tristate_disable(PINGRP_ATA);
	/* for CD GPIO PH2 */
	pinmux_tristate_disable(PINGRP_ATD);
}

/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();

	/* init dev 0, SD slot, with 4-bit bus */
	tegra_mmc_init(0, 4, GPIO_PI6, GPIO_PH2);

	return 0;
}
#endif

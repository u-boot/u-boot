/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *  (C) Copyright 2011
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
#include <asm/arch/tegra2.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/uart.h>
#include <asm/arch/mmc.h>
#include "tamonten.h"

#ifdef CONFIG_TEGRA2_MMC
#include <mmc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

const struct tegra2_sysinfo sysinfo = {
	CONFIG_TEGRA2_BOARD_STRING
};

/*
 * Routine: timer_init
 * Description: init the timestamp and lastinc value
 */
int timer_init(void)
{
	return 0;
}

#ifdef CONFIG_TEGRA2_MMC
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
static void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_GME_8_BIT);
}
#endif

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	clock_init();
	clock_verify();

	/* boot param addr */
	gd->bd->bi_boot_params = (NV_PA_SDRAM_BASE + 0x100);

	return 0;
}

#ifdef CONFIG_TEGRA2_MMC
/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	debug("board_mmc_init called\n");
	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();
	gpio_config_mmc();

	debug("board_mmc_init: init eMMC\n");
	/* init dev 0, eMMC chip, with 4-bit bus */
	tegra2_mmc_init(0, 4, -1, GPIO_PH2);

	return 0;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	/* Initialize selected UARTs */
	board_init_uart_f();
	return 0;
}
#endif /* EARLY_INIT */

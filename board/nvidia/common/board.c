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
#include <ns16550.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/emc.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/pmu.h>
#include <asm/arch/tegra.h>
#include <asm/arch/usb.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/sys_proto.h>
#include <asm/arch-tegra/uart.h>
#include <asm/arch-tegra/warmboot.h>
#include <spi.h>
#include <i2c.h>
#include "emc.h"

DECLARE_GLOBAL_DATA_PTR;

const struct tegra_sysinfo sysinfo = {
	CONFIG_TEGRA_BOARD_STRING
};

#ifndef CONFIG_SPL_BUILD
/*
 * Routine: timer_init
 * Description: init the timestamp and lastinc value
 */
int timer_init(void)
{
	return 0;
}
#endif

void __pin_mux_usb(void)
{
}

void pin_mux_usb(void) __attribute__((weak, alias("__pin_mux_usb")));

void __pin_mux_spi(void)
{
}

void pin_mux_spi(void) __attribute__((weak, alias("__pin_mux_spi")));

void __gpio_early_init_uart(void)
{
}

void gpio_early_init_uart(void)
__attribute__((weak, alias("__gpio_early_init_uart")));

void __pin_mux_nand(void)
{
	funcmux_select(PERIPH_ID_NDFLASH, FUNCMUX_DEFAULT);
}

void pin_mux_nand(void) __attribute__((weak, alias("__pin_mux_nand")));

/*
 * Routine: power_det_init
 * Description: turn off power detects
 */
static void power_det_init(void)
{
#if defined(CONFIG_TEGRA20)
	struct pmc_ctlr *const pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	/* turn off power detects */
	writel(0, &pmc->pmc_pwr_det_latch);
	writel(0, &pmc->pmc_pwr_det);
#endif
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	__maybe_unused int err;

	/* Do clocks and UART first so that printf() works */
	clock_init();
	clock_verify();

#ifdef CONFIG_SPI_UART_SWITCH
	gpio_config_uart();
#endif
#ifdef CONFIG_TEGRA_SPI
	pin_mux_spi();
	spi_init();
#endif
	/* boot param addr */
	gd->bd->bi_boot_params = (NV_PA_SDRAM_BASE + 0x100);

	power_det_init();

#ifdef CONFIG_TEGRA_I2C
#ifndef CONFIG_SYS_I2C_INIT_BOARD
#error "You must define CONFIG_SYS_I2C_INIT_BOARD to use i2c on Nvidia boards"
#endif
	i2c_init_board();
# ifdef CONFIG_TEGRA_PMU
	if (pmu_set_nominal())
		debug("Failed to select nominal voltages\n");
#  ifdef CONFIG_TEGRA_CLOCK_SCALING
	err = board_emc_init();
	if (err)
		debug("Memory controller init failed: %d\n", err);
#  endif
# endif /* CONFIG_TEGRA_PMU */
#endif /* CONFIG_TEGRA_I2C */

#ifdef CONFIG_USB_EHCI_TEGRA
	pin_mux_usb();
	board_usb_init(gd->fdt_blob);
#endif

#ifdef CONFIG_TEGRA_NAND
	pin_mux_nand();
#endif

#ifdef CONFIG_TEGRA_LP0
	/* save Sdram params to PMC 2, 4, and 24 for WB0 */
	warmboot_save_sdram_params();

	/* prepare the WB code to LP0 location */
	warmboot_prepare_code(TEGRA_LP0_ADDR, TEGRA_LP0_SIZE);
#endif

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
static void __gpio_early_init(void)
{
}

void gpio_early_init(void) __attribute__((weak, alias("__gpio_early_init")));

int board_early_init_f(void)
{
	board_init_uart_f();

	/* Initialize periph GPIOs */
	gpio_early_init();
	gpio_early_init_uart();

	return 0;
}
#endif	/* EARLY_INIT */

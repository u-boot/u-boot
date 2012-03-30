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
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/sys_proto.h>

#include <asm/arch/board.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/uart.h>
#include <spi.h>
#include <asm/arch/usb.h>
#include <i2c.h>
#include "board.h"

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

void __pin_mux_usb(void)
{
}

void pin_mux_usb(void) __attribute__((weak, alias("__pin_mux_usb")));

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	/* Do clocks and UART first so that printf() works */
	clock_init();
	clock_verify();

#ifdef CONFIG_SPI_UART_SWITCH
	gpio_config_uart();
#endif
#ifdef CONFIG_TEGRA2_SPI
	spi_init();
#endif
	/* boot param addr */
	gd->bd->bi_boot_params = (NV_PA_SDRAM_BASE + 0x100);
#ifdef CONFIG_TEGRA_I2C
#ifndef CONFIG_SYS_I2C_INIT_BOARD
#error "You must define CONFIG_SYS_I2C_INIT_BOARD to use i2c on Nvidia boards"
#endif
	i2c_init_board();
#endif

#ifdef CONFIG_USB_EHCI_TEGRA
	pin_mux_usb();
	board_usb_init(gd->fdt_blob);
#endif

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	board_init_uart_f();

	/* Initialize periph GPIOs */
#ifdef CONFIG_SPI_UART_SWITCH
	gpio_early_init_uart();
#else
	gpio_config_uart();
#endif
	return 0;
}
#endif	/* EARLY_INIT */

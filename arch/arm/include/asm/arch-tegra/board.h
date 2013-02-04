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

#ifndef _TEGRA_BOARD_H_
#define _TEGRA_BOARD_H_

/* Set up pinmux to make UART usable */
void gpio_config_uart(void);      /* CONFIG_SPI_UART_SWITCH */
void gpio_early_init_uart(void);  /*!CONFIG_SPI_UART_SWITCH */

/* Set up early UART output */
void board_init_uart_f(void);

/* Set up any early GPIOs the board might need for proper operation */
void gpio_early_init(void);  /* overrideable GPIO config        */

/*
 * Hooks to allow boards to set up the pinmux for a specific function.
 * Has to be implemented in the board files as we don't yet support pinmux
 * setup from FTD. If a board file does not implement one of those functions
 * an empty stub function will be called.
 */

void pin_mux_usb(void);      /* overrideable USB pinmux setup   */
void pin_mux_spi(void);      /* overrideable SPI pinmux setup   */
void pin_mux_nand(void);     /* overrideable NAND pinmux setup  */

#endif

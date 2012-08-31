/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/uart-spi-switch.h>
#include <asm/arch/tegra20.h>
#include <asm/arch/tegra_spi.h>


/* position of the UART/SPI select switch */
enum spi_uart_switch {
	SWITCH_UNKNOWN,
	SWITCH_SPI,
	SWITCH_UART,
	SWITCH_BOTH
};

/* Information about the spi/uart switch */
struct spi_uart {
	int gpio;                       /* GPIO to control switch */
	u32 port;                       /* Port number of UART affected */
};

static struct spi_uart local;
static enum spi_uart_switch switch_pos; /* Current switch position */


static void get_config(struct spi_uart *config)
{
#if defined CONFIG_SPI_CORRUPTS_UART
	config->gpio = CONFIG_UART_DISABLE_GPIO;
	config->port = CONFIG_SPI_CORRUPTS_UART_NR;
#else
	config->gpio = -1;
#endif
}

/*
 * Init the UART / SPI switch. This can be called before relocation so we must
 * not access BSS.
 */
void gpio_early_init_uart(void)
{
	struct spi_uart config;

	get_config(&config);
	if (config.gpio != -1) {
		/* Cannot provide a label prior to relocation */
		gpio_request(config.gpio, NULL);
		gpio_direction_output(config.gpio, 0);
	}
}

/*
 * Configure the UART / SPI switch.
 */
void gpio_config_uart(void)
{
	get_config(&local);
	if (local.gpio != -1) {
		gpio_direction_output(local.gpio, 0);
		switch_pos = SWITCH_UART;
	} else {
		/*
		 * If we're here we don't have a SPI switch; go ahead and
		 * enable the SPI now.  We didn't in spi_init() so we wouldn't
		 * kill the UART.
		 */
		pinmux_set_func(PINGRP_GMC, PMUX_FUNC_SFLASH);
		switch_pos = SWITCH_BOTH;
	}
}

static void spi_uart_switch(struct spi_uart *config,
			      enum spi_uart_switch new_pos)
{
	if (switch_pos == SWITCH_BOTH || new_pos == switch_pos)
		return;

	/* pre-delay, allow SPI/UART to settle, FIFO to empty, etc. */
	udelay(CONFIG_SPI_CORRUPTS_UART_DLY);

	/* We need to dynamically change the pinmux, shared w/UART RXD/CTS */
	pinmux_set_func(PINGRP_GMC, new_pos == SWITCH_SPI ?
				PMUX_FUNC_SFLASH : PMUX_FUNC_UARTD);

	/*
	 * On Seaboard, MOSI/MISO are shared w/UART.
	 * Use GPIO I3 (UART_DISABLE) to tristate UART during SPI activity.
	 * Enable UART later (cs_deactivate) so we can use it for U-Boot comms.
	 */
	gpio_direction_output(config->gpio, new_pos == SWITCH_SPI);
	switch_pos = new_pos;
}

void pinmux_select_uart(void)
{
		spi_uart_switch(&local, SWITCH_UART);
}

void pinmux_select_spi(void)
{
	spi_uart_switch(&local, SWITCH_SPI);
}

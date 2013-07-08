/*
 * Copyright (C) 2009
 * Albin Tonnerre, Free Electrons <albin.tonnerre@free-electrons.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_spi.h>
#include <asm/arch/gpio.h>
#include <spi.h>

#define TNY_A9260_CS_PIN	AT91_PIN_PC11

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	at91_set_gpio_value(TNY_A9260_CS_PIN, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	at91_set_gpio_value(TNY_A9260_CS_PIN, 1);
}

void spi_init_f(void)
{
	/* everything done in board_init */
}

/*
 * Copyright (C) 2009
 * Albin Tonnerre, Free Electrons <albin.tonnerre@free-electrons.com>
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
#include <asm/arch/hardware.h>
#include <asm/arch/at91_spi.h>
#include <asm/arch/gpio.h>
#include <spi.h>

#define SBC_A9260_CS0_PIN	AT91_PIN_PA3
#define SBC_A9260_CS1_PIN	AT91_PIN_PC11

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && (cs == 1 || cs == 0);
}

void spi_cs_activate(struct spi_slave *slave)
{
	if(slave->cs == 0)
		at91_set_gpio_value(SBC_A9260_CS0_PIN, 0);
	else if(slave->cs == 1)
		at91_set_gpio_value(SBC_A9260_CS1_PIN, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	if(slave->cs == 0)
		at91_set_gpio_value(SBC_A9260_CS0_PIN, 1);
	else if(slave->cs == 1)
		at91_set_gpio_value(SBC_A9260_CS1_PIN, 1);
}

void spi_init_f(void)
{
	/* everything done in board_init */
}

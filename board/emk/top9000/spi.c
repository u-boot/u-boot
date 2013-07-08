/*
 * Copyright (C) 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_spi.h>
#include <asm/arch/gpio.h>
#include <spi.h>

static const struct {
	u32	port;
	u32	bit;
} cs_to_portbit[2][4] = {
	{{AT91_PIO_PORTA,  3}, {AT91_PIO_PORTC, 11},
			{AT91_PIO_PORTC, 16}, {AT91_PIO_PORTC, 17} },
	{{AT91_PIO_PORTB,  3}, {AT91_PIO_PORTC,  5},
			{AT91_PIO_PORTC,  4}, {AT91_PIO_PORTC,  3} }
};

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	debug("spi_cs_is_valid: bus=%u cs=%u\n", bus, cs);
	if (bus < 2 && cs < 4)
		return 1;
	return 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	debug("spi_cs_activate: bus=%u cs=%u\n", slave->bus, slave->cs);
	at91_set_pio_output(cs_to_portbit[slave->bus][slave->cs].port,
		cs_to_portbit[slave->bus][slave->cs].bit, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	debug("spi_cs_deactivate: bus=%u cs=%u\n", slave->bus, slave->cs);
	at91_set_pio_output(cs_to_portbit[slave->bus][slave->cs].port,
		cs_to_portbit[slave->bus][slave->cs].bit, 1);
}

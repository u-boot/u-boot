/*
    This file defines the interface to the lpc22xx SPI module.
    Copyright (C) 2006  Embedded Artists AB (www.embeddedartists.com)

    This file may be included in software not adhering to the GPL.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef SPI_H
#define SPI_H

#include <config.h>
#include <common.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>

#define SPIF 0x80

#define spi_lock() disable_interrupts();
#define spi_unlock() enable_interrupts();

extern unsigned long spi_flags;
extern unsigned char spi_idle;

int spi_init(void);

static inline unsigned char spi_read(void)
{
	unsigned char b;

	PUT8(S0SPDR, spi_idle);
	while (!(GET8(S0SPSR) & SPIF));
	b = GET8(S0SPDR);

	return b;
}

static inline void spi_write(unsigned char b)
{
	PUT8(S0SPDR, b);
	while (!(GET8(S0SPSR) & SPIF));
	GET8(S0SPDR);		/* this will clear the SPIF bit */
}

static inline void spi_set_clock(unsigned char clk_value)
{
	PUT8(S0SPCCR, clk_value);
}

static inline void spi_set_cfg(unsigned char phase,
			       unsigned char polarity,
			       unsigned char lsbf)
{
	unsigned char v = 0x20;	/* master bit set */

	if (phase)
		v |= 0x08;			/* set phase bit */
	if (polarity) {
		v |= 0x10;			/* set polarity bit */
		spi_idle = 0xFF;
	} else {
		spi_idle = 0x00;
	}
	if (lsbf)
		v |= 0x40;			/* set lsbf bit */

	PUT8(S0SPCR, v);
}
#endif /* SPI_H */

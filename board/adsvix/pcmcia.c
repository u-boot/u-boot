/*
 * (C) Copyright 2004
 * Robert Whaley, Applied Data Systems, Inc. rwhaley@applieddata.net
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
#include <asm/arch/pxa-regs.h>

void pcmcia_power_on(void)
{
#if 0
	if (!(GPLR(20) & GPIO_bit(20))) { /* 3.3V */
		GPCR(81) = GPIO_bit(81);
		GPSR(82) = GPIO_bit(82);
	}
	else if (!(GPLR(21) & GPIO_bit(21))) { /* 5.0V */
		GPCR(81) = GPIO_bit(81);
		GPCR(82) = GPIO_bit(82);
	}
#else
#warning "Board will only supply 5V, wait for next HW spin for selectable power"
	/* 5.0V */
	GPCR(81) = GPIO_bit(81);
	GPCR(82) = GPIO_bit(82);
#endif

	udelay(300000);

	/* reset the card */
	GPSR(52) = GPIO_bit(52);

	/* enable PCMCIA */
	GPCR(83) = GPIO_bit(83);

	/* clear reset */
	udelay(10);
	GPCR(52) = GPIO_bit(52);

	udelay(20000);
}

void pcmcia_power_off(void)
{
	/* 0V */
	GPSR(81) = GPIO_bit(81);
	GPSR(82) = GPIO_bit(82);
	/* disable PCMCIA */
	GPSR(83) = GPIO_bit(83);
}

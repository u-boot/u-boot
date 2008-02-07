/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop <at> leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/AT91CAP9.h>

#define	RED_LED		AT91C_PIO_PC29	/* this is the power led */
#define	GREEN_LED	AT91C_PIO_PA10	/* this is the user1 led */
#define	YELLOW_LED	AT91C_PIO_PA11	/* this is the user1 led */

void red_LED_on(void)
{
	AT91C_BASE_PIOC->PIO_SODR = RED_LED;
}

void red_LED_off(void)
{
	AT91C_BASE_PIOC->PIO_CODR = RED_LED;
}

void green_LED_on(void)
{
	AT91C_BASE_PIOA->PIO_CODR = GREEN_LED;
}

void green_LED_off(void)
{
	AT91C_BASE_PIOA->PIO_SODR = GREEN_LED;
}

void yellow_LED_on(void)
{
	AT91C_BASE_PIOA->PIO_CODR = YELLOW_LED;
}

void yellow_LED_off(void)
{
	AT91C_BASE_PIOA->PIO_SODR = YELLOW_LED;
}

void coloured_LED_init(void)
{
	/* Enable clock */
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOABCD;

	/* Disable peripherals on LEDs */
	AT91C_BASE_PIOA->PIO_PER = GREEN_LED | YELLOW_LED;
	/* Enable pins as outputs */
	AT91C_BASE_PIOA->PIO_OER = GREEN_LED | YELLOW_LED;
	/* Turn all LEDs OFF */
	AT91C_BASE_PIOA->PIO_SODR = GREEN_LED | YELLOW_LED;

	/* Disable peripherals on LEDs */
	AT91C_BASE_PIOC->PIO_PER = RED_LED;
	/* Enable pins as outputs */
	AT91C_BASE_PIOC->PIO_OER = RED_LED;
	/* Turn all LEDs OFF */
	AT91C_BASE_PIOC->PIO_CODR = RED_LED;
}

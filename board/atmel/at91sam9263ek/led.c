/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
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
#include <asm/arch/at91sam9263.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>

#define	RED_LED		AT91_PIN_PB7	/* this is the power led */
#define	GREEN_LED	AT91_PIN_PB8	/* this is the user1 led */
#define	YELLOW_LED	AT91_PIN_PC29	/* this is the user2 led */

void red_LED_on(void)
{
	at91_set_gpio_value(RED_LED, 1);
}

void red_LED_off(void)
{
	at91_set_gpio_value(RED_LED, 0);
}

void green_LED_on(void)
{
	at91_set_gpio_value(GREEN_LED, 0);
}

void green_LED_off(void)
{
	at91_set_gpio_value(GREEN_LED, 1);
}

void yellow_LED_on(void)
{
	at91_set_gpio_value(YELLOW_LED, 0);
}

void yellow_LED_off(void)
{
	at91_set_gpio_value(YELLOW_LED, 1);
}

void coloured_LED_init(void)
{
	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9263_ID_PIOB |
				      1 << AT91SAM9263_ID_PIOCDE);

	at91_set_gpio_output(RED_LED, 1);
	at91_set_gpio_output(GREEN_LED, 1);
	at91_set_gpio_output(YELLOW_LED, 1);

	at91_set_gpio_value(RED_LED, 0);
	at91_set_gpio_value(GREEN_LED, 1);
	at91_set_gpio_value(YELLOW_LED, 1);
}

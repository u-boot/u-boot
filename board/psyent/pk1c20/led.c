/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <nios2-io.h>
#include <status_led.h>

/* The LED port is configured as output only, so we
 * must track the state manually.
 */
static led_id_t val = 0;

void __led_init (led_id_t mask, int state)
{
	nios_pio_t *pio = (nios_pio_t *)CONFIG_SYS_LEDPIO_ADDR;

	if (state == STATUS_LED_ON)
		val &= ~mask;
	else
		val |= mask;
	writel (val, &pio->data);
}

void __led_set (led_id_t mask, int state)
{
	nios_pio_t *pio = (nios_pio_t *)CONFIG_SYS_LEDPIO_ADDR;

	if (state == STATUS_LED_ON)
		val &= ~mask;
	else
		val |= mask;
	writel (val, &pio->data);
}

void __led_toggle (led_id_t mask)
{
	nios_pio_t *pio = (nios_pio_t *)CONFIG_SYS_LEDPIO_ADDR;

	val ^= mask;
	writel (val, &pio->data);
}

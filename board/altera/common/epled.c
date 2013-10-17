/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

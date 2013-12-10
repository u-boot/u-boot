/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Ilko Iliev <www.ronetix.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/gpio.h>

void coloured_LED_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable clock */
	writel(1 << ATMEL_ID_PIOB, &pmc->pcer);

	gpio_direction_output(CONFIG_RED_LED, 1);
	gpio_direction_output(CONFIG_GREEN_LED, 1);

	gpio_set_value(CONFIG_RED_LED, 0);
	gpio_set_value(CONFIG_GREEN_LED, 1);
}

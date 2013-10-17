/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91sam9263.h>

void coloured_LED_init(void)
{
	/* Enable clock */
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	writel(1 << ATMEL_ID_PIOB | 1 << ATMEL_ID_PIOCDE,
		&pmc->pcer);

	at91_set_gpio_output(CONFIG_RED_LED, 1);
	at91_set_gpio_output(CONFIG_GREEN_LED, 1);
	at91_set_gpio_output(CONFIG_YELLOW_LED, 1);

	at91_set_gpio_value(CONFIG_RED_LED, 0);
	at91_set_gpio_value(CONFIG_GREEN_LED, 1);
	at91_set_gpio_value(CONFIG_YELLOW_LED, 1);
}

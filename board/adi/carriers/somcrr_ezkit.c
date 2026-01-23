// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2025 - Analog Devices, Inc.
 */

#include <asm/gpio.h>

#include "somcrr.h"

void adi_somcrr_enable_ethernet(void)
{
	struct gpio_desc *eth1;
	struct gpio_desc *eth1_reset;
	struct gpio_desc *gige_reset;

	gpio_hog_lookup_name("eth1-en", &eth1);
	gpio_hog_lookup_name("eth1-reset", &eth1_reset);
	gpio_hog_lookup_name("gige-reset", &gige_reset);

	dm_gpio_set_value(eth1, 1);
	dm_gpio_set_value(eth1_reset, 0);
	dm_gpio_set_value(gige_reset, 0);
}

void adi_somcrr_disable_ethernet(void)
{
	struct gpio_desc *eth1;
	struct gpio_desc *eth1_reset;
	struct gpio_desc *gige_reset;

	gpio_hog_lookup_name("eth1-en", &eth1);
	gpio_hog_lookup_name("eth1-reset", &eth1_reset);
	gpio_hog_lookup_name("gige-reset", &gige_reset);

	dm_gpio_set_value(eth1, 0);
	dm_gpio_set_value(eth1_reset, 1);
	dm_gpio_set_value(gige_reset, 1);
}

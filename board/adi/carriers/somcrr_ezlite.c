// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2025 - Analog Devices, Inc.
 */

#include <asm/gpio.h>

#include "somcrr.h"

void adi_somcrr_enable_ethernet(void)
{
	struct gpio_desc *gige_reset;

	gpio_hog_lookup_name("eth0-reset", &gige_reset);
	dm_gpio_set_value(gige_reset, 0);
}

void adi_somcrr_disable_ethernet(void)
{
	struct gpio_desc *gige_reset;

	gpio_hog_lookup_name("eth0-reset", &gige_reset);
	dm_gpio_set_value(gige_reset, 1);
}

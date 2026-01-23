// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2025 - Analog Devices, Inc.
 */

#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <linux/delay.h>

#include "somcrr.h"

void adi_somcrr_init_ethernet(void)
{
	adi_somcrr_enable_ethernet();
	mdelay(20);
	adi_somcrr_disable_ethernet();
	mdelay(90);
	adi_somcrr_enable_ethernet();
	mdelay(20);
}

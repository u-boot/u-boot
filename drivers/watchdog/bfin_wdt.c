/*
 * watchdog.c - driver for Blackfin on-chip watchdog
 *
 * Copyright (c) 2007-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <watchdog.h>
#include <asm/blackfin.h>
#include <asm/clock.h>
#include <asm/mach-common/bits/watchdog.h>

void hw_watchdog_reset(void)
{
	bfin_write_WDOG_STAT(0);
}

void hw_watchdog_init(void)
{
	bfin_write_WDOG_CTL(WDDIS);
	SSYNC();
	bfin_write_WDOG_CNT(CONFIG_WATCHDOG_TIMEOUT_MSECS / 1000 * get_sclk());
	hw_watchdog_reset();
	bfin_write_WDOG_CTL(WDEN);
}

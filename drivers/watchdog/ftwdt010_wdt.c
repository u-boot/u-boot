/*
 * Watchdog driver for the FTWDT010 Watch Dog Driver
 *
 * (c) Copyright 2004 Faraday Technology Corp. (www.faraday-tech.com)
 * Based on sa1100_wdt.c by Oleg Drokin <green@crimea.edu>
 * Based on SoftDog driver by Alan Cox <alan@redhat.com>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * 27/11/2004 Initial release, Faraday.
 * 12/01/2011 Port to u-boot, Macpaul Lin.
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <faraday/ftwdt010_wdt.h>

/*
 * Set the watchdog time interval.
 * Counter is 32 bit.
 */
int ftwdt010_wdt_settimeout(unsigned int timeout)
{
	unsigned int reg;

	struct ftwdt010_wdt *wd = (struct ftwdt010_wdt *)CONFIG_FTWDT010_BASE;

	debug("Activating WDT..\n");

	/* Check if disabled */
	if (readl(&wd->wdcr) & ~FTWDT010_WDCR_ENABLE) {
		printf("sorry, watchdog is disabled\n");
		return -1;
	}

	/*
	 * In a 66MHz system,
	 * if you set WDLOAD as 0x03EF1480 (66000000)
	 * the reset timer is 1 second.
	 */
	reg = FTWDT010_WDLOAD(timeout * FTWDT010_TIMEOUT_FACTOR);

	writel(reg, &wd->wdload);

	return 0;
}

void ftwdt010_wdt_reset(void)
{
	struct ftwdt010_wdt *wd = (struct ftwdt010_wdt *)CONFIG_FTWDT010_BASE;

	/* clear control register */
	writel(0, &wd->wdcr);

	/* Write Magic number */
	writel(FTWDT010_WDRESTART_MAGIC, &wd->wdrestart);

	/* Enable WDT */
	writel((FTWDT010_WDCR_RST | FTWDT010_WDCR_ENABLE), &wd->wdcr);
}

void ftwdt010_wdt_disable(void)
{
	struct ftwdt010_wdt *wd = (struct ftwdt010_wdt *)CONFIG_FTWDT010_BASE;

	debug("Deactivating WDT..\n");

	/*
	 * It was defined with CONFIG_WATCHDOG_NOWAYOUT in Linux
	 *
	 * Shut off the timer.
	 * Lock it in if it's a module and we defined ...NOWAYOUT
	 */
	writel(0, &wd->wdcr);
}

#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
	ftwdt010_wdt_reset();
}

void hw_watchdog_init(void)
{
	/* set timer in ms */
	ftwdt010_wdt_settimeout(CONFIG_FTWDT010_HW_TIMEOUT * 1000);
}
#endif

/*
 * [origin: Linux kernel drivers/watchdog/at91sam9_wdt.c]
 *
 * Watchdog driver for Atmel AT91SAM9x processors.
 *
 * Copyright (C) 2008 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 * Copyright (C) 2008 Renaud CERRATO r.cerrato@til-technologies.fr
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

/*
 * The Watchdog Timer Mode Register can be only written to once. If the
 * timeout need to be set from U-Boot, be sure that the bootstrap doesn't
 * write to this register. Inform Linux to it too
 */

#include <common.h>
#include <watchdog.h>
#include <asm/arch/hardware.h>
#include <asm/arch/io.h>
#include <asm/arch/at91_wdt.h>

/*
 * AT91SAM9 watchdog runs a 12bit counter @ 256Hz,
 * use this to convert a watchdog
 * value from/to milliseconds.
 */
#define ms_to_ticks(t)	(((t << 8) / 1000) - 1)
#define ticks_to_ms(t)	(((t + 1) * 1000) >> 8)

/* Hardware timeout in seconds */
#define WDT_HW_TIMEOUT 2

/*
 * Set the watchdog time interval in 1/256Hz (write-once)
 * Counter is 12 bit.
 */
static int at91_wdt_settimeout(unsigned int timeout)
{
	unsigned int reg;
	unsigned int mr;

	/* Check if disabled */
	mr = at91_sys_read(AT91_WDT_MR);
	if (mr & AT91_WDT_WDDIS) {
		printf("sorry, watchdog is disabled\n");
		return -1;
	}

	/*
	 * All counting occurs at SLOW_CLOCK / 128 = 256 Hz
	 *
	 * Since WDV is a 12-bit counter, the maximum period is
	 * 4096 / 256 = 16 seconds.
	 */
	reg = AT91_WDT_WDRSTEN	/* causes watchdog reset */
		/* | AT91_WDT_WDRPROC	causes processor reset only */
		| AT91_WDT_WDDBGHLT		/* disabled in debug mode */
		| AT91_WDT_WDD			/* restart at any time */
		| (timeout & AT91_WDT_WDV);	/* timer value */
	at91_sys_write(AT91_WDT_MR, reg);

	return 0;
}

void hw_watchdog_reset(void)
{
	at91_sys_write(AT91_WDT_CR, AT91_WDT_KEY | AT91_WDT_WDRSTT);
}

void hw_watchdog_init(void)
{
	/* 16 seconds timer, resets enabled */
	at91_wdt_settimeout(ms_to_ticks(WDT_HW_TIMEOUT * 1000));
}

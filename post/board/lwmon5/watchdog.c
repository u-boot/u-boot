/*
 * (C) Copyright 2008 Dmitry Rakhchev, EmCraft Systems, rda@emcraft.com
 *
 * Developed for DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/* This test verifies if the reason of last reset was an abnormal voltage
 * condition, than it performs watchdog test, measuing time required to
 * trigger watchdog reset.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_WATCHDOG

#include <watchdog.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/io.h>

static uint watchdog_magic_read(void)
{
	return in_be32((void *)CONFIG_SYS_WATCHDOG_FLAGS_ADDR) &
		CONFIG_SYS_WATCHDOG_MAGIC_MASK;
}

static void watchdog_magic_write(uint value)
{
	out_be32((void *)CONFIG_SYS_WATCHDOG_FLAGS_ADDR, value |
		(in_be32((void *)CONFIG_SYS_WATCHDOG_FLAGS_ADDR) &
			~CONFIG_SYS_WATCHDOG_MAGIC_MASK));
}

int sysmon1_post_test(int flags)
{
	if (gpio_read_in_bit(CONFIG_SYS_GPIO_SYSMON_STATUS) == 0) {
		/*
		 * 3.1. GPIO62 is low
		 * Assuming system voltage failure.
		 */
		post_log("sysmon1 Abnormal voltage detected (GPIO62)\n");
		post_log("POST sysmon1 FAILED\n");
		return 1;
	} else {
		post_log("sysmon1 PASSED\n");
	}

	return 0;
}

int lwmon5_watchdog_post_test(int flags)
{
	/* On each reset scratch register 1 should be tested,
	 * but first test GPIO62:
	 */
	if (!(flags & POST_MANUAL) && sysmon1_post_test(flags)) {
		/* 3.1. GPIO62 is low
		 * Assuming system voltage failure.
		 */
		/* 3.1.1. Set scratch register 1 to 0x0000xxxx */
		watchdog_magic_write(0);
		/* 3.1.2. Mark test as failed due to voltage?! */
		return 1;
	}

	if (watchdog_magic_read() != CONFIG_SYS_WATCHDOG_MAGIC) {
		/* 3.2. Scratch register 1 differs from magic value 0x1248xxxx
		 * Assuming PowerOn
		 */
		int ints;
		ulong base;
		ulong time;

		/* 3.2.1. Set magic value to scratch register */
		watchdog_magic_write(CONFIG_SYS_WATCHDOG_MAGIC);

		ints = disable_interrupts ();
		/* 3.2.2. strobe watchdog once */
		WATCHDOG_RESET();
		out_be32((void *)CONFIG_SYS_WATCHDOG_TIME_ADDR, 0);
		/* 3.2.3. save time of strobe in scratch register 2 */
		base = post_time_ms (0);

		/* 3.2.4. Wait for 150 ms (enough for reset to happen) */
		while ((time = post_time_ms (base)) < 150)
			out_be32((void *)CONFIG_SYS_WATCHDOG_TIME_ADDR, time);
		if (ints)
			enable_interrupts ();

		/* 3.2.5. Reset didn't happen. - Set 0x0000xxxx
		 * into scratch register 1
		 */
		watchdog_magic_write(0);
		/* 3.2.6. Mark test as failed. */
		post_log("hw watchdog time : %u ms, failed ", time);
		return 2;
	} else {
		/* 3.3. Scratch register matches magic value 0x1248xxxx
		 * Assume this is watchdog-initiated reset
		 */
		ulong time;
		/* 3.3.1. So, the test succeed, save measured time to syslog. */
		time = in_be32((void *)CONFIG_SYS_WATCHDOG_TIME_ADDR);
		if (time > 90 ) { /* ms*/
			post_log("hw watchdog time : %u ms, passed ", time);
			/* 3.3.2. Set scratch register 1 to 0x0000xxxx */
			watchdog_magic_write(0);
			return 0;
		} else {
			/*test minimum watchdogtime */
			post_log("hw watchdog time : %u ms, failed ", time);
			return 2;
		}
	}
	return -1;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_WATCHDOG */

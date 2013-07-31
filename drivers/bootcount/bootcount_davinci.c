/*
 * (C) Copyright 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <bootcount.h>
#include <asm/arch/da850_lowlevel.h>
#include <asm/arch/davinci_misc.h>

void bootcount_store(ulong a)
{
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_BOOTCOUNT_ADDR;

	/*
	 * write RTC kick register to enable write
	 * for RTC Scratch registers. Scratch0 and 1 are
	 * used for bootcount values.
	 */
	writel(RTC_KICK0R_WE, &reg->kick0r);
	writel(RTC_KICK1R_WE, &reg->kick1r);
	raw_bootcount_store(&reg->scratch0, a);
	raw_bootcount_store(&reg->scratch1, BOOTCOUNT_MAGIC);
}

ulong bootcount_load(void)
{
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_BOOTCOUNT_ADDR;

	if (raw_bootcount_load(&reg->scratch1) != BOOTCOUNT_MAGIC)
		return 0;
	else
		return raw_bootcount_load(&reg->scratch0);
}

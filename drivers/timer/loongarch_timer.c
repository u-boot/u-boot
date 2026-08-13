// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <asm/loongarch.h>

static u64 notrace loongarch_timer_get_count(struct udevice *dev)
{
	u32 hi, lo;

	if (IS_ENABLED(CONFIG_64BIT))
		return drdtime();

	do {
		hi = rdtimeh();
		lo = rdtimel();
	} while (hi != rdtimeh());

	return ((u64)hi << 32) | lo;
}

static unsigned int loongarch_timer_get_freq_cpucfg(void)
{
	unsigned int res;
	unsigned int base_freq;
	unsigned int cfm, cfd;

	res = read_cpucfg(LOONGARCH_CPUCFG2);
	if (!(res & CPUCFG2_LLFTP))
		return 0;

	base_freq = read_cpucfg(LOONGARCH_CPUCFG4);
	res = read_cpucfg(LOONGARCH_CPUCFG5);
	cfm = res & 0xffff;
	cfd = (res >> 16) & 0xffff;

	if (!base_freq || !cfm || !cfd)
		return 0;

	return (base_freq * cfm / cfd);
}

#if IS_ENABLED(CONFIG_TIMER_EARLY)
/**
 * timer_early_get_rate() - Get the timer rate before driver model
 */
unsigned long notrace timer_early_get_rate(void)
{
	return loongarch_timer_get_freq_cpucfg();
}

/**
 * timer_early_get_count() - Get the timer count before driver model
 *
 */
u64 notrace timer_early_get_count(void)
{
	return loongarch_timer_get_count(NULL);
}
#endif

#if CONFIG_IS_ENABLED(BOOTSTAGE)
ulong timer_get_boot_us(void)
{
	int ret;
	u64 ticks = 0;
	u32 rate;

	ret = dm_timer_init();
	if (!ret) {
		rate = timer_get_rate(gd->timer);
		timer_get_count(gd->timer, &ticks);
	} else {
		rate = loongarch_timer_get_freq_cpucfg();
		ticks = loongarch_timer_get_count(NULL);
	}

	/* Below is converted from time(us) = (tick / rate) * 10000000 */
	return lldiv(ticks * 1000, (rate / 1000));
}
#endif

static int loongarch_timer_bind(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	u32 rate;

	rate = loongarch_timer_get_freq_cpucfg();
	uc_priv->clock_rate = rate;

	return 0;
}

static const struct timer_ops loongarch_timer_ops = {
	.get_count = loongarch_timer_get_count,
};

U_BOOT_DRIVER(loongarch_timer) = {
	.name = "loongarch_timer",
	.id = UCLASS_TIMER,
	.probe = loongarch_timer_bind,
	.ops = &loongarch_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRVINFO(loongarch_timer) = {
	.name = "loongarch_timer",
};

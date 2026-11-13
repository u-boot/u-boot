// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <asm/loongarch.h>
#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <timer.h>

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

static int loongarch_timer_get_freq_cpucfg(unsigned int *freq)
{
	unsigned int res;
	unsigned int base_freq;
	unsigned int cfm, cfd;

	res = read_cpucfg(LOONGARCH_CPUCFG2);
	if (!(res & CPUCFG2_LLFTP))
		return -ENODEV;

	base_freq = read_cpucfg(LOONGARCH_CPUCFG4);
	res = read_cpucfg(LOONGARCH_CPUCFG5);
	cfm = res & 0xffff;
	cfd = (res >> 16) & 0xffff;

	if (!base_freq || !cfm || !cfd)
		return -EINVAL;

	*freq = (unsigned long)base_freq * cfm / cfd;

	return 0;
}

#if IS_ENABLED(CONFIG_TIMER_EARLY)
/**
 * timer_early_get_rate() - Get the timer rate before driver model
 */
unsigned long notrace timer_early_get_rate(void)
{
	unsigned int freq;
	int ret = loongarch_timer_get_freq_cpucfg(&freq);

	if (ret)
		panic("Failed to read timer frequency from cpucfg: %d\n", ret);

	return freq;
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
		ret = loongarch_timer_get_freq_cpucfg(&rate);
		if (ret)
			panic("failed to read timer frequency from cpucfg: %d\n", ret);

		ticks = loongarch_timer_get_count(NULL);
	}

	/* Below is converted from time(us) = (tick / rate) * 1000000 */
	return lldiv(ticks * 1000, (rate / 1000));
}
#endif

static int loongarch_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	unsigned int rate;
	int ret;

	ret = loongarch_timer_get_freq_cpucfg(&rate);
	if (ret < 0) {
		dev_err(dev, "failed to read timer frequency from cpucfg: %d\n",
			ret);
		return ret;
	}

	uc_priv->clock_rate = rate;

	return 0;
}

static const struct timer_ops loongarch_timer_ops = {
	.get_count = loongarch_timer_get_count,
};

U_BOOT_DRIVER(loongarch_timer) = {
	.name = "loongarch_timer",
	.id = UCLASS_TIMER,
	.probe = loongarch_timer_probe,
	.ops = &loongarch_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRVINFO(loongarch_timer) = {
	.name = "loongarch_timer",
};

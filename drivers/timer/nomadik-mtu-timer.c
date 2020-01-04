// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 *
 * Based on arch/arm/cpu/armv7/u8500/timer.c:
 * Copyright (C) 2010 Linaro Limited
 * John Rigby <john.rigby@linaro.org>
 *
 * Based on Linux kernel source and internal ST-Ericsson U-Boot source:
 * Copyright (C) 2009 Alessandro Rubini
 * Copyright (C) 2010 ST-Ericsson
 * Copyright (C) 2010 Linus Walleij for ST-Ericsson
 */

#include <common.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>

#define MTU_NUM_TIMERS		4

/* The timers */
struct nomadik_mtu_timer_regs {
	u32 lr;		/* Load register */
	u32 cv;		/* Current value */
	u32 cr;		/* Control register */
	u32 bglr;	/* Background load register */
};

/* The MTU that contains the timers */
struct nomadik_mtu_regs {
	u32 imsc;	/* Interrupt mask set/clear */
	u32 ris;	/* Raw interrupt status */
	u32 mis;	/* Masked interrupt status */
	u32 icr;	/* Interrupt clear register */

	struct nomadik_mtu_timer_regs timers[MTU_NUM_TIMERS];
};

/* Bits for the control register */
#define MTU_CR_ONESHOT		BIT(0)	/* if 0 = wraps reloading from BGLR */
#define MTU_CR_32BITS		BIT(1)	/* if 0 = 16-bit counter */

#define MTU_CR_PRESCALE_SHIFT	2
#define MTU_CR_PRESCALE_1	(0 << MTU_CR_PRESCALE_SHIFT)
#define MTU_CR_PRESCALE_16	(1 << MTU_CR_PRESCALE_SHIFT)
#define MTU_CR_PRESCALE_256	(2 << MTU_CR_PRESCALE_SHIFT)

#define MTU_CR_PERIODIC		BIT(6)	/* if 0 = free-running */
#define MTU_CR_ENABLE		BIT(7)

struct nomadik_mtu_priv {
	struct nomadik_mtu_timer_regs *timer;
};

static int nomadik_mtu_get_count(struct udevice *dev, u64 *count)
{
	struct nomadik_mtu_priv *priv = dev_get_priv(dev);

	/* Decrementing counter: invert the value */
	*count = timer_conv_64(~readl(&priv->timer->cv));

	return 0;
}

static int nomadik_mtu_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct nomadik_mtu_priv *priv = dev_get_priv(dev);
	struct nomadik_mtu_regs *mtu;
	fdt_addr_t addr;
	u32 prescale;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	mtu = (struct nomadik_mtu_regs *)addr;
	priv->timer = mtu->timers; /* Use first timer */

	if (!uc_priv->clock_rate)
		return -EINVAL;

	/* Use divide-by-16 counter if tick rate is more than 32 MHz */
	if (uc_priv->clock_rate > 32000000) {
		uc_priv->clock_rate /= 16;
		prescale = MTU_CR_PRESCALE_16;
	} else {
		prescale = MTU_CR_PRESCALE_1;
	}

	/* Configure a free-running, auto-wrap counter with selected prescale */
	writel(MTU_CR_ENABLE | prescale | MTU_CR_32BITS, &priv->timer->cr);

	return 0;
}

static const struct timer_ops nomadik_mtu_ops = {
	.get_count = nomadik_mtu_get_count,
};

static const struct udevice_id nomadik_mtu_ids[] = {
	{ .compatible = "st,nomadik-mtu" },
	{}
};

U_BOOT_DRIVER(nomadik_mtu) = {
	.name = "nomadik_mtu",
	.id = UCLASS_TIMER,
	.of_match = nomadik_mtu_ids,
	.priv_auto_alloc_size = sizeof(struct nomadik_mtu_priv),
	.probe = nomadik_mtu_probe,
	.ops = &nomadik_mtu_ops,
};

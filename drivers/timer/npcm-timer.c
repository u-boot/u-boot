// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <dm.h>
#include <timer.h>
#include <asm/io.h>

#define NPCM_TIMER_CLOCK_RATE	25000000UL	/* 25MHz */

/* Register offsets */
#define SECCNT	0x0	/* Seconds Counter Register */
#define CNTR25M	0x4	/* 25MHz Counter Register */

struct npcm_timer_priv {
	void __iomem *base;
};

static u64 npcm_timer_get_count(struct udevice *dev)
{
	struct npcm_timer_priv *priv = dev_get_priv(dev);
	u64 reg_sec, reg_25m;
	u64 counter;

	reg_sec = readl(priv->base + SECCNT);
	reg_25m = readl(priv->base + CNTR25M);
	/*
	 * When CNTR25M reaches 25M, it goes to 0 and SECCNT is increased by 1.
	 * When CNTR25M is zero, wait for CNTR25M to become non-zero in case
	 * SECCNT is not updated yet.
	 */
	if (reg_25m == 0) {
		while (reg_25m == 0)
			reg_25m = readl(priv->base + CNTR25M);
		reg_sec = readl(priv->base + SECCNT);
	}
	counter = reg_sec * NPCM_TIMER_CLOCK_RATE + reg_25m;

	return counter;
}

static int npcm_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct npcm_timer_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;
	uc_priv->clock_rate = NPCM_TIMER_CLOCK_RATE;

	return 0;
}

static const struct timer_ops npcm_timer_ops = {
	.get_count = npcm_timer_get_count,
};

static const struct udevice_id npcm_timer_ids[] = {
	{ .compatible = "nuvoton,npcm845-timer"},
	{ .compatible = "nuvoton,npcm750-timer"},
	{}
};

U_BOOT_DRIVER(npcm_timer) = {
	.name	= "npcm_timer",
	.id	= UCLASS_TIMER,
	.of_match = npcm_timer_ids,
	.priv_auto = sizeof(struct npcm_timer_priv),
	.probe = npcm_timer_probe,
	.ops	= &npcm_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

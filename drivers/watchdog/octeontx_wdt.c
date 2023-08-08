// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define CORE0_POKE_OFFSET 0x50000

#if CONFIG_IS_ENABLED(ARCH_OCTEONTX)
#define REG_BASE 0x844000000000
#elif CONFIG_IS_ENABLED(ARCH_OCTEONTX2)
#define REG_BASE 0x802000000000
#endif

struct octeontx_wdt {
	void __iomem *reg;
};

static struct udevice *wdt_dev;

static int octeontx_wdt_reset(struct udevice *dev)
{
	struct octeontx_wdt *priv;
	u64 poke_reg;

	if (dev) {
		priv = dev_get_priv(dev);
		poke_reg = ((u64)priv->reg & ~0xfffffULL) | CORE0_POKE_OFFSET;
	} else {
		poke_reg = REG_BASE + CORE0_POKE_OFFSET;
	}
	writeq(~0ULL, poke_reg);

	return 0;
}

static int octeontx_wdt_probe(struct udevice *dev)
{
	struct octeontx_wdt *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE)
		return -ENODEV;

	priv->reg = (void __iomem *)addr;
	wdt_dev = dev;

	return 0;
}

static const struct wdt_ops octeontx_wdt_ops = {
	.reset = octeontx_wdt_reset,
};

static const struct udevice_id octeontx_wdt_ids[] = {
	{ .compatible = "arm,sbsa-gwdt" },
	{}
};

U_BOOT_DRIVER(wdt_octeontx) = {
	.name = "wdt_octeontx",
	.id = UCLASS_WDT,
	.of_match = octeontx_wdt_ids,
	.ops = &octeontx_wdt_ops,
	.priv_auto_alloc_size = sizeof(struct octeontx_wdt),
	.probe = octeontx_wdt_probe,
};

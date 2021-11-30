// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <asm/io.h>

#define DART_PARAMS2		0x0004
#define  DART_PARAMS2_BYPASS_SUPPORT	BIT(0)
#define DART_TLB_OP		0x0020
#define  DART_TLB_OP_OPMASK	(0xfff << 20)
#define  DART_TLB_OP_FLUSH	(0x001 << 20)
#define  DART_TLB_OP_BUSY	BIT(2)
#define DART_TLB_OP_SIDMASK	0x0034
#define DART_ERROR_STATUS	0x0040
#define DART_TCR(sid)		(0x0100 + 4 * (sid))
#define  DART_TCR_TRANSLATE_ENABLE	BIT(7)
#define  DART_TCR_BYPASS_DART		BIT(8)
#define  DART_TCR_BYPASS_DAPF		BIT(12)
#define DART_TTBR(sid, idx)	(0x0200 + 16 * (sid) + 4 * (idx))
#define  DART_TTBR_VALID	BIT(31)
#define  DART_TTBR_SHIFT	12

static int apple_dart_probe(struct udevice *dev)
{
	void *base;
	int sid, i;

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	u32 params2 = readl(base + DART_PARAMS2);
	if (params2 & DART_PARAMS2_BYPASS_SUPPORT) {
		for (sid = 0; sid < 16; sid++) {
			writel(DART_TCR_BYPASS_DART | DART_TCR_BYPASS_DAPF,
			       base + DART_TCR(sid));
			for (i = 0; i < 4; i++)
				writel(0, base + DART_TTBR(sid, i));
		}
	}

	return 0;
}

static const struct udevice_id apple_dart_ids[] = {
	{ .compatible = "apple,t8103-dart" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(apple_dart) = {
	.name = "apple_dart",
	.id = UCLASS_IOMMU,
	.of_match = apple_dart_ids,
	.probe = apple_dart_probe
};

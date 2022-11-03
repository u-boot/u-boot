// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019, Rick Chen <rick@andestech.com>
 *
 * U-Boot syscon driver for Andes's Platform Level Interrupt Controller (PLIC).
 * The PLIC block holds memory-mapped claim and pending registers
 * associated with software interrupt.
 */

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/syscon.h>
#include <cpu.h>
#include <linux/err.h>

/* pending register */
#define PENDING_REG(base, hart)	((ulong)(base) + 0x1000 + ((hart) / 4) * 4)
/* enable register */
#define ENABLE_REG(base, hart)	((ulong)(base) + 0x2000 + (hart) * 0x80)
/* claim register */
#define CLAIM_REG(base, hart)	((ulong)(base) + 0x200004 + (hart) * 0x1000)

#define ENABLE_HART_IPI         (0x01010101)
#define SEND_IPI_TO_HART(hart)  (0x1 << (hart))

DECLARE_GLOBAL_DATA_PTR;

static int enable_ipi(int hart)
{
	unsigned int en;

	en = ENABLE_HART_IPI << hart;
	writel(en, (void __iomem *)ENABLE_REG(gd->arch.plicsw, hart));
	writel(en, (void __iomem *)ENABLE_REG(gd->arch.plicsw + 0x4, hart));

	return 0;
}

int riscv_init_ipi(void)
{
	int ret;
	long *base = syscon_get_first_range(RISCV_SYSCON_PLICSW);
	ofnode node;
	struct udevice *dev;
	u32 reg;

	if (IS_ERR(base))
		return PTR_ERR(base);
	gd->arch.plicsw = base;

	ret = uclass_find_first_device(UCLASS_CPU, &dev);
	if (ret)
		return ret;
	else if (!dev)
		return -ENODEV;

	ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
		const char *device_type;

		device_type = ofnode_read_string(node, "device_type");
		if (!device_type)
			continue;

		if (strcmp(device_type, "cpu"))
			continue;

		/* skip if hart is marked as not available */
		if (!ofnode_is_enabled(node))
			continue;

		/* read hart ID of CPU */
		ret = ofnode_read_u32(node, "reg", &reg);
		if (ret == 0)
			enable_ipi(reg);
	}

	return 0;
}

int riscv_send_ipi(int hart)
{
	unsigned int ipi = (SEND_IPI_TO_HART(hart) << (8 * gd->arch.boot_hart));

	writel(ipi, (void __iomem *)PENDING_REG(gd->arch.plicsw,
				gd->arch.boot_hart));

	return 0;
}

int riscv_clear_ipi(int hart)
{
	u32 source_id;

	source_id = readl((void __iomem *)CLAIM_REG(gd->arch.plicsw, hart));
	writel(source_id, (void __iomem *)CLAIM_REG(gd->arch.plicsw, hart));

	return 0;
}

int riscv_get_ipi(int hart, int *pending)
{
	unsigned int ipi = (SEND_IPI_TO_HART(hart) << (8 * gd->arch.boot_hart));

	*pending = readl((void __iomem *)PENDING_REG(gd->arch.plicsw,
						     gd->arch.boot_hart));
	*pending = !!(*pending & ipi);

	return 0;
}

static const struct udevice_id andes_plicsw_ids[] = {
	{ .compatible = "andestech,plicsw", .data = RISCV_SYSCON_PLICSW },
	{ }
};

U_BOOT_DRIVER(andes_plicsw) = {
	.name		= "andes_plicsw",
	.id		= UCLASS_SYSCON,
	.of_match	= andes_plicsw_ids,
	.flags		= DM_FLAG_PRE_RELOC,
};

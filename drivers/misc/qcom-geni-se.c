// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm Generic Interface (GENI) Serial Engine (SE) Wrapper
 *
 * Copyright (C) 2023 Linaro Ltd. <vladimir.zapolskiy@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>

static int geni_se_qup_read(struct udevice *dev, int offset,
			    void *buf, int size)
{
	fdt_addr_t base = dev_read_addr(dev);

	if (size != sizeof(u32))
		return -EINVAL;

	*(u32 *)buf = readl(base + offset);

	return size;
}

static struct misc_ops geni_se_qup_ops = {
	.read = geni_se_qup_read,
};

static const struct udevice_id geni_se_qup_ids[] = {
	{ .compatible = "qcom,geni-se-qup" },
	{}
};

U_BOOT_DRIVER(geni_se_qup) = {
	.name = "geni_se_qup",
	.id = UCLASS_MISC,
	.of_match = geni_se_qup_ids,
	.ops = &geni_se_qup_ops,
	.flags  = DM_FLAG_PRE_RELOC,
};

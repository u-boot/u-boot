// SPDX-License-Identifier: GPL-2.0+
// SPDX-FileCopyrightText: 2024 CERN (home.cern)

#include <bootcount.h>
#include <dm.h>
#include <stdio.h>
#include <zynqmp_firmware.h>
#include <asm/arch/hardware.h>
#include <dm/platdata.h>

static int bootcount_zynqmp_set(struct udevice *dev, const u32 val)
{
	int ret;

	ret = zynqmp_mmio_write((ulong)&pmu_base->pers_gen_storage2, 0xFF, val);
	if (ret)
		pr_info("%s write fail\n", __func__);

	return ret;
}

static int bootcount_zynqmp_get(struct udevice *dev, u32 *val)
{
	int ret;

	*val = 0;
	ret = zynqmp_mmio_read((ulong)&pmu_base->pers_gen_storage2, val);
	if (ret)
		pr_info("%s read fail\n", __func__);

	return ret;
}

U_BOOT_DRVINFO(bootcount_zynqmp) = {
	.name = "bootcount_zynqmp",
};

static const struct bootcount_ops bootcount_zynqmp_ops = {
	.get = bootcount_zynqmp_get,
	.set = bootcount_zynqmp_set,
};

U_BOOT_DRIVER(bootcount_zynqmp) = {
	.name = "bootcount_zynqmp",
	.id = UCLASS_BOOTCOUNT,
	.ops = &bootcount_zynqmp_ops,
};

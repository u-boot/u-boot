// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Pali Rohár <pali@kernel.org>
 * Copyright (C) 2024 Marek Behún <kabel@kernel.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>
#include <regmap.h>
#include <reset-uclass.h>
#include <syscon.h>
#include <asm/io.h>

#define MVEBU_SOC_CONTROL_1_REG 0x4

#define MVEBU_PCIE_ID 0

static int mvebu_reset_of_xlate(struct reset_ctl *rst,
				struct ofnode_phandle_args *args)
{
	if (args->args_count < 2)
		return -EINVAL;

	rst->id = args->args[0];
	rst->data = args->args[1];

	/* Currently only PCIe is implemented */
	if (rst->id != MVEBU_PCIE_ID)
		return -EINVAL;

	/* Four PCIe enable bits are shared across more PCIe links */
	if (!(rst->data >= 0 && rst->data <= 3))
		return -EINVAL;

	return 0;
}

static int mvebu_reset_request(struct reset_ctl *rst)
{
	return 0;
}

static int mvebu_reset_free(struct reset_ctl *rst)
{
	return 0;
}

static int mvebu_reset_assert(struct reset_ctl *rst)
{
	struct regmap *regmap = syscon_get_regmap(rst->dev->parent);

	return regmap_update_bits(regmap, MVEBU_SOC_CONTROL_1_REG,
				  BIT(rst->data), 0);
}

static int mvebu_reset_deassert(struct reset_ctl *rst)
{
	struct regmap *regmap = syscon_get_regmap(rst->dev->parent);

	return regmap_update_bits(regmap, MVEBU_SOC_CONTROL_1_REG,
				  BIT(rst->data), BIT(rst->data));
}

static int mvebu_reset_status(struct reset_ctl *rst)
{
	struct regmap *regmap = syscon_get_regmap(rst->dev->parent);
	uint val;
	int ret;

	ret = regmap_read(regmap, MVEBU_SOC_CONTROL_1_REG, &val);
	if (ret < 0)
		return ret;

	return !(val & BIT(rst->data));
}

static const struct reset_ops mvebu_reset_ops = {
	.of_xlate = mvebu_reset_of_xlate,
	.request = mvebu_reset_request,
	.rfree = mvebu_reset_free,
	.rst_assert = mvebu_reset_assert,
	.rst_deassert = mvebu_reset_deassert,
	.rst_status = mvebu_reset_status,
};

U_BOOT_DRIVER(mvebu_reset) = {
	.name = "mvebu-reset",
	.id = UCLASS_RESET,
	.ops = &mvebu_reset_ops,
};

static int mvebu_syscon_bind(struct udevice *dev)
{
	/* bind also mvebu-reset, with the same ofnode */
	return device_bind_driver_to_node(dev, "mvebu-reset", "mvebu-reset",
					  dev_ofnode(dev), NULL);
}

static const struct udevice_id mvebu_syscon_of_match[] = {
	{ .compatible = "marvell,armada-370-xp-system-controller" },
	{ .compatible = "marvell,armada-375-system-controller" },
	{ .compatible = "marvell,armada-380-system-controller" },
	{ .compatible = "marvell,armada-390-system-controller" },
	{ },
};

U_BOOT_DRIVER(mvebu_syscon) = {
	.name = "mvebu-system-controller",
	.id = UCLASS_SYSCON,
	.of_match = mvebu_syscon_of_match,
	.bind = mvebu_syscon_bind,
};

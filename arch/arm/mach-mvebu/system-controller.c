// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Pali Rohár <pali@kernel.org>
 * Copyright (C) 2024 Marek Behún <kabel@kernel.org>
 */

#include <dm.h>
#include <dm/lists.h>
#include <regmap.h>
#include <reset-uclass.h>
#include <syscon.h>
#include <sysreset.h>
#include <asm/io.h>

#define MVEBU_SOC_CONTROL_1_REG		0x4

#if defined(CONFIG_ARMADA_375)
# define MVEBU_RSTOUTN_MASK_REG		0x54
# define MVEBU_SYS_SOFT_RST_REG		0x58
#else
# define MVEBU_RSTOUTN_MASK_REG		0x60
# define MVEBU_SYS_SOFT_RST_REG		0x64
#endif

#define MVEBU_GLOBAL_SOFT_RST_BIT	BIT(0)

#define MVEBU_PCIE_ID			0

#if IS_ENABLED(CONFIG_ARMADA_32BIT_SYSCON_RESET)

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

#endif /* IS_ENABLED(CONFIG_ARMADA_32BIT_SYSCON_RESET) */

#if IS_ENABLED(CONFIG_ARMADA_32BIT_SYSCON_SYSRESET)

static int mvebu_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct regmap *regmap = syscon_get_regmap(dev->parent);
	uint bit;

	if (type != SYSRESET_COLD)
		return -EPROTONOSUPPORT;

	bit = MVEBU_GLOBAL_SOFT_RST_BIT;

	regmap_update_bits(regmap, MVEBU_RSTOUTN_MASK_REG, bit, bit);
	regmap_update_bits(regmap, MVEBU_SYS_SOFT_RST_REG, bit, bit);

	/* Loop while waiting for the reset */
	while (1)
		;

	return 0;
}

static struct sysreset_ops mvebu_sysreset_ops = {
	.request = mvebu_sysreset_request,
};

U_BOOT_DRIVER(mvebu_sysreset) = {
	.name = "mvebu-sysreset",
	.id = UCLASS_SYSRESET,
	.ops = &mvebu_sysreset_ops,
};

#endif /* IS_ENABLED(CONFIG_ARMADA_32BIT_SYSCON_SYSRESET) */

static int mvebu_syscon_bind(struct udevice *dev)
{
	int ret = 0;

	/* bind also mvebu-reset, with the same ofnode */
	if (IS_ENABLED(CONFIG_ARMADA_32BIT_SYSCON_RESET)) {
		ret = device_bind_driver_to_node(dev, "mvebu-reset",
						 "mvebu-reset", dev_ofnode(dev),
						 NULL);
		if (ret < 0)
			return ret;
	}

	/* bind also mvebu-sysreset, with the same ofnode */
	if (IS_ENABLED(CONFIG_ARMADA_32BIT_SYSCON_SYSRESET)) {
		ret = device_bind_driver_to_node(dev, "mvebu-sysreset",
						 "mvebu-sysreset",
						 dev_ofnode(dev), NULL);
		if (ret < 0)
			return ret;
	}

	return ret;
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

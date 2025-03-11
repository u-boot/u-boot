// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Renesas Electronics Corporation
 */

#include <asm/io.h>
#include <dm.h>
#include <power/regulator.h>
#include <renesas/rzg2l-usbphy.h>

#define VBENCTL			0x03c
#define VBENCTL_VBUS_SEL	BIT(0)

static int rzg2l_usbphy_regulator_set_enable(struct udevice *dev, bool enable)
{
	struct rzg2l_usbphy_ctrl_priv *priv = dev_get_priv(dev->parent);

	if (enable)
		clrbits_le32(priv->regs + VBENCTL, VBENCTL_VBUS_SEL);
	else
		setbits_le32(priv->regs + VBENCTL, VBENCTL_VBUS_SEL);

	return 0;
}

static int rzg2l_usbphy_regulator_get_enable(struct udevice *dev)
{
	struct rzg2l_usbphy_ctrl_priv *priv = dev_get_priv(dev->parent);

	return !!readl(priv->regs + VBENCTL) & VBENCTL_VBUS_SEL;
}

static const struct dm_regulator_ops rzg2l_usbphy_regulator_ops = {
	.get_enable = rzg2l_usbphy_regulator_get_enable,
	.set_enable = rzg2l_usbphy_regulator_set_enable,
};

U_BOOT_DRIVER(rzg2l_usbphy_regulator) = {
	.name = "rzg2l_usbphy_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &rzg2l_usbphy_regulator_ops,
};

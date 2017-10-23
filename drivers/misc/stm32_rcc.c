/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <dm/lists.h>

static int stm32_rcc_bind(struct udevice *dev)
{
	int ret;
	struct udevice *child;

	debug("%s(dev=%p)\n", __func__, dev);

	ret = device_bind_driver_to_node(dev, "stm32h7_rcc_clock",
					 "stm32h7_rcc_clock",
					 dev_ofnode(dev), &child);
	if (ret)
		return ret;

	return device_bind_driver_to_node(dev, "stm32_rcc_reset",
					  "stm32_rcc_reset",
					  dev_ofnode(dev), &child);
}

static const struct misc_ops stm32_rcc_ops = {
};

static const struct udevice_id stm32_rcc_ids[] = {
	{.compatible = "st,stm32h743-rcc"},
	{ }
};

U_BOOT_DRIVER(stm32_rcc) = {
	.name		= "stm32-rcc",
	.id		= UCLASS_MISC,
	.of_match	= stm32_rcc_ids,
	.bind		= stm32_rcc_bind,
	.ops		= &stm32_rcc_ops,
};

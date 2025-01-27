// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx8mq_pinctrl_soc_info __section(".data");

static const struct udevice_id imx8m_pinctrl_match[] = {
	{ .compatible = "fsl,imx8mq-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mm-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mn-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mp-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ /* sentinel */ }
};

static const struct pinctrl_ops imx8m_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_mmio,
};

U_BOOT_DRIVER(imx8mq_pinctrl) = {
	.name = "imx8mq-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx8m_pinctrl_match),
	.probe = imx_pinctrl_probe_mmio,
	.remove = imx_pinctrl_remove_mmio,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx8m_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

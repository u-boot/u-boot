// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx93_pinctrl_soc_info __section(".data") = {
	.flags = ZERO_OFFSET_VALID,
};

static const struct udevice_id imx93_pinctrl_match[] = {
	{ .compatible = "fsl,imx93-iomuxc", .data = (ulong)&imx93_pinctrl_soc_info },
	{ .compatible = "fsl,imx91-iomuxc", .data = (ulong)&imx93_pinctrl_soc_info },
	{ /* sentinel */ }
};

static const struct pinctrl_ops imx93_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_mmio,
};

U_BOOT_DRIVER(imx93_pinctrl) = {
	.name = "imx93-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx93_pinctrl_match),
	.probe = imx_pinctrl_probe_mmio,
	.remove = imx_pinctrl_remove_mmio,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx93_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

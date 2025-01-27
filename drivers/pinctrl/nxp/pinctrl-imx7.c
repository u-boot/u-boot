// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 */

#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx7_pinctrl_soc_info __section(".data");

static struct imx_pinctrl_soc_info imx7_lpsr_pinctrl_soc_info = {
	.flags = ZERO_OFFSET_VALID,
};

static const struct udevice_id imx7_pinctrl_match[] = {
	{ .compatible = "fsl,imx7d-iomuxc", .data = (ulong)&imx7_pinctrl_soc_info },
	{ .compatible = "fsl,imx7d-iomuxc-lpsr", .data = (ulong)&imx7_lpsr_pinctrl_soc_info },
	{ /* sentinel */ }
};

static const struct pinctrl_ops imx7_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_mmio,
};

U_BOOT_DRIVER(imx7_pinctrl) = {
	.name = "imx7-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx7_pinctrl_match),
	.probe = imx_pinctrl_probe_mmio,
	.remove = imx_pinctrl_remove_mmio,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx7_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

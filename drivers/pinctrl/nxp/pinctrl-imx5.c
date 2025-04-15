// SPDX-License-Identifier: GPL-2.0+

/*
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 */

#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx5_pinctrl_soc_info __section(".data");

static const struct udevice_id imx5_pinctrl_match[] = {
	{
		.compatible = "fsl,imx53-iomuxc",
		.data = (ulong)&imx5_pinctrl_soc_info
	},
	{
		.compatible = "fsl,imx53-iomuxc-gpr",
		.data = (ulong)&imx5_pinctrl_soc_info
	},
	{ /* sentinel */ }
};

static const struct pinctrl_ops imx5_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_mmio,
};

U_BOOT_DRIVER(imx5_pinctrl) = {
	.name = "imx5-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx5_pinctrl_match),
	.probe = imx_pinctrl_probe_mmio,
	.remove = imx_pinctrl_remove_mmio,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx5_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

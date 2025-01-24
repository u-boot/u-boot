// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <asm/global_data.h>
#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

DECLARE_GLOBAL_DATA_PTR;

static struct imx_pinctrl_soc_info imx8_pinctrl_soc_info = {
	.flags = IMX8_USE_SCU,
};

static const struct udevice_id imx8_pinctrl_match[] = {
	{ .compatible = "fsl,imx8qxp-iomuxc", .data = (ulong)&imx8_pinctrl_soc_info },
	{ .compatible = "fsl,imx8qm-iomuxc", .data = (ulong)&imx8_pinctrl_soc_info },
	{ /* sentinel */ }
};

static const struct pinctrl_ops imx8_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state,
};

U_BOOT_DRIVER(imx8_pinctrl) = {
	.name = "imx8_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx8_pinctrl_match),
	.probe = imx_pinctrl_probe_common,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx8_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

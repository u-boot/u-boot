// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx7ulp_pinctrl_soc_info0 = {
	.flags = ZERO_OFFSET_VALID | SHARE_MUX_CONF_REG | CFG_IBE_OBE,
};

static struct imx_pinctrl_soc_info imx7ulp_pinctrl_soc_info1 = {
	.flags = ZERO_OFFSET_VALID | SHARE_MUX_CONF_REG | CFG_IBE_OBE,
};

static const struct udevice_id imx7ulp_pinctrl_match[] = {
	{ .compatible = "fsl,imx7ulp-iomuxc-0", .data = (ulong)&imx7ulp_pinctrl_soc_info0 },
	{ .compatible = "fsl,imx7ulp-iomuxc-1", .data = (ulong)&imx7ulp_pinctrl_soc_info1 },
	{ /* sentinel */ }
};

static const struct pinctrl_ops imx7ulp_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_mmio,
};

U_BOOT_DRIVER(imx7ulp_pinctrl) = {
	.name = "imx7ulp-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx7ulp_pinctrl_match),
	.probe = imx_pinctrl_probe_mmio,
	.remove = imx_pinctrl_remove_mmio,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx7ulp_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

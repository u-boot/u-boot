// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info vf610_pinctrl_soc_info = {
	.flags = SHARE_MUX_CONF_REG | ZERO_OFFSET_VALID,
	.mux_mask = 0x700000,
};

static const struct udevice_id vf610_pinctrl_match[] = {
	{ .compatible = "fsl,vf610-iomuxc",
	  .data = (ulong)&vf610_pinctrl_soc_info },
	{ /* sentinel */ }
};

static const struct pinctrl_ops vf610_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_mmio,
};

U_BOOT_DRIVER(vf610_pinctrl) = {
	.name = "vf610-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(vf610_pinctrl_match),
	.probe = imx_pinctrl_probe_mmio,
	.remove = imx_pinctrl_remove_mmio,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &vf610_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

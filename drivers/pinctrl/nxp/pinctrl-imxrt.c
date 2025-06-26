// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imxrt_pinctrl_soc_info = {
	.flags = ZERO_OFFSET_VALID,
	.mux_mask = 0x7,
};

static const struct udevice_id imxrt_pinctrl_match[] = {
	{ .compatible = "fsl,imxrt-iomuxc",
	  .data = (ulong)&imxrt_pinctrl_soc_info },
	{ /* sentinel */ }
};

static const struct pinctrl_ops imxrt_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_mmio,
};

U_BOOT_DRIVER(imxrt_pinctrl) = {
	.name = "imxrt-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imxrt_pinctrl_match),
	.probe = imx_pinctrl_probe_mmio,
	.remove = imx_pinctrl_remove_mmio,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imxrt_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

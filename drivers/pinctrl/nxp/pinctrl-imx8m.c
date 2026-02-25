// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <asm/io.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx8mq_pinctrl_soc_info __section(".data");

static const struct udevice_id imx8m_pinctrl_match[] = {
	{ .compatible = "fsl,imx8mq-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mm-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mn-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mp-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ /* sentinel */ }
};

#if CONFIG_IS_ENABLED(CMD_PINMUX)

#if IS_ENABLED(CONFIG_IMX8MP)
#include "pinctrl-imx8mp.c"
#elif IS_ENABLED(CONFIG_IMX8MN)
#include "pinctrl-imx8mn.c"
#endif

static int imx8m_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(imx8m_pinctrl_pads);
}

static const char *imx8m_get_pin_name(struct udevice *dev,
				      unsigned int selector)
{
	/* sanity checking */
	if (selector != imx8m_pinctrl_pads[selector].number) {
		dev_err(dev,
			"selector(%u) not match with imx8m_pinctrl_pads[selector].number(%u)\n",
			selector, imx8m_pinctrl_pads[selector].number);
		return NULL;
	}

	return imx8m_pinctrl_pads[selector].name;
}

static int imx8m_get_pin_muxing(struct udevice *dev, unsigned int selector,
				char *buf, int size)
{
	struct imx_pinctrl_priv *priv = dev_get_priv(dev);
	struct imx_pinctrl_soc_info *info = priv->info;
	u32 mux_reg = selector << 2;
	u32 mux_mode = readl(info->base + mux_reg);

	snprintf(buf, size, "Function(%d) at: 0x%p", mux_mode & 0x7, info->base + mux_reg);

	return 0;
}
#endif

static const struct pinctrl_ops imx8m_pinctrl_ops = {
#if CONFIG_IS_ENABLED(CMD_PINMUX)
	.get_pin_name = imx8m_get_pin_name,
	.get_pins_count = imx8m_get_pins_count,
	.get_pin_muxing = imx8m_get_pin_muxing,
#endif
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

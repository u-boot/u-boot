// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <firmware/imx/sci/sci.h>
#include <misc.h>
#include <asm/global_data.h>
#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

DECLARE_GLOBAL_DATA_PTR;

#define PADRING_IFMUX_EN_SHIFT		31
#define PADRING_IFMUX_EN_MASK		BIT(31)
#define PADRING_GP_EN_SHIFT		30
#define PADRING_GP_EN_MASK		BIT(30)
#define PADRING_IFMUX_SHIFT		27
#define PADRING_IFMUX_MASK		GENMASK(29, 27)

static int imx_pinconf_scu_set(struct imx_pinctrl_soc_info *info, u32 pad,
			       u32 mux, u32 val)
{
	int ret;

	/*
	 * Mux should be done in pmx set, but we do not have a good api
	 * to handle that in scfw, so config it in pad conf func
	 */

	if (!sc_rm_is_pad_owned(-1, pad)) {
		debug("Pad[%u] is not owned by curr partition\n", pad);
		return -EPERM;
	}

	val |= PADRING_IFMUX_EN_MASK;
	val |= PADRING_GP_EN_MASK;
	val |= (mux << PADRING_IFMUX_SHIFT) & PADRING_IFMUX_MASK;

	ret = sc_pad_set(-1, pad, val);
	if (ret)
		printf("%s %d\n", __func__, ret);

	return 0;
}

int imx_pinctrl_set_state_scu(struct udevice *dev, struct udevice *config)
{
	struct imx_pinctrl_priv *priv = dev_get_priv(dev);
	struct imx_pinctrl_soc_info *info = priv->info;
	int pin_id, mux, config_val;
	u32 *pin_data;
	int i, j = 0;
	int npins;
	int ret;

	ret = imx_pinctrl_set_state_common(dev, config, SHARE_IMX8_PIN_SIZE,
					   &pin_data, &npins);
	if (ret)
		return ret;

	/*
	 * Refer to linux documentation for details:
	 * Documentation/devicetree/bindings/pinctrl/fsl,imx-pinctrl.txt
	 */
	for (i = 0; i < npins; i++) {
		pin_id = pin_data[j++];
		mux = pin_data[j++];
		config_val = pin_data[j++];

		ret = imx_pinconf_scu_set(info, pin_id, mux, config_val);
		if (ret && ret != -EPERM)
			printf("Set pin %d, mux %d, val %d, error\n", pin_id,
			       mux, config_val);
	}

	return 0;
}
static struct imx_pinctrl_soc_info imx8_pinctrl_soc_info = {
	.flags = IMX8_USE_SCU,
};

static const struct udevice_id imx8_pinctrl_match[] = {
	{ .compatible = "fsl,imx8qxp-iomuxc", .data = (ulong)&imx8_pinctrl_soc_info },
	{ .compatible = "fsl,imx8qm-iomuxc", .data = (ulong)&imx8_pinctrl_soc_info },
	{ /* sentinel */ }
};

static const struct pinctrl_ops imx8_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_scu,
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

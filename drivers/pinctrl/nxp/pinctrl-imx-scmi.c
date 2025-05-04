// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include <asm/io.h>
#include <asm/mach-imx/sys_proto.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/pinctrl.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>

#include "pinctrl-imx.h"

#define DAISY_OFFSET_IMX95      0x408

/* SCMI pin control types */
#define PINCTRL_TYPE_MUX        192
#define PINCTRL_TYPE_CONFIG     193
#define PINCTRL_TYPE_DAISY_ID   194
#define PINCTRL_TYPE_DAISY_CFG  195
#define PINCTRL_NUM_CFGS_SHIFT  2

struct imx_scmi_pinctrl_priv {
	u16		daisy_offset;
};

static int imx_pinconf_scmi_set(struct udevice *dev, u32 mux_ofs, u32 mux, u32 config_val,
				u32 input_ofs, u32 input_val)
{
	struct imx_scmi_pinctrl_priv *priv = dev_get_priv(dev);
	int ret, num_cfgs = 0;
	struct scmi_msg msg;

	/* Call SCMI API to set the pin mux and configuration. */
	struct scmi_pinctrl_config_set_out out;
	struct scmi_pinctrl_config_set_in in = {
		.identifier = mux_ofs / 4,
		.function_id = 0xFFFFFFFF,
		.attributes = 0,
	};

	if (mux_ofs) {
		in.configs[num_cfgs].type = PINCTRL_TYPE_MUX;
		in.configs[num_cfgs].val = mux;
		num_cfgs++;
	}

	if (config_val) {
		in.configs[num_cfgs].type = PINCTRL_TYPE_CONFIG;
		in.configs[num_cfgs].val = config_val;
		num_cfgs++;
	}

	if (input_ofs) {
		in.configs[num_cfgs].type = PINCTRL_TYPE_DAISY_ID;
		in.configs[num_cfgs].val = (input_ofs -  priv->daisy_offset) / 4;
		num_cfgs++;
		in.configs[num_cfgs].type = PINCTRL_TYPE_DAISY_CFG;
		in.configs[num_cfgs].val = input_val;
		num_cfgs++;
	}

	/* Update the number of configs sent in this call. */
	in.attributes = num_cfgs << PINCTRL_NUM_CFGS_SHIFT;

	msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_PINCTRL,
			  SCMI_MSG_PINCTRL_CONFIG_SET, in, out);

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret || out.status) {
		dev_err(dev, "Failed to set PAD = %d, daisy = %d, scmi_err = %d, ret = %d\n",
			mux_ofs / 4, input_ofs / 4, out.status, ret);
	}

	return ret;
}

static int imx_pinctrl_set_state_scmi(struct udevice *dev, struct udevice *config)
{
	int mux_ofs, mux, config_val, input_reg, input_val;
	u32 *pin_data;
	int i, j = 0;
	int npins;
	int ret;

	ret = imx_pinctrl_set_state_common(dev, config, FSL_PIN_SIZE,
					   &pin_data, &npins);
	if (ret)
		return ret;

	/*
	 * Refer to linux documentation for details:
	 * Documentation/devicetree/bindings/pinctrl/fsl,imx-pinctrl.txt
	 */
	for (i = 0; i < npins; i++) {
		mux_ofs = pin_data[j++];
		/* Skip config_reg */
		j++;
		input_reg = pin_data[j++];

		mux = pin_data[j++];
		input_val = pin_data[j++];
		config_val = pin_data[j++];

		if (config_val & IMX_PAD_SION)
			mux |= IOMUXC_CONFIG_SION;

		config_val &= ~IMX_PAD_SION;

		ret = imx_pinconf_scmi_set(dev, mux_ofs, mux, config_val, input_reg, input_val);
		if (ret && ret != -EPERM) {
			dev_err(dev, "Set pin %d, mux %d, val %d, error\n",
				mux_ofs, mux, config_val);
		}
	}

	devm_kfree(dev, pin_data);

	return ret;
}

static const struct pinctrl_ops imx_scmi_pinctrl_ops = {
	.set_state = imx_pinctrl_set_state_scmi,
};

static int imx_scmi_pinctrl_probe(struct udevice *dev)
{
	struct imx_scmi_pinctrl_priv *priv = dev_get_priv(dev);

	if (IS_ENABLED(CONFIG_IMX95))
		priv->daisy_offset = DAISY_OFFSET_IMX95;
	else
		return -EINVAL;

	return devm_scmi_of_get_channel(dev);
}

static int imx_scmi_pinctrl_bind(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_IMX95))
		return 0;

	return -ENODEV;
}

U_BOOT_DRIVER(scmi_pinctrl_imx) = {
	.name = "scmi_pinctrl_imx",
	.id = UCLASS_PINCTRL,
	.bind = imx_scmi_pinctrl_bind,
	.probe = imx_scmi_pinctrl_probe,
	.priv_auto = sizeof(struct imx_scmi_pinctrl_priv),
	.ops = &imx_scmi_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static struct scmi_proto_match match[] = {
	{ .proto_id = SCMI_PROTOCOL_ID_PINCTRL },
	{ /* Sentinel */ }
};

U_BOOT_SCMI_PROTO_DRIVER(scmi_pinctrl_imx, match);

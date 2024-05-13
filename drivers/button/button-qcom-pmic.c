// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm generic pmic gpio driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 * (C) Copyright 2023 Linaro Ltd.
 */

#include <button.h>
#include <dt-bindings/input/linux-event-codes.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <log.h>
#include <power/pmic.h>
#include <spmi/spmi.h>
#include <linux/bitops.h>

#define REG_TYPE		0x4
#define REG_SUBTYPE		0x5

struct qcom_pmic_btn_data {
	char *compatible;
	unsigned int status_bit;
	int code;
	char *label;
};

struct qcom_pmic_btn_priv {
	u32 base;
	u32 status_bit;
	int code;
	struct udevice *pmic;
};

#define PON_INT_RT_STS                        0x10
#define  PON_KPDPWR_N_SET		0
#define  PON_RESIN_N_SET		1
#define  PON_GEN3_RESIN_N_SET		6
#define  PON_GEN3_KPDPWR_N_SET		7

static enum button_state_t qcom_pwrkey_get_state(struct udevice *dev)
{
	struct qcom_pmic_btn_priv *priv = dev_get_priv(dev);

	int reg = pmic_reg_read(priv->pmic, priv->base + PON_INT_RT_STS);

	if (reg < 0)
		return 0;

	return (reg & BIT(priv->status_bit)) != 0;
}

static int qcom_pwrkey_get_code(struct udevice *dev)
{
	struct qcom_pmic_btn_priv *priv = dev_get_priv(dev);

	return priv->code;
}

static const struct qcom_pmic_btn_data qcom_pmic_btn_data_table[] = {
	{
		.compatible = "qcom,pm8941-pwrkey",
		.status_bit = PON_KPDPWR_N_SET,
		.code = KEY_ENTER,
		.label = "pwrkey",
	},
	{
		.compatible = "qcom,pm8941-resin",
		.status_bit = PON_RESIN_N_SET,
		.code = KEY_DOWN,
		.label = "vol_down",
	},
	{
		.compatible = "qcom,pmk8350-pwrkey",
		.status_bit = PON_GEN3_KPDPWR_N_SET,
		.code = KEY_ENTER,
		.label = "pwrkey",
	},
	{
		.compatible = "qcom,pmk8350-resin",
		.status_bit = PON_GEN3_RESIN_N_SET,
		.code = KEY_DOWN,
		.label = "vol_down",
	},
};

static const struct qcom_pmic_btn_data *button_qcom_pmic_match(ofnode node)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(qcom_pmic_btn_data_table); ++i) {
		if (ofnode_device_is_compatible(node,
						qcom_pmic_btn_data_table[i].compatible))
			return &qcom_pmic_btn_data_table[i];
	}

	return NULL;
}

static int qcom_pwrkey_probe(struct udevice *dev)
{
	struct button_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct qcom_pmic_btn_priv *priv = dev_get_priv(dev);
	const struct qcom_pmic_btn_data *btn_data;
	ofnode node = dev_ofnode(dev);
	int ret;
	u64 base;

	/* Ignore the top-level pon node */
	if (!uc_plat->label)
		return 0;

	/* Get the data for the node compatible */
	btn_data = button_qcom_pmic_match(node);
	if (!btn_data)
		return -EINVAL;

	priv->status_bit = btn_data->status_bit;
	priv->code = btn_data->code;

	/* the pwrkey and resin nodes are children of the "pon" node, get the
	 * PMIC device to use in pmic_reg_* calls.
	 */
	priv->pmic = dev->parent->parent;

	/* Get the address of the parent pon node */
	base = dev_read_addr(dev->parent);
	if (base == FDT_ADDR_T_NONE) {
		printf("%s: Can't find address\n", dev->name);
		return -EINVAL;
	}

	priv->base = base;

	/* Do a sanity check */
	ret = pmic_reg_read(priv->pmic, priv->base + REG_TYPE);
	if (ret != 0x1 && ret != 0xb) {
		printf("%s: unexpected PMIC function type %d\n", dev->name, ret);
		return -ENXIO;
	}

	ret = pmic_reg_read(priv->pmic, priv->base + REG_SUBTYPE);
	if (ret < 0 || (ret & 0x7) == 0) {
		printf("%s: unexpected PMIC function subtype %d\n", dev->name, ret);
		return -ENXIO;
	}

	return 0;
}

static int button_qcom_pmic_bind(struct udevice *parent)
{
	struct udevice *dev;
	ofnode node;
	int ret;

	dev_for_each_subnode(node, parent) {
		const struct qcom_pmic_btn_data *btn_data;
		struct button_uc_plat *uc_plat;
		const char *label;

		if (!ofnode_is_enabled(node))
			continue;

		/* Get the data for the node compatible */
		btn_data = button_qcom_pmic_match(node);
		if (!btn_data) {
			debug("Unknown button node '%s'\n", ofnode_get_name(node));
			continue;
		}

		ret = device_bind_driver_to_node(parent, "qcom_pwrkey",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret) {
			printf("Failed to bind %s! %d\n", label, ret);
			return ret;
		}
		uc_plat = dev_get_uclass_plat(dev);
		uc_plat->label = btn_data->label;
	}

	return 0;
}

static const struct button_ops button_qcom_pmic_ops = {
	.get_state	= qcom_pwrkey_get_state,
	.get_code	= qcom_pwrkey_get_code,
};

static const struct udevice_id qcom_pwrkey_ids[] = {
	{ .compatible = "qcom,pm8916-pon" },
	{ .compatible = "qcom,pm8941-pon" },
	{ .compatible = "qcom,pm8998-pon" },
	{ .compatible = "qcom,pmk8350-pon" },
	{ }
};

U_BOOT_DRIVER(qcom_pwrkey) = {
	.name = "qcom_pwrkey",
	.id = UCLASS_BUTTON,
	.of_match = qcom_pwrkey_ids,
	.bind = button_qcom_pmic_bind,
	.probe = qcom_pwrkey_probe,
	.ops = &button_qcom_pmic_ops,
	.priv_auto = sizeof(struct qcom_pmic_btn_priv),
};

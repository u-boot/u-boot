/*
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd
 *
 * Based on kernel drivers/regulator/pwm-regulator.c
 * Copyright (C) 2014 - STMicroelectronics Inc.
 * Author: Lee Jones <lee.jones@linaro.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pwm.h>
#include <power/regulator.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

struct pwm_regulator_info {
	/* pwm id corresponding to the PWM driver */
	int pwm_id;
	/* the period of one PWM cycle */
	int period_ns;
	struct udevice *pwm;
	/* initialize voltage of regulator */
	unsigned int init_voltage;
	/* the maximum voltage of regulator */
	unsigned int max_voltage;
	/* the minimum voltage of regulator */
	unsigned int min_voltage;
	/* the current voltage of regulator */
	unsigned int volt_uV;
};

static int pwm_regulator_enable(struct udevice *dev, bool enable)
{
	struct pwm_regulator_info *priv = dev_get_priv(dev);

	return pwm_set_enable(priv->pwm, priv->pwm_id, enable);
}

static int pwm_voltage_to_duty_cycle_percentage(struct udevice *dev, int req_uV)
{
	struct pwm_regulator_info *priv = dev_get_priv(dev);
	int min_uV = priv->min_voltage;
	int max_uV = priv->max_voltage;
	int diff = max_uV - min_uV;

	return 100 - (((req_uV * 100) - (min_uV * 100)) / diff);
}

static int pwm_regulator_get_voltage(struct udevice *dev)
{
	struct pwm_regulator_info *priv = dev_get_priv(dev);

	return priv->volt_uV;
}

static int pwm_regulator_set_voltage(struct udevice *dev, int uvolt)
{
	struct pwm_regulator_info *priv = dev_get_priv(dev);
	int duty_cycle;
	int ret = 0;

	duty_cycle = pwm_voltage_to_duty_cycle_percentage(dev, uvolt);

	ret = pwm_set_config(priv->pwm, priv->pwm_id,
			(priv->period_ns / 100) * duty_cycle, priv->period_ns);
	if (ret) {
		dev_err(dev, "Failed to configure PWM\n");
		return ret;
	}

	ret = pwm_set_enable(priv->pwm, priv->pwm_id, true);
	if (ret) {
		dev_err(dev, "Failed to enable PWM\n");
		return ret;
	}
	priv->volt_uV = uvolt;
	return ret;
}

static int pwm_regulator_ofdata_to_platdata(struct udevice *dev)
{
	struct pwm_regulator_info *priv = dev_get_priv(dev);
	struct fdtdec_phandle_args args;
	const void *blob = gd->fdt_blob;
	int node = dev->of_offset;
	int ret;

	ret = fdtdec_parse_phandle_with_args(blob, node, "pwms", "#pwm-cells",
					     0, 0, &args);
	if (ret) {
		debug("%s: Cannot get PWM phandle: ret=%d\n", __func__, ret);
		return ret;
	}
	/* TODO: pwm_id here from device tree if needed */

	priv->period_ns = args.args[1];

	priv->init_voltage = fdtdec_get_int(blob, node,
			"regulator-init-microvolt", -1);
	if (priv->init_voltage < 0) {
		printf("Cannot find regulator pwm init_voltage\n");
		return -EINVAL;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_PWM, args.node, &priv->pwm);
	if (ret) {
		debug("%s: Cannot get PWM: ret=%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int pwm_regulator_probe(struct udevice *dev)
{
	struct pwm_regulator_info *priv = dev_get_priv(dev);
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode_count = 0;
	priv->max_voltage = uc_pdata->max_uV;
	priv->min_voltage = uc_pdata->min_uV;

	if (priv->init_voltage)
		pwm_regulator_set_voltage(dev, priv->init_voltage);

	pwm_regulator_enable(dev, 1);

	return 0;
}

static const struct dm_regulator_ops pwm_regulator_ops = {
	.get_value  = pwm_regulator_get_voltage,
	.set_value  = pwm_regulator_set_voltage,
	.set_enable = pwm_regulator_enable,
};

static const struct udevice_id pwm_regulator_ids[] = {
	{ .compatible = "pwm-regulator" },
	{ }
};

U_BOOT_DRIVER(pwm_regulator) = {
	.name = "pwm_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &pwm_regulator_ops,
	.probe = pwm_regulator_probe,
	.of_match = pwm_regulator_ids,
	.ofdata_to_platdata	= pwm_regulator_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct pwm_regulator_info),
};

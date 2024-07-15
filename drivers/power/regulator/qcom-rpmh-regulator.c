// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
// Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/err.h>
#include <dm/device_compat.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <power/regulator.h>
#include <log.h>

#include <soc/qcom/cmd-db.h>
#include <soc/qcom/rpmh.h>

#include <dt-bindings/regulator/qcom,rpmh-regulator.h>

/**
 * enum rpmh_regulator_type - supported RPMh accelerator types
 * @VRM:	RPMh VRM accelerator which supports voting on enable, voltage,
 *		and mode of LDO, SMPS, and BOB type PMIC regulators.
 * @XOB:	RPMh XOB accelerator which supports voting on the enable state
 *		of PMIC regulators.
 */
enum rpmh_regulator_type {
	VRM,
	XOB,
};

enum rpmh_regulator_mode {
	REGULATOR_MODE_RETENTION,
	REGULATOR_MODE_LPM,
	REGULATOR_MODE_AUTO,
	REGULATOR_MODE_HPM,
};

#define RPMH_REGULATOR_REG_VRM_VOLTAGE		0x0
#define RPMH_REGULATOR_REG_ENABLE		0x4
#define RPMH_REGULATOR_REG_VRM_MODE		0x8

#define PMIC4_LDO_MODE_RETENTION		4
#define PMIC4_LDO_MODE_LPM			5
#define PMIC4_LDO_MODE_HPM			7

#define PMIC4_SMPS_MODE_RETENTION		4
#define PMIC4_SMPS_MODE_PFM			5
#define PMIC4_SMPS_MODE_AUTO			6
#define PMIC4_SMPS_MODE_PWM			7

#define PMIC4_BOB_MODE_PASS			0
#define PMIC4_BOB_MODE_PFM			1
#define PMIC4_BOB_MODE_AUTO			2
#define PMIC4_BOB_MODE_PWM			3

#define PMIC5_LDO_MODE_RETENTION		3
#define PMIC5_LDO_MODE_LPM			4
#define PMIC5_LDO_MODE_HPM			7

#define PMIC5_SMPS_MODE_RETENTION		3
#define PMIC5_SMPS_MODE_PFM			4
#define PMIC5_SMPS_MODE_AUTO			6
#define PMIC5_SMPS_MODE_PWM			7

#define PMIC5_BOB_MODE_PASS			2
#define PMIC5_BOB_MODE_PFM			4
#define PMIC5_BOB_MODE_AUTO			6
#define PMIC5_BOB_MODE_PWM			7


/**
 * struct linear_range - table of selector - value pairs
 *
 * Define a lookup-table for range of values. Intended to help when looking
 * for a register value matching certaing physical measure (like voltage).
 * Usable when increment of one in register always results a constant increment
 * of the physical measure (like voltage).
 *
 * @min:  Lowest value in range
 * @min_sel: Lowest selector for range
 * @max_sel: Highest selector for range
 * @step: Value step size
 */
struct linear_range {
	unsigned int min;
	unsigned int min_sel;
	unsigned int max_sel;
	unsigned int step;
};

/* Initialize struct linear_range for regulators */
#define REGULATOR_LINEAR_RANGE(_min_uV, _min_sel, _max_sel, _step_uV)	\
{									\
	.min		= _min_uV,					\
	.min_sel	= _min_sel,					\
	.max_sel	= _max_sel,					\
	.step		= _step_uV,					\
}

/**
 * struct rpmh_vreg_hw_data - RPMh regulator hardware configurations
 * @regulator_type:		RPMh accelerator type used to manage this
 *				regulator
 * @ops:			Pointer to regulator ops callback structure
 * @voltage_range:		The single range of voltages supported by this
 *				PMIC regulator type
 * @n_voltages:			The number of unique voltage set points defined
 *				by voltage_range
 * @hpm_min_load_uA:		Minimum load current in microamps that requires
 *				high power mode (HPM) operation.  This is used
 *				for LDO hardware type regulators only.
 * @pmic_mode_map:		Array indexed by regulator framework mode
 *				containing PMIC hardware modes.  Must be large
 *				enough to index all framework modes supported
 *				by this regulator hardware type.
 * @of_map_mode:		Maps an RPMH_REGULATOR_MODE_* mode value defined
 *				in device tree to a regulator framework mode
 */
struct rpmh_vreg_hw_data {
	enum rpmh_regulator_type		regulator_type;
	const struct dm_regulator_ops		*ops;
	struct linear_range			voltage_range;
	int					n_voltages;
	int					hpm_min_load_uA;
	struct dm_regulator_mode		*pmic_mode_map;
	int					n_modes;
	unsigned int				(*of_map_mode)(unsigned int mode);
};

/**
 * struct rpmh_vreg - individual RPMh regulator data structure encapsulating a
 *		single regulator device
 * @dev:			Device pointer for the top-level PMIC RPMh
 *				regulator parent device.  This is used as a
 *				handle in RPMh write requests.
 * @addr:			Base address of the regulator resource within
 *				an RPMh accelerator
 * @rdesc:			Regulator descriptor
 * @hw_data:			PMIC regulator configuration data for this RPMh
 *				regulator
 * @always_wait_for_ack:	Boolean flag indicating if a request must always
 *				wait for an ACK from RPMh before continuing even
 *				if it corresponds to a strictly lower power
 *				state (e.g. enabled --> disabled).
 * @enabled:			Flag indicating if the regulator is enabled or
 *				not
 * @bypassed:			Boolean indicating if the regulator is in
 *				bypass (pass-through) mode or not.  This is
 *				only used by BOB rpmh-regulator resources.
 * @uv:				Selector used for get_voltage_sel() and
 *				set_value() callbacks
 * @mode:			RPMh VRM regulator current framework mode
 */
struct rpmh_vreg {
	struct udevice			*dev;
	u32				addr;
	const struct rpmh_vreg_hw_data	*hw_data;
	bool				always_wait_for_ack;

	int				enabled;
	bool				bypassed;
	int				uv;
	int			mode;
};

/**
 * struct rpmh_vreg_init_data - initialization data for an RPMh regulator
 * @name:			Name for the regulator which also corresponds
 *				to the device tree subnode name of the regulator
 * @resource_name:		RPMh regulator resource name format string.
 *				This must include exactly one field: '%s' which
 *				is filled at run-time with the PMIC ID provided
 *				by device tree property qcom,pmic-id.  Example:
 *				"ldo%s1" for RPMh resource "ldoa1".
 * @supply_name:		Parent supply regulator name
 * @hw_data:			Configuration data for this PMIC regulator type
 */
struct rpmh_vreg_init_data {
	const char			*name;
	const char			*resource_name;
	const char			*supply_name;
	const struct rpmh_vreg_hw_data	*hw_data;
};

/**
 * rpmh_regulator_send_request() - send the request to RPMh
 * @vreg:		Pointer to the RPMh regulator
 * @cmd:		Pointer to the RPMh command to send
 * @wait_for_ack:	Boolean indicating if execution must wait until the
 *			request has been acknowledged as complete
 *
 * Return: 0 on success, errno on failure
 */
static int rpmh_regulator_send_request(struct rpmh_vreg *vreg,
			struct tcs_cmd *cmd, bool wait_for_ack)
{
	int ret;

	if (wait_for_ack || vreg->always_wait_for_ack)
		ret = rpmh_write(vreg->dev, RPMH_ACTIVE_ONLY_STATE, cmd, 1);
	else
		ret = rpmh_write_async(vreg->dev, RPMH_ACTIVE_ONLY_STATE, cmd,
					1);

	return ret;
}

static int _rpmh_regulator_vrm_set_voltage_sel(struct regulator_dev *rdev,
				unsigned int selector, bool wait_for_ack)
{
	struct rpmh_vreg *vreg = rdev_get_drvdata(rdev);
	struct tcs_cmd cmd = {
		.addr = vreg->addr + RPMH_REGULATOR_REG_VRM_VOLTAGE,
	};
	int ret;

	/* VRM voltage control register is set with voltage in millivolts. */
	cmd.data = DIV_ROUND_UP(regulator_list_voltage_linear_range(rdev,
							selector), 1000);

	ret = rpmh_regulator_send_request(vreg, &cmd, wait_for_ack);
	if (!ret)
		vreg->voltage_selector = selector;

	return ret;
}

static int rpmh_regulator_vrm_set_voltage_sel(struct regulator_dev *rdev,
					unsigned int selector)
{
	struct rpmh_vreg *vreg = rdev_get_drvdata(rdev);

	if (vreg->enabled == -EINVAL) {
		/*
		 * Cache the voltage and send it later when the regulator is
		 * enabled or disabled.
		 */
		vreg->voltage_selector = selector;
		return 0;
	}

	return _rpmh_regulator_vrm_set_voltage_sel(rdev, selector,
					selector > vreg->voltage_selector);
}

static int rpmh_regulator_vrm_get_voltage_sel(struct regulator_dev *rdev)
{
	struct rpmh_vreg *vreg = rdev_get_drvdata(rdev);

	return vreg->voltage_selector;
}

static int rpmh_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct rpmh_vreg *vreg = rdev_get_drvdata(rdev);

	return vreg->enabled;
}

static int rpmh_regulator_set_enable_state(struct regulator_dev *rdev,
					bool enable)
{
	struct rpmh_vreg *vreg = rdev_get_drvdata(rdev);
	struct tcs_cmd cmd = {
		.addr = vreg->addr + RPMH_REGULATOR_REG_ENABLE,
		.data = enable,
	};
	int ret;

	if (vreg->enabled == -EINVAL &&
	    vreg->voltage_selector != -ENOTRECOVERABLE) {
		ret = _rpmh_regulator_vrm_set_voltage_sel(rdev,
						vreg->voltage_selector, true);
		if (ret < 0)
			return ret;
	}

	ret = rpmh_regulator_send_request(vreg, &cmd, enable);
	if (!ret)
		vreg->enabled = enable;

	return ret;
}

static int rpmh_regulator_enable(struct regulator_dev *rdev)
{
	return rpmh_regulator_set_enable_state(rdev, true);
}

static int rpmh_regulator_disable(struct regulator_dev *rdev)
{
	return rpmh_regulator_set_enable_state(rdev, false);
}

static int rpmh_regulator_vrm_set_mode_bypass(struct rpmh_vreg *vreg,
					unsigned int mode, bool bypassed)
{
	struct tcs_cmd cmd = {
		.addr = vreg->addr + RPMH_REGULATOR_REG_VRM_MODE,
	};
	int pmic_mode;

	if (mode > REGULATOR_MODE_STANDBY)
		return -EINVAL;

	pmic_mode = vreg->hw_data->pmic_mode_map[mode];
	if (pmic_mode < 0)
		return pmic_mode;

	if (bypassed)
		cmd.data = PMIC4_BOB_MODE_PASS;
	else
		cmd.data = pmic_mode;

	return rpmh_regulator_send_request(vreg, &cmd, true);
}

static int rpmh_regulator_vrm_set_mode(struct regulator_dev *rdev,
					unsigned int mode)
{
	struct rpmh_vreg *vreg = rdev_get_drvdata(rdev);
	int ret;

	if (mode == vreg->mode)
		return 0;

	ret = rpmh_regulator_vrm_set_mode_bypass(vreg, mode, vreg->bypassed);
	if (!ret)
		vreg->mode = mode;

	return ret;
}

static unsigned int rpmh_regulator_vrm_get_mode(struct regulator_dev *rdev)
{
	struct rpmh_vreg *vreg = rdev_get_drvdata(rdev);

	return vreg->mode;
}

static const struct regulator_ops rpmh_regulator_vrm_drms_ops = {
	.enable			= rpmh_regulator_enable,
	.disable		= rpmh_regulator_disable,
	.is_enabled		= rpmh_regulator_is_enabled,
	.set_voltage_sel	= rpmh_regulator_vrm_set_voltage_sel,
	.get_voltage_sel	= rpmh_regulator_vrm_get_voltage_sel,
	.list_voltage		= regulator_list_voltage_linear_range,
	.set_mode		= rpmh_regulator_vrm_set_mode,
	.get_mode		= rpmh_regulator_vrm_get_mode,
	.get_optimum_mode	= rpmh_regulator_vrm_get_optimum_mode,
};

static const struct rpmh_vreg_hw_data pmic5_pldo = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_ranges = (struct linear_range[]) {
		REGULATOR_LINEAR_RANGE(1504000, 0, 255, 8000),
	},
	.n_linear_ranges = 1,
	.n_voltages = 256,
	.hpm_min_load_uA = 10000,
	.pmic_mode_map = pmic_mode_map_pmic5_ldo,
	.of_map_mode = rpmh_regulator_pmic4_ldo_of_map_mode,
};

static const struct rpmh_vreg_hw_data pmic5_pldo_lv = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_ranges = (struct linear_range[]) {
		REGULATOR_LINEAR_RANGE(1504000, 0, 62, 8000),
	},
	.n_linear_ranges = 1,
	.n_voltages = 63,
	.hpm_min_load_uA = 10000,
	.pmic_mode_map = pmic_mode_map_pmic5_ldo,
	.of_map_mode = rpmh_regulator_pmic4_ldo_of_map_mode,
};

#define RPMH_VREG(_name, _resource_name, _hw_data, _supply_name) \
{ \
	.name		= _name, \
	.resource_name	= _resource_name, \
	.hw_data	= _hw_data, \
	.supply_name	= _supply_name, \
}

static const struct rpmh_vreg_init_data pm8150_vreg_data[] = {
	RPMH_VREG("ldo13",  "ldo%s13", &pmic5_pldo,      "vdd-l13-l16-l17"),
	{}
};

static const struct rpmh_vreg_init_data pm8150l_vreg_data[] = {
	RPMH_VREG("ldo1",   "ldo%s1",  &pmic5_pldo_lv,   "vdd-l1-l8"),
	RPMH_VREG("ldo11",  "ldo%s11", &pmic5_pldo,      "vdd-l7-l11"),
	{}
};

static int rpmh_regulator_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct rpmh_vreg_init_data *vreg_data;
	struct device_node *node;
	struct rpmh_vreg *vreg;
	const char *pmic_id;
	int ret;

	vreg_data = of_device_get_match_data(dev);
	if (!vreg_data)
		return -ENODEV;

	ret = of_property_read_string(dev->of_node, "qcom,pmic-id", &pmic_id);
	if (ret < 0) {
		dev_err(dev, "qcom,pmic-id missing in DT node\n");
		return ret;
	}

	for_each_available_child_of_node(dev->of_node, node) {
		vreg = devm_kzalloc(dev, sizeof(*vreg), GFP_KERNEL);
		if (!vreg) {
			of_node_put(node);
			return -ENOMEM;
		}

		ret = rpmh_regulator_init_vreg(vreg, dev, node, pmic_id,
						vreg_data);
		if (ret < 0) {
			of_node_put(node);
			return ret;
		}
	}

	return 0;
}

static const struct of_device_id __maybe_unused rpmh_regulator_match_table[] = {
	{
		.compatible = "qcom,pm8150-rpmh-regulators",
		.data = pm8150_vreg_data,
	},
	{
		.compatible = "qcom,pm8150l-rpmh-regulators",
		.data = pm8150l_vreg_data,
	},
	{}
};
MODULE_DEVICE_TABLE(of, rpmh_regulator_match_table);

static struct platform_driver rpmh_regulator_driver = {
	.driver = {
		.name = "qcom-rpmh-regulator",
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
		.of_match_table	= of_match_ptr(rpmh_regulator_match_table),
	},
	.probe = rpmh_regulator_probe,
};
module_platform_driver(rpmh_regulator_driver);

MODULE_DESCRIPTION("Qualcomm RPMh regulator driver");
MODULE_LICENSE("GPL v2");

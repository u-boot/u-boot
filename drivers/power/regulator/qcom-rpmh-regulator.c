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
				       const struct tcs_cmd *cmd, bool wait_for_ack)
{
	int ret;

	if (wait_for_ack || vreg->always_wait_for_ack)
		ret = rpmh_write(vreg->dev->parent, RPMH_ACTIVE_ONLY_STATE, cmd, 1);
	else
		ret = rpmh_write_async(vreg->dev->parent, RPMH_ACTIVE_ONLY_STATE, cmd, 1);

	return ret;
}

static int _rpmh_regulator_vrm_set_value(struct udevice *rdev,
					 int uv, bool wait_for_ack)
{
	struct rpmh_vreg *vreg = dev_get_priv(rdev);
	struct tcs_cmd cmd = {
		.addr = vreg->addr + RPMH_REGULATOR_REG_VRM_VOLTAGE,
	};
	int ret;
	unsigned int selector;

	selector = (uv - vreg->hw_data->voltage_range.min) / vreg->hw_data->voltage_range.step;
	cmd.data = DIV_ROUND_UP(vreg->hw_data->voltage_range.min +
				selector * vreg->hw_data->voltage_range.step, 1000);

	ret = rpmh_regulator_send_request(vreg, &cmd, wait_for_ack);
	if (!ret)
		vreg->uv = cmd.data * 1000;

	return ret;
}

static int rpmh_regulator_vrm_set_value(struct udevice *rdev,
					int uv)
{
	struct rpmh_vreg *vreg = dev_get_priv(rdev);

	debug("%s: set_value %d (current %d)\n", rdev->name, uv, vreg->uv);

	if (vreg->enabled == -EINVAL) {
		/*
		 * Cache the voltage and send it later when the regulator is
		 * enabled or disabled.
		 */
		vreg->uv = uv;
		return 0;
	}

	return _rpmh_regulator_vrm_set_value(rdev, uv,
					uv > vreg->uv);
}

static int rpmh_regulator_vrm_get_value(struct udevice *rdev)
{
	struct rpmh_vreg *vreg = dev_get_priv(rdev);

	debug("%s: get_value %d\n", rdev->name, vreg->uv);

	return vreg->uv;
}

static int rpmh_regulator_is_enabled(struct udevice *rdev)
{
	struct rpmh_vreg *vreg = dev_get_priv(rdev);

	debug("%s: is_enabled %d\n", rdev->name, vreg->enabled);

	return vreg->enabled > 0;
}

static int rpmh_regulator_set_enable_state(struct udevice *rdev,
					   bool enable)
{
	struct rpmh_vreg *vreg = dev_get_priv(rdev);
	struct tcs_cmd cmd = {
		.addr = vreg->addr + RPMH_REGULATOR_REG_ENABLE,
		.data = enable,
	};
	int ret;

	debug("%s: set_enable %d (current %d)\n", rdev->name, enable,
	      vreg->enabled);

	if (vreg->enabled == -EINVAL &&
	    vreg->uv != -ENOTRECOVERABLE) {
		ret = _rpmh_regulator_vrm_set_value(rdev,
						    vreg->uv, true);
		if (ret < 0)
			return ret;
	}

	ret = rpmh_regulator_send_request(vreg, &cmd, enable);
	if (!ret)
		vreg->enabled = enable;

	return ret;
}

static int rpmh_regulator_vrm_set_mode_bypass(struct rpmh_vreg *vreg,
					      unsigned int mode, bool bypassed)
{
	struct tcs_cmd cmd = {
		.addr = vreg->addr + RPMH_REGULATOR_REG_VRM_MODE,
	};
	struct dm_regulator_mode *pmic_mode;
	int i;

	if (mode > REGULATOR_MODE_HPM)
		return -EINVAL;

	for (i = 0; i < vreg->hw_data->n_modes; i++) {
		pmic_mode = &vreg->hw_data->pmic_mode_map[i];
		if (pmic_mode->id == mode)
			break;
	}
	if (pmic_mode->id != mode) {
		printf("Invalid mode %d\n", mode);
		return -EINVAL;
	}

	if (bypassed)
		cmd.data = PMIC4_BOB_MODE_PASS;
	else
		cmd.data = pmic_mode->id;

	return rpmh_regulator_send_request(vreg, &cmd, true);
}

static int rpmh_regulator_vrm_set_mode(struct udevice *rdev,
				       int mode)
{
	struct rpmh_vreg *vreg = dev_get_priv(rdev);
	int ret;

	debug("%s: set_mode %d (current %d)\n", rdev->name, mode, vreg->mode);

	if (mode == vreg->mode)
		return 0;

	ret = rpmh_regulator_vrm_set_mode_bypass(vreg, mode, vreg->bypassed);
	if (!ret)
		vreg->mode = mode;

	return ret;
}

static int rpmh_regulator_vrm_get_mode(struct udevice *rdev)
{
	struct rpmh_vreg *vreg = dev_get_priv(rdev);

	debug("%s: get_mode %d\n", rdev->name, vreg->mode);

	return vreg->mode;
}
static const struct dm_regulator_ops rpmh_regulator_vrm_drms_ops = {
	.get_value = rpmh_regulator_vrm_get_value,
	.set_value = rpmh_regulator_vrm_set_value,
	.set_enable = rpmh_regulator_set_enable_state,
	.get_enable = rpmh_regulator_is_enabled,
	.set_mode = rpmh_regulator_vrm_set_mode,
	.get_mode = rpmh_regulator_vrm_get_mode,
};

static struct dm_regulator_mode pmic_mode_map_pmic5_bob[] = {
	{
		.id = REGULATOR_MODE_LPM,
		.register_value = PMIC5_BOB_MODE_PFM,
		.name = "PMIC5_BOB_MODE_PFM"
	}, {
		.id = REGULATOR_MODE_AUTO,
		.register_value = PMIC5_BOB_MODE_AUTO,
		.name = "PMIC5_BOB_MODE_AUTO"
	}, {
		.id = REGULATOR_MODE_HPM,
		.register_value = PMIC5_BOB_MODE_PWM,
		.name = "PMIC5_BOB_MODE_PWM"
	},
};

static struct dm_regulator_mode pmic_mode_map_pmic5_smps[] = {
	{
		.id = REGULATOR_MODE_RETENTION,
		.register_value = PMIC5_SMPS_MODE_RETENTION,
		.name = "PMIC5_SMPS_MODE_RETENTION"
	}, {
		.id = REGULATOR_MODE_LPM,
		.register_value = PMIC5_SMPS_MODE_PFM,
		.name = "PMIC5_SMPS_MODE_PFM"
	}, {
		.id = REGULATOR_MODE_AUTO,
		.register_value = PMIC5_SMPS_MODE_AUTO,
		.name = "PMIC5_SMPS_MODE_AUTO"
	}, {
		.id = REGULATOR_MODE_HPM,
		.register_value = PMIC5_SMPS_MODE_PWM,
		.name = "PMIC5_SMPS_MODE_PWM"
	},
};

static const struct rpmh_vreg_hw_data pmic5_bob = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_range = REGULATOR_LINEAR_RANGE(3000000, 0, 31, 32000),
	.n_voltages = 32,
	.pmic_mode_map = pmic_mode_map_pmic5_bob,
	.n_modes = ARRAY_SIZE(pmic_mode_map_pmic5_bob),
};

static const struct rpmh_vreg_hw_data pmic5_ftsmps525_lv = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_range = REGULATOR_LINEAR_RANGE(300000, 0, 267, 4000),
	.n_voltages = 268,
	.pmic_mode_map = pmic_mode_map_pmic5_smps,
	.n_modes = ARRAY_SIZE(pmic_mode_map_pmic5_smps),
};

static const struct rpmh_vreg_hw_data pmic5_ftsmps525_mv = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_range = REGULATOR_LINEAR_RANGE(600000, 0, 267, 8000),
	.n_voltages = 268,
	.pmic_mode_map = pmic_mode_map_pmic5_smps,
	.n_modes = ARRAY_SIZE(pmic_mode_map_pmic5_smps),
};

static struct dm_regulator_mode pmic_mode_map_pmic5_ldo[] = {
	{
		.id = REGULATOR_MODE_RETENTION,
		.register_value = PMIC5_LDO_MODE_RETENTION,
		.name = "PMIC5_LDO_MODE_RETENTION"
	}, {
		.id = REGULATOR_MODE_LPM,
		.register_value = PMIC5_LDO_MODE_LPM,
		.name = "PMIC5_LDO_MODE_LPM"
	}, {
		.id = REGULATOR_MODE_HPM,
		.register_value = PMIC5_LDO_MODE_HPM,
		.name = "PMIC5_LDO_MODE_HPM"
	},
};

static const struct rpmh_vreg_hw_data pmic5_pldo = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_range = REGULATOR_LINEAR_RANGE(1504000, 0, 255, 8000),
	.n_voltages = 256,
	.hpm_min_load_uA = 10000,
	.pmic_mode_map = pmic_mode_map_pmic5_ldo,
	.n_modes = ARRAY_SIZE(pmic_mode_map_pmic5_ldo),
};

static const struct rpmh_vreg_hw_data pmic5_pldo_lv = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_range = REGULATOR_LINEAR_RANGE(1504000, 0, 62, 8000),
	.n_voltages = 63,
	.hpm_min_load_uA = 10000,
	.pmic_mode_map = pmic_mode_map_pmic5_ldo,
	.n_modes = ARRAY_SIZE(pmic_mode_map_pmic5_ldo),
};

static const struct rpmh_vreg_hw_data pmic5_nldo515 = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_range = REGULATOR_LINEAR_RANGE(320000, 0, 210, 8000),
	.n_voltages = 211,
	.hpm_min_load_uA = 30000,
	.pmic_mode_map = pmic_mode_map_pmic5_ldo,
	.n_modes = ARRAY_SIZE(pmic_mode_map_pmic5_ldo),
};

static const struct rpmh_vreg_hw_data pmic5_ftsmps527 = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_range = REGULATOR_LINEAR_RANGE(320000, 0, 215, 8000),
	.n_voltages = 215,
	.pmic_mode_map = pmic_mode_map_pmic5_smps,
	.n_modes = ARRAY_SIZE(pmic_mode_map_pmic5_smps),
};

static const struct rpmh_vreg_hw_data pmic5_pldo515_mv = {
	.regulator_type = VRM,
	.ops = &rpmh_regulator_vrm_drms_ops,
	.voltage_range = REGULATOR_LINEAR_RANGE(1800000, 0, 187, 8000),
	.n_voltages = 188,
	.hpm_min_load_uA = 10000,
	.pmic_mode_map = pmic_mode_map_pmic5_ldo,
	.n_modes = ARRAY_SIZE(pmic_mode_map_pmic5_ldo),
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
	RPMH_VREG("ldo4",   "ldo%s4",  &pmic5_pldo,      "vdd-l4-l5-l6"),
	RPMH_VREG("ldo5",   "ldo%s5",  &pmic5_pldo,      "vdd-l4-l5-l6"),
	RPMH_VREG("ldo6",   "ldo%s6",  &pmic5_pldo,      "vdd-l4-l5-l6"),
	RPMH_VREG("ldo7",   "ldo%s7",  &pmic5_pldo,      "vdd-l7-l11"),
	RPMH_VREG("ldo8",   "ldo%s8",  &pmic5_pldo_lv,   "vdd-l1-l8"),
	RPMH_VREG("ldo9",   "ldo%s9",  &pmic5_pldo,      "vdd-l9-l10"),
	RPMH_VREG("ldo10",  "ldo%s10", &pmic5_pldo,      "vdd-l9-l10"),
	RPMH_VREG("ldo11",  "ldo%s11", &pmic5_pldo,      "vdd-l7-l11"),
	{}
};

static const struct rpmh_vreg_init_data pm8550_vreg_data[] = {
	RPMH_VREG("ldo1",   "ldo%s1",  &pmic5_nldo515,    "vdd-l1-l4-l10"),
	RPMH_VREG("ldo2",   "ldo%s2",  &pmic5_pldo,    "vdd-l2-l13-l14"),
	RPMH_VREG("ldo3",   "ldo%s3",  &pmic5_nldo515,    "vdd-l3"),
	RPMH_VREG("ldo4",   "ldo%s4",  &pmic5_nldo515,    "vdd-l1-l4-l10"),
	RPMH_VREG("ldo5",   "ldo%s5",  &pmic5_pldo,    "vdd-l5-l16"),
	RPMH_VREG("ldo6",   "ldo%s6",  &pmic5_pldo, "vdd-l6-l7"),
	RPMH_VREG("ldo7",   "ldo%s7",  &pmic5_pldo, "vdd-l6-l7"),
	RPMH_VREG("ldo8",   "ldo%s8",  &pmic5_pldo, "vdd-l8-l9"),
	RPMH_VREG("ldo9",   "ldo%s9",  &pmic5_pldo,    "vdd-l8-l9"),
	RPMH_VREG("ldo10",  "ldo%s10", &pmic5_nldo515,    "vdd-l1-l4-l10"),
	RPMH_VREG("ldo11",  "ldo%s11", &pmic5_nldo515,    "vdd-l11"),
	RPMH_VREG("ldo12",  "ldo%s12", &pmic5_nldo515,    "vdd-l12"),
	RPMH_VREG("ldo13",  "ldo%s13", &pmic5_pldo,    "vdd-l2-l13-l14"),
	RPMH_VREG("ldo14",  "ldo%s14", &pmic5_pldo,    "vdd-l2-l13-l14"),
	RPMH_VREG("ldo15",  "ldo%s15", &pmic5_nldo515,    "vdd-l15"),
	RPMH_VREG("ldo16",  "ldo%s16", &pmic5_pldo,    "vdd-l5-l16"),
	RPMH_VREG("ldo17",  "ldo%s17", &pmic5_pldo,    "vdd-l17"),
	RPMH_VREG("bob1",   "bob%s1",  &pmic5_bob,     "vdd-bob1"),
	RPMH_VREG("bob2",   "bob%s2",  &pmic5_bob,     "vdd-bob2"),
	{}
};

static const struct rpmh_vreg_init_data pm8550vs_vreg_data[] = {
	RPMH_VREG("smps1",  "smp%s1",  &pmic5_ftsmps525_lv, "vdd-s1"),
	RPMH_VREG("smps2",  "smp%s2",  &pmic5_ftsmps525_lv, "vdd-s2"),
	RPMH_VREG("smps3",  "smp%s3",  &pmic5_ftsmps525_lv, "vdd-s3"),
	RPMH_VREG("smps4",  "smp%s4",  &pmic5_ftsmps525_lv, "vdd-s4"),
	RPMH_VREG("smps5",  "smp%s5",  &pmic5_ftsmps525_lv, "vdd-s5"),
	RPMH_VREG("smps6",  "smp%s6",  &pmic5_ftsmps525_mv, "vdd-s6"),
	RPMH_VREG("ldo1",   "ldo%s1",  &pmic5_nldo515,   "vdd-l1"),
	RPMH_VREG("ldo2",   "ldo%s2",  &pmic5_nldo515,   "vdd-l2"),
	RPMH_VREG("ldo3",   "ldo%s3",  &pmic5_nldo515,   "vdd-l3"),
	{}
};

static const struct rpmh_vreg_init_data pm8550ve_vreg_data[] = {
	RPMH_VREG("smps1", "smp%s1", &pmic5_ftsmps525_lv, "vdd-s1"),
	RPMH_VREG("smps2", "smp%s2", &pmic5_ftsmps525_lv, "vdd-s2"),
	RPMH_VREG("smps3", "smp%s3", &pmic5_ftsmps525_lv, "vdd-s3"),
	RPMH_VREG("smps4", "smp%s4", &pmic5_ftsmps525_mv, "vdd-s4"),
	RPMH_VREG("smps5", "smp%s5", &pmic5_ftsmps525_lv, "vdd-s5"),
	RPMH_VREG("smps6", "smp%s6", &pmic5_ftsmps525_lv, "vdd-s6"),
	RPMH_VREG("smps7", "smp%s7", &pmic5_ftsmps525_lv, "vdd-s7"),
	RPMH_VREG("smps8", "smp%s8", &pmic5_ftsmps525_lv, "vdd-s8"),
	RPMH_VREG("ldo1",  "ldo%s1", &pmic5_nldo515,   "vdd-l1"),
	RPMH_VREG("ldo2",  "ldo%s2", &pmic5_nldo515,   "vdd-l2"),
	RPMH_VREG("ldo3",  "ldo%s3", &pmic5_nldo515,   "vdd-l3"),
	{}
};

static const struct rpmh_vreg_init_data pmc8380_vreg_data[] = {
	RPMH_VREG("smps1", "smp%s1", &pmic5_ftsmps525_lv, "vdd-s1"),
	RPMH_VREG("smps2", "smp%s2", &pmic5_ftsmps525_lv, "vdd-s2"),
	RPMH_VREG("smps3", "smp%s3", &pmic5_ftsmps525_lv, "vdd-s3"),
	RPMH_VREG("smps4", "smp%s4", &pmic5_ftsmps525_mv, "vdd-s4"),
	RPMH_VREG("smps5", "smp%s5", &pmic5_ftsmps525_lv, "vdd-s5"),
	RPMH_VREG("smps6", "smp%s6", &pmic5_ftsmps525_lv, "vdd-s6"),
	RPMH_VREG("smps7", "smp%s7", &pmic5_ftsmps525_lv, "vdd-s7"),
	RPMH_VREG("smps8", "smp%s8", &pmic5_ftsmps525_lv, "vdd-s8"),
	RPMH_VREG("ldo1",  "ldo%s1", &pmic5_nldo515,   "vdd-l1"),
	RPMH_VREG("ldo2",  "ldo%s2", &pmic5_nldo515,   "vdd-l2"),
	RPMH_VREG("ldo3",  "ldo%s3", &pmic5_nldo515,   "vdd-l3"),
	{}
};

static const struct rpmh_vreg_init_data pmm8654au_vreg_data[] = {
	RPMH_VREG("smps1",  "smp%s1",  &pmic5_ftsmps527,  "vdd-s1"),
	RPMH_VREG("smps2",  "smp%s2",  &pmic5_ftsmps527,  "vdd-s2"),
	RPMH_VREG("smps3",  "smp%s3",  &pmic5_ftsmps527,  "vdd-s3"),
	RPMH_VREG("smps4",  "smp%s4",  &pmic5_ftsmps527,  "vdd-s4"),
	RPMH_VREG("smps5",  "smp%s5",  &pmic5_ftsmps527,  "vdd-s5"),
	RPMH_VREG("smps6",  "smp%s6",  &pmic5_ftsmps527,  "vdd-s6"),
	RPMH_VREG("smps7",  "smp%s7",  &pmic5_ftsmps527,  "vdd-s7"),
	RPMH_VREG("smps8",  "smp%s8",  &pmic5_ftsmps527,  "vdd-s8"),
	RPMH_VREG("smps9",  "smp%s9",  &pmic5_ftsmps527,  "vdd-s9"),
	RPMH_VREG("ldo1",   "ldo%s1",  &pmic5_nldo515,    "vdd-s9"),
	RPMH_VREG("ldo2",   "ldo%s2",  &pmic5_nldo515,    "vdd-l2-l3"),
	RPMH_VREG("ldo3",   "ldo%s3",  &pmic5_nldo515,    "vdd-l2-l3"),
	RPMH_VREG("ldo4",   "ldo%s4",  &pmic5_nldo515,    "vdd-s9"),
	RPMH_VREG("ldo5",   "ldo%s5",  &pmic5_nldo515,    "vdd-s9"),
	RPMH_VREG("ldo6",   "ldo%s6",  &pmic5_nldo515,    "vdd-l6-l7"),
	RPMH_VREG("ldo7",   "ldo%s7",  &pmic5_nldo515,    "vdd-l6-l7"),
	RPMH_VREG("ldo8",   "ldo%s8",  &pmic5_pldo515_mv, "vdd-l8-l9"),
	RPMH_VREG("ldo9",   "ldo%s9",  &pmic5_pldo,       "vdd-l8-l9"),
	{}
};

/* probe an individual regulator */
static int rpmh_regulator_probe(struct udevice *dev)
{
	const struct rpmh_vreg_init_data *init_data;
	struct rpmh_vreg *priv;
	struct dm_regulator_uclass_plat *plat_data;

	init_data = (const struct rpmh_vreg_init_data *)dev_get_driver_data(dev);
	priv = dev_get_priv(dev);
	plat_data = dev_get_uclass_plat(dev);

	priv->dev = dev;
	priv->addr = cmd_db_read_addr(dev->name);
	if (!priv->addr) {
		dev_err(dev, "Failed to read RPMh address for %s\n", dev->name);
		return -ENODEV;
	}

	priv->hw_data = init_data->hw_data;
	priv->enabled = -EINVAL;
	priv->uv = -ENOTRECOVERABLE;
	if (ofnode_read_u32(dev_ofnode(dev), "regulator-initial-mode", &priv->mode))
		priv->mode = -EINVAL;

	plat_data->mode = priv->hw_data->pmic_mode_map;
	plat_data->mode_count = priv->hw_data->n_modes;

	return 0;
}

/* for non-drm, xob, or bypass regulators add additional driver definitions */
U_BOOT_DRIVER(rpmh_regulator_drm) = {
	.name = "rpmh_regulator_drm",
	.id = UCLASS_REGULATOR,
	.probe = rpmh_regulator_probe,
	.priv_auto = sizeof(struct rpmh_vreg),
	.ops = &rpmh_regulator_vrm_drms_ops,
};

/* This driver intentionally only supports a subset of the available regulators.
 * This function checks to see if a given regulator node in DT matches a regulator
 * defined in the driver.
 */
static const struct rpmh_vreg_init_data *
vreg_get_init_data(const struct rpmh_vreg_init_data *init_data, ofnode node)
{
	const struct rpmh_vreg_init_data *data;

	for (data = init_data; data->name; data++) {
		if (!strcmp(data->name, ofnode_get_name(node)))
			return data;
	}

	return NULL;
}

static int rpmh_regulators_bind(struct udevice *dev)
{
	const struct rpmh_vreg_init_data *init_data, *data;
	const char *pmic_id;
	char *name;
	struct driver *drv;
	ofnode node;
	int ret;
	size_t namelen;

	init_data = (const struct rpmh_vreg_init_data *)dev_get_driver_data(dev);
	if (!init_data) {
		dev_err(dev, "No RPMh regulator init data\n");
		return -ENODEV;
	}

	pmic_id = ofnode_read_string(dev_ofnode(dev), "qcom,pmic-id");
	if (!pmic_id) {
		dev_err(dev, "No PMIC ID\n");
		return -ENODEV;
	}

	drv = lists_driver_lookup_name("rpmh_regulator_drm");

	ofnode_for_each_subnode(node, dev_ofnode(dev)) {
		data = vreg_get_init_data(init_data, node);
		if (!data)
			continue;

		/* %s is replaced with pmic_id, so subtract 2, then add 1 for the null terminator */
		namelen = strlen(data->resource_name) + strlen(pmic_id) - 1;
		name = devm_kzalloc(dev, namelen, GFP_KERNEL);
		ret = snprintf(name, namelen, data->resource_name, pmic_id);
		if (ret < 0 || ret >= namelen) {
			dev_err(dev, "Failed to create RPMh regulator name\n");
			return -ENOMEM;
		}

		ret = device_bind_with_driver_data(dev, drv, name, (ulong)data,
						   node, NULL);
		if (ret < 0) {
			dev_err(dev, "Failed to bind RPMh regulator %s: %d\n", name, ret);
			return ret;
		}
	}

	return 0;
}

static const struct udevice_id rpmh_regulator_ids[] = {
	{
		.compatible = "qcom,pm8150-rpmh-regulators",
		.data = (ulong)pm8150_vreg_data,
	},
	{
		.compatible = "qcom,pm8150l-rpmh-regulators",
		.data = (ulong)pm8150l_vreg_data,
	},
	{
		.compatible = "qcom,pm8550-rpmh-regulators",
		.data = (ulong)pm8550_vreg_data,
	},
	{
		.compatible = "qcom,pm8550ve-rpmh-regulators",
		.data = (ulong)pm8550ve_vreg_data,
	},
	{
		.compatible = "qcom,pm8550vs-rpmh-regulators",
		.data = (ulong)pm8550vs_vreg_data,
	},
	{
		.compatible = "qcom,pmc8380-rpmh-regulators",
		.data = (ulong)pmc8380_vreg_data,
	},
	{
		.compatible = "qcom,pmm8654au-rpmh-regulators",
		.data = (ulong)pmm8654au_vreg_data,
	},
	{ /* sentinal */ },
};

/* Driver for a 'bank' of regulators. This creates devices for each
 * individual regulator
 */
U_BOOT_DRIVER(rpmh_regulators) = {
	.name = "rpmh_regulators",
	.id = UCLASS_MISC,
	.bind = rpmh_regulators_bind,
	.of_match = rpmh_regulator_ids,
	.ops = &rpmh_regulator_vrm_drms_ops,
};

MODULE_DESCRIPTION("Qualcomm RPMh regulator driver");
MODULE_LICENSE("GPL v2");

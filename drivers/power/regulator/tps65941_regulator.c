// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Keerthy <j-keerthy@ti.com>
 */

#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65941.h>

/* Single Phase Buck IDs */
#define TPS65941_BUCK_ID_1        1
#define TPS65941_BUCK_ID_2        2
#define TPS65941_BUCK_ID_3        3
#define TPS65941_BUCK_ID_4        4
#define TPS65941_BUCK_ID_5        5

/* Multi Phase Buck IDs */
#define TPS65941_BUCK_ID_12      12
#define TPS65941_BUCK_ID_34      34
#define TPS65941_BUCK_ID_123    123
#define TPS65941_BUCK_ID_1234  1234

/* LDO IDs */
#define TPS65941_LDO_ID_1         1
#define TPS65941_LDO_ID_2         2
#define TPS65941_LDO_ID_3         3
#define TPS65941_LDO_ID_4         4

#define TPS65941_BUCK_CONV_OPS_IDX  0
#define TPS65941_LDO_CONV_OPS_IDX   0
#define TPS65224_LDO_CONV_OPS_IDX   1
#define TPS65224_BUCK_CONV_OPS_IDX  1

struct tps65941_reg_conv_ops {
	int volt_mask;
	int (*volt2val)(int idx, int uV);
	int (*val2volt)(int idx, int volt);
	int slew_mask;
	int (*lookup_slew)(int id);
};

static const char tps65941_buck_ctrl[TPS65941_BUCK_NUM] = {0x4, 0x6, 0x8, 0xA,
								0xC};
static const char tps65941_buck_vout[TPS65941_BUCK_NUM] = {0xE, 0x10, 0x12,
								0x14, 0x16};
static const char tps65941_ldo_ctrl[TPS65941_BUCK_NUM] = {0x1D, 0x1E, 0x1F,
								0x20};
static const char tps65941_ldo_vout[TPS65941_BUCK_NUM] = {0x23, 0x24, 0x25,
								0x26};

static inline int tps65941_get_chip_id(struct udevice *dev)
{
	return dev->parent->driver_data;
}

static int tps65941_buck_enable(struct udevice *dev, int op, bool *enable)
{
	int ret;
	unsigned int adr;
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	adr = uc_pdata->ctrl_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= TPS65941_BUCK_MODE_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= TPS65941_BUCK_MODE_MASK;
		else
			ret &= ~TPS65941_BUCK_MODE_MASK;
		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int tps65941_buck_volt2val(__maybe_unused int idx, int uV)
{
	if (uV > TPS65941_BUCK_VOLT_MAX)
		return -EINVAL;
	else if (uV > 1650000)
		return (uV - 1660000) / 20000 + 0xAB;
	else if (uV > 1110000)
		return (uV - 1110000) / 10000 + 0x73;
	else if (uV > 600000)
		return (uV - 600000) / 5000 + 0x0F;
	else if (uV >= 300000)
		return (uV - 300000) / 20000 + 0x00;
	else
		return -EINVAL;
}

static int tps65941_buck_val2volt(__maybe_unused int idx, int val)
{
	if (val > TPS65941_BUCK_VOLT_MAX_HEX)
		return -EINVAL;
	else if (val > 0xAB)
		return 1660000 + (val - 0xAB) * 20000;
	else if (val > 0x73)
		return 1100000 + (val - 0x73) * 10000;
	else if (val > 0xF)
		return 600000 + (val - 0xF) * 5000;
	else if (val >= 0x0)
		return 300000 + val * 5000;
	else
		return -EINVAL;
}

int tps65941_lookup_slew(int id)
{
	switch (id) {
	case 0:
		return 33000;
	case 1:
		return 20000;
	case 2:
		return 10000;
	case 3:
		return 5000;
	case 4:
		return 2500;
	case 5:
		return 1300;
	case 6:
		return 630;
	case 7:
		return 310;
	default:
		return -1;
	}
}

static int tps65224_buck_volt2val(int idx, int uV)
{
	/* This functions maps a value which is in micro Volts to the VSET value.
	 * The mapping is as per the datasheet of TPS65224.
	 */

	if (uV > TPS65224_BUCK_VOLT_MAX)
		return -EINVAL;

	if (idx > 0) {
		/* Buck2, Buck3 and Buck4 of TPS65224 has a different schema in
		 * converting b/w micro_volt and VSET hex values
		 *
		 * VSET value starts from 0x00 for 0.5V, and for every increment
		 * in VSET value the output voltage increases by 25mV. This is upto
		 * 1.15V where VSET is 0x1A.
		 *
		 * For 0x1B the output voltage is 1.2V, and for every increment of
		 * VSET the output voltage increases by 50mV upto the max voltage of
		 * 3.3V
		 *
		 * | Voltage Ranges  | VSET Ranges  | Voltage Step |
		 * +-----------------+--------------+--------------+
		 * | 0.5V to 1.50V   | 0x00 to 0x1A |  25mV        |
		 * | 1.2V to 3.3V    | 0x1B to 0x45 |  50mV        |
		 */
		if (uV >= 1200000)
			return (uV - 1200000) / 50000 + 0x1B;
		else if (uV >= 500000)
			return (uV - 500000) / 25000;
		else
			return -EINVAL;
	}

	/* Buck1 and Buck12(dual phase) has a different mapping b/w output
	 * voltage and VSET value.
	 *
	 * | Voltage Ranges  | VSET Ranges  | Voltage Step |
	 * +-----------------+--------------+--------------+
	 * | 0.5V to 0.58V   | 0xA to 0xE   |  20mV        |
	 * | 0.6V to 1.095V  | 0xF to 0x72  |  5mV         |
	 * | 1.1V to 1.65V   | 0x73 to 0xAA |  10mV        |
	 * | 1.6V to 3.3V    | 0xAB to 0xFD |  20mV        |
	 *
	 */
	if (uV >= 1660000)
		return (uV - 1660000) / 20000 + 0xAB;
	else if (uV >= 1100000)
		return (uV - 1100000) / 10000 + 0x73;
	else if (uV >= 600000)
		return (uV - 600000) / 5000 + 0x0F;
	else if (uV >= 500000)
		return (uV - 500000) / 20000 + 0x0A;
	else
		return -EINVAL;
}

static int tps65224_buck_val2volt(int idx, int val)
{
	/* This function does the opposite to the tps65224_buck_volt2val function
	 * described above.
	 * This maps the VSET value to micro volts. Please refer to the ranges
	 * mentioned the comments of tps65224_buck_volt2val.
	 */

	if (idx > 0) {
		if (val > TPS65224_BUCK234_VOLT_MAX_HEX)
			return -EINVAL;
		else if (val >= 0x1B)
			return 1200000 + (val - 0x1B) * 50000;
		else if (val >= 0x00)
			return 500000 + (val - 0x00) * 25000;
		else
			return -EINVAL;
	}

	if (val > TPS65224_BUCK1_VOLT_MAX_HEX)
		return -EINVAL;
	else if (val >= 0xAB)
		return 1660000 + (val - 0xAB) * 20000;
	else if (val >= 0x73)
		return 1100000 + (val - 0x73) * 10000;
	else if (val >= 0xF)
		return 600000 + (val - 0xF) * 5000;
	else if (val >= 0xA)
		return 500000 + (val - 0xA) * 20000;
	else
		return -EINVAL;
}

int tps65224_lookup_slew(int id)
{
	switch (id) {
	case 0:
		return 10000;
	case 1:
		return 5000;
	case 2:
		return 2500;
	case 3:
		return 1250;
	default:
		return -1;
	}
}

static const struct tps65941_reg_conv_ops buck_conv_ops[] = {
	[TPS65941_BUCK_CONV_OPS_IDX] = {
		.volt_mask = TPS65941_BUCK_VOLT_MASK,
		.volt2val = tps65941_buck_volt2val,
		.val2volt = tps65941_buck_val2volt,
		.slew_mask = TP65941_BUCK_CONF_SLEW_MASK,
		.lookup_slew = tps65941_lookup_slew,
	},
	[TPS65224_BUCK_CONV_OPS_IDX] = {
		.volt_mask = TPS65941_BUCK_VOLT_MASK,
		.volt2val = tps65224_buck_volt2val,
		.val2volt = tps65224_buck_val2volt,
		.slew_mask = TPS65224_BUCK_CONF_SLEW_MASK,
		.lookup_slew = tps65224_lookup_slew,
	},
};

static int tps65941_buck_val(struct udevice *dev, int op, int *uV)
{
	unsigned int hex, adr;
	int ret, delta, uwait, slew, idx;
	struct dm_regulator_uclass_plat *uc_pdata;
	const struct tps65941_reg_conv_ops *conv_ops;
	ulong chip_id;

	idx = dev->driver_data;
	chip_id = tps65941_get_chip_id(dev);
	if (chip_id == TPS65224) {
		/* idx is the buck id number as per devicetree node which will be same
		 * as the regulator name in the datasheet.
		 * The idx for buck1. buck2, buck3, buck4, buck12 will be 1, 2, 3, 4
		 * and 12 respectively.
		 * In the driver the numbering is from 0. Hence the -1.
		 */
		idx = (idx == TPS65941_BUCK_ID_12) ? 0 : (idx - 1);
		conv_ops = &buck_conv_ops[TPS65224_BUCK_CONV_OPS_IDX];
	} else {
		conv_ops = &buck_conv_ops[TPS65941_BUCK_CONV_OPS_IDX];
	}

	uc_pdata = dev_get_uclass_plat(dev);

	if (op == PMIC_OP_GET)
		*uV = 0;

	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	ret &= conv_ops->volt_mask;
	ret = conv_ops->val2volt(idx, ret);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		*uV = ret;
		return 0;
	}

	/*
	 * Compute the delta voltage, find the slew rate and wait
	 * for the appropriate amount of time after voltage switch
	 */
	if (*uV > ret)
		delta = *uV - ret;
	else
		delta = ret - *uV;

	slew = pmic_reg_read(dev->parent, uc_pdata->ctrl_reg + 1);
	if (slew < 0)
		return ret;

	slew &= conv_ops->slew_mask;
	slew = conv_ops->lookup_slew(slew);
	if (slew <= 0)
		return ret;

	uwait = delta / slew;

	hex = conv_ops->volt2val(idx, *uV);
	if (hex < 0)
		return hex;

	ret &= 0x0;
	ret = hex;

	ret = pmic_reg_write(dev->parent, adr, ret);

	udelay(uwait);

	return ret;
}

static int tps65941_ldo_enable(struct udevice *dev, int op, bool *enable)
{
	int ret;
	unsigned int adr;
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	adr = uc_pdata->ctrl_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	if (op == PMIC_OP_GET) {
		ret &= TPS65941_LDO_MODE_MASK;

		if (ret)
			*enable = true;
		else
			*enable = false;

		return 0;
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			ret |= TPS65941_LDO_MODE_MASK;
		else
			ret &= ~TPS65941_LDO_MODE_MASK;
		ret = pmic_reg_write(dev->parent, adr, ret);
		if (ret)
			return ret;
	}

	return 0;
}

static int tps65941_ldo_volt2val(__maybe_unused int idx, int uV)
{
	if (uV > TPS65941_LDO_VOLT_MAX || uV < TPS65941_LDO_VOLT_MIN)
		return -EINVAL;

	return ((uV - 600000) / 50000 + 0x4) << TPS65941_LDO_MODE_MASK;
}

static int tps65941_ldo_val2volt(__maybe_unused int idx, int val)
{
	if (val > TPS65941_LDO_VOLT_MAX_HEX || val < TPS65941_LDO_VOLT_MIN_HEX)
		return -EINVAL;
	else if (val >= TPS65941_LDO_VOLT_MIN_HEX)
		return 600000 + (val - TPS65941_LDO_VOLT_MIN_HEX) * 50000;
	else
		return -EINVAL;
}

static int tps65224_ldo_volt2val(int idx, int uV)
{
	int base = TPS65224_LDO1_VOLT_MIN;
	int max = TPS65224_LDO1_VOLT_MAX;
	int offset = TPS65224_LDO1_VOLT_MIN_HEX;
	int step = TPS65224_LDO_STEP;

	if (idx > 0) {
		base = TPS65224_LDO23_VOLT_MIN;
		max = TPS65224_LDO23_VOLT_MAX;
		offset = TPS65224_LDO23_VOLT_MIN_HEX;
	}

	if (uV > max)
		return -EINVAL;
	else if (uV >= base)
		return (uV - base) / step + offset;
	else
		return -EINVAL;
}

static int tps65224_ldo_val2volt(int idx, int val)
{
	int reg_base = TPS65224_LDO1_VOLT_MIN_HEX;
	int reg_max = TPS65224_LDO1_VOLT_MAX_HEX;
	int base = TPS65224_LDO1_VOLT_MIN;
	int max = TPS65224_LDO1_VOLT_MAX;
	int step = TPS65224_LDO_STEP;
	/* In LDOx_VOUT reg the BIT0 is reserved and the
	 * vout value is stored from BIT1 to BIT7.
	 * Hence the below bit shit is done.
	 */
	int mask = TPS65224_LDO_VOLT_MASK >> 1;

	if (idx > 0) {
		base = TPS65224_LDO23_VOLT_MIN;
		max = TPS65224_LDO23_VOLT_MAX;
		reg_base = TPS65224_LDO23_VOLT_MIN_HEX;
		reg_max = TPS65224_LDO23_VOLT_MAX_HEX;
	}

	/* The VSET register of LDO has its 0th bit as reserved
	 * hence shifting the value to right by 1 bit.
	 */
	val = val >> 1;

	if (val < 0 || val > mask)
		return -EINVAL;

	if (val <= reg_base)
		return base;

	if (val >= reg_max)
		return max;

	return base + (step * (val - reg_base));
}

static const struct tps65941_reg_conv_ops ldo_conv_ops[] = {
	[TPS65941_LDO_CONV_OPS_IDX] = {
		.volt_mask = TPS65941_LDO_VOLT_MASK,
		.volt2val = tps65941_ldo_volt2val,
		.val2volt = tps65941_ldo_val2volt,
	},
	[TPS65224_LDO_CONV_OPS_IDX] = {
		.volt_mask = TPS65224_LDO_VOLT_MASK,
		.volt2val = tps65224_ldo_volt2val,
		.val2volt = tps65224_ldo_val2volt,
	},
};

static int tps65941_ldo_val(struct udevice *dev, int op, int *uV)
{
	unsigned int hex, adr;
	int ret, ret_volt, idx, ldo_bypass;
	struct dm_regulator_uclass_plat *uc_pdata;
	const struct tps65941_reg_conv_ops *conv_ops;
	ulong chip_id;

	chip_id = tps65941_get_chip_id(dev);
	idx = dev->driver_data;
	if (chip_id == TPS65224) {
		/* idx is the ldo id number as per devicetree node which will be same
		 * as the regulator name in the datasheet.
		 * The idx for ldo1, ldo2, ldo3 will be 1, 2 & 3 respectively.
		 * In the driver the numbering is from 0. Hence the -1.
		 */
		idx = idx - 1;
		conv_ops = &ldo_conv_ops[TPS65224_LDO_CONV_OPS_IDX];
	} else {
		conv_ops = &ldo_conv_ops[TPS65941_LDO_CONV_OPS_IDX];
	}

	uc_pdata = dev_get_uclass_plat(dev);

	if (op == PMIC_OP_GET)
		*uV = 0;

	adr = uc_pdata->volt_reg;

	ret = pmic_reg_read(dev->parent, adr);
	if (ret < 0)
		return ret;

	ldo_bypass = ret & TPS65941_LDO_BYPASS_EN;
	ret &= conv_ops->volt_mask;
	ret = ret >> TPS65941_LDO_MODE_MASK;
	ret_volt = conv_ops->val2volt(idx, ret);
	if (ret_volt < 0)
		return ret_volt;

	if (op == PMIC_OP_GET) {
		*uV = ret_volt;
		return 0;
	}

	/* TPS65224 LDO1 in BYPASS mode only supports 2.2V min to 3.6V max */
	if (chip_id == TPS65224 && idx == 0 && (ret & BIT(TPS65224_LDO_BYP_CONFIG)) &&
	    *uV < TPS65224_LDO1_VOLT_BYP_MIN)
		return -EINVAL;

	/* TPS65224 LDO2 & LDO3 in BYPASS mode supports 1.5V min to 5.5V max */
	if (chip_id == TPS65224 && idx > 0 && (ret & BIT(TPS65224_LDO_BYP_CONFIG)) &&
	    *uV < TPS65224_LDO23_VOLT_BYP_MIN)
		return -EINVAL;

	hex = conv_ops->volt2val(idx, *uV);
	if (hex < 0)
		return hex;

	if (chip_id == TPS65224) {
		hex = hex << TPS65941_LDO_MODE_MASK;
		ret &= ~TPS65224_LDO_VOLT_MASK;
		ret |= hex;
	} else {
		ret = hex | ldo_bypass;
	}

	ret = pmic_reg_write(dev->parent, adr, ret);

	return ret;
}

static int tps65941_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;
	ulong chip_id;

	chip_id = tps65941_get_chip_id(dev);

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_LDO;

	idx = dev->driver_data;
	switch (idx) {
	case TPS65941_LDO_ID_1:
	case TPS65941_LDO_ID_2:
	case TPS65941_LDO_ID_3:
		debug("Single phase regulator\n");
		break;
	case TPS65941_LDO_ID_4:
		if (chip_id != TPS65224) {
			debug("Single phase regulator\n");
			break;
		}
	default:
		pr_err("Wrong ID for regulator\n");
		return -EINVAL;
	}

	uc_pdata->ctrl_reg = tps65941_ldo_ctrl[idx - 1];
	uc_pdata->volt_reg = tps65941_ldo_vout[idx - 1];

	return 0;
}

static int tps65941_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	int idx;
	ulong chip_id;

	chip_id = tps65941_get_chip_id(dev);

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_BUCK;

	idx = dev->driver_data;
	switch (idx) {
	case TPS65941_BUCK_ID_1:
	case TPS65941_BUCK_ID_2:
	case TPS65941_BUCK_ID_3:
	case TPS65941_BUCK_ID_4:
		debug("Single phase regulator\n");
		break;
	case TPS65941_BUCK_ID_5:
		if (chip_id != TPS65224) {
			debug("Single phase regulator\n");
		} else {
			pr_err("Wrong ID for regulator\n");
			return -EINVAL;
		}
		break;
	case TPS65941_BUCK_ID_12:
		idx = 1;
		break;
	case TPS65941_BUCK_ID_123:
	case TPS65941_BUCK_ID_1234:
		if (chip_id != TPS65224) {
			idx = 1;
		} else {
			pr_err("Wrong ID for regulator\n");
			return -EINVAL;
		}
		break;
	case TPS65941_BUCK_ID_34:
		if (chip_id != TPS65224) {
			idx = 3;
		} else {
			pr_err("Wrong ID for regulator\n");
			return -EINVAL;
		}
		break;
	default:
		pr_err("Wrong ID for regulator\n");
		return -EINVAL;
	}

	uc_pdata->ctrl_reg = tps65941_buck_ctrl[idx - 1];
	uc_pdata->volt_reg = tps65941_buck_vout[idx - 1];

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps65941_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return tps65941_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = tps65941_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	return tps65941_ldo_enable(dev, PMIC_OP_SET, &enable);
}

static int buck_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = tps65941_buck_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int buck_set_value(struct udevice *dev, int uV)
{
	return tps65941_buck_val(dev, PMIC_OP_SET, &uV);
}

static int buck_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = tps65941_buck_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return tps65941_buck_enable(dev, PMIC_OP_SET, &enable);
}

static const struct dm_regulator_ops tps65941_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
};

U_BOOT_DRIVER(tps65941_ldo) = {
	.name = TPS65941_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65941_ldo_ops,
	.probe = tps65941_ldo_probe,
};

static const struct dm_regulator_ops tps65941_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
};

U_BOOT_DRIVER(tps65941_buck) = {
	.name = TPS65941_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65941_buck_ops,
	.probe = tps65941_buck_probe,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/max8907.h>

static const char max8907_regmap[] = {
	0x00, MAX8907_REG_SDCTL1, MAX8907_REG_SDCTL2, MAX8907_REG_SDCTL3,
	 MAX8907_REG_LDOCTL1,  MAX8907_REG_LDOCTL2,  MAX8907_REG_LDOCTL3,
	 MAX8907_REG_LDOCTL4,  MAX8907_REG_LDOCTL5,  MAX8907_REG_LDOCTL6,
	 MAX8907_REG_LDOCTL7,  MAX8907_REG_LDOCTL8,  MAX8907_REG_LDOCTL9,
	MAX8907_REG_LDOCTL10, MAX8907_REG_LDOCTL11, MAX8907_REG_LDOCTL12,
	MAX8907_REG_LDOCTL13, MAX8907_REG_LDOCTL14, MAX8907_REG_LDOCTL15,
	MAX8907_REG_LDOCTL16, MAX8907_REG_LDOCTL17, MAX8907_REG_LDOCTL18,
	MAX8907_REG_LDOCTL19, MAX8907_REG_LDOCTL20
};

static int max8907_enable(struct udevice *dev, int op, bool *enable)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	int val, ret = 0;

	if (op == PMIC_OP_GET) {
		val = pmic_reg_read(dev->parent, uc_pdata->ctrl_reg);
		if (val < 0)
			return val;

		if (val & MAX8907_MASK_LDO_EN)
			*enable = true;
		else
			*enable = false;
	} else if (op == PMIC_OP_SET) {
		if (*enable) {
			ret = pmic_clrsetbits(dev->parent,
					      uc_pdata->ctrl_reg,
					      MAX8907_MASK_LDO_EN |
					      MAX8907_MASK_LDO_SEQ,
					      MAX8907_MASK_LDO_EN |
					      MAX8907_MASK_LDO_SEQ);
		} else {
			ret = pmic_clrsetbits(dev->parent,
					      uc_pdata->ctrl_reg,
					      MAX8907_MASK_LDO_EN |
					      MAX8907_MASK_LDO_SEQ,
					      MAX8907_MASK_LDO_SEQ);
		}
	}

	return ret;
}

static int max8907_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = max8907_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int max8907_set_enable(struct udevice *dev, bool enable)
{
	return max8907_enable(dev, PMIC_OP_SET, &enable);
}

/**
 * max8907_volt2hex() - convert voltage in uV into
 *			applicable to register hex value
 *
 * @idx:	regulator index
 * @uV:		voltage in uV
 *
 * Return: voltage in hex on success, -ve on failure
 */
static int max8907_volt2hex(int idx, int uV)
{
	switch (idx) {
	case 1: /* SD1 */
		if (uV > SD1_VOLT_MAX || uV < SD1_VOLT_MIN)
			break;

		return (uV - SD1_VOLT_MIN) / SD1_VOLT_STEP;

	case 2: /* SD2 */
		if (uV > SD2_VOLT_MAX || uV < SD2_VOLT_MIN)
			break;

		return (uV - SD2_VOLT_MIN) / SD2_VOLT_STEP;

	case 3: /* SD3 */
		if (uV > SD2_VOLT_MAX || uV < SD2_VOLT_MIN)
			break;

		return (uV - SD2_VOLT_MIN) / SD2_VOLT_STEP;

	case 5: /* LDO2 */
	case 6: /* LDO3 */
	case 20: /* LDO17 */
	case 21: /* LDO18 */
		if (uV > LDO_650_VOLT_MAX || uV < LDO_650_VOLT_MIN)
			break;

		return (uV - LDO_650_VOLT_MIN) / LDO_650_VOLT_STEP;

	default: /* LDO1, 4..16, 19..20 */
		if (uV > LDO_750_VOLT_MAX || uV < LDO_750_VOLT_MIN)
			break;

		return (uV - LDO_750_VOLT_MIN) / LDO_750_VOLT_STEP;
	};

	return -EINVAL;
}

/**
 * max8907_hex2volt() - convert register hex value into
 *			actual voltage in uV
 *
 * @idx:	regulator index
 * @hex:	hex value of register
 *
 * Return: voltage in uV on success, -ve on failure
 */
static int max8907_hex2volt(int idx, int hex)
{
	switch (idx) {
	case 1:
		return hex * SD1_VOLT_STEP + SD1_VOLT_MIN;

	case 2:
		return hex * SD2_VOLT_STEP + SD2_VOLT_MIN;

	case 3:
		return hex * SD3_VOLT_STEP + SD3_VOLT_MIN;

	case 5: /* LDO2 */
	case 6: /* LDO3 */
	case 20: /* LDO17 */
	case 21: /* LDO18 */
		return hex * LDO_650_VOLT_STEP + LDO_650_VOLT_MIN;

	default: /* LDO1, 4..16, 19..20 */
		return hex * LDO_750_VOLT_STEP + LDO_750_VOLT_MIN;
	};

	return -EINVAL;
}

static int max8907_val(struct udevice *dev, int op, int *uV)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	int idx = dev->driver_data;
	int hex, ret;

	if (op == PMIC_OP_GET) {
		hex = pmic_reg_read(dev->parent, uc_pdata->volt_reg);
		if (hex < 0)
			return hex;

		*uV = 0;

		ret = max8907_hex2volt(idx, hex);
		if (ret < 0)
			return ret;
		*uV = ret;

		return 0;
	}

	hex = max8907_volt2hex(idx, *uV);
	if (hex < 0)
		return hex;

	return pmic_reg_write(dev->parent, uc_pdata->volt_reg, hex);
}

static int max8907_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = max8907_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int max8907_set_value(struct udevice *dev, int uV)
{
	return max8907_val(dev, PMIC_OP_SET, &uV);
}

static const struct dm_regulator_ops max8907_regulator_ops = {
	.get_value  = max8907_get_value,
	.set_value  = max8907_set_value,
	.get_enable = max8907_get_enable,
	.set_enable = max8907_set_enable,
};

static int max8907_sd_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	int idx = dev->driver_data;

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->ctrl_reg = max8907_regmap[idx];
	uc_pdata->volt_reg = uc_pdata->ctrl_reg + MAX8907_VOUT;

	return 0;
}

U_BOOT_DRIVER(max8907_sd) = {
	.name = MAX8907_SD_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &max8907_regulator_ops,
	.probe = max8907_sd_probe,
};

static int max8907_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata =
					dev_get_uclass_plat(dev);
	/* LDO regulator id is shifted by number for SD regulators */
	int idx = dev->driver_data + 3;

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->ctrl_reg = max8907_regmap[idx];
	uc_pdata->volt_reg = uc_pdata->ctrl_reg + MAX8907_VOUT;

	return 0;
}

U_BOOT_DRIVER(max8907_ldo) = {
	.name = MAX8907_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &max8907_regulator_ops,
	.probe = max8907_ldo_probe,
};

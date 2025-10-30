// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2018 Samsung Electronics
 *  Jaehoon Chung <jh80.chung@samsung.com>
 */

#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/s2mps11.h>

#define regulator_desc_s2mps11_buck(num, mask, min, step, max_hex)			\
	[num] = {									\
		.mode_reg	= S2MPS11_REG_B##num##CTRL1,				\
		.mode_mask	= S2MPS11_BUCK_MODE_MASK << S2MPS11_BUCK_MODE_SHIFT,	\
		.volt_reg	= S2MPS11_REG_B##num##CTRL2,				\
		.volt_mask	= mask,							\
		.volt_min	= min,							\
		.volt_step	= step,							\
		.volt_max_hex	= max_hex,						\
	}

#define regulator_desc_s2mps11_buck1_2_3_4_6(num)			\
	regulator_desc_s2mps11_buck(num, S2MPS11_BUCK_VOLT_MASK,	\
					 S2MPS11_BUCK_UV_MIN,		\
					 S2MPS11_BUCK_LSTEP,		\
					 S2MPS11_BUCK_VOLT_MAX_HEX)

#define regulator_desc_s2mps11_buck5					\
	regulator_desc_s2mps11_buck(5, S2MPS11_BUCK_VOLT_MASK,		\
				       S2MPS11_BUCK_UV_MIN,		\
				       S2MPS11_BUCK_LSTEP,		\
				       S2MPS11_BUCK5_VOLT_MAX_HEX)

#define regulator_desc_s2mps11_buck7_8_10(num)				\
	regulator_desc_s2mps11_buck(num, S2MPS11_BUCK_VOLT_MASK,	\
					 S2MPS11_BUCK_UV_HMIN,		\
					 S2MPS11_BUCK_HSTEP,		\
					 S2MPS11_BUCK7_8_10_VOLT_MAX_HEX)

#define regulator_desc_s2mps11_buck9					\
	regulator_desc_s2mps11_buck(9, S2MPS11_BUCK9_VOLT_MASK,		\
				       S2MPS11_BUCK_UV_MIN,		\
				       S2MPS11_BUCK9_STEP,		\
				       S2MPS11_BUCK9_VOLT_MAX_HEX)

static const struct sec_regulator_desc s2mps11_buck_desc[] = {
	regulator_desc_s2mps11_buck1_2_3_4_6(1),
	regulator_desc_s2mps11_buck1_2_3_4_6(2),
	regulator_desc_s2mps11_buck1_2_3_4_6(3),
	regulator_desc_s2mps11_buck1_2_3_4_6(4),
	regulator_desc_s2mps11_buck5,
	regulator_desc_s2mps11_buck1_2_3_4_6(6),
	regulator_desc_s2mps11_buck7_8_10(7),
	regulator_desc_s2mps11_buck7_8_10(8),
	regulator_desc_s2mps11_buck9,
	regulator_desc_s2mps11_buck7_8_10(10),
};

#define regulator_desc_s2mps11_ldo(num, step)						\
	[num] = {									\
		.mode_reg	= S2MPS11_REG_L##num##CTRL,				\
		.mode_mask	= S2MPS11_LDO_MODE_MASK << S2MPS11_LDO_MODE_SHIFT,	\
		.volt_reg	= S2MPS11_REG_L##num##CTRL,				\
		.volt_mask	= S2MPS11_LDO_VOLT_MASK,				\
		.volt_min	= S2MPS11_LDO_UV_MIN,					\
		.volt_step	= step,							\
		.volt_max_hex	= S2MPS11_LDO_VOLT_MAX_HEX				\
	}

#define regulator_desc_s2mps11_ldo_type1(num)	\
	regulator_desc_s2mps11_ldo(num, S2MPS11_LDO_STEP)

#define regulator_desc_s2mps11_ldo_type2(num)	\
	regulator_desc_s2mps11_ldo(num, S2MPS11_LDO_STEP * 2)

static const struct sec_regulator_desc s2mps11_ldo_desc[] = {
	regulator_desc_s2mps11_ldo_type1(1),
	regulator_desc_s2mps11_ldo_type2(2),
	regulator_desc_s2mps11_ldo_type2(3),
	regulator_desc_s2mps11_ldo_type2(4),
	regulator_desc_s2mps11_ldo_type2(5),
	regulator_desc_s2mps11_ldo_type1(6),
	regulator_desc_s2mps11_ldo_type2(7),
	regulator_desc_s2mps11_ldo_type2(8),
	regulator_desc_s2mps11_ldo_type2(9),
	regulator_desc_s2mps11_ldo_type2(10),
	regulator_desc_s2mps11_ldo_type1(11),
	regulator_desc_s2mps11_ldo_type2(12),
	regulator_desc_s2mps11_ldo_type2(13),
	regulator_desc_s2mps11_ldo_type2(14),
	regulator_desc_s2mps11_ldo_type2(15),
	regulator_desc_s2mps11_ldo_type2(16),
	regulator_desc_s2mps11_ldo_type2(17),
	regulator_desc_s2mps11_ldo_type2(18),
	regulator_desc_s2mps11_ldo_type2(19),
	regulator_desc_s2mps11_ldo_type2(20),
	regulator_desc_s2mps11_ldo_type2(21),
	regulator_desc_s2mps11_ldo_type1(22),
	regulator_desc_s2mps11_ldo_type1(23),
	regulator_desc_s2mps11_ldo_type2(24),
	regulator_desc_s2mps11_ldo_type2(25),
	regulator_desc_s2mps11_ldo_type2(26),
	regulator_desc_s2mps11_ldo_type1(27),
	regulator_desc_s2mps11_ldo_type2(28),
	regulator_desc_s2mps11_ldo_type2(29),
	regulator_desc_s2mps11_ldo_type2(30),
	regulator_desc_s2mps11_ldo_type2(31),
	regulator_desc_s2mps11_ldo_type2(32),
	regulator_desc_s2mps11_ldo_type2(33),
	regulator_desc_s2mps11_ldo_type2(34),
	regulator_desc_s2mps11_ldo_type1(35),
	regulator_desc_s2mps11_ldo_type2(36),
	regulator_desc_s2mps11_ldo_type2(37),
	regulator_desc_s2mps11_ldo_type2(38),
};

#define regulator_desc_s2mpu05_buck(num, which)						\
	[num] = {									\
		.mode_reg	= S2MPU05_REG_B##num##CTRL1,				\
		.mode_mask	= S2MPS11_BUCK_MODE_MASK << S2MPS11_BUCK_MODE_SHIFT,	\
		.volt_reg	= S2MPU05_REG_B##num##CTRL2,				\
		.volt_mask	= S2MPS11_BUCK_VOLT_MASK,				\
		.volt_min	= S2MPU05_BUCK_MIN##which,				\
		.volt_step	= S2MPU05_BUCK_STEP##which,				\
		.volt_max_hex	= S2MPS11_BUCK_VOLT_MASK,				\
	}

#define regulator_desc_s2mpu05_buck1_2_3(num)	\
	regulator_desc_s2mpu05_buck(num, 1)

#define regulator_desc_s2mpu05_buck4_5(num)	\
	regulator_desc_s2mpu05_buck(num, 2)

static const struct sec_regulator_desc s2mpu05_buck_desc[] = {
	regulator_desc_s2mpu05_buck1_2_3(1),
	regulator_desc_s2mpu05_buck1_2_3(2),
	regulator_desc_s2mpu05_buck1_2_3(3),
	regulator_desc_s2mpu05_buck4_5(4),
	regulator_desc_s2mpu05_buck4_5(5),
};

#define regulator_desc_s2mpu05_ldo(num, reg, min, step)					\
	[num] = {									\
		.mode_reg	= S2MPU05_REG_L##num##reg,				\
		.mode_mask	= S2MPS11_LDO_MODE_MASK << S2MPS11_LDO_MODE_SHIFT,	\
		.volt_reg	= S2MPU05_REG_L##num##reg,				\
		.volt_mask	= S2MPS11_LDO_VOLT_MASK,				\
		.volt_min	= min,							\
		.volt_step	= step,							\
		.volt_max_hex	= S2MPS11_LDO_VOLT_MAX_HEX,				\
	}

#define regulator_desc_s2mpu05_ldo_type1(num)				\
	regulator_desc_s2mpu05_ldo(num, CTRL, S2MPU05_LDO_MIN1,		\
					      S2MPU05_LDO_STEP1)

#define regulator_desc_s2mpu05_ldo_type2(num)				\
	regulator_desc_s2mpu05_ldo(num, CTRL, S2MPU05_LDO_MIN1,		\
					      S2MPU05_LDO_STEP2)

#define regulator_desc_s2mpu05_ldo_type3(num)				\
	regulator_desc_s2mpu05_ldo(num, CTRL, S2MPU05_LDO_MIN2,		\
					      S2MPU05_LDO_STEP2)

#define regulator_desc_s2mpu05_ldo_type4(num)				\
	regulator_desc_s2mpu05_ldo(num, CTRL, S2MPU05_LDO_MIN3,		\
					      S2MPU05_LDO_STEP2)

#define regulator_desc_s2mpu05_ldo_type5(num)				\
	regulator_desc_s2mpu05_ldo(num, CTRL1, S2MPU05_LDO_MIN3,	\
					       S2MPU05_LDO_STEP2)

static const struct sec_regulator_desc s2mpu05_ldo_desc[] = {
	regulator_desc_s2mpu05_ldo_type4(1),
	regulator_desc_s2mpu05_ldo_type3(2),
	regulator_desc_s2mpu05_ldo_type2(3),
	regulator_desc_s2mpu05_ldo_type1(4),
	regulator_desc_s2mpu05_ldo_type1(5),
	regulator_desc_s2mpu05_ldo_type1(6),
	regulator_desc_s2mpu05_ldo_type2(7),
	regulator_desc_s2mpu05_ldo_type3(8),
	regulator_desc_s2mpu05_ldo_type5(9),
	regulator_desc_s2mpu05_ldo_type4(10),
	/* LDOs 11-24 are used for CP. They aren't documented. */
	regulator_desc_s2mpu05_ldo_type2(25),
	regulator_desc_s2mpu05_ldo_type3(26),
	regulator_desc_s2mpu05_ldo_type2(27),
	regulator_desc_s2mpu05_ldo_type3(28),
	regulator_desc_s2mpu05_ldo_type3(29),
	regulator_desc_s2mpu05_ldo_type2(30),
	regulator_desc_s2mpu05_ldo_type3(31),
	regulator_desc_s2mpu05_ldo_type3(32),
	regulator_desc_s2mpu05_ldo_type3(33),
	regulator_desc_s2mpu05_ldo_type3(34),
	regulator_desc_s2mpu05_ldo_type3(35),
};

#define MODE(_id, _val, _name) { \
	.id = _id, \
	.register_value = _val, \
	.name = _name, \
}

/* BUCK : 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 */
static struct dm_regulator_mode s2mps11_buck_modes[] = {
	MODE(OP_OFF, S2MPS11_BUCK_MODE_OFF, "OFF"),
	MODE(OP_STANDBY, S2MPS11_BUCK_MODE_STANDBY, "ON/OFF"),
	MODE(OP_ON, S2MPS11_BUCK_MODE_STANDBY, "ON"),
};

static struct dm_regulator_mode s2mps11_ldo_modes[] = {
	MODE(OP_OFF, S2MPS11_LDO_MODE_OFF, "OFF"),
	MODE(OP_STANDBY, S2MPS11_LDO_MODE_STANDBY, "ON/OFF"),
	MODE(OP_STANDBY_LPM, S2MPS11_LDO_MODE_STANDBY_LPM, "ON/LPM"),
	MODE(OP_ON, S2MPS11_LDO_MODE_ON, "ON"),
};

static struct dm_regulator_mode s2mpu05_regulator_modes[] = {
	MODE(OP_OFF, S2MPS11_LDO_MODE_OFF, "OFF"),
	MODE(OP_ON, S2MPS11_LDO_MODE_ON, "ON"),
};

static const ulong s2mps11_get_variant(struct udevice *dev)
{
	struct udevice *parent = dev_get_parent(dev);

	if (!parent) {
		pr_err("Parent is non-existent, this shouldn't happen!\n");
		return VARIANT_NONE;
	}

	return dev_get_driver_data(parent);
}

static int s2mps11_buck_val(struct udevice *dev, int op, int *uV)
{
	const struct sec_regulator_desc *buck_desc;
	int num_bucks, hex, buck, ret;
	u32 addr;
	u8 val;

	switch (s2mps11_get_variant(dev)) {
	case VARIANT_S2MPS11:
		buck_desc = s2mps11_buck_desc;
		num_bucks = ARRAY_SIZE(s2mps11_buck_desc);
		break;
	case VARIANT_S2MPU05:
		buck_desc = s2mpu05_buck_desc;
		num_bucks = ARRAY_SIZE(s2mpu05_buck_desc);
		break;
	default:
		pr_err("Unknown device type\n");
		return -EINVAL;
	}

	buck = dev->driver_data;
	if (buck < 1 || buck > num_bucks) {
		pr_err("Wrong buck number: %d\n", buck);
		return -EINVAL;
	}

	if (op == PMIC_OP_GET)
		*uV = 0;

	addr = buck_desc[buck].volt_reg;

	ret = pmic_read(dev->parent, addr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= buck_desc[buck].volt_mask;
		*uV = val * buck_desc[buck].volt_step + buck_desc[buck].volt_min;
		return 0;
	}

	hex = (*uV - buck_desc[buck].volt_min) / buck_desc[buck].volt_step;
	if (hex > buck_desc[buck].volt_max_hex) {
		pr_err("Value: %d uV is wrong for LDO%d\n", *uV, buck);
		return -EINVAL;
	}

	val &= ~buck_desc[buck].volt_mask;
	val |= hex;
	ret = pmic_write(dev->parent, addr, &val, 1);

	return ret;
}

static int s2mps11_buck_mode(struct udevice *dev, int op, int *opmode)
{
	struct dm_regulator_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	const struct sec_regulator_desc *buck_desc;
	unsigned int addr, mode;
	unsigned char val;
	int num_bucks, buck, ret, i;

	switch (s2mps11_get_variant(dev)) {
	case VARIANT_S2MPS11:
		buck_desc = s2mps11_buck_desc;
		num_bucks = ARRAY_SIZE(s2mps11_buck_desc);
		break;
	case VARIANT_S2MPU05:
		buck_desc = s2mpu05_buck_desc;
		num_bucks = ARRAY_SIZE(s2mpu05_buck_desc);
		break;
	default:
		pr_err("Unknown device type\n");
		return -EINVAL;
	}

	buck = dev->driver_data;
	if (buck < 1 || buck > num_bucks) {
		pr_err("Wrong buck number: %d\n", buck);
		return -EINVAL;
	}

	addr = buck_desc[buck].mode_reg;

	ret = pmic_read(dev->parent, addr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= buck_desc[buck].mode_mask;
		for (i = 0; i < uc_pdata->mode_count; i++) {
			if (uc_pdata->mode[i].register_value != val)
				continue;

			*opmode = uc_pdata->mode[i].id;
			return 0;
		}

		return -EINVAL;
	}

	for (i = 0; i < uc_pdata->mode_count; i++) {
		if (uc_pdata->mode[i].id != *opmode)
			continue;

		mode = uc_pdata->mode[i].register_value;
		val &= ~buck_desc[buck].mode_mask;
		val |= mode;
		return pmic_write(dev->parent, addr, &val, 1);
	}

	pr_err("Wrong mode: %d for buck: %d\n", *opmode, buck);
	return -EINVAL;
}

static int s2mps11_buck_enable(struct udevice *dev, int op, bool *enable)
{
	int ret, on_off;

	if (op == PMIC_OP_GET) {
		ret = s2mps11_buck_mode(dev, op, &on_off);
		if (ret)
			return ret;
		switch (on_off) {
		case OP_OFF:
			*enable = false;
			break;
		case OP_ON:
			*enable = true;
			break;
		default:
			return -EINVAL;
		}
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			on_off = OP_ON;
		else
			on_off = OP_OFF;

		ret = s2mps11_buck_mode(dev, op, &on_off);
		if (ret)
			return ret;
	}

	return 0;
}

static int buck_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = s2mps11_buck_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;
	return uV;
}

static int buck_set_value(struct udevice *dev, int uV)
{
	return s2mps11_buck_val(dev, PMIC_OP_SET, &uV);
}

static int buck_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = s2mps11_buck_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;
	return enable;
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	return s2mps11_buck_enable(dev, PMIC_OP_SET, &enable);
}

static int buck_get_mode(struct udevice *dev)
{
	int mode;
	int ret;

	ret = s2mps11_buck_mode(dev, PMIC_OP_GET, &mode);
	if (ret)
		return ret;

	return mode;
}

static int buck_set_mode(struct udevice *dev, int mode)
{
	return s2mps11_buck_mode(dev, PMIC_OP_SET, &mode);
}

static int s2mps11_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_BUCK;

	switch (s2mps11_get_variant(dev)) {
	case VARIANT_S2MPS11:
		uc_pdata->mode = s2mps11_buck_modes;
		uc_pdata->mode_count = ARRAY_SIZE(s2mps11_buck_modes);
		break;
	case VARIANT_S2MPU05:
		uc_pdata->mode = s2mpu05_regulator_modes;
		uc_pdata->mode_count = ARRAY_SIZE(s2mpu05_regulator_modes);
		break;
	default:
		pr_err("Unknown device type\n");
		return -EINVAL;
	}

	return 0;
}

static const struct dm_regulator_ops s2mps11_buck_ops = {
	.get_value	= buck_get_value,
	.set_value	= buck_set_value,
	.get_enable	= buck_get_enable,
	.set_enable	= buck_set_enable,
	.get_mode	= buck_get_mode,
	.set_mode	= buck_set_mode,
};

U_BOOT_DRIVER(s2mps11_buck) = {
	.name = S2MPS11_BUCK_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &s2mps11_buck_ops,
	.probe = s2mps11_buck_probe,
};

static int s2mps11_ldo_val(struct udevice *dev, int op, int *uV)
{
	const struct sec_regulator_desc *ldo_desc;
	unsigned int addr;
	unsigned char val;
	int num_ldos, hex, ldo, ret;

	switch (s2mps11_get_variant(dev)) {
	case VARIANT_S2MPS11:
		ldo_desc = s2mps11_ldo_desc;
		num_ldos = ARRAY_SIZE(s2mps11_ldo_desc);
		break;
	case VARIANT_S2MPU05:
		ldo_desc = s2mpu05_ldo_desc;
		num_ldos = ARRAY_SIZE(s2mpu05_ldo_desc);
		break;
	default:
		pr_err("Unknown device type\n");
		return -EINVAL;
	}

	ldo = dev->driver_data;
	if (ldo < 1 || ldo > num_ldos) {
		pr_err("Wrong ldo number: %d\n", ldo);
		return -EINVAL;
	}

	addr = ldo_desc[ldo].volt_reg;

	if (op == PMIC_OP_GET)
		*uV = 0;

	ret = pmic_read(dev->parent, addr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= ldo_desc[ldo].volt_mask;
		*uV = val * ldo_desc[ldo].volt_step + ldo_desc[ldo].volt_min;
		return 0;
	}

	hex = (*uV - ldo_desc[ldo].volt_min) / ldo_desc[ldo].volt_step;
	if (hex > ldo_desc[ldo].volt_max_hex) {
		pr_err("Value: %d uV is wrong for LDO%d\n", *uV, ldo);
		return -EINVAL;
	}

	val &= ~ldo_desc[ldo].volt_mask;
	val |= hex;
	ret = pmic_write(dev->parent, addr, &val, 1);

	return ret;
}

static int s2mps11_ldo_mode(struct udevice *dev, int op, int *opmode)
{
	struct dm_regulator_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	const struct sec_regulator_desc *ldo_desc;
	unsigned int addr, mode;
	unsigned char val;
	int num_ldos, ldo, ret, i;

	switch (s2mps11_get_variant(dev)) {
	case VARIANT_S2MPS11:
		ldo_desc = s2mps11_ldo_desc;
		num_ldos = ARRAY_SIZE(s2mps11_ldo_desc);
		break;
	case VARIANT_S2MPU05:
		ldo_desc = s2mpu05_ldo_desc;
		num_ldos = ARRAY_SIZE(s2mpu05_ldo_desc);
		break;
	default:
		pr_err("Unknown device type\n");
		return -EINVAL;
	}

	ldo = dev->driver_data;
	if (ldo < 1 || ldo > num_ldos) {
		pr_err("Wrong ldo number: %d\n", ldo);
		return -EINVAL;
	}

	addr = ldo_desc[ldo].mode_reg;

	ret = pmic_read(dev->parent, addr, &val, 1);
	if (ret)
		return ret;

	if (op == PMIC_OP_GET) {
		val &= ldo_desc[ldo].mode_mask;

		for (i = 0; i < uc_pdata->mode_count; i++) {
			if (uc_pdata->mode[i].register_value != val)
				continue;

			*opmode = uc_pdata->mode[i].id;
			return 0;
		}

		return -EINVAL;
	}

	for (i = 0; i < uc_pdata->mode_count; i++) {
		if (uc_pdata->mode[i].id != *opmode)
			continue;

		mode = uc_pdata->mode[i].register_value;
		val &= ~ldo_desc[ldo].mode_mask;
		val |= mode;
		return pmic_write(dev->parent, addr, &val, 1);
	}

	pr_err("Wrong mode: %d for ldo: %d\n", *opmode, ldo);
	return -EINVAL;
}

static int s2mps11_ldo_enable(struct udevice *dev, int op, bool *enable)
{
	int ret, on_off;

	if (op == PMIC_OP_GET) {
		ret = s2mps11_ldo_mode(dev, op, &on_off);
		if (ret)
			return ret;
		switch (on_off) {
		case OP_OFF:
			*enable = false;
			break;
		case OP_ON:
			*enable = true;
			break;
		default:
			return -EINVAL;
		}
	} else if (op == PMIC_OP_SET) {
		if (*enable)
			on_off = OP_ON;
		else
			on_off = OP_OFF;

		ret = s2mps11_ldo_mode(dev, op, &on_off);
		if (ret)
			return ret;
	}

	return 0;
}

static int ldo_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = s2mps11_ldo_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int ldo_set_value(struct udevice *dev, int uV)
{
	return s2mps11_ldo_val(dev, PMIC_OP_SET, &uV);
}

static int ldo_get_enable(struct udevice *dev)
{
	bool enable = false;
	int ret;

	ret = s2mps11_ldo_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;
	return enable;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	int ret;

	ret = s2mps11_ldo_enable(dev, PMIC_OP_SET, &enable);
	if (ret)
		return ret;

	/* Wait the "enable delay" for voltage to start to rise */
	udelay(15);

	return 0;
}

static int ldo_get_mode(struct udevice *dev)
{
	int mode, ret;

	ret = s2mps11_ldo_mode(dev, PMIC_OP_GET, &mode);
	if (ret)
		return ret;
	return mode;
}

static int ldo_set_mode(struct udevice *dev, int mode)
{
	return s2mps11_ldo_mode(dev, PMIC_OP_SET, &mode);
}

static int s2mps11_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_LDO;

	switch (s2mps11_get_variant(dev)) {
	case VARIANT_S2MPS11:
		uc_pdata->mode = s2mps11_ldo_modes;
		uc_pdata->mode_count = ARRAY_SIZE(s2mps11_ldo_modes);
		break;
	case VARIANT_S2MPU05:
		uc_pdata->mode = s2mpu05_regulator_modes;
		uc_pdata->mode_count = ARRAY_SIZE(s2mpu05_regulator_modes);
		break;
	default:
		pr_err("Unknown device type\n");
		return -EINVAL;
	}

	return 0;
}

static const struct dm_regulator_ops s2mps11_ldo_ops = {
	.get_value	= ldo_get_value,
	.set_value	= ldo_set_value,
	.get_enable	= ldo_get_enable,
	.set_enable	= ldo_set_enable,
	.get_mode	= ldo_get_mode,
	.set_mode	= ldo_set_mode,
};

U_BOOT_DRIVER(s2mps11_ldo) = {
	.name = S2MPS11_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &s2mps11_ldo_ops,
	.probe = s2mps11_ldo_probe,
};

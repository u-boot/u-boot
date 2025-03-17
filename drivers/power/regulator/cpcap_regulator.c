// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/cpcap.h>
#include <linux/delay.h>
#include <linux/err.h>

/* CPCAP_REG_ASSIGN2 bits - Resource Assignment 2 */
#define CPCAP_BIT_VSDIO_SEL		BIT(15)
#define CPCAP_BIT_VDIG_SEL		BIT(14)
#define CPCAP_BIT_VCAM_SEL		BIT(13)
#define CPCAP_BIT_SW6_SEL		BIT(12)
#define CPCAP_BIT_SW5_SEL		BIT(11)
#define CPCAP_BIT_SW4_SEL		BIT(10)
#define CPCAP_BIT_SW3_SEL		BIT(9)
#define CPCAP_BIT_SW2_SEL		BIT(8)
#define CPCAP_BIT_SW1_SEL		BIT(7)

/* CPCAP_REG_ASSIGN3 bits - Resource Assignment 3 */
#define CPCAP_BIT_VUSBINT2_SEL		BIT(15)
#define CPCAP_BIT_VUSBINT1_SEL		BIT(14)
#define CPCAP_BIT_VVIB_SEL		BIT(13)
#define CPCAP_BIT_VWLAN1_SEL		BIT(12)
#define CPCAP_BIT_VRF1_SEL		BIT(11)
#define CPCAP_BIT_VHVIO_SEL		BIT(10)
#define CPCAP_BIT_VDAC_SEL		BIT(9)
#define CPCAP_BIT_VUSB_SEL		BIT(8)
#define CPCAP_BIT_VSIM_SEL		BIT(7)
#define CPCAP_BIT_VRFREF_SEL		BIT(6)
#define CPCAP_BIT_VPLL_SEL		BIT(5)
#define CPCAP_BIT_VFUSE_SEL		BIT(4)
#define CPCAP_BIT_VCSI_SEL		BIT(3)
#define CPCAP_BIT_SPARE_14_2		BIT(2)
#define CPCAP_BIT_VWLAN2_SEL		BIT(1)
#define CPCAP_BIT_VRF2_SEL		BIT(0)
#define CPCAP_BIT_NONE			0

/* CPCAP_REG_ASSIGN4 bits - Resource Assignment 4 */
#define CPCAP_BIT_VAUDIO_SEL		BIT(0)

/*
 * Off mode configuration bit. Used currently only by SW5 on omap4. There's
 * the following comment in Motorola Linux kernel tree for it:
 *
 * When set in the regulator mode, the regulator assignment will be changed
 * to secondary when the regulator is disabled. The mode will be set back to
 * primary when the regulator is turned on.
 */
#define CPCAP_REG_OFF_MODE_SEC		BIT(15)

#define CPCAP_REG(_reg, _assignment_reg, _assignment_mask, _mode_mask,		\
		  _volt_mask, _volt_shft, _mode_val, _off_mode_val, _val_tbl,	\
		  _mode_cntr, _volt_trans_time, _turn_on_time, _bit_offset) {	\
	.reg = CPCAP_REG_##_reg,						\
	.assignment_reg = CPCAP_REG_##_assignment_reg,				\
	.assignment_mask = CPCAP_BIT_##_assignment_mask,			\
	.mode_mask = _mode_mask,						\
	.volt_mask = _volt_mask,						\
	.volt_shft = _volt_shft,						\
	.mode_val = _mode_val,							\
	.off_mode_val = _off_mode_val,						\
	.val_tbl_sz = ARRAY_SIZE(_val_tbl),					\
	.val_tbl = _val_tbl,							\
	.mode_cntr = _mode_cntr,						\
	.volt_trans_time = _volt_trans_time,					\
	.turn_on_time = _turn_on_time,						\
	.bit_offset_from_cpcap_lowest_voltage = _bit_offset,			\
}

static const struct cpcap_regulator_data tegra20_regulators[CPCAP_REGULATORS_COUNT] = {
	/* BUCK */
	[CPCAP_SW1]      = CPCAP_REG(S1C1, ASSIGN2, SW1_SEL, 0x6f00, 0x007f,
				     0, 0x6800, 0, sw1_val_tbl, 0, 0, 1500, 0x0c),
	[CPCAP_SW2]      = CPCAP_REG(S2C1, ASSIGN2, SW2_SEL, 0x6f00, 0x007f,
				     0, 0x4804, 0, sw2_sw4_val_tbl, 0, 0, 1500, 0x18),
	[CPCAP_SW3]      = CPCAP_REG(S3C, ASSIGN2, SW3_SEL, 0x0578, 0x0003,
				     0, 0x043c, 0, sw3_val_tbl, 0, 0, 0, 0),
	[CPCAP_SW4]      = CPCAP_REG(S4C1, ASSIGN2, SW4_SEL, 0x6f00, 0x007f,
				     0, 0x4909, 0, sw2_sw4_val_tbl, 0, 0, 1500, 0x18),
	[CPCAP_SW5]      = CPCAP_REG(S5C, ASSIGN2, SW5_SEL, 0x0028, 0x0000,
				     0, 0x0020, 0, sw5_val_tbl, 0, 0, 1500, 0),
	[CPCAP_SW6]      = CPCAP_REG(S6C, ASSIGN2, SW6_SEL, 0x0000, 0x0000,
				     0, 0, 0, unknown_val_tbl, 0, 0, 0, 0),
	/* LDO */
	[CPCAP_VCAM]     = CPCAP_REG(VCAMC, ASSIGN2, VCAM_SEL, 0x0087, 0x0030,
				     4, 0x7, 0, vcam_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VCSI]     = CPCAP_REG(VCSIC, ASSIGN3, VCSI_SEL, 0x0047, 0x0010,
				     4, 0x7, 0, vcsi_val_tbl, 0, 350, 1000, 0),
	[CPCAP_VDAC]     = CPCAP_REG(VDACC, ASSIGN3, VDAC_SEL, 0x0087, 0x0030,
				     4, 0x0, 0, vdac_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VDIG]     = CPCAP_REG(VDIGC, ASSIGN2, VDIG_SEL, 0x0087, 0x0030,
				     4, 0x0, 0, vdig_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VFUSE]    = CPCAP_REG(VFUSEC, ASSIGN3, VFUSE_SEL, 0x00a0, 0x000f,
				     0, 0x0, 0, vfuse_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VHVIO]    = CPCAP_REG(VHVIOC, ASSIGN3, VHVIO_SEL, 0x0017, 0x0000,
				     0, 0x2, 0, vhvio_val_tbl, 0, 0, 1000, 0),
	[CPCAP_VSDIO]    = CPCAP_REG(VSDIOC, ASSIGN2, VSDIO_SEL, 0x0087, 0x0038,
				     3, 0x2, 0, vsdio_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VPLL]     = CPCAP_REG(VPLLC, ASSIGN3, VPLL_SEL, 0x0047, 0x0018,
				     3, 0x1, 0, vpll_val_tbl, 0, 420, 100, 0),
	[CPCAP_VRF1]     = CPCAP_REG(VRF1C, ASSIGN3, VRF1_SEL, 0x00ac, 0x0002,
				     1, 0x0, 0, vrf1_val_tbl, 0, 10, 1000, 0),
	[CPCAP_VRF2]     = CPCAP_REG(VRF2C, ASSIGN3, VRF2_SEL, 0x0023, 0x0008,
				     3, 0x0, 0, vrf2_val_tbl, 0, 10, 1000, 0),
	[CPCAP_VRFREF]   = CPCAP_REG(VRFREFC, ASSIGN3, VRFREF_SEL, 0x0023, 0x0008,
				     3, 0x0, 0, vrfref_val_tbl, 0, 420, 100, 0),
	[CPCAP_VWLAN1]   = CPCAP_REG(VWLAN1C, ASSIGN3, VWLAN1_SEL, 0x0047, 0x0010,
				     4, 0x0, 0, vwlan1_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VWLAN2]   = CPCAP_REG(VWLAN2C, ASSIGN3, VWLAN2_SEL, 0x020c, 0x00c0,
				     6, 0xd, 0, vwlan2_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VSIM]     = CPCAP_REG(VSIMC, ASSIGN3, NONE, 0x0023, 0x0008,
				     3, 0x0, 0, vsim_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VSIMCARD] = CPCAP_REG(VSIMC, ASSIGN3, NONE, 0x1e80, 0x0008,
				     3, 0x1E00, 0, vsimcard_val_tbl, 0, 420, 1000, 0),
	[CPCAP_VVIB]     = CPCAP_REG(VVIBC, ASSIGN3, VVIB_SEL, 0x0001, 0x000c,
				     2, 0x1, 0, vvib_val_tbl, 0, 500, 500, 0),
	[CPCAP_VUSB]     = CPCAP_REG(VUSBC, ASSIGN3, VUSB_SEL, 0x011c, 0x0040,
				     6, 0xc, 0, vusb_val_tbl, 0, 0, 1000, 0),
	[CPCAP_VAUDIO]   = CPCAP_REG(VAUDIOC, ASSIGN4, VAUDIO_SEL, 0x0016, 0x0001,
				     0, 0x5, 0, vaudio_val_tbl, 0, 0, 1000, 0),
};

static int cpcap_regulator_get_value(struct udevice *dev)
{
	const struct cpcap_regulator_data *regulator =
					&tegra20_regulators[dev->driver_data];
	int value, volt_shift = regulator->volt_shft;

	value = pmic_reg_read(dev->parent, regulator->reg);
	if (value < 0)
		return value;

	if (!(value & regulator->mode_mask))
		return 0;

	value &= regulator->volt_mask;
	value -= regulator->bit_offset_from_cpcap_lowest_voltage;

	return regulator->val_tbl[value >> volt_shift];
}

static int cpcap_regulator_set_value(struct udevice *dev, int uV)
{
	const struct cpcap_regulator_data *regulator =
					&tegra20_regulators[dev->driver_data];
	int value, ret, volt_shift = regulator->volt_shft;

	if (dev->driver_data == CPCAP_VRF1) {
		if (uV > 2500000)
			value = 0;
		else
			value = regulator->volt_mask;
	} else {
		for (value = 0; value < regulator->val_tbl_sz; value++)
			if (regulator->val_tbl[value] >= uV)
				break;

		if (value >= regulator->val_tbl_sz)
			value = regulator->val_tbl_sz;

		value <<= volt_shift;
		value += regulator->bit_offset_from_cpcap_lowest_voltage;
	}

	ret = pmic_clrsetbits(dev->parent, regulator->reg, regulator->volt_mask,
			      value);
	if (ret)
		return ret;

	if (regulator->volt_trans_time)
		udelay(regulator->volt_trans_time);

	return 0;
}

static int cpcap_regulator_get_enable(struct udevice *dev)
{
	const struct cpcap_regulator_data *regulator =
					&tegra20_regulators[dev->driver_data];
	int value;

	value = pmic_reg_read(dev->parent, regulator->reg);
	if (value < 0)
		return value;

	return (value & regulator->mode_mask) ? 1 : 0;
}

static int cpcap_regulator_set_enable(struct udevice *dev, bool enable)
{
	const struct cpcap_regulator_data *regulator =
					&tegra20_regulators[dev->driver_data];
	int ret;

	if (enable) {
		ret = pmic_clrsetbits(dev->parent, regulator->reg, regulator->mode_mask,
				      regulator->mode_val);
		if (ret)
			return ret;
	}

	if (regulator->mode_val & CPCAP_REG_OFF_MODE_SEC) {
		ret = pmic_clrsetbits(dev->parent, regulator->assignment_reg,
				      regulator->assignment_mask,
				      enable ? 0 : regulator->assignment_mask);
		if (ret)
			return ret;
	}

	if (!enable) {
		ret = pmic_clrsetbits(dev->parent, regulator->reg, regulator->mode_mask,
				      regulator->off_mode_val);
		if (ret)
			return ret;
	}

	if (regulator->turn_on_time)
		udelay(regulator->turn_on_time);

	return 0;
}

static int cpcap_regulator_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	int id;

	for (id = 0; id < CPCAP_REGULATORS_COUNT; id++)
		if (cpcap_regulator_to_name[id])
			if (!strcmp(dev->name, cpcap_regulator_to_name[id]))
				break;

	switch (id) {
	case CPCAP_SW1 ... CPCAP_SW6:
		uc_pdata->type = REGULATOR_TYPE_BUCK;
		break;

	case CPCAP_VCAM ... CPCAP_VAUDIO:
		uc_pdata->type = REGULATOR_TYPE_LDO;
		break;

	default:
		log_err("CPCAP: Invalid regulator ID\n");
		return -ENODEV;
	}

	dev->driver_data = id;
	return 0;
}

static const struct dm_regulator_ops cpcap_regulator_ops = {
	.get_value  = cpcap_regulator_get_value,
	.set_value  = cpcap_regulator_set_value,
	.get_enable = cpcap_regulator_get_enable,
	.set_enable = cpcap_regulator_set_enable,
};

U_BOOT_DRIVER(cpcap_sw) = {
	.name = CPCAP_SW_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &cpcap_regulator_ops,
	.probe = cpcap_regulator_probe,
};

U_BOOT_DRIVER(cpcap_ldo) = {
	.name = CPCAP_LDO_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &cpcap_regulator_ops,
	.probe = cpcap_regulator_probe,
};

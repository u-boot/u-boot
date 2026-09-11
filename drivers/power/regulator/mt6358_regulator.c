// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 MediaTek Inc.
 *
 * MediaTek MT6358-family regulator driver used for MT6366 rails.
 */

#include <dm.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <power/mt6358.h>
#include <power/pmic.h>
#include <power/regulator.h>

enum mt6358_regulator_type {
	MT6358_REG_TYPE_LINEAR,
	MT6358_REG_TYPE_TABLE,
	MT6358_REG_TYPE_FIXED,
};

struct mt6358_regulator_desc {
	const char *name;
	const char *of_match;
	enum mt6358_regulator_type type;
	int id;
	unsigned int uV_step;
	unsigned int n_voltages;
	const unsigned int *volt_table;
	unsigned int min_uV;
	unsigned int vsel_reg;
	unsigned int vsel_mask;
	unsigned int enable_reg;
	unsigned int enable_mask;
	unsigned int fixed_uV;
};

struct mt6358_regulator_info {
	struct mt6358_regulator_desc desc;
	const u32 *index_table;
	unsigned int n_table;
	u32 da_vsel_reg;
	u32 da_vsel_mask;
};

#define MT6366_BUCK(match, _name, _min, _max, _step, _enable_reg, _vsel_reg, \
		    _vsel_mask, _da_vsel_reg, _da_vsel_mask) \
	[MT6366_ID_##_name] = { \
		.desc = { \
			.name = #_name, \
			.of_match = match, \
			.type = MT6358_REG_TYPE_LINEAR, \
			.id = MT6366_ID_##_name, \
			.uV_step = (_step), \
			.n_voltages = ((_max) - (_min)) / (_step) + 1, \
			.min_uV = (_min), \
			.vsel_reg = (_vsel_reg), \
			.vsel_mask = (_vsel_mask), \
			.enable_reg = (_enable_reg), \
			.enable_mask = BIT(0), \
		}, \
		.da_vsel_reg = (_da_vsel_reg), \
		.da_vsel_mask = (_da_vsel_mask), \
	}

#define MT6366_LDO(match, _name, _volt_table, _index_table, _enable_reg, \
		   _vsel_reg, _vsel_mask) \
	[MT6366_ID_##_name] = { \
		.desc = { \
			.name = #_name, \
			.of_match = match, \
			.type = MT6358_REG_TYPE_TABLE, \
			.id = MT6366_ID_##_name, \
			.n_voltages = ARRAY_SIZE(_volt_table), \
			.volt_table = (_volt_table), \
			.vsel_reg = (_vsel_reg), \
			.vsel_mask = (_vsel_mask), \
			.enable_reg = (_enable_reg), \
			.enable_mask = BIT(0), \
		}, \
		.index_table = (_index_table), \
		.n_table = ARRAY_SIZE(_index_table), \
	}

#define MT6366_LDO1(match, _name, _min, _max, _step, _enable_reg, _vsel_reg, \
		    _vsel_mask, _da_vsel_reg, _da_vsel_mask) \
	[MT6366_ID_##_name] = { \
		.desc = { \
			.name = #_name, \
			.of_match = match, \
			.type = MT6358_REG_TYPE_LINEAR, \
			.id = MT6366_ID_##_name, \
			.uV_step = (_step), \
			.n_voltages = ((_max) - (_min)) / (_step) + 1, \
			.min_uV = (_min), \
			.vsel_reg = (_vsel_reg), \
			.vsel_mask = (_vsel_mask), \
			.enable_reg = (_enable_reg), \
			.enable_mask = BIT(0), \
		}, \
		.da_vsel_reg = (_da_vsel_reg), \
		.da_vsel_mask = (_da_vsel_mask), \
	}

#define MT6366_REG_FIXED(match, _name, _enable_reg, _fixed_uV) \
	[MT6366_ID_##_name] = { \
		.desc = { \
			.name = #_name, \
			.of_match = match, \
			.type = MT6358_REG_TYPE_FIXED, \
			.id = MT6366_ID_##_name, \
			.n_voltages = 1, \
			.enable_reg = (_enable_reg), \
			.enable_mask = BIT(0), \
			.fixed_uV = (_fixed_uV), \
		}, \
	}

static const unsigned int vdram2_voltages[] = {
	600000, 1800000
};

static const unsigned int vsim_voltages[] = {
	1700000, 1800000, 2700000, 3000000, 3100000
};

static const unsigned int vibr_voltages[] = {
	1200000, 1300000, 1500000, 1800000, 2000000, 2800000, 3000000, 3300000
};

static const unsigned int vusb_voltages[] = {
	3000000, 3100000
};

static const unsigned int vefuse_voltages[] = {
	1700000, 1800000, 1900000
};

static const unsigned int vmch_vemc_voltages[] = {
	2900000, 3000000, 3300000
};

static const unsigned int vcn33_voltages[] = {
	3300000, 3400000, 3500000
};

static const unsigned int vmc_voltages[] = {
	1800000, 2900000, 3000000, 3300000
};

static const u32 vdram2_idx[] = { 0, 12 };
static const u32 vsim_idx[] = { 3, 4, 8, 11, 12 };
static const u32 vibr_idx[] = { 0, 1, 2, 4, 5, 9, 11, 13 };
static const u32 vusb_idx[] = { 3, 4 };
static const u32 vefuse_idx[] = { 11, 12, 13 };
static const u32 vmch_vemc_idx[] = { 2, 3, 5 };
static const u32 vcn33_idx[] = { 1, 2, 3 };
static const u32 vmc_idx[] = { 4, 10, 11, 13 };

static int mt6358_set_voltage_sel_regmap(struct udevice *dev,
					 struct mt6358_regulator_info *info,
					 unsigned int sel)
{
	return pmic_clrsetbits(dev->parent, info->desc.vsel_reg,
			       info->desc.vsel_mask,
			       field_prep(info->desc.vsel_mask, sel));
}

static int mt6358_get_voltage_sel(struct udevice *dev,
				  struct mt6358_regulator_info *info)
{
	int ret;

	ret = pmic_reg_read(dev->parent, info->desc.vsel_reg);
	if (ret < 0)
		return ret;

	return field_get(info->desc.vsel_mask, ret);
}

static int mt6358_get_enable(struct udevice *dev)
{
	struct mt6358_regulator_info *info = dev_get_priv(dev);
	int ret;

	ret = pmic_reg_read(dev->parent, info->desc.enable_reg);
	if (ret < 0)
		return ret;

	return field_get(info->desc.enable_mask, ret) ? true : false;
}

static int mt6358_set_enable(struct udevice *dev, bool enable)
{
	struct mt6358_regulator_info *info = dev_get_priv(dev);

	return pmic_clrsetbits(dev->parent, info->desc.enable_reg,
			       info->desc.enable_mask,
			       field_prep(info->desc.enable_mask, enable));
}

static int mt6358_get_value(struct udevice *dev)
{
	struct mt6358_regulator_info *info = dev_get_priv(dev);
	int ret, idx;
	u32 selector;

	switch (info->desc.type) {
	case MT6358_REG_TYPE_LINEAR:
		ret = pmic_reg_read(dev->parent, info->da_vsel_reg);
		if (ret < 0)
			return ret;

		selector = field_get(info->da_vsel_mask, ret);
		if (selector >= info->desc.n_voltages)
			return -EINVAL;

		return info->desc.min_uV + (info->desc.uV_step * selector);
	case MT6358_REG_TYPE_TABLE:
		ret = mt6358_get_voltage_sel(dev, info);
		if (ret < 0)
			return ret;

		selector = ret;

		for (idx = 0; idx < info->n_table; idx++) {
			if (info->index_table[idx] == selector)
				return info->desc.volt_table[idx];
		}

		return -EINVAL;
	case MT6358_REG_TYPE_FIXED:
		return info->desc.fixed_uV;
	default:
		return -EINVAL;
	}
}

static int mt6358_set_value(struct udevice *dev, int uvolt)
{
	struct mt6358_regulator_info *info = dev_get_priv(dev);
	u32 selector;
	int idx;

	switch (info->desc.type) {
	case MT6358_REG_TYPE_LINEAR:
		if (uvolt < (int)info->desc.min_uV)
			return -EINVAL;

		selector = DIV_ROUND_UP(uvolt - info->desc.min_uV,
					info->desc.uV_step);
		if (selector >= info->desc.n_voltages)
			return -EINVAL;

		return mt6358_set_voltage_sel_regmap(dev, info, selector);
	case MT6358_REG_TYPE_TABLE:
		for (idx = 0; idx < info->desc.n_voltages; idx++) {
			if (info->desc.volt_table[idx] == (u32)uvolt)
				return mt6358_set_voltage_sel_regmap(dev, info,
							info->index_table[idx]);
		}

		return -EINVAL;
	default:
		return -EINVAL;
	}
}

static const struct mt6358_regulator_info mt6366_regulators[] = {
	MT6366_BUCK("vdram1", VDRAM1, 500000, 2087500, 12500,
		    MT6358_BUCK_VDRAM1_CON0, MT6358_BUCK_VDRAM1_ELR0, GENMASK(6, 0),
		    MT6358_BUCK_VDRAM1_DBG0, GENMASK(6, 0)),
	MT6366_BUCK("vcore", VCORE, 500000, 1293750, 6250,
		    MT6358_BUCK_VCORE_CON0, MT6358_BUCK_VCORE_ELR0, GENMASK(6, 0),
		    MT6358_BUCK_VCORE_DBG0, GENMASK(6, 0)),
	MT6366_BUCK("vpa", VPA, 500000, 3650000, 50000,
		    MT6358_BUCK_VPA_CON0, MT6358_BUCK_VPA_CON1, GENMASK(5, 0),
		    MT6358_BUCK_VPA_DBG0, GENMASK(5, 0)),
	MT6366_BUCK("vproc11", VPROC11, 500000, 1293750, 6250,
		    MT6358_BUCK_VPROC11_CON0, MT6358_BUCK_VPROC11_ELR0, GENMASK(6, 0),
		    MT6358_BUCK_VPROC11_DBG0, GENMASK(6, 0)),
	MT6366_BUCK("vproc12", VPROC12, 500000, 1293750, 6250,
		    MT6358_BUCK_VPROC12_CON0, MT6358_BUCK_VPROC12_ELR0, GENMASK(6, 0),
		    MT6358_BUCK_VPROC12_DBG0, GENMASK(6, 0)),
	MT6366_BUCK("vgpu", VGPU, 500000, 1293750, 6250,
		    MT6358_BUCK_VGPU_CON0, MT6358_BUCK_VGPU_ELR0, GENMASK(6, 0),
		    MT6358_BUCK_VGPU_ELR0, GENMASK(6, 0)),
	MT6366_BUCK("vs2", VS2, 500000, 2087500, 12500,
		    MT6358_BUCK_VS2_CON0, MT6358_BUCK_VS2_ELR0, GENMASK(6, 0),
		    MT6358_BUCK_VS2_DBG0, GENMASK(6, 0)),
	MT6366_BUCK("vmodem", VMODEM, 500000, 1293750, 6250,
		    MT6358_BUCK_VMODEM_CON0, MT6358_BUCK_VMODEM_ELR0, GENMASK(6, 0),
		    MT6358_BUCK_VMODEM_DBG0, GENMASK(6, 0)),
	MT6366_BUCK("vs1", VS1, 1000000, 2587500, 12500,
		    MT6358_BUCK_VS1_CON0, MT6358_BUCK_VS1_ELR0, GENMASK(6, 0),
		    MT6358_BUCK_VS1_DBG0, GENMASK(6, 0)),
	MT6366_REG_FIXED("vrf12", VRF12, MT6358_LDO_VRF12_CON0, 1200000),
	MT6366_REG_FIXED("vio18", VIO18, MT6358_LDO_VIO18_CON0, 1800000),
	MT6366_REG_FIXED("vcn18", VCN18, MT6358_LDO_VCN18_CON0, 1800000),
	MT6366_REG_FIXED("vfe28", VFE28, MT6358_LDO_VFE28_CON0, 2800000),
	MT6366_REG_FIXED("vcn28", VCN28, MT6358_LDO_VCN28_CON0, 2800000),
	MT6366_REG_FIXED("vxo22", VXO22, MT6358_LDO_VXO22_CON0, 2200000),
	MT6366_REG_FIXED("vaux18", VAUX18, MT6358_LDO_VAUX18_CON0, 1800000),
	MT6366_REG_FIXED("vbif28", VBIF28, MT6358_LDO_VBIF28_CON0, 2800000),
	MT6366_REG_FIXED("vio28", VIO28, MT6358_LDO_VIO28_CON0, 2800000),
	MT6366_REG_FIXED("va12", VA12, MT6358_LDO_VA12_CON0, 1200000),
	MT6366_REG_FIXED("vrf18", VRF18, MT6358_LDO_VRF18_CON0, 1800000),
	MT6366_REG_FIXED("vaud28", VAUD28, MT6358_LDO_VAUD28_CON0, 2800000),
	MT6366_LDO("vdram2", VDRAM2, vdram2_voltages, vdram2_idx,
		   MT6358_LDO_VDRAM2_CON0, MT6358_LDO_VDRAM2_ELR0, GENMASK(3, 0)),
	MT6366_LDO("vsim1", VSIM1, vsim_voltages, vsim_idx,
		   MT6358_LDO_VSIM1_CON0, MT6358_VSIM1_ANA_CON0, GENMASK(11, 8)),
	MT6366_LDO("vibr", VIBR, vibr_voltages, vibr_idx,
		   MT6358_LDO_VIBR_CON0, MT6358_VIBR_ANA_CON0, GENMASK(11, 8)),
	MT6366_LDO("vusb", VUSB, vusb_voltages, vusb_idx,
		   MT6358_LDO_VUSB_CON0_0, MT6358_VUSB_ANA_CON0, GENMASK(10, 8)),
	MT6366_LDO("vefuse", VEFUSE, vefuse_voltages, vefuse_idx,
		   MT6358_LDO_VEFUSE_CON0, MT6358_VEFUSE_ANA_CON0, GENMASK(11, 8)),
	MT6366_LDO("vmch", VMCH, vmch_vemc_voltages, vmch_vemc_idx,
		   MT6358_LDO_VMCH_CON0, MT6358_VMCH_ANA_CON0, GENMASK(10, 8)),
	MT6366_LDO("vemc", VEMC, vmch_vemc_voltages, vmch_vemc_idx,
		   MT6358_LDO_VEMC_CON0, MT6358_VEMC_ANA_CON0, GENMASK(10, 8)),
	MT6366_LDO("vcn33", VCN33, vcn33_voltages, vcn33_idx,
		   MT6358_LDO_VCN33_CON0_0, MT6358_VCN33_ANA_CON0, GENMASK(9, 8)),
	MT6366_LDO("vmc", VMC, vmc_voltages, vmc_idx,
		   MT6358_LDO_VMC_CON0, MT6358_VMC_ANA_CON0, GENMASK(11, 8)),
	MT6366_LDO("vsim2", VSIM2, vsim_voltages, vsim_idx,
		   MT6358_LDO_VSIM2_CON0, MT6358_VSIM2_ANA_CON0, GENMASK(11, 8)),
	MT6366_LDO1("vsram-proc11", VSRAM_PROC11, 500000, 1293750, 6250,
		    MT6358_LDO_VSRAM_PROC11_CON0, MT6358_LDO_VSRAM_CON0,
		    GENMASK(6, 0), MT6358_LDO_VSRAM_PROC11_DBG0, GENMASK(14, 8)),
	MT6366_LDO1("vsram-others", VSRAM_OTHERS, 500000, 1293750, 6250,
		    MT6358_LDO_VSRAM_OTHERS_CON0, MT6358_LDO_VSRAM_CON2,
		    GENMASK(6, 0), MT6358_LDO_VSRAM_OTHERS_DBG0, GENMASK(14, 8)),
	MT6366_LDO1("vsram-gpu", VSRAM_GPU, 500000, 1293750, 6250,
		    MT6358_LDO_VSRAM_GPU_CON0, MT6358_LDO_VSRAM_CON3,
		    GENMASK(6, 0), MT6358_LDO_VSRAM_GPU_DBG0, GENMASK(14, 8)),
	MT6366_LDO1("vsram-proc12", VSRAM_PROC12, 500000, 1293750, 6250,
		    MT6358_LDO_VSRAM_PROC12_CON0, MT6358_LDO_VSRAM_CON1,
		    GENMASK(6, 0), MT6358_LDO_VSRAM_PROC12_DBG0, GENMASK(14, 8)),
};

static int mt6358_regulator_probe(struct udevice *dev)
{
	struct mt6358_regulator_info *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(mt6366_regulators); i++) {
		if (!strcmp(dev->name, mt6366_regulators[i].desc.of_match)) {
			*priv = mt6366_regulators[i];
			return 0;
		}
	}

	return -ENOENT;
}

static const struct dm_regulator_ops mt6358_regulator_ops = {
	.get_value = mt6358_get_value,
	.set_value = mt6358_set_value,
	.get_enable = mt6358_get_enable,
	.set_enable = mt6358_set_enable,
};

U_BOOT_DRIVER(mt6358_regulator) = {
	.name = MT6358_REGULATOR_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &mt6358_regulator_ops,
	.probe = mt6358_regulator_probe,
	.priv_auto = sizeof(struct mt6358_regulator_info),
};

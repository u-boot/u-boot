// SPDX-License-Identifier: GPL-2.0

#include <common.h>
#include <dm/device_compat.h>
#include <dm.h>
#include <linux/bitops.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <pwrap/pwrap.h>

/*
 * MT6328 regulators' information
 *
 * @desc: standard fields of regulator description.
 * @qi: Mask for query enable signal status of regulators
 */

enum {
    MT6328_LDOMODE_STANDBY,
    MT6328_LDOMODE_NORMAL,
};

enum {
    MT6328_BUCKMODE_FAST,
    MT6328_BUCKMODE_NORMAL,
};

struct mt6328_regulator_info {
	const char *dt_node_name;
    enum regulator_type type;
    u32 voltage;
	u32 qi;
    u32 vsel_reg;
    u32 vsel_mask;
    u32 vsel_shift;
	u32 modeset_reg;
	u32 modeset_mask;
    u32 enable_reg;
    u32 enable_mask;
    u32 modeset_shift;
    u32 min_uV;
	u32 step_uV;
	u32 max_uV;
    unsigned int *volt_table;
};
struct mt6328_priv {
    struct mt6328_regulator_info *reg_info;
};

#define MT6328_BUCK(dt_name, vreg, min, max, step, enreg,	\
		vosel, vosel_mask, voselon, vosel_ctrl, _modeset_reg, _modeset_shift)			\
{							\
    .min_uV = min,                  \
    .max_uV = max,                  \
    .step_uV = step,              \
	.type = REGULATOR_TYPE_BUCK,             \
    .dt_node_name = dt_name,                \
	.vsel_reg = vosel,					\
	.vsel_mask = vosel_mask,				\
	.enable_reg = enreg,					\
	.enable_mask = BIT(0),					\
	.qi = BIT(13),							\
    .modeset_reg = _modeset_reg,				\
	.modeset_mask = BIT(_modeset_shift),			\
}

#define MT6328_LDO(dt_name, vreg, ldo_volt_table, enreg, enbit, vosel,	\
		vosel_mask, _modeset_reg, _modeset_mask)		\
{							\
	.dt_node_name = dt_name,                \
	.type = REGULATOR_TYPE_LDO,				\
	.volt_table = ldo_volt_table,				\
	.vsel_reg = vosel,					\
	.vsel_mask = vosel_mask,				\
	.enable_reg = enreg,					\
	.enable_mask = BIT(enbit),				\
	.qi = BIT(15),							\
	.modeset_reg = _modeset_reg,					\
	.modeset_mask = _modeset_mask,					\
}

#define MT6328_REG_FIXED(dt_name, vreg, enreg, enbit, volt,		\
		_modeset_reg, _modeset_mask)				\
{							\
	.dt_node_name = dt_name,                \
	.type = REGULATOR_TYPE_FIXED,				\
	.enable_reg = enreg,					\
	.enable_mask = BIT(enbit),				\
	.min_uV = volt,						\
    .qi = BIT(15),							\
	.modeset_reg = _modeset_reg,					\
	.modeset_mask = _modeset_mask,					\
}

static unsigned int ldo_volt_table1[] = {
	1500000, 1800000, 2500000, 2800000,
};

static unsigned int ldo_volt_table2[] = {
	1700000, 1800000, 1860000, 2760000, 3000000, 3100000,
};

static unsigned int ldo_volt_table3[] = {
	1200000, 1300000, 1500000, 1800000, 2000000, 2800000, 3000000, 3300000,
};

static unsigned int ldo_volt_table4[] = {
	1800000, 2900000, 3000000, 3300000,
};

static unsigned int ldo_volt_table5[] = {
	2900000, 3000000, 3300000,
};

static unsigned int ldo_volt_table6[] = {
	3300000, 3400000, 3500000, 3600000,
};

static unsigned int ldo_volt_table7[] = {
	1800000, 1900000, 2000000, 2100000, 2200000,
};

static unsigned int ldo_volt_table8[] = {
	1240000, 13900000, 1540000,
};

static unsigned int ldo_volt_table9[] = {
	1200000, 1300000, 1500000, 1800000,
};

static unsigned int ldo_volt_table10[] = {
	900000, 1000000, 1100000, 1220000, 1300000, 1500000,
};

static unsigned int ldo_volt_table11[] = {
    1300000, 1500000, 1800000, 2000000,
};

static struct dm_regulator_mode mt6328_ldo_modes[] = {
	{ .id = MT6328_LDOMODE_NORMAL,
		.register_value = 0, .name = "NORMAL" },
	{ .id = MT6328_LDOMODE_STANDBY,
		.register_value = 1, .name = "STANDBY" },
};

static struct dm_regulator_mode mt6328_buck_modes[] = {
	{ .id = MT6328_BUCKMODE_NORMAL,
		.register_value = 0, .name = "NORMAL" },
	{ .id = MT6328_BUCKMODE_FAST,
		.register_value = 1, .name = "FAST" },
};
static int mt6328_enable(struct udevice *dev, bool enable)
{
    struct mt6328_priv *priv = dev_get_priv(dev);
    struct mt6328_regulator_info *info = priv->reg_info;

    return pmic_write_u32(dev->parent, info->enable_reg, enable ? info->enable_mask : 0);
}

static int mt6328_get_status(struct udevice *dev)
{
	int ret;
	u32 regval;
    struct mt6328_priv *priv = dev_get_priv(dev);
	struct mt6328_regulator_info *info = priv->reg_info;

	ret = pmic_read_u32(dev->parent, info->enable_reg, &regval);
	if (ret != 0) {
		dev_err(dev, "Failed to get enable reg: %d\n", ret);
		return ret;
	}

	return (regval & info->qi) ? true : false;
}

static struct dm_regulator_mode
	*mt6328_find_mode_by_id(int id,
				 struct dm_regulator_mode *modes,
				uint mode_count)
{
	for (; mode_count; mode_count--) {
		if (modes->id == id)
			return modes;
		modes++;
	}
	return NULL;
}

static int mt6328_ldo_set_mode(struct udevice *dev, int mode_id)
{
    struct mt6328_priv *priv = dev_get_priv(dev);
	struct mt6328_regulator_info *info = priv->reg_info;
	struct dm_regulator_mode *mode;
	u32 rdata;
    if (!info->modeset_mask) {
		dev_err(dev, "regulator %s doesn't support get_mode\n",
			info->dt_node_name);
		return -EINVAL;
	}
	mode = mt6328_find_mode_by_id(mode_id,
				      mt6328_ldo_modes,
				      ARRAY_SIZE(mt6328_ldo_modes));
	if (!mode) {
		return -EINVAL;
	}
	pmic_read_u32(dev->parent, info->modeset_reg, &rdata);
	rdata &= ~(info->modeset_mask);
	rdata |= (mode->register_value << (ffs(info->modeset_mask) - 1));
	return pmic_write_u32(dev->parent, info->modeset_reg, rdata);
}

static int mt6328_ldo_get_mode(struct udevice *dev)
{
	unsigned int val;
	unsigned int mode;
	int ret;
    struct mt6328_priv *priv = dev_get_priv(dev);
	struct mt6328_regulator_info *info = priv->reg_info;

	if (!info->modeset_mask) {
		dev_err(dev->parent, "regulator %s doesn't support get_mode\n",
			info->dt_node_name);
		return -EINVAL;
	}

	ret = pmic_read_u32(dev->parent, info->modeset_reg, &val);
	if (ret < 0)
		return ret;
	if (val & info->modeset_mask)
		mode = MT6328_LDOMODE_STANDBY;
	else
		mode = MT6328_LDOMODE_NORMAL;

	return mode;
}

static int mt6328_buck_set_mode(struct udevice *dev,
				     int mode_id)
{
    struct mt6328_priv *priv = dev_get_priv(dev);
	struct mt6328_regulator_info *info = priv->reg_info;
	struct dm_regulator_mode *mode;
	u32 rdata;
	mode = mt6328_find_mode_by_id(mode_id,
				      mt6328_buck_modes,
				      ARRAY_SIZE(mt6328_buck_modes));
	if (!mode) {
		return -EINVAL;
	}
	pmic_read_u32(dev->parent, info->modeset_reg, &rdata);
	rdata &= ~(info->modeset_mask);
	rdata |= (mode->register_value << (ffs(info->modeset_mask) - 1));
	return pmic_write_u32(dev->parent, info->modeset_reg, rdata);
}
static int mt6328_get_voltage(struct udevice *dev)
{
    struct mt6328_priv *priv = dev_get_priv(dev);
	struct mt6328_regulator_info *info = priv->reg_info;
    u32 rdata;
	u32 val, ret, voltage;
    u32 uV =0;

    if (info->type == REGULATOR_TYPE_FIXED) {
        voltage = info->voltage;
    } else if (info->volt_table) {
        ret = pmic_read_u32(dev->parent, info->vsel_reg, &rdata);
		rdata &= info->vsel_mask;
        val = (rdata >> (ffs(info->vsel_mask) - 1));
        if (ret < 0)
            return val;
        voltage = info->volt_table[val];
    } else {
        if (info->min_uV < 0) {
            debug("Need to provide min_uV in dts.\n");
            return -EINVAL;
        }
        ret = pmic_read_u32(dev->parent, info->vsel_reg, &rdata);
		rdata &= info->vsel_mask;
        val = (rdata >> (ffs(info->vsel_mask) -1));
        if (val < 0)
            return val;
        uV = info->min_uV + (int)val * info->step_uV;
        voltage = uV;
		}

		return voltage;
}

static int mt6328_set_voltage(struct udevice *dev, int uV)
{
    struct dm_regulator_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
    struct mt6328_priv *priv = dev_get_priv(dev);
	struct mt6328_regulator_info *info = priv->reg_info;
    int i;
	u32 rdata;
    if (uc_pdata->type == REGULATOR_TYPE_FIXED) {
		debug("Set voltage for REGULATOR_TYPE_FIXED regulator\n");
		return -EINVAL;
	} else if (info->volt_table) {
		for (i = 0; i <= info->vsel_mask; i++) {
			if (uV == info->volt_table[i])
				break;
		}
		if (i == info->vsel_mask + 1) {
			debug("Unsupported voltage %u\n", uV);
			return -EINVAL;
		}
		pmic_read_u32(dev->parent, info->vsel_reg, &rdata);
		rdata &= ~(info->vsel_mask);
		rdata |= (i << (ffs(info->vsel_mask) - 1));
		return pmic_write_u32(dev->parent, info->vsel_reg, rdata);
	} else {
		if (info->min_uV < 0) {
			debug("Need to provide min_uV in dts.\n");
			return -EINVAL;
		}
		pmic_read_u32(dev->parent, info->vsel_reg, &rdata);
		rdata &= ~(info->vsel_mask);
		rdata |= (((uV - info->min_uV) / info->step_uV) << (ffs(info->vsel_mask) - 1));
		return pmic_write_u32(dev->parent, info->vsel_reg,
				       rdata);
	}

	return 0;
}

static int mt6328_buck_get_mode(struct udevice *dev)
{
    struct mt6328_priv *priv = dev_get_priv(dev);
	struct mt6328_regulator_info *info = priv->reg_info;
	int ret, regval;

	ret = pmic_read_u32(dev->parent, info->modeset_reg, &regval);
	if (ret != 0) {
		dev_err(dev,
			"Failed to get mt6328 regulator mode: %d\n", ret);
		return ret;
	}

	switch ((regval & info->modeset_mask) >> (ffs(info->modeset_mask) - 1)) {
	case MT6328_BUCKMODE_NORMAL:
		return MT6328_BUCKMODE_NORMAL;
	case MT6328_BUCKMODE_FAST:
		return MT6328_BUCKMODE_FAST;
	default:
		return -EINVAL;
	}
}

/* The array is indexed by id(MT6328_ID_XXX) */
static struct mt6328_regulator_info mt6328_bucks[] = {
	MT6328_BUCK("buck_vproc", VPROC, 700000, 1493750, 6250, 0x0486, 0x048a, 0x7f, 0x048c, 0x0482, 0x0456, 11),
    MT6328_BUCK("buck_vlte", VMODEM, 700000, 1493750, 6250, 0x04d6, 0x04da, 0x7f, 0x04dc, 0x04d2, 0x046c, 11),
    MT6328_BUCK("buck_vcore1", VCORE, 700000, 1493750, 6250, 0x0612, 0x0616, 0x7f, 0x0618, 0x060e, 0x0442, 11),
	MT6328_BUCK("buck_vsys22", VSYS, 1400000, 2987500, 12500, 0x063a, 0x063e, 0x7f, 0x0640, 0x0636, 0x044c, 9),
	MT6328_BUCK("buck_vpa", VPA, 500000, 3650000, 50000, 0x0662, 0x0666, 0x3f, 0x0668, 0x065e, 0x0464, 10),
	MT6328_BUCK("buck_vsram", VSRAM, 700000, 1493750, 6250, 0x0a40, 0x0a88, 0x7f, 0x04b4, 0x04aa, 0x0a40, 0),
};

static struct mt6328_regulator_info mt6328_fixed[] = {
    MT6328_REG_FIXED("fixed_ldo_vio18", VIO18, 0x0a36, 14, 1800000, 0x0a36, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vtcxo_0", VTCXO_0, 0x0a00, 1, 2800000, 0x0a00, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vtcxo_1", VTCXO_1, 0x0a02, 1, 2800000, 0x0a02, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vcn28", VCN28, 0x0a0e, 1, 2800000, 0x0a0e, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vaud28", VAUD28, 0x0a04, 1, 2800000, 0x0a04, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vaux18", VAUX18, 0x0a06, 1, 2800000, 0x0a06, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vio28", VIO28, 0x0a28, 1, 2800000, 0x0a28, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vusb33", VUSB, 0x0a1a, 1, 3300000, 0x0a1a, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vrtc", VRTC, 0x0a4a, 1, 2800000, -1, 0),
    MT6328_REG_FIXED("fixed_ldo_vrf18_0", VRF18_0, 0x0a08, 1, 1825000, 0x0a08, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vrf18_1", VRF18_1, 0x0a16, 1, 1825000, 0x0a16, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vcn18", VCN18, 0x0a3a, 1, 1800000, 0x0a3a, 0x1),
    MT6328_REG_FIXED("fixed_ldo_vtref", TREF, 0x0a44, 1, 1800000, -1, 0),
};

static struct mt6328_regulator_info mt6328_ldo[] = {
	MT6328_LDO("ldo_vcn33_bt", VCN33_BT, ldo_volt_table6, 0x0a14, 1, 0x0a54, 0x70, 0x0a10, 0x1),
	MT6328_LDO("ldo_vcn33_wifi", VCN33_WIFI, ldo_volt_table6, 0x0a12, 1, 0x0a5a, 0x70, 0x0a10, 0x1),
	MT6328_LDO("ldo_vcama", VCAMA, ldo_volt_table1, 0x0a0c, 1, 0x0a58, 0x30, -1, 0),
	MT6328_LDO("ldo_vmc", VMC, ldo_volt_table4, 0x0a20, 1, 0x0a6a, 0x30, 0x0a20, 0x1),
	MT6328_LDO("ldo_vmch", VMCH, ldo_volt_table5, 0x0a1c, 1, 0x0a66, 0x30, 0x0a1c, 0x1),
	MT6328_LDO("ldo_vemc3v3", VEMC3V3, ldo_volt_table5, 0x0a24, 1, 0x0a64, 0x30, 0x0a24, 0x1),
	MT6328_LDO("ldo_vgp1", VGP1, ldo_volt_table3, 0x0a2c, 1, 0x0a72, 0x70, 0x0a2c, 0x1),
	MT6328_LDO("ldo_vsim1", VSIM1, ldo_volt_table2, 0x0a32, 1, 0x0a60, 0x70, 0x0a32, 0x1),
	MT6328_LDO("ldo_vsim2", VSIM2, ldo_volt_table2, 0x0a34, 1, 0x0a62, 0x70, 0x0a34, 0x1),
	MT6328_LDO("ldo_vcamaf", VCAMAF, ldo_volt_table3, 0x0a2a, 1, 0x0a6c, 0x70, 0x0a2a, 0x1),
    MT6328_LDO("ldo_vefuse", VEFUSE, ldo_volt_table7, 0x0a30, 1, 0x0a5e, 0x70, 0x0a30, 0x1),
	MT6328_LDO("ldo_vibr", VIBR, ldo_volt_table11, 0x0a38, 1, 0x0a6e, 0x70, 0x0a38, 0x1),
	MT6328_LDO("ldo_vm", VM, ldo_volt_table8, 0x0a46, 1, 0x0a76, 0x3, 0x0a46, 0x1),
	MT6328_LDO("ldo_vcamd", VCAMD, ldo_volt_table10, 0x0a3c, 1, 0x0a7a, 0x70, 0x0a3c, 0x1),
	MT6328_LDO("ldo_vcamio", VCAMIO, ldo_volt_table9, 0x0a3e, 1,  0x0a84, 0x30, 0x0a3e, 0x1),
};

static int mt6328_buck_probe(struct udevice *dev)
{
    struct dm_regulator_uclass_plat *uc_pdata;
    struct mt6328_priv *priv = dev_get_priv(dev);
    int i;
    
    for (i = 0; i < ARRAY_SIZE(mt6328_bucks); i++) {
        struct mt6328_regulator_info *info = &mt6328_bucks[i];
        if (!strcmp(info->dt_node_name, dev->name)) {
            priv->reg_info = info;
            break;
        }
    }
    if (!priv->reg_info) {
        return -ENODEV;
    }
    
    printf("buck = %s initialized\n", dev->name);
    
	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode = mt6328_buck_modes;
	uc_pdata->mode_count = ARRAY_SIZE(mt6328_buck_modes);

	return 0;
}

static int mt6328_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct mt6328_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(mt6328_ldo); i++) {
		struct mt6328_regulator_info *info = &mt6328_ldo[i];
		if (!strcmp(info->dt_node_name, dev->name)) {
			priv->reg_info = info;
			break;
        }
	}
    if (!priv->reg_info) {
        return -ENODEV;
    }
    
    printf("ldo = %s initialized\n", dev->name);
    
	uc_pdata = dev_get_uclass_plat(dev);
	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode = mt6328_ldo_modes;
	uc_pdata->mode_count = ARRAY_SIZE(mt6328_ldo_modes);

	return 0;
}

static int mt6328_fixed_probe(struct udevice *dev)
{
    struct dm_regulator_uclass_plat *uc_pdata;
    struct mt6328_priv *priv = dev_get_priv(dev);
    int i;

    for (i = 0; i < ARRAY_SIZE(mt6328_fixed); i++) {
        struct mt6328_regulator_info *info = &mt6328_fixed[i];
        if (!strcmp(info->dt_node_name, dev->name)) {
            priv->reg_info = info;
            break;
        }
    }
    if (!priv->reg_info) {
        return -ENODEV;
    }
    printf("fixed ldo = %s initialized\n", dev->name);
    uc_pdata = dev_get_uclass_plat(dev);
    uc_pdata->type = REGULATOR_TYPE_FIXED;
    uc_pdata->mode = mt6328_ldo_modes;
    uc_pdata->mode_count = ARRAY_SIZE(mt6328_ldo_modes);

    return 0;
}

static const struct dm_regulator_ops mt6328_buck_ops = {
	.set_value = mt6328_set_voltage,
	.get_value = mt6328_get_voltage,
	.set_enable = mt6328_enable,
	.get_enable = mt6328_get_status,
    .set_mode = mt6328_buck_set_mode,
	.get_mode = mt6328_buck_get_mode,
};

U_BOOT_DRIVER(mt6328_buck) = {
	.name = "mt6328_buck",
	.id = UCLASS_REGULATOR,
	.ops = &mt6328_buck_ops,
	.probe = mt6328_buck_probe,
	.priv_auto	= sizeof(struct mt6328_priv),
};

static const struct dm_regulator_ops mt6328_ldo_ops = {
	.set_value = mt6328_set_voltage,
	.get_value = mt6328_get_voltage,
	.set_enable = mt6328_enable,
	.get_enable = mt6328_get_status,
	.set_mode = mt6328_ldo_set_mode,
	.get_mode = mt6328_ldo_get_mode,
};

U_BOOT_DRIVER(mt6328_ldo) = {
	.name = "mt6328_ldo",
	.id = UCLASS_REGULATOR,
	.ops = &mt6328_ldo_ops,
	.probe = mt6328_ldo_probe,
	.priv_auto	= sizeof(struct mt6328_priv),
};

static const struct dm_regulator_ops mt6328_fixed_ops = {
    .set_enable = mt6328_enable,
    .get_enable = mt6328_get_status,
    .set_mode = mt6328_ldo_set_mode,
    .get_mode = mt6328_ldo_get_mode,
};

U_BOOT_DRIVER(mt6328_fixed) = {
    .name = "mt6328_fixed",
    .id = UCLASS_REGULATOR,
    .ops = &mt6328_fixed_ops,
    .probe = mt6328_fixed_probe,
    .priv_auto    = sizeof(struct mt6328_priv),
};

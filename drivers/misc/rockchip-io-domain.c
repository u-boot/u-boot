// SPDX-License-Identifier: GPL-2.0
/*
 * Rockchip IO Voltage Domain driver
 *
 * Ported from linux drivers/soc/rockchip/io-domain.c
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <regmap.h>
#include <syscon.h>
#include <power/regulator.h>

#define MAX_SUPPLIES		16

/*
 * The max voltage for 1.8V and 3.3V come from the Rockchip datasheet under
 * "Recommended Operating Conditions" for "Digital GPIO".   When the typical
 * is 3.3V the max is 3.6V.  When the typical is 1.8V the max is 1.98V.
 *
 * They are used like this:
 * - If the voltage on a rail is above the "1.8" voltage (1.98V) we'll tell the
 *   SoC we're at 3.3.
 * - If the voltage on a rail is above the "3.3" voltage (3.6V) we'll consider
 *   that to be an error.
 */
#define MAX_VOLTAGE_1_8		1980000
#define MAX_VOLTAGE_3_3		3600000

#define PX30_IO_VSEL			0x180
#define PX30_IO_VSEL_VCCIO6_SRC		BIT(0)
#define PX30_IO_VSEL_VCCIO6_SUPPLY_NUM	1

#define RK3328_SOC_CON4			0x410
#define RK3328_SOC_CON4_VCCIO2		BIT(7)
#define RK3328_SOC_VCCIO2_SUPPLY_NUM	1

#define RK3399_PMUGRF_CON0		0x180
#define RK3399_PMUGRF_CON0_VSEL		BIT(8)
#define RK3399_PMUGRF_VSEL_SUPPLY_NUM	9

#define RK3568_PMU_GRF_IO_VSEL0		0x0140
#define RK3568_PMU_GRF_IO_VSEL1		0x0144
#define RK3568_PMU_GRF_IO_VSEL2		0x0148

struct rockchip_iodomain_soc_data {
	int grf_offset;
	const char *supply_names[MAX_SUPPLIES];
	int (*write)(struct regmap *grf, uint offset, int idx, int uV);
};

static int rk3568_iodomain_write(struct regmap *grf, uint offset, int idx, int uV)
{
	u32 is_3v3 = uV > MAX_VOLTAGE_1_8;
	u32 val0, val1;
	int b;

	switch (idx) {
	case 0: /* pmuio1 */
		break;
	case 1: /* pmuio2 */
		b = idx;
		val0 = BIT(16 + b) | (is_3v3 ? 0 : BIT(b));
		b = idx + 4;
		val1 = BIT(16 + b) | (is_3v3 ? BIT(b) : 0);

		regmap_write(grf, RK3568_PMU_GRF_IO_VSEL2, val0);
		regmap_write(grf, RK3568_PMU_GRF_IO_VSEL2, val1);
		break;
	case 3: /* vccio2 */
		break;
	case 2: /* vccio1 */
	case 4: /* vccio3 */
	case 5: /* vccio4 */
	case 6: /* vccio5 */
	case 7: /* vccio6 */
	case 8: /* vccio7 */
		b = idx - 1;
		val0 = BIT(16 + b) | (is_3v3 ? 0 : BIT(b));
		val1 = BIT(16 + b) | (is_3v3 ? BIT(b) : 0);

		regmap_write(grf, RK3568_PMU_GRF_IO_VSEL0, val0);
		regmap_write(grf, RK3568_PMU_GRF_IO_VSEL1, val1);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int rockchip_iodomain_write(struct regmap *grf, uint offset, int idx, int uV)
{
	u32 val;

	/* set value bit */
	val = (uV > MAX_VOLTAGE_1_8) ? 0 : 1;
	val <<= idx;

	/* apply hiword-mask */
	val |= (BIT(idx) << 16);

	return regmap_write(grf, offset, val);
}

static int px30_iodomain_write(struct regmap *grf, uint offset, int idx, int uV)
{
	int ret = rockchip_iodomain_write(grf, offset, idx, uV);

	if (!ret && idx == PX30_IO_VSEL_VCCIO6_SUPPLY_NUM) {
		/*
		 * set vccio6 iodomain to also use this framework
		 * instead of a special gpio.
		 */
		u32 val = PX30_IO_VSEL_VCCIO6_SRC | (PX30_IO_VSEL_VCCIO6_SRC << 16);
		ret = regmap_write(grf, PX30_IO_VSEL, val);
	}

	return ret;
}

static int rk3328_iodomain_write(struct regmap *grf, uint offset, int idx, int uV)
{
	int ret = rockchip_iodomain_write(grf, offset, idx, uV);

	if (!ret && idx == RK3328_SOC_VCCIO2_SUPPLY_NUM) {
		/*
		 * set vccio2 iodomain to also use this framework
		 * instead of a special gpio.
		 */
		u32 val = RK3328_SOC_CON4_VCCIO2 | (RK3328_SOC_CON4_VCCIO2 << 16);
		ret = regmap_write(grf, RK3328_SOC_CON4, val);
	}

	return ret;
}

static int rk3399_pmu_iodomain_write(struct regmap *grf, uint offset, int idx, int uV)
{
	int ret = rockchip_iodomain_write(grf, offset, idx, uV);

	if (!ret && idx == RK3399_PMUGRF_VSEL_SUPPLY_NUM) {
		/*
		 * set pmu io iodomain to also use this framework
		 * instead of a special gpio.
		 */
		u32 val = RK3399_PMUGRF_CON0_VSEL | (RK3399_PMUGRF_CON0_VSEL << 16);
		ret = regmap_write(grf, RK3399_PMUGRF_CON0, val);
	}

	return ret;
}

static const struct rockchip_iodomain_soc_data soc_data_px30 = {
	.grf_offset = 0x180,
	.supply_names = {
		NULL,
		"vccio6-supply",
		"vccio1-supply",
		"vccio2-supply",
		"vccio3-supply",
		"vccio4-supply",
		"vccio5-supply",
		"vccio-oscgpi-supply",
	},
	.write = px30_iodomain_write,
};

static const struct rockchip_iodomain_soc_data soc_data_px30_pmu = {
	.grf_offset = 0x100,
	.supply_names = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"pmuio1-supply",
		"pmuio2-supply",
	},
	.write = rockchip_iodomain_write,
};

static const struct rockchip_iodomain_soc_data soc_data_rk3328 = {
	.grf_offset = 0x410,
	.supply_names = {
		"vccio1-supply",
		"vccio2-supply",
		"vccio3-supply",
		"vccio4-supply",
		"vccio5-supply",
		"vccio6-supply",
		"pmuio-supply",
	},
	.write = rk3328_iodomain_write,
};

static const struct rockchip_iodomain_soc_data soc_data_rk3399 = {
	.grf_offset = 0xe640,
	.supply_names = {
		"bt656-supply",		/* APIO2_VDD */
		"audio-supply",		/* APIO5_VDD */
		"sdmmc-supply",		/* SDMMC0_VDD */
		"gpio1830-supply",	/* APIO4_VDD */
	},
	.write = rockchip_iodomain_write,
};

static const struct rockchip_iodomain_soc_data soc_data_rk3399_pmu = {
	.grf_offset = 0x180,
	.supply_names = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"pmu1830-supply",	/* PMUIO2_VDD */
	},
	.write = rk3399_pmu_iodomain_write,
};

static const struct rockchip_iodomain_soc_data soc_data_rk3568_pmu = {
	.grf_offset = 0x140,
	.supply_names = {
		NULL,
		"pmuio2-supply",
		"vccio1-supply",
		NULL,
		"vccio3-supply",
		"vccio4-supply",
		"vccio5-supply",
		"vccio6-supply",
		"vccio7-supply",
	},
	.write = rk3568_iodomain_write,
};

static const struct udevice_id rockchip_iodomain_ids[] = {
	{
		.compatible = "rockchip,px30-io-voltage-domain",
		.data = (ulong)&soc_data_px30,
	},
	{
		.compatible = "rockchip,px30-pmu-io-voltage-domain",
		.data = (ulong)&soc_data_px30_pmu,
	},
	{
		.compatible = "rockchip,rk3328-io-voltage-domain",
		.data = (ulong)&soc_data_rk3328,
	},
	{
		.compatible = "rockchip,rk3399-io-voltage-domain",
		.data = (ulong)&soc_data_rk3399,
	},
	{
		.compatible = "rockchip,rk3399-pmu-io-voltage-domain",
		.data = (ulong)&soc_data_rk3399_pmu,
	},
	{
		.compatible = "rockchip,rk3568-pmu-io-voltage-domain",
		.data = (ulong)&soc_data_rk3568_pmu,
	},
	{ }
};

static int rockchip_iodomain_bind(struct udevice *dev)
{
	/*
	 * According to the Hardware Design Guide, IO-domain configuration must
	 * be consistent with the power supply voltage (1.8V or 3.3V).
	 * Probe after bind to configure IO-domain voltage early during boot.
	 */
	dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	return 0;
}

static int rockchip_iodomain_probe(struct udevice *dev)
{
	struct rockchip_iodomain_soc_data *soc_data =
		(struct rockchip_iodomain_soc_data *)dev_get_driver_data(dev);
	struct regmap *grf;
	int ret;

	grf = syscon_get_regmap(dev_get_parent(dev));
	if (IS_ERR(grf))
		return PTR_ERR(grf);

	for (int i = 0; i < MAX_SUPPLIES; i++) {
		const char *supply_name = soc_data->supply_names[i];
		struct udevice *reg;
		int uV;

		if (!supply_name)
			continue;

		ret = device_get_supply_regulator(dev, supply_name, &reg);
		if (ret)
			continue;

		ret = regulator_autoset(reg);
		if (ret && ret != -EALREADY && ret != -EMEDIUMTYPE &&
		    ret != -ENOSYS)
			continue;

		uV = regulator_get_value(reg);
		if (uV <= 0)
			continue;

		if (uV > MAX_VOLTAGE_3_3) {
			dev_crit(dev, "%s: %d uV is too high. May damage SoC!\n",
				 supply_name, uV);
			continue;
		}

		ret = soc_data->write(grf, soc_data->grf_offset, i, uV);
		if (ret)
			dev_err(dev, "%s: Couldn't write to GRF\n", supply_name);
	}

	return 0;
}

U_BOOT_DRIVER(rockchip_iodomain) = {
	.name = "rockchip_iodomain",
	.id = UCLASS_NOP,
	.of_match = rockchip_iodomain_ids,
	.bind = rockchip_iodomain_bind,
	.probe = rockchip_iodomain_probe,
};

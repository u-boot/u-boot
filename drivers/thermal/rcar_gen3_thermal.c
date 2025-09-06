// SPDX-License-Identifier: GPL-2.0
/*
 *  R-Car Gen3/Gen4 and RZ/G2 THS thermal sensor driver
 *  Based on rcar_thermal.c and work from Hien Dang and Khiem Nguyen.
 *
 * Copyright (C) 2016 Renesas Electronics Corporation.
 * Copyright (C) 2016 Sang Engineering
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/delay.h>
#include <thermal.h>

/* Register offsets */
#define REG_GEN3_IRQSTR		0x04
#define REG_GEN3_IRQMSK		0x08
#define REG_GEN3_IRQCTL		0x0c
#define REG_GEN3_IRQEN		0x10
#define REG_GEN3_IRQTEMP1	0x14
#define REG_GEN3_IRQTEMP2	0x18
#define REG_GEN3_IRQTEMP3	0x1c
#define REG_GEN3_THCTR		0x20
#define REG_GEN3_TEMP		0x28
#define REG_GEN3_THCODE1	0x50
#define REG_GEN3_THCODE2	0x54
#define REG_GEN3_THCODE3	0x58
#define REG_GEN3_PTAT1		0x5c
#define REG_GEN3_PTAT2		0x60
#define REG_GEN3_PTAT3		0x64
#define REG_GEN3_THSCP		0x68
#define REG_GEN4_THSFMON00	0x180
#define REG_GEN4_THSFMON01	0x184
#define REG_GEN4_THSFMON02	0x188
#define REG_GEN4_THSFMON15	0x1bc
#define REG_GEN4_THSFMON16	0x1c0
#define REG_GEN4_THSFMON17	0x1c4

/* IRQ{STR,MSK,EN} bits */
#define IRQ_TEMP1		BIT(0)
#define IRQ_TEMP2		BIT(1)
#define IRQ_TEMP3		BIT(2)
#define IRQ_TEMPD1		BIT(3)
#define IRQ_TEMPD2		BIT(4)
#define IRQ_TEMPD3		BIT(5)

/* THCTR bits */
#define THCTR_PONM	BIT(6)
#define THCTR_THSST	BIT(0)

/* THSCP bits */
#define THSCP_COR_PARA_VLD	(BIT(15) | BIT(14))

#define CTEMP_MASK	0xfff

#define MCELSIUS(temp)	((temp) * 1000)
#define GEN3_FUSE_MASK	0xfff
#define GEN4_FUSE_MASK	0xfff

#define TSC_MAX_NUM	5

struct rcar_gen3_thermal_priv;

struct rcar_gen3_thermal_fuse_info {
	u32 ptat[3];
	u32 thcode[3];
	u32 mask;
};

struct rcar_gen3_thermal_fuse_default {
	u32 ptat[3];
	u32 thcodes[TSC_MAX_NUM][3];
};

struct rcar_thermal_info {
	int scale;
	int adj_below;
	int adj_above;
	const struct rcar_gen3_thermal_fuse_info *fuses;
	const struct rcar_gen3_thermal_fuse_default *fuse_defaults;
};

struct equation_set_coef {
	int a;
	int b;
};

struct rcar_gen3_thermal_tsc {
	void __iomem *base;
	/* Different coefficients are used depending on a threshold. */
	struct {
		struct equation_set_coef below;
		struct equation_set_coef above;
	} coef;
	int thcode[3];
};

struct rcar_gen3_thermal_priv {
	struct rcar_gen3_thermal_tsc tscs[TSC_MAX_NUM];
	unsigned int num_tscs;
	int ptat[3];
};

static inline u32 rcar_gen3_thermal_read(struct rcar_gen3_thermal_tsc *tsc,
					 u32 reg)
{
	return readl(tsc->base + reg);
}

static inline void rcar_gen3_thermal_write(struct rcar_gen3_thermal_tsc *tsc,
					   u32 reg, u32 data)
{
	writel(data, tsc->base + reg);
}

/*
 * Linear approximation for temperature
 *
 * [temp] = ((thadj - [reg]) * a) / b + adj
 * [reg] = thadj - ([temp] - adj) * b / a
 *
 * The constants a and b are calculated using two triplets of int values PTAT
 * and THCODE. PTAT and THCODE can either be read from hardware or use hard
 * coded values from the driver. The formula to calculate a and b are taken from
 * the datasheet. Different calculations are needed for a and b depending on
 * if the input variables ([temp] or [reg]) are above or below a threshold. The
 * threshold is also calculated from PTAT and THCODE using formulas from the
 * datasheet.
 *
 * The constant thadj is one of the THCODE values, which one to use depends on
 * the threshold and input value.
 *
 * The constants adj is taken verbatim from the datasheet. Two values exists,
 * which one to use depends on the input value and the calculated threshold.
 * Furthermore different SoC models supported by the driver have different sets
 * of values. The values for each model are stored in the device match data.
 */
static void rcar_gen3_thermal_tsc_coefs(struct udevice *dev,
					struct rcar_gen3_thermal_tsc *tsc)
{
	struct rcar_thermal_info *info = (struct rcar_thermal_info *)dev_get_driver_data(dev);
	struct rcar_gen3_thermal_priv *priv = dev_get_plat(dev);

	tsc->coef.below.a = info->scale * (priv->ptat[2] - priv->ptat[1]);
	tsc->coef.above.a = info->scale * (priv->ptat[0] - priv->ptat[1]);

	tsc->coef.below.b = (priv->ptat[2] - priv->ptat[0]) * (tsc->thcode[2] - tsc->thcode[1]);
	tsc->coef.above.b = (priv->ptat[0] - priv->ptat[2]) * (tsc->thcode[1] - tsc->thcode[0]);
}

static int rcar_gen3_thermal_get_temp(struct udevice *dev, int *temp)
{
	struct rcar_thermal_info *info = (struct rcar_thermal_info *)dev_get_driver_data(dev->parent);
	struct rcar_gen3_thermal_priv *priv = dev_get_plat(dev->parent);
	unsigned int tsc_id = dev_get_driver_data(dev);
	struct rcar_gen3_thermal_tsc *tsc = &(priv->tscs[tsc_id]);
	const struct equation_set_coef *coef;
	int adj, decicelsius, reg, thcode;

	/* Read register and convert to millidegree Celsius */
	reg = rcar_gen3_thermal_read(tsc, REG_GEN3_TEMP) & CTEMP_MASK;

	if (reg < tsc->thcode[1]) {
		adj = info->adj_below;
		coef = &tsc->coef.below;
		thcode = tsc->thcode[2];
	} else {
		adj = info->adj_above;
		coef = &tsc->coef.above;
		thcode = tsc->thcode[0];
	}

	/*
	 * The dividend can't be grown as it might overflow, instead shorten the
	 * divisor to convert to decidegree Celsius. If we convert after the
	 * division precision is lost as we will scale up from whole degrees
	 * Celsius.
	 */
	decicelsius = DIV_ROUND_CLOSEST(coef->a * (thcode - reg), coef->b / 10);

	/* Guaranteed operating range is -40C to 125C. */

	/* Reporting is done in millidegree Celsius */
	*temp = decicelsius * 100 + adj * 1000;

	return 0;
}

static const struct dm_thermal_ops rcar_gen3_thermal_ops = {
	.get_temp	= rcar_gen3_thermal_get_temp,
};

static void rcar_gen3_thermal_fetch_fuses(struct udevice *dev)
{
	struct rcar_thermal_info *info = (struct rcar_thermal_info *)dev_get_driver_data(dev);
	struct rcar_gen3_thermal_priv *priv = dev_get_plat(dev);
	const struct rcar_gen3_thermal_fuse_info *fuses = info->fuses;

	/*
	 * Set the pseudo calibration points with fused values.
	 * PTAT is shared between all TSCs but only fused for the first
	 * TSC while THCODEs are fused for each TSC.
	 */
	priv->ptat[0] = rcar_gen3_thermal_read(&(priv->tscs[0]), fuses->ptat[0])
		& fuses->mask;
	priv->ptat[1] = rcar_gen3_thermal_read(&(priv->tscs[0]), fuses->ptat[1])
		& fuses->mask;
	priv->ptat[2] = rcar_gen3_thermal_read(&(priv->tscs[0]), fuses->ptat[2])
		& fuses->mask;

	for (unsigned int i = 0; i < priv->num_tscs; i++) {
		struct rcar_gen3_thermal_tsc *tsc = &(priv->tscs[i]);

		tsc->thcode[0] = rcar_gen3_thermal_read(tsc, fuses->thcode[0])
			& fuses->mask;
		tsc->thcode[1] = rcar_gen3_thermal_read(tsc, fuses->thcode[1])
			& fuses->mask;
		tsc->thcode[2] = rcar_gen3_thermal_read(tsc, fuses->thcode[2])
			& fuses->mask;
	}
}

static bool rcar_gen3_thermal_read_fuses(struct udevice *dev)
{
	struct rcar_thermal_info *info = (struct rcar_thermal_info *)dev_get_driver_data(dev);
	struct rcar_gen3_thermal_priv *priv = dev_get_plat(dev);
	const struct rcar_gen3_thermal_fuse_default *fuse_defaults = info->fuse_defaults;
	unsigned int i;
	u32 thscp;

	/* If fuses are not set, fallback to pseudo values. */
	thscp = rcar_gen3_thermal_read(&(priv->tscs[0]), REG_GEN3_THSCP);
	if (!info->fuses ||
	    (thscp & THSCP_COR_PARA_VLD) != THSCP_COR_PARA_VLD) {
		/* Default THCODE values in case FUSEs are not set. */
		priv->ptat[0] = fuse_defaults->ptat[0];
		priv->ptat[1] = fuse_defaults->ptat[1];
		priv->ptat[2] = fuse_defaults->ptat[2];

		for (i = 0; i < priv->num_tscs; i++) {
			struct rcar_gen3_thermal_tsc *tsc = &(priv->tscs[i]);

			tsc->thcode[0] = fuse_defaults->thcodes[i][0];
			tsc->thcode[1] = fuse_defaults->thcodes[i][1];
			tsc->thcode[2] = fuse_defaults->thcodes[i][2];
		}

		return false;
	}

	rcar_gen3_thermal_fetch_fuses(dev);

	return true;
}

static void rcar_gen3_thermal_init(struct rcar_gen3_thermal_priv *priv,
				   struct rcar_gen3_thermal_tsc *tsc)
{
	u32 reg_val;

	reg_val = rcar_gen3_thermal_read(tsc, REG_GEN3_THCTR);
	reg_val &= ~THCTR_PONM;
	rcar_gen3_thermal_write(tsc, REG_GEN3_THCTR, reg_val);

	udelay(1000);

	rcar_gen3_thermal_write(tsc, REG_GEN3_IRQCTL, 0);
	rcar_gen3_thermal_write(tsc, REG_GEN3_IRQMSK, 0);

	reg_val = rcar_gen3_thermal_read(tsc, REG_GEN3_THCTR);
	reg_val |= THCTR_THSST;
	rcar_gen3_thermal_write(tsc, REG_GEN3_THCTR, reg_val);

	udelay(1000);
}

static int rcar_gen3_thermal_probe(struct udevice *dev)
{
	struct rcar_gen3_thermal_priv *priv = dev_get_plat(dev);
	unsigned int i;

	if (!rcar_gen3_thermal_read_fuses(dev))
		dev_dbg(dev, "No calibration values fused, fallback to driver values\n");

	for (i = 0; i < priv->num_tscs; i++) {
		struct rcar_gen3_thermal_tsc *tsc = &(priv->tscs[i]);

		rcar_gen3_thermal_init(priv, tsc);
		rcar_gen3_thermal_tsc_coefs(dev, tsc);
	}

	return 0;
}

static const struct rcar_gen3_thermal_fuse_info rcar_gen3_thermal_fuse_info_gen3 = {
	.ptat = { REG_GEN3_PTAT1, REG_GEN3_PTAT2, REG_GEN3_PTAT3 },
	.thcode = { REG_GEN3_THCODE1, REG_GEN3_THCODE2, REG_GEN3_THCODE3 },
	.mask = GEN3_FUSE_MASK,
};

static const struct rcar_gen3_thermal_fuse_info rcar_gen3_thermal_fuse_info_gen4 = {
	.ptat = { REG_GEN4_THSFMON16, REG_GEN4_THSFMON17, REG_GEN4_THSFMON15 },
	.thcode = { REG_GEN4_THSFMON01, REG_GEN4_THSFMON02, REG_GEN4_THSFMON00 },
	.mask = GEN4_FUSE_MASK,
};

static const struct rcar_gen3_thermal_fuse_default rcar_gen3_thermal_fuse_default_info_gen3 = {
	.ptat = { 2631, 1509, 435 },
	.thcodes = {
		{ 3397, 2800, 2221 },
		{ 3393, 2795, 2216 },
		{ 3389, 2805, 2237 },
		{ 3415, 2694, 2195 },
		{ 3356, 2724, 2244 },
	},
};

static const struct rcar_gen3_thermal_fuse_default rcar_gen3_thermal_fuse_default_info_v4h = {
	.ptat = { 3274, 2164, 985 },
	.thcodes = { /* All four THS units share the same trimming */
		{ 3218, 2617, 1980 },
		{ 3218, 2617, 1980 },
		{ 3218, 2617, 1980 },
		{ 3218, 2617, 1980 },
	}
};

static const struct rcar_thermal_info rcar_m3w_thermal_info = {
	.scale = 157,
	.adj_below = -41,
	.adj_above = 116,
	.fuses = &rcar_gen3_thermal_fuse_info_gen3,
	.fuse_defaults = &rcar_gen3_thermal_fuse_default_info_gen3,
};

static const struct rcar_thermal_info rcar_gen3_thermal_info = {
	.scale = 167,
	.adj_below = -41,
	.adj_above = 126,
	.fuses = &rcar_gen3_thermal_fuse_info_gen3,
	.fuse_defaults = &rcar_gen3_thermal_fuse_default_info_gen3,
};

static const struct rcar_thermal_info rcar_gen4_thermal_info = {
	.scale = 167,
	.adj_below = -41,
	.adj_above = 126,
	.fuses = &rcar_gen3_thermal_fuse_info_gen4,
	.fuse_defaults = &rcar_gen3_thermal_fuse_default_info_gen3,
};

static const struct rcar_thermal_info rcar_v4h_thermal_info = {
	.scale = 167,
	.adj_below = -41,
	.adj_above = 126,
	.fuses = &rcar_gen3_thermal_fuse_info_gen4,
	.fuse_defaults = &rcar_gen3_thermal_fuse_default_info_v4h,
};

static const struct udevice_id rcar_gen3_thermal_ids[] = {
	{
		.compatible = "renesas,r8a774a1-thermal",
		.data = (ulong)&rcar_m3w_thermal_info,
	},
	{
		.compatible = "renesas,r8a774b1-thermal",
		.data = (ulong)&rcar_gen3_thermal_info,
	},
	{
		.compatible = "renesas,r8a774e1-thermal",
		.data = (ulong)&rcar_gen3_thermal_info,
	},
	{
		.compatible = "renesas,r8a7795-thermal",
		.data = (ulong)&rcar_gen3_thermal_info,
	},
	{
		.compatible = "renesas,r8a7796-thermal",
		.data = (ulong)&rcar_m3w_thermal_info,
	},
	{
		.compatible = "renesas,r8a77961-thermal",
		.data = (ulong)&rcar_m3w_thermal_info,
	},
	{
		.compatible = "renesas,r8a77965-thermal",
		.data = (ulong)&rcar_gen3_thermal_info,
	},
	{
		.compatible = "renesas,r8a77980-thermal",
		.data = (ulong)&rcar_gen3_thermal_info,
	},
	{
		.compatible = "renesas,r8a779a0-thermal",
		.data = (ulong)&rcar_gen3_thermal_info,
	},
	{
		.compatible = "renesas,r8a779f0-thermal",
		.data = (ulong)&rcar_gen4_thermal_info,
	},
	{
		.compatible = "renesas,r8a779g0-thermal",
		.data = (ulong)&rcar_v4h_thermal_info,
	},
	{
		.compatible = "renesas,r8a779h0-thermal",
		.data = (ulong)&rcar_gen4_thermal_info,
	},
	{ }
};

U_BOOT_DRIVER(thermal_rcar_gen3_tsc) = {
	.name		= "thermal-rcar-gen3-tsc",
	.id		= UCLASS_THERMAL,
	.ops		= &rcar_gen3_thermal_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static int rcar_gen3_thermal_bind(struct udevice *dev)
{
	/*
	 * We use dev_get_plat() here, because plat data are available
	 * in bind, while private data are not allocated yet.
	 */
	struct rcar_gen3_thermal_priv *priv = dev_get_plat(dev);
	struct udevice *tdev;
	struct driver *tdrv;
	char name[32];
	void *addr;
	int i, ret;

	tdrv = lists_driver_lookup_name("thermal-rcar-gen3-tsc");
	if (!tdrv)
		return -ENOENT;

	for (i = 0; i < TSC_MAX_NUM; i++) {
		addr = dev_read_addr_index_ptr(dev, i);
		if (!addr)
			break;

		priv->tscs[i].base = addr;

		tdev = NULL;
		snprintf(name, sizeof(name), "thermal-rcar-gen3-tsc.%d", i);
		ret = device_bind_with_driver_data(dev, tdrv, strdup(name), i,
						   dev_ofnode(dev), &tdev);
		if (ret)
			return ret;
	}

	priv->num_tscs = i;

	return 0;
}

U_BOOT_DRIVER(thermal_rcar_gen3) = {
	.name		= "thermal-rcar-gen3",
	.id		= UCLASS_NOP,
	.of_match	= rcar_gen3_thermal_ids,
	.bind		= rcar_gen3_thermal_bind,
	.probe		= rcar_gen3_thermal_probe,
	.plat_auto	= sizeof(struct rcar_gen3_thermal_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};

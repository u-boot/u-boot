/*
 * Renesas RCar Gen3 CPG MSSR driver
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on the following driver from Linux kernel:
 * r8a7796 Clock Pulse Generator / Module Standby and Software Reset
 *
 * Copyright (C) 2016 Glider bvba
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <wait_bit.h>
#include <asm/io.h>

#include <dt-bindings/clock/renesas-cpg-mssr.h>

#include "renesas-cpg-mssr.h"

#define CPG_RST_MODEMR		0x0060

#define CPG_PLL0CR		0x00d8
#define CPG_PLL2CR		0x002c
#define CPG_PLL4CR		0x01f4

#define CPG_RPC_PREDIV_MASK	0x3
#define CPG_RPC_PREDIV_OFFSET	3
#define CPG_RPC_POSTDIV_MASK	0x7
#define CPG_RPC_POSTDIV_OFFSET	0

/*
 * Module Standby and Software Reset register offets.
 *
 * If the registers exist, these are valid for SH-Mobile, R-Mobile,
 * R-Car Gen2, R-Car Gen3, and RZ/G1.
 * These are NOT valid for R-Car Gen1 and RZ/A1!
 */

/*
 * Module Stop Status Register offsets
 */

static const u16 mstpsr[] = {
	0x030, 0x038, 0x040, 0x048, 0x04C, 0x03C, 0x1C0, 0x1C4,
	0x9A0, 0x9A4, 0x9A8, 0x9AC,
};

#define	MSTPSR(i)	mstpsr[i]


/*
 * System Module Stop Control Register offsets
 */

static const u16 smstpcr[] = {
	0x130, 0x134, 0x138, 0x13C, 0x140, 0x144, 0x148, 0x14C,
	0x990, 0x994, 0x998, 0x99C,
};

#define	SMSTPCR(i)	smstpcr[i]


/* Realtime Module Stop Control Register offsets */
#define RMSTPCR(i)	(smstpcr[i] - 0x20)

/* Modem Module Stop Control Register offsets (r8a73a4) */
#define MMSTPCR(i)	(smstpcr[i] + 0x20)

/* Software Reset Clearing Register offsets */
#define	SRSTCLR(i)	(0x940 + (i) * 4)

/*
 * CPG Clock Data
 */

/*
 *   MD		EXTAL		PLL0	PLL1	PLL2	PLL3	PLL4
 * 14 13 19 17	(MHz)
 *-------------------------------------------------------------------
 * 0  0  0  0	16.66 x 1	x180	x192	x144	x192	x144
 * 0  0  0  1	16.66 x 1	x180	x192	x144	x128	x144
 * 0  0  1  0	Prohibited setting
 * 0  0  1  1	16.66 x 1	x180	x192	x144	x192	x144
 * 0  1  0  0	20    x 1	x150	x160	x120	x160	x120
 * 0  1  0  1	20    x 1	x150	x160	x120	x106	x120
 * 0  1  1  0	Prohibited setting
 * 0  1  1  1	20    x 1	x150	x160	x120	x160	x120
 * 1  0  0  0	25    x 1	x120	x128	x96	x128	x96
 * 1  0  0  1	25    x 1	x120	x128	x96	x84	x96
 * 1  0  1  0	Prohibited setting
 * 1  0  1  1	25    x 1	x120	x128	x96	x128	x96
 * 1  1  0  0	33.33 / 2	x180	x192	x144	x192	x144
 * 1  1  0  1	33.33 / 2	x180	x192	x144	x128	x144
 * 1  1  1  0	Prohibited setting
 * 1  1  1  1	33.33 / 2	x180	x192	x144	x192	x144
 */
#define CPG_PLL_CONFIG_INDEX(md)	((((md) & BIT(14)) >> 11) | \
					 (((md) & BIT(13)) >> 11) | \
					 (((md) & BIT(19)) >> 18) | \
					 (((md) & BIT(17)) >> 17))

static const struct rcar_gen3_cpg_pll_config cpg_pll_configs[16] = {
	/* EXTAL div	PLL1 mult	PLL3 mult */
	{ 1,		192,		192,	},
	{ 1,		192,		128,	},
	{ 0, /* Prohibited setting */		},
	{ 1,		192,		192,	},
	{ 1,		160,		160,	},
	{ 1,		160,		106,	},
	{ 0, /* Prohibited setting */		},
	{ 1,		160,		160,	},
	{ 1,		128,		128,	},
	{ 1,		128,		84,	},
	{ 0, /* Prohibited setting */		},
	{ 1,		128,		128,	},
	{ 2,		192,		192,	},
	{ 2,		192,		128,	},
	{ 0, /* Prohibited setting */		},
	{ 2,		192,		192,	},
};

/*
 * SDn Clock
 */
#define CPG_SD_STP_HCK		BIT(9)
#define CPG_SD_STP_CK		BIT(8)

#define CPG_SD_STP_MASK		(CPG_SD_STP_HCK | CPG_SD_STP_CK)
#define CPG_SD_FC_MASK		(0x7 << 2 | 0x3 << 0)

#define CPG_SD_DIV_TABLE_DATA(stp_hck, stp_ck, sd_srcfc, sd_fc, sd_div) \
{ \
	.val = ((stp_hck) ? CPG_SD_STP_HCK : 0) | \
	       ((stp_ck) ? CPG_SD_STP_CK : 0) | \
	       ((sd_srcfc) << 2) | \
	       ((sd_fc) << 0), \
	.div = (sd_div), \
}

struct sd_div_table {
	u32 val;
	unsigned int div;
};

/* SDn divider
 *                     sd_srcfc   sd_fc   div
 * stp_hck   stp_ck    (div)      (div)     = sd_srcfc x sd_fc
 *-------------------------------------------------------------------
 *  0         0         0 (1)      1 (4)      4
 *  0         0         1 (2)      1 (4)      8
 *  1         0         2 (4)      1 (4)     16
 *  1         0         3 (8)      1 (4)     32
 *  1         0         4 (16)     1 (4)     64
 *  0         0         0 (1)      0 (2)      2
 *  0         0         1 (2)      0 (2)      4
 *  1         0         2 (4)      0 (2)      8
 *  1         0         3 (8)      0 (2)     16
 *  1         0         4 (16)     0 (2)     32
 */
static const struct sd_div_table cpg_sd_div_table[] = {
/*	CPG_SD_DIV_TABLE_DATA(stp_hck,  stp_ck,   sd_srcfc,   sd_fc,  sd_div) */
	CPG_SD_DIV_TABLE_DATA(0,        0,        0,          1,        4),
	CPG_SD_DIV_TABLE_DATA(0,        0,        1,          1,        8),
	CPG_SD_DIV_TABLE_DATA(1,        0,        2,          1,       16),
	CPG_SD_DIV_TABLE_DATA(1,        0,        3,          1,       32),
	CPG_SD_DIV_TABLE_DATA(1,        0,        4,          1,       64),
	CPG_SD_DIV_TABLE_DATA(0,        0,        0,          0,        2),
	CPG_SD_DIV_TABLE_DATA(0,        0,        1,          0,        4),
	CPG_SD_DIV_TABLE_DATA(1,        0,        2,          0,        8),
	CPG_SD_DIV_TABLE_DATA(1,        0,        3,          0,       16),
	CPG_SD_DIV_TABLE_DATA(1,        0,        4,          0,       32),
};

static bool gen3_clk_is_mod(struct clk *clk)
{
	return (clk->id >> 16) == CPG_MOD;
}

static int gen3_clk_get_mod(struct clk *clk, const struct mssr_mod_clk **mssr)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	struct cpg_mssr_info *info = priv->info;
	const unsigned long clkid = clk->id & 0xffff;
	int i;

	if (!gen3_clk_is_mod(clk))
		return -EINVAL;

	for (i = 0; i < info->mod_clk_size; i++) {
		if (info->mod_clk[i].id != MOD_CLK_ID(clkid))
			continue;

		*mssr = &info->mod_clk[i];
		return 0;
	}

	return -ENODEV;
}

static int gen3_clk_get_core(struct clk *clk, const struct cpg_core_clk **core)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	struct cpg_mssr_info *info = priv->info;
	const unsigned long clkid = clk->id & 0xffff;
	int i;

	if (gen3_clk_is_mod(clk))
		return -EINVAL;

	for (i = 0; i < info->core_clk_size; i++) {
		if (info->core_clk[i].id != clkid)
			continue;

		*core = &info->core_clk[i];
		return 0;
	}

	return -ENODEV;
}

static int gen3_clk_get_parent(struct clk *clk, struct clk *parent)
{
	const struct cpg_core_clk *core;
	const struct mssr_mod_clk *mssr;
	int ret;

	if (gen3_clk_is_mod(clk)) {
		ret = gen3_clk_get_mod(clk, &mssr);
		if (ret)
			return ret;

		parent->id = mssr->parent;
	} else {
		ret = gen3_clk_get_core(clk, &core);
		if (ret)
			return ret;

		if (core->type == CLK_TYPE_IN)
			parent->id = ~0;	/* Top-level clock */
		else
			parent->id = core->parent;
	}

	parent->dev = clk->dev;

	return 0;
}

static int gen3_clk_setup_sdif_div(struct clk *clk)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	const struct cpg_core_clk *core;
	struct clk parent;
	int ret;

	ret = gen3_clk_get_parent(clk, &parent);
	if (ret) {
		printf("%s[%i] parent fail, ret=%i\n", __func__, __LINE__, ret);
		return ret;
	}

	if (gen3_clk_is_mod(&parent))
		return 0;

	ret = gen3_clk_get_core(&parent, &core);
	if (ret)
		return ret;

	if (core->type != CLK_TYPE_GEN3_SD)
		return 0;

	debug("%s[%i] SDIF offset=%x\n", __func__, __LINE__, core->offset);

	writel(1, priv->base + core->offset);

	return 0;
}

static int gen3_clk_endisable(struct clk *clk, bool enable)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	const unsigned long clkid = clk->id & 0xffff;
	const unsigned int reg = clkid / 100;
	const unsigned int bit = clkid % 100;
	const u32 bitmask = BIT(bit);
	int ret;

	if (!gen3_clk_is_mod(clk))
		return -EINVAL;

	debug("%s[%i] MSTP %lu=%02u/%02u %s\n", __func__, __LINE__,
	      clkid, reg, bit, enable ? "ON" : "OFF");

	if (enable) {
		ret = gen3_clk_setup_sdif_div(clk);
		if (ret)
			return ret;
		clrbits_le32(priv->base + SMSTPCR(reg), bitmask);
		return wait_for_bit("MSTP", priv->base + MSTPSR(reg),
				    bitmask, 0, 100, 0);
	} else {
		setbits_le32(priv->base + SMSTPCR(reg), bitmask);
		return 0;
	}
}

static int gen3_clk_enable(struct clk *clk)
{
	return gen3_clk_endisable(clk, true);
}

static int gen3_clk_disable(struct clk *clk)
{
	return gen3_clk_endisable(clk, false);
}

static ulong gen3_clk_get_rate(struct clk *clk)
{
	struct gen3_clk_priv *priv = dev_get_priv(clk->dev);
	struct clk parent;
	const struct cpg_core_clk *core;
	const struct rcar_gen3_cpg_pll_config *pll_config =
					priv->cpg_pll_config;
	u32 value, mult, prediv, postdiv, rate = 0;
	int i, ret;

	debug("%s[%i] Clock: id=%lu\n", __func__, __LINE__, clk->id);

	ret = gen3_clk_get_parent(clk, &parent);
	if (ret) {
		printf("%s[%i] parent fail, ret=%i\n", __func__, __LINE__, ret);
		return ret;
	}

	if (gen3_clk_is_mod(clk)) {
		rate = gen3_clk_get_rate(&parent);
		debug("%s[%i] MOD clk: parent=%lu => rate=%u\n",
		      __func__, __LINE__, parent.id, rate);
		return rate;
	}

	ret = gen3_clk_get_core(clk, &core);
	if (ret)
		return ret;

	switch (core->type) {
	case CLK_TYPE_IN:
		if (core->id == CLK_EXTAL) {
			rate = clk_get_rate(&priv->clk_extal);
			debug("%s[%i] EXTAL clk: rate=%u\n",
			      __func__, __LINE__, rate);
			return rate;
		}

		if (core->id == CLK_EXTALR) {
			rate = clk_get_rate(&priv->clk_extalr);
			debug("%s[%i] EXTALR clk: rate=%u\n",
			      __func__, __LINE__, rate);
			return rate;
		}

		return -EINVAL;

	case CLK_TYPE_GEN3_MAIN:
		rate = gen3_clk_get_rate(&parent) / pll_config->extal_div;
		debug("%s[%i] MAIN clk: parent=%i extal_div=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->extal_div, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL0:
		value = readl(priv->base + CPG_PLL0CR);
		mult = (((value >> 24) & 0x7f) + 1) * 2;
		rate = gen3_clk_get_rate(&parent) * mult;
		debug("%s[%i] PLL0 clk: parent=%i mult=%u => rate=%u\n",
		      __func__, __LINE__, core->parent, mult, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL1:
		rate = gen3_clk_get_rate(&parent) * pll_config->pll1_mult;
		debug("%s[%i] PLL1 clk: parent=%i mul=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->pll1_mult, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL2:
		value = readl(priv->base + CPG_PLL2CR);
		mult = (((value >> 24) & 0x7f) + 1) * 2;
		rate = gen3_clk_get_rate(&parent) * mult;
		debug("%s[%i] PLL2 clk: parent=%i mult=%u => rate=%u\n",
		      __func__, __LINE__, core->parent, mult, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL3:
		rate = gen3_clk_get_rate(&parent) * pll_config->pll3_mult;
		debug("%s[%i] PLL3 clk: parent=%i mul=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, pll_config->pll3_mult, rate);
		return rate;

	case CLK_TYPE_GEN3_PLL4:
		value = readl(priv->base + CPG_PLL4CR);
		mult = (((value >> 24) & 0x7f) + 1) * 2;
		rate = gen3_clk_get_rate(&parent) * mult;
		debug("%s[%i] PLL4 clk: parent=%i mult=%u => rate=%u\n",
		      __func__, __LINE__, core->parent, mult, rate);
		return rate;

	case CLK_TYPE_FF:
	case CLK_TYPE_GEN3_PE:		/* FIXME */
		rate = (gen3_clk_get_rate(&parent) * core->mult) / core->div;
		debug("%s[%i] FIXED clk: parent=%i div=%i mul=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, core->mult, core->div, rate);
		return rate;

	case CLK_TYPE_GEN3_SD:		/* FIXME */
		value = readl(priv->base + core->offset);
		value &= CPG_SD_STP_MASK | CPG_SD_FC_MASK;

		for (i = 0; i < ARRAY_SIZE(cpg_sd_div_table); i++) {
			if (cpg_sd_div_table[i].val != value)
				continue;

			rate = gen3_clk_get_rate(&parent) /
			       cpg_sd_div_table[i].div;
			debug("%s[%i] SD clk: parent=%i div=%i => rate=%u\n",
			      __func__, __LINE__,
			      core->parent, cpg_sd_div_table[i].div, rate);

			return rate;
		}

		return -EINVAL;

	case CLK_TYPE_GEN3_RPC:
		rate = gen3_clk_get_rate(&parent);

		value = readl(priv->base + core->offset);

		prediv = (value >> CPG_RPC_PREDIV_OFFSET) &
			 CPG_RPC_PREDIV_MASK;
		if (prediv == 2)
			rate /= 5;
		else if (prediv == 3)
			rate /= 6;
		else
			return -EINVAL;

		postdiv = (value >> CPG_RPC_POSTDIV_OFFSET) &
			  CPG_RPC_POSTDIV_MASK;
		rate /= postdiv + 1;

		debug("%s[%i] RPC clk: parent=%i prediv=%i postdiv=%i => rate=%u\n",
		      __func__, __LINE__,
		      core->parent, prediv, postdiv, rate);

		return -EINVAL;

	}

	printf("%s[%i] unknown fail\n", __func__, __LINE__);

	return -ENOENT;
}

static ulong gen3_clk_set_rate(struct clk *clk, ulong rate)
{
	return gen3_clk_get_rate(clk);
}

static int gen3_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count != 2) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	clk->id = (args->args[0] << 16) | args->args[1];

	return 0;
}

const struct clk_ops gen3_clk_ops = {
	.enable		= gen3_clk_enable,
	.disable	= gen3_clk_disable,
	.get_rate	= gen3_clk_get_rate,
	.set_rate	= gen3_clk_set_rate,
	.of_xlate	= gen3_clk_of_xlate,
};

int gen3_clk_probe(struct udevice *dev)
{
	struct gen3_clk_priv *priv = dev_get_priv(dev);
	struct cpg_mssr_info *info =
		(struct cpg_mssr_info *)dev_get_driver_data(dev);
	fdt_addr_t rst_base;
	u32 cpg_mode;
	int ret;

	priv->base = (struct gen3_base *)devfdt_get_addr(dev);
	if (!priv->base)
		return -EINVAL;

	priv->info = info;
	ret = fdt_node_offset_by_compatible(gd->fdt_blob, -1, info->reset_node);
	if (ret < 0)
		return ret;

	rst_base = fdtdec_get_addr(gd->fdt_blob, ret, "reg");
	if (rst_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	cpg_mode = readl(rst_base + CPG_RST_MODEMR);

	priv->cpg_pll_config = &cpg_pll_configs[CPG_PLL_CONFIG_INDEX(cpg_mode)];
	if (!priv->cpg_pll_config->extal_div)
		return -EINVAL;

	ret = clk_get_by_name(dev, "extal", &priv->clk_extal);
	if (ret < 0)
		return ret;

	if (info->extalr_node) {
		ret = clk_get_by_name(dev, info->extalr_node, &priv->clk_extalr);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int gen3_clk_remove(struct udevice *dev)
{
	struct gen3_clk_priv *priv = dev_get_priv(dev);
	struct cpg_mssr_info *info = priv->info;
	unsigned int i;

	/* Stop TMU0 */
	clrbits_le32(TMU_BASE + TSTR0, TSTR0_STR0);

	/* Stop module clock */
	for (i = 0; i < info->mstp_table_size; i++) {
		clrsetbits_le32(priv->base + SMSTPCR(i),
				info->mstp_table[i].dis,
				info->mstp_table[i].en);
		clrsetbits_le32(priv->base + RMSTPCR(i),
				info->mstp_table[i].dis, 0x0);
	}

	return 0;
}

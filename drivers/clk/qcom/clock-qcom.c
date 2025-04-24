// SPDX-License-Identifier: BSD-3-Clause AND GPL-2.0
/*
 * Clock and reset drivers for Qualcomm platforms Global Clock
 * Controller (GCC).
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 * (C) Copyright 2020 Sartura Ltd. (reset driver)
 *     Author: Robert Marko <robert.marko@sartura.hr>
 * (C) Copyright 2022 Linaro Ltd. (reset driver)
 *     Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * Based on Little Kernel driver, simplified
 */

#include <clk-uclass.h>
#include <linux/clk-provider.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <reset-uclass.h>
#include <power-domain-uclass.h>

#include "clock-qcom.h"

/* CBCR register fields */
#define CBCR_BRANCH_ENABLE_BIT  BIT(0)
#define CBCR_BRANCH_OFF_BIT     BIT(31)

#define GDSC_SW_COLLAPSE_MASK		BIT(0)
#define GDSC_POWER_DOWN_COMPLETE	BIT(15)
#define GDSC_POWER_UP_COMPLETE		BIT(16)
#define GDSC_PWR_ON_MASK		BIT(31)
#define CFG_GDSCR_OFFSET		0x4
#define GDSC_STATUS_POLL_TIMEOUT_US	1500

/* Enable clock controlled by CBC soft macro */
void clk_enable_cbc(phys_addr_t cbcr)
{
	setbits_le32(cbcr, CBCR_BRANCH_ENABLE_BIT);

	while (readl(cbcr) & CBCR_BRANCH_OFF_BIT)
		;
}

void clk_enable_gpll0(phys_addr_t base, const struct pll_vote_clk *gpll0)
{
	if (readl(base + gpll0->status) & gpll0->status_bit)
		return; /* clock already enabled */

	setbits_le32(base + gpll0->ena_vote, gpll0->vote_bit);

	while ((readl(base + gpll0->status) & gpll0->status_bit) == 0)
		;
}

#define BRANCH_ON_VAL (0)
#define BRANCH_NOC_FSM_ON_VAL BIT(29)
#define BRANCH_CHECK_MASK GENMASK(31, 28)

void clk_enable_vote_clk(phys_addr_t base, const struct vote_clk *vclk)
{
	u32 val;

	setbits_le32(base + vclk->ena_vote, vclk->vote_bit);
	do {
		val = readl(base + vclk->cbcr_reg);
		val &= BRANCH_CHECK_MASK;
	} while ((val != BRANCH_ON_VAL) && (val != BRANCH_NOC_FSM_ON_VAL));
}

int qcom_gate_clk_en(const struct msm_clk_priv *priv, unsigned long id)
{
	if (id >= priv->data->num_clks || priv->data->clks[id].reg == 0) {
		log_err("gcc@%#08llx: unknown clock ID %lu!\n",
			priv->base, id);
		return -ENOENT;
	}

	setbits_le32(priv->base + priv->data->clks[id].reg, priv->data->clks[id].en_val);
	if (priv->data->clks[id].cbcr_reg) {
		unsigned int count;
		u32 val;

		for (count = 0; count < 200; count++) {
			val = readl(priv->base + priv->data->clks[id].cbcr_reg);
			val &= BRANCH_CHECK_MASK;
			if (val == BRANCH_ON_VAL || val == BRANCH_NOC_FSM_ON_VAL)
				break;
			udelay(1);
		}
		if (WARN(count == 200, "WARNING: Clock @ %#lx [%#010x] stuck at off\n",
			 priv->data->clks[id].cbcr_reg, val))
			return -EBUSY;
	}
	return 0;
}

#define APPS_CMD_RCGR_UPDATE BIT(0)

/* Update clock command via CMD_RCGR */
void clk_bcr_update(phys_addr_t apps_cmd_rcgr)
{
	u32 count;
	setbits_le32(apps_cmd_rcgr, APPS_CMD_RCGR_UPDATE);

	/* Wait for frequency to be updated. */
	for (count = 0; count < 50000; count++) {
		if (!(readl(apps_cmd_rcgr) & APPS_CMD_RCGR_UPDATE))
			break;
		udelay(1);
	}
	WARN(count == 50000, "WARNING: RCG @ %#llx [%#010x] stuck at off\n",
	     apps_cmd_rcgr, readl(apps_cmd_rcgr));
}

#define CFG_SRC_DIV_MASK	0b11111
#define CFG_SRC_SEL_SHIFT	8
#define CFG_SRC_SEL_MASK	(0x7 << CFG_SRC_SEL_SHIFT)
#define CFG_MODE_SHIFT		12
#define CFG_MODE_MASK		(0x3 << CFG_MODE_SHIFT)
#define CFG_MODE_DUAL_EDGE	(0x2 << CFG_MODE_SHIFT)
#define CFG_HW_CLK_CTRL_MASK	BIT(20)

/*
 * root set rate for clocks with half integer and MND divider
 * div should be pre-calculated ((div * 2) - 1)
 */
void clk_rcg_set_rate_mnd(phys_addr_t base, uint32_t cmd_rcgr,
			  int div, int m, int n, int source, u8 mnd_width)
{
	u32 cfg;
	/* M value for MND divider. */
	u32 m_val = m;
	u32 n_minus_m = n - m;
	/* NOT(N-M) value for MND divider. */
	u32 n_val = ~n_minus_m * !!(n);
	/* NOT 2D value for MND divider. */
	u32 d_val = ~(clamp_t(u32, n, m, n_minus_m));
	u32 mask = BIT(mnd_width) - 1;

	debug("m %#x n %#x d %#x div %#x mask %#x\n", m_val, n_val, d_val, div, mask);

	/* Program MND values */
	writel(m_val & mask, base + cmd_rcgr + RCG_M_REG);
	writel(n_val & mask, base + cmd_rcgr + RCG_N_REG);
	writel(d_val & mask, base + cmd_rcgr + RCG_D_REG);

	/* setup src select and divider */
	cfg  = readl(base + cmd_rcgr + RCG_CFG_REG);
	cfg &= ~(CFG_SRC_SEL_MASK | CFG_MODE_MASK | CFG_HW_CLK_CTRL_MASK |
		 CFG_SRC_DIV_MASK);
	cfg |= source & CFG_SRC_SEL_MASK; /* Select clock source */

	if (div)
		cfg |= div & CFG_SRC_DIV_MASK;

	if (n && n != m)
		cfg |= CFG_MODE_DUAL_EDGE;

	writel(cfg, base + cmd_rcgr + RCG_CFG_REG); /* Write new clock configuration */

	/* Inform h/w to start using the new config. */
	clk_bcr_update(base + cmd_rcgr);
}

/* root set rate for clocks with half integer and mnd_width=0 */
void clk_rcg_set_rate(phys_addr_t base, uint32_t cmd_rcgr, int div,
		      int source)
{
	u32 cfg;

	/* setup src select and divider */
	cfg  = readl(base + cmd_rcgr + RCG_CFG_REG);
	cfg &= ~(CFG_SRC_SEL_MASK | CFG_MODE_MASK | CFG_HW_CLK_CTRL_MASK);
	cfg |= source & CFG_CLK_SRC_MASK; /* Select clock source */

	/*
	 * Set the divider; HW permits fraction dividers (+0.5), but
	 * for simplicity, we will support integers only
	 */
	if (div)
		cfg |= (2 * div - 1) & CFG_SRC_DIV_MASK;

	writel(cfg, base + cmd_rcgr + RCG_CFG_REG); /* Write new clock configuration */

	/* Inform h/w to start using the new config. */
	clk_bcr_update(base + cmd_rcgr);
}

#define PHY_MUX_MASK		GENMASK(1, 0)
#define PHY_MUX_PHY_SRC		0
#define PHY_MUX_REF_SRC		2

void clk_phy_mux_enable(phys_addr_t base, uint32_t cmd_rcgr, bool enabled)
{
	u32 cfg;

	/* setup src select and divider */
	cfg  = readl(base + cmd_rcgr);
	cfg &= ~(PHY_MUX_MASK);
	if (enabled)
		cfg |= FIELD_PREP(PHY_MUX_MASK, PHY_MUX_PHY_SRC);
	else
		cfg |= FIELD_PREP(PHY_MUX_MASK, PHY_MUX_REF_SRC);

	writel(cfg, base + cmd_rcgr);
}

const struct freq_tbl *qcom_find_freq(const struct freq_tbl *f, uint rate)
{
	if (!f)
		return NULL;

	if (!f->freq)
		return f;

	for (; f->freq; f++)
		if (rate <= f->freq)
			return f;

	/* Default to our fastest rate */
	return f - 1;
}

static int msm_clk_probe(struct udevice *dev)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(dev);
	struct msm_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = data;

	return 0;
}

static ulong msm_clk_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(clk->dev);

	if (data->set_rate)
		return data->set_rate(clk, rate);

	return 0;
}

static int msm_clk_enable(struct clk *clk)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(clk->dev);

	if (data->enable)
		return data->enable(clk);

	return 0;
}

static void dump_gplls(struct udevice *dev, phys_addr_t base)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(dev);
	u32 i;
	bool locked;
	u64 l, a, xo_rate = 19200000;
	struct clk *clk = NULL;
	struct udevice *xodev;
	const phys_addr_t *gplls = data->dbg_pll_addrs;

	uclass_foreach_dev_probe(UCLASS_CLK, xodev) {
		if (!strcmp(xodev->name, "xo-board") || !strcmp(xodev->name, "xo_board")) {
			clk = dev_get_clk_ptr(xodev);
			break;
		}
	}

	if (clk) {
		xo_rate = clk_get_rate(clk);

		/* On SDM845 this needs to be divided by 2 for some reason */
		if (xo_rate && of_machine_is_compatible("qcom,sdm845"))
			xo_rate /= 2;
	} else {
		printf("Can't find XO clock, XO_BOARD rate may be wrong\n");
	}

	printf("GPLL clocks:\n");
	printf("| GPLL   | LOCKED | XO_BOARD  |  PLL_L     | ALPHA          |\n");
	printf("+--------+--------+-----------+------------+----------------+\n");
	for (i = 0; i < data->num_plls; i++) {
		locked = !!(readl(gplls[i]) & BIT(31));
		l = readl(gplls[i] + 4) & (BIT(16) - 1);
		a = readq(gplls[i] + 40) & (BIT(16) - 1);
		printf("| GPLL%-2d | %-6s | %9llu * (%#-9llx + %#-13llx  * 2 ** -40 ) / 1000000\n",
		       i, locked ? "X" : "", xo_rate, l, a);
	}
}

static void dump_rcgs(struct udevice *dev)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(dev);
	int i;
	u32 cmd;
	u32 cfg;
	u32 not_n_minus_m;
	u32 src, m, n, div;
	bool root_on, d_odd;

	printf("\nRCGs:\n");

	/*
	 * Which GPLL SRC corresponds to depends on the parent map, see gcc-<soc>.c in Linux
	 * and find the parent map associated with the clock. Note that often there are multiple
	 * outputs from a single GPLL where one is actually half the rate of the other (_EVEN).
	 * intput_freq = associated GPLL output freq (potentially divided depending on SRC).
	 */
	printf("| NAME                             | ON | SRC | OUT_FREQ = input_freq * (m/n) * (1/d) | [CMD REG   ] |\n");
	printf("+----------------------------------+----+-----+---------------------------------------+--------------+\n");
	for (i = 0; i < data->num_rcgs; i++) {
		cmd = readl(data->dbg_rcg_addrs[i]);
		cfg = readl(data->dbg_rcg_addrs[i] + 0x4);
		m = readl(data->dbg_rcg_addrs[i] + 0x8);
		n = 0;
		not_n_minus_m = readl(data->dbg_rcg_addrs[i] + 0xc);

		root_on = !(cmd & BIT(31)); // ROOT_OFF
		src = (cfg >> 8) & 7;

		if (not_n_minus_m) {
			n = (~not_n_minus_m & 0xffff);

			/* A clumsy assumption that this is an 8-bit MND RCG */
			if ((n & 0xff00) == 0xff00)
				n = n & 0xff;

			n += m;
		}

		div = ((cfg & 0b11111) + 1) / 2;
		d_odd = ((cfg & 0b11111) + 1) % 2 == 1;
		printf("%-34s | %-2s | %3d | input_freq * (%4d/%5d) * (1/%1d%-2s)   | [%#010x]\n",
		       data->dbg_rcg_names[i], root_on ? "X" : "", src,
		       m ?: 1, n ?: 1, div, d_odd ? ".5" : "", cmd);
	}

	printf("\n");
}

static void __maybe_unused msm_dump_clks(struct udevice *dev)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(dev);
	struct msm_clk_priv *priv = dev_get_priv(dev);
	const struct gate_clk *sclk;
	int val, i;

	if (!data->clks) {
		printf("No clocks\n");
		return;
	}

	printf("Gate Clocks:\n");
	for (i = 0; i < data->num_clks; i++) {
		sclk = &data->clks[i];
		if (!sclk->name)
			continue;
		printf("%-32s: ", sclk->name);
		val = readl(priv->base + sclk->reg) & sclk->en_val;
		printf("%s\n", val ? "ON" : "");
	}

	dump_gplls(dev, priv->base);
	dump_rcgs(dev);
}

static struct clk_ops msm_clk_ops = {
	.set_rate = msm_clk_set_rate,
	.enable = msm_clk_enable,
#if IS_ENABLED(CONFIG_CMD_CLK)
	.dump = msm_dump_clks,
#endif
};

U_BOOT_DRIVER(qcom_clk) = {
	.name		= "qcom_clk",
	.id		= UCLASS_CLK,
	.ops		= &msm_clk_ops,
	.priv_auto	= sizeof(struct msm_clk_priv),
	.probe		= msm_clk_probe,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

int qcom_cc_bind(struct udevice *parent)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(parent);
	struct udevice *clkdev = NULL, *rstdev = NULL, *pwrdev;
	struct driver *drv;
	int ret;

	/* Get a handle to the common clk handler */
	drv = lists_driver_lookup_name("qcom_clk");
	if (!drv)
		return -ENOENT;

	/* Register the clock controller */
	ret = device_bind_with_driver_data(parent, drv, "qcom_clk", (ulong)data,
					   dev_ofnode(parent), &clkdev);
	if (ret)
		return ret;

	if (data->resets) {
		/* Get a handle to the common reset handler */
		drv = lists_driver_lookup_name("qcom_reset");
		if (!drv) {
			ret = -ENOENT;
			goto unbind_clkdev;
		}

		/* Register the reset controller */
		ret = device_bind_with_driver_data(parent, drv, "qcom_reset", (ulong)data,
						   dev_ofnode(parent), &rstdev);
		if (ret)
			goto unbind_clkdev;
	}

	if (data->power_domains) {
		/* Get a handle to the common power domain handler */
		drv = lists_driver_lookup_name("qcom_power");
		if (!drv) {
			ret = -ENOENT;
			goto unbind_rstdev;
		}
		/* Register the power domain controller */
		ret = device_bind_with_driver_data(parent, drv, "qcom_power", (ulong)data,
						   dev_ofnode(parent), &pwrdev);
		if (ret)
			goto unbind_rstdev;
	}

	return 0;

unbind_rstdev:
	device_unbind(rstdev);
unbind_clkdev:
	device_unbind(clkdev);

	return ret;
}

static int qcom_reset_set(struct reset_ctl *rst, bool assert)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(rst->dev);
	void __iomem *base = dev_get_priv(rst->dev);
	const struct qcom_reset_map *map;
	u32 value;

	map = &data->resets[rst->id];

	value = readl(base + map->reg);

	if (assert)
		value |= BIT(map->bit);
	else
		value &= ~BIT(map->bit);

	writel(value, base + map->reg);

	return 0;
}

static int qcom_reset_assert(struct reset_ctl *rst)
{
	return qcom_reset_set(rst, true);
}

static int qcom_reset_deassert(struct reset_ctl *rst)
{
	return qcom_reset_set(rst, false);
}

static const struct reset_ops qcom_reset_ops = {
	.rst_assert = qcom_reset_assert,
	.rst_deassert = qcom_reset_deassert,
};

static int qcom_reset_probe(struct udevice *dev)
{
	/* Set our priv pointer to the base address */
	dev_set_priv(dev, (void *)dev_read_addr(dev));

	return 0;
}

U_BOOT_DRIVER(qcom_reset) = {
	.name = "qcom_reset",
	.id = UCLASS_RESET,
	.ops = &qcom_reset_ops,
	.probe = qcom_reset_probe,
};

static int qcom_power_set(struct power_domain *pwr, bool on)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(pwr->dev);
	void __iomem *base = dev_get_priv(pwr->dev);
	const struct qcom_power_map *map;
	u32 value;
	int ret;

	if (pwr->id >= data->num_power_domains)
		return -ENODEV;

	map = &data->power_domains[pwr->id];

	if (!map->reg)
		return -ENODEV;

	value = readl(base + map->reg);

	if (on)
		value &= ~GDSC_SW_COLLAPSE_MASK;
	else
		value |= GDSC_SW_COLLAPSE_MASK;

	writel(value, base + map->reg);

	if (on)
		ret = readl_poll_timeout(base + map->reg + CFG_GDSCR_OFFSET,
					 value,
					 (value & GDSC_POWER_UP_COMPLETE) ||
					 (value & GDSC_PWR_ON_MASK),
					 GDSC_STATUS_POLL_TIMEOUT_US);

	else
		ret = readl_poll_timeout(base + map->reg + CFG_GDSCR_OFFSET,
					 value,
					 (value & GDSC_POWER_DOWN_COMPLETE) ||
					 !(value & GDSC_PWR_ON_MASK),
					 GDSC_STATUS_POLL_TIMEOUT_US);

	if (ret == -ETIMEDOUT)
		printf("WARNING: GDSC %lu is stuck during power on/off\n",
		       pwr->id);
	return ret;
}

static int qcom_power_on(struct power_domain *pwr)
{
	return qcom_power_set(pwr, true);
}

static int qcom_power_off(struct power_domain *pwr)
{
	return qcom_power_set(pwr, false);
}

static const struct power_domain_ops qcom_power_ops = {
	.on = qcom_power_on,
	.off = qcom_power_off,
};

static int qcom_power_probe(struct udevice *dev)
{
	/* Set our priv pointer to the base address */
	dev_set_priv(dev, (void *)dev_read_addr(dev));

	return 0;
}

U_BOOT_DRIVER(qcom_power) = {
	.name = "qcom_power",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &qcom_power_ops,
	.probe = qcom_power_probe,
	.flags = DM_FLAG_PRE_RELOC,
};

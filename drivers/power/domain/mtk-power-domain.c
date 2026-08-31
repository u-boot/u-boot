// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <dm/device_compat.h>
#include <dm/devres.h>

#include "mtk-power-domain.h"

/**
 * This function enables the bus protection bits for disabled power
 * domains so that the system does not hang when some unit accesses the
 * bus while in power down.
 */
static int mtk_infracfg_set_bus_protection(void __iomem *infracfg,
					   u32 mask)
{
	u32 val;

	clrsetbits_le32(infracfg + INFRA_TOPAXI_PROT_EN, mask, mask);

	return readl_poll_timeout(infracfg + INFRA_TOPAXI_PROT_STA1, val,
				  (val & mask) == mask, 100);
}

static int mtk_infracfg_clear_bus_protection(void __iomem *infracfg,
					     u32 mask)
{
	u32 val;

	clrbits_le32(infracfg + INFRA_TOPAXI_PROT_EN, mask);

	return readl_poll_timeout(infracfg + INFRA_TOPAXI_PROT_STA1, val,
				  !(val & mask), 100);
}

static int _scpsys_bus_protect_enable(const struct mtk_scpsys_bus_prot_data *bpd,
				      void __iomem *reg)
{
	u32 val = 0, mask = bpd->bus_prot_mask;

	if (!mask)
		return 0;

	if (bpd->bus_prot_reg_update)
		clrsetbits_le32(reg + bpd->bus_prot_set, mask, mask);
	else
		writel(mask, reg + bpd->bus_prot_set);

	return readl_poll_timeout(reg + bpd->bus_prot_sta, val, (val & mask) == mask, 1000);
}

static int _scpsys_bus_protect_disable(const struct mtk_scpsys_bus_prot_data *bpd,
				       void __iomem *reg)
{
	u32 val = 0, mask = bpd->bus_prot_mask;

	if (!mask)
		return 0;

	if (bpd->bus_prot_reg_update)
		clrbits_le32(reg + bpd->bus_prot_clr, mask);
	else
		writel(mask, reg + bpd->bus_prot_clr);

	if (bpd->ignore_clr_ack)
		return 0;

	return readl_poll_timeout(reg + bpd->bus_prot_sta, val, !(val & mask), 1000);
}

static int scpsys_bus_protect_enable(const struct mtk_scpsys_bus_prot_data *bpd,
				     int bpd_size, void __iomem *reg)
{
	int ret, i;

	for (i = 0; i < bpd_size; i++) {
		ret = _scpsys_bus_protect_enable(&bpd[i], reg);
		if (ret)
			return ret;
	}

	return 0;
}

static int scpsys_bus_protect_disable(const struct mtk_scpsys_bus_prot_data *bpd,
				      int bpd_size, void __iomem *reg)
{
	int i, ret;

	for (i = bpd_size - 1; i >= 0; i--) {
		ret = _scpsys_bus_protect_disable(&bpd[i], reg);
		if (ret)
			return ret;
	}

	return 0;
}

static int mtk_scpsys_domain_is_on(struct power_domain *power_domain)
{
	struct mtk_scpsys *scpsys = dev_get_priv(power_domain->dev);
	const struct mtk_scp_domain_data *data = scpsys->domains[power_domain->id].data;
	u32 spm_pwr_status = data->pwr_sta_offs ?: SPM_PWR_STATUS;
	u32 spm_pwr_status_2nd = data->pwr_sta2nd_offs ?: SPM_PWR_STATUS_2ND;
	u32 sta, sta2;

	sta = readl(scpsys->base + spm_pwr_status) & data->sta_mask;
	sta2 = readl(scpsys->base + spm_pwr_status_2nd) & data->sta_mask;

	/*
	 * A domain is on when both status bits are set. If only one is set
	 * return an error. This happens while powering up a domain
	 */
	if (sta && sta2)
		return true;
	if (!sta && !sta2)
		return false;

	return -EINVAL;
}

static int mtk_scpsys_power_on(struct power_domain *power_domain)
{
	struct mtk_scpsys *scpsys = dev_get_priv(power_domain->dev);
	struct mtk_scp_domain *domain;
	const struct mtk_scp_domain_data *data;
	void __iomem *ctl_addr;
	void __iomem *infracfg;
	u32 pdn_ack;
	u32 val;
	int ret, tmp;

	if (power_domain->id >= scpsys->soc_data->num_domains)
		return -EINVAL;

	domain = &scpsys->domains[power_domain->id];
	data = domain->data;
	if (!data)
		return -EINVAL;

	ctl_addr = scpsys->base + data->ctl_offs;
	infracfg = domain->infracfg ? domain->infracfg : scpsys->infracfg;
	pdn_ack = data->sram_pdn_ack_bits;

	if (domain->has_pd) {
		ret = power_domain_on(&domain->parent_pd);
		if (ret)
			return ret;
	}

	ret = clk_enable_bulk(&domain->clks);
	if (ret)
		return ret;

	writel(SPM_EN, scpsys->base);

	val = readl(ctl_addr);
	val |= PWR_ON_BIT;
	writel(val, ctl_addr);

	val |= PWR_ON_2ND_BIT;
	writel(val, ctl_addr);

	ret = readx_poll_timeout(mtk_scpsys_domain_is_on, power_domain, tmp, tmp > 0,
				 100);
	if (ret < 0)
		return ret;

	val &= ~PWR_CLK_DIS_BIT;
	writel(val, ctl_addr);

	val &= ~PWR_ISO_BIT;
	writel(val, ctl_addr);

	val |= PWR_RST_B_BIT;
	writel(val, ctl_addr);

	ret = clk_enable_bulk(&domain->subsys_clks);
	if (ret)
		return ret;

	val &= ~data->sram_pdn_bits;
	writel(val, ctl_addr);

	ret = readl_poll_timeout(ctl_addr, tmp, !(tmp & pdn_ack), 100);
	if (ret < 0)
		return ret;

	if (data->bus_prot_mask) {
		ret = mtk_infracfg_clear_bus_protection(infracfg,
							data->bus_prot_mask);
		if (ret)
			return ret;
	}
	ret = scpsys_bus_protect_disable(data->bp_infracfg, SPM_MAX_BUS_PROT_DATA, infracfg);
	if (ret < 0)
		return ret;

	return 0;
}

static int mtk_scpsys_power_off(struct power_domain *power_domain)
{
	struct mtk_scpsys *scpsys = dev_get_priv(power_domain->dev);
	struct mtk_scp_domain *domain;
	const struct mtk_scp_domain_data *data;
	void __iomem *ctl_addr;
	void __iomem *infracfg;
	u32 pdn_ack;
	u32 val;
	int ret, tmp;

	if (power_domain->id >= scpsys->soc_data->num_domains)
		return -EINVAL;

	domain = &scpsys->domains[power_domain->id];
	data = domain->data;
	if (!data)
		return -EINVAL;

	ctl_addr = scpsys->base + data->ctl_offs;
	infracfg = domain->infracfg ?: scpsys->infracfg;
	pdn_ack = data->sram_pdn_ack_bits;

	if (data->bus_prot_mask) {
		ret = mtk_infracfg_set_bus_protection(infracfg,
						      data->bus_prot_mask);
		if (ret)
			return ret;
	}

	ret = scpsys_bus_protect_enable(data->bp_infracfg, SPM_MAX_BUS_PROT_DATA, infracfg);
	if (ret < 0)
		return ret;

	val = readl(ctl_addr);
	val |= data->sram_pdn_bits;
	writel(val, ctl_addr);

	ret = readl_poll_timeout(ctl_addr, tmp, (tmp & pdn_ack) == pdn_ack,
				 100);
	if (ret < 0)
		return ret;

	ret = clk_disable_bulk(&domain->subsys_clks);
	if (ret)
		return ret;

	val |= PWR_ISO_BIT;
	writel(val, ctl_addr);

	val &= ~PWR_RST_B_BIT;
	writel(val, ctl_addr);

	val |= PWR_CLK_DIS_BIT;
	writel(val, ctl_addr);

	val &= ~PWR_ON_BIT;
	writel(val, ctl_addr);

	val &= ~PWR_ON_2ND_BIT;
	writel(val, ctl_addr);

	ret = readx_poll_timeout(mtk_scpsys_domain_is_on, power_domain, tmp, !tmp, 100);
	if (ret < 0)
		return ret;

	ret = clk_disable_bulk(&domain->clks);
	if (ret)
		return ret;

	return 0;
}

int mtk_scpsys_probe(struct udevice *dev)
{
	struct ofnode_phandle_args args;
	struct mtk_scpsys *scpsys = dev_get_priv(dev);
	struct regmap *regmap;
	struct clk_bulk bulk;
	int err, i;

	scpsys->base = dev_read_addr_ptr(dev);
	if (!scpsys->base)
		return -ENOENT;

	scpsys->soc_data = (const struct mtk_scp_soc_data *)dev_get_driver_data(dev);

	scpsys->domains = devm_kcalloc(dev, scpsys->soc_data->num_domains,
				       sizeof(*scpsys->domains), GFP_KERNEL);
	if (!scpsys->domains)
		return -ENOMEM;

	for (i = 0; i < scpsys->soc_data->num_domains; i++)
		scpsys->domains[i].data = &scpsys->soc_data->data[i];

	/* get corresponding syscon phandle */
	err = dev_read_phandle_with_args(dev, "infracfg", NULL, 0, 0, &args);
	if (err)
		return err;

	regmap = syscon_node_to_regmap(args.node);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	scpsys->infracfg = regmap_get_range(regmap, 0);
	if (!scpsys->infracfg)
		return -ENOENT;

	/* enable Infra DCM */
	setbits_le32(scpsys->infracfg + INFRA_TOPDCM_CTRL, DCM_TOP_EN);

	err = clk_get_bulk(dev, &bulk);
	if (err)
		return err;

	return clk_enable_bulk(&bulk);
}

static int mtk_scpsys_add_one_domain(struct udevice *dev, ofnode node, int parent_id)
{
	struct mtk_scpsys *scpsys = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	struct mtk_scp_domain *domain;
	struct regmap *regmap;
	const char *clk_name;
	int i, ret, num_clks;
	u32 id;

	ret = ofnode_read_u32(node, "reg", &id);
	if (ret) {
		dev_err(dev, "%s: failed to retrieve domain id from reg: %d\n",
			ofnode_get_name(node), ret);
		return ret;
	}

	if (id >= scpsys->soc_data->num_domains) {
		dev_err(dev, "%s: invalid domain id %d\n", ofnode_get_name(node), id);
		return -EINVAL;
	}

	domain = &scpsys->domains[id];
	domain->data = &scpsys->soc_data->data[id];

	if (parent_id >= 0) {
		domain->has_pd = true;
		domain->parent_pd.dev = dev;
		domain->parent_pd.id = parent_id;
	}

	if (ofnode_read_bool(node, "mediatek,infracfg")) {
		ret = ofnode_parse_phandle_with_args(node, "mediatek,infracfg", NULL, 0, 0, &args);
		if (ret)
			return ret;

		regmap = syscon_node_to_regmap(args.node);
		if (IS_ERR(regmap))
			return PTR_ERR(regmap);

		domain->infracfg = regmap_get_range(regmap, 0);

		/* enable Infra DCM */
		if (domain->infracfg)
			setbits_le32(domain->infracfg + INFRA_TOPDCM_CTRL,
				     DCM_TOP_EN);
	}

	num_clks = ofnode_read_string_count(node, "clock-names");
	for (i = 0; i < num_clks; i++) {
		ret = ofnode_read_string_index(node, "clock-names", i, &clk_name);
		if (ret) {
			dev_err(dev, "%s: failed to retrieve clock-names at index %i: %d\n",
				ofnode_get_name(node), i, ret);
			return ret;
		}

		if (strchr(clk_name, '-'))
			domain->subsys_clks.count++;
		else
			domain->clks.count++;
	}

	if (domain->clks.count) {
		domain->clks.clks = devm_kcalloc(dev, domain->clks.count,
						 sizeof(struct clk), GFP_KERNEL);
		if (!domain->clks.clks)
			return -ENOMEM;
	}

	if (domain->subsys_clks.count) {
		domain->subsys_clks.clks = devm_kcalloc(dev,
							domain->subsys_clks.count,
							sizeof(struct clk), GFP_KERNEL);
		if (!domain->subsys_clks.clks)
			return -ENOMEM;
	}

	for (i = 0; i < domain->clks.count; i++) {
		ret = clk_get_by_index_nodev(node, i, &domain->clks.clks[i]);
		if (ret < 0) {
			dev_err(dev, "%s: failed to get clk at index %d: %d\n",
				ofnode_get_name(node), i, ret);
			goto err_put_clocks;
		}
	}

	for (i = 0; i < domain->subsys_clks.count; i++) {
		ret = clk_get_by_index_nodev(node, i + domain->clks.count,
					     &domain->subsys_clks.clks[i]);
		if (ret < 0) {
			dev_err(dev, "%s: failed to get subsys clk at index %d: %d\n",
				ofnode_get_name(node), i + domain->clks.count, ret);
			goto err_put_subsys_clocks;
		}
	}

	return 0;

err_put_subsys_clocks:
	clk_release_all(domain->subsys_clks.clks, domain->subsys_clks.count);
	domain->subsys_clks.count = 0;
err_put_clocks:
	clk_release_all(domain->clks.clks, domain->clks.count);
	domain->clks.count = 0;
	domain->data = NULL;
	return ret;
}

static int mtk_scpsys_add_subdomain(struct udevice *dev, ofnode node)
{
	ofnode subnode;
	int ret;
	u32 id;

	ret = ofnode_read_u32(node, "reg", &id);
	if (ret) {
		dev_err(dev, "%s: failed to get domain id\n", ofnode_get_name(node));
		return ret;
	}

	ofnode_for_each_subnode(subnode, node) {
		ret = mtk_scpsys_add_one_domain(dev, subnode, id);
		if (ret) {
			dev_err(dev, "failed to add child domain: %s\n",
				ofnode_get_name(subnode));
			continue;
		}

		ret = mtk_scpsys_add_subdomain(dev, subnode);
		if (ret)
			return ret;
	}

	return 0;
}

int mtk_power_controller_probe(struct udevice *dev)
{
	struct mtk_scpsys *scpsys = dev_get_priv(dev);
	ofnode subnode;
	int ret;

	scpsys->base = dev_read_addr_ptr(dev_get_parent(dev));
	if (!scpsys->base)
		return -ENOENT;

	scpsys->soc_data = (const struct mtk_scp_soc_data *)dev_get_driver_data(dev);

	scpsys->domains = devm_kcalloc(dev, scpsys->soc_data->num_domains,
				       sizeof(*scpsys->domains), GFP_KERNEL);
	if (!scpsys->domains)
		return -ENOMEM;

	dev_for_each_subnode(subnode, dev) {
		ret = mtk_scpsys_add_one_domain(dev, subnode, -1);
		if (ret) {
			dev_err(dev, "failed to add child domain: %s\n",
				ofnode_get_name(subnode));
			continue;
		}

		ret = mtk_scpsys_add_subdomain(dev, subnode);
		if (ret) {
			dev_err(dev, "failed to add sub domain: %s\n",
				ofnode_get_name(subnode));
			return ret;
		}
	}

	return 0;
}

const struct power_domain_ops mtk_power_domain_ops = {
	.off = mtk_scpsys_power_off,
	.on = mtk_scpsys_power_on,
};

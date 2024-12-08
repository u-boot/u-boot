// SPDX-License-Identifier: GPL-2.0+
/*
 * StarFive PLDA PCIe host controller driver
 *
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 * Author: Mason Huo <mason.huo@starfivetech.com>
 *
 */

#include <clk.h>
#include <dm.h>
#include <pci.h>
#include <pci_ids.h>
#include <power-domain.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include "pcie_plda_common.h"

/* system control */
#define STG_SYSCON_PCIE0_BASE                  0x48
#define STG_SYSCON_PCIE1_BASE                  0x1f8

#define STG_SYSCON_AR_OFFSET                   0x78
#define STG_SYSCON_AXI4_SLVL_ARFUNC_MASK       GENMASK(22, 8)
#define STG_SYSCON_AXI4_SLVL_ARFUNC_SHIFT      8
#define STG_SYSCON_AW_OFFSET                   0x7c
#define STG_SYSCON_AXI4_SLVL_AWFUNC_MASK       GENMASK(14, 0)
#define STG_SYSCON_CLKREQ_MASK                 BIT(22)
#define STG_SYSCON_CKREF_SRC_SHIFT             18
#define STG_SYSCON_CKREF_SRC_MASK              GENMASK(19, 18)
#define STG_SYSCON_RP_NEP_OFFSET               0xe8
#define STG_SYSCON_K_RP_NEP_MASK               BIT(8)

DECLARE_GLOBAL_DATA_PTR;

struct starfive_pcie {
	struct pcie_plda plda;
	struct clk_bulk	clks;
	struct reset_ctl_bulk	rsts;
	struct gpio_desc	reset_gpio;
	struct regmap *regmap;
	unsigned int stg_pcie_base;
};

static int starfive_pcie_atr_init(struct starfive_pcie *priv)
{
	struct udevice *ctlr = pci_get_controller(priv->plda.dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	int i, ret;

	/*
	 * As the two host bridges in JH7110 soc have the same default
	 * address translation table, this cause the second root port can't
	 * access it's host bridge config space correctly.
	 * To workaround, config the ATR of host bridge config space by SW.
	 */

	ret = plda_pcie_set_atr_entry(&priv->plda,
				      (phys_addr_t)priv->plda.cfg_base, 0,
				      priv->plda.cfg_size,
				      XR3PCI_ATR_TRSLID_PCIE_CONFIG);
	if (ret)
		return ret;

	for (i = 0; i < hose->region_count; i++) {
		if (hose->regions[i].flags == PCI_REGION_SYS_MEMORY)
			continue;

		/* Only support identity mappings. */
		if (hose->regions[i].bus_start !=
		    hose->regions[i].phys_start)
			return -EINVAL;

		ret = plda_pcie_set_atr_entry(&priv->plda,
					      hose->regions[i].phys_start,
					      hose->regions[i].bus_start,
					      hose->regions[i].size,
					      XR3PCI_ATR_TRSLID_PCIE_MEMORY);
		if (ret)
			return ret;
	}

	return 0;
}

static int starfive_pcie_get_syscon(struct udevice *dev)
{
	struct starfive_pcie *priv = dev_get_priv(dev);
	struct udevice *syscon;
	struct ofnode_phandle_args syscfg_phandle;
	int ret;

	/* get corresponding syscon phandle */
	ret = dev_read_phandle_with_args(dev, "starfive,stg-syscon", NULL, 0, 0,
					 &syscfg_phandle);

	if (ret < 0) {
		dev_err(dev, "Can't get syscfg phandle: %d\n", ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_SYSCON, syscfg_phandle.node,
					  &syscon);
	if (ret) {
		dev_err(dev, "Unable to find syscon device (%d)\n", ret);
		return ret;
	}

	priv->regmap = syscon_get_regmap(syscon);
	if (!priv->regmap) {
		dev_err(dev, "Unable to find regmap\n");
		return -ENODEV;
	}

	return 0;
}

static int starfive_pcie_parse_dt(struct udevice *dev)
{
	struct starfive_pcie *priv = dev_get_priv(dev);
	int ret;
	u32 domain_nr;

	priv->plda.reg_base = (void *)dev_read_addr_name(dev, "apb");
	if (priv->plda.reg_base == (void __iomem *)FDT_ADDR_T_NONE) {
		dev_err(dev, "Missing required reg address range\n");
		return -EINVAL;
	}

	priv->plda.cfg_base =
		(void *)dev_read_addr_size_name(dev,
						"cfg",
						&priv->plda.cfg_size);
	if (priv->plda.cfg_base == (void __iomem *)FDT_ADDR_T_NONE) {
		dev_err(dev, "Missing required config address range");
		return -EINVAL;
	}

	ret = starfive_pcie_get_syscon(dev);
	if (ret) {
		dev_err(dev, "Can't get syscon: %d\n", ret);
		return ret;
	}

	ret = reset_get_bulk(dev, &priv->rsts);
	if (ret) {
		dev_err(dev, "Can't get reset: %d\n", ret);
		return ret;
	}

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "Can't get clock: %d\n", ret);
		return ret;
	}

	ret = dev_read_u32(dev, "linux,pci-domain", &domain_nr);
	if (ret) {
		dev_err(dev, "Can't get pci domain: %d\n", ret);
		return ret;
	}

	if (domain_nr == 0)
		priv->stg_pcie_base = STG_SYSCON_PCIE0_BASE;
	else
		priv->stg_pcie_base = STG_SYSCON_PCIE1_BASE;

	ret = gpio_request_by_name(dev, "perst-gpios", 0, &priv->reset_gpio,
				   GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Can't get reset-gpio: %d\n", ret);
		return ret;
	}

	if (!dm_gpio_is_valid(&priv->reset_gpio)) {
		dev_err(dev, "reset-gpio is not valid\n");
		return -EINVAL;
	}
	return 0;
}

static int starfive_pcie_init_port(struct udevice *dev)
{
	int ret, i;
	struct starfive_pcie *priv = dev_get_priv(dev);
	struct pcie_plda *plda = &priv->plda;

	ret = clk_enable_bulk(&priv->clks);
	if (ret) {
		dev_err(dev, "Failed to enable clks (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_deassert_bulk(&priv->rsts);
	if (ret) {
		dev_err(dev, "Failed to deassert resets (ret=%d)\n", ret);
		goto err_deassert_clk;
	}

	dm_gpio_set_value(&priv->reset_gpio, 1);
	/* Disable physical functions except #0 */
	for (i = 1; i < PLDA_FUNC_NUM; i++) {
		regmap_update_bits(priv->regmap,
				   priv->stg_pcie_base + STG_SYSCON_AR_OFFSET,
				   STG_SYSCON_AXI4_SLVL_ARFUNC_MASK,
				   (i << PLDA_PHY_FUNC_SHIFT) <<
				   STG_SYSCON_AXI4_SLVL_ARFUNC_SHIFT);
		regmap_update_bits(priv->regmap,
				   priv->stg_pcie_base + STG_SYSCON_AW_OFFSET,
				   STG_SYSCON_AXI4_SLVL_AWFUNC_MASK,
				   i << PLDA_PHY_FUNC_SHIFT);

		plda_pcie_disable_func(plda);
	}

	/* Disable physical functions */
	regmap_update_bits(priv->regmap,
			   priv->stg_pcie_base + STG_SYSCON_AR_OFFSET,
			   STG_SYSCON_AXI4_SLVL_ARFUNC_MASK,
			   0);
	regmap_update_bits(priv->regmap,
			   priv->stg_pcie_base + STG_SYSCON_AW_OFFSET,
			   STG_SYSCON_AXI4_SLVL_AWFUNC_MASK,
			   0);

	plda_pcie_enable_root_port(plda);

	/* PCIe PCI Standard Configuration Identification Settings. */
	plda_pcie_set_standard_class(plda);

	/*
	 * The LTR message forwarding of PCIe Message Reception was set by core
	 * as default, but the forward id & addr are also need to be reset.
	 * If we do not disable LTR message forwarding here, or set a legal
	 * forwarding address, the kernel will get stuck after this driver probe.
	 * To workaround, disable the LTR message forwarding support on
	 * PCIe Message Reception.
	 */
	plda_pcie_disable_ltr(plda);

	/* Prefetchable memory window 64-bit addressing support */
	plda_pcie_set_pref_win_64bit(plda);
	starfive_pcie_atr_init(priv);

	dm_gpio_set_value(&priv->reset_gpio, 0);
	/* Ensure that PERST in default at least 300 ms */
	mdelay(300);

	return 0;

err_deassert_clk:
	clk_disable_bulk(&priv->clks);
	return ret;
}

static int starfive_pcie_probe(struct udevice *dev)
{
	struct starfive_pcie *priv = dev_get_priv(dev);
	int ret;

	priv->plda.atr_table_num = 0;
	priv->plda.dev = dev;

	ret = starfive_pcie_parse_dt(dev);
	if (ret)
		return ret;

	regmap_update_bits(priv->regmap,
			   priv->stg_pcie_base + STG_SYSCON_RP_NEP_OFFSET,
			   STG_SYSCON_K_RP_NEP_MASK,
			   STG_SYSCON_K_RP_NEP_MASK);

	regmap_update_bits(priv->regmap,
			   priv->stg_pcie_base + STG_SYSCON_AW_OFFSET,
			   STG_SYSCON_CKREF_SRC_MASK,
			   2 << STG_SYSCON_CKREF_SRC_SHIFT);

	regmap_update_bits(priv->regmap,
			   priv->stg_pcie_base + STG_SYSCON_AW_OFFSET,
			   STG_SYSCON_CLKREQ_MASK,
			   STG_SYSCON_CLKREQ_MASK);

	ret = starfive_pcie_init_port(dev);
	if (ret)
		return ret;

	dev_err(dev, "Starfive PCIe bus probed.\n");

	return 0;
}

static const struct dm_pci_ops starfive_pcie_ops = {
	.read_config	= plda_pcie_config_read,
	.write_config	= plda_pcie_config_write,
};

static const struct udevice_id starfive_pcie_ids[] = {
	{ .compatible = "starfive,jh7110-pcie" },
	{ }
};

U_BOOT_DRIVER(starfive_pcie_drv) = {
	.name			= "starfive_7110_pcie",
	.id			= UCLASS_PCI,
	.of_match		= starfive_pcie_ids,
	.ops			= &starfive_pcie_ops,
	.probe			= starfive_pcie_probe,
	.priv_auto	= sizeof(struct starfive_pcie),
};

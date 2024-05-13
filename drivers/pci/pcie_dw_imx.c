// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Linaro Ltd.
 *
 * Author: Sumit Garg <sumit.garg@linaro.org>
 */

#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <log.h>
#include <pci.h>
#include <power/regulator.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <time.h>

#include "pcie_dw_common.h"

#define PCIE_LINK_CAPABILITY		0x7c
#define TARGET_LINK_SPEED_MASK		0xf
#define LINK_SPEED_GEN_1		0x1
#define LINK_SPEED_GEN_2		0x2
#define LINK_SPEED_GEN_3		0x3

#define PCIE_MISC_CONTROL_1_OFF		0x8bc
#define PCIE_DBI_RO_WR_EN		BIT(0)

#define PCIE_PORT_DEBUG0			0x728
#define PCIE_PORT_DEBUG1			0x72c
#define PCIE_PORT_DEBUG1_LINK_UP		BIT(4)
#define PCIE_PORT_DEBUG1_LINK_IN_TRAINING	BIT(29)

#define PCIE_LINK_UP_TIMEOUT_MS		100

#define IOMUXC_GPR14_OFFSET			0x38
#define IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE_EN	BIT(10)
#define IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE		BIT(11)

struct pcie_dw_imx {
	/* Must be first member of the struct */
	struct pcie_dw			dw;
	struct regmap			*iomuxc_gpr;
	struct clk_bulk			clks;
	struct gpio_desc		reset_gpio;
	struct reset_ctl		apps_reset;
	struct phy			phy;
	struct udevice			*vpcie;
};

struct pcie_chip_info {
	const char *gpr;
};

static const struct pcie_chip_info imx8mm_chip_info = {
	.gpr = "fsl,imx8mm-iomuxc-gpr",
};

static const struct pcie_chip_info imx8mp_chip_info = {
	.gpr = "fsl,imx8mp-iomuxc-gpr",
};

static void pcie_dw_configure(struct pcie_dw_imx *priv, u32 cap_speed)
{
	dw_pcie_dbi_write_enable(&priv->dw, true);

	clrsetbits_le32(priv->dw.dbi_base + PCIE_LINK_CAPABILITY,
			TARGET_LINK_SPEED_MASK, cap_speed);

	dw_pcie_dbi_write_enable(&priv->dw, false);
}

static void imx_pcie_ltssm_enable(struct pcie_dw_imx *priv)
{
	reset_deassert(&priv->apps_reset);
}

static void imx_pcie_ltssm_disable(struct pcie_dw_imx *priv)
{
	reset_assert(&priv->apps_reset);
}

static bool is_link_up(u32 val)
{
	return ((val & PCIE_PORT_DEBUG1_LINK_UP) &&
		(!(val & PCIE_PORT_DEBUG1_LINK_IN_TRAINING)));
}

static int wait_link_up(struct pcie_dw_imx *priv)
{
	u32 val;

	return readl_poll_sleep_timeout(priv->dw.dbi_base + PCIE_PORT_DEBUG1,
					val, is_link_up(val), 10000, 100000);
}

static int pcie_link_up(struct pcie_dw_imx *priv, u32 cap_speed)
{
	int ret;

	/* DW pre link configurations */
	pcie_dw_configure(priv, cap_speed);

	/* Initiate link training */
	imx_pcie_ltssm_enable(priv);

	/* Check that link was established */
	ret = wait_link_up(priv);
	if (ret)
		imx_pcie_ltssm_disable(priv);

	return ret;
}

static int imx_pcie_assert_core_reset(struct pcie_dw_imx *priv)
{
	if (dm_gpio_is_valid(&priv->reset_gpio)) {
		dm_gpio_set_value(&priv->reset_gpio, 1);
		mdelay(20);
	}

	return reset_assert(&priv->apps_reset);
}

static int imx_pcie_clk_enable(struct pcie_dw_imx *priv)
{
	int ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		return ret;

	/*
	 * Set the over ride low and enabled make sure that
	 * REF_CLK is turned on.
	 */
	regmap_update_bits(priv->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE, 0);
	regmap_update_bits(priv->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE_EN,
			   IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE_EN);

	/* allow the clocks to stabilize */
	udelay(500);

	return 0;
}

static void imx_pcie_deassert_core_reset(struct pcie_dw_imx *priv)
{
	if (!dm_gpio_is_valid(&priv->reset_gpio))
		return;

	mdelay(100);
	dm_gpio_set_value(&priv->reset_gpio, 0);
	/* Wait for 100ms after PERST# deassertion (PCIe r5.0, 6.6.1) */
	mdelay(100);
}

static int pcie_dw_imx_probe(struct udevice *dev)
{
	struct pcie_dw_imx *priv = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	int ret;

	if (priv->vpcie) {
		ret = regulator_set_enable(priv->vpcie, true);
		if (ret) {
			dev_err(dev, "failed to enable vpcie regulator\n");
			return ret;
		}
	}

	ret = imx_pcie_assert_core_reset(priv);
	if (ret) {
		dev_err(dev, "failed to assert core reset\n");
		return ret;
	}

	ret = imx_pcie_clk_enable(priv);
	if (ret) {
		dev_err(dev, "failed to enable clocks\n");
		goto err_clk;
	}

	ret = generic_phy_init(&priv->phy);
	if (ret) {
		dev_err(dev, "failed to initialize PHY\n");
		goto err_phy_init;
	}

	ret = generic_phy_power_on(&priv->phy);
	if (ret) {
		dev_err(dev, "failed to power on PHY\n");
		goto err_phy_power;
	}

	imx_pcie_deassert_core_reset(priv);

	priv->dw.first_busno = dev_seq(dev);
	priv->dw.dev = dev;
	pcie_dw_setup_host(&priv->dw);

	if (pcie_link_up(priv, LINK_SPEED_GEN_1)) {
		printf("PCIE-%d: Link down\n", dev_seq(dev));
		ret = -ENODEV;
		goto err_link;
	}

	printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n", dev_seq(dev),
	       pcie_dw_get_link_speed(&priv->dw),
	       pcie_dw_get_link_width(&priv->dw),
	       hose->first_busno);

	pcie_dw_prog_outbound_atu_unroll(&priv->dw, PCIE_ATU_REGION_INDEX0,
					 PCIE_ATU_TYPE_MEM,
					 priv->dw.mem.phys_start,
					 priv->dw.mem.bus_start, priv->dw.mem.size);

	return 0;

err_link:
	generic_shutdown_phy(&priv->phy);
err_phy_power:
	generic_phy_exit(&priv->phy);
err_phy_init:
	clk_disable_bulk(&priv->clks);
err_clk:
	imx_pcie_deassert_core_reset(priv);

	return ret;
}

static int pcie_dw_imx_remove(struct udevice *dev)
{
	struct pcie_dw_imx *priv = dev_get_priv(dev);

	generic_shutdown_phy(&priv->phy);
	dm_gpio_free(dev, &priv->reset_gpio);
	reset_free(&priv->apps_reset);
	clk_release_bulk(&priv->clks);

	return 0;
}

static int pcie_dw_imx_of_to_plat(struct udevice *dev)
{
	struct pcie_chip_info *info = (void *)dev_get_driver_data(dev);
	struct pcie_dw_imx *priv = dev_get_priv(dev);
	ofnode gpr;
	int ret;

	/* Get the controller base address */
	priv->dw.dbi_base = (void *)dev_read_addr_name(dev, "dbi");
	if ((fdt_addr_t)priv->dw.dbi_base == FDT_ADDR_T_NONE) {
		dev_err(dev, "failed to get dbi_base address\n");
		return -EINVAL;
	}

	/* Get the config space base address and size */
	priv->dw.cfg_base = (void *)dev_read_addr_size_name(dev, "config",
							    &priv->dw.cfg_size);
	if ((fdt_addr_t)priv->dw.cfg_base == FDT_ADDR_T_NONE) {
		dev_err(dev, "failed to get cfg_base address\n");
		return -EINVAL;
	}

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "failed to get PCIe clks\n");
		return ret;
	}

	ret = reset_get_by_name(dev, "apps", &priv->apps_reset);
	if (ret) {
		dev_err(dev,
			"Failed to get PCIe apps reset control\n");
		goto err_reset;
	}

	ret = gpio_request_by_name(dev, "reset-gpio", 0, &priv->reset_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		dev_err(dev, "unable to get reset-gpio\n");
		goto err_gpio;
	}

	ret = generic_phy_get_by_name(dev, "pcie-phy", &priv->phy);
	if (ret) {
		dev_err(dev, "failed to get pcie phy\n");
		goto err_phy;
	}

	gpr = ofnode_by_compatible(ofnode_null(), info->gpr);
	if (ofnode_equal(gpr, ofnode_null())) {
		dev_err(dev, "unable to find GPR node\n");
		ret = -ENODEV;
		goto err_phy;
	}

	priv->iomuxc_gpr = syscon_node_to_regmap(gpr);
	if (IS_ERR(priv->iomuxc_gpr)) {
		dev_err(dev, "unable to find iomuxc registers\n");
		ret = PTR_ERR(priv->iomuxc_gpr);
		goto err_phy;
	}

	/* vpcie-supply regulator is optional */
	device_get_supply_regulator(dev, "vpcie-supply", &priv->vpcie);

	return 0;

err_phy:
	dm_gpio_free(dev, &priv->reset_gpio);
err_gpio:
	reset_free(&priv->apps_reset);
err_reset:
	clk_release_bulk(&priv->clks);

	return ret;
}

static const struct dm_pci_ops pcie_dw_imx_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id pcie_dw_imx_ids[] = {
	{ .compatible = "fsl,imx8mm-pcie", .data = (ulong)&imx8mm_chip_info, },
	{ .compatible = "fsl,imx8mp-pcie", .data = (ulong)&imx8mp_chip_info, },
	{ }
};

U_BOOT_DRIVER(pcie_dw_imx) = {
	.name		= "pcie_dw_imx",
	.id		= UCLASS_PCI,
	.of_match	= pcie_dw_imx_ids,
	.ops		= &pcie_dw_imx_ops,
	.of_to_plat	= pcie_dw_imx_of_to_plat,
	.probe		= pcie_dw_imx_probe,
	.remove		= pcie_dw_imx_remove,
	.priv_auto	= sizeof(struct pcie_dw_imx),
};

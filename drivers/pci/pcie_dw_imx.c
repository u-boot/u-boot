// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Linaro Ltd.
 * Copyright 2025 NXP
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

#define IMX95_PCIE_PHY_GEN_CTRL			0x0
#define IMX95_PCIE_REF_USE_PAD			BIT(17)

#define IMX95_PCIE_PHY_MPLLA_CTRL		0x10
#define IMX95_PCIE_PHY_MPLL_STATE		BIT(30)

#define IMX95_PCIE_SS_RW_REG_0			0xf0
#define IMX95_PCIE_REF_CLKEN			BIT(23)
#define IMX95_PCIE_PHY_CR_PARA_SEL		BIT(9)
#define IMX95_PCIE_SS_RW_REG_1			0xf4
#define IMX95_PCIE_CLKREQ_OVERRIDE_EN		BIT(8)
#define IMX95_PCIE_CLKREQ_OVERRIDE_VAL		BIT(9)
#define IMX95_PCIE_SYS_AUX_PWR_DET		BIT(31)

#define IMX95_PE0_GEN_CTRL_1			0x1050
#define IMX95_PCIE_DEVICE_TYPE			GENMASK(3, 0)

#define IMX95_PE0_GEN_CTRL_3			0x1058
#define IMX95_PCIE_LTSSM_EN			BIT(0)

#define IMX95_PCIE_RST_CTRL			0x3010
#define IMX95_PCIE_COLD_RST			BIT(0)

#define GEN3_RELATED_OFF			0x890
#define GEN3_RELATED_OFF_GEN3_ZRXDC_NONCOMPL	BIT(0)
#define GEN3_RELATED_OFF_RXEQ_RGRDLESS_RXTS	BIT(13)
#define GEN3_RELATED_OFF_GEN3_EQ_DISABLE	BIT(16)
#define GEN3_RELATED_OFF_RATE_SHADOW_SEL_SHIFT	24
#define GEN3_RELATED_OFF_RATE_SHADOW_SEL_MASK	GENMASK(25, 24)
#define GEN3_RELATED_OFF_RATE_SHADOW_SEL_16_0GT	0x1

#define IMX_PCIE_FLAG_HAS_PHYDRV        BIT(3)
#define IMX_PCIE_FLAG_HAS_APP_RESET     BIT(4)
#define IMX_PCIE_FLAG_HAS_SERDES        BIT(6)

#define IMX_PCIE_MAX_INSTANCES	2

/* Parameters for the waiting for PCIe PHY PLL to lock s*/
#define PHY_PLL_LOCK_WAIT_USLEEP_MAX	200
#define PHY_PLL_LOCK_WAIT_TIMEOUT	(2000 * PHY_PLL_LOCK_WAIT_USLEEP_MAX / 1000)

struct pcie_dw_imx {
	/* Must be first member of the struct */
	struct pcie_dw			dw;
	struct regmap			*iomuxc_gpr;
	struct clk_bulk			clks;
	struct gpio_desc		reset_gpio;
	struct reset_ctl		apps_reset;
	struct phy			phy;
	struct udevice			*vpcie;
	void				*info;
	u32				max_link_speed;
	bool				enable_ext_refclk;
	bool				supports_clkreq;
};

struct pcie_chip_info {
	u32 flags;
	const u32 ltssm_off;
	const u32 ltssm_mask;
	const u32 mode_off[IMX_PCIE_MAX_INSTANCES];
	const u32 mode_mask[IMX_PCIE_MAX_INSTANCES];
	const char *gpr;
	void (*init_phy)(struct pcie_dw_imx *priv);
	int (*enable_ref_clk)(struct pcie_dw_imx *priv, bool enable);
	int (*core_reset)(struct pcie_dw_imx *priv, bool assert);
	int (*wait_pll_lock)(struct pcie_dw_imx *priv);
	void (*post_config)(struct pcie_dw_imx *priv);
};

static void imx95_pcie_init_phy(struct pcie_dw_imx *priv)
{
/*
 * Workaround for ERR051624: The Controller Without Vaux Cannot
 * Exit L23 Ready Through Beacon or PERST# De-assertion
 *
 * When the auxiliary power is not available the controller
 * cannot exit from L23 Ready with beacon or PERST# de-assertion
 * when main power is not removed.
 *
 * Workaround: Set SS_RW_REG_1[SYS_AUX_PWR_DET] to 1.
 */
	regmap_update_bits(priv->iomuxc_gpr, IMX95_PCIE_SS_RW_REG_1,
			   IMX95_PCIE_SYS_AUX_PWR_DET,
			   IMX95_PCIE_SYS_AUX_PWR_DET);

	regmap_update_bits(priv->iomuxc_gpr, IMX95_PCIE_SS_RW_REG_0,
			   IMX95_PCIE_PHY_CR_PARA_SEL,
			   IMX95_PCIE_PHY_CR_PARA_SEL);

	if (priv->enable_ext_refclk) {
		/* External clock is used as reference clock */
		regmap_update_bits(priv->iomuxc_gpr, IMX95_PCIE_PHY_GEN_CTRL,
				   IMX95_PCIE_REF_USE_PAD,
				   IMX95_PCIE_REF_USE_PAD);
		regmap_update_bits(priv->iomuxc_gpr, IMX95_PCIE_SS_RW_REG_0,
				   IMX95_PCIE_REF_CLKEN, 0);
	} else {
		regmap_update_bits(priv->iomuxc_gpr, IMX95_PCIE_PHY_GEN_CTRL,
				   IMX95_PCIE_REF_USE_PAD, 0);

		regmap_update_bits(priv->iomuxc_gpr, IMX95_PCIE_SS_RW_REG_0,
				   IMX95_PCIE_REF_CLKEN, IMX95_PCIE_REF_CLKEN);
	}

	/* Force CLKREQ# low by override */
	if (!priv->supports_clkreq)
		regmap_update_bits(priv->iomuxc_gpr,
				   IMX95_PCIE_SS_RW_REG_1,
				   IMX95_PCIE_CLKREQ_OVERRIDE_EN |
				   IMX95_PCIE_CLKREQ_OVERRIDE_VAL,
				   IMX95_PCIE_CLKREQ_OVERRIDE_EN |
				   IMX95_PCIE_CLKREQ_OVERRIDE_VAL);
}

static int imx95_pcie_wait_for_phy_pll_lock(struct pcie_dw_imx *priv)
{
	u32 val;

	if (regmap_read_poll_timeout(priv->iomuxc_gpr,
				     IMX95_PCIE_PHY_MPLLA_CTRL, val,
				     val & IMX95_PCIE_PHY_MPLL_STATE,
				     PHY_PLL_LOCK_WAIT_USLEEP_MAX,
				     PHY_PLL_LOCK_WAIT_TIMEOUT)) {
		printf("PCIe PLL lock timeout\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int imx95_pcie_core_reset(struct pcie_dw_imx *priv, bool assert)
{
	u32 val;

	if (assert) {
		/*
		 * From i.MX95 PCIe PHY perspective, the COLD reset toggle
		 * should be complete after power-up by the following sequence.
		 *                 > 10us(at power-up)
		 *                 > 10ns(warm reset)
		 *               |<------------>|
		 *                ______________
		 * phy_reset ____/              \________________
		 *                                   ____________
		 * ref_clk_en_______________________/
		 * Toggle COLD reset aligned with this sequence for i.MX95 PCIe.
		 */
		regmap_update_bits(priv->iomuxc_gpr, IMX95_PCIE_RST_CTRL,
				   IMX95_PCIE_COLD_RST, IMX95_PCIE_COLD_RST);
		/*
		 * Make sure the write to IMX95_PCIE_RST_CTRL is flushed to the
		 * hardware by doing a read. Otherwise, there is no guarantee
		 * that the write has reached the hardware before udelay().
		 */
		regmap_read(priv->iomuxc_gpr, IMX95_PCIE_RST_CTRL, &val);
		udelay(15);
		regmap_update_bits(priv->iomuxc_gpr, IMX95_PCIE_RST_CTRL,
				   IMX95_PCIE_COLD_RST, 0);
		regmap_read(priv->iomuxc_gpr, IMX95_PCIE_RST_CTRL, &val);
		udelay(10);
	}

	return 0;
}

static void imx95_pcie_post_config(struct pcie_dw_imx *priv)
{
	u32 val;

	/*
	 * Workaround for ERR051586: Compliance with 8GT/s Receiver
	 * Impedance ECN
	 *
	 * The default value of GEN3_RELATED_OFF[GEN3_ZRXDC_NONCOMPL] is
	 * 1 which makes receiver non-compliant with the ZRX-DC
	 * parameter for 2.5 GT/s when operating at 8 GT/s or higher. It
	 * causes unnecessary timeout in L1.
	 *
	 * Workaround: Program GEN3_RELATED_OFF[GEN3_ZRXDC_NONCOMPL] to 0.
	 */
	dw_pcie_dbi_write_enable(&priv->dw, true);
	val = readl(priv->dw.dbi_base + GEN3_RELATED_OFF);
	val &= ~GEN3_RELATED_OFF_GEN3_ZRXDC_NONCOMPL;
	writel(val, priv->dw.dbi_base + GEN3_RELATED_OFF);
	dw_pcie_dbi_write_enable(&priv->dw, false);
}

static int imx8mm_pcie_enable_ref_clk(struct pcie_dw_imx *priv, bool enable)
{
	regmap_update_bits(priv->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE,
			   enable ? 0 : IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE);
	regmap_update_bits(priv->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE_EN,
			   enable ? IMX8M_GPR_PCIE_CLK_REQ_OVERRIDE_EN : 0);
	return 0;
}

static const struct pcie_chip_info imx8mm_chip_info = {
	.flags = IMX_PCIE_FLAG_HAS_APP_RESET | IMX_PCIE_FLAG_HAS_PHYDRV,
	.gpr = "fsl,imx8mm-iomuxc-gpr",
	.enable_ref_clk = imx8mm_pcie_enable_ref_clk,
};

static const struct pcie_chip_info imx8mp_chip_info = {
	.flags = IMX_PCIE_FLAG_HAS_APP_RESET | IMX_PCIE_FLAG_HAS_PHYDRV,
	.gpr = "fsl,imx8mp-iomuxc-gpr",
	.enable_ref_clk = imx8mm_pcie_enable_ref_clk,
};

static const struct pcie_chip_info imx95_chip_info = {
	.flags = IMX_PCIE_FLAG_HAS_SERDES,
	.ltssm_off = IMX95_PE0_GEN_CTRL_3,
	.ltssm_mask = IMX95_PCIE_LTSSM_EN,
	.mode_off[0]  = IMX95_PE0_GEN_CTRL_1,
	.mode_mask[0] = IMX95_PCIE_DEVICE_TYPE,
	.init_phy = imx95_pcie_init_phy,
	.core_reset = imx95_pcie_core_reset,
	.wait_pll_lock = imx95_pcie_wait_for_phy_pll_lock,
	.post_config = imx95_pcie_post_config,
};

static void imx_pcie_configure_type(struct pcie_dw_imx *priv)
{
	struct pcie_chip_info *info = (struct pcie_chip_info *)(priv->info);
	unsigned int mask, val, mode;

	mode = PCI_EXP_TYPE_ROOT_PORT;

	/* If mode_mask is 0, then generic PHY driver is used to set the mode */
	if (!info->mode_mask[0])
		return;

	mask = info->mode_mask[0];
	val = mode << (ffs(mask) - 1);

	regmap_update_bits(priv->iomuxc_gpr, info->mode_off[0], mask, val);
}

static void pcie_dw_configure(struct pcie_dw_imx *priv, u32 cap_speed)
{
	dw_pcie_dbi_write_enable(&priv->dw, true);

	clrsetbits_le32(priv->dw.dbi_base + PCIE_LINK_CAPABILITY,
			TARGET_LINK_SPEED_MASK, cap_speed);

	clrsetbits_le32(priv->dw.dbi_base + PCIE_LINK_CTL_2,
			TARGET_LINK_SPEED_MASK, cap_speed);

	dw_pcie_dbi_write_enable(&priv->dw, false);
}

static void imx_pcie_ltssm_enable(struct pcie_dw_imx *priv)
{
	struct pcie_chip_info *info = (struct pcie_chip_info *)(priv->info);

	if (info->ltssm_mask)
		regmap_update_bits(priv->iomuxc_gpr, info->ltssm_off, info->ltssm_mask,
				   info->ltssm_mask);

	if (info->flags & IMX_PCIE_FLAG_HAS_APP_RESET)
		reset_deassert(&priv->apps_reset);
}

static void imx_pcie_ltssm_disable(struct pcie_dw_imx *priv)
{
	struct pcie_chip_info *info = (struct pcie_chip_info *)(priv->info);

	if (info->ltssm_mask)
		regmap_update_bits(priv->iomuxc_gpr, info->ltssm_off,
				   info->ltssm_mask, 0);

	if (info->flags & IMX_PCIE_FLAG_HAS_APP_RESET)
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
	struct pcie_chip_info *info = (struct pcie_chip_info *)(priv->info);

	if (info->core_reset)
		info->core_reset(priv, true);

	if (dm_gpio_is_valid(&priv->reset_gpio)) {
		dm_gpio_set_value(&priv->reset_gpio, 1);
		mdelay(20);
	}

	return reset_assert(&priv->apps_reset);
}

static int imx_pcie_clk_enable(struct pcie_dw_imx *priv)
{
	int ret;
	struct pcie_chip_info *info = (struct pcie_chip_info *)(priv->info);

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		return ret;

	/*
	 * Set the over ride low and enabled make sure that
	 * REF_CLK is turned on.
	 */
	if (info->enable_ref_clk)
		info->enable_ref_clk(priv, true);

	/* allow the clocks to stabilize */
	udelay(500);

	return 0;
}

static void imx_pcie_deassert_core_reset(struct pcie_dw_imx *priv)
{
	struct pcie_chip_info *info = (struct pcie_chip_info *)(priv->info);

	if (info->core_reset)
		info->core_reset(priv, false);

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
	struct pcie_chip_info *info = (void *)dev_get_driver_data(dev);
	int ret;

	if (priv->vpcie) {
		ret = regulator_set_enable_if_allowed(priv->vpcie, true);
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

	if (info->init_phy)
		info->init_phy(priv);

	imx_pcie_configure_type(priv);

	ret = imx_pcie_clk_enable(priv);
	if (ret) {
		dev_err(dev, "failed to enable clocks\n");
		goto err_clk;
	}

	if (info->flags & IMX_PCIE_FLAG_HAS_PHYDRV) {
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
	}

	imx_pcie_deassert_core_reset(priv);

	if (info->wait_pll_lock) {
		ret = info->wait_pll_lock(priv);
		if (ret) {
			dev_err(dev, "failed to wait pll lock\n");
			goto err_link;
		}
	}

	if (info->post_config)
		info->post_config(priv);

	priv->dw.first_busno = dev_seq(dev);
	priv->dw.dev = dev;
	pcie_dw_setup_host(&priv->dw);

	if (pcie_link_up(priv, priv->max_link_speed)) {
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
	if (info->flags & IMX_PCIE_FLAG_HAS_PHYDRV)
		generic_shutdown_phy(&priv->phy);
err_phy_power:
	if (info->flags & IMX_PCIE_FLAG_HAS_PHYDRV)
		generic_phy_exit(&priv->phy);
err_phy_init:
	clk_release_bulk(&priv->clks);
err_clk:
	imx_pcie_deassert_core_reset(priv);

	dm_gpio_free(dev, &priv->reset_gpio);

	if (priv->vpcie)
		regulator_set_enable_if_allowed(priv->vpcie, false);

	return ret;
}

static int pcie_dw_imx_remove(struct udevice *dev)
{
	struct pcie_dw_imx *priv = dev_get_priv(dev);
	struct pcie_chip_info *info = (void *)dev_get_driver_data(dev);

	if (info->flags & IMX_PCIE_FLAG_HAS_PHYDRV)
		generic_shutdown_phy(&priv->phy);

	dm_gpio_free(dev, &priv->reset_gpio);
	if (info->flags & IMX_PCIE_FLAG_HAS_APP_RESET)
		reset_free(&priv->apps_reset);

	clk_release_bulk(&priv->clks);

	if (priv->vpcie)
		regulator_set_enable_if_allowed(priv->vpcie, false);

	return 0;
}

static int pcie_dw_imx_of_to_plat(struct udevice *dev)
{
	struct pcie_chip_info *info = (void *)dev_get_driver_data(dev);
	struct pcie_dw_imx *priv = dev_get_priv(dev);
	ofnode gpr;
	int ret, index;

	priv->info = info;

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

	priv->dw.atu_base = (void *)dev_read_addr_name_ptr(dev, "atu");
	if (!priv->dw.atu_base)
		dev_dbg(dev, "failed to get atu address from dtb\n");

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "failed to get PCIe clks\n");
		return ret;
	}

	index = ofnode_stringlist_search(dev_ofnode(dev), "clock-names", "ext-ref");
	if (index < 0)
		priv->enable_ext_refclk = false;
	else
		priv->enable_ext_refclk = true;

	if (info->flags & IMX_PCIE_FLAG_HAS_APP_RESET) {
		ret = reset_get_by_name(dev, "apps", &priv->apps_reset);
		if (ret) {
			dev_err(dev,
				"Failed to get PCIe apps reset control\n");
			goto err_reset;
		}
	}

	ret = gpio_request_by_name(dev, "reset-gpio", 0, &priv->reset_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		dev_err(dev, "unable to get reset-gpio\n");
		goto err_gpio;
	}

	if (info->flags & IMX_PCIE_FLAG_HAS_PHYDRV) {
		ret = generic_phy_get_by_name(dev, "pcie-phy", &priv->phy);
		if (ret) {
			dev_err(dev, "failed to get pcie phy\n");
			goto err_phy;
		}
	}

	if (info->flags & IMX_PCIE_FLAG_HAS_SERDES) {
		void __iomem *app_base;
		fdt_size_t app_size;
		struct regmap_config config;

		app_base = (void *)dev_read_addr_size_name(dev, "app", &app_size);
		if ((fdt_addr_t)app_base == FDT_ADDR_T_NONE) {
			dev_err(dev, "failed to get app_base address\n");
			return -EINVAL;
		}

		config.r_start = (ulong)app_base;
		config.r_size = (ulong)app_size;
		config.reg_offset_shift = 0;
		config.width = REGMAP_SIZE_32;

		priv->iomuxc_gpr = devm_regmap_init(dev, NULL, NULL, &config);
		if (IS_ERR(priv->iomuxc_gpr)) {
			dev_err(dev, "unable to remap gpr\n");
			ret = PTR_ERR(priv->iomuxc_gpr);
			goto err_phy;
		}
	}

	if (info->gpr) {
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
	}

	priv->max_link_speed = dev_read_u32_default(dev, "fsl,max-link-speed", LINK_SPEED_GEN_1);

	priv->supports_clkreq = dev_read_bool(dev, "supports-clkreq");

	/* vpcie-supply regulator is optional */
	device_get_supply_regulator(dev, "vpcie-supply", &priv->vpcie);

	return 0;

err_phy:
	dm_gpio_free(dev, &priv->reset_gpio);
err_gpio:
	if (info->flags & IMX_PCIE_FLAG_HAS_APP_RESET)
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
	{ .compatible = "fsl,imx95-pcie", .data = (ulong)&imx95_chip_info, },
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

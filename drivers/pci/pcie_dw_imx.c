// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Texas Instruments, Inc
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <generic-phy.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>

#include "pcie_dw_common.h"

DECLARE_GLOBAL_DATA_PTR;

/* IOMUXC offsets */
#define IOMUXC_GPR12			0x30
#define  IMX8MQ_GPR_PCIE2_DEVICE_TYPE_MASK	(0xf << 8)
#define  IMX8MQ_GPR_PCIE2_DEVICE_TYPE_RC	(0x4 << 8)
#define  IMX8MQ_GPR_PCIE1_DEVICE_TYPE_MASK	(0xf << 12)
#define  IMX8MQ_GPR_PCIE1_DEVICE_TYPE_RC	(0x4 << 12)
#define IOMUXC_GPR14			0x38
#define IOMUXC_GPR16			0x40
#define  IMX8MQ_GPR_PCIE_REF_USE_PAD		(1 << 9)
#define  IMX8MQ_GPR_PCIE_CLK_REQ_OVERRIDE_EN	(1 << 10)
#define  IMX8MQ_GPR_PCIE_CLK_REQ_OVERRIDE	(1 << 11)

/* Anatop */
#define ANATOP_PLLOUT_CTL		0x74
#define  ANATOP_PLLOUT_CTL_CKE			(1 << 4)
#define  ANATOP_PLLOUT_CTL_SEL_SYSPLL1		0xb
#define  ANATOP_PLLOUT_CTL_SEL_MASK		0xf
#define ANATOP_PLLOUT_DIV		0x7c
#define  ANATOP_PLLOUT_DIV_SYSPLL1		0x7

#define PLR_OFFSET			0x700
#define PCIE_PORT_DEBUG0		(PLR_OFFSET + 0x28)
#define PCIE_PORT_DEBUG1		(PLR_OFFSET + 0x2c)
#define PCIE_PHY_DEBUG_R1_LINK_UP		(1 << 4)
#define PCIE_PHY_DEBUG_R1_LINK_IN_TRAINING	(1 << 29)

#define PCIE_LINK_UP_TIMEOUT_MS		100

/**
 * struct pcie_dw_imx - i.MX DW PCIe controller state
 */
struct pcie_dw_imx {
	/* Must be first member of the struct */
	struct pcie_dw dw;
	uint32_t ctrl_id;
	bool internal_refclk;
	uint32_t link_gen;

	/* Clocks */
	struct clk *pcie_bus;
	struct clk *pcie_phy;
	struct clk *pcie;
	struct clk *pcie_aux;

	/* Resets */
	struct reset_ctl pciephy_ctl;
	struct reset_ctl apps_ctl;
	struct reset_ctl turnoff_ctl;
	int has_turnoff_ctl;

	/* GPIO */
	struct gpio_desc clkreq_gpio;
	struct gpio_desc disable_gpio;
	struct gpio_desc reset_gpio;

	/* regmap */
	struct regmap *anatop;
	struct regmap *gpr;
};

/**
 * pcie_dw_configure() - Configure link capabilities and speed
 *
 * @regs_base: A pointer to the PCIe controller registers
 * @cap_speed: The capabilities and speed to configure
 *
 * Configure the link capabilities and speed in the PCIe root complex.
 */
static void pcie_dw_configure(struct pcie_dw_imx *pci, u32 cap_speed)
{
	u32 val;

	dw_pcie_dbi_write_enable(&pci->dw, true);

	val = readl(pci->dw.dbi_base + PCIE_LINK_CAPABILITY);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	writel(val, pci->dw.dbi_base + PCIE_LINK_CAPABILITY);

	dw_pcie_dbi_write_enable(&pci->dw, false);
}

/**
 * is_link_up() - Return the link state
 *
 * @regs_base: A pointer to the PCIe DBICS registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link
 */
static int is_link_up(struct pcie_dw_imx *pci)
{
	u32 val;

	val = readl(pci->dw.dbi_base + PCIE_PORT_DEBUG1);
	if ((val & PCIE_PHY_DEBUG_R1_LINK_UP) != 0 &&
	    (val & PCIE_PHY_DEBUG_R1_LINK_IN_TRAINING) == 0)
		return 1;

	return 0;
}

/**
 * wait_link_up() - Wait for the link to come up
 *
 * @regs_base: A pointer to the PCIe controller registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link (timeout)
 */
static int wait_link_up(struct pcie_dw_imx *pci)
{
	unsigned long timeout;

	timeout = get_timer(0) + PCIE_LINK_UP_TIMEOUT_MS;
	while (!is_link_up(pci)) {
		if (get_timer(0) > timeout)
			return 0;
	};

	return 1;
}

static void pcie_dw_imx_init_phy(struct pcie_dw_imx *pci)
{
	struct udevice *dev = pci->dw.dev;
	u32 val;

	if (device_is_compatible(dev, "fsl,imx8mq-pcie")) {
		if (pci->ctrl_id == 0)
			val = IOMUXC_GPR14;
		else
			val = IOMUXC_GPR16;

		if (!pci->internal_refclk) {
			regmap_update_bits(pci->gpr, val,
					   IMX8MQ_GPR_PCIE_REF_USE_PAD,
					   IMX8MQ_GPR_PCIE_REF_USE_PAD);
		} else {
			regmap_update_bits(pci->gpr, val,
					   IMX8MQ_GPR_PCIE_REF_USE_PAD,
					   0);

			regmap_write(pci->anatop, ANATOP_PLLOUT_CTL,
				     ANATOP_PLLOUT_CTL_CKE |
				     ANATOP_PLLOUT_CTL_SEL_SYSPLL1);
			regmap_write(pci->anatop, ANATOP_PLLOUT_DIV,
				     ANATOP_PLLOUT_DIV_SYSPLL1);
		}

		if (pci->ctrl_id == 0) {
			regmap_update_bits(pci->gpr,
					   IOMUXC_GPR12,
					   IMX8MQ_GPR_PCIE1_DEVICE_TYPE_MASK,
					   IMX8MQ_GPR_PCIE1_DEVICE_TYPE_RC);
		} else {
			regmap_update_bits(pci->gpr,
					   IOMUXC_GPR12,
					   IMX8MQ_GPR_PCIE2_DEVICE_TYPE_MASK,
					   IMX8MQ_GPR_PCIE2_DEVICE_TYPE_RC);
		}
	}
}

static int pcie_dw_imx_enable_ref_clk(struct pcie_dw_imx *pci)
{
	struct udevice *dev = pci->dw.dev;
	unsigned int offset;
	int ret = 0;

	if (device_is_compatible(dev, "fsl,imx8mq-pcie")) {
		ret = clk_prepare_enable(pci->pcie_aux);
		if (ret) {
			dev_err(dev, "unable to enable pcie_aux clock\n");
			return ret;
		}

		if (pci->ctrl_id == 0)
			offset = IOMUXC_GPR14;
		else
			offset = IOMUXC_GPR16;

		regmap_update_bits(pci->gpr, offset,
				   IMX8MQ_GPR_PCIE_CLK_REQ_OVERRIDE,
				   0);
		regmap_update_bits(pci->gpr, offset,
				   IMX8MQ_GPR_PCIE_CLK_REQ_OVERRIDE_EN,
				   IMX8MQ_GPR_PCIE_CLK_REQ_OVERRIDE_EN);
	}

	return ret;
}

static int pcie_dw_imx_clk_enable(struct pcie_dw_imx *pci)
{
	struct udevice *dev = pci->dw.dev;
	int ret;

	ret = clk_prepare_enable(pci->pcie_phy);
	if (ret) {
		dev_err(dev, "unable to enable pcie_phy clock\n");
		return ret;
	}

	ret = clk_prepare_enable(pci->pcie_bus);
	if (ret) {
		dev_err(dev, "unable to enable pcie_bus clock\n");
		goto err_pcie_bus;
	}

	ret = clk_prepare_enable(pci->pcie);
	if (ret) {
		dev_err(dev, "unable to enable pcie clock\n");
		goto err_pcie;
	}

	ret = pcie_dw_imx_enable_ref_clk(pci);
	if (ret) {
		dev_err(dev, "unable to enable pcie ref clock\n");
		goto err_ref_clk;
	}

	udelay(200);
	return 0;

err_ref_clk:
	clk_disable_unprepare(pci->pcie);
err_pcie:
	clk_disable_unprepare(pci->pcie_bus);
err_pcie_bus:
	clk_disable_unprepare(pci->pcie_phy);

	return ret;
}

static void pcie_dw_imx_assert_core_reset(struct pcie_dw_imx *pci)
{
	struct udevice *dev = pci->dw.dev;

	if (device_is_compatible(dev, "fsl,imx8mq-pcie")) {
		reset_assert(&pci->pciephy_ctl);
		reset_assert(&pci->apps_ctl);
	}

	if (dm_gpio_is_valid(&pci->reset_gpio))
		dm_gpio_set_value(&pci->reset_gpio, 1);
}

static void pcie_dw_imx_deassert_core_reset(struct pcie_dw_imx *pci)
{
	struct udevice *dev = pci->dw.dev;

	if (device_is_compatible(dev, "fsl,imx8mq-pcie"))
		reset_deassert(&pci->pciephy_ctl);

	if (dm_gpio_is_valid(&pci->reset_gpio)) {
		mdelay(100);
		dm_gpio_set_value(&pci->reset_gpio, 0);
		mdelay(100);
	}
}

static void pcie_dw_imx_ltssm_enable(struct pcie_dw_imx *pci)
{
	struct udevice *dev = pci->dw.dev;

	if (device_is_compatible(dev, "fsl,imx8mq-pcie"))
		reset_deassert(&pci->apps_ctl);
}

static int pcie_dw_imx_establish_link(struct pcie_dw_imx *pci)
{
	u32 val;

	pcie_dw_configure(pci, LINK_SPEED_GEN_1);
	pcie_dw_imx_ltssm_enable(pci);

	if (!wait_link_up(pci))
		return -ENODEV;

	if (pci->link_gen == 2) {
		pcie_dw_configure(pci, LINK_SPEED_GEN_2);
		val = readl(pci->dw.dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL);
		val |= PORT_LOGIC_SPEED_CHANGE;
		writel(val, pci->dw.dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL);
		if (!wait_link_up(pci))
			return -ENODEV;
	}

	return 0;
}

static int pcie_dw_imx_host_init(struct udevice *dev)
{
	struct pcie_dw_imx *priv = dev_get_priv(dev);
	int ret;

	pcie_dw_imx_assert_core_reset(priv);
	pcie_dw_imx_init_phy(priv);

	ret = pcie_dw_imx_clk_enable(priv);
	if (ret) {
		dev_err(dev, "unable to enable pcie clocks: %d\n", ret);
		return ret;
	}

	pcie_dw_imx_deassert_core_reset(priv);
	pcie_dw_setup_host(&priv->dw);
	return pcie_dw_imx_establish_link(priv);
}

static int pcie_dw_imx_parse_dt(struct udevice *dev)
{
	struct pcie_dw_imx *priv = dev_get_priv(dev);
	struct udevice *syscon;
	int ret;

	priv->dw.dbi_base = (void *)dev_read_addr_index(dev, 0);
	if (!priv->dw.dbi_base)
		return -ENODEV;

	dev_dbg(dev, "DBI address is 0x%p\n", priv->dw.dbi_base);

	priv->dw.cfg_base = (void *)dev_read_addr_size_index(dev, 1,
							     &priv->dw.cfg_size);
	if (!priv->dw.cfg_base)
		return -ENODEV;

	dev_dbg(dev, "CFG address is 0x%p+%llx\n", priv->dw.cfg_base, priv->dw.cfg_size);

	if (dev_read_u32(dev, "linux,pci-domain", &priv->ctrl_id) < 0)
		priv->ctrl_id = 0;
	priv->internal_refclk = dev_read_bool(dev, "internal-refclk");
	if (dev_read_u32(dev, "fsl,max-link-speed", &priv->link_gen) < 0)
		priv->link_gen = 1;

	ret = reset_get_by_name(dev, "pciephy", &priv->pciephy_ctl);
	if (ret) {
		dev_err(dev, "failed to get reset for pciephy\n");
		return ret;
	}

	ret = reset_get_by_name(dev, "apps", &priv->apps_ctl);
	if (ret) {
		dev_err(dev, "failed to get reset for apps\n");
		return ret;
	}

	ret = reset_get_by_name(dev, "turnoff", &priv->turnoff_ctl);
	if (!ret)
		priv->has_turnoff_ctl = 1;

	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "anatop", &syscon);
	if (ret) {
		dev_err(dev, "unable to find anatop\n");
		return ret;
	}

	priv->anatop = syscon_get_regmap(syscon);
	if (IS_ERR(priv->anatop)) {
		dev_err(dev, "failed to get regmap for anatop\n");
		return PTR_ERR(priv->anatop);
	}

	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "gpr", &syscon);
	if (ret) {
		dev_err(dev, "unable to find gpr\n");
		return ret;
	}

	priv->gpr = syscon_get_regmap(syscon);
	if (IS_ERR(priv->gpr)) {
		dev_err(dev, "failed to get regmap for gpr\n");
		return PTR_ERR(priv->gpr);
	}

	gpio_request_by_name(dev, "clkreq-gpio", 0, &priv->clkreq_gpio,
			     GPIOD_IS_OUT);
	if (dm_gpio_is_valid(&priv->clkreq_gpio))
		dm_gpio_set_value(&priv->clkreq_gpio, 1);
	gpio_request_by_name(dev, "disable-gpio", 0, &priv->disable_gpio,
			     GPIOD_IS_OUT);
	if (dm_gpio_is_valid(&priv->disable_gpio))
		dm_gpio_set_value(&priv->disable_gpio, 0);
	gpio_request_by_name(dev, "reset-gpio", 0, &priv->reset_gpio,
			     GPIOD_IS_OUT);

	priv->pcie_bus = devm_clk_get(dev, "pcie_bus");
	if (IS_ERR(priv->pcie_bus)) {
		dev_err(dev, "pcie_bus clock source missing or invalid\n");
		return PTR_ERR(priv->pcie_bus);
	}

	priv->pcie = devm_clk_get(dev, "pcie");
	if (IS_ERR(priv->pcie)) {
		dev_err(dev, "pcie clock source missing or invalid\n");
		return PTR_ERR(priv->pcie);
	}

	if (device_is_compatible(dev, "fsl,imx8mq-pcie")) {
		priv->pcie_aux = devm_clk_get(dev, "pcie_aux");
		if (IS_ERR(priv->pcie_aux)) {
			dev_err(dev, "pcie_aux clock source missing or invalid\n");
			return PTR_ERR(priv->pcie_aux);
		}
	}

	priv->pcie_phy = devm_clk_get(dev, "pcie_phy");
	if (IS_ERR(priv->pcie_phy)) {
		dev_err(dev, "pcie_phy clock source missing or invalid\n");
		return PTR_ERR(priv->pcie_phy);
	}

	return 0;
}

/**
 * pcie_dw_imx_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_dw_imx_probe(struct udevice *dev)
{
	struct pcie_dw_imx *priv = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	int ret = 0;

	priv->dw.first_busno = dev_seq(dev);
	priv->dw.dev = dev;

	ret = pcie_dw_imx_parse_dt(dev);
	if (ret)
		return ret;

	ret = pcie_dw_imx_host_init(dev);
	if (ret) {
		printf("PCIE-%d: Link down\n", dev_seq(dev));
		return -ENODEV;
	}

	dev_info(dev, "PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n",
		 dev_seq(dev), pcie_dw_get_link_speed(&priv->dw),
		 pcie_dw_get_link_width(&priv->dw),
		 hose->first_busno);

	return pcie_dw_prog_outbound_atu_unroll(&priv->dw,
						PCIE_ATU_REGION_INDEX0,
						PCIE_ATU_TYPE_MEM,
						priv->dw.mem.phys_start,
						priv->dw.mem.bus_start,
						priv->dw.mem.size);
}

static const struct dm_pci_ops pcie_dw_imx_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id pcie_dw_imx_ids[] = {
	{ .compatible = "fsl,imx8mq-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_dw_imx) = {
	.name			= "pcie_dw_imx",
	.id			= UCLASS_PCI,
	.of_match		= pcie_dw_imx_ids,
	.ops			= &pcie_dw_imx_ops,
	.probe			= pcie_dw_imx_probe,
	.priv_auto		= sizeof(struct pcie_dw_imx),
};

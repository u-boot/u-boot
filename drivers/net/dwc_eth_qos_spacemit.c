// SPDX-License-Identifier: GPL-2.0+
/*
 * SpacemiT DWMAC / DWC EQOS platform glue
 *
 * Based on Linux dwmac-spacemit.c driver:
 *   Copyright (C) 2026 Inochi Amaoto <inochiama@gmail.com>
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <log.h>
#include <net.h>
#include <phy.h>
#include <regmap.h>
#include <reset.h>

#include "dwc_eth_qos.h"

/* ctrl register bits */
#define CTRL_PHY_INTF_MODE		GENMASK(4, 3)
#define CTRL_PHY_INTF_RMII		FIELD_PREP(CTRL_PHY_INTF_MODE, 0)
#define CTRL_PHY_INTF_RGMII		FIELD_PREP(CTRL_PHY_INTF_MODE, 1)
#define CTRL_PHY_INTF_MII		FIELD_PREP(CTRL_PHY_INTF_MODE, 3)

/* dline register bits */
#define RGMII_RX_DLINE_EN		BIT(0)
#define RGMII_RX_DLINE_STEP		GENMASK(5, 4)
#define RGMII_RX_DLINE_CODE		GENMASK(15, 8)
#define RGMII_TX_DLINE_EN		BIT(16)
#define RGMII_TX_DLINE_STEP		GENMASK(21, 20)
#define RGMII_TX_DLINE_CODE		GENMASK(31, 24)

#define MAX_WORKED_DELAY		2800
/* Note: the delay step value is at 0.1ps */
#define K3_DELAY_STEP			367

struct spacemit_dwmac {
	struct regmap *apmu;
	unsigned int ctrl_offset;
	unsigned int dline_offset;
	struct clk_bulk clks;
};

static int spacemit_dwmac_set_delay(struct spacemit_dwmac *dwmac,
				    unsigned int tx_code, unsigned int rx_code)
{
	unsigned int mask, val;

	mask = RGMII_TX_DLINE_STEP | RGMII_TX_DLINE_CODE | RGMII_TX_DLINE_EN |
	       RGMII_RX_DLINE_STEP | RGMII_RX_DLINE_CODE | RGMII_RX_DLINE_EN;

	/*
	 * Since the delay step provided by config 0 is small enough, and
	 * it can cover the range of the valid delay, so there is no needed
	 * to use other step config.
	 */
	val = FIELD_PREP(RGMII_TX_DLINE_STEP, 0) |
	      FIELD_PREP(RGMII_TX_DLINE_CODE, tx_code) | RGMII_TX_DLINE_EN |
	      FIELD_PREP(RGMII_RX_DLINE_STEP, 0) |
	      FIELD_PREP(RGMII_RX_DLINE_CODE, rx_code) | RGMII_RX_DLINE_EN;

	return regmap_update_bits(dwmac->apmu, dwmac->dline_offset, mask, val);
}

static int spacemit_dwmac_detected_delay_value(unsigned int delay)
{
	if (delay == 0)
		return 0;

	if (delay > MAX_WORKED_DELAY)
		return -EINVAL;

	/*
	 * Note K3 require a specific factor for calculate
	 * the delay, in this scenario it is 0.9. So the
	 * formula is code * step / 10 * 0.9
	 */
	return DIV_ROUND_CLOSEST(delay * 10 * 10, K3_DELAY_STEP * 9);
}

static int spacemit_dwmac_fix_delay(struct spacemit_dwmac *dwmac,
				    unsigned int tx_delay,
				    unsigned int rx_delay)
{
	int rx_code;
	int tx_code;

	rx_code = spacemit_dwmac_detected_delay_value(rx_delay);
	if (rx_code < 0)
		return rx_code;

	tx_code = spacemit_dwmac_detected_delay_value(tx_delay);
	if (tx_code < 0)
		return tx_code;

	return spacemit_dwmac_set_delay(dwmac, tx_code, rx_code);
}

static int spacemit_set_phy_intf_sel(struct spacemit_dwmac *dwmac,
				     phy_interface_t interface)
{
	unsigned int mask = CTRL_PHY_INTF_MODE;
	unsigned int val;

	switch (interface) {
	case PHY_INTERFACE_MODE_MII:
		val = CTRL_PHY_INTF_MII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		val = CTRL_PHY_INTF_RMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		val = CTRL_PHY_INTF_RGMII;
		break;
	default:
		return -EINVAL;
	}

	return regmap_update_bits(dwmac->apmu, dwmac->ctrl_offset, mask, val);
}

static int eqos_start_clks_spacemit(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct spacemit_dwmac *dwmac = pdata->priv_pdata;
	ulong rate;
	int ret;

	ret = clk_enable_bulk(&dwmac->clks);
	if (ret)
		return ret;

	rate = clk_get_rate(&eqos->clk_master_bus);
	if (!rate) {
		pr_err("stmmaceth rate is 0 (AXI/bus clock not running?)\n");
		clk_disable_bulk(&dwmac->clks);
		return -EIO;
	}

	return 0;
}

static int eqos_stop_clks_spacemit(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct spacemit_dwmac *dwmac = pdata->priv_pdata;

	return clk_disable_bulk(&dwmac->clks);
}

static int eqos_start_resets_spacemit(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct spacemit_dwmac *dwmac = pdata->priv_pdata;
	int ret;

	ret = reset_deassert(&eqos->reset_ctl);
	if (ret)
		return ret;

	udelay(100);

	return spacemit_set_phy_intf_sel(dwmac, pdata->phy_interface);
}

static int eqos_stop_resets_spacemit(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	return reset_assert(&eqos->reset_ctl);
}

static int eqos_remove_resources_spacemit(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct spacemit_dwmac *dwmac = pdata->priv_pdata;

	if (!dwmac)
		return 0;

	reset_free(&eqos->reset_ctl);
	if (dwmac->apmu)
		regmap_uninit(dwmac->apmu);
	free(dwmac);
	pdata->priv_pdata = NULL;

	return 0;
}

static ulong eqos_get_tick_clk_rate_spacemit(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_master_bus);
}

static int eqos_probe_resources_spacemit(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ofnode_phandle_args args;
	struct spacemit_dwmac *dwmac;
	u32 rx_delay = 0;
	u32 tx_delay = 0;
	int ret;

	ret = eqos_get_base_addr_dt(dev);
	if (ret) {
		pr_err("eqos_get_base_addr_dt failed: %d\n", ret);
		return ret;
	}

	dwmac = calloc(1, sizeof(*dwmac));
	if (!dwmac)
		return -ENOMEM;

	pdata->priv_pdata = dwmac;

	pdata->phy_interface = eqos->config->interface(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	ret = dev_read_phandle_with_args(dev, "spacemit,apmu", NULL, 2, 0,
					 &args);
	if (ret) {
		pr_err("Failed to parse spacemit,apmu: %d\n", ret);
		return ret;
	}

	if (args.args_count != 2)
		return -EINVAL;

	dwmac->ctrl_offset = args.args[0];
	dwmac->dline_offset = args.args[1];

	ret = regmap_init_mem(args.node, &dwmac->apmu);
	if (ret) {
		pr_err("Failed to init apmu regmap: %d\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "stmmaceth", &eqos->reset_ctl);
	if (ret) {
		pr_err("Failed to get stmmaceth reset: %d\n", ret);
		return ret;
	}

	ret = clk_get_bulk(dev, &dwmac->clks);
	if (ret) {
		pr_err("Failed to get clocks: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(stmmaceth) failed: %d\n", ret);
		return ret;
	}

	dev_read_u32(dev, "tx-internal-delay-ps", &tx_delay);
	dev_read_u32(dev, "rx-internal-delay-ps", &rx_delay);

	ret = spacemit_dwmac_fix_delay(dwmac, tx_delay, rx_delay);
	if (ret) {
		pr_err("Failed to configure delay: %d\n", ret);
		return ret;
	}

	return 0;
}

static struct eqos_ops eqos_spacemit_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_spacemit,
	.eqos_remove_resources = eqos_remove_resources_spacemit,
	.eqos_stop_resets = eqos_stop_resets_spacemit,
	.eqos_start_resets = eqos_start_resets_spacemit,
	.eqos_stop_clks = eqos_stop_clks_spacemit,
	.eqos_start_clks = eqos_start_clks_spacemit,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_null_ops,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_spacemit,
};

struct eqos_config eqos_spacemit_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 500,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_spacemit_ops,
};

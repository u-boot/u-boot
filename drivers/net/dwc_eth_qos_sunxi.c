// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017 Corentin Labbe <clabbe.montjoie@gmail.com>
 * Copyright (C) 2025 Chen-Yu Tsai <wens@csie.org>
 * Copyright (C) 2026 Junhui Liu <junhui.liu@pigmoral.tech>
 */

#include <clk.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <net.h>
#include <phy.h>
#include <power/regulator.h>
#include <regmap.h>
#include <reset.h>
#include <malloc.h>
#include <syscon.h>

#include "dwc_eth_qos.h"

#define SYSCON_REG			0x34

#define SYSCON_PHY_SELECT		BIT(15) /* 1: internal, 0: external */
/* RMII specific bits */
#define SYSCON_RMII_EN			BIT(13) /* 1: enable RMII (overrides EPIT) */
/* Generic system control EMAC_CLK bits */
#define SYSCON_ETXDC_MASK		GENMASK(12, 10)
#define SYSCON_ERXDC_MASK		GENMASK(9, 5)
/* EMAC PHY Interface Type */
#define SYSCON_EPIT			BIT(2) /* 1: RGMII, 0: MII */
#define SYSCON_ETCS_MASK		GENMASK(1, 0)
#define SYSCON_ETCS_MII			0x0
#define SYSCON_ETCS_EXT_GMII		0x1
#define SYSCON_ETCS_INT_GMII		0x2

struct sunxi_platform_data {
	struct clk_bulk clks;
	struct reset_ctl_bulk resets;
};

static int sunxi_gmac_set_syscon(struct udevice *dev,
				 phy_interface_t interface_type)
{
	struct regmap *regmap;
	u32 val, reg = 0;
	int ret;

	regmap = syscon_regmap_lookup_by_phandle(dev, "syscon");
	if (IS_ERR(regmap)) {
		dev_err(dev, "Unable to map syscon\n");
		return PTR_ERR(regmap);
	}

	if (!dev_read_u32(dev, "tx-internal-delay-ps", &val)) {
		if (val % 100) {
			dev_err(dev, "tx-delay must be a multiple of 100\n");
			return -EINVAL;
		}
		val /= 100;
		dev_dbg(dev, "set tx-delay to %x\n", val);

		if (!FIELD_FIT(SYSCON_ETXDC_MASK, val)) {
			dev_err(dev, "TX clock delay exceeds maximum\n");
			return -EINVAL;
		}

		reg |= FIELD_PREP(SYSCON_ETXDC_MASK, val);
	}

	if (!dev_read_u32(dev, "rx-internal-delay-ps", &val)) {
		if (val % 100) {
			dev_err(dev, "rx-delay must be a multiple of 100\n");
			return -EINVAL;
		}
		val /= 100;
		dev_dbg(dev, "set rx-delay to %x\n", val);
		if (!FIELD_FIT(SYSCON_ERXDC_MASK, val)) {
			dev_err(dev, "Invalid RX clock delay: %d\n", val);
			return -EINVAL;
		}

		reg |= FIELD_PREP(SYSCON_ERXDC_MASK, val);
	}

	switch (interface_type) {
	case PHY_INTERFACE_MODE_MII:
		/* default */
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		reg |= SYSCON_EPIT | SYSCON_ETCS_INT_GMII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		reg |= SYSCON_RMII_EN;
		break;
	default:
		dev_err(dev, "Unsupported interface mode: %s\n",
			phy_interface_strings[interface_type]);
		return -EINVAL;
	}

	ret = regmap_write(regmap, SYSCON_REG, reg);
	if (ret < 0) {
		dev_err(dev, "Failed to write to syscon\n");
		return ret;
	}

	return 0;
}

static int eqos_start_clks_sunxi(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct sunxi_platform_data *data = pdata->priv_pdata;

	return clk_enable_bulk(&data->clks);
}

static int eqos_stop_clks_sunxi(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct sunxi_platform_data *data = pdata->priv_pdata;

	return clk_disable_bulk(&data->clks);
}

static int eqos_start_resets_sunxi(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct sunxi_platform_data *data = pdata->priv_pdata;
	int ret;

	ret = reset_deassert_bulk(&data->resets);
	if (ret)
		return ret;

	return sunxi_gmac_set_syscon(dev, pdata->phy_interface);
}

static int eqos_stop_resets_sunxi(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct sunxi_platform_data *data = pdata->priv_pdata;

	return reset_assert_bulk(&data->resets);
}

static int eqos_probe_resources_sunxi(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct sunxi_platform_data *data;
	struct udevice *phy_supply = NULL;
	int ret;

	ret = eqos_get_base_addr_dt(dev);
	if (ret) {
		dev_err(dev, "Failed to get base address: %d\n", ret);
		return ret;
	}

	data = calloc(1, sizeof(struct sunxi_platform_data));
	if (!data)
		return -ENOMEM;

	pdata->priv_pdata = data;

	pdata->phy_interface = eqos->config->interface(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA) {
		dev_err(dev, "Failed to get PHY interface mode\n");
		return -EINVAL;
	}

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		dev_err(dev, "Failed to get stmmaceth clock: %d\n", ret);
		return ret;
	}

	ret = reset_get_bulk(dev, &data->resets);
	if (ret) {
		dev_err(dev, "Failed to get resets: %d\n", ret);
		return ret;
	}

	ret = clk_get_bulk(dev, &data->clks);
	if (ret) {
		dev_err(dev, "Failed to get clocks: %d\n", ret);
		return ret;
	}

	if (IS_ENABLED(CONFIG_DM_REGULATOR)) {
		ret = device_get_supply_regulator(dev, "phy-supply", &phy_supply);
		if (ret && ret != -ENOENT) {
			dev_err(dev, "Failed to get PHY supply: %d\n", ret);
			return ret;
		}

		if (phy_supply) {
			ret = regulator_set_enable_if_allowed(phy_supply, true);
			if (ret) {
				dev_err(dev, "Failed to enable phy supply: %d\n", ret);
				return ret;
			}
		}
	}

	return 0;
}

static int eqos_remove_resources_sunxi(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct sunxi_platform_data *data = pdata->priv_pdata;

	reset_assert_bulk(&data->resets);
	clk_disable_bulk(&data->clks);

	return 0;
}

static struct eqos_ops eqos_sunxi_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_sunxi,
	.eqos_remove_resources = eqos_remove_resources_sunxi,
	.eqos_stop_resets = eqos_stop_resets_sunxi,
	.eqos_start_resets = eqos_start_resets_sunxi,
	.eqos_stop_clks = eqos_stop_clks_sunxi,
	.eqos_start_clks = eqos_start_clks_sunxi,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_null_ops,
	.eqos_get_enetaddr = eqos_null_ops,
};

struct eqos_config __maybe_unused eqos_sunxi_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_150_250,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_sunxi_ops
};

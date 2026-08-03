// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <net.h>
#include <phy.h>
#include <regmap.h>
#include <syscon.h>

#include "dwc_eth_qos.h"

/*
 * Peri Configuration register is SoC specific,
 * so add a SoC specific prefix.
 */
#define MT8189_PERI_ETH_CTRL0		0x270
#define MT8189_PERI_ETH_CTRL1		0x274
#define MT8189_PERI_ETH_CTRL2		0x278

#define EQOS_MTK_RMII_CLK_SRC_INTERNAL	BIT(28)
#define EQOS_MTK_RMII_CLK_SRC_RXC	BIT(27)
#define EQOS_MTK_ETH_INTF_SEL		GENMASK(26, 24)
#define EQOS_MTK_PHY_INTF_MII		0
#define EQOS_MTK_PHY_INTF_RGMII		1
#define EQOS_MTK_PHY_INTF_RMII		4
#define EQOS_MTK_RGMII_TXC_PHASE_CTRL	BIT(22)
#define EQOS_MTK_EXT_PHY_MODE		BIT(21)
#define EQOS_MTK_TXC_OUT_OP		BIT(20)
#define EQOS_MTK_DLY_GTXC_INV		BIT(12)
#define EQOS_MTK_DLY_GTXC_STAGE_FINE	GENMASK(11, 6)
#define EQOS_MTK_DLY_GTXC_ENABLE	BIT(5)
#define EQOS_MTK_DLY_GTXC_STAGES	GENMASK(4, 0)

#define EQOS_MTK_DLY_RXC_INV		BIT(25)
#define EQOS_MTK_DLY_RXC_ENABLE		BIT(18)
#define EQOS_MTK_DLY_RXC_STAGES		GENMASK(17, 13)
#define EQOS_MTK_DLY_TXC_INV		BIT(12)
#define EQOS_MTK_DLY_TXC_ENABLE		BIT(5)
#define EQOS_MTK_DLY_TXC_STAGES		GENMASK(4, 0)

#define EQOS_MTK_DLY_RMII_RXC_INV	BIT(25)
#define EQOS_MTK_DLY_RMII_RXC_ENABLE	BIT(18)
#define EQOS_MTK_DLY_RMII_RXC_STAGES	GENMASK(17, 13)
#define EQOS_MTK_DLY_RMII_TXC_INV	BIT(12)
#define EQOS_MTK_DLY_RMII_TXC_ENABLE	BIT(5)
#define EQOS_MTK_DLY_RMII_TXC_STAGES	GENMASK(4, 0)

#define DELAY_MAX_PS			9800
#define DELAY_PS_PER_STAGE		290

struct eqos_mtk_priv {
	struct regmap *peri_regmap;
	bool rmii_clk_from_mac;
	bool rmii_rxc;
	u32 tx_delay_stage;
	u32 rx_delay_stage;
	bool tx_inv;
	bool rx_inv;
};

static int mtk_clk_init(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	ret = clk_get_by_name(dev, "mac_main", &eqos->clk_tx);
	if (ret) {
		dev_err(dev, "clk_get_by_name(mac_main) failed: %d", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "ptp_ref", &eqos->clk_ptp_ref);
	if (ret) {
		dev_err(dev, "clk_get_by_name(ptp_ref) failed: %d", ret);
		return ret;
	}

	return 0;
}

static int mtk_set_delay(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eqos_mtk_priv *mtk_pdata = pdata->priv_pdata;
	u32 gtxc_delay_val = 0, delay_val = 0, rmii_delay_val = 0;

	switch (pdata->phy_interface) {
	case PHY_INTERFACE_MODE_MII:
		delay_val |= FIELD_PREP(EQOS_MTK_DLY_TXC_ENABLE,
					!!mtk_pdata->tx_delay_stage);
		delay_val |= FIELD_PREP(EQOS_MTK_DLY_TXC_STAGES, mtk_pdata->tx_delay_stage);
		delay_val |= FIELD_PREP(EQOS_MTK_DLY_TXC_INV, mtk_pdata->tx_inv);

		delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_ENABLE,
					!!mtk_pdata->rx_delay_stage);
		delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_STAGES, mtk_pdata->rx_delay_stage);
		delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_INV, mtk_pdata->rx_inv);
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (mtk_pdata->rmii_clk_from_mac) {
			/* case 1: mac provides the rmii reference clock,
			 * and the clock output to TXC pin.
			 * The egress timing can be adjusted by RMII_TXC delay macro circuit.
			 * The ingress timing can be adjusted by RMII_RXC delay macro circuit.
			 */
			rmii_delay_val |= FIELD_PREP(EQOS_MTK_DLY_RMII_TXC_ENABLE,
						     !!mtk_pdata->tx_delay_stage);
			rmii_delay_val |= FIELD_PREP(EQOS_MTK_DLY_RMII_TXC_STAGES,
						     mtk_pdata->tx_delay_stage);
			rmii_delay_val |= FIELD_PREP(EQOS_MTK_DLY_RMII_TXC_INV,
						     mtk_pdata->tx_inv);

			rmii_delay_val |= FIELD_PREP(EQOS_MTK_DLY_RMII_RXC_ENABLE,
						     !!mtk_pdata->rx_delay_stage);
			rmii_delay_val |= FIELD_PREP(EQOS_MTK_DLY_RMII_RXC_STAGES,
						     mtk_pdata->rx_delay_stage);
			rmii_delay_val |= FIELD_PREP(EQOS_MTK_DLY_RMII_RXC_INV,
						     mtk_pdata->rx_inv);
		} else {
			/* case 2: the rmii reference clock is from external phy,
			 * and the property "rmii_rxc" indicates which pin(TXC/RXC)
			 * the reference clk is connected to. The reference clock is a
			 * received signal, so rx_delay_stage/rx_inv are used to indicate
			 * the reference clock timing adjustment
			 */
			if (mtk_pdata->rmii_rxc) {
				/* the rmii reference clock from outside is connected
				 * to RXC pin, the reference clock will be adjusted
				 * by RXC delay macro circuit.
				 */
				delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_ENABLE,
							!!mtk_pdata->rx_delay_stage);
				delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_STAGES,
							mtk_pdata->rx_delay_stage);
				delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_INV,
							mtk_pdata->rx_inv);
			} else {
				/* the rmii reference clock from outside is connected
				 * to TXC pin, the reference clock will be adjusted
				 * by TXC delay macro circuit.
				 */
				delay_val |= FIELD_PREP(EQOS_MTK_DLY_TXC_ENABLE,
							!!mtk_pdata->rx_delay_stage);
				delay_val |= FIELD_PREP(EQOS_MTK_DLY_TXC_STAGES,
							mtk_pdata->rx_delay_stage);
				delay_val |= FIELD_PREP(EQOS_MTK_DLY_TXC_INV,
							mtk_pdata->rx_inv);
			}
		}
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_TXID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_ID:
		gtxc_delay_val |= FIELD_PREP(EQOS_MTK_DLY_GTXC_ENABLE,
					     !!mtk_pdata->tx_delay_stage);
		gtxc_delay_val |= FIELD_PREP(EQOS_MTK_DLY_GTXC_STAGES,
					     mtk_pdata->tx_delay_stage);
		gtxc_delay_val |= FIELD_PREP(EQOS_MTK_DLY_GTXC_INV, mtk_pdata->tx_inv);
		gtxc_delay_val |= EQOS_MTK_DLY_GTXC_STAGE_FINE;

		delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_ENABLE,
					!!mtk_pdata->rx_delay_stage);
		delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_STAGES, mtk_pdata->rx_delay_stage);
		delay_val |= FIELD_PREP(EQOS_MTK_DLY_RXC_INV, mtk_pdata->rx_inv);

		break;
	default:
		dev_err(dev, "phy interface not supported\n");
		return -EINVAL;
	}

	regmap_update_bits(mtk_pdata->peri_regmap,
			   MT8189_PERI_ETH_CTRL0,
			   EQOS_MTK_RGMII_TXC_PHASE_CTRL |
			   EQOS_MTK_DLY_GTXC_ENABLE |
			   EQOS_MTK_DLY_GTXC_INV |
			   EQOS_MTK_DLY_GTXC_STAGE_FINE |
			   EQOS_MTK_DLY_GTXC_STAGES,
			   gtxc_delay_val);
	regmap_write(mtk_pdata->peri_regmap, MT8189_PERI_ETH_CTRL1, delay_val);
	regmap_write(mtk_pdata->peri_regmap, MT8189_PERI_ETH_CTRL2, rmii_delay_val);

	return 0;
}

static int mtk_set_interface(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eqos_mtk_priv *mtk_pdata = pdata->priv_pdata;
	int rmii_clk_from_mac = mtk_pdata->rmii_clk_from_mac ? EQOS_MTK_RMII_CLK_SRC_INTERNAL : 0;
	int rmii_rxc = mtk_pdata->rmii_rxc ? EQOS_MTK_RMII_CLK_SRC_RXC : 0;
	u32 intf_val = 0;

	/* select phy interface in top control domain */
	switch (pdata->phy_interface) {
	case PHY_INTERFACE_MODE_MII:
		intf_val |= FIELD_PREP(EQOS_MTK_ETH_INTF_SEL, EQOS_MTK_PHY_INTF_MII);
		break;
	case PHY_INTERFACE_MODE_RMII:
		intf_val |= (rmii_rxc | rmii_clk_from_mac);
		intf_val |= FIELD_PREP(EQOS_MTK_ETH_INTF_SEL, EQOS_MTK_PHY_INTF_RMII);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_TXID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_ID:
		intf_val |= FIELD_PREP(EQOS_MTK_ETH_INTF_SEL, EQOS_MTK_PHY_INTF_RGMII);
		break;
	default:
		dev_err(dev, "phy interface not supported\n");
		return -EINVAL;
	}

	/* only support external PHY */
	intf_val |= EQOS_MTK_EXT_PHY_MODE;

	intf_val |= EQOS_MTK_TXC_OUT_OP;

	regmap_write(mtk_pdata->peri_regmap, MT8189_PERI_ETH_CTRL0, intf_val);

	return 0;
}

static int mtk_config_dt(struct udevice *dev)
{	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eqos_mtk_priv *mtk_pdata = pdata->priv_pdata;
	struct ofnode_phandle_args args;
	u32 tx_delay_ps = 0, rx_delay_ps = 0;
	int ret;

	if (!dev_read_u32(dev, "mediatek,tx-delay-ps", &tx_delay_ps)) {
		if (tx_delay_ps > DELAY_MAX_PS) {
			dev_err(dev, "Invalid TX clock delay: %dps\n", tx_delay_ps);
			return -EINVAL;
		}
	}

	if (!dev_read_u32(dev, "mediatek,rx-delay-ps", &rx_delay_ps)) {
		if (rx_delay_ps > DELAY_MAX_PS) {
			dev_err(dev, "Invalid RX clock delay: %dps\n", rx_delay_ps);
			return -EINVAL;
		}
	}

	mtk_pdata->tx_delay_stage = tx_delay_ps / DELAY_PS_PER_STAGE;
	mtk_pdata->rx_delay_stage = rx_delay_ps / DELAY_PS_PER_STAGE;

	mtk_pdata->tx_inv = dev_read_bool(dev, "mediatek,txc-inverse");
	mtk_pdata->rx_inv = dev_read_bool(dev, "mediatek,rxc-inverse");
	mtk_pdata->rmii_clk_from_mac = dev_read_bool(dev, "mediatek,rmii-clk-from-mac");
	mtk_pdata->rmii_rxc = dev_read_bool(dev, "mediatek,rmii-rxc");

	ret = dev_read_phandle_with_args(dev, "mediatek,pericfg", NULL, 0, 0, &args);
	if (ret) {
		dev_err(dev, "Failed to get mediatek,pericfg property: %d\n", ret);
		return ret;
	}

	mtk_pdata->peri_regmap = syscon_node_to_regmap(args.node);
	if (IS_ERR(mtk_pdata->peri_regmap)) {
		dev_err(dev, "fail to get regmap: %d\n", (int)PTR_ERR(mtk_pdata->peri_regmap));
		return PTR_ERR(mtk_pdata->peri_regmap);
	}

	return 0;
}

static int eqos_probe_resources_mtk(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eqos_mtk_priv *mtk_pdata;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = eqos_get_base_addr_dt(dev);
	if (ret) {
		dev_err(dev, "eqos_get_base_addr_dt failed: %d\n", ret);
		return ret;
	}

	mtk_pdata = calloc(1, sizeof(struct eqos_mtk_priv));
	if (!mtk_pdata)
		return -ENOMEM;

	pdata->priv_pdata = mtk_pdata;

	ret = mtk_config_dt(dev);
	if (ret) {
		dev_err(dev, "mtk config dt failed: %d\n", ret);
		goto err;
	}

	ret = mtk_clk_init(dev);
	if (ret)
		goto err;

	pdata->phy_interface = eqos->config->interface(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA) {
		dev_err(dev, "Invalid PHY interface\n");
		ret = -EINVAL;
		goto err;
	}

	ret = mtk_set_interface(dev);
	if (ret)
		goto err;

	ret = mtk_set_delay(dev);
	if (ret)
		goto err;

	debug("%s: OK\n", __func__);
	return 0;
err:
	free(mtk_pdata);
	return ret;
}

static int eqos_remove_resources_mtk(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eqos_mtk_priv *mtk_pdata = pdata->priv_pdata;

	debug("%s(dev=%p):\n", __func__, dev);

	free(mtk_pdata);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_stop_clks_mtk(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_ptp_ref);
	clk_disable(&eqos->clk_tx);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_start_clks_mtk(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		dev_err(dev, "clk_enable(mac_main) failed: %d", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_ptp_ref);
	if (ret < 0) {
		dev_err(dev, "clk_enable(ptp_ref) failed: %d", ret);
		goto err_disable_clk_mac_main;
	}

	debug("%s: OK\n", __func__);
	return 0;

err_disable_clk_mac_main:
	clk_disable(&eqos->clk_tx);
err:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
}

static int eqos_fix_mac_speed_mtk(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eqos_mtk_priv *mtk_pdata = pdata->priv_pdata;

	debug("%s(dev=%p):\n", __func__, dev);

	switch (pdata->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_TXID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_ID:
		if (eqos->phy->speed == SPEED_1000)
			regmap_update_bits(mtk_pdata->peri_regmap,
					   MT8189_PERI_ETH_CTRL0,
					   EQOS_MTK_RGMII_TXC_PHASE_CTRL |
					   EQOS_MTK_DLY_GTXC_ENABLE |
					   EQOS_MTK_DLY_GTXC_INV |
					   EQOS_MTK_DLY_GTXC_STAGE_FINE |
					   EQOS_MTK_DLY_GTXC_STAGES,
					   EQOS_MTK_RGMII_TXC_PHASE_CTRL);
		else
			mtk_set_delay(dev);
		break;
	default:
		debug("%s: dev=%p no need to adjust mac delay\n", __func__, dev);
		break;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static struct eqos_ops eqos_mtk_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_mtk,
	.eqos_remove_resources = eqos_remove_resources_mtk,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = eqos_null_ops,
	.eqos_stop_clks = eqos_stop_clks_mtk,
	.eqos_start_clks = eqos_start_clks_mtk,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_fix_mac_speed_mtk,
	.eqos_get_enetaddr = eqos_null_ops,
};

struct eqos_config eqos_mtk_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 10,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_60_100,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_mtk_ops
};

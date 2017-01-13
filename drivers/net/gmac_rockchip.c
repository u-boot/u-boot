/*
 * (C) Copyright 2015 Sjoerd Simons <sjoerd.simons@collabora.co.uk>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Rockchip GMAC ethernet IP driver for U-Boot
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <phy.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/periph.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk3288.h>
#include <dm/pinctrl.h>
#include <dt-bindings/clock/rk3288-cru.h>
#include "designware.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Platform data for the gmac
 *
 * dw_eth_pdata: Required platform data for designware driver (must be first)
 */
struct gmac_rockchip_platdata {
	struct dw_eth_pdata dw_eth_pdata;
	int tx_delay;
	int rx_delay;
};

static int gmac_rockchip_ofdata_to_platdata(struct udevice *dev)
{
	struct gmac_rockchip_platdata *pdata = dev_get_platdata(dev);

	pdata->tx_delay = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					 "tx-delay", 0x30);
	pdata->rx_delay = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					 "rx-delay", 0x10);

	return designware_eth_ofdata_to_platdata(dev);
}

static int gmac_rockchip_fix_mac_speed(struct dw_eth_dev *priv)
{
	struct rk3288_grf *grf;
	int clk;

	switch (priv->phydev->speed) {
	case 10:
		clk = GMAC_CLK_SEL_2_5M;
		break;
	case 100:
		clk = GMAC_CLK_SEL_25M;
		break;
	case 1000:
		clk = GMAC_CLK_SEL_125M;
		break;
	default:
		debug("Unknown phy speed: %d\n", priv->phydev->speed);
		return -EINVAL;
	}

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->soc_con1,
		     GMAC_CLK_SEL_MASK << GMAC_CLK_SEL_SHIFT,
		     clk << GMAC_CLK_SEL_SHIFT);

	return 0;
}

static int gmac_rockchip_probe(struct udevice *dev)
{
	struct gmac_rockchip_platdata *pdata = dev_get_platdata(dev);
	struct rk3288_grf *grf;
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	/* Since mac_clk is fed by an external clock we can use 0 here */
	ret = clk_set_rate(&clk, 0);
	if (ret)
		return ret;

	/* Set to RGMII mode */
	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrsetreg(&grf->soc_con1,
		     RMII_MODE_MASK << RMII_MODE_SHIFT |
		     GMAC_PHY_INTF_SEL_MASK << GMAC_PHY_INTF_SEL_SHIFT,
		     GMAC_PHY_INTF_SEL_RGMII << GMAC_PHY_INTF_SEL_SHIFT);

	rk_clrsetreg(&grf->soc_con3,
		     RXCLK_DLY_ENA_GMAC_MASK <<  RXCLK_DLY_ENA_GMAC_SHIFT |
		     TXCLK_DLY_ENA_GMAC_MASK <<  TXCLK_DLY_ENA_GMAC_SHIFT |
		     CLK_RX_DL_CFG_GMAC_MASK <<  CLK_RX_DL_CFG_GMAC_SHIFT |
		     CLK_TX_DL_CFG_GMAC_MASK <<  CLK_TX_DL_CFG_GMAC_SHIFT,
		     RXCLK_DLY_ENA_GMAC_ENABLE << RXCLK_DLY_ENA_GMAC_SHIFT |
		     TXCLK_DLY_ENA_GMAC_ENABLE << TXCLK_DLY_ENA_GMAC_SHIFT |
		     pdata->rx_delay << CLK_RX_DL_CFG_GMAC_SHIFT |
		     pdata->tx_delay << CLK_TX_DL_CFG_GMAC_SHIFT);

	return designware_eth_probe(dev);
}

static int gmac_rockchip_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct dw_eth_dev *priv = dev_get_priv(dev);
	int ret;

	ret = designware_eth_init(priv, pdata->enetaddr);
	if (ret)
		return ret;
	ret = gmac_rockchip_fix_mac_speed(priv);
	if (ret)
		return ret;
	ret = designware_eth_enable(priv);
	if (ret)
		return ret;

	return 0;
}

const struct eth_ops gmac_rockchip_eth_ops = {
	.start			= gmac_rockchip_eth_start,
	.send			= designware_eth_send,
	.recv			= designware_eth_recv,
	.free_pkt		= designware_eth_free_pkt,
	.stop			= designware_eth_stop,
	.write_hwaddr		= designware_eth_write_hwaddr,
};

static const struct udevice_id rockchip_gmac_ids[] = {
	{ .compatible = "rockchip,rk3288-gmac" },
	{ }
};

U_BOOT_DRIVER(eth_gmac_rockchip) = {
	.name	= "gmac_rockchip",
	.id	= UCLASS_ETH,
	.of_match = rockchip_gmac_ids,
	.ofdata_to_platdata = gmac_rockchip_ofdata_to_platdata,
	.probe	= gmac_rockchip_probe,
	.ops	= &gmac_rockchip_eth_ops,
	.priv_auto_alloc_size = sizeof(struct dw_eth_dev),
	.platdata_auto_alloc_size = sizeof(struct gmac_rockchip_platdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};

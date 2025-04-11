// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024, Marek Vasut <marex@denx.de>
 *
 * This is code moved from drivers/net/dwc_eth_qos.c , which is:
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <asm/cache.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <eth_phy.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <wait_bit.h>
#include <linux/bitfield.h>
#include <linux/delay.h>

#include "dwc_eth_qos.h"

/* SYSCFG registers */
#define SYSCFG_PMCSETR		0x04
#define SYSCFG_PMCCLRR_MP13	0x08
#define SYSCFG_PMCCLRR_MP15	0x44

#define SYSCFG_PMCSETR_ETH1_MASK	GENMASK(23, 16)
#define SYSCFG_PMCSETR_ETH2_MASK	GENMASK(31, 24)

#define SYSCFG_PMCSETR_ETH_CLK_SEL	BIT(16)
#define SYSCFG_PMCSETR_ETH_REF_CLK_SEL	BIT(17)

/* STM32MP15xx specific bit */
#define SYSCFG_PMCSETR_ETH_SELMII	BIT(20)

#define SYSCFG_PMCSETR_ETH_SEL_MASK	GENMASK(23, 21)
#define SYSCFG_PMCSETR_ETH_SEL_GMII_MII	0x0
#define SYSCFG_PMCSETR_ETH_SEL_RGMII	0x1
#define SYSCFG_PMCSETR_ETH_SEL_RMII	0x4

static ulong eqos_get_tick_clk_rate_stm32(struct udevice *dev)
{
	struct eqos_priv __maybe_unused *eqos = dev_get_priv(dev);

	if (!CONFIG_IS_ENABLED(CLK))
		return 0;

	return clk_get_rate(&eqos->clk_master_bus);
}

static int eqos_start_clks_stm32(struct udevice *dev)
{
	struct eqos_priv __maybe_unused *eqos = dev_get_priv(dev);
	int ret;

	if (!CONFIG_IS_ENABLED(CLK))
		return 0;

	dev_dbg(dev, "%s:\n", __func__);

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		dev_err(dev, "clk_enable(clk_master_bus) failed: %d\n", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		dev_err(dev, "clk_enable(clk_rx) failed: %d\n", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		dev_err(dev, "clk_enable(clk_tx) failed: %d\n", ret);
		goto err_disable_clk_rx;
	}

	if (clk_valid(&eqos->clk_ck) && !eqos->clk_ck_enabled) {
		ret = clk_enable(&eqos->clk_ck);
		if (ret < 0) {
			dev_err(dev, "clk_enable(clk_ck) failed: %d\n", ret);
			goto err_disable_clk_tx;
		}
		eqos->clk_ck_enabled = true;
	}

	dev_dbg(dev, "%s: OK\n", __func__);
	return 0;

err_disable_clk_tx:
	clk_disable(&eqos->clk_tx);
err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err:
	dev_dbg(dev, "%s: FAILED: %d\n", __func__, ret);

	return ret;
}

static int eqos_stop_clks_stm32(struct udevice *dev)
{
	struct eqos_priv __maybe_unused *eqos = dev_get_priv(dev);

	if (!CONFIG_IS_ENABLED(CLK))
		return 0;

	dev_dbg(dev, "%s:\n", __func__);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);

	dev_dbg(dev, "%s: OK\n", __func__);

	return 0;
}

static int eqos_probe_syscfg_stm32(struct udevice *dev,
				   phy_interface_t interface_type)
{
	/* Ethernet 50MHz RMII clock selection. */
	const bool eth_ref_clk_sel = dev_read_bool(dev, "st,eth-ref-clk-sel");
	/* SoC is STM32MP13xx with two ethernet MACs */
	const bool is_mp13 = device_is_compatible(dev, "st,stm32mp13-dwmac");
	/* Gigabit Ethernet 125MHz clock selection. */
	const bool eth_clk_sel = dev_read_bool(dev, "st,eth-clk-sel");
	/* Ethernet clock source is RCC. */
	const bool ext_phyclk = dev_read_bool(dev, "st,ext-phyclk");
	struct regmap *regmap;
	u32 regmap_mask;
	u32 value;

	regmap = syscon_regmap_lookup_by_phandle(dev, "st,syscon");
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	regmap_mask = dev_read_u32_index_default(dev, "st,syscon", 2,
						 SYSCFG_PMCSETR_ETH1_MASK);

	switch (interface_type) {
	case PHY_INTERFACE_MODE_MII:
		dev_dbg(dev, "PHY_INTERFACE_MODE_MII\n");
		value = FIELD_PREP(SYSCFG_PMCSETR_ETH_SEL_MASK,
				   SYSCFG_PMCSETR_ETH_SEL_GMII_MII);
		/*
		 * STM32MP15xx supports both MII and GMII, STM32MP13xx MII only.
		 * SYSCFG_PMCSETR ETH_SELMII is present only on STM32MP15xx and
		 * acts as a selector between 0:GMII and 1:MII. As STM32MP13xx
		 * supports only MII, ETH_SELMII is not present.
		 */
		if (!is_mp13)	/* Select MII mode on STM32MP15xx */
			value |= SYSCFG_PMCSETR_ETH_SELMII;
		break;
	case PHY_INTERFACE_MODE_GMII:	/* STM32MP15xx only */
		dev_dbg(dev, "PHY_INTERFACE_MODE_GMII\n");
		value = FIELD_PREP(SYSCFG_PMCSETR_ETH_SEL_MASK,
				   SYSCFG_PMCSETR_ETH_SEL_GMII_MII);
		/*
		 * If eth_clk_sel is set, use internal ETH_CLKx clock from RCC,
		 * otherwise use external clock from IO pin (requires matching
		 * GPIO block AF setting of that pin).
		 */
		if (eth_clk_sel || ext_phyclk)
			value |= SYSCFG_PMCSETR_ETH_CLK_SEL;
		break;
	case PHY_INTERFACE_MODE_RMII:
		dev_dbg(dev, "PHY_INTERFACE_MODE_RMII\n");
		value = FIELD_PREP(SYSCFG_PMCSETR_ETH_SEL_MASK,
				   SYSCFG_PMCSETR_ETH_SEL_RMII);
		/*
		 * If eth_ref_clk_sel is set, use internal clock from RCC,
		 * otherwise use external clock from ETHn_RX_CLK/ETHn_REF_CLK
		 * IO pin (requires matching GPIO block AF setting of that
		 * pin).
		 */
		if (eth_ref_clk_sel || ext_phyclk)
			value |= SYSCFG_PMCSETR_ETH_REF_CLK_SEL;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		dev_dbg(dev, "PHY_INTERFACE_MODE_RGMII\n");
		value = FIELD_PREP(SYSCFG_PMCSETR_ETH_SEL_MASK,
				   SYSCFG_PMCSETR_ETH_SEL_RGMII);
		/*
		 * If eth_clk_sel is set, use internal ETH_CLKx clock from RCC,
		 * otherwise use external clock from ETHx_CLK125 pin (requires
		 * matching GPIO block AF setting of that pin).
		 */
		if (eth_clk_sel || ext_phyclk)
			value |= SYSCFG_PMCSETR_ETH_CLK_SEL;
		break;
	default:
		dev_dbg(dev, "Do not manage %d interface\n",
			interface_type);
		/* Do not manage others interfaces */
		return -EINVAL;
	}

	/* Shift value at correct ethernet MAC offset in SYSCFG_PMCSETR */
	value <<= ffs(regmap_mask) - ffs(SYSCFG_PMCSETR_ETH1_MASK);

	/* Update PMCCLRR (clear register) */
	regmap_write(regmap, is_mp13 ?
			     SYSCFG_PMCCLRR_MP13 : SYSCFG_PMCCLRR_MP15,
			     regmap_mask);

	return regmap_update_bits(regmap, SYSCFG_PMCSETR, regmap_mask, value);
}

static int eqos_probe_resources_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface;
	int ret;

	dev_dbg(dev, "%s:\n", __func__);

	interface = eqos->config->interface(dev);

	ret = eqos_get_base_addr_dt(dev);
	if (ret) {
		dev_err(dev, "eqos_get_base_addr_dt failed: %d\n", ret);
		return ret;
	}

	if (interface == PHY_INTERFACE_MODE_NA) {
		dev_err(dev, "Invalid PHY interface\n");
		return -EINVAL;
	}

	ret = eqos_probe_syscfg_stm32(dev, interface);
	if (ret)
		return -EINVAL;

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		dev_err(dev, "clk_get_by_name(master_bus) failed: %d\n", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, "mac-clk-rx", &eqos->clk_rx);
	if (ret) {
		dev_err(dev, "clk_get_by_name(rx) failed: %d\n", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, "mac-clk-tx", &eqos->clk_tx);
	if (ret) {
		dev_err(dev, "clk_get_by_name(tx) failed: %d\n", ret);
		goto err_probe;
	}

	/*  Get ETH_CLK clocks (optional) */
	ret = clk_get_by_name(dev, "eth-ck", &eqos->clk_ck);
	if (ret)
		dev_warn(dev, "No phy clock provided %d\n", ret);

	/* Get reset gpio pin (optional) */
	ret = gpio_request_by_name(dev, "phy-reset-gpios", 0,
				   &eqos->phy_reset_gpio, GPIOD_IS_OUT);
	if (ret)
		pr_warn("No phy reset gpio provided: %d\n", ret);

	dev_dbg(dev, "%s: OK\n", __func__);

	return 0;

err_probe:

	dev_dbg(dev, "%s: returns %d\n", __func__, ret);

	return ret;
}

static int eqos_start_resets_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio)) {
		dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
		udelay(2);
		dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
	}

	return 0;
}

static int eqos_remove_resources_stm32(struct udevice *dev)
{
	dev_dbg(dev, "%s:\n", __func__);

	return 0;
}

static struct eqos_ops eqos_stm32_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_stm32,
	.eqos_remove_resources = eqos_remove_resources_stm32,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = eqos_start_resets_stm32,
	.eqos_stop_clks = eqos_stop_clks_stm32,
	.eqos_start_clks = eqos_start_clks_stm32,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_null_ops,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_stm32
};

struct eqos_config __maybe_unused eqos_stm32mp13_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_32,
	.interface = dev_read_phy_mode,
	.ops = &eqos_stm32_ops
};

struct eqos_config __maybe_unused eqos_stm32mp15_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_stm32_ops
};

struct eqos_config __maybe_unused eqos_stm32mp25_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_stm32_ops
};

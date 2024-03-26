// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024, Marek Vasut <marex@denx.de>
 *
 * This is code moved from drivers/net/dwc_eth_qos.c , which is:
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
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
#include <reset.h>
#include <wait_bit.h>
#include <linux/delay.h>

#include "dwc_eth_qos.h"

static ulong eqos_get_tick_clk_rate_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_master_bus);
#else
	return 0;
#endif
}

static int eqos_start_clks_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_master_bus) failed: %d", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(clk_rx) failed: %d", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d", ret);
		goto err_disable_clk_rx;
	}

	if (clk_valid(&eqos->clk_ck) && !eqos->clk_ck_enabled) {
		ret = clk_enable(&eqos->clk_ck);
		if (ret < 0) {
			pr_err("clk_enable(clk_ck) failed: %d", ret);
			goto err_disable_clk_tx;
		}
		eqos->clk_ck_enabled = true;
	}
#endif

	debug("%s: OK\n", __func__);
	return 0;

#ifdef CONFIG_CLK
err_disable_clk_tx:
	clk_disable(&eqos->clk_tx);
err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
#endif
}

static int eqos_stop_clks_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);
#endif

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_probe_resources_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;
	phy_interface_t interface;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NA) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	ret = board_interface_eth_init(dev, interface);
	if (ret)
		return -EINVAL;

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(master_bus) failed: %d", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, "mac-clk-rx", &eqos->clk_rx);
	if (ret) {
		pr_err("clk_get_by_name(rx) failed: %d", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, "mac-clk-tx", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(tx) failed: %d", ret);
		goto err_probe;
	}

	/*  Get ETH_CLK clocks (optional) */
	ret = clk_get_by_name(dev, "eth-ck", &eqos->clk_ck);
	if (ret)
		pr_warn("No phy clock provided %d", ret);

	debug("%s: OK\n", __func__);
	return 0;

err_probe:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove_resources_stm32(struct udevice *dev)
{
	debug("%s(dev=%p):\n", __func__, dev);

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
	.eqos_start_resets = eqos_null_ops,
	.eqos_stop_clks = eqos_stop_clks_stm32,
	.eqos_start_clks = eqos_start_clks_stm32,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_null_ops,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_stm32
};

struct eqos_config __maybe_unused eqos_stm32_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_stm32_ops
};

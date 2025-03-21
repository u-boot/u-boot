// SPDX-License-Identifier: GPL-2.0
/**
 * (C) Copyright 2024 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Author: Greg Malysa <greg.malysa@timesys.com>
 * Additional Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 */

#include <clk.h>
#include <dm.h>
#include <net.h>
#include <phy.h>
#include <reset.h>
#include <linux/io.h>

#include <asm/arch-adi/sc5xx/sc5xx.h>

#include "dwc_eth_qos.h"

static int eqos_start_resets_adi(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	/*
	 * Settings need to latch with the DMA reset below. Currently only
	 * rgmii is supported but other phy interfaces may be supported in
	 * the future
	 */
	sc5xx_enable_rgmii();
	setbits_32(&eqos->dma_regs->mode, EQOS_DMA_MODE_SWR);

	return 0;
}

static int eqos_probe_resources_adi(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface;
	int ret;

	ret = eqos_get_base_addr_dt(dev);
	if (ret) {
		pr_err("eqos_get_base_addr_dt failed: %d\n", ret);
		return ret;
	}

	interface = eqos->config->interface(dev);
	if (interface == PHY_INTERFACE_MODE_NA) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	return 0;
}

/**
 * rgmii tx clock rate is set to 125 MHz regardless of phy mode, and
 * by default the internal clock is always connected to 125 MHz. According
 * to the HRM it is invalid for this clock to have any other speed, so
 * the hardware won't work anyway if this is wrong.
 */
static ulong eqos_get_tick_clk_rate_adi(struct udevice *dev)
{
	return 125 * 1000000;
}

static int eqos_get_enetaddr_adi(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);

	return eth_env_get_enetaddr("ethaddr", pdata->enetaddr);
}

static struct eqos_ops eqos_adi_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_adi,
	.eqos_remove_resources = eqos_null_ops,
	.eqos_start_resets = eqos_start_resets_adi,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_clks = eqos_null_ops,
	.eqos_stop_clks = eqos_null_ops,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_null_ops,
	.eqos_get_enetaddr = eqos_get_enetaddr_adi,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_adi,
};

struct eqos_config __maybe_unused eqos_adi_config = {
	.reg_access_always_ok = true,
	.mdio_wait =  20,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_150_250,
	.axi_bus_width = EQOS_AXI_WIDTH_32,
	.interface = dev_read_phy_mode,
	.ops = &eqos_adi_ops,
};

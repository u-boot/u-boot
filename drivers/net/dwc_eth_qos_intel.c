// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023-2024 DENX Software Engineering GmbH
 * Philip Oberfichtner <pro@denx.de>
 *
 * Based on linux v6.6.39, especially drivers/net/ethernet/stmicro/stmmac
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <miiphy.h>
#include <net.h>
#include <pci.h>

#include "dwc_eth_qos.h"
#include "dwc_eth_qos_intel.h"

static struct pci_device_id intel_pci_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_RGMII1G) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_SGMII1) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_SGMII2G5) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_PSE0_RGMII1G) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_PSE0_SGMII1G) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_PSE0_SGMII2G5) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_PSE1_RGMII1G) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_PSE1_SGMII1G) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EHL_PSE1_SGMII2G5) },
	{}
};

static int pci_config(struct udevice *dev)
{
	u32 val;

	/* Try to enable I/O accesses and bus-mastering */
	dm_pci_read_config32(dev, PCI_COMMAND, &val);
	val |= PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	dm_pci_write_config32(dev, PCI_COMMAND, val);

	/* Make sure it worked */
	dm_pci_read_config32(dev, PCI_COMMAND, &val);
	if (!(val & PCI_COMMAND_MEMORY)) {
		dev_err(dev, "%s: Can't enable I/O memory\n", __func__);
		return -ENOSPC;
	}

	if (!(val & PCI_COMMAND_MASTER)) {
		dev_err(dev, "%s: Can't enable bus-mastering\n", __func__);
		return -EPERM;
	}

	return 0;
}

static void limit_fifo_size(struct udevice *dev)
{
	/*
	 * As described in Intel Erratum EHL22, Document Number: 636674-2.1,
	 * the PSE GbE Controllers advertise a wrong RX and TX fifo size.
	 * Software should limit this value to 64KB.
	 */
	struct eqos_priv *eqos = dev_get_priv(dev);

	eqos->tx_fifo_sz = 0x8000;
	eqos->rx_fifo_sz = 0x8000;
}

static int serdes_status_poll(struct udevice *dev,
			      unsigned char phyaddr, unsigned char phyreg,
			      unsigned short mask, unsigned short val)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	unsigned int retries = 10;
	unsigned short val_rd;

	do {
		miiphy_read(eqos->mii->name, phyaddr, phyreg, &val_rd);
		if ((val_rd & mask) == (val & mask))
			return 0;
		udelay(POLL_DELAY_US);
	} while (--retries);

	return -ETIMEDOUT;
}

 /* Returns -ve if MAC is unknown and 0 on success */
static int mac_check_pse(const struct udevice *dev, bool *is_pse)
{
	struct pci_child_plat *plat = dev_get_parent_plat(dev);

	if (!plat || plat->vendor != PCI_VENDOR_ID_INTEL)
		return -ENXIO;

	switch (plat->device) {
	case PCI_DEVICE_ID_INTEL_EHL_PSE0_RGMII1G:
	case PCI_DEVICE_ID_INTEL_EHL_PSE1_RGMII1G:
	case PCI_DEVICE_ID_INTEL_EHL_PSE0_SGMII1G:
	case PCI_DEVICE_ID_INTEL_EHL_PSE1_SGMII1G:
	case PCI_DEVICE_ID_INTEL_EHL_PSE0_SGMII2G5:
	case PCI_DEVICE_ID_INTEL_EHL_PSE1_SGMII2G5:
		*is_pse = 1;
		return 0;

	case PCI_DEVICE_ID_INTEL_EHL_RGMII1G:
	case PCI_DEVICE_ID_INTEL_EHL_SGMII1:
	case PCI_DEVICE_ID_INTEL_EHL_SGMII2G5:
		*is_pse = 0;
		return 0;
	};

	return -ENXIO;
}

/* Check if we're in 2G5 mode */
static bool serdes_link_mode_2500(struct udevice *dev)
{
	const unsigned char phyad = INTEL_MGBE_ADHOC_ADDR;
	struct eqos_priv *eqos = dev_get_priv(dev);
	unsigned short data;

	miiphy_read(eqos->mii->name, phyad, SERDES_GCR, &data);
	if (FIELD_GET(SERDES_LINK_MODE_MASK, data) == SERDES_LINK_MODE_2G5)
		return true;

	return false;
}

static int serdes_powerup(struct udevice *dev)
{
	/* Based on linux/drivers/net/ethernet/stmicro/stmmac/dwmac-intel.c */

	const unsigned char phyad = INTEL_MGBE_ADHOC_ADDR;
	struct eqos_priv *eqos = dev_get_priv(dev);
	unsigned short data;
	int ret;
	bool is_pse;

	/* Set the serdes rate and the PCLK rate */
	miiphy_read(eqos->mii->name, phyad, SERDES_GCR0, &data);

	data &= ~SERDES_RATE_MASK;
	data &= ~SERDES_PCLK_MASK;

	if (serdes_link_mode_2500(dev))
		data |= SERDES_RATE_PCIE_GEN2 << SERDES_RATE_PCIE_SHIFT |
			SERDES_PCLK_37p5MHZ << SERDES_PCLK_SHIFT;
	else
		data |= SERDES_RATE_PCIE_GEN1 << SERDES_RATE_PCIE_SHIFT |
			SERDES_PCLK_70MHZ << SERDES_PCLK_SHIFT;

	miiphy_write(eqos->mii->name, phyad, SERDES_GCR0, data);

	/* assert clk_req */
	miiphy_read(eqos->mii->name, phyad, SERDES_GCR0, &data);
	data |= SERDES_PLL_CLK;
	miiphy_write(eqos->mii->name, phyad, SERDES_GCR0, data);

	/* check for clk_ack assertion */
	ret = serdes_status_poll(dev, phyad, SERDES_GSR0,
				 SERDES_PLL_CLK, SERDES_PLL_CLK);

	if (ret) {
		dev_err(dev, "Serdes PLL clk request timeout\n");
		return ret;
	}

	/* assert lane reset*/
	miiphy_read(eqos->mii->name, phyad, SERDES_GCR0, &data);
	data |= SERDES_RST;
	miiphy_write(eqos->mii->name, phyad, SERDES_GCR0, data);

	/* check for assert lane reset reflection */
	ret = serdes_status_poll(dev, phyad, SERDES_GSR0,
				 SERDES_RST, SERDES_RST);

	if (ret) {
		dev_err(dev, "Serdes assert lane reset timeout\n");
		return ret;
	}

	/* move power state to P0 */
	miiphy_read(eqos->mii->name, phyad, SERDES_GCR0, &data);
	data &= ~SERDES_PWR_ST_MASK;
	data |= SERDES_PWR_ST_P0 << SERDES_PWR_ST_SHIFT;
	miiphy_write(eqos->mii->name, phyad, SERDES_GCR0, data);

	/* Check for P0 state */
	ret = serdes_status_poll(dev, phyad, SERDES_GSR0,
				 SERDES_PWR_ST_MASK,
				 SERDES_PWR_ST_P0 << SERDES_PWR_ST_SHIFT);

	if (ret) {
		dev_err(dev, "Serdes power state P0 timeout.\n");
		return ret;
	}

	/* PSE only - ungate SGMII PHY Rx Clock*/
	ret = mac_check_pse(dev, &is_pse);
	if (ret) {
		dev_err(dev, "Failed to determine MAC type.\n");
		return ret;
	}

	if (is_pse) {
		miiphy_read(eqos->mii->name, phyad, SERDES_GCR0, &data);
		data |= SERDES_PHY_RX_CLK;
		miiphy_write(eqos->mii->name, phyad, SERDES_GCR0, data);
	}

	return 0;
}

static int xpcs_access(struct udevice *dev, int reg, int v)
{
	/*
	 * Common read/write helper function
	 *
	 * It may seem a bit odd at a first glance that we use bus->read()
	 * directly insetad of one of the wrapper functions. But:
	 *
	 * (1) phy_read() can't be used because we do not access an acutal PHY,
	 *     but a MAC-internal submodule.
	 *
	 * (2) miiphy_read() can't be used because it assumes MDIO_DEVAD_NONE.
	 */

	int port = INTEL_MGBE_XPCS_ADDR;
	int devad = 0x1f;
	u16 val;
	struct eqos_priv *eqos;
	struct mii_dev *bus;

	eqos = dev_get_priv(dev);
	bus = eqos->mii;

	if (v < 0)
		return bus->read(bus, port, devad, reg);

	val = v;
	return bus->write(bus, port, devad, reg, val);
}

static int xpcs_read(struct udevice *dev, int reg)
{
	return xpcs_access(dev, reg, -1);
}

static int xpcs_write(struct udevice *dev, int reg, u16 val)
{
	return xpcs_access(dev, reg, val);
}

static int xpcs_clr_bits(struct udevice *dev, int reg, u16 bits)
{
	int ret;

	ret = xpcs_read(dev, reg);
	if (ret < 0)
		return ret;

	ret &= ~bits;

	return xpcs_write(dev, reg, ret);
}

static int xpcs_set_bits(struct udevice *dev, int reg, u16 bits)
{
	int ret;

	ret = xpcs_read(dev, reg);
	if (ret < 0)
		return ret;

	ret |= bits;

	return xpcs_write(dev, reg, ret);
}

static int xpcs_init(struct udevice *dev)
{
	/* Based on linux/drivers/net/pcs/pcs-xpcs.c */
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface = eqos->config->interface(dev);

	if (interface != PHY_INTERFACE_MODE_SGMII)
		return 0;

	if (xpcs_clr_bits(dev, VR_MII_MMD_CTRL,  XPCS_AN_CL37_EN)  ||
	    xpcs_set_bits(dev, VR_MII_AN_CTRL,   XPCS_MODE_SGMII)  ||
	    xpcs_set_bits(dev, VR_MII_DIG_CTRL1, XPCS_MAC_AUTO_SW) ||
	    xpcs_set_bits(dev, VR_MII_MMD_CTRL,  XPCS_AN_CL37_EN))
		return -EIO;

	return 0;
}

static int eqos_probe_ressources_intel(struct udevice *dev)
{
	int ret;

	ret = eqos_get_base_addr_pci(dev);
	if (ret) {
		dev_err(dev, "eqos_get_base_addr_pci failed: %d\n", ret);
		return ret;
	}

	limit_fifo_size(dev);

	ret = pci_config(dev);
	if (ret) {
		dev_err(dev, "pci_config failed: %d\n", ret);
		return ret;
	}

	return 0;
}

struct eqos_config eqos_intel_config;

/*
 * overwrite __weak function from eqos_intel.c
 *
 * For PCI devices the devcie tree is optional. Choose driver data based on PCI
 * IDs instead.
 */
void *eqos_get_driver_data(struct udevice *dev)
{
	const struct pci_device_id *id;
	const struct pci_child_plat *plat;

	plat = dev_get_parent_plat(dev);

	if (!plat)
		return NULL;

	/* last intel_pci_ids element is zero initialized */
	for (id = intel_pci_ids; id->vendor != 0; id++) {
		if (id->vendor == plat->vendor && id->device == plat->device)
			return &eqos_intel_config;
	}

	return NULL;
}

static int eqos_start_resets_intel(struct udevice *dev)
{
	int ret;

	ret = xpcs_init(dev);
	if (ret) {
		dev_err(dev, "xpcs init failed.\n");
		return ret;
	}

	ret = serdes_powerup(dev);
	if (ret) {
		dev_err(dev, "Failed to power up serdes.\n");
		return ret;
	}

	return 0;
}

static ulong eqos_get_tick_clk_rate_intel(struct udevice *dev)
{
	return 0;
}

static int eqos_get_enetaddr_intel(struct udevice *dev)
{
	/* Assume MAC address is programmed by previous boot stage */
	struct eth_pdata *plat = dev_get_plat(dev);
	struct eqos_priv *eqos = dev_get_priv(dev);
	u8 *lo = (u8 *)&eqos->mac_regs->address0_low;
	u8 *hi = (u8 *)&eqos->mac_regs->address0_high;

	plat->enetaddr[0] = lo[0];
	plat->enetaddr[1] = lo[1];
	plat->enetaddr[2] = lo[2];
	plat->enetaddr[3] = lo[3];
	plat->enetaddr[4] = hi[0];
	plat->enetaddr[5] = hi[1];

	return 0;
}

static phy_interface_t eqos_get_interface_intel(const struct udevice *dev)
{
	struct pci_child_plat *plat = dev_get_parent_plat(dev);

	if (!plat || plat->vendor != PCI_VENDOR_ID_INTEL)
		return PHY_INTERFACE_MODE_NA;

	switch (plat->device) {
	/* The GbE Host Controller has no RGMII interface */
	case PCI_DEVICE_ID_INTEL_EHL_RGMII1G:
		return PHY_INTERFACE_MODE_NA;

	case PCI_DEVICE_ID_INTEL_EHL_PSE0_RGMII1G:
	case PCI_DEVICE_ID_INTEL_EHL_PSE1_RGMII1G:
		return PHY_INTERFACE_MODE_RGMII;

	/* Host SGMII and Host SGMII2G5 share the same device id */
	case PCI_DEVICE_ID_INTEL_EHL_SGMII1:
	case PCI_DEVICE_ID_INTEL_EHL_SGMII2G5:
	case PCI_DEVICE_ID_INTEL_EHL_PSE0_SGMII2G5:
	case PCI_DEVICE_ID_INTEL_EHL_PSE0_SGMII1G:
	case PCI_DEVICE_ID_INTEL_EHL_PSE1_SGMII1G:
	case PCI_DEVICE_ID_INTEL_EHL_PSE1_SGMII2G5:
		return PHY_INTERFACE_MODE_SGMII;
	};

	return PHY_INTERFACE_MODE_NA;
}

static struct eqos_ops eqos_intel_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources =  eqos_probe_ressources_intel,
	.eqos_remove_resources = eqos_null_ops,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = eqos_start_resets_intel,
	.eqos_stop_clks = eqos_null_ops,
	.eqos_start_clks = eqos_null_ops,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_null_ops,
	.eqos_get_enetaddr = eqos_get_enetaddr_intel,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_intel,
};

struct eqos_config eqos_intel_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = eqos_get_interface_intel,
	.ops = &eqos_intel_ops
};

extern U_BOOT_DRIVER(eth_eqos);
U_BOOT_PCI_DEVICE(eth_eqos, intel_pci_ids);

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023, Intel Corporation.
 *
 * Portions based on U-Boot's dwc_eth_qos.c.
 */

/*
 * This driver supports the Synopsys Designware Ethernet XGMAC (10G Ethernet
 * MAC) IP block. The IP supports multiple options for bus type, clocking/
 * reset structure, and feature list.
 *
 * The driver is written such that generic core logic is kept separate from
 * configuration-specific logic. Code that interacts with configuration-
 * specific resources is split out into separate functions to avoid polluting
 * common code. If/when this driver is enhanced to support multiple
 * configurations, the core code should be adapted to call all configuration-
 * specific functions through function pointers, with the definition of those
 * function pointers being supplied by struct udevice_id xgmac_ids[]'s .data
 * field.
 *
 * This configuration uses an AXI master/DMA bus, an AHB slave/register bus,
 * contains the DMA, MTL, and MAC sub-blocks, and supports a single RGMII PHY.
 * This configuration also has SW control over all clock and reset signals to
 * the HW block.
 */

#define LOG_CATEGORY UCLASS_ETH

#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
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
#include <asm/cache.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include "dwc_eth_xgmac.h"

static void *xgmac_alloc_descs(struct xgmac_priv *xgmac, unsigned int num)
{
	return memalign(ARCH_DMA_MINALIGN, num * xgmac->desc_size);
}

static void xgmac_free_descs(void *descs)
{
	free(descs);
}

static struct xgmac_desc *xgmac_get_desc(struct xgmac_priv *xgmac,
					 unsigned int num, bool rx)
{
	return (rx ? xgmac->rx_descs : xgmac->tx_descs) +
	       (num * xgmac->desc_size);
}

void xgmac_inval_desc_generic(void *desc)
{
	unsigned long start;
	unsigned long end;

	if (!desc) {
		pr_err("%s invalid input buffer\n", __func__);
		return;
	}

	start = (unsigned long)desc & ~(ARCH_DMA_MINALIGN - 1);
	end = ALIGN(start + sizeof(struct xgmac_desc),
		    ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

void xgmac_flush_desc_generic(void *desc)
{
	unsigned long start;
	unsigned long end;

	if (!desc) {
		pr_err("%s invalid input buffer\n", __func__);
		return;
	}

	start = (unsigned long)desc & ~(ARCH_DMA_MINALIGN - 1);
	end = ALIGN(start + sizeof(struct xgmac_desc),
		    ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

void xgmac_inval_buffer_generic(void *buf, size_t size)
{
	unsigned long start;
	unsigned long end;

	if (!buf) {
		pr_err("%s invalid input buffer\n", __func__);
		return;
	}

	start = (unsigned long)buf & ~(ARCH_DMA_MINALIGN - 1);
	end = ALIGN((unsigned long)buf + size,
		    ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

void xgmac_flush_buffer_generic(void *buf, size_t size)
{
	unsigned long start;
	unsigned long end;

	if (!buf) {
		pr_err("%s invalid input buffer\n", __func__);
		return;
	}

	start = (unsigned long)buf & ~(ARCH_DMA_MINALIGN - 1);
	end = ALIGN((unsigned long)buf + size,
		    ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

static int xgmac_mdio_wait_idle(struct xgmac_priv *xgmac)
{
	return wait_for_bit_le32(&xgmac->mac_regs->mdio_data,
				 XGMAC_MAC_MDIO_ADDRESS_SBUSY, false,
				 XGMAC_TIMEOUT_100MS, true);
}

static int xgmac_mdio_read(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			   int mdio_reg)
{
	struct xgmac_priv *xgmac = bus->priv;
	u32 val;
	u32 hw_addr;
	int ret;

	debug("%s(dev=%p, addr=0x%x, reg=%d):\n", __func__, xgmac->dev, mdio_addr,
	      mdio_reg);

	ret = xgmac_mdio_wait_idle(xgmac);
	if (ret) {
		pr_err("%s MDIO not idle at entry: %d\n",
		       xgmac->dev->name, ret);

		return ret;
	}

	/* Set clause 22 format */
	val = BIT(mdio_addr);
	writel(val, &xgmac->mac_regs->mdio_clause_22_port);

	hw_addr = (mdio_addr << XGMAC_MAC_MDIO_ADDRESS_PA_SHIFT) |
		   (mdio_reg & XGMAC_MAC_MDIO_REG_ADDR_C22P_MASK);

	val = xgmac->config->config_mac_mdio <<
	      XGMAC_MAC_MDIO_ADDRESS_CR_SHIFT;

	val |= XGMAC_MAC_MDIO_ADDRESS_SADDR |
	       XGMAC_MDIO_SINGLE_CMD_ADDR_CMD_READ |
	       XGMAC_MAC_MDIO_ADDRESS_SBUSY;

	ret = xgmac_mdio_wait_idle(xgmac);
	if (ret) {
		pr_err("%s MDIO not idle at entry: %d\n",
		       xgmac->dev->name, ret);

		return ret;
	}

	writel(hw_addr, &xgmac->mac_regs->mdio_address);
	writel(val, &xgmac->mac_regs->mdio_data);

	ret = xgmac_mdio_wait_idle(xgmac);
	if (ret) {
		pr_err("%s MDIO read didn't complete: %d\n",
		       xgmac->dev->name, ret);

		return ret;
	}

	val = readl(&xgmac->mac_regs->mdio_data);
	val &= XGMAC_MAC_MDIO_DATA_GD_MASK;

	debug("%s: val=0x%x\n", __func__, val);

	return val;
}

static int xgmac_mdio_write(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			    int mdio_reg, u16 mdio_val)
{
	struct xgmac_priv *xgmac = bus->priv;
	u32 val;
	u32 hw_addr;
	int ret;

	debug("%s(dev=%p, addr=0x%x, reg=%d, val=0x%x):\n", __func__, xgmac->dev,
	      mdio_addr, mdio_reg, mdio_val);

	ret = xgmac_mdio_wait_idle(xgmac);
	if (ret) {
		pr_err("%s MDIO not idle at entry: %d\n",
		       xgmac->dev->name, ret);

		return ret;
	}

	/* Set clause 22 format */
	val = BIT(mdio_addr);
	writel(val, &xgmac->mac_regs->mdio_clause_22_port);

	hw_addr = (mdio_addr << XGMAC_MAC_MDIO_ADDRESS_PA_SHIFT) |
		   (mdio_reg & XGMAC_MAC_MDIO_REG_ADDR_C22P_MASK);

	hw_addr |= (mdio_reg >> XGMAC_MAC_MDIO_ADDRESS_PA_SHIFT) <<
		    XGMAC_MAC_MDIO_ADDRESS_DA_SHIFT;

	val = (xgmac->config->config_mac_mdio <<
	       XGMAC_MAC_MDIO_ADDRESS_CR_SHIFT);

	val |= XGMAC_MAC_MDIO_ADDRESS_SADDR |
		mdio_val | XGMAC_MDIO_SINGLE_CMD_ADDR_CMD_WRITE |
		XGMAC_MAC_MDIO_ADDRESS_SBUSY;

	ret = xgmac_mdio_wait_idle(xgmac);
	if (ret) {
		pr_err("%s MDIO not idle at entry: %d\n",
		       xgmac->dev->name, ret);

		return ret;
	}

	writel(hw_addr, &xgmac->mac_regs->mdio_address);
	writel(val, &xgmac->mac_regs->mdio_data);

	ret = xgmac_mdio_wait_idle(xgmac);
	if (ret) {
		pr_err("%s MDIO write didn't complete: %d\n",
		       xgmac->dev->name, ret);

		return ret;
	}

	return 0;
}

static int xgmac_set_full_duplex(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&xgmac->mac_regs->mac_extended_conf, XGMAC_MAC_EXT_CONF_HD);

	return 0;
}

static int xgmac_set_half_duplex(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&xgmac->mac_regs->mac_extended_conf, XGMAC_MAC_EXT_CONF_HD);

	/* WAR: Flush TX queue when switching to half-duplex */
	setbits_le32(&xgmac->mtl_regs->txq0_operation_mode,
		     XGMAC_MTL_TXQ0_OPERATION_MODE_FTQ);

	return 0;
}

static int xgmac_set_gmii_speed(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	u32 val;

	debug("%s(dev=%p):\n", __func__, dev);

	val = XGMAC_MAC_CONF_SS_1G_GMII << XGMAC_MAC_CONF_SS_SHIFT;
	writel(val, &xgmac->mac_regs->tx_configuration);

	return 0;
}

static int xgmac_set_mii_speed_100(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	u32 val;

	debug("%s(dev=%p):\n", __func__, dev);

	val = XGMAC_MAC_CONF_SS_100M_MII << XGMAC_MAC_CONF_SS_SHIFT;
	writel(val, &xgmac->mac_regs->tx_configuration);

	return 0;
}

static int xgmac_set_mii_speed_10(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	u32 val;

	debug("%s(dev=%p):\n", __func__, dev);

	val = XGMAC_MAC_CONF_SS_2_10M_MII << XGMAC_MAC_CONF_SS_SHIFT;
	writel(val, &xgmac->mac_regs->tx_configuration);

	return 0;
}

static int xgmac_adjust_link(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	int ret;
	bool en_calibration;

	debug("%s(dev=%p):\n", __func__, dev);

	if (xgmac->phy->duplex)
		ret = xgmac_set_full_duplex(dev);
	else
		ret = xgmac_set_half_duplex(dev);
	if (ret < 0) {
		pr_err("%s xgmac_set_*_duplex() failed: %d\n", dev->name, ret);
		return ret;
	}

	switch (xgmac->phy->speed) {
	case SPEED_1000:
		en_calibration = true;
		ret = xgmac_set_gmii_speed(dev);
		break;
	case SPEED_100:
		en_calibration = true;
		ret = xgmac_set_mii_speed_100(dev);
		break;
	case SPEED_10:
		en_calibration = false;
		ret = xgmac_set_mii_speed_10(dev);
		break;
	default:
		pr_err("%s invalid speed %d\n", dev->name, xgmac->phy->speed);
		return -EINVAL;
	}
	if (ret < 0) {
		pr_err("%s xgmac_set_*mii_speed*() failed: %d\n", dev->name, ret);
		return ret;
	}

	if (en_calibration) {
		ret = xgmac->config->ops->xgmac_calibrate_pads(dev);
		if (ret < 0) {
			pr_err("%s xgmac_calibrate_pads() failed: %d\n",
			       dev->name, ret);

			return ret;
		}
	} else {
		ret = xgmac->config->ops->xgmac_disable_calibration(dev);
		if (ret < 0) {
			pr_err("%s xgmac_disable_calibration() failed: %d\n",
			       dev->name, ret);

			return ret;
		}
	}

	return 0;
}

static int xgmac_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	u32 val;

	/*
	 * This function may be called before start() or after stop(). At that
	 * time, on at least some configurations of the XGMAC HW, all clocks to
	 * the XGMAC HW block will be stopped, and a reset signal applied. If
	 * any register access is attempted in this state, bus timeouts or CPU
	 * hangs may occur. This check prevents that.
	 *
	 * A simple solution to this problem would be to not implement
	 * write_hwaddr(), since start() always writes the MAC address into HW
	 * anyway. However, it is desirable to implement write_hwaddr() to
	 * support the case of SW that runs subsequent to U-Boot which expects
	 * the MAC address to already be programmed into the XGMAC registers,
	 * which must happen irrespective of whether the U-Boot user (or
	 * scripts) actually made use of the XGMAC device, and hence
	 * irrespective of whether start() was ever called.
	 *
	 */
	if (!xgmac->config->reg_access_always_ok && !xgmac->reg_access_ok)
		return 0;

	/* Update the MAC address */
	val = (plat->enetaddr[5] << 8) |
		(plat->enetaddr[4]);
	writel(val, &xgmac->mac_regs->address0_high);
	val = (plat->enetaddr[3] << 24) |
		(plat->enetaddr[2] << 16) |
		(plat->enetaddr[1] << 8) |
		(plat->enetaddr[0]);
	writel(val, &xgmac->mac_regs->address0_low);
	return 0;
}

static int xgmac_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	int ret;

	ret = xgmac->config->ops->xgmac_get_enetaddr(dev);
	if (ret < 0)
		return ret;

	return !is_valid_ethaddr(pdata->enetaddr);
}

static int xgmac_get_phy_addr(struct xgmac_priv *priv, struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	int reg;

	if (dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
				       &phandle_args)) {
		debug("Failed to find phy-handle");
		return -ENODEV;
	}

	priv->phy_of_node = phandle_args.node;

	reg = ofnode_read_u32_default(phandle_args.node, "reg", 0);

	return reg;
}

static int xgmac_start(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	int ret, i;
	u32 val, tx_fifo_sz, rx_fifo_sz, tqs, rqs, pbl;
	ulong last_rx_desc;
	ulong desc_pad, address;

	struct xgmac_desc *tx_desc = NULL;
	struct xgmac_desc *rx_desc = NULL;
	int addr = -1;

	debug("%s(dev=%p):\n", __func__, dev);

	xgmac->tx_desc_idx = 0;
	xgmac->rx_desc_idx = 0;

	ret = xgmac->config->ops->xgmac_start_resets(dev);
	if (ret < 0) {
		pr_err("%s xgmac_start_resets() failed: %d\n", dev->name, ret);
		goto err;
	}

	xgmac->reg_access_ok = true;

	ret = wait_for_bit_le32(&xgmac->dma_regs->mode,
				XGMAC_DMA_MODE_SWR, false,
				xgmac->config->swr_wait, false);
	if (ret) {
		pr_err("%s XGMAC_DMA_MODE_SWR stuck: %d\n", dev->name, ret);
		goto err_stop_resets;
	}

	ret = xgmac->config->ops->xgmac_calibrate_pads(dev);
	if (ret < 0) {
		pr_err("%s xgmac_calibrate_pads() failed: %d\n", dev->name, ret);
		goto err_stop_resets;
	}

	/*
	 * if PHY was already connected and configured,
	 * don't need to reconnect/reconfigure again
	 */
	if (!xgmac->phy) {
		addr = xgmac_get_phy_addr(xgmac, dev);
		xgmac->phy = phy_connect(xgmac->mii, addr, dev,
					 xgmac->config->interface(dev));
		if (!xgmac->phy) {
			pr_err("%s phy_connect() failed\n", dev->name);
			goto err_stop_resets;
		}

		if (xgmac->max_speed) {
			ret = phy_set_supported(xgmac->phy, xgmac->max_speed);
			if (ret) {
				pr_err("%s phy_set_supported() failed: %d\n",
				       dev->name, ret);

				goto err_shutdown_phy;
			}
		}

		xgmac->phy->node = xgmac->phy_of_node;
		ret = phy_config(xgmac->phy);
		if (ret < 0) {
			pr_err("%s phy_config() failed: %d\n", dev->name, ret);
			goto err_shutdown_phy;
		}
	}

	ret = phy_startup(xgmac->phy);
	if (ret < 0) {
		pr_err("%s phy_startup() failed: %d\n", dev->name, ret);
		goto err_shutdown_phy;
	}

	if (!xgmac->phy->link) {
		pr_err("%s No link\n", dev->name);
		goto err_shutdown_phy;
	}

	ret = xgmac_adjust_link(dev);
	if (ret < 0) {
		pr_err("%s xgmac_adjust_link() failed: %d\n", dev->name, ret);
		goto err_shutdown_phy;
	}

	/* Configure MTL */

	/* Enable Store and Forward mode for TX */
	/* Program Tx operating mode */
	setbits_le32(&xgmac->mtl_regs->txq0_operation_mode,
		     XGMAC_MTL_TXQ0_OPERATION_MODE_TSF |
		     (XGMAC_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED <<
		      XGMAC_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT));

	/* Transmit Queue weight */
	writel(0x10, &xgmac->mtl_regs->txq0_quantum_weight);

	/* Enable Store and Forward mode for RX, since no jumbo frame */
	setbits_le32(&xgmac->mtl_regs->rxq0_operation_mode,
		     XGMAC_MTL_RXQ0_OPERATION_MODE_RSF);

	/* Transmit/Receive queue fifo size; use all RAM for 1 queue */
	val = readl(&xgmac->mac_regs->hw_feature1);
	tx_fifo_sz = (val >> XGMAC_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT) &
		XGMAC_MAC_HW_FEATURE1_TXFIFOSIZE_MASK;
	rx_fifo_sz = (val >> XGMAC_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT) &
		XGMAC_MAC_HW_FEATURE1_RXFIFOSIZE_MASK;

	/*
	 * r/tx_fifo_sz is encoded as log2(n / 128). Undo that by shifting.
	 * r/tqs is encoded as (n / 256) - 1.
	 */
	tqs = (128 << tx_fifo_sz) / 256 - 1;
	rqs = (128 << rx_fifo_sz) / 256 - 1;

	clrsetbits_le32(&xgmac->mtl_regs->txq0_operation_mode,
			XGMAC_MTL_TXQ0_OPERATION_MODE_TQS_MASK <<
			XGMAC_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT,
			tqs << XGMAC_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
	clrsetbits_le32(&xgmac->mtl_regs->rxq0_operation_mode,
			XGMAC_MTL_RXQ0_OPERATION_MODE_RQS_MASK <<
			XGMAC_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT,
			rqs << XGMAC_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT);

	setbits_le32(&xgmac->mtl_regs->rxq0_operation_mode,
		     XGMAC_MTL_RXQ0_OPERATION_MODE_EHFC);

	/* Configure MAC */
	clrsetbits_le32(&xgmac->mac_regs->rxq_ctrl0,
			XGMAC_MAC_RXQ_CTRL0_RXQ0EN_MASK <<
			XGMAC_MAC_RXQ_CTRL0_RXQ0EN_SHIFT,
			xgmac->config->config_mac <<
			XGMAC_MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

	/* Multicast and Broadcast Queue Enable */
	setbits_le32(&xgmac->mac_regs->rxq_ctrl1,
		     XGMAC_MAC_RXQ_CTRL1_MCBCQEN);

	/* enable promise mode and receive all mode */
	setbits_le32(&xgmac->mac_regs->mac_packet_filter,
		     XGMAC_MAC_PACKET_FILTER_RA |
			 XGMAC_MAC_PACKET_FILTER_PR);

	/* Set TX flow control parameters */
	/* Set Pause Time */
	setbits_le32(&xgmac->mac_regs->q0_tx_flow_ctrl,
		     XGMAC_MAC_Q0_TX_FLOW_CTRL_PT_MASK <<
		     XGMAC_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT);

	/* Assign priority for RX flow control */
	clrbits_le32(&xgmac->mac_regs->rxq_ctrl2,
		     XGMAC_MAC_RXQ_CTRL2_PSRQ0_MASK <<
		     XGMAC_MAC_RXQ_CTRL2_PSRQ0_SHIFT);

	/* Enable flow control */
	setbits_le32(&xgmac->mac_regs->q0_tx_flow_ctrl,
		     XGMAC_MAC_Q0_TX_FLOW_CTRL_TFE);
	setbits_le32(&xgmac->mac_regs->rx_flow_ctrl,
		     XGMAC_MAC_RX_FLOW_CTRL_RFE);

	clrbits_le32(&xgmac->mac_regs->tx_configuration,
		     XGMAC_MAC_CONF_JD);

	clrbits_le32(&xgmac->mac_regs->rx_configuration,
		     XGMAC_MAC_CONF_JE |
		     XGMAC_MAC_CONF_GPSLCE |
		     XGMAC_MAC_CONF_WD);

	setbits_le32(&xgmac->mac_regs->rx_configuration,
		     XGMAC_MAC_CONF_ACS |
		     XGMAC_MAC_CONF_CST);

	ret = xgmac_write_hwaddr(dev);
	if (ret < 0) {
		pr_err("%s xgmac_write_hwaddr() failed: %d\n", dev->name, ret);
		goto err;
	}

	/* Configure DMA */
	clrsetbits_le32(&xgmac->dma_regs->sysbus_mode,
			XGMAC_DMA_SYSBUS_MODE_AAL,
			XGMAC_DMA_SYSBUS_MODE_EAME |
			XGMAC_DMA_SYSBUS_MODE_UNDEF);

	/* Enable OSP mode */
	setbits_le32(&xgmac->dma_regs->ch0_tx_control,
		     XGMAC_DMA_CH0_TX_CONTROL_OSP);

	/* RX buffer size. Must be a multiple of bus width */
	clrsetbits_le32(&xgmac->dma_regs->ch0_rx_control,
			XGMAC_DMA_CH0_RX_CONTROL_RBSZ_MASK <<
			XGMAC_DMA_CH0_RX_CONTROL_RBSZ_SHIFT,
			XGMAC_MAX_PACKET_SIZE <<
			XGMAC_DMA_CH0_RX_CONTROL_RBSZ_SHIFT);

	desc_pad = (xgmac->desc_size - sizeof(struct xgmac_desc)) /
		    xgmac->config->axi_bus_width;

	setbits_le32(&xgmac->dma_regs->ch0_control,
		     XGMAC_DMA_CH0_CONTROL_PBLX8 |
		     (desc_pad << XGMAC_DMA_CH0_CONTROL_DSL_SHIFT));

	/*
	 * Burst length must be < 1/2 FIFO size.
	 * FIFO size in tqs is encoded as (n / 256) - 1.
	 * Each burst is n * 8 (PBLX8) * 16 (AXI width) == 128 bytes.
	 * Half of n * 256 is n * 128, so pbl == tqs, modulo the -1.
	 */
	pbl = tqs + 1;
	if (pbl > 32)
		pbl = 32;

	clrsetbits_le32(&xgmac->dma_regs->ch0_tx_control,
			XGMAC_DMA_CH0_TX_CONTROL_TXPBL_MASK <<
			XGMAC_DMA_CH0_TX_CONTROL_TXPBL_SHIFT,
			pbl << XGMAC_DMA_CH0_TX_CONTROL_TXPBL_SHIFT);

	clrsetbits_le32(&xgmac->dma_regs->ch0_rx_control,
			XGMAC_DMA_CH0_RX_CONTROL_RXPBL_MASK <<
			XGMAC_DMA_CH0_RX_CONTROL_RXPBL_SHIFT,
			8 << XGMAC_DMA_CH0_RX_CONTROL_RXPBL_SHIFT);

	/* DMA performance configuration */
	val = (XGMAC_DMA_SYSBUS_MODE_RD_OSR_LMT_MASK <<
	       XGMAC_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT) |
	       (XGMAC_DMA_SYSBUS_MODE_WR_OSR_LMT_MASK <<
	       XGMAC_DMA_SYSBUS_MODE_WR_OSR_LMT_SHIFT) |
	       XGMAC_DMA_SYSBUS_MODE_EAME |
	       XGMAC_DMA_SYSBUS_MODE_BLEN16 |
	       XGMAC_DMA_SYSBUS_MODE_BLEN8 |
	       XGMAC_DMA_SYSBUS_MODE_BLEN4 |
	       XGMAC_DMA_SYSBUS_MODE_BLEN32;

	writel(val, &xgmac->dma_regs->sysbus_mode);

	/* Set up descriptors */

	memset(xgmac->tx_descs, 0, xgmac->desc_size * XGMAC_DESCRIPTORS_TX);
	memset(xgmac->rx_descs, 0, xgmac->desc_size * XGMAC_DESCRIPTORS_RX);

	for (i = 0; i < XGMAC_DESCRIPTORS_TX; i++) {
		tx_desc = (struct xgmac_desc *)xgmac_get_desc(xgmac, i, false);

		xgmac->config->ops->xgmac_flush_desc(tx_desc);
	}

	for (i = 0; i < XGMAC_DESCRIPTORS_RX; i++) {
		rx_desc = (struct xgmac_desc *)xgmac_get_desc(xgmac, i, true);

		address = (uintptr_t)(xgmac->rx_dma_buf +
					(i * XGMAC_MAX_PACKET_SIZE));

		rx_desc->des0 = lower_32_bits(address);
		rx_desc->des1 = upper_32_bits(address);
		rx_desc->des3 = XGMAC_DESC3_OWN;
		/* Flush the cache to the memory */
		mb();
		xgmac->config->ops->xgmac_flush_desc(rx_desc);
		xgmac->config->ops->xgmac_inval_buffer(xgmac->rx_dma_buf +
						       (i * XGMAC_MAX_PACKET_SIZE),
						       XGMAC_MAX_PACKET_SIZE);
	}

	address = (ulong)xgmac_get_desc(xgmac, 0, false);
	writel(upper_32_bits(address),
	       &xgmac->dma_regs->ch0_txdesc_list_haddress);
	writel(lower_32_bits(address),
	       &xgmac->dma_regs->ch0_txdesc_list_address);
	writel(XGMAC_DESCRIPTORS_TX - 1,
	       &xgmac->dma_regs->ch0_txdesc_ring_length);
	address = (ulong)xgmac_get_desc(xgmac, 0, true);
	writel(upper_32_bits(address),
	       &xgmac->dma_regs->ch0_rxdesc_list_haddress);
	writel(lower_32_bits(address),
	       &xgmac->dma_regs->ch0_rxdesc_list_address);
	writel(XGMAC_DESCRIPTORS_RX - 1,
	       &xgmac->dma_regs->ch0_rxdesc_ring_length);

	/* Enable everything */
	setbits_le32(&xgmac->dma_regs->ch0_tx_control,
		     XGMAC_DMA_CH0_TX_CONTROL_ST);
	setbits_le32(&xgmac->dma_regs->ch0_rx_control,
		     XGMAC_DMA_CH0_RX_CONTROL_SR);
	setbits_le32(&xgmac->mac_regs->tx_configuration,
		     XGMAC_MAC_CONF_TE);
	setbits_le32(&xgmac->mac_regs->rx_configuration,
		     XGMAC_MAC_CONF_RE);

	/* TX tail pointer not written until we need to TX a packet */
	/*
	 * Point RX tail pointer at last descriptor. Ideally, we'd point at the
	 * first descriptor, implying all descriptors were available. However,
	 * that's not distinguishable from none of the descriptors being
	 * available.
	 */
	last_rx_desc = (ulong)xgmac_get_desc(xgmac, XGMAC_DESCRIPTORS_RX - 1, true);
	writel(last_rx_desc, &xgmac->dma_regs->ch0_rxdesc_tail_pointer);

	xgmac->started = true;

	debug("%s: OK\n", __func__);
	return 0;

err_shutdown_phy:
	phy_shutdown(xgmac->phy);
err_stop_resets:
	xgmac->config->ops->xgmac_stop_resets(dev);
err:
	pr_err("%s FAILED: %d\n", dev->name, ret);
	return ret;
}

static void xgmac_stop(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	unsigned long start_time;
	u32 val;
	u32 trcsts;
	u32 txqsts;
	u32 prxq;
	u32 rxqsts;

	debug("%s(dev=%p):\n", __func__, dev);

	if (!xgmac->started)
		return;
	xgmac->started = false;
	xgmac->reg_access_ok = false;

	/* Disable TX DMA */
	clrbits_le32(&xgmac->dma_regs->ch0_tx_control,
		     XGMAC_DMA_CH0_TX_CONTROL_ST);

	/* Wait for TX all packets to drain out of MTL */
	start_time = get_timer(0);

	while (get_timer(start_time) < XGMAC_TIMEOUT_100MS) {
		val = readl(&xgmac->mtl_regs->txq0_debug);

		trcsts = (val >> XGMAC_MTL_TXQ0_DEBUG_TRCSTS_SHIFT) &
			  XGMAC_MTL_TXQ0_DEBUG_TRCSTS_MASK;

		txqsts = val & XGMAC_MTL_TXQ0_DEBUG_TXQSTS;

		if (trcsts != XGMAC_MTL_TXQ0_DEBUG_TRCSTS_READ_STATE && !txqsts)
			break;
	}

	/* Turn off MAC TX and RX */
	clrbits_le32(&xgmac->mac_regs->tx_configuration,
		     XGMAC_MAC_CONF_RE);
	clrbits_le32(&xgmac->mac_regs->rx_configuration,
		     XGMAC_MAC_CONF_RE);

	/* Wait for all RX packets to drain out of MTL */
	start_time = get_timer(0);

	while (get_timer(start_time) < XGMAC_TIMEOUT_100MS) {
		val = readl(&xgmac->mtl_regs->rxq0_debug);

		prxq = (val >> XGMAC_MTL_RXQ0_DEBUG_PRXQ_SHIFT) &
			XGMAC_MTL_RXQ0_DEBUG_PRXQ_MASK;

		rxqsts = (val >> XGMAC_MTL_RXQ0_DEBUG_RXQSTS_SHIFT) &
			  XGMAC_MTL_RXQ0_DEBUG_RXQSTS_MASK;

		if (!prxq && !rxqsts)
			break;
	}

	/* Turn off RX DMA */
	clrbits_le32(&xgmac->dma_regs->ch0_rx_control,
		     XGMAC_DMA_CH0_RX_CONTROL_SR);

	if (xgmac->phy)
		phy_shutdown(xgmac->phy);

	xgmac->config->ops->xgmac_stop_resets(dev);

	debug("%s: OK\n", __func__);
}

static int xgmac_send(struct udevice *dev, void *packet, int length)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	struct xgmac_desc *tx_desc;
	unsigned long start_time;

	debug("%s(dev=%p, packet=%p, length=%d):\n", __func__, dev, packet,
	      length);

	memcpy(xgmac->tx_dma_buf, packet, length);
	xgmac->config->ops->xgmac_flush_buffer(xgmac->tx_dma_buf, length);

	tx_desc = xgmac_get_desc(xgmac, xgmac->tx_desc_idx, false);
	xgmac->tx_desc_idx++;
	xgmac->tx_desc_idx %= XGMAC_DESCRIPTORS_TX;

	tx_desc->des0 = lower_32_bits((ulong)xgmac->tx_dma_buf);
	tx_desc->des1 = upper_32_bits((ulong)xgmac->tx_dma_buf);
	tx_desc->des2 = length;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	tx_desc->des3 = XGMAC_DESC3_OWN | XGMAC_DESC3_FD | XGMAC_DESC3_LD | length;
	xgmac->config->ops->xgmac_flush_desc(tx_desc);

	writel((ulong)xgmac_get_desc(xgmac, xgmac->tx_desc_idx, false),
	       &xgmac->dma_regs->ch0_txdesc_tail_pointer);

	start_time = get_timer(0);

	while (get_timer(start_time) < XGMAC_TIMEOUT_100MS) {
		xgmac->config->ops->xgmac_inval_desc(tx_desc);
		if (!(readl(&tx_desc->des3) & XGMAC_DESC3_OWN))
			return 0;
	}
	debug("%s: TX timeout\n", __func__);

	return -ETIMEDOUT;
}

static int xgmac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	struct xgmac_desc *rx_desc;
	int length;

	debug("%s(dev=%p, flags=0x%x):\n", __func__, dev, flags);

	rx_desc = xgmac_get_desc(xgmac, xgmac->rx_desc_idx, true);
	xgmac->config->ops->xgmac_inval_desc(rx_desc);
	if (rx_desc->des3 & XGMAC_DESC3_OWN) {
		debug("%s: RX packet not available\n", __func__);
		return -EAGAIN;
	}

	*packetp = xgmac->rx_dma_buf +
		   (xgmac->rx_desc_idx * XGMAC_MAX_PACKET_SIZE);
	length = rx_desc->des3 & XGMAC_RDES3_PKT_LENGTH_MASK;
	debug("%s: *packetp=%p, length=%d\n", __func__, *packetp, length);

	xgmac->config->ops->xgmac_inval_buffer(*packetp, length);

	return length;
}

static int xgmac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	u32 idx, idx_mask = xgmac->desc_per_cacheline - 1;
	uchar *packet_expected;
	struct xgmac_desc *rx_desc;
	ulong address;

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	packet_expected = xgmac->rx_dma_buf +
			  (xgmac->rx_desc_idx * XGMAC_MAX_PACKET_SIZE);
	if (packet != packet_expected) {
		debug("%s: Unexpected packet (expected %p)\n", __func__,
		      packet_expected);
		return -EINVAL;
	}

	xgmac->config->ops->xgmac_inval_buffer(packet, length);

	if ((xgmac->rx_desc_idx & idx_mask) == idx_mask) {
		for (idx = xgmac->rx_desc_idx - idx_mask;
		     idx <= xgmac->rx_desc_idx;
		     idx++) {
			rx_desc = xgmac_get_desc(xgmac, idx, true);
			rx_desc->des0 = 0;
			rx_desc->des1 = 0;
			/* Flush the cache to the memory */
			mb();
			xgmac->config->ops->xgmac_flush_desc(rx_desc);
			xgmac->config->ops->xgmac_inval_buffer(packet, length);
			address = (ulong)(xgmac->rx_dma_buf +
					(idx * XGMAC_MAX_PACKET_SIZE));
			rx_desc->des0 = lower_32_bits(address);
			rx_desc->des1 = upper_32_bits(address);
			rx_desc->des2 = 0;
			/*
			 * Make sure that if HW sees the _OWN write below,
			 * it will see all the writes to the rest of the
			 * descriptor too.
			 */
			mb();
			rx_desc->des3 = XGMAC_DESC3_OWN;
			xgmac->config->ops->xgmac_flush_desc(rx_desc);
		}
		writel((ulong)rx_desc, &xgmac->dma_regs->ch0_rxdesc_tail_pointer);
	}

	xgmac->rx_desc_idx++;
	xgmac->rx_desc_idx %= XGMAC_DESCRIPTORS_RX;

	return 0;
}

static int xgmac_probe_resources_core(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	unsigned int desc_step;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	/* Maximum distance between neighboring descriptors, in Bytes. */
	desc_step = sizeof(struct xgmac_desc);

	if (desc_step < ARCH_DMA_MINALIGN) {
		/*
		 * The hardware implementation cannot place one descriptor
		 * per cacheline, it is necessary to place multiple descriptors
		 * per cacheline in memory and do cache management carefully.
		 */
		xgmac->desc_size = BIT(fls(desc_step) - 1);
	} else {
		xgmac->desc_size = ALIGN(sizeof(struct xgmac_desc),
					 (unsigned int)ARCH_DMA_MINALIGN);
	}
	xgmac->desc_per_cacheline = ARCH_DMA_MINALIGN / xgmac->desc_size;

	xgmac->tx_descs = xgmac_alloc_descs(xgmac, XGMAC_DESCRIPTORS_TX);
	if (!xgmac->tx_descs) {
		debug("%s: xgmac_alloc_descs(tx) failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	xgmac->rx_descs = xgmac_alloc_descs(xgmac, XGMAC_DESCRIPTORS_RX);
	if (!xgmac->rx_descs) {
		debug("%s: xgmac_alloc_descs(rx) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_descs;
	}

	xgmac->tx_dma_buf = memalign(XGMAC_BUFFER_ALIGN, XGMAC_MAX_PACKET_SIZE);
	if (!xgmac->tx_dma_buf) {
		debug("%s: memalign(tx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_descs;
	}
	debug("%s: tx_dma_buf=%p\n", __func__, xgmac->tx_dma_buf);

	xgmac->rx_dma_buf = memalign(XGMAC_BUFFER_ALIGN, XGMAC_RX_BUFFER_SIZE);
	if (!xgmac->rx_dma_buf) {
		debug("%s: memalign(rx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_dma_buf;
	}
	debug("%s: rx_dma_buf=%p\n", __func__, xgmac->rx_dma_buf);

	xgmac->rx_pkt = malloc(XGMAC_MAX_PACKET_SIZE);
	if (!xgmac->rx_pkt) {
		debug("%s: malloc(rx_pkt) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_rx_dma_buf;
	}
	debug("%s: rx_pkt=%p\n", __func__, xgmac->rx_pkt);

	xgmac->config->ops->xgmac_inval_buffer(xgmac->rx_dma_buf,
			XGMAC_MAX_PACKET_SIZE * XGMAC_DESCRIPTORS_RX);

	debug("%s: OK\n", __func__);
	return 0;

err_free_rx_dma_buf:
	free(xgmac->rx_dma_buf);
err_free_tx_dma_buf:
	free(xgmac->tx_dma_buf);
err_free_descs:
	xgmac_free_descs(xgmac->rx_descs);
err_free_tx_descs:
	xgmac_free_descs(xgmac->tx_descs);
err:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int xgmac_remove_resources_core(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	free(xgmac->rx_pkt);
	free(xgmac->rx_dma_buf);
	free(xgmac->tx_dma_buf);
	xgmac_free_descs(xgmac->rx_descs);
	xgmac_free_descs(xgmac->tx_descs);

	debug("%s: OK\n", __func__);
	return 0;
}

/* board-specific Ethernet Interface initializations. */
__weak int board_interface_eth_init(struct udevice *dev,
				    phy_interface_t interface_type)
{
	return 0;
}

static int xgmac_probe(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	xgmac->dev = dev;
	xgmac->config = (void *)dev_get_driver_data(dev);

	xgmac->regs = dev_read_addr(dev);
	if (xgmac->regs == FDT_ADDR_T_NONE) {
		pr_err("%s dev_read_addr() failed\n", dev->name);
		return -ENODEV;
	}
	xgmac->mac_regs = (void *)(xgmac->regs + XGMAC_MAC_REGS_BASE);
	xgmac->mtl_regs = (void *)(xgmac->regs + XGMAC_MTL_REGS_BASE);
	xgmac->dma_regs = (void *)(xgmac->regs + XGMAC_DMA_REGS_BASE);

	xgmac->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	ret = xgmac_probe_resources_core(dev);
	if (ret < 0) {
		pr_err("%s xgmac_probe_resources_core() failed: %d\n",
		       dev->name, ret);

		return ret;
	}

	ret = xgmac->config->ops->xgmac_probe_resources(dev);
	if (ret < 0) {
		pr_err("%s xgmac_probe_resources() failed: %d\n",
		       dev->name, ret);

		goto err_remove_resources_core;
	}

	ret = xgmac->config->ops->xgmac_start_clks(dev);
	if (ret < 0) {
		pr_err("%s xgmac_start_clks() failed: %d\n", dev->name, ret);
		goto err_remove_resources_core;
	}

	if (IS_ENABLED(CONFIG_DM_ETH_PHY))
		xgmac->mii = eth_phy_get_mdio_bus(dev);

	if (!xgmac->mii) {
		xgmac->mii = mdio_alloc();
		if (!xgmac->mii) {
			pr_err("%s mdio_alloc() failed\n", dev->name);
			ret = -ENOMEM;
			goto err_stop_clks;
		}
		xgmac->mii->read = xgmac_mdio_read;
		xgmac->mii->write = xgmac_mdio_write;
		xgmac->mii->priv = xgmac;
		strcpy(xgmac->mii->name, dev->name);

		ret = mdio_register(xgmac->mii);
		if (ret < 0) {
			pr_err("%s mdio_register() failed: %d\n",
			       dev->name, ret);

			goto err_free_mdio;
		}
	}

	if (IS_ENABLED(CONFIG_DM_ETH_PHY))
		eth_phy_set_mdio_bus(dev, xgmac->mii);

	debug("%s: OK\n", __func__);
	return 0;

err_free_mdio:
	mdio_free(xgmac->mii);
err_stop_clks:
	xgmac->config->ops->xgmac_stop_clks(dev);
err_remove_resources_core:
	xgmac_remove_resources_core(dev);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int xgmac_remove(struct udevice *dev)
{
	struct xgmac_priv *xgmac = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	mdio_unregister(xgmac->mii);
	mdio_free(xgmac->mii);
	xgmac->config->ops->xgmac_stop_clks(dev);
	xgmac->config->ops->xgmac_remove_resources(dev);

	xgmac_remove_resources_core(dev);

	debug("%s: OK\n", __func__);
	return 0;
}

int xgmac_null_ops(struct udevice *dev)
{
	return 0;
}

static const struct eth_ops xgmac_ops = {
	.start = xgmac_start,
	.stop = xgmac_stop,
	.send = xgmac_send,
	.recv = xgmac_recv,
	.free_pkt = xgmac_free_pkt,
	.write_hwaddr = xgmac_write_hwaddr,
	.read_rom_hwaddr = xgmac_read_rom_hwaddr,
};

static const struct udevice_id xgmac_ids[] = {
	{
		.compatible = "intel,socfpga-dwxgmac",
		.data = (ulong)&xgmac_socfpga_config
	},
	{ }
};

U_BOOT_DRIVER(eth_xgmac) = {
	.name = "eth_xgmac",
	.id = UCLASS_ETH,
	.of_match = of_match_ptr(xgmac_ids),
	.probe = xgmac_probe,
	.remove = xgmac_remove,
	.ops = &xgmac_ops,
	.priv_auto = sizeof(struct xgmac_priv),
	.plat_auto = sizeof(struct eth_pdata),
};

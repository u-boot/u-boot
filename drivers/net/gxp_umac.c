// SPDX-License-Identifier: GPL-2.0+
/*
 * HPE GXP UMAC ethernet driver
 *
 * Copyright (C) 2020 Hewlett Packard Enterprise Development LP.
 * Copyright (C) 2026 9elements GmbH
 */

#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#include "gxp_umac.h"

#define GXP_UMAC_TX_TIMEOUT_MS		1000

static int gxp_umac_mdio_read(struct mii_dev *bus, int phy_addr, int dev_addr,
			      int reg_addr)
{
	struct gxp_umac_priv *priv = bus->priv;
	struct gxp_mdio_regs *mdio = priv->mdio_regs;
	u32 val;
	int ret;

	val = readl(&mdio->mmi);
	val &= ~(UMAC_MMI_PHY_ADDR_MASK | UMAC_MMI_REG_ADDR);
	val |= (phy_addr << UMAC_MMI_PHY_ADDR_SHIFT) & UMAC_MMI_PHY_ADDR_MASK;
	val |= reg_addr & UMAC_MMI_REG_ADDR;
	val |= UMAC_MMI_MRNW;  /* Read operation */
	writel(val, &mdio->mmi);

	/* Activate transfer */
	val |= UMAC_MMI_MOWNER;
	writel(val, &mdio->mmi);

	/* Wait for transfer to complete */
	ret = readl_poll_timeout(&mdio->mmi, val, !(val & UMAC_MMI_MOWNER),
				 GXP_MDIO_TIMEOUT_USEC);
	if (ret) {
		pr_err("%s: mdio read timeout (phy:%d reg:%x)\n",
		       bus->name, phy_addr, reg_addr);
		return ret;
	}

	return readl(&mdio->mmi_data) & UMAC_MMI_DATA_MASK;
}

static int gxp_umac_mdio_write(struct mii_dev *bus, int phy_addr, int dev_addr,
			       int reg_addr, u16 value)
{
	struct gxp_umac_priv *priv = bus->priv;
	struct gxp_mdio_regs *mdio = priv->mdio_regs;
	u32 val;
	int ret;

	/* Write data first */
	writel(value & UMAC_MMI_DATA_MASK, &mdio->mmi_data);

	val = readl(&mdio->mmi);
	val &= ~(UMAC_MMI_PHY_ADDR_MASK | UMAC_MMI_REG_ADDR | UMAC_MMI_MRNW);
	val |= (phy_addr << UMAC_MMI_PHY_ADDR_SHIFT) & UMAC_MMI_PHY_ADDR_MASK;
	val |= reg_addr & UMAC_MMI_REG_ADDR;
	/* MRNW cleared = write operation */
	writel(val, &mdio->mmi);

	/* Activate transfer */
	val |= UMAC_MMI_MOWNER;
	writel(val, &mdio->mmi);

	/* Wait for transfer to complete */
	ret = readl_poll_timeout(&mdio->mmi, val, !(val & UMAC_MMI_MOWNER),
				 GXP_MDIO_TIMEOUT_USEC);
	if (ret) {
		pr_err("%s: mdio write timeout (phy:%d reg:%x)\n",
		       bus->name, phy_addr, reg_addr);
	}

	return ret;
}

static int gxp_umac_mdio_init(struct udevice *dev)
{
	struct gxp_umac_priv *priv = dev_get_priv(dev);
	struct mii_dev *bus;
	int ret;

	bus = mdio_alloc();
	if (!bus)
		return -ENOMEM;

	bus->read = gxp_umac_mdio_read;
	bus->write = gxp_umac_mdio_write;
	bus->priv = priv;

	ret = mdio_register_seq(bus, dev_seq(dev));
	if (ret) {
		free(bus);
		return ret;
	}

	priv->bus = bus;
	return 0;
}

static u32 gxp_umac_get_ring_size_val(u32 num_entries)
{
	switch (num_entries) {
	case 4:
		return 0;
	case 8:
		return 1;
	case 16:
		return 3;
	case 32:
		return 7;
	case 64:
		return 0xf;
	case 128:
		return 0x1f;
	case 256:
		return 0x3f;
	default:
		return 1;  /* Default to 8 entries */
	}
}

static void gxp_umac_init_rings(struct gxp_umac_priv *priv)
{
	int i;

	memset(priv->txdes, 0, UMAC_MAX_RING_ENTRIES * sizeof(*priv->txdes));
	memset(priv->rxdes, 0, UMAC_MAX_RING_ENTRIES * sizeof(*priv->rxdes));

	priv->tx_index = 0;
	priv->rx_index = 0;

	/* Initialize RX descriptors with buffer addresses */
	for (i = 0; i < UMAC_MAX_RING_ENTRIES; i++) {
		priv->rxdes[i].dma_address = (u32)&priv->rx_packets[i];
		priv->rxdes[i].count = UMAC_MAX_FRAME_SIZE;
		priv->rxdes[i].status = UMAC_RING_ENTRY_HW_OWN;
	}

	/* Flush descriptors to memory */
	flush_dcache_range((ulong)priv->txdes,
			   (ulong)priv->txdes + UMAC_MAX_RING_ENTRIES * sizeof(*priv->txdes));
	flush_dcache_range((ulong)priv->rxdes,
			   (ulong)priv->rxdes + UMAC_MAX_RING_ENTRIES * sizeof(*priv->rxdes));
	flush_dcache_range((ulong)priv->rx_packets,
			   (ulong)priv->rx_packets +
			   UMAC_MAX_RING_ENTRIES * sizeof(*priv->rx_packets));
}

static void gxp_umac_set_mac_addr(struct gxp_umac_priv *priv, const u8 *addr)
{
	struct gxp_umac_regs *regs = priv->regs;

	writel((addr[0] << 8) | addr[1], &regs->mac_addr_hi);
	writel((addr[2] << 8) | addr[3], &regs->mac_addr_mid);
	writel((addr[4] << 8) | addr[5], &regs->mac_addr_lo);
}

static void gxp_umac_set_channel_enable(struct gxp_umac_priv *priv, int enable)
{
	struct gxp_umac_regs *regs = priv->regs;
	u32 val;

	if (enable) {
		val = readl(&regs->config_status);
		val |= UMAC_CFG_TXEN | UMAC_CFG_RXEN;
		writel(val, &regs->config_status);
		writel(0, &regs->ring_prompt);
	} else {
		writel(0, &regs->config_status);
	}
}

static void gxp_umac_adjust_link(struct gxp_umac_priv *priv)
{
	struct gxp_umac_regs *regs = priv->regs;
	struct phy_device *phydev = priv->phydev;
	u32 val;

	/* Disable MAC during configuration */
	gxp_umac_set_channel_enable(priv, 0);

	/* Disable both clocks first */
	val = readl(&regs->config_status);
	val &= ~(UMAC_CFG_GIGE_EN | UMAC_CFG_100M_EN);
	writel(val, &regs->config_status);
	udelay(2);

	/* Configure speed and duplex */
	val &= ~(UMAC_CFG_GIGE_EN | UMAC_CFG_100M_EN | UMAC_CFG_GIGE_MODE |
		 UMAC_CFG_FULL_DUPLEX);

	if (phydev->speed == 1000)
		val |= UMAC_CFG_GIGE_EN | UMAC_CFG_GIGE_MODE;
	else
		val |= UMAC_CFG_100M_EN;

	if (phydev->duplex)
		val |= UMAC_CFG_FULL_DUPLEX;

	writel(val, &regs->config_status);
	udelay(2);

	/* Re-enable MAC */
	gxp_umac_set_channel_enable(priv, 1);
}

static void gxp_umac_init_hw(struct gxp_umac_priv *priv)
{
	struct gxp_umac_regs *regs = priv->regs;
	u32 ring_size;

	/* Reset ring pointers */
	writel(0, &regs->ring_ptr);

	/* Clear missed status */
	writel(0, &regs->clear_status);

	/* Disable checksum generation */
	writel(0, &regs->cksum_config);

	/* Set ring size */
	ring_size = gxp_umac_get_ring_size_val(UMAC_MAX_RING_ENTRIES);
	writel((ring_size << UMAC_RING_SIZE_TX_SHIFT) |
	       (ring_size << UMAC_RING_SIZE_RX_SHIFT), &regs->ring_size);

	/* Set ring base addresses */
	writel((u32)priv->rxdes, &regs->rx_ring_addr);
	writel((u32)priv->txdes, &regs->tx_ring_addr);

	/* Set DMA burst size */
	writel(0x22, &regs->dma_config);

	/* Disable clocks initially */
	writel(0, &regs->config_status);
}

static int gxp_umac_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct gxp_umac_priv *priv = dev_get_priv(dev);
	struct phy_device *phydev = priv->phydev;
	int ret;

	/* Initialize ring buffers */
	gxp_umac_init_rings(priv);

	/* Initialize hardware */
	gxp_umac_init_hw(priv);

	/* Set MAC address */
	gxp_umac_set_mac_addr(priv, pdata->enetaddr);

	/* Start PHY */
	ret = phy_startup(phydev);
	if (ret) {
		dev_err(dev, "Could not start PHY\n");
		return ret;
	}

	if (!phydev->link) {
		dev_err(dev, "No link\n");
		return -ENOLINK;
	}

	/* Configure MAC for link speed/duplex */
	gxp_umac_adjust_link(priv);

	printf("%s: link up, %d Mbps %s-duplex\n", dev->name,
	       phydev->speed, phydev->duplex ? "full" : "half");

	return 0;
}

static void gxp_umac_stop(struct udevice *dev)
{
	struct gxp_umac_priv *priv = dev_get_priv(dev);

	gxp_umac_set_channel_enable(priv, 0);

	if (priv->phydev)
		phy_shutdown(priv->phydev);
}

static int gxp_umac_send(struct udevice *dev, void *packet, int length)
{
	struct gxp_umac_priv *priv = dev_get_priv(dev);
	struct gxp_umac_regs *regs = priv->regs;
	struct gxp_umac_tx_desc *txdes = &priv->txdes[priv->tx_index];
	ulong des_start, des_end, data_start, data_end;
	u32 val;
	int timeout;

	/* Wait for descriptor to be available */
	des_start = (ulong)txdes & ~(ARCH_DMA_MINALIGN - 1);
	des_end = des_start + roundup(sizeof(*txdes), ARCH_DMA_MINALIGN);
	invalidate_dcache_range(des_start, des_end);

	if (txdes->status & UMAC_RING_ENTRY_HW_OWN) {
		dev_err(dev, "TX ring full\n");
		return -EBUSY;
	}

	/* Pad short frames */
	if (length < UMAC_MIN_FRAME_SIZE)
		length = UMAC_MIN_FRAME_SIZE;

	/* Set up descriptor */
	txdes->dma_address = (u32)packet;
	txdes->count = length;
	txdes->cksum_offset = 0;
	txdes->status = UMAC_RING_ENTRY_HW_OWN;

	/* Flush packet data and descriptor */
	data_start = (ulong)packet & ~(ARCH_DMA_MINALIGN - 1);
	data_end = data_start + roundup(length, ARCH_DMA_MINALIGN);
	flush_dcache_range(data_start, data_end);
	flush_dcache_range(des_start, des_end);

	/* Trigger transmission */
	writel(0, &regs->ring_prompt);

	/* Wait for TX interrupt */
	timeout = GXP_UMAC_TX_TIMEOUT_MS * 1000;
	do {
		val = readl(&regs->interrupt);
		if (val & UMAC_INT_TX)
			break;
		udelay(1);
	} while (--timeout > 0);

	/* Clear TX interrupt */
	writel(UMAC_INT_TX, &regs->interrupt);

	/* Wait for descriptor to be released */
	timeout = GXP_UMAC_TX_TIMEOUT_MS * 1000;
	do {
		invalidate_dcache_range(des_start, des_end);
		if (!(txdes->status & UMAC_RING_ENTRY_HW_OWN))
			break;
		udelay(1);
	} while (--timeout > 0);

	if (timeout <= 0) {
		dev_err(dev, "TX timeout\n");
		return -ETIMEDOUT;
	}

	/* Move to next descriptor */
	priv->tx_index = (priv->tx_index + 1) % UMAC_MAX_RING_ENTRIES;

	return 0;
}

static int gxp_umac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct gxp_umac_priv *priv = dev_get_priv(dev);
	struct gxp_umac_regs *regs = priv->regs;
	struct gxp_umac_rx_desc *rxdes = &priv->rxdes[priv->rx_index];
	ulong des_start, des_end, data_start, data_end;
	u32 val;
	int length;

	/* Check and clear interrupts */
	val = readl(&regs->interrupt);
	if (val & UMAC_INT_OVERRUN)
		dev_warn(dev, "RX overrun\n");
	if (val)
		writel(val, &regs->interrupt);

	/* Check for missed frames */
	val = readl(&regs->config_status);
	if (val & UMAC_CFG_MISSED) {
		dev_warn(dev, "RX missed frame\n");
		writel(0, &regs->clear_status);
	}

	/* Invalidate descriptor */
	des_start = (ulong)rxdes & ~(ARCH_DMA_MINALIGN - 1);
	des_end = des_start + roundup(sizeof(*rxdes), ARCH_DMA_MINALIGN);
	invalidate_dcache_range(des_start, des_end);

	/* Check if descriptor is owned by hardware */
	if (rxdes->status & UMAC_RING_ENTRY_HW_OWN) {
		/* Check for errors even if HW owned */
		if (rxdes->status & UMAC_RING_RX_ERR_MASK) {
			rxdes->status &= ~UMAC_RING_RX_ERR_MASK;
			flush_dcache_range(des_start, des_end);
		}
		return -EAGAIN;
	}

	length = rxdes->count;

	/* Invalidate received data */
	data_start = (ulong)rxdes->dma_address & ~(ARCH_DMA_MINALIGN - 1);
	data_end = data_start + roundup(length, ARCH_DMA_MINALIGN);
	invalidate_dcache_range(data_start, data_end);

	*packetp = (uchar *)rxdes->dma_address;

	return length;
}

static int gxp_umac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct gxp_umac_priv *priv = dev_get_priv(dev);
	struct gxp_umac_regs *regs = priv->regs;
	struct gxp_umac_rx_desc *rxdes = &priv->rxdes[priv->rx_index];
	ulong des_start, des_end;

	/* Return descriptor to hardware */
	rxdes->count = UMAC_MAX_FRAME_SIZE;
	rxdes->status = UMAC_RING_ENTRY_HW_OWN;

	/* Flush descriptor */
	des_start = (ulong)rxdes & ~(ARCH_DMA_MINALIGN - 1);
	des_end = des_start + roundup(sizeof(*rxdes), ARCH_DMA_MINALIGN);
	flush_dcache_range(des_start, des_end);

	/* Prompt hardware to check for new buffers */
	writel(0, &regs->ring_prompt);

	/* Move to next descriptor */
	priv->rx_index = (priv->rx_index + 1) % UMAC_MAX_RING_ENTRIES;

	return 0;
}

static int gxp_umac_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct gxp_umac_priv *priv = dev_get_priv(dev);

	gxp_umac_set_mac_addr(priv, pdata->enetaddr);
	return 0;
}

static int gxp_umac_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);

	pdata->iobase = dev_read_addr(dev);
	if (pdata->iobase == FDT_ADDR_T_NONE) {
		dev_err(dev, "Could not read register address\n");
		return -EINVAL;
	}

	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		pdata->phy_interface = PHY_INTERFACE_MODE_SGMII;

	return 0;
}

static int gxp_umac_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct gxp_umac_priv *priv = dev_get_priv(dev);
	int ret;

	priv->regs = (struct gxp_umac_regs *)pdata->iobase;
	priv->mdio_regs = (struct gxp_mdio_regs *)(pdata->iobase + 0x80);
	priv->phy_mode = pdata->phy_interface;
	priv->phy_addr = 0;

	/* Allocate DMA-aligned ring buffers */
	priv->txdes = memalign(ARCH_DMA_MINALIGN,
			       UMAC_MAX_RING_ENTRIES * sizeof(*priv->txdes));
	if (!priv->txdes)
		return -ENOMEM;

	priv->rxdes = memalign(ARCH_DMA_MINALIGN,
			       UMAC_MAX_RING_ENTRIES * sizeof(*priv->rxdes));
	if (!priv->rxdes) {
		ret = -ENOMEM;
		goto err_free_txdes;
	}

	priv->rx_packets = memalign(ARCH_DMA_MINALIGN,
				    UMAC_MAX_RING_ENTRIES * sizeof(*priv->rx_packets));
	if (!priv->rx_packets) {
		ret = -ENOMEM;
		goto err_free_rxdes;
	}

	/* Initialize MDIO bus */
	ret = gxp_umac_mdio_init(dev);
	if (ret) {
		dev_err(dev, "Failed to initialize MDIO bus\n");
		goto err_free_rx_packets;
	}

	priv->phydev = phy_connect(priv->bus, priv->phy_addr, dev, priv->phy_mode);
	if (!priv->phydev) {
		dev_err(dev, "Failed to connect to PHY\n");
		ret = -ENODEV;
		goto err_free_mdio;
	}

	/* Configure PHY */
	priv->phydev->supported &= PHY_GBIT_FEATURES;
	priv->phydev->advertising = priv->phydev->supported;
	phy_config(priv->phydev);

	return 0;

err_free_mdio:
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);
err_free_rx_packets:
	free(priv->rx_packets);
err_free_rxdes:
	free(priv->rxdes);
err_free_txdes:
	free(priv->txdes);
	return ret;
}

static int gxp_umac_remove(struct udevice *dev)
{
	struct gxp_umac_priv *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);
	free(priv->rx_packets);
	free(priv->rxdes);
	free(priv->txdes);

	return 0;
}

static const struct eth_ops gxp_umac_ops = {
	.start		= gxp_umac_start,
	.send		= gxp_umac_send,
	.recv		= gxp_umac_recv,
	.stop		= gxp_umac_stop,
	.free_pkt	= gxp_umac_free_pkt,
	.write_hwaddr	= gxp_umac_write_hwaddr,
};

static const struct udevice_id gxp_umac_ids[] = {
	{ .compatible = "hpe,gxp-umac" },
	{ }
};

U_BOOT_DRIVER(gxp_umac) = {
	.name		= "gxp_umac",
	.id		= UCLASS_ETH,
	.of_match	= gxp_umac_ids,
	.of_to_plat	= gxp_umac_of_to_plat,
	.probe		= gxp_umac_probe,
	.remove		= gxp_umac_remove,
	.ops		= &gxp_umac_ops,
	.priv_auto	= sizeof(struct gxp_umac_priv),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};

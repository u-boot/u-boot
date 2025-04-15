// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday FTGMAC100 Ethernet
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * (C) Copyright 2010 Andes Technology
 * Macpaul Lin <macpaul@andestech.com>
 *
 * Copyright (C) 2018, IBM Corporation.
 */

#include <clk.h>
#include <reset.h>
#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <wait_bit.h>
#include <asm/cache.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/printk.h>
#include <linux/bitfield.h>

#include "ftgmac100.h"

/* Min frame ethernet frame size without FCS */
#define ETH_ZLEN			60

/* Receive Buffer Size Register - HW default is 0x640 */
#define FTGMAC100_RBSR_DEFAULT		0x640

/* PKTBUFSTX/PKTBUFSRX must both be power of 2 */
#define PKTBUFSTX	4	/* must be power of 2 */

/* Timeout for transmit */
#define FTGMAC100_TX_TIMEOUT_MS		1000

/* Timeout for a mdio read/write operation */
#define FTGMAC100_MDIO_TIMEOUT_USEC	10000

/*
 * MDC clock cycle threshold
 *
 * 20us * 100 = 2ms > (1 / 2.5Mhz) * 0x34
 */
#define MDC_CYCTHR			0x34

/*
 * ftgmac100 model variants
 */
enum ftgmac100_model {
	FTGMAC100_MODEL_FARADAY,
	FTGMAC100_MODEL_ASPEED,
	FTGMAC100_MODEL_ASPEED_AST2700,
};

union ftgmac100_dma_addr {
	dma_addr_t addr;
	struct {
		u32 lo;
		u32 hi;
	};
};

/**
 * struct ftgmac100_data - private data for the FTGMAC100 driver
 *
 * @iobase: The base address of the hardware registers
 * @txdes: The array of transmit descriptors
 * @rxdes: The array of receive descriptors
 * @tx_index: Transmit descriptor index in @txdes
 * @rx_index: Receive descriptor index in @rxdes
 * @phy_addr: The PHY interface address to use
 * @phydev: The PHY device backing the MAC
 * @bus: The mdio bus
 * @phy_mode: The mode of the PHY interface (rgmii, rmii, ...)
 * @max_speed: Maximum speed of Ethernet connection supported by MAC
 * @clks: The bulk of clocks assigned to the device in the DT
 * @rxdes0_edorr_mask: The bit number identifying the end of the RX ring buffer
 * @txdes0_edotr_mask: The bit number identifying the end of the TX ring buffer
 */
struct ftgmac100_data {
	struct ftgmac100 *iobase;

	struct ftgmac100_txdes txdes[PKTBUFSTX] __aligned(ARCH_DMA_MINALIGN);
	struct ftgmac100_rxdes rxdes[PKTBUFSRX] __aligned(ARCH_DMA_MINALIGN);
	int tx_index;
	int rx_index;

	u32 phy_addr;
	struct phy_device *phydev;
	struct mii_dev *bus;
	u32 phy_mode;
	u32 max_speed;

	struct clk_bulk clks;
	struct reset_ctl *reset_ctl;

	/* End of RX/TX ring buffer bits. Depend on model */
	u32 rxdes0_edorr_mask;
	u32 txdes0_edotr_mask;

	bool is_ast2700;
};

/*
 * struct mii_bus functions
 */
static int ftgmac100_mdio_read(struct mii_dev *bus, int phy_addr, int dev_addr,
			       int reg_addr)
{
	struct ftgmac100_data *priv = bus->priv;
	struct ftgmac100 *ftgmac100 = priv->iobase;
	int phycr;
	int data;
	int ret;

	phycr = FTGMAC100_PHYCR_MDC_CYCTHR(MDC_CYCTHR) |
		FTGMAC100_PHYCR_PHYAD(phy_addr) |
		FTGMAC100_PHYCR_REGAD(reg_addr) |
		FTGMAC100_PHYCR_MIIRD;
	writel(phycr, &ftgmac100->phycr);

	ret = readl_poll_timeout(&ftgmac100->phycr, phycr,
				 !(phycr & FTGMAC100_PHYCR_MIIRD),
				 FTGMAC100_MDIO_TIMEOUT_USEC);
	if (ret) {
		pr_err("%s: mdio read failed (phy:%d reg:%x)\n",
		       bus->name, phy_addr, reg_addr);
		return ret;
	}

	data = readl(&ftgmac100->phydata);

	return FTGMAC100_PHYDATA_MIIRDATA(data);
}

static int ftgmac100_mdio_write(struct mii_dev *bus, int phy_addr, int dev_addr,
				int reg_addr, u16 value)
{
	struct ftgmac100_data *priv = bus->priv;
	struct ftgmac100 *ftgmac100 = priv->iobase;
	int phycr;
	int data;
	int ret;

	phycr = FTGMAC100_PHYCR_MDC_CYCTHR(MDC_CYCTHR) |
		FTGMAC100_PHYCR_PHYAD(phy_addr) |
		FTGMAC100_PHYCR_REGAD(reg_addr) |
		FTGMAC100_PHYCR_MIIWR;
	data = FTGMAC100_PHYDATA_MIIWDATA(value);

	writel(data, &ftgmac100->phydata);
	writel(phycr, &ftgmac100->phycr);

	ret = readl_poll_timeout(&ftgmac100->phycr, phycr,
				 !(phycr & FTGMAC100_PHYCR_MIIWR),
				 FTGMAC100_MDIO_TIMEOUT_USEC);
	if (ret) {
		pr_err("%s: mdio write failed (phy:%d reg:%x)\n",
		       bus->name, phy_addr, reg_addr);
	}

	return ret;
}

static int ftgmac100_mdio_init(struct udevice *dev)
{
	struct ftgmac100_data *priv = dev_get_priv(dev);
	struct mii_dev *bus;
	int ret;

	bus = mdio_alloc();
	if (!bus)
		return -ENOMEM;

	bus->read  = ftgmac100_mdio_read;
	bus->write = ftgmac100_mdio_write;
	bus->priv  = priv;

	ret = mdio_register_seq(bus, dev_seq(dev));
	if (ret) {
		free(bus);
		return ret;
	}

	priv->bus = bus;

	return 0;
}

static int ftgmac100_phy_adjust_link(struct ftgmac100_data *priv)
{
	struct ftgmac100 *ftgmac100 = priv->iobase;
	struct phy_device *phydev = priv->phydev;
	u32 maccr;

	if (!phydev->link && priv->phy_mode != PHY_INTERFACE_MODE_NCSI) {
		dev_err(phydev->dev, "No link\n");
		return -EREMOTEIO;
	}

	/* read MAC control register and clear related bits */
	maccr = readl(&ftgmac100->maccr) &
		~(FTGMAC100_MACCR_GIGA_MODE |
		  FTGMAC100_MACCR_FAST_MODE |
		  FTGMAC100_MACCR_FULLDUP);

	if (phy_interface_is_rgmii(phydev) && phydev->speed == 1000)
		maccr |= FTGMAC100_MACCR_GIGA_MODE;

	if (phydev->speed == 100)
		maccr |= FTGMAC100_MACCR_FAST_MODE;

	if (phydev->duplex)
		maccr |= FTGMAC100_MACCR_FULLDUP;

	/* update MII config into maccr */
	writel(maccr, &ftgmac100->maccr);

	return 0;
}

static int ftgmac100_phy_init(struct udevice *dev)
{
	struct ftgmac100_data *priv = dev_get_priv(dev);
	struct phy_device *phydev;
	int ret;

	if (IS_ENABLED(CONFIG_DM_MDIO) && priv->phy_mode != PHY_INTERFACE_MODE_NCSI)
		phydev = dm_eth_phy_connect(dev);
	else
		phydev = phy_connect(priv->bus, priv->phy_addr, dev, priv->phy_mode);

	if (!phydev)
		return -ENODEV;

	if (priv->phy_mode != PHY_INTERFACE_MODE_NCSI)
		phydev->supported &= PHY_GBIT_FEATURES;
	if (priv->max_speed) {
		ret = phy_set_supported(phydev, priv->max_speed);
		if (ret)
			return ret;
	}
	phydev->advertising = phydev->supported;
	priv->phydev = phydev;
	phy_config(phydev);

	return 0;
}

/*
 * Reset MAC
 */
static void ftgmac100_reset(struct ftgmac100_data *priv)
{
	struct ftgmac100 *ftgmac100 = priv->iobase;

	debug("%s()\n", __func__);

	setbits_le32(&ftgmac100->maccr, FTGMAC100_MACCR_SW_RST);

	while (readl(&ftgmac100->maccr) & FTGMAC100_MACCR_SW_RST)
		;
}

/*
 * Set MAC address
 */
static int ftgmac100_set_mac(struct ftgmac100_data *priv,
			     const unsigned char *mac)
{
	struct ftgmac100 *ftgmac100 = priv->iobase;
	unsigned int maddr = mac[0] << 8 | mac[1];
	unsigned int laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

	debug("%s(%x %x)\n", __func__, maddr, laddr);

	writel(maddr, &ftgmac100->mac_madr);
	writel(laddr, &ftgmac100->mac_ladr);

	return 0;
}

/*
 * Get MAC address
 */
static int ftgmac100_get_mac(struct ftgmac100_data *priv,
				unsigned char *mac)
{
	struct ftgmac100 *ftgmac100 = priv->iobase;
	unsigned int maddr = readl(&ftgmac100->mac_madr);
	unsigned int laddr = readl(&ftgmac100->mac_ladr);

	debug("%s(%x %x)\n", __func__, maddr, laddr);

	mac[0] = (maddr >> 8) & 0xff;
	mac[1] =  maddr & 0xff;
	mac[2] = (laddr >> 24) & 0xff;
	mac[3] = (laddr >> 16) & 0xff;
	mac[4] = (laddr >> 8) & 0xff;
	mac[5] =  laddr & 0xff;

	return 0;
}

/*
 * disable transmitter, receiver
 */
static void ftgmac100_stop(struct udevice *dev)
{
	struct ftgmac100_data *priv = dev_get_priv(dev);
	struct ftgmac100 *ftgmac100 = priv->iobase;

	debug("%s()\n", __func__);

	writel(0, &ftgmac100->maccr);

	if (priv->phy_mode != PHY_INTERFACE_MODE_NCSI)
		phy_shutdown(priv->phydev);
}

static int ftgmac100_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct ftgmac100_data *priv = dev_get_priv(dev);
	struct ftgmac100 *ftgmac100 = priv->iobase;
	union ftgmac100_dma_addr dma_addr = {.hi = 0, .lo = 0};
	struct phy_device *phydev = priv->phydev;
	unsigned int maccr, dblac, desc_size;
	ulong start, end;
	int ret;
	int i;

	debug("%s()\n", __func__);

	ftgmac100_reset(priv);

	/* set the ethernet address */
	ftgmac100_set_mac(priv, plat->enetaddr);

	/* disable all interrupts */
	writel(0, &ftgmac100->ier);

	/* initialize descriptors */
	priv->tx_index = 0;
	priv->rx_index = 0;

	for (i = 0; i < PKTBUFSTX; i++) {
		priv->txdes[i].txdes2 = 0;
		priv->txdes[i].txdes3 = 0;
		priv->txdes[i].txdes0 = 0;
	}
	priv->txdes[PKTBUFSTX - 1].txdes0 = priv->txdes0_edotr_mask;

	start = ((ulong)&priv->txdes[0]) & ~(ARCH_DMA_MINALIGN - 1);
	end = start + roundup(sizeof(priv->txdes), ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

	for (i = 0; i < PKTBUFSRX; i++) {
		unsigned int ip_align = 0;

		dma_addr.addr = (dma_addr_t)net_rx_packets[i];
		priv->rxdes[i].rxdes2 = FIELD_PREP(FTGMAC100_RXDES2_RXBUF_BADR_HI, dma_addr.hi);
		/* For IP alignment */
		if ((dma_addr.lo & (PKTALIGN - 1)) == 0)
			ip_align = 2;
		priv->rxdes[i].rxdes3 = dma_addr.lo + ip_align;
		priv->rxdes[i].rxdes0 = 0;
	}
	priv->rxdes[PKTBUFSRX - 1].rxdes0 = priv->rxdes0_edorr_mask;

	start = ((ulong)&priv->rxdes[0]) & ~(ARCH_DMA_MINALIGN - 1);
	end = start + roundup(sizeof(priv->rxdes), ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

	/* transmit ring */
	dma_addr.addr = (dma_addr_t)priv->txdes;
	writel(dma_addr.lo, &ftgmac100->txr_badr);
	writel(dma_addr.hi, &ftgmac100->txr_badr_hi);

	/* receive ring */
	dma_addr.addr = (dma_addr_t)priv->rxdes;
	writel(dma_addr.lo, &ftgmac100->rxr_badr);
	writel(dma_addr.hi, &ftgmac100->rxr_badr_hi);

	/* Configure TX/RX decsriptor size
	 * This size is calculated based on cache line.
	 */
	desc_size = ARCH_DMA_MINALIGN / FTGMAC100_DESC_UNIT;
	/* The descriptor size is at least 2 descriptor units. */
	if (desc_size < 2)
		desc_size = 2;
	dblac = readl(&ftgmac100->dblac) & ~GENMASK(19, 12);
	dblac |= FTGMAC100_DBLAC_RXDES_SIZE(desc_size) | FTGMAC100_DBLAC_TXDES_SIZE(desc_size);
	writel(dblac, &ftgmac100->dblac);

	/* poll receive descriptor automatically */
	writel(FTGMAC100_APTC_RXPOLL_CNT(1), &ftgmac100->aptc);

	/* config receive buffer size register */
	writel(FTGMAC100_RBSR_SIZE(FTGMAC100_RBSR_DEFAULT), &ftgmac100->rbsr);

	/* enable transmitter, receiver */
	maccr = FTGMAC100_MACCR_TXMAC_EN |
		FTGMAC100_MACCR_RXMAC_EN |
		FTGMAC100_MACCR_TXDMA_EN |
		FTGMAC100_MACCR_RXDMA_EN |
		FTGMAC100_MACCR_CRC_APD |
		FTGMAC100_MACCR_FULLDUP |
		FTGMAC100_MACCR_RX_RUNT |
		FTGMAC100_MACCR_RX_BROADPKT;

	if (priv->is_ast2700 && (priv->phydev->interface == PHY_INTERFACE_MODE_RMII ||
				 priv->phydev->interface == PHY_INTERFACE_MODE_NCSI))
		maccr |= FTGMAC100_MACCR_RMII_ENABLE;

	writel(maccr, &ftgmac100->maccr);

	ret = phy_startup(phydev);
	if (ret) {
		dev_err(phydev->dev, "Could not start PHY\n");
		return ret;
	}

	ret = ftgmac100_phy_adjust_link(priv);
	if (ret) {
		dev_err(phydev->dev,  "Could not adjust link\n");
		return ret;
	}

	printf("%s: link up, %d Mbps %s-duplex mac:%pM\n", phydev->dev->name,
	       phydev->speed, phydev->duplex ? "full" : "half", plat->enetaddr);

	return 0;
}

static int ftgmac100_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct ftgmac100_data *priv = dev_get_priv(dev);
	struct ftgmac100_rxdes *curr_des = &priv->rxdes[priv->rx_index];
	ulong des_start = ((ulong)curr_des) & ~(ARCH_DMA_MINALIGN - 1);
	ulong des_end = des_start +
		roundup(sizeof(*curr_des), ARCH_DMA_MINALIGN);

	/*
	 * Make sure there are no stale data in write-back over this area, which
	 * might get written into the memory while the ftgmac100 also writes
	 * into the same memory area.
	 */
	flush_dcache_range((ulong)net_rx_packets[priv->rx_index],
			   (ulong)net_rx_packets[priv->rx_index] + PKTSIZE_ALIGN);

	/* Release buffer to DMA and flush descriptor */
	curr_des->rxdes0 &= ~FTGMAC100_RXDES0_RXPKT_RDY;
	flush_dcache_range(des_start, des_end);

	/* Move to next descriptor */
	priv->rx_index = (priv->rx_index + 1) % PKTBUFSRX;

	return 0;
}

/*
 * Get a data block via Ethernet
 */
static int ftgmac100_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ftgmac100_data *priv = dev_get_priv(dev);
	struct ftgmac100_rxdes *curr_des = &priv->rxdes[priv->rx_index];
	unsigned short rxlen;
	ulong des_start = ((ulong)curr_des) & ~(ARCH_DMA_MINALIGN - 1);
	ulong des_end = des_start +
		roundup(sizeof(*curr_des), ARCH_DMA_MINALIGN);
	union ftgmac100_dma_addr data_start = { .lo = 0, .hi = 0 };
	ulong data_end;

	data_start.hi = FIELD_GET(FTGMAC100_RXDES2_RXBUF_BADR_HI, curr_des->rxdes2);
	data_start.lo = curr_des->rxdes3;
	invalidate_dcache_range(des_start, des_end);

	if (!(curr_des->rxdes0 & FTGMAC100_RXDES0_RXPKT_RDY))
		return -EAGAIN;

	if (curr_des->rxdes0 & (FTGMAC100_RXDES0_RX_ERR |
				FTGMAC100_RXDES0_CRC_ERR |
				FTGMAC100_RXDES0_FTL |
				FTGMAC100_RXDES0_RUNT |
				FTGMAC100_RXDES0_RX_ODD_NB)) {
		return -EAGAIN;
	}

	rxlen = FTGMAC100_RXDES0_VDBC(curr_des->rxdes0);

	debug("%s(): RX buffer %d, %x received\n",
	       __func__, priv->rx_index, rxlen);

	/* Invalidate received data */
	data_end = data_start.addr + roundup(rxlen, ARCH_DMA_MINALIGN);
	invalidate_dcache_range(data_start.addr, data_end);
	*packetp = (uchar *)data_start.addr;

	return rxlen;
}

static u32 ftgmac100_read_txdesc(const void *desc)
{
	const struct ftgmac100_txdes *txdes = desc;
	ulong des_start = ((ulong)txdes) & ~(ARCH_DMA_MINALIGN - 1);
	ulong des_end = des_start + roundup(sizeof(*txdes), ARCH_DMA_MINALIGN);

	invalidate_dcache_range(des_start, des_end);

	return txdes->txdes0;
}

BUILD_WAIT_FOR_BIT(ftgmac100_txdone, u32, ftgmac100_read_txdesc)

/*
 * Send a data block via Ethernet
 */
static int ftgmac100_send(struct udevice *dev, void *packet, int length)
{
	struct ftgmac100_data *priv = dev_get_priv(dev);
	struct ftgmac100 *ftgmac100 = priv->iobase;
	struct ftgmac100_txdes *curr_des = &priv->txdes[priv->tx_index];
	union ftgmac100_dma_addr dma_addr;
	ulong des_start = ((ulong)curr_des) & ~(ARCH_DMA_MINALIGN - 1);
	ulong des_end = des_start +
		roundup(sizeof(*curr_des), ARCH_DMA_MINALIGN);
	ulong data_start;
	ulong data_end;
	int rc;

	invalidate_dcache_range(des_start, des_end);

	if (curr_des->txdes0 & FTGMAC100_TXDES0_TXDMA_OWN) {
		dev_err(dev, "no TX descriptor available\n");
		return -EPERM;
	}

	debug("%s(%x, %x)\n", __func__, (int)packet, length);

	length = (length < ETH_ZLEN) ? ETH_ZLEN : length;

	dma_addr.addr = (dma_addr_t)packet;
	curr_des->txdes2 = FIELD_PREP(FTGMAC100_TXDES2_TXBUF_BADR_HI, dma_addr.hi);
	curr_des->txdes3 = dma_addr.lo;

	/* Flush data to be sent */
	data_start = (ulong)dma_addr.addr;
	data_end = data_start + roundup(length, ARCH_DMA_MINALIGN);
	flush_dcache_range(data_start, data_end);

	/* Only one segment on TXBUF */
	curr_des->txdes0 &= priv->txdes0_edotr_mask;
	curr_des->txdes0 |= FTGMAC100_TXDES0_FTS |
			    FTGMAC100_TXDES0_LTS |
			    FTGMAC100_TXDES0_TXBUF_SIZE(length) |
			    FTGMAC100_TXDES0_TXDMA_OWN ;

	/* Flush modified buffer descriptor */
	flush_dcache_range(des_start, des_end);

	/* Start transmit */
	writel(1, &ftgmac100->txpd);

	rc = wait_for_bit_ftgmac100_txdone(curr_des,
					   FTGMAC100_TXDES0_TXDMA_OWN, false,
					   FTGMAC100_TX_TIMEOUT_MS, true);
	if (rc)
		return rc;

	debug("%s(): packet sent\n", __func__);

	/* Move to next descriptor */
	priv->tx_index = (priv->tx_index + 1) % PKTBUFSTX;

	return 0;
}

static int ftgmac100_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ftgmac100_data *priv = dev_get_priv(dev);

	return ftgmac100_set_mac(priv, pdata->enetaddr);
}

static int ftgmac_read_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ftgmac100_data *priv = dev_get_priv(dev);

	return ftgmac100_get_mac(priv, pdata->enetaddr);
}

static int ftgmac100_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ftgmac100_data *priv = dev_get_priv(dev);

	pdata->iobase = dev_read_addr(dev);

	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	pdata->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	if (dev_get_driver_data(dev) == FTGMAC100_MODEL_ASPEED) {
		priv->rxdes0_edorr_mask = BIT(30);
		priv->txdes0_edotr_mask = BIT(30);
		priv->is_ast2700 = false;
	} else if (dev_get_driver_data(dev) == FTGMAC100_MODEL_ASPEED_AST2700) {
		priv->rxdes0_edorr_mask = BIT(30);
		priv->txdes0_edotr_mask = BIT(30);
		priv->is_ast2700 = true;
	} else {
		priv->rxdes0_edorr_mask = BIT(15);
		priv->txdes0_edotr_mask = BIT(15);
	}

	priv->reset_ctl = devm_reset_control_get_optional(dev, NULL);

	return clk_get_bulk(dev, &priv->clks);
}

static int ftgmac100_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ftgmac100_data *priv = dev_get_priv(dev);
	int ret;

	priv->iobase = (struct ftgmac100 *)pdata->iobase;
	priv->phy_mode = pdata->phy_interface;
	priv->max_speed = pdata->max_speed;
	priv->phy_addr = 0;

	if (dev_read_bool(dev, "use-ncsi"))
		priv->phy_mode = PHY_INTERFACE_MODE_NCSI;

#ifdef CONFIG_PHY_ADDR
	priv->phy_addr = CONFIG_PHY_ADDR;
#endif

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		goto out;

	if (priv->reset_ctl) {
		ret = reset_deassert(priv->reset_ctl);
		if (ret)
			goto out;
	}

	/*
	 * If DM MDIO is enabled, the MDIO bus will be initialized later in
	 * dm_eth_phy_connect
	 */
	if (priv->phy_mode != PHY_INTERFACE_MODE_NCSI &&
	    !IS_ENABLED(CONFIG_DM_MDIO)) {
		ret = ftgmac100_mdio_init(dev);
		if (ret) {
			dev_err(dev, "Failed to initialize mdiobus: %d\n", ret);
			goto out;
		}
	}

	ret = ftgmac100_phy_init(dev);
	if (ret) {
		dev_err(dev, "Failed to initialize PHY: %d\n", ret);
		goto out;
	}

	ftgmac_read_hwaddr(dev);

out:
	if (ret)
		clk_release_bulk(&priv->clks);

	return ret;
}

static int ftgmac100_remove(struct udevice *dev)
{
	struct ftgmac100_data *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);
	if (priv->reset_ctl)
		reset_assert(priv->reset_ctl);
	clk_release_bulk(&priv->clks);

	return 0;
}

static const struct eth_ops ftgmac100_ops = {
	.start	= ftgmac100_start,
	.send	= ftgmac100_send,
	.recv	= ftgmac100_recv,
	.stop	= ftgmac100_stop,
	.free_pkt = ftgmac100_free_pkt,
	.write_hwaddr = ftgmac100_write_hwaddr,
};

static const struct udevice_id ftgmac100_ids[] = {
	{ .compatible = "faraday,ftgmac100", .data = FTGMAC100_MODEL_FARADAY },
	{ .compatible = "aspeed,ast2500-mac", .data = FTGMAC100_MODEL_ASPEED },
	{ .compatible = "aspeed,ast2600-mac", .data = FTGMAC100_MODEL_ASPEED },
	{ .compatible = "aspeed,ast2700-mac", .data = FTGMAC100_MODEL_ASPEED_AST2700 },
	{}
};

U_BOOT_DRIVER(ftgmac100) = {
	.name	= "ftgmac100",
	.id	= UCLASS_ETH,
	.of_match = ftgmac100_ids,
	.of_to_plat = ftgmac100_of_to_plat,
	.probe	= ftgmac100_probe,
	.remove = ftgmac100_remove,
	.ops	= &ftgmac100_ops,
	.priv_auto	= sizeof(struct ftgmac100_data),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

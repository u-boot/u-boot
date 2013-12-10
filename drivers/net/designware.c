/*
 * (C) Copyright 2010
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Designware ethernet IP driver for u-boot
 */

#include <common.h>
#include <miiphy.h>
#include <malloc.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <asm/io.h>
#include "designware.h"

static int configure_phy(struct eth_device *dev);

static void tx_descs_init(struct eth_device *dev)
{
	struct dw_eth_dev *priv = dev->priv;
	struct eth_dma_regs *dma_p = priv->dma_regs_p;
	struct dmamacdescr *desc_table_p = &priv->tx_mac_descrtable[0];
	char *txbuffs = &priv->txbuffs[0];
	struct dmamacdescr *desc_p;
	u32 idx;

	for (idx = 0; idx < CONFIG_TX_DESCR_NUM; idx++) {
		desc_p = &desc_table_p[idx];
		desc_p->dmamac_addr = &txbuffs[idx * CONFIG_ETH_BUFSIZE];
		desc_p->dmamac_next = &desc_table_p[idx + 1];

#if defined(CONFIG_DW_ALTDESCRIPTOR)
		desc_p->txrx_status &= ~(DESC_TXSTS_TXINT | DESC_TXSTS_TXLAST |
				DESC_TXSTS_TXFIRST | DESC_TXSTS_TXCRCDIS | \
				DESC_TXSTS_TXCHECKINSCTRL | \
				DESC_TXSTS_TXRINGEND | DESC_TXSTS_TXPADDIS);

		desc_p->txrx_status |= DESC_TXSTS_TXCHAIN;
		desc_p->dmamac_cntl = 0;
		desc_p->txrx_status &= ~(DESC_TXSTS_MSK | DESC_TXSTS_OWNBYDMA);
#else
		desc_p->dmamac_cntl = DESC_TXCTRL_TXCHAIN;
		desc_p->txrx_status = 0;
#endif
	}

	/* Correcting the last pointer of the chain */
	desc_p->dmamac_next = &desc_table_p[0];

	writel((ulong)&desc_table_p[0], &dma_p->txdesclistaddr);
}

static void rx_descs_init(struct eth_device *dev)
{
	struct dw_eth_dev *priv = dev->priv;
	struct eth_dma_regs *dma_p = priv->dma_regs_p;
	struct dmamacdescr *desc_table_p = &priv->rx_mac_descrtable[0];
	char *rxbuffs = &priv->rxbuffs[0];
	struct dmamacdescr *desc_p;
	u32 idx;

	for (idx = 0; idx < CONFIG_RX_DESCR_NUM; idx++) {
		desc_p = &desc_table_p[idx];
		desc_p->dmamac_addr = &rxbuffs[idx * CONFIG_ETH_BUFSIZE];
		desc_p->dmamac_next = &desc_table_p[idx + 1];

		desc_p->dmamac_cntl =
			(MAC_MAX_FRAME_SZ & DESC_RXCTRL_SIZE1MASK) | \
				      DESC_RXCTRL_RXCHAIN;

		desc_p->txrx_status = DESC_RXSTS_OWNBYDMA;
	}

	/* Correcting the last pointer of the chain */
	desc_p->dmamac_next = &desc_table_p[0];

	writel((ulong)&desc_table_p[0], &dma_p->rxdesclistaddr);
}

static void descs_init(struct eth_device *dev)
{
	tx_descs_init(dev);
	rx_descs_init(dev);
}

static int mac_reset(struct eth_device *dev)
{
	struct dw_eth_dev *priv = dev->priv;
	struct eth_mac_regs *mac_p = priv->mac_regs_p;
	struct eth_dma_regs *dma_p = priv->dma_regs_p;

	ulong start;
	int timeout = CONFIG_MACRESET_TIMEOUT;

	writel(readl(&dma_p->busmode) | DMAMAC_SRST, &dma_p->busmode);

	if (priv->interface != PHY_INTERFACE_MODE_RGMII)
		writel(MII_PORTSELECT, &mac_p->conf);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if (!(readl(&dma_p->busmode) & DMAMAC_SRST))
			return 0;

		/* Try again after 10usec */
		udelay(10);
	};

	return -1;
}

static int dw_write_hwaddr(struct eth_device *dev)
{
	struct dw_eth_dev *priv = dev->priv;
	struct eth_mac_regs *mac_p = priv->mac_regs_p;
	u32 macid_lo, macid_hi;
	u8 *mac_id = &dev->enetaddr[0];

	macid_lo = mac_id[0] + (mac_id[1] << 8) + \
		   (mac_id[2] << 16) + (mac_id[3] << 24);
	macid_hi = mac_id[4] + (mac_id[5] << 8);

	writel(macid_hi, &mac_p->macaddr0hi);
	writel(macid_lo, &mac_p->macaddr0lo);

	return 0;
}

static int dw_eth_init(struct eth_device *dev, bd_t *bis)
{
	struct dw_eth_dev *priv = dev->priv;
	struct eth_mac_regs *mac_p = priv->mac_regs_p;
	struct eth_dma_regs *dma_p = priv->dma_regs_p;
	u32 conf;

	if (priv->phy_configured != 1)
		configure_phy(dev);

	/* Print link status only once */
	if (!priv->link_printed) {
		printf("ENET Speed is %d Mbps - %s duplex connection\n",
		       priv->speed, (priv->duplex == HALF) ? "HALF" : "FULL");
		priv->link_printed = 1;
	}

	/* Reset ethernet hardware */
	if (mac_reset(dev) < 0)
		return -1;

	/* Resore the HW MAC address as it has been lost during MAC reset */
	dw_write_hwaddr(dev);

	writel(FIXEDBURST | PRIORXTX_41 | BURST_16,
			&dma_p->busmode);

	writel(readl(&dma_p->opmode) | FLUSHTXFIFO | STOREFORWARD |
		TXSECONDFRAME, &dma_p->opmode);

	conf = FRAMEBURSTENABLE | DISABLERXOWN;

	if (priv->speed != 1000)
		conf |= MII_PORTSELECT;

	if ((priv->interface != PHY_INTERFACE_MODE_MII) &&
		(priv->interface != PHY_INTERFACE_MODE_GMII)) {

		if (priv->speed == 100)
			conf |= FES_100;
	}

	if (priv->duplex == FULL)
		conf |= FULLDPLXMODE;

	writel(conf, &mac_p->conf);

	descs_init(dev);

	/*
	 * Start/Enable xfer at dma as well as mac level
	 */
	writel(readl(&dma_p->opmode) | RXSTART, &dma_p->opmode);
	writel(readl(&dma_p->opmode) | TXSTART, &dma_p->opmode);

	writel(readl(&mac_p->conf) | RXENABLE | TXENABLE, &mac_p->conf);

	return 0;
}

static int dw_eth_send(struct eth_device *dev, void *packet, int length)
{
	struct dw_eth_dev *priv = dev->priv;
	struct eth_dma_regs *dma_p = priv->dma_regs_p;
	u32 desc_num = priv->tx_currdescnum;
	struct dmamacdescr *desc_p = &priv->tx_mac_descrtable[desc_num];

	/* Check if the descriptor is owned by CPU */
	if (desc_p->txrx_status & DESC_TXSTS_OWNBYDMA) {
		printf("CPU not owner of tx frame\n");
		return -1;
	}

	memcpy((void *)desc_p->dmamac_addr, packet, length);

#if defined(CONFIG_DW_ALTDESCRIPTOR)
	desc_p->txrx_status |= DESC_TXSTS_TXFIRST | DESC_TXSTS_TXLAST;
	desc_p->dmamac_cntl |= (length << DESC_TXCTRL_SIZE1SHFT) & \
			       DESC_TXCTRL_SIZE1MASK;

	desc_p->txrx_status &= ~(DESC_TXSTS_MSK);
	desc_p->txrx_status |= DESC_TXSTS_OWNBYDMA;
#else
	desc_p->dmamac_cntl |= ((length << DESC_TXCTRL_SIZE1SHFT) & \
			       DESC_TXCTRL_SIZE1MASK) | DESC_TXCTRL_TXLAST | \
			       DESC_TXCTRL_TXFIRST;

	desc_p->txrx_status = DESC_TXSTS_OWNBYDMA;
#endif

	/* Test the wrap-around condition. */
	if (++desc_num >= CONFIG_TX_DESCR_NUM)
		desc_num = 0;

	priv->tx_currdescnum = desc_num;

	/* Start the transmission */
	writel(POLL_DATA, &dma_p->txpolldemand);

	return 0;
}

static int dw_eth_recv(struct eth_device *dev)
{
	struct dw_eth_dev *priv = dev->priv;
	u32 desc_num = priv->rx_currdescnum;
	struct dmamacdescr *desc_p = &priv->rx_mac_descrtable[desc_num];

	u32 status = desc_p->txrx_status;
	int length = 0;

	/* Check  if the owner is the CPU */
	if (!(status & DESC_RXSTS_OWNBYDMA)) {

		length = (status & DESC_RXSTS_FRMLENMSK) >> \
			 DESC_RXSTS_FRMLENSHFT;

		NetReceive(desc_p->dmamac_addr, length);

		/*
		 * Make the current descriptor valid again and go to
		 * the next one
		 */
		desc_p->txrx_status |= DESC_RXSTS_OWNBYDMA;

		/* Test the wrap-around condition. */
		if (++desc_num >= CONFIG_RX_DESCR_NUM)
			desc_num = 0;
	}

	priv->rx_currdescnum = desc_num;

	return length;
}

static void dw_eth_halt(struct eth_device *dev)
{
	struct dw_eth_dev *priv = dev->priv;

	mac_reset(dev);
	priv->tx_currdescnum = priv->rx_currdescnum = 0;
}

static int eth_mdio_read(struct eth_device *dev, u8 addr, u8 reg, u16 *val)
{
	struct dw_eth_dev *priv = dev->priv;
	struct eth_mac_regs *mac_p = priv->mac_regs_p;
	ulong start;
	u32 miiaddr;
	int timeout = CONFIG_MDIO_TIMEOUT;

	miiaddr = ((addr << MIIADDRSHIFT) & MII_ADDRMSK) | \
		  ((reg << MIIREGSHIFT) & MII_REGMSK);

	writel(miiaddr | MII_CLKRANGE_150_250M | MII_BUSY, &mac_p->miiaddr);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if (!(readl(&mac_p->miiaddr) & MII_BUSY)) {
			*val = readl(&mac_p->miidata);
			return 0;
		}

		/* Try again after 10usec */
		udelay(10);
	};

	return -1;
}

static int eth_mdio_write(struct eth_device *dev, u8 addr, u8 reg, u16 val)
{
	struct dw_eth_dev *priv = dev->priv;
	struct eth_mac_regs *mac_p = priv->mac_regs_p;
	ulong start;
	u32 miiaddr;
	int ret = -1, timeout = CONFIG_MDIO_TIMEOUT;
	u16 value;

	writel(val, &mac_p->miidata);
	miiaddr = ((addr << MIIADDRSHIFT) & MII_ADDRMSK) | \
		  ((reg << MIIREGSHIFT) & MII_REGMSK) | MII_WRITE;

	writel(miiaddr | MII_CLKRANGE_150_250M | MII_BUSY, &mac_p->miiaddr);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if (!(readl(&mac_p->miiaddr) & MII_BUSY)) {
			ret = 0;
			break;
		}

		/* Try again after 10usec */
		udelay(10);
	};

	/* Needed as a fix for ST-Phy */
	eth_mdio_read(dev, addr, reg, &value);

	return ret;
}

#if defined(CONFIG_DW_SEARCH_PHY)
static int find_phy(struct eth_device *dev)
{
	int phy_addr = 0;
	u16 ctrl, oldctrl;

	do {
		eth_mdio_read(dev, phy_addr, MII_BMCR, &ctrl);
		oldctrl = ctrl & BMCR_ANENABLE;

		ctrl ^= BMCR_ANENABLE;
		eth_mdio_write(dev, phy_addr, MII_BMCR, ctrl);
		eth_mdio_read(dev, phy_addr, MII_BMCR, &ctrl);
		ctrl &= BMCR_ANENABLE;

		if (ctrl == oldctrl) {
			phy_addr++;
		} else {
			ctrl ^= BMCR_ANENABLE;
			eth_mdio_write(dev, phy_addr, MII_BMCR, ctrl);

			return phy_addr;
		}
	} while (phy_addr < 32);

	return -1;
}
#endif

static int dw_reset_phy(struct eth_device *dev)
{
	struct dw_eth_dev *priv = dev->priv;
	u16 ctrl;
	ulong start;
	int timeout = CONFIG_PHYRESET_TIMEOUT;
	u32 phy_addr = priv->address;

	eth_mdio_write(dev, phy_addr, MII_BMCR, BMCR_RESET);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		eth_mdio_read(dev, phy_addr, MII_BMCR, &ctrl);
		if (!(ctrl & BMCR_RESET))
			break;

		/* Try again after 10usec */
		udelay(10);
	};

	if (get_timer(start) >= CONFIG_PHYRESET_TIMEOUT)
		return -1;

#ifdef CONFIG_PHY_RESET_DELAY
	udelay(CONFIG_PHY_RESET_DELAY);
#endif
	return 0;
}

/*
 * Add weak default function for board specific PHY configuration
 */
int __weak designware_board_phy_init(struct eth_device *dev, int phy_addr,
		int (*mii_write)(struct eth_device *, u8, u8, u16),
		int dw_reset_phy(struct eth_device *))
{
	return 0;
}

static int configure_phy(struct eth_device *dev)
{
	struct dw_eth_dev *priv = dev->priv;
	int phy_addr;
	u16 bmcr;
#if defined(CONFIG_DW_AUTONEG)
	u16 bmsr;
	u32 timeout;
	ulong start;
#endif

#if defined(CONFIG_DW_SEARCH_PHY)
	phy_addr = find_phy(dev);
	if (phy_addr >= 0)
		priv->address = phy_addr;
	else
		return -1;
#else
	phy_addr = priv->address;
#endif

	/*
	 * Some boards need board specific PHY initialization. This is
	 * after the main driver init code but before the auto negotiation
	 * is run.
	 */
	if (designware_board_phy_init(dev, phy_addr,
				      eth_mdio_write, dw_reset_phy) < 0)
		return -1;

	if (dw_reset_phy(dev) < 0)
		return -1;

#if defined(CONFIG_DW_AUTONEG)
	/* Set Auto-Neg Advertisement capabilities to 10/100 half/full */
	eth_mdio_write(dev, phy_addr, MII_ADVERTISE, 0x1E1);

	bmcr = BMCR_ANENABLE | BMCR_ANRESTART;
#else
	bmcr = BMCR_SPEED100 | BMCR_FULLDPLX;

#if defined(CONFIG_DW_SPEED10M)
	bmcr &= ~BMCR_SPEED100;
#endif
#if defined(CONFIG_DW_DUPLEXHALF)
	bmcr &= ~BMCR_FULLDPLX;
#endif
#endif
	if (eth_mdio_write(dev, phy_addr, MII_BMCR, bmcr) < 0)
		return -1;

	/* Read the phy status register and populate priv structure */
#if defined(CONFIG_DW_AUTONEG)
	timeout = CONFIG_AUTONEG_TIMEOUT;
	start = get_timer(0);
	puts("Waiting for PHY auto negotiation to complete");
	while (get_timer(start) < timeout) {
		eth_mdio_read(dev, phy_addr, MII_BMSR, &bmsr);
		if (bmsr & BMSR_ANEGCOMPLETE) {
			priv->phy_configured = 1;
			break;
		}

		/* Print dot all 1s to show progress */
		if ((get_timer(start) % 1000) == 0)
			putc('.');

		/* Try again after 1msec */
		udelay(1000);
	};

	if (!(bmsr & BMSR_ANEGCOMPLETE))
		puts(" TIMEOUT!\n");
	else
		puts(" done\n");
#else
	priv->phy_configured = 1;
#endif

	priv->speed = miiphy_speed(dev->name, phy_addr);
	priv->duplex = miiphy_duplex(dev->name, phy_addr);

	return 0;
}

#if defined(CONFIG_MII)
static int dw_mii_read(const char *devname, u8 addr, u8 reg, u16 *val)
{
	struct eth_device *dev;

	dev = eth_get_dev_by_name(devname);
	if (dev)
		eth_mdio_read(dev, addr, reg, val);

	return 0;
}

static int dw_mii_write(const char *devname, u8 addr, u8 reg, u16 val)
{
	struct eth_device *dev;

	dev = eth_get_dev_by_name(devname);
	if (dev)
		eth_mdio_write(dev, addr, reg, val);

	return 0;
}
#endif

int designware_initialize(u32 id, ulong base_addr, u32 phy_addr, u32 interface)
{
	struct eth_device *dev;
	struct dw_eth_dev *priv;

	dev = (struct eth_device *) malloc(sizeof(struct eth_device));
	if (!dev)
		return -ENOMEM;

	/*
	 * Since the priv structure contains the descriptors which need a strict
	 * buswidth alignment, memalign is used to allocate memory
	 */
	priv = (struct dw_eth_dev *) memalign(16, sizeof(struct dw_eth_dev));
	if (!priv) {
		free(dev);
		return -ENOMEM;
	}

	memset(dev, 0, sizeof(struct eth_device));
	memset(priv, 0, sizeof(struct dw_eth_dev));

	sprintf(dev->name, "mii%d", id);
	dev->iobase = (int)base_addr;
	dev->priv = priv;

	eth_getenv_enetaddr_by_index("eth", id, &dev->enetaddr[0]);

	priv->dev = dev;
	priv->mac_regs_p = (struct eth_mac_regs *)base_addr;
	priv->dma_regs_p = (struct eth_dma_regs *)(base_addr +
			DW_DMA_BASE_OFFSET);
	priv->address = phy_addr;
	priv->phy_configured = 0;
	priv->interface = interface;

	dev->init = dw_eth_init;
	dev->send = dw_eth_send;
	dev->recv = dw_eth_recv;
	dev->halt = dw_eth_halt;
	dev->write_hwaddr = dw_write_hwaddr;

	eth_register(dev);

#if defined(CONFIG_MII)
	miiphy_register(dev->name, dw_mii_read, dw_mii_write);
#endif
	return 1;
}

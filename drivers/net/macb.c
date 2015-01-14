/*
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

/*
 * The u-boot networking stack is a little weird.  It seems like the
 * networking core allocates receive buffers up front without any
 * regard to the hardware that's supposed to actually receive those
 * packets.
 *
 * The MACB receives packets into 128-byte receive buffers, so the
 * buffers allocated by the core isn't very practical to use.  We'll
 * allocate our own, but we need one such buffer in case a packet
 * wraps around the DMA ring so that we have to copy it.
 *
 * Therefore, define CONFIG_SYS_RX_ETH_BUFFER to 1 in the board-specific
 * configuration header.  This way, the core allocates one RX buffer
 * and one TX buffer, each of which can hold a ethernet packet of
 * maximum size.
 *
 * For some reason, the networking core unconditionally specifies a
 * 32-byte packet "alignment" (which really should be called
 * "padding").  MACB shouldn't need that, but we'll refrain from any
 * core modifications here...
 */

#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>

#include <linux/mii.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>
#include <asm/arch/clk.h>
#include <asm-generic/errno.h>

#include "macb.h"

#define MACB_RX_BUFFER_SIZE		4096
#define MACB_RX_RING_SIZE		(MACB_RX_BUFFER_SIZE / 128)
#define MACB_TX_RING_SIZE		16
#define MACB_TX_TIMEOUT		1000
#define MACB_AUTONEG_TIMEOUT	5000000

struct macb_dma_desc {
	u32	addr;
	u32	ctrl;
};

#define DMA_DESC_BYTES(n)	(n * sizeof(struct macb_dma_desc))
#define MACB_TX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_TX_RING_SIZE))
#define MACB_RX_DMA_DESC_SIZE	(DMA_DESC_BYTES(MACB_RX_RING_SIZE))

#define RXADDR_USED		0x00000001
#define RXADDR_WRAP		0x00000002

#define RXBUF_FRMLEN_MASK	0x00000fff
#define RXBUF_FRAME_START	0x00004000
#define RXBUF_FRAME_END		0x00008000
#define RXBUF_TYPEID_MATCH	0x00400000
#define RXBUF_ADDR4_MATCH	0x00800000
#define RXBUF_ADDR3_MATCH	0x01000000
#define RXBUF_ADDR2_MATCH	0x02000000
#define RXBUF_ADDR1_MATCH	0x04000000
#define RXBUF_BROADCAST		0x80000000

#define TXBUF_FRMLEN_MASK	0x000007ff
#define TXBUF_FRAME_END		0x00008000
#define TXBUF_NOCRC		0x00010000
#define TXBUF_EXHAUSTED		0x08000000
#define TXBUF_UNDERRUN		0x10000000
#define TXBUF_MAXRETRY		0x20000000
#define TXBUF_WRAP		0x40000000
#define TXBUF_USED		0x80000000

struct macb_device {
	void			*regs;

	unsigned int		rx_tail;
	unsigned int		tx_head;
	unsigned int		tx_tail;

	void			*rx_buffer;
	void			*tx_buffer;
	struct macb_dma_desc	*rx_ring;
	struct macb_dma_desc	*tx_ring;

	unsigned long		rx_buffer_dma;
	unsigned long		rx_ring_dma;
	unsigned long		tx_ring_dma;

	const struct device	*dev;
	struct eth_device	netdev;
	unsigned short		phy_addr;
	struct mii_dev		*bus;
};
#define to_macb(_nd) container_of(_nd, struct macb_device, netdev)

static int macb_is_gem(struct macb_device *macb)
{
	return MACB_BFEXT(IDNUM, macb_readl(macb, MID)) == 0x2;
}

static void macb_mdio_write(struct macb_device *macb, u8 reg, u16 value)
{
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;

	netctl = macb_readl(macb, NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 1)
		 | MACB_BF(PHYA, macb->phy_addr)
		 | MACB_BF(REGA, reg)
		 | MACB_BF(CODE, 2)
		 | MACB_BF(DATA, value));
	macb_writel(macb, MAN, frame);

	do {
		netstat = macb_readl(macb, NSR);
	} while (!(netstat & MACB_BIT(IDLE)));

	netctl = macb_readl(macb, NCR);
	netctl &= ~MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);
}

static u16 macb_mdio_read(struct macb_device *macb, u8 reg)
{
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;

	netctl = macb_readl(macb, NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 2)
		 | MACB_BF(PHYA, macb->phy_addr)
		 | MACB_BF(REGA, reg)
		 | MACB_BF(CODE, 2));
	macb_writel(macb, MAN, frame);

	do {
		netstat = macb_readl(macb, NSR);
	} while (!(netstat & MACB_BIT(IDLE)));

	frame = macb_readl(macb, MAN);

	netctl = macb_readl(macb, NCR);
	netctl &= ~MACB_BIT(MPE);
	macb_writel(macb, NCR, netctl);

	return MACB_BFEXT(DATA, frame);
}

void __weak arch_get_mdio_control(const char *name)
{
	return;
}

#if defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)

int macb_miiphy_read(const char *devname, u8 phy_adr, u8 reg, u16 *value)
{
	struct eth_device *dev = eth_get_dev_by_name(devname);
	struct macb_device *macb = to_macb(dev);

	if (macb->phy_addr != phy_adr)
		return -1;

	arch_get_mdio_control(devname);
	*value = macb_mdio_read(macb, reg);

	return 0;
}

int macb_miiphy_write(const char *devname, u8 phy_adr, u8 reg, u16 value)
{
	struct eth_device *dev = eth_get_dev_by_name(devname);
	struct macb_device *macb = to_macb(dev);

	if (macb->phy_addr != phy_adr)
		return -1;

	arch_get_mdio_control(devname);
	macb_mdio_write(macb, reg, value);

	return 0;
}
#endif

#define RX	1
#define TX	0
static inline void macb_invalidate_ring_desc(struct macb_device *macb, bool rx)
{
	if (rx)
		invalidate_dcache_range(macb->rx_ring_dma, macb->rx_ring_dma +
			MACB_RX_DMA_DESC_SIZE);
	else
		invalidate_dcache_range(macb->tx_ring_dma, macb->tx_ring_dma +
			MACB_TX_DMA_DESC_SIZE);
}

static inline void macb_flush_ring_desc(struct macb_device *macb, bool rx)
{
	if (rx)
		flush_dcache_range(macb->rx_ring_dma, macb->rx_ring_dma +
			MACB_RX_DMA_DESC_SIZE);
	else
		flush_dcache_range(macb->tx_ring_dma, macb->tx_ring_dma +
			MACB_TX_DMA_DESC_SIZE);
}

static inline void macb_flush_rx_buffer(struct macb_device *macb)
{
	flush_dcache_range(macb->rx_buffer_dma, macb->rx_buffer_dma +
				MACB_RX_BUFFER_SIZE);
}

static inline void macb_invalidate_rx_buffer(struct macb_device *macb)
{
	invalidate_dcache_range(macb->rx_buffer_dma, macb->rx_buffer_dma +
				MACB_RX_BUFFER_SIZE);
}

#if defined(CONFIG_CMD_NET)

static int macb_send(struct eth_device *netdev, void *packet, int length)
{
	struct macb_device *macb = to_macb(netdev);
	unsigned long paddr, ctrl;
	unsigned int tx_head = macb->tx_head;
	int i;

	paddr = dma_map_single(packet, length, DMA_TO_DEVICE);

	ctrl = length & TXBUF_FRMLEN_MASK;
	ctrl |= TXBUF_FRAME_END;
	if (tx_head == (MACB_TX_RING_SIZE - 1)) {
		ctrl |= TXBUF_WRAP;
		macb->tx_head = 0;
	} else {
		macb->tx_head++;
	}

	macb->tx_ring[tx_head].ctrl = ctrl;
	macb->tx_ring[tx_head].addr = paddr;
	barrier();
	macb_flush_ring_desc(macb, TX);
	/* Do we need check paddr and length is dcache line aligned? */
	flush_dcache_range(paddr, paddr + length);
	macb_writel(macb, NCR, MACB_BIT(TE) | MACB_BIT(RE) | MACB_BIT(TSTART));

	/*
	 * I guess this is necessary because the networking core may
	 * re-use the transmit buffer as soon as we return...
	 */
	for (i = 0; i <= MACB_TX_TIMEOUT; i++) {
		barrier();
		macb_invalidate_ring_desc(macb, TX);
		ctrl = macb->tx_ring[tx_head].ctrl;
		if (ctrl & TXBUF_USED)
			break;
		udelay(1);
	}

	dma_unmap_single(packet, length, paddr);

	if (i <= MACB_TX_TIMEOUT) {
		if (ctrl & TXBUF_UNDERRUN)
			printf("%s: TX underrun\n", netdev->name);
		if (ctrl & TXBUF_EXHAUSTED)
			printf("%s: TX buffers exhausted in mid frame\n",
			       netdev->name);
	} else {
		printf("%s: TX timeout\n", netdev->name);
	}

	/* No one cares anyway */
	return 0;
}

static void reclaim_rx_buffers(struct macb_device *macb,
			       unsigned int new_tail)
{
	unsigned int i;

	i = macb->rx_tail;

	macb_invalidate_ring_desc(macb, RX);
	while (i > new_tail) {
		macb->rx_ring[i].addr &= ~RXADDR_USED;
		i++;
		if (i > MACB_RX_RING_SIZE)
			i = 0;
	}

	while (i < new_tail) {
		macb->rx_ring[i].addr &= ~RXADDR_USED;
		i++;
	}

	barrier();
	macb_flush_ring_desc(macb, RX);
	macb->rx_tail = new_tail;
}

static int macb_recv(struct eth_device *netdev)
{
	struct macb_device *macb = to_macb(netdev);
	unsigned int rx_tail = macb->rx_tail;
	void *buffer;
	int length;
	int wrapped = 0;
	u32 status;

	for (;;) {
		macb_invalidate_ring_desc(macb, RX);

		if (!(macb->rx_ring[rx_tail].addr & RXADDR_USED))
			return -1;

		status = macb->rx_ring[rx_tail].ctrl;
		if (status & RXBUF_FRAME_START) {
			if (rx_tail != macb->rx_tail)
				reclaim_rx_buffers(macb, rx_tail);
			wrapped = 0;
		}

		if (status & RXBUF_FRAME_END) {
			buffer = macb->rx_buffer + 128 * macb->rx_tail;
			length = status & RXBUF_FRMLEN_MASK;

			macb_invalidate_rx_buffer(macb);
			if (wrapped) {
				unsigned int headlen, taillen;

				headlen = 128 * (MACB_RX_RING_SIZE
						 - macb->rx_tail);
				taillen = length - headlen;
				memcpy((void *)NetRxPackets[0],
				       buffer, headlen);
				memcpy((void *)NetRxPackets[0] + headlen,
				       macb->rx_buffer, taillen);
				buffer = (void *)NetRxPackets[0];
			}

			NetReceive(buffer, length);
			if (++rx_tail >= MACB_RX_RING_SIZE)
				rx_tail = 0;
			reclaim_rx_buffers(macb, rx_tail);
		} else {
			if (++rx_tail >= MACB_RX_RING_SIZE) {
				wrapped = 1;
				rx_tail = 0;
			}
		}
		barrier();
	}

	return 0;
}

static void macb_phy_reset(struct macb_device *macb)
{
	struct eth_device *netdev = &macb->netdev;
	int i;
	u16 status, adv;

	adv = ADVERTISE_CSMA | ADVERTISE_ALL;
	macb_mdio_write(macb, MII_ADVERTISE, adv);
	printf("%s: Starting autonegotiation...\n", netdev->name);
	macb_mdio_write(macb, MII_BMCR, (BMCR_ANENABLE
					 | BMCR_ANRESTART));

	for (i = 0; i < MACB_AUTONEG_TIMEOUT / 100; i++) {
		status = macb_mdio_read(macb, MII_BMSR);
		if (status & BMSR_ANEGCOMPLETE)
			break;
		udelay(100);
	}

	if (status & BMSR_ANEGCOMPLETE)
		printf("%s: Autonegotiation complete\n", netdev->name);
	else
		printf("%s: Autonegotiation timed out (status=0x%04x)\n",
		       netdev->name, status);
}

#ifdef CONFIG_MACB_SEARCH_PHY
static int macb_phy_find(struct macb_device *macb)
{
	int i;
	u16 phy_id;

	/* Search for PHY... */
	for (i = 0; i < 32; i++) {
		macb->phy_addr = i;
		phy_id = macb_mdio_read(macb, MII_PHYSID1);
		if (phy_id != 0xffff) {
			printf("%s: PHY present at %d\n", macb->netdev.name, i);
			return 1;
		}
	}

	/* PHY isn't up to snuff */
	printf("%s: PHY not found\n", macb->netdev.name);

	return 0;
}
#endif /* CONFIG_MACB_SEARCH_PHY */


static int macb_phy_init(struct macb_device *macb)
{
	struct eth_device *netdev = &macb->netdev;
#ifdef CONFIG_PHYLIB
	struct phy_device *phydev;
#endif
	u32 ncfgr;
	u16 phy_id, status, adv, lpa;
	int media, speed, duplex;
	int i;

	arch_get_mdio_control(netdev->name);
#ifdef CONFIG_MACB_SEARCH_PHY
	/* Auto-detect phy_addr */
	if (!macb_phy_find(macb))
		return 0;
#endif /* CONFIG_MACB_SEARCH_PHY */

	/* Check if the PHY is up to snuff... */
	phy_id = macb_mdio_read(macb, MII_PHYSID1);
	if (phy_id == 0xffff) {
		printf("%s: No PHY present\n", netdev->name);
		return 0;
	}

#ifdef CONFIG_PHYLIB
	/* need to consider other phy interface mode */
	phydev = phy_connect(macb->bus, macb->phy_addr, netdev,
			     PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		printf("phy_connect failed\n");
		return -ENODEV;
	}

	phy_config(phydev);
#endif

	status = macb_mdio_read(macb, MII_BMSR);
	if (!(status & BMSR_LSTATUS)) {
		/* Try to re-negotiate if we don't have link already. */
		macb_phy_reset(macb);

		for (i = 0; i < MACB_AUTONEG_TIMEOUT / 100; i++) {
			status = macb_mdio_read(macb, MII_BMSR);
			if (status & BMSR_LSTATUS)
				break;
			udelay(100);
		}
	}

	if (!(status & BMSR_LSTATUS)) {
		printf("%s: link down (status: 0x%04x)\n",
		       netdev->name, status);
		return 0;
	}

	/* First check for GMAC */
	if (macb_is_gem(macb)) {
		lpa = macb_mdio_read(macb, MII_STAT1000);

		if (lpa & (LPA_1000FULL | LPA_1000HALF)) {
			duplex = ((lpa & LPA_1000FULL) ? 1 : 0);

			printf("%s: link up, 1000Mbps %s-duplex (lpa: 0x%04x)\n",
			       netdev->name,
			       duplex ? "full" : "half",
			       lpa);

			ncfgr = macb_readl(macb, NCFGR);
			ncfgr &= ~(MACB_BIT(SPD) | MACB_BIT(FD));
			ncfgr |= GEM_BIT(GBE);

			if (duplex)
				ncfgr |= MACB_BIT(FD);

			macb_writel(macb, NCFGR, ncfgr);

			return 1;
		}
	}

	/* fall back for EMAC checking */
	adv = macb_mdio_read(macb, MII_ADVERTISE);
	lpa = macb_mdio_read(macb, MII_LPA);
	media = mii_nway_result(lpa & adv);
	speed = (media & (ADVERTISE_100FULL | ADVERTISE_100HALF)
		 ? 1 : 0);
	duplex = (media & ADVERTISE_FULL) ? 1 : 0;
	printf("%s: link up, %sMbps %s-duplex (lpa: 0x%04x)\n",
	       netdev->name,
	       speed ? "100" : "10",
	       duplex ? "full" : "half",
	       lpa);

	ncfgr = macb_readl(macb, NCFGR);
	ncfgr &= ~(MACB_BIT(SPD) | MACB_BIT(FD));
	if (speed)
		ncfgr |= MACB_BIT(SPD);
	if (duplex)
		ncfgr |= MACB_BIT(FD);
	macb_writel(macb, NCFGR, ncfgr);

	return 1;
}

static int macb_write_hwaddr(struct eth_device *dev);
static int macb_init(struct eth_device *netdev, bd_t *bd)
{
	struct macb_device *macb = to_macb(netdev);
	unsigned long paddr;
	int i;

	/*
	 * macb_halt should have been called at some point before now,
	 * so we'll assume the controller is idle.
	 */

	/* initialize DMA descriptors */
	paddr = macb->rx_buffer_dma;
	for (i = 0; i < MACB_RX_RING_SIZE; i++) {
		if (i == (MACB_RX_RING_SIZE - 1))
			paddr |= RXADDR_WRAP;
		macb->rx_ring[i].addr = paddr;
		macb->rx_ring[i].ctrl = 0;
		paddr += 128;
	}
	macb_flush_ring_desc(macb, RX);
	macb_flush_rx_buffer(macb);

	for (i = 0; i < MACB_TX_RING_SIZE; i++) {
		macb->tx_ring[i].addr = 0;
		if (i == (MACB_TX_RING_SIZE - 1))
			macb->tx_ring[i].ctrl = TXBUF_USED | TXBUF_WRAP;
		else
			macb->tx_ring[i].ctrl = TXBUF_USED;
	}
	macb_flush_ring_desc(macb, TX);

	macb->rx_tail = 0;
	macb->tx_head = 0;
	macb->tx_tail = 0;

	macb_writel(macb, RBQP, macb->rx_ring_dma);
	macb_writel(macb, TBQP, macb->tx_ring_dma);

	if (macb_is_gem(macb)) {
		/*
		 * When the GMAC IP with GE feature, this bit is used to
		 * select interface between RGMII and GMII.
		 * When the GMAC IP without GE feature, this bit is used
		 * to select interface between RMII and MII.
		 */
#if defined(CONFIG_RGMII) || defined(CONFIG_RMII)
		gem_writel(macb, UR, GEM_BIT(RGMII));
#else
		gem_writel(macb, UR, 0);
#endif
	} else {
	/* choose RMII or MII mode. This depends on the board */
#ifdef CONFIG_RMII
#ifdef CONFIG_AT91FAMILY
	macb_writel(macb, USRIO, MACB_BIT(RMII) | MACB_BIT(CLKEN));
#else
	macb_writel(macb, USRIO, 0);
#endif
#else
#ifdef CONFIG_AT91FAMILY
	macb_writel(macb, USRIO, MACB_BIT(CLKEN));
#else
	macb_writel(macb, USRIO, MACB_BIT(MII));
#endif
#endif /* CONFIG_RMII */
	}

	/* update the ethaddr */
	if (is_valid_ether_addr(netdev->enetaddr)) {
		macb_write_hwaddr(netdev);
	} else {
		printf("%s: mac address is not valid\n", netdev->name);
		return -1;
	}

	if (!macb_phy_init(macb))
		return -1;

	/* Enable TX and RX */
	macb_writel(macb, NCR, MACB_BIT(TE) | MACB_BIT(RE));

	return 0;
}

static void macb_halt(struct eth_device *netdev)
{
	struct macb_device *macb = to_macb(netdev);
	u32 ncr, tsr;

	/* Halt the controller and wait for any ongoing transmission to end. */
	ncr = macb_readl(macb, NCR);
	ncr |= MACB_BIT(THALT);
	macb_writel(macb, NCR, ncr);

	do {
		tsr = macb_readl(macb, TSR);
	} while (tsr & MACB_BIT(TGO));

	/* Disable TX and RX, and clear statistics */
	macb_writel(macb, NCR, MACB_BIT(CLRSTAT));
}

static int macb_write_hwaddr(struct eth_device *dev)
{
	struct macb_device *macb = to_macb(dev);
	u32 hwaddr_bottom;
	u16 hwaddr_top;

	/* set hardware address */
	hwaddr_bottom = dev->enetaddr[0] | dev->enetaddr[1] << 8 |
			dev->enetaddr[2] << 16 | dev->enetaddr[3] << 24;
	macb_writel(macb, SA1B, hwaddr_bottom);
	hwaddr_top = dev->enetaddr[4] | dev->enetaddr[5] << 8;
	macb_writel(macb, SA1T, hwaddr_top);
	return 0;
}

static u32 macb_mdc_clk_div(int id, struct macb_device *macb)
{
	u32 config;
	unsigned long macb_hz = get_macb_pclk_rate(id);

	if (macb_hz < 20000000)
		config = MACB_BF(CLK, MACB_CLK_DIV8);
	else if (macb_hz < 40000000)
		config = MACB_BF(CLK, MACB_CLK_DIV16);
	else if (macb_hz < 80000000)
		config = MACB_BF(CLK, MACB_CLK_DIV32);
	else
		config = MACB_BF(CLK, MACB_CLK_DIV64);

	return config;
}

static u32 gem_mdc_clk_div(int id, struct macb_device *macb)
{
	u32 config;
	unsigned long macb_hz = get_macb_pclk_rate(id);

	if (macb_hz < 20000000)
		config = GEM_BF(CLK, GEM_CLK_DIV8);
	else if (macb_hz < 40000000)
		config = GEM_BF(CLK, GEM_CLK_DIV16);
	else if (macb_hz < 80000000)
		config = GEM_BF(CLK, GEM_CLK_DIV32);
	else if (macb_hz < 120000000)
		config = GEM_BF(CLK, GEM_CLK_DIV48);
	else if (macb_hz < 160000000)
		config = GEM_BF(CLK, GEM_CLK_DIV64);
	else
		config = GEM_BF(CLK, GEM_CLK_DIV96);

	return config;
}

/*
 * Get the DMA bus width field of the network configuration register that we
 * should program. We find the width from decoding the design configuration
 * register to find the maximum supported data bus width.
 */
static u32 macb_dbw(struct macb_device *macb)
{
	switch (GEM_BFEXT(DBWDEF, gem_readl(macb, DCFG1))) {
	case 4:
		return GEM_BF(DBW, GEM_DBW128);
	case 2:
		return GEM_BF(DBW, GEM_DBW64);
	case 1:
	default:
		return GEM_BF(DBW, GEM_DBW32);
	}
}

int macb_eth_initialize(int id, void *regs, unsigned int phy_addr)
{
	struct macb_device *macb;
	struct eth_device *netdev;
	u32 ncfgr;

	macb = malloc(sizeof(struct macb_device));
	if (!macb) {
		printf("Error: Failed to allocate memory for MACB%d\n", id);
		return -1;
	}
	memset(macb, 0, sizeof(struct macb_device));

	netdev = &macb->netdev;

	macb->rx_buffer = dma_alloc_coherent(MACB_RX_BUFFER_SIZE,
					     &macb->rx_buffer_dma);
	macb->rx_ring = dma_alloc_coherent(MACB_RX_DMA_DESC_SIZE,
					   &macb->rx_ring_dma);
	macb->tx_ring = dma_alloc_coherent(MACB_TX_DMA_DESC_SIZE,
					   &macb->tx_ring_dma);

	/* TODO: we need check the rx/tx_ring_dma is dcache line aligned */

	macb->regs = regs;
	macb->phy_addr = phy_addr;

	if (macb_is_gem(macb))
		sprintf(netdev->name, "gmac%d", id);
	else
		sprintf(netdev->name, "macb%d", id);

	netdev->init = macb_init;
	netdev->halt = macb_halt;
	netdev->send = macb_send;
	netdev->recv = macb_recv;
	netdev->write_hwaddr = macb_write_hwaddr;

	/*
	 * Do some basic initialization so that we at least can talk
	 * to the PHY
	 */
	if (macb_is_gem(macb)) {
		ncfgr = gem_mdc_clk_div(id, macb);
		ncfgr |= macb_dbw(macb);
	} else {
		ncfgr = macb_mdc_clk_div(id, macb);
	}

	macb_writel(macb, NCFGR, ncfgr);

	eth_register(netdev);

#if defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	miiphy_register(netdev->name, macb_miiphy_read, macb_miiphy_write);
	macb->bus = miiphy_get_dev_by_name(netdev->name);
#endif
	return 0;
}

#endif

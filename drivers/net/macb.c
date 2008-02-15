/*
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <common.h>

#if defined(CONFIG_MACB) \
	&& (defined(CONFIG_CMD_NET) || defined(CONFIG_CMD_MII))

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
 * Therefore, define CFG_RX_ETH_BUFFER to 1 in the board-specific
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
#include <malloc.h>

#include <linux/mii.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>
#include <asm/arch/clk.h>

#include "macb.h"

#define barrier() asm volatile("" ::: "memory")

#define CFG_MACB_RX_BUFFER_SIZE		4096
#define CFG_MACB_RX_RING_SIZE		(CFG_MACB_RX_BUFFER_SIZE / 128)
#define CFG_MACB_TX_RING_SIZE		16
#define CFG_MACB_TX_TIMEOUT		1000
#define CFG_MACB_AUTONEG_TIMEOUT	5000000

struct macb_dma_desc {
	u32	addr;
	u32	ctrl;
};

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
};
#define to_macb(_nd) container_of(_nd, struct macb_device, netdev)

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

#if defined(CONFIG_CMD_NET)

static int macb_send(struct eth_device *netdev, volatile void *packet,
		     int length)
{
	struct macb_device *macb = to_macb(netdev);
	unsigned long paddr, ctrl;
	unsigned int tx_head = macb->tx_head;
	int i;

	paddr = dma_map_single(packet, length, DMA_TO_DEVICE);

	ctrl = length & TXBUF_FRMLEN_MASK;
	ctrl |= TXBUF_FRAME_END;
	if (tx_head == (CFG_MACB_TX_RING_SIZE - 1)) {
		ctrl |= TXBUF_WRAP;
		macb->tx_head = 0;
	} else
		macb->tx_head++;

	macb->tx_ring[tx_head].ctrl = ctrl;
	macb->tx_ring[tx_head].addr = paddr;
	barrier();
	macb_writel(macb, NCR, MACB_BIT(TE) | MACB_BIT(RE) | MACB_BIT(TSTART));

	/*
	 * I guess this is necessary because the networking core may
	 * re-use the transmit buffer as soon as we return...
	 */
	for (i = 0; i <= CFG_MACB_TX_TIMEOUT; i++) {
		barrier();
		ctrl = macb->tx_ring[tx_head].ctrl;
		if (ctrl & TXBUF_USED)
			break;
		udelay(1);
	}

	dma_unmap_single(packet, length, paddr);

	if (i <= CFG_MACB_TX_TIMEOUT) {
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
	while (i > new_tail) {
		macb->rx_ring[i].addr &= ~RXADDR_USED;
		i++;
		if (i > CFG_MACB_RX_RING_SIZE)
			i = 0;
	}

	while (i < new_tail) {
		macb->rx_ring[i].addr &= ~RXADDR_USED;
		i++;
	}

	barrier();
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
			if (wrapped) {
				unsigned int headlen, taillen;

				headlen = 128 * (CFG_MACB_RX_RING_SIZE
						 - macb->rx_tail);
				taillen = length - headlen;
				memcpy((void *)NetRxPackets[0],
				       buffer, headlen);
				memcpy((void *)NetRxPackets[0] + headlen,
				       macb->rx_buffer, taillen);
				buffer = (void *)NetRxPackets[0];
			}

			NetReceive(buffer, length);
			if (++rx_tail >= CFG_MACB_RX_RING_SIZE)
				rx_tail = 0;
			reclaim_rx_buffers(macb, rx_tail);
		} else {
			if (++rx_tail >= CFG_MACB_RX_RING_SIZE) {
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

	for (i = 0; i < CFG_MACB_AUTONEG_TIMEOUT / 100; i++) {
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

static int macb_phy_init(struct macb_device *macb)
{
	struct eth_device *netdev = &macb->netdev;
	u32 ncfgr;
	u16 phy_id, status, adv, lpa;
	int media, speed, duplex;
	int i;

	/* Check if the PHY is up to snuff... */
	phy_id = macb_mdio_read(macb, MII_PHYSID1);
	if (phy_id == 0xffff) {
		printf("%s: No PHY present\n", netdev->name);
		return 0;
	}

	status = macb_mdio_read(macb, MII_BMSR);
	if (!(status & BMSR_LSTATUS)) {
		/* Try to re-negotiate if we don't have link already. */
		macb_phy_reset(macb);

		for (i = 0; i < CFG_MACB_AUTONEG_TIMEOUT / 100; i++) {
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
	} else {
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
}

static int macb_init(struct eth_device *netdev, bd_t *bd)
{
	struct macb_device *macb = to_macb(netdev);
	unsigned long paddr;
	u32 hwaddr_bottom;
	u16 hwaddr_top;
	int i;

	/*
	 * macb_halt should have been called at some point before now,
	 * so we'll assume the controller is idle.
	 */

	/* initialize DMA descriptors */
	paddr = macb->rx_buffer_dma;
	for (i = 0; i < CFG_MACB_RX_RING_SIZE; i++) {
		if (i == (CFG_MACB_RX_RING_SIZE - 1))
			paddr |= RXADDR_WRAP;
		macb->rx_ring[i].addr = paddr;
		macb->rx_ring[i].ctrl = 0;
		paddr += 128;
	}
	for (i = 0; i < CFG_MACB_TX_RING_SIZE; i++) {
		macb->tx_ring[i].addr = 0;
		if (i == (CFG_MACB_TX_RING_SIZE - 1))
			macb->tx_ring[i].ctrl = TXBUF_USED | TXBUF_WRAP;
		else
			macb->tx_ring[i].ctrl = TXBUF_USED;
	}
	macb->rx_tail = macb->tx_head = macb->tx_tail = 0;

	macb_writel(macb, RBQP, macb->rx_ring_dma);
	macb_writel(macb, TBQP, macb->tx_ring_dma);

	/* set hardware address */
	hwaddr_bottom = cpu_to_le32(*((u32 *)netdev->enetaddr));
	macb_writel(macb, SA1B, hwaddr_bottom);
	hwaddr_top = cpu_to_le16(*((u16 *)(netdev->enetaddr + 4)));
	macb_writel(macb, SA1T, hwaddr_top);

	/* choose RMII or MII mode. This depends on the board */
#ifdef CONFIG_RMII
#ifdef CONFIG_AT91CAP9ADK
	macb_writel(macb, USRIO, MACB_BIT(RMII) | MACB_BIT(CLKEN));
#else
	macb_writel(macb, USRIO, 0);
#endif
#else
#ifdef CONFIG_AT91CAP9ADK
	macb_writel(macb, USRIO, MACB_BIT(CLKEN));
#else
	macb_writel(macb, USRIO, MACB_BIT(MII));
#endif
#endif /* CONFIG_RMII */

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

int macb_eth_initialize(int id, void *regs, unsigned int phy_addr)
{
	struct macb_device *macb;
	struct eth_device *netdev;
	unsigned long macb_hz;
	u32 ncfgr;

	macb = malloc(sizeof(struct macb_device));
	if (!macb) {
		printf("Error: Failed to allocate memory for MACB%d\n", id);
		return -1;
	}
	memset(macb, 0, sizeof(struct macb_device));

	netdev = &macb->netdev;

	macb->rx_buffer = dma_alloc_coherent(CFG_MACB_RX_BUFFER_SIZE,
					     &macb->rx_buffer_dma);
	macb->rx_ring = dma_alloc_coherent(CFG_MACB_RX_RING_SIZE
					   * sizeof(struct macb_dma_desc),
					   &macb->rx_ring_dma);
	macb->tx_ring = dma_alloc_coherent(CFG_MACB_TX_RING_SIZE
					   * sizeof(struct macb_dma_desc),
					   &macb->tx_ring_dma);

	macb->regs = regs;
	macb->phy_addr = phy_addr;

	sprintf(netdev->name, "macb%d", id);
	netdev->init = macb_init;
	netdev->halt = macb_halt;
	netdev->send = macb_send;
	netdev->recv = macb_recv;

	/*
	 * Do some basic initialization so that we at least can talk
	 * to the PHY
	 */
	macb_hz = get_macb_pclk_rate(id);
	if (macb_hz < 20000000)
		ncfgr = MACB_BF(CLK, MACB_CLK_DIV8);
	else if (macb_hz < 40000000)
		ncfgr = MACB_BF(CLK, MACB_CLK_DIV16);
	else if (macb_hz < 80000000)
		ncfgr = MACB_BF(CLK, MACB_CLK_DIV32);
	else
		ncfgr = MACB_BF(CLK, MACB_CLK_DIV64);

	macb_writel(macb, NCFGR, ncfgr);

	eth_register(netdev);

	return 0;
}

#endif

#if defined(CONFIG_CMD_MII)

int miiphy_read(unsigned char addr, unsigned char reg, unsigned short *value)
{
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;
	int iflag;

	iflag = disable_interrupts();
	netctl = macb_readl(&macb, EMACB_NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(&macb, EMACB_NCR, netctl);
	if (iflag)
		enable_interrupts();

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 2)
		 | MACB_BF(PHYA, addr)
		 | MACB_BF(REGA, reg)
		 | MACB_BF(CODE, 2));
	macb_writel(&macb, EMACB_MAN, frame);

	do {
		netstat = macb_readl(&macb, EMACB_NSR);
	} while (!(netstat & MACB_BIT(IDLE)));

	frame = macb_readl(&macb, EMACB_MAN);
	*value = MACB_BFEXT(DATA, frame);

	iflag = disable_interrupts();
	netctl = macb_readl(&macb, EMACB_NCR);
	netctl &= ~MACB_BIT(MPE);
	macb_writel(&macb, EMACB_NCR, netctl);
	if (iflag)
		enable_interrupts();

	return 0;
}

int miiphy_write(unsigned char addr, unsigned char reg, unsigned short value)
{
	unsigned long netctl;
	unsigned long netstat;
	unsigned long frame;
	int iflag;

	iflag = disable_interrupts();
	netctl = macb_readl(&macb, EMACB_NCR);
	netctl |= MACB_BIT(MPE);
	macb_writel(&macb, EMACB_NCR, netctl);
	if (iflag)
		enable_interrupts();

	frame = (MACB_BF(SOF, 1)
		 | MACB_BF(RW, 1)
		 | MACB_BF(PHYA, addr)
		 | MACB_BF(REGA, reg)
		 | MACB_BF(CODE, 2)
		 | MACB_BF(DATA, value));
	macb_writel(&macb, EMACB_MAN, frame);

	do {
		netstat = macb_readl(&macb, EMACB_NSR);
	} while (!(netstat & MACB_BIT(IDLE)));

	iflag = disable_interrupts();
	netctl = macb_readl(&macb, EMACB_NCR);
	netctl &= ~MACB_BIT(MPE);
	macb_writel(&macb, EMACB_NCR, netctl);
	if (iflag)
		enable_interrupts();

	return 0;
}

#endif

#endif /* CONFIG_MACB */

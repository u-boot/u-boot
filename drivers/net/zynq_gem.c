/*
 * (C) Copyright 2011 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 *
 * Based on Xilinx gmac driver:
 * (C) Copyright 2011 Xilinx
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <net.h>
#include <netdev.h>
#include <config.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <asm/io.h>
#include <phy.h>
#include <miiphy.h>
#include <watchdog.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

#if !defined(CONFIG_PHYLIB)
# error XILINX_GEM_ETHERNET requires PHYLIB
#endif

/* Bit/mask specification */
#define ZYNQ_GEM_PHYMNTNC_OP_MASK	0x40020000 /* operation mask bits */
#define ZYNQ_GEM_PHYMNTNC_OP_R_MASK	0x20000000 /* read operation */
#define ZYNQ_GEM_PHYMNTNC_OP_W_MASK	0x10000000 /* write operation */
#define ZYNQ_GEM_PHYMNTNC_PHYAD_SHIFT_MASK	23 /* Shift bits for PHYAD */
#define ZYNQ_GEM_PHYMNTNC_PHREG_SHIFT_MASK	18 /* Shift bits for PHREG */

#define ZYNQ_GEM_RXBUF_EOF_MASK		0x00008000 /* End of frame. */
#define ZYNQ_GEM_RXBUF_SOF_MASK		0x00004000 /* Start of frame. */
#define ZYNQ_GEM_RXBUF_LEN_MASK		0x00003FFF /* Mask for length field */

#define ZYNQ_GEM_RXBUF_WRAP_MASK	0x00000002 /* Wrap bit, last BD */
#define ZYNQ_GEM_RXBUF_NEW_MASK		0x00000001 /* Used bit.. */
#define ZYNQ_GEM_RXBUF_ADD_MASK		0xFFFFFFFC /* Mask for address */

/* Wrap bit, last descriptor */
#define ZYNQ_GEM_TXBUF_WRAP_MASK	0x40000000
#define ZYNQ_GEM_TXBUF_LAST_MASK	0x00008000 /* Last buffer */

#define ZYNQ_GEM_NWCTRL_TXEN_MASK	0x00000008 /* Enable transmit */
#define ZYNQ_GEM_NWCTRL_RXEN_MASK	0x00000004 /* Enable receive */
#define ZYNQ_GEM_NWCTRL_MDEN_MASK	0x00000010 /* Enable MDIO port */
#define ZYNQ_GEM_NWCTRL_STARTTX_MASK	0x00000200 /* Start tx (tx_go) */

#define ZYNQ_GEM_NWCFG_SPEED100		0x000000001 /* 100 Mbps operation */
#define ZYNQ_GEM_NWCFG_SPEED1000	0x000000400 /* 1Gbps operation */
#define ZYNQ_GEM_NWCFG_FDEN		0x000000002 /* Full Duplex mode */
#define ZYNQ_GEM_NWCFG_FSREM		0x000020000 /* FCS removal */
#define ZYNQ_GEM_NWCFG_MDCCLKDIV	0x000080000 /* Div pclk by 32, 80MHz */
#define ZYNQ_GEM_NWCFG_MDCCLKDIV2	0x0000c0000 /* Div pclk by 48, 120MHz */

#define ZYNQ_GEM_NWCFG_INIT		(ZYNQ_GEM_NWCFG_FDEN | \
					ZYNQ_GEM_NWCFG_FSREM | \
					ZYNQ_GEM_NWCFG_MDCCLKDIV)

#define ZYNQ_GEM_NWSR_MDIOIDLE_MASK	0x00000004 /* PHY management idle */

#define ZYNQ_GEM_DMACR_BLENGTH		0x00000004 /* INCR4 AHB bursts */
/* Use full configured addressable space (8 Kb) */
#define ZYNQ_GEM_DMACR_RXSIZE		0x00000300
/* Use full configured addressable space (4 Kb) */
#define ZYNQ_GEM_DMACR_TXSIZE		0x00000400
/* Set with binary 00011000 to use 1536 byte(1*max length frame/buffer) */
#define ZYNQ_GEM_DMACR_RXBUF		0x00180000

#define ZYNQ_GEM_DMACR_INIT		(ZYNQ_GEM_DMACR_BLENGTH | \
					ZYNQ_GEM_DMACR_RXSIZE | \
					ZYNQ_GEM_DMACR_TXSIZE | \
					ZYNQ_GEM_DMACR_RXBUF)

/* Use MII register 1 (MII status register) to detect PHY */
#define PHY_DETECT_REG  1

/* Mask used to verify certain PHY features (or register contents)
 * in the register above:
 *  0x1000: 10Mbps full duplex support
 *  0x0800: 10Mbps half duplex support
 *  0x0008: Auto-negotiation support
 */
#define PHY_DETECT_MASK 0x1808

/* TX BD status masks */
#define ZYNQ_GEM_TXBUF_FRMLEN_MASK	0x000007ff
#define ZYNQ_GEM_TXBUF_EXHAUSTED	0x08000000
#define ZYNQ_GEM_TXBUF_UNDERRUN		0x10000000

/* Clock frequencies for different speeds */
#define ZYNQ_GEM_FREQUENCY_10	2500000UL
#define ZYNQ_GEM_FREQUENCY_100	25000000UL
#define ZYNQ_GEM_FREQUENCY_1000	125000000UL

/* Device registers */
struct zynq_gem_regs {
	u32 nwctrl; /* Network Control reg */
	u32 nwcfg; /* Network Config reg */
	u32 nwsr; /* Network Status reg */
	u32 reserved1;
	u32 dmacr; /* DMA Control reg */
	u32 txsr; /* TX Status reg */
	u32 rxqbase; /* RX Q Base address reg */
	u32 txqbase; /* TX Q Base address reg */
	u32 rxsr; /* RX Status reg */
	u32 reserved2[2];
	u32 idr; /* Interrupt Disable reg */
	u32 reserved3;
	u32 phymntnc; /* Phy Maintaince reg */
	u32 reserved4[18];
	u32 hashl; /* Hash Low address reg */
	u32 hashh; /* Hash High address reg */
#define LADDR_LOW	0
#define LADDR_HIGH	1
	u32 laddr[4][LADDR_HIGH + 1]; /* Specific1 addr low/high reg */
	u32 match[4]; /* Type ID1 Match reg */
	u32 reserved6[18];
	u32 stat[44]; /* Octects transmitted Low reg - stat start */
};

/* BD descriptors */
struct emac_bd {
	u32 addr; /* Next descriptor pointer */
	u32 status;
};

#define RX_BUF 3
/* Page table entries are set to 1MB, or multiples of 1MB
 * (not < 1MB). driver uses less bd's so use 1MB bdspace.
 */
#define BD_SPACE	0x100000
/* BD separation space */
#define BD_SEPRN_SPACE	64

/* Initialized, rxbd_current, rx_first_buf must be 0 after init */
struct zynq_gem_priv {
	struct emac_bd *tx_bd;
	struct emac_bd *rx_bd;
	char *rxbuffers;
	u32 rxbd_current;
	u32 rx_first_buf;
	int phyaddr;
	u32 emio;
	int init;
	struct phy_device *phydev;
	struct mii_dev *bus;
};

static inline int mdio_wait(struct eth_device *dev)
{
	struct zynq_gem_regs *regs = (struct zynq_gem_regs *)dev->iobase;
	u32 timeout = 200;

	/* Wait till MDIO interface is ready to accept a new transaction. */
	while (--timeout) {
		if (readl(&regs->nwsr) & ZYNQ_GEM_NWSR_MDIOIDLE_MASK)
			break;
		WATCHDOG_RESET();
	}

	if (!timeout) {
		printf("%s: Timeout\n", __func__);
		return 1;
	}

	return 0;
}

static u32 phy_setup_op(struct eth_device *dev, u32 phy_addr, u32 regnum,
							u32 op, u16 *data)
{
	u32 mgtcr;
	struct zynq_gem_regs *regs = (struct zynq_gem_regs *)dev->iobase;

	if (mdio_wait(dev))
		return 1;

	/* Construct mgtcr mask for the operation */
	mgtcr = ZYNQ_GEM_PHYMNTNC_OP_MASK | op |
		(phy_addr << ZYNQ_GEM_PHYMNTNC_PHYAD_SHIFT_MASK) |
		(regnum << ZYNQ_GEM_PHYMNTNC_PHREG_SHIFT_MASK) | *data;

	/* Write mgtcr and wait for completion */
	writel(mgtcr, &regs->phymntnc);

	if (mdio_wait(dev))
		return 1;

	if (op == ZYNQ_GEM_PHYMNTNC_OP_R_MASK)
		*data = readl(&regs->phymntnc);

	return 0;
}

static u32 phyread(struct eth_device *dev, u32 phy_addr, u32 regnum, u16 *val)
{
	return phy_setup_op(dev, phy_addr, regnum,
				ZYNQ_GEM_PHYMNTNC_OP_R_MASK, val);
}

static u32 phywrite(struct eth_device *dev, u32 phy_addr, u32 regnum, u16 data)
{
	return phy_setup_op(dev, phy_addr, regnum,
				ZYNQ_GEM_PHYMNTNC_OP_W_MASK, &data);
}

static void phy_detection(struct eth_device *dev)
{
	int i;
	u16 phyreg;
	struct zynq_gem_priv *priv = dev->priv;

	if (priv->phyaddr != -1) {
		phyread(dev, priv->phyaddr, PHY_DETECT_REG, &phyreg);
		if ((phyreg != 0xFFFF) &&
		    ((phyreg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			debug("Default phy address %d is valid\n",
			      priv->phyaddr);
			return;
		} else {
			debug("PHY address is not setup correctly %d\n",
			      priv->phyaddr);
			priv->phyaddr = -1;
		}
	}

	debug("detecting phy address\n");
	if (priv->phyaddr == -1) {
		/* detect the PHY address */
		for (i = 31; i >= 0; i--) {
			phyread(dev, i, PHY_DETECT_REG, &phyreg);
			if ((phyreg != 0xFFFF) &&
			    ((phyreg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
				/* Found a valid PHY address */
				priv->phyaddr = i;
				debug("Found valid phy address, %d\n", i);
				return;
			}
		}
	}
	printf("PHY is not detected\n");
}

static int zynq_gem_setup_mac(struct eth_device *dev)
{
	u32 i, macaddrlow, macaddrhigh;
	struct zynq_gem_regs *regs = (struct zynq_gem_regs *)dev->iobase;

	/* Set the MAC bits [31:0] in BOT */
	macaddrlow = dev->enetaddr[0];
	macaddrlow |= dev->enetaddr[1] << 8;
	macaddrlow |= dev->enetaddr[2] << 16;
	macaddrlow |= dev->enetaddr[3] << 24;

	/* Set MAC bits [47:32] in TOP */
	macaddrhigh = dev->enetaddr[4];
	macaddrhigh |= dev->enetaddr[5] << 8;

	for (i = 0; i < 4; i++) {
		writel(0, &regs->laddr[i][LADDR_LOW]);
		writel(0, &regs->laddr[i][LADDR_HIGH]);
		/* Do not use MATCHx register */
		writel(0, &regs->match[i]);
	}

	writel(macaddrlow, &regs->laddr[0][LADDR_LOW]);
	writel(macaddrhigh, &regs->laddr[0][LADDR_HIGH]);

	return 0;
}

static int zynq_gem_init(struct eth_device *dev, bd_t * bis)
{
	u32 i;
	unsigned long clk_rate = 0;
	struct phy_device *phydev;
	const u32 stat_size = (sizeof(struct zynq_gem_regs) -
				offsetof(struct zynq_gem_regs, stat)) / 4;
	struct zynq_gem_regs *regs = (struct zynq_gem_regs *)dev->iobase;
	struct zynq_gem_priv *priv = dev->priv;
	const u32 supported = SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full |
			SUPPORTED_1000baseT_Half |
			SUPPORTED_1000baseT_Full;

	if (!priv->init) {
		/* Disable all interrupts */
		writel(0xFFFFFFFF, &regs->idr);

		/* Disable the receiver & transmitter */
		writel(0, &regs->nwctrl);
		writel(0, &regs->txsr);
		writel(0, &regs->rxsr);
		writel(0, &regs->phymntnc);

		/* Clear the Hash registers for the mac address
		 * pointed by AddressPtr
		 */
		writel(0x0, &regs->hashl);
		/* Write bits [63:32] in TOP */
		writel(0x0, &regs->hashh);

		/* Clear all counters */
		for (i = 0; i <= stat_size; i++)
			readl(&regs->stat[i]);

		/* Setup RxBD space */
		memset(priv->rx_bd, 0, RX_BUF * sizeof(struct emac_bd));

		for (i = 0; i < RX_BUF; i++) {
			priv->rx_bd[i].status = 0xF0000000;
			priv->rx_bd[i].addr =
					((u32)(priv->rxbuffers) +
							(i * PKTSIZE_ALIGN));
		}
		/* WRAP bit to last BD */
		priv->rx_bd[--i].addr |= ZYNQ_GEM_RXBUF_WRAP_MASK;
		/* Write RxBDs to IP */
		writel((u32)priv->rx_bd, &regs->rxqbase);

		/* Setup for DMA Configuration register */
		writel(ZYNQ_GEM_DMACR_INIT, &regs->dmacr);

		/* Setup for Network Control register, MDIO, Rx and Tx enable */
		setbits_le32(&regs->nwctrl, ZYNQ_GEM_NWCTRL_MDEN_MASK);

		priv->init++;
	}

	phy_detection(dev);

	/* interface - look at tsec */
	phydev = phy_connect(priv->bus, priv->phyaddr, dev,
			     PHY_INTERFACE_MODE_MII);

	phydev->supported = supported | ADVERTISED_Pause |
			    ADVERTISED_Asym_Pause;
	phydev->advertising = phydev->supported;
	priv->phydev = phydev;
	phy_config(phydev);
	phy_startup(phydev);

	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return -1;
	}

	switch (phydev->speed) {
	case SPEED_1000:
		writel(ZYNQ_GEM_NWCFG_INIT | ZYNQ_GEM_NWCFG_SPEED1000,
		       &regs->nwcfg);
		clk_rate = ZYNQ_GEM_FREQUENCY_1000;
		break;
	case SPEED_100:
		clrsetbits_le32(&regs->nwcfg, ZYNQ_GEM_NWCFG_SPEED1000,
				ZYNQ_GEM_NWCFG_INIT | ZYNQ_GEM_NWCFG_SPEED100);
		clk_rate = ZYNQ_GEM_FREQUENCY_100;
		break;
	case SPEED_10:
		clk_rate = ZYNQ_GEM_FREQUENCY_10;
		break;
	}

	/* Change the rclk and clk only not using EMIO interface */
	if (!priv->emio)
		zynq_slcr_gem_clk_setup(dev->iobase !=
					ZYNQ_GEM_BASEADDR0, clk_rate);

	setbits_le32(&regs->nwctrl, ZYNQ_GEM_NWCTRL_RXEN_MASK |
					ZYNQ_GEM_NWCTRL_TXEN_MASK);

	return 0;
}

static int zynq_gem_send(struct eth_device *dev, void *ptr, int len)
{
	u32 addr, size;
	struct zynq_gem_priv *priv = dev->priv;
	struct zynq_gem_regs *regs = (struct zynq_gem_regs *)dev->iobase;

	/* setup BD */
	writel((u32)priv->tx_bd, &regs->txqbase);

	/* Setup Tx BD */
	memset(priv->tx_bd, 0, sizeof(struct emac_bd));

	priv->tx_bd->addr = (u32)ptr;
	priv->tx_bd->status = (len & ZYNQ_GEM_TXBUF_FRMLEN_MASK) |
				ZYNQ_GEM_TXBUF_LAST_MASK;

	addr = (u32) ptr;
	addr &= ~(ARCH_DMA_MINALIGN - 1);
	size = roundup(len, ARCH_DMA_MINALIGN);
	flush_dcache_range(addr, addr + size);
	barrier();

	/* Start transmit */
	setbits_le32(&regs->nwctrl, ZYNQ_GEM_NWCTRL_STARTTX_MASK);

	/* Read TX BD status */
	if (priv->tx_bd->status & ZYNQ_GEM_TXBUF_UNDERRUN)
		printf("TX underrun\n");
	if (priv->tx_bd->status & ZYNQ_GEM_TXBUF_EXHAUSTED)
		printf("TX buffers exhausted in mid frame\n");

	return 0;
}

/* Do not check frame_recd flag in rx_status register 0x20 - just poll BD */
static int zynq_gem_recv(struct eth_device *dev)
{
	int frame_len;
	struct zynq_gem_priv *priv = dev->priv;
	struct emac_bd *current_bd = &priv->rx_bd[priv->rxbd_current];
	struct emac_bd *first_bd;

	if (!(current_bd->addr & ZYNQ_GEM_RXBUF_NEW_MASK))
		return 0;

	if (!(current_bd->status &
			(ZYNQ_GEM_RXBUF_SOF_MASK | ZYNQ_GEM_RXBUF_EOF_MASK))) {
		printf("GEM: SOF or EOF not set for last buffer received!\n");
		return 0;
	}

	frame_len = current_bd->status & ZYNQ_GEM_RXBUF_LEN_MASK;
	if (frame_len) {
		u32 addr = current_bd->addr & ZYNQ_GEM_RXBUF_ADD_MASK;
		addr &= ~(ARCH_DMA_MINALIGN - 1);
		u32 size = roundup(frame_len, ARCH_DMA_MINALIGN);
		invalidate_dcache_range(addr, addr + size);

		NetReceive((u8 *)addr, frame_len);

		if (current_bd->status & ZYNQ_GEM_RXBUF_SOF_MASK)
			priv->rx_first_buf = priv->rxbd_current;
		else {
			current_bd->addr &= ~ZYNQ_GEM_RXBUF_NEW_MASK;
			current_bd->status = 0xF0000000; /* FIXME */
		}

		if (current_bd->status & ZYNQ_GEM_RXBUF_EOF_MASK) {
			first_bd = &priv->rx_bd[priv->rx_first_buf];
			first_bd->addr &= ~ZYNQ_GEM_RXBUF_NEW_MASK;
			first_bd->status = 0xF0000000;
		}

		if ((++priv->rxbd_current) >= RX_BUF)
			priv->rxbd_current = 0;
	}

	return frame_len;
}

static void zynq_gem_halt(struct eth_device *dev)
{
	struct zynq_gem_regs *regs = (struct zynq_gem_regs *)dev->iobase;

	clrsetbits_le32(&regs->nwctrl, ZYNQ_GEM_NWCTRL_RXEN_MASK |
						ZYNQ_GEM_NWCTRL_TXEN_MASK, 0);
}

static int zynq_gem_miiphyread(const char *devname, uchar addr,
							uchar reg, ushort *val)
{
	struct eth_device *dev = eth_get_dev();
	int ret;

	ret = phyread(dev, addr, reg, val);
	debug("%s 0x%x, 0x%x, 0x%x\n", __func__, addr, reg, *val);
	return ret;
}

static int zynq_gem_miiphy_write(const char *devname, uchar addr,
							uchar reg, ushort val)
{
	struct eth_device *dev = eth_get_dev();

	debug("%s 0x%x, 0x%x, 0x%x\n", __func__, addr, reg, val);
	return phywrite(dev, addr, reg, val);
}

int zynq_gem_initialize(bd_t *bis, int base_addr, int phy_addr, u32 emio)
{
	struct eth_device *dev;
	struct zynq_gem_priv *priv;
	void *bd_space;

	dev = calloc(1, sizeof(*dev));
	if (dev == NULL)
		return -1;

	dev->priv = calloc(1, sizeof(struct zynq_gem_priv));
	if (dev->priv == NULL) {
		free(dev);
		return -1;
	}
	priv = dev->priv;

	/* Align rxbuffers to ARCH_DMA_MINALIGN */
	priv->rxbuffers = memalign(ARCH_DMA_MINALIGN, RX_BUF * PKTSIZE_ALIGN);
	memset(priv->rxbuffers, 0, RX_BUF * PKTSIZE_ALIGN);

	/* Align bd_space to 1MB */
	bd_space = memalign(1 << MMU_SECTION_SHIFT, BD_SPACE);
	mmu_set_region_dcache_behaviour((u32)bd_space, BD_SPACE, DCACHE_OFF);

	/* Initialize the bd spaces for tx and rx bd's */
	priv->tx_bd = (struct emac_bd *)bd_space;
	priv->rx_bd = (struct emac_bd *)((u32)bd_space + BD_SEPRN_SPACE);

	priv->phyaddr = phy_addr;
	priv->emio = emio;

	sprintf(dev->name, "Gem.%x", base_addr);

	dev->iobase = base_addr;

	dev->init = zynq_gem_init;
	dev->halt = zynq_gem_halt;
	dev->send = zynq_gem_send;
	dev->recv = zynq_gem_recv;
	dev->write_hwaddr = zynq_gem_setup_mac;

	eth_register(dev);

	miiphy_register(dev->name, zynq_gem_miiphyread, zynq_gem_miiphy_write);
	priv->bus = miiphy_get_dev_by_name(dev->name);

	return 1;
}

#ifdef CONFIG_OF_CONTROL
int zynq_gem_of_init(const void *blob)
{
	int offset = 0;
	u32 ret = 0;
	u32 reg, phy_reg;

	debug("ZYNQ GEM: Initialization\n");

	do {
		offset = fdt_node_offset_by_compatible(blob, offset,
					"xlnx,ps7-ethernet-1.00.a");
		if (offset != -1) {
			reg = fdtdec_get_addr(blob, offset, "reg");
			if (reg != FDT_ADDR_T_NONE) {
				offset = fdtdec_lookup_phandle(blob, offset,
							       "phy-handle");
				if (offset != -1)
					phy_reg = fdtdec_get_addr(blob, offset,
								  "reg");
				else
					phy_reg = 0;

				debug("ZYNQ GEM: addr %x, phyaddr %x\n",
				      reg, phy_reg);

				ret |= zynq_gem_initialize(NULL, reg,
							   phy_reg, 0);

			} else {
				debug("ZYNQ GEM: Can't get base address\n");
				return -1;
			}
		}
	} while (offset != -1);

	return ret;
}
#endif

/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <phy.h>
#include <miiphy.h>

#undef ETH_HALTING

#define XTE_EMMC_LINKSPEED_MASK	0xC0000000 /* Link speed */
/* XTE_EMCFG_LINKSPD_MASK */
#define XTE_EMMC_LINKSPD_10	0x00000000 /* for 10 Mbit */
#define XTE_EMMC_LINKSPD_100	0x40000000 /* for 100 Mbit */
#define XTE_EMMC_LINKSPD_1000	0x80000000 /* forr 1000 Mbit */

#define XTE_RSE_MIIM_RR_MASK	0x0002
#define XTE_RSE_MIIM_WR_MASK	0x0004
#define XTE_RSE_CFG_RR_MASK	0x0020
#define XTE_RSE_CFG_WR_MASK	0x0040

/* XPS_LL_TEMAC indirect registers offset definition */
#define RCW1	0x240
#define TC	0x280
#define EMMC	0x300
#define MC	0x340
#define UAW0	0x380
#define UAW1	0x384
#define AFM	0x390
#define MIIMWD	0x3b0
#define MIIMAI	0x3b4

#define CNTLREG_WRITE_ENABLE_MASK	0x8000

#define MDIO_ENABLE_MASK	0x40
#define MDIO_CLOCK_DIV_100MHz	0x28

/* XPS_LL_TEMAC SDMA registers definition */
#define TX_CURDESC_PTR		0x03
#define TX_TAILDESC_PTR		0x04
#define TX_CHNL_CTRL		0x05
#define TX_IRQ_REG		0x06
#define TX_CHNL_STS		0x07
#define RX_NXTDESC_PTR		0x08
#define RX_CURDESC_PTR		0x0b
#define RX_TAILDESC_PTR		0x0c
#define RX_CHNL_CTRL		0x0d
#define RX_IRQ_REG		0x0e
#define RX_CHNL_STS		0x0f
#define DMA_CONTROL_REG		0x10

/* DMA control bit */
#define DMA_CONTROL_RESET	0x1

/* CDMAC descriptor status bit definitions */
# define BDSTAT_STOP_ON_END_MASK	0x20
# define BDSTAT_COMPLETED_MASK		0x10
# define BDSTAT_SOP_MASK		0x08
# define BDSTAT_EOP_MASK		0x04

# define CHNL_STS_ERROR_MASK		0x80

/* All interrupt enable bits */
#define XLLDMA_CR_IRQ_ALL_EN_MASK	0x00000087
/* All interrupt bits */
#define XLLDMA_IRQ_ALL_MASK		0x0000001F
/* Disable error when 2 or 4 bit coalesce counter overflows */
#define XLLDMA_DMACR_RX_OVERFLOW_ERR_DIS_MASK	0x00000010
/* Disable error when 2 or 4 bit coalesce counter overflows */
#define XLLDMA_DMACR_TX_OVERFLOW_ERR_DIS_MASK	0x00000008
/* Enable use of tail pointer register */
#define XLLDMA_DMACR_TAIL_PTR_EN_MASK	0x00000004

#define LL_FIFO_ISR_RC_COMPLETE	0x04000000

#define SDMA_BIT	1
#define DCR_BIT		2

#define DMAALIGN	32

/* SDMA Buffer Descriptor */
struct cdmac_bd_t {
	struct cdmac_bd_t *next_p;
	unsigned char *phys_buf_p;
	unsigned long buf_len;
	unsigned char stat;
	unsigned char app1_1;
	unsigned short app1_2;
	unsigned long app2;
	unsigned long app3;
	unsigned long app4;
	unsigned long app5;
};

static struct cdmac_bd_t tx_bd __attribute((aligned(DMAALIGN)));
static struct cdmac_bd_t rx_bd __attribute((aligned(DMAALIGN)));

struct ll_fifo_s {
	u32 isr; /* Interrupt Status Register 0x0 */
	u32 ier; /* Interrupt Enable Register 0x4 */
	u32 tdfr; /* Transmit data FIFO reset 0x8 */
	u32 tdfv; /* Transmit data FIFO Vacancy 0xC */
	u32 tdfd; /* Transmit data FIFO 32bit wide data write port 0x10 */
	u32 tlf; /* Write Transmit Length FIFO 0x14 */
	u32 rdfr; /* Read Receive data FIFO reset 0x18 */
	u32 rdfo; /* Receive data FIFO Occupancy 0x1C */
	u32 rdfd; /* Read Receive data FIFO 32bit wide data read port 0x20 */
	u32 rlf; /* Read Receive Length FIFO 0x24 */
	u32 llr; /* Read LocalLink reset 0x28 */
};

static u8 tx_buffer[PKTSIZE_ALIGN] __attribute((aligned(DMAALIGN)));
static u8 rx_buffer[PKTSIZE_ALIGN] __attribute((aligned(DMAALIGN)));

struct temac_reg {
	u32 reserved[8];
	u32 msw; /* Hard TEMAC MSW Data Register */
	u32 lsw; /* Hard TEMAC LSW Data Register */
	u32 ctl; /* Hard TEMAC Control Register */
	u32 rdy; /* Hard TEMAC Ready Status */
};

struct ll_priv {
	u32 ctrl;
	u32 mode;
	int phyaddr;

	struct phy_device *phydev;
	struct mii_dev *bus;
};

#define XILINX_INDIRECT_DCR_ADDRESS_REG	0
#define XILINX_INDIRECT_DCR_ACCESS_REG	1

static void mtdcr_local(u32 reg, u32 val)
{
#if defined(CONFIG_XILINX_440)
	mtdcr(XILINX_INDIRECT_DCR_ADDRESS_REG, reg);
	mtdcr(XILINX_INDIRECT_DCR_ACCESS_REG, val);
#endif
}

static u32 mfdcr_local(u32 reg)
{
	u32 val = 0;
#if defined(CONFIG_XILINX_440)
	mtdcr(XILINX_INDIRECT_DCR_ADDRESS_REG, reg);
	val = mfdcr(XILINX_INDIRECT_DCR_ACCESS_REG);
#endif
	return val;
}

static void sdma_out_be32(struct ll_priv *priv, u32 offset, u32 val)
{
	if (priv->mode & DCR_BIT)
		mtdcr_local(priv->ctrl + offset, val);
	else
		out_be32((u32 *)(priv->ctrl + offset * 4), val);
}

static u32 sdma_in_be32(struct ll_priv *priv, u32 offset)
{
	if (priv->mode & DCR_BIT)
		return mfdcr_local(priv->ctrl + offset);

	return in_be32((u32 *)(priv->ctrl + offset * 4));
}

static void xps_ll_temac_check_status(struct temac_reg *regs, int mask)
{
	u32 timeout = 2000;

	while (timeout && (!(in_be32(&regs->rdy) & mask))) {
		timeout--;
		udelay(1);
	}

	if (!timeout)
		printf("%s: Timeout\n", __func__);
}

/* undirect hostif write to ll_temac */
static void xps_ll_temac_hostif_set(struct eth_device *dev, int emac,
			int phy_addr, int reg_addr, int phy_data)
{
	struct temac_reg *regs = (struct temac_reg *)dev->iobase;

	out_be32(&regs->lsw, phy_data);
	out_be32(&regs->ctl, CNTLREG_WRITE_ENABLE_MASK | MIIMWD);
	out_be32(&regs->lsw, (phy_addr << 5) | reg_addr);
	out_be32(&regs->ctl, CNTLREG_WRITE_ENABLE_MASK | MIIMAI | (emac << 10));
	xps_ll_temac_check_status(regs, XTE_RSE_MIIM_WR_MASK);
}

/* undirect hostif read from ll_temac */
static unsigned int xps_ll_temac_hostif_get(struct eth_device *dev,
			int emac, int phy_addr, int reg_addr)
{
	struct temac_reg *regs = (struct temac_reg *)dev->iobase;

	out_be32(&regs->lsw, (phy_addr << 5) | reg_addr);
	out_be32(&regs->ctl, MIIMAI | (emac << 10));
	xps_ll_temac_check_status(regs, XTE_RSE_MIIM_RR_MASK);
	return in_be32(&regs->lsw);
}

/* undirect write to ll_temac */
static void xps_ll_temac_indirect_set(struct eth_device *dev,
				int emac, int reg_offset, int reg_data)
{
	struct temac_reg *regs = (struct temac_reg *)dev->iobase;

	out_be32(&regs->lsw, reg_data);
	out_be32(&regs->ctl,
			CNTLREG_WRITE_ENABLE_MASK | (emac << 10) | reg_offset);
	xps_ll_temac_check_status(regs, XTE_RSE_CFG_WR_MASK);
}

/* undirect read from ll_temac */
static int xps_ll_temac_indirect_get(struct eth_device *dev,
			int emac, int reg_offset)
{
	struct temac_reg *regs = (struct temac_reg *)dev->iobase;

	out_be32(&regs->ctl, (emac << 10) | reg_offset);
	xps_ll_temac_check_status(regs, XTE_RSE_CFG_RR_MASK);
	return in_be32(&regs->lsw);
}

#ifdef DEBUG
/* read from phy */
static void read_phy_reg(struct eth_device *dev, int phy_addr)
{
	int j, result;

	debug("phy%d ", phy_addr);
	for (j = 0; j < 32; j++) {
		result = xps_ll_temac_hostif_get(dev, 0, phy_addr, j);
		debug("%d: 0x%x ", j, result);
	}
	debug("\n");
}
#endif

static void phy_detection(struct eth_device *dev)
{

	int i;
	struct ll_priv *priv = dev->priv;
	unsigned int phyreg = 0;


	if (priv->phyaddr != -1 ) {
		phyreg = xps_ll_temac_hostif_get(dev, 0, priv->phyaddr, 1);
		if ((phyreg & 0x0ffff) != 0x0ffff) {
			/* Found a valid PHY address */
			debug("Default phy address %d is valid\n", priv->phyaddr);
		} else {
			debug("PHY address is not setup correctly %d\n", priv->phyaddr);
			priv->phyaddr = -1;
		}
	}

	/* try out if have ever found the right phy? */
	if (priv->phyaddr == -1) {
		for (i = 31; i >= 0; i--) {
			phyreg = xps_ll_temac_hostif_get(dev, 0, i, 1);
			if ((phyreg & 0x0ffff) != 0x0ffff) {
				debug("phy %x result %x\n", i, phyreg);
				priv->phyaddr = i;
				break;
			}
		}
	}
}

/* setting ll_temac and phy to proper setting */
static int xps_ll_temac_phy_ctrl(struct eth_device *dev)
{
#ifdef CONFIG_PHYLIB
	unsigned int temp, speed;
	struct ll_priv *priv = dev->priv;
	struct phy_device *phydev;

	u32 supported = SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full |
			SUPPORTED_1000baseT_Half |
			SUPPORTED_1000baseT_Full;

	phy_detection(dev);

	/* interface - look at tsec */
	phydev = phy_connect(priv->bus, priv->phyaddr, dev, 0);

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;
	priv->phydev = phydev;
	phy_config(phydev);
	phy_startup(phydev);

	switch (phydev->speed) {
	case 1000:
		speed = XTE_EMMC_LINKSPD_1000;
		break;
	case 100:
		speed = XTE_EMMC_LINKSPD_100;
		break;
	case 10:
		speed = XTE_EMMC_LINKSPD_10;
		break;
	default:
		return 0;
	}

	temp = xps_ll_temac_indirect_get(dev, 0, EMMC);
	temp &= ~XTE_EMMC_LINKSPEED_MASK;
	temp |= speed;
	xps_ll_temac_indirect_set(dev, 0, EMMC, temp);

	return 1;

#else
	int i;
	unsigned int result;
	struct ll_priv *priv = dev->priv;
	unsigned retries = 10;
	unsigned int phyreg = 0;

	phy_detection(dev);

#ifdef DEBUG
	read_phy_reg(dev, priv->phyaddr);
#endif

	/* wait for link up */
	puts("Waiting for link ... ");
	retries = 20000;
	while (retries-- &&
		((xps_ll_temac_hostif_get(dev, 0, priv->phyaddr, 1)
							& 0x04) != 0x04)) {
			udelay(100);
	}

	phyreg = xps_ll_temac_indirect_get(dev, 0, EMMC) &
						(~XTE_EMMC_LINKSPEED_MASK);

	/* get PHY id */
	i = (xps_ll_temac_hostif_get(dev, 0, priv->phyaddr, 2) << 16) |
		xps_ll_temac_hostif_get(dev, 0, priv->phyaddr, 3);
	debug("LL_TEMAC: Phy ID 0x%x\n", i);

	/* FIXME this part will be replaced by PHY lib */
	/* s3e boards */
	if (i == 0x7c0a3) {
		/* 100BASE-T/FD */
		xps_ll_temac_indirect_set(dev, 0, EMMC,
					(phyreg | XTE_EMMC_LINKSPD_100));
		return 1;
	}

#if 0
	/* Support for Xilinx 1000BASE-X PCS/PMA core */
	if (i == 0x0) {
		/* 1000BASE-X/FD */
		xps_ll_temac_indirect_set(dev, 0, EMMC, 0x80000000);
		/* Clear the Isolate bit from PHY control register */
		xps_ll_temac_hostif_set(dev, 0, phy_addr, 0, 0x1140);
		link = 1;
		return 1;
	}
#endif

	result = xps_ll_temac_hostif_get(dev, 0, priv->phyaddr, 5);
	if ((result & 0x8000) == 0x8000) {
		xps_ll_temac_indirect_set(dev, 0, EMMC,
					(phyreg | XTE_EMMC_LINKSPD_1000));
		printf("1000BASE-T/FD\n");
	} else if ((result & 0x4000) == 0x4000) {
		xps_ll_temac_indirect_set(dev, 0, EMMC,
					(phyreg | XTE_EMMC_LINKSPD_100));
		printf("100BASE-T/FD\n");
	} else {
		/* unsupported mode or auto-negotiation failed */
		puts("Unsupported mode or auto-negotiation failed\n");
	}

	return 1;
#endif
}

static inline int xps_ll_temac_dma_error(struct eth_device *dev)
{
	int err;
	struct ll_priv *priv = dev->priv;

	/* Check for TX and RX channel errrors.  */
	err = sdma_in_be32(priv, TX_CHNL_STS) & CHNL_STS_ERROR_MASK;
	err |= sdma_in_be32(priv, RX_CHNL_STS) & CHNL_STS_ERROR_MASK;
	return err;
}

static void xps_ll_temac_reset_dma(struct eth_device *dev)
{
	u32 r;
	struct ll_priv *priv = dev->priv;

	/* Soft reset the DMA.  */
	sdma_out_be32(priv, DMA_CONTROL_REG, DMA_CONTROL_RESET);
	while (sdma_in_be32(priv, DMA_CONTROL_REG) & DMA_CONTROL_RESET)
		;

	/* Now clear the interrupts.  */
	r = sdma_in_be32(priv, TX_CHNL_CTRL);
	r &= ~XLLDMA_CR_IRQ_ALL_EN_MASK;
	sdma_out_be32(priv, TX_CHNL_CTRL, r);

	r = sdma_in_be32(priv, RX_CHNL_CTRL);
	r &= ~XLLDMA_CR_IRQ_ALL_EN_MASK;
	sdma_out_be32(priv, RX_CHNL_CTRL, r);

	/* Now ACK pending IRQs.  */
	sdma_out_be32(priv, TX_IRQ_REG, XLLDMA_IRQ_ALL_MASK);
	sdma_out_be32(priv, RX_IRQ_REG, XLLDMA_IRQ_ALL_MASK);

	/* Set tail-ptr mode, disable errors for both channels.  */
	sdma_out_be32(priv, DMA_CONTROL_REG,
			XLLDMA_DMACR_TAIL_PTR_EN_MASK |
			XLLDMA_DMACR_RX_OVERFLOW_ERR_DIS_MASK |
			XLLDMA_DMACR_TX_OVERFLOW_ERR_DIS_MASK);
}

/* bd init */
static void xps_ll_temac_bd_init(struct eth_device *dev)
{
	struct ll_priv *priv = dev->priv;

	memset(&tx_bd, 0, sizeof(tx_bd));
	memset(&rx_bd, 0, sizeof(rx_bd));

	rx_bd.phys_buf_p = rx_buffer;
	rx_bd.next_p = &rx_bd;
	rx_bd.buf_len = PKTSIZE_ALIGN;
	flush_cache((u32)&rx_bd, sizeof(tx_bd));
	flush_cache((u32)rx_bd.phys_buf_p, PKTSIZE_ALIGN);

	sdma_out_be32(priv, RX_CURDESC_PTR, (u32)&rx_bd);
	sdma_out_be32(priv, RX_TAILDESC_PTR, (u32)&rx_bd);
	sdma_out_be32(priv, RX_NXTDESC_PTR, (u32)&rx_bd); /* setup first fd */

	tx_bd.phys_buf_p = tx_buffer;
	tx_bd.next_p = &tx_bd;

	flush_cache((u32)&tx_bd, sizeof(tx_bd));
	sdma_out_be32(priv, TX_CURDESC_PTR, (u32)&tx_bd);
}

static int ll_temac_send_sdma(struct eth_device *dev,
				void *buffer, int length)
{
	struct ll_priv *priv = dev->priv;

	if (xps_ll_temac_dma_error(dev)) {
		xps_ll_temac_reset_dma(dev);
		xps_ll_temac_bd_init(dev);
	}

	memcpy(tx_buffer, (void *)buffer, length);
	flush_cache((u32)tx_buffer, length);

	tx_bd.stat = BDSTAT_SOP_MASK | BDSTAT_EOP_MASK |
			BDSTAT_STOP_ON_END_MASK;
	tx_bd.buf_len = length;
	flush_cache((u32)&tx_bd, sizeof(tx_bd));

	sdma_out_be32(priv, TX_CURDESC_PTR, (u32)&tx_bd);
	sdma_out_be32(priv, TX_TAILDESC_PTR, (u32)&tx_bd); /* DMA start */

	do {
		flush_cache((u32)&tx_bd, sizeof(tx_bd));
	} while (!(tx_bd.stat & BDSTAT_COMPLETED_MASK));

	return 0;
}

static int ll_temac_recv_sdma(struct eth_device *dev)
{
	int length;
	struct ll_priv *priv = dev->priv;

	if (xps_ll_temac_dma_error(dev)) {
		xps_ll_temac_reset_dma(dev);
		xps_ll_temac_bd_init(dev);
	}

	flush_cache((u32)&rx_bd, sizeof(rx_bd));

	if (!(rx_bd.stat & BDSTAT_COMPLETED_MASK))
		return 0;

	/*
	 * Read out the packet info and start the DMA
	 * onto the second buffer to enable the ethernet rx
	 * path to run in parallel with sw processing
	 * packets.
	 */
	length = rx_bd.app5 & 0x3FFF; /* max length mask */
	if (length > 0)
		NetReceive(rx_bd.phys_buf_p, length);

	/* flip the buffer and re-enable the DMA.  */
	flush_cache((u32)rx_bd.phys_buf_p, length);

	rx_bd.buf_len = PKTSIZE_ALIGN;
	rx_bd.stat = 0;
	rx_bd.app5 = 0;

	flush_cache((u32)&rx_bd, sizeof(rx_bd));
	sdma_out_be32(priv, RX_TAILDESC_PTR, (u32)&rx_bd);

	return length;
}

#ifdef DEBUG
static void debugll(struct eth_device *dev, int count)
{
	struct ll_priv *priv = dev->priv;
	struct ll_fifo_s *ll_fifo = (void *)priv->ctrl;

	printf("%d fifo isr 0x%08x, fifo_ier 0x%08x, fifo_rdfr 0x%08x, "
		"fifo_rdfo 0x%08x fifo_rlr 0x%08x\n", count,
		in_be32(&ll_fifo->isr), in_be32(&ll_fifo->ier),
		in_be32(&ll_fifo->rdfr), in_be32(&ll_fifo->rdfo),
		in_be32(&ll_fifo->rlf));
}
#endif

static int ll_temac_send_fifo(struct eth_device *dev,
					void *buffer, int length)
{
	struct ll_priv *priv = dev->priv;
	struct ll_fifo_s *ll_fifo = (void *)priv->ctrl;
	u32 *buf = (u32 *)buffer;
	u32 i;

	for (i = 0; i < length; i += 4)
		out_be32(&ll_fifo->tdfd, *buf++);

	out_be32(&ll_fifo->tlf, length);
	return 0;
}

static int ll_temac_recv_fifo(struct eth_device *dev)
{
	struct ll_priv *priv = dev->priv;
	struct ll_fifo_s *ll_fifo = (void *)priv->ctrl;
	u32 i, len = 0;
	u32 *buf = (u32 *)&rx_buffer;

	if (in_be32(&ll_fifo->isr) & LL_FIFO_ISR_RC_COMPLETE) {
		out_be32(&ll_fifo->isr, 0xffffffff); /* reset isr */

		len = in_be32(&ll_fifo->rlf) & 0x7FF;

		for (i = 0; i < len; i += 4)
			*buf++ = in_be32(&ll_fifo->rdfd);

#ifdef DEBUG
		debugll(dev, 1);
#endif
		NetReceive((uchar *)&rx_buffer, len);
	}
	return len;
}

/* setup mac addr */
static int ll_temac_addr_setup(struct eth_device *dev)
{
	int val;

	/* set up unicast MAC address filter */
	val = ((dev->enetaddr[3] << 24) | (dev->enetaddr[2] << 16) |
		(dev->enetaddr[1] << 8) | (dev->enetaddr[0]));
	xps_ll_temac_indirect_set(dev, 0, UAW0, val);
	val = (dev->enetaddr[5] << 8) | dev->enetaddr[4] ;
	xps_ll_temac_indirect_set(dev, 0, UAW1, val);

	return 0;
}

static int xps_ll_temac_init(struct eth_device *dev, bd_t *bis)
{
	struct ll_priv *priv = dev->priv;
	struct ll_fifo_s *ll_fifo = (void *)priv->ctrl;

	if (priv->mode & SDMA_BIT) {
		xps_ll_temac_reset_dma(dev);
		xps_ll_temac_bd_init(dev);
	} else {
		out_be32(&ll_fifo->tdfr, 0x000000a5); /* Fifo reset key */
		out_be32(&ll_fifo->rdfr, 0x000000a5); /* Fifo reset key */
		out_be32(&ll_fifo->isr, 0xFFFFFFFF); /* Reset status register */
		out_be32(&ll_fifo->ier, 0); /* Disable all IRQs */
	}

	xps_ll_temac_indirect_set(dev, 0, MC,
				MDIO_ENABLE_MASK | MDIO_CLOCK_DIV_100MHz);

	/* Promiscuous mode disable */
	xps_ll_temac_indirect_set(dev, 0, AFM, 0);
	/* Enable Receiver - RX bit */
	xps_ll_temac_indirect_set(dev, 0, RCW1, 0x10000000);
	/* Enable Transmitter - TX bit */
	xps_ll_temac_indirect_set(dev, 0, TC, 0x10000000);
	return 0;
}

/* halt device */
static void ll_temac_halt(struct eth_device *dev)
{
#ifdef ETH_HALTING
	struct ll_priv *priv = dev->priv;

	/* Disable Receiver */
	xps_ll_temac_indirect_set(dev, 0, RCW1, 0);
	/* Disable Transmitter */
	xps_ll_temac_indirect_set(dev, 0, TC, 0);

	if (priv->mode & SDMA_BIT) {
		sdma_out_be32(priv->ctrl, DMA_CONTROL_REG, DMA_CONTROL_RESET);
		while (sdma_in_be32(priv->ctrl, DMA_CONTROL_REG)
							& DMA_CONTROL_RESET)
			;
	}
#endif
}

static int ll_temac_init(struct eth_device *dev, bd_t *bis)
{
#if DEBUG
	int i;
#endif
	xps_ll_temac_init(dev, bis);

	printf("%s: Xilinx XPS LocalLink Tri-Mode Ether MAC #%d at 0x%08X.\n",
		dev->name, 0, dev->iobase);

#if DEBUG
	for (i = 0; i < 32; i++)
		read_phy_reg(dev, i);
#endif

	if (!xps_ll_temac_phy_ctrl(dev)) {
		ll_temac_halt(dev);
		return -1;
	}

	return 0;
}

static int ll_temac_miiphy_read(const char *devname, uchar addr,
							uchar reg, ushort *val)
{
	struct eth_device *dev = eth_get_dev();

	*val = xps_ll_temac_hostif_get(dev, 0, addr, reg); /* emac = 0 */

	debug("%s 0x%x, 0x%x, 0x%x\n", __func__, addr, reg, *val);
	return 0;
}

static int ll_temac_miiphy_write(const char *devname, uchar addr,
							uchar reg, ushort val)
{
	struct eth_device *dev = eth_get_dev();

	debug("%s 0x%x, 0x%x, 0x%x\n", __func__, addr, reg, val);

	xps_ll_temac_hostif_set(dev, 0, addr, reg, val);

	return 0;
}

static int ll_temac_bus_reset(struct mii_dev *bus)
{
	debug("Just bus reset\n");
	return 0;
}

/* mode bits: 0bit - fifo(0)/sdma(1):SDMA_BIT, 1bit - no dcr(0)/dcr(1):DCR_BIT
 * ctrl - control address for file/sdma */
int xilinx_ll_temac_initialize(bd_t *bis, unsigned long base_addr,
						int mode, unsigned long ctrl)
{
	struct eth_device *dev;
	struct ll_priv *priv;

	dev = calloc(1, sizeof(*dev));
	if (dev == NULL)
		return -1;

	dev->priv = calloc(1, sizeof(struct ll_priv));
	if (dev->priv == NULL) {
		free(dev);
		return -1;
	}

	priv = dev->priv;

	sprintf(dev->name, "Xlltem.%lx", base_addr);

	dev->iobase = base_addr;
	priv->ctrl = ctrl;
	priv->mode = mode;

#ifdef CONFIG_PHY_ADDR
	priv->phyaddr = CONFIG_PHY_ADDR;
#else
	priv->phyaddr = -1;
#endif

	dev->init = ll_temac_init;
	dev->halt = ll_temac_halt;
	dev->write_hwaddr = ll_temac_addr_setup;

	if (priv->mode & SDMA_BIT) {
		dev->send = ll_temac_send_sdma;
		dev->recv = ll_temac_recv_sdma;
	} else {
		dev->send = ll_temac_send_fifo;
		dev->recv = ll_temac_recv_fifo;
	}

	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	miiphy_register(dev->name, ll_temac_miiphy_read, ll_temac_miiphy_write);
	priv->bus = miiphy_get_dev_by_name(dev->name);
	priv->bus->reset = ll_temac_bus_reset;
#endif
	return 1;
}

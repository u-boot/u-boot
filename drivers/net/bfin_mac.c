/*
 * Driver for Blackfin On-Chip MAC device
 *
 * Copyright (c) 2005-2008 Analog Device, Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <net.h>
#include <netdev.h>
#include <command.h>
#include <malloc.h>
#include <miiphy.h>
#include <linux/mii.h>

#include <asm/blackfin.h>
#include <asm/clock.h>
#include <asm/portmux.h>
#include <asm/mach-common/bits/dma.h>
#include <asm/mach-common/bits/emac.h>
#include <asm/mach-common/bits/pll.h>

#include "bfin_mac.h"

#ifndef CONFIG_PHY_ADDR
# define CONFIG_PHY_ADDR 1
#endif
#ifndef CONFIG_PHY_CLOCK_FREQ
# define CONFIG_PHY_CLOCK_FREQ 2500000
#endif

#ifdef CONFIG_POST
#include <post.h>
#endif

#define RXBUF_BASE_ADDR		0xFF900000
#define TXBUF_BASE_ADDR		0xFF800000
#define TX_BUF_CNT		1

#define TOUT_LOOP		1000000

static ADI_ETHER_BUFFER *txbuf[TX_BUF_CNT];
static ADI_ETHER_BUFFER *rxbuf[PKTBUFSRX];
static u16 txIdx;		/* index of the current RX buffer */
static u16 rxIdx;		/* index of the current TX buffer */

/* DMAx_CONFIG values at DMA Restart */
static const union {
	u16 data;
	ADI_DMA_CONFIG_REG reg;
} txdmacfg = {
	.reg = {
		.b_DMA_EN  = 1,	/* enabled */
		.b_WNR     = 0,	/* read from memory */
		.b_WDSIZE  = 2,	/* wordsize is 32 bits */
		.b_DMA2D   = 0,
		.b_RESTART = 0,
		.b_DI_SEL  = 0,
		.b_DI_EN   = 0,	/* no interrupt */
		.b_NDSIZE  = 5,	/* 5 half words is desc size */
		.b_FLOW    = 7	/* large desc flow */
	},
};

static int bfin_miiphy_wait(void)
{
	/* poll the STABUSY bit */
	while (bfin_read_EMAC_STAADD() & STABUSY)
		continue;
	return 0;
}

static int bfin_miiphy_read(const char *devname, uchar addr, uchar reg, ushort *val)
{
	if (bfin_miiphy_wait())
		return 1;
	bfin_write_EMAC_STAADD(SET_PHYAD(addr) | SET_REGAD(reg) | STABUSY);
	if (bfin_miiphy_wait())
		return 1;
	*val = bfin_read_EMAC_STADAT();
	return 0;
}

static int bfin_miiphy_write(const char *devname, uchar addr, uchar reg, ushort val)
{
	if (bfin_miiphy_wait())
		return 1;
	bfin_write_EMAC_STADAT(val);
	bfin_write_EMAC_STAADD(SET_PHYAD(addr) | SET_REGAD(reg) | STAOP | STABUSY);
	return 0;
}

int bfin_EMAC_initialize(bd_t *bis)
{
	struct eth_device *dev;
	dev = malloc(sizeof(*dev));
	if (dev == NULL)
		hang();

	memset(dev, 0, sizeof(*dev));
	strcpy(dev->name, "bfin_mac");

	dev->iobase = 0;
	dev->priv = 0;
	dev->init = bfin_EMAC_init;
	dev->halt = bfin_EMAC_halt;
	dev->send = bfin_EMAC_send;
	dev->recv = bfin_EMAC_recv;
	dev->write_hwaddr = bfin_EMAC_setup_addr;

	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	miiphy_register(dev->name, bfin_miiphy_read, bfin_miiphy_write);
#endif

	return 0;
}

static int bfin_EMAC_send(struct eth_device *dev, void *packet, int length)
{
	int i;
	int result = 0;

	if (length <= 0) {
		printf("Ethernet: bad packet size: %d\n", length);
		goto out;
	}

	if (bfin_read_DMA2_IRQ_STATUS() & DMA_ERR) {
		printf("Ethernet: tx DMA error\n");
		goto out;
	}

	for (i = 0; (bfin_read_DMA2_IRQ_STATUS() & DMA_RUN); ++i) {
		if (i > TOUT_LOOP) {
			puts("Ethernet: tx time out\n");
			goto out;
		}
	}
	txbuf[txIdx]->FrmData->NoBytes = length;
	memcpy(txbuf[txIdx]->FrmData->Dest, (void *)packet, length);
	txbuf[txIdx]->Dma[0].START_ADDR = (u32) txbuf[txIdx]->FrmData;
	bfin_write_DMA2_NEXT_DESC_PTR(txbuf[txIdx]->Dma);
	bfin_write_DMA2_CONFIG(txdmacfg.data);
	bfin_write_EMAC_OPMODE(bfin_read_EMAC_OPMODE() | TE);

	for (i = 0; (txbuf[txIdx]->StatusWord & TX_COMP) == 0; i++) {
		if (i > TOUT_LOOP) {
			puts("Ethernet: tx error\n");
			goto out;
		}
	}
	result = txbuf[txIdx]->StatusWord;
	txbuf[txIdx]->StatusWord = 0;
	if ((txIdx + 1) >= TX_BUF_CNT)
		txIdx = 0;
	else
		txIdx++;
 out:
	debug("BFIN EMAC send: length = %d\n", length);
	return result;
}

static int bfin_EMAC_recv(struct eth_device *dev)
{
	int length = 0;

	for (;;) {
		if ((rxbuf[rxIdx]->StatusWord & RX_COMP) == 0) {
			length = -1;
			break;
		}
		if ((rxbuf[rxIdx]->StatusWord & RX_DMAO) != 0) {
			printf("Ethernet: rx dma overrun\n");
			break;
		}
		if ((rxbuf[rxIdx]->StatusWord & RX_OK) == 0) {
			printf("Ethernet: rx error\n");
			break;
		}
		length = rxbuf[rxIdx]->StatusWord & 0x000007FF;
		if (length <= 4) {
			printf("Ethernet: bad frame\n");
			break;
		}

		debug("%s: len = %d\n", __func__, length - 4);

		NetRxPackets[rxIdx] = rxbuf[rxIdx]->FrmData->Dest;
		NetReceive(NetRxPackets[rxIdx], length - 4);
		bfin_write_DMA1_IRQ_STATUS(DMA_DONE | DMA_ERR);
		rxbuf[rxIdx]->StatusWord = 0x00000000;
		if ((rxIdx + 1) >= PKTBUFSRX)
			rxIdx = 0;
		else
			rxIdx++;
	}

	return length;
}

/**************************************************************
 *
 * Ethernet Initialization Routine
 *
 *************************************************************/

/* MDC = SCLK / MDC_freq / 2 - 1 */
#define MDC_FREQ_TO_DIV(mdc_freq) (get_sclk() / (mdc_freq) / 2 - 1)

#ifndef CONFIG_BFIN_MAC_PINS
# ifdef CONFIG_RMII
#  define CONFIG_BFIN_MAC_PINS P_RMII0
# else
#  define CONFIG_BFIN_MAC_PINS P_MII0
# endif
#endif

static int bfin_miiphy_init(struct eth_device *dev, int *opmode)
{
	const unsigned short pins[] = CONFIG_BFIN_MAC_PINS;
	u16 phydat;
	size_t count;

	/* Enable PHY output */
	bfin_write_VR_CTL(bfin_read_VR_CTL() | CLKBUFOE);

	/* Set all the pins to peripheral mode */
	peripheral_request_list(pins, "bfin_mac");

	/* Odd word alignment for Receive Frame DMA word */
	/* Configure checksum support and rcve frame word alignment */
	bfin_write_EMAC_SYSCTL(RXDWA | RXCKS | SET_MDCDIV(MDC_FREQ_TO_DIV(CONFIG_PHY_CLOCK_FREQ)));

	/* turn on auto-negotiation and wait for link to come up */
	bfin_miiphy_write(dev->name, CONFIG_PHY_ADDR, MII_BMCR, BMCR_ANENABLE);
	count = 0;
	while (1) {
		++count;
		if (bfin_miiphy_read(dev->name, CONFIG_PHY_ADDR, MII_BMSR, &phydat))
			return -1;
		if (phydat & BMSR_LSTATUS)
			break;
		if (count > 30000) {
			printf("%s: link down, check cable\n", dev->name);
			return -1;
		}
		udelay(100);
	}

	/* see what kind of link we have */
	if (bfin_miiphy_read(dev->name, CONFIG_PHY_ADDR, MII_LPA, &phydat))
		return -1;
	if (phydat & LPA_DUPLEX)
		*opmode = FDMODE;
	else
		*opmode = 0;

	bfin_write_EMAC_MMC_CTL(RSTC | CROLL);
	bfin_write_EMAC_VLAN1(EMAC_VLANX_DEF_VAL);
	bfin_write_EMAC_VLAN2(EMAC_VLANX_DEF_VAL);

	/* Initialize the TX DMA channel registers */
	bfin_write_DMA2_X_COUNT(0);
	bfin_write_DMA2_X_MODIFY(4);
	bfin_write_DMA2_Y_COUNT(0);
	bfin_write_DMA2_Y_MODIFY(0);

	/* Initialize the RX DMA channel registers */
	bfin_write_DMA1_X_COUNT(0);
	bfin_write_DMA1_X_MODIFY(4);
	bfin_write_DMA1_Y_COUNT(0);
	bfin_write_DMA1_Y_MODIFY(0);

	return 0;
}

static int bfin_EMAC_setup_addr(struct eth_device *dev)
{
	bfin_write_EMAC_ADDRLO(
		dev->enetaddr[0] |
		dev->enetaddr[1] << 8 |
		dev->enetaddr[2] << 16 |
		dev->enetaddr[3] << 24
	);
	bfin_write_EMAC_ADDRHI(
		dev->enetaddr[4] |
		dev->enetaddr[5] << 8
	);
	return 0;
}

static int bfin_EMAC_init(struct eth_device *dev, bd_t *bd)
{
	u32 opmode;
	int dat;
	int i;
	debug("Eth_init: ......\n");

	txIdx = 0;
	rxIdx = 0;

	/* Initialize System Register */
	if (bfin_miiphy_init(dev, &dat) < 0)
		return -1;

	/* Initialize EMAC address */
	bfin_EMAC_setup_addr(dev);

	/* Initialize TX and RX buffer */
	for (i = 0; i < PKTBUFSRX; i++) {
		rxbuf[i] = SetupRxBuffer(i);
		if (i > 0) {
			rxbuf[i - 1]->Dma[1].NEXT_DESC_PTR = rxbuf[i]->Dma;
			if (i == (PKTBUFSRX - 1))
				rxbuf[i]->Dma[1].NEXT_DESC_PTR = rxbuf[0]->Dma;
		}
	}
	for (i = 0; i < TX_BUF_CNT; i++) {
		txbuf[i] = SetupTxBuffer(i);
		if (i > 0) {
			txbuf[i - 1]->Dma[1].NEXT_DESC_PTR = txbuf[i]->Dma;
			if (i == (TX_BUF_CNT - 1))
				txbuf[i]->Dma[1].NEXT_DESC_PTR = txbuf[0]->Dma;
		}
	}

	/* Set RX DMA */
	bfin_write_DMA1_NEXT_DESC_PTR(rxbuf[0]->Dma);
	bfin_write_DMA1_CONFIG(rxbuf[0]->Dma[0].CONFIG_DATA);

	/* Wait MII done */
	bfin_miiphy_wait();

	/* We enable only RX here */
	/* ASTP   : Enable Automatic Pad Stripping
	   PR     : Promiscuous Mode for test
	   PSF    : Receive frames with total length less than 64 bytes.
	   FDMODE : Full Duplex Mode
	   LB	  : Internal Loopback for test
	   RE     : Receiver Enable */
	if (dat == FDMODE)
		opmode = ASTP | FDMODE | PSF;
	else
		opmode = ASTP | PSF;
	opmode |= RE;
#ifdef CONFIG_RMII
	opmode |= TE | RMII;
#endif
	/* Turn on the EMAC */
	bfin_write_EMAC_OPMODE(opmode);
	return 0;
}

static void bfin_EMAC_halt(struct eth_device *dev)
{
	debug("Eth_halt: ......\n");
	/* Turn off the EMAC */
	bfin_write_EMAC_OPMODE(0);
	/* Turn off the EMAC RX DMA */
	bfin_write_DMA1_CONFIG(0);
	bfin_write_DMA2_CONFIG(0);
}

ADI_ETHER_BUFFER *SetupRxBuffer(int no)
{
	ADI_ETHER_FRAME_BUFFER *frmbuf;
	ADI_ETHER_BUFFER *buf;
	int nobytes_buffer = sizeof(ADI_ETHER_BUFFER[2]) / 2;	/* ensure a multi. of 4 */
	int total_size = nobytes_buffer + RECV_BUFSIZE;

	buf = (void *) (RXBUF_BASE_ADDR + no * total_size);
	frmbuf = (void *) (RXBUF_BASE_ADDR + no * total_size + nobytes_buffer);

	memset(buf, 0x00, nobytes_buffer);
	buf->FrmData = frmbuf;
	memset(frmbuf, 0xfe, RECV_BUFSIZE);

	/* set up first desc to point to receive frame buffer */
	buf->Dma[0].NEXT_DESC_PTR = &(buf->Dma[1]);
	buf->Dma[0].START_ADDR = (u32) buf->FrmData;
	buf->Dma[0].CONFIG.b_DMA_EN = 1;	/* enabled */
	buf->Dma[0].CONFIG.b_WNR = 1;	/* Write to memory */
	buf->Dma[0].CONFIG.b_WDSIZE = 2;	/* wordsize is 32 bits */
	buf->Dma[0].CONFIG.b_NDSIZE = 5;	/* 5 half words is desc size. */
	buf->Dma[0].CONFIG.b_FLOW = 7;	/* large desc flow */

	/* set up second desc to point to status word */
	buf->Dma[1].NEXT_DESC_PTR = buf->Dma;
	buf->Dma[1].START_ADDR = (u32) & buf->IPHdrChksum;
	buf->Dma[1].CONFIG.b_DMA_EN = 1;	/* enabled */
	buf->Dma[1].CONFIG.b_WNR = 1;	/* Write to memory */
	buf->Dma[1].CONFIG.b_WDSIZE = 2;	/* wordsize is 32 bits */
	buf->Dma[1].CONFIG.b_DI_EN = 1;	/* enable interrupt */
	buf->Dma[1].CONFIG.b_NDSIZE = 5;	/* must be 0 when FLOW is 0 */
	buf->Dma[1].CONFIG.b_FLOW = 7;	/* stop */

	return buf;
}

ADI_ETHER_BUFFER *SetupTxBuffer(int no)
{
	ADI_ETHER_FRAME_BUFFER *frmbuf;
	ADI_ETHER_BUFFER *buf;
	int nobytes_buffer = sizeof(ADI_ETHER_BUFFER[2]) / 2;	/* ensure a multi. of 4 */
	int total_size = nobytes_buffer + RECV_BUFSIZE;

	buf = (void *) (TXBUF_BASE_ADDR + no * total_size);
	frmbuf = (void *) (TXBUF_BASE_ADDR + no * total_size + nobytes_buffer);

	memset(buf, 0x00, nobytes_buffer);
	buf->FrmData = frmbuf;
	memset(frmbuf, 0x00, RECV_BUFSIZE);

	/* set up first desc to point to receive frame buffer */
	buf->Dma[0].NEXT_DESC_PTR = &(buf->Dma[1]);
	buf->Dma[0].START_ADDR = (u32) buf->FrmData;
	buf->Dma[0].CONFIG.b_DMA_EN = 1;	/* enabled */
	buf->Dma[0].CONFIG.b_WNR = 0;	/* Read to memory */
	buf->Dma[0].CONFIG.b_WDSIZE = 2;	/* wordsize is 32 bits */
	buf->Dma[0].CONFIG.b_NDSIZE = 5;	/* 5 half words is desc size. */
	buf->Dma[0].CONFIG.b_FLOW = 7;	/* large desc flow */

	/* set up second desc to point to status word */
	buf->Dma[1].NEXT_DESC_PTR = &(buf->Dma[0]);
	buf->Dma[1].START_ADDR = (u32) & buf->StatusWord;
	buf->Dma[1].CONFIG.b_DMA_EN = 1;	/* enabled */
	buf->Dma[1].CONFIG.b_WNR = 1;	/* Write to memory */
	buf->Dma[1].CONFIG.b_WDSIZE = 2;	/* wordsize is 32 bits */
	buf->Dma[1].CONFIG.b_DI_EN = 1;	/* enable interrupt */
	buf->Dma[1].CONFIG.b_NDSIZE = 0;	/* must be 0 when FLOW is 0 */
	buf->Dma[1].CONFIG.b_FLOW = 0;	/* stop */

	return buf;
}

#if defined(CONFIG_POST) && defined(CONFIG_SYS_POST_ETHER)
int ether_post_test(int flags)
{
	uchar buf[64];
	int i, value = 0;
	int length;
	uint addr;

	printf("\n--------");
	bfin_EMAC_init(NULL, NULL);
	/* construct the package */
	addr = bfin_read_EMAC_ADDRLO();
	buf[0] = buf[6] = addr;
	buf[1] = buf[7] = addr >> 8;
	buf[2] = buf[8] = addr >> 16;
	buf[3] = buf[9] = addr >> 24;
	addr = bfin_read_EMAC_ADDRHI();
	buf[4] = buf[10] = addr;
	buf[5] = buf[11] = addr >> 8;
	buf[12] = 0x08;		/* Type: ARP */
	buf[13] = 0x06;
	buf[14] = 0x00;		/* Hardware type: Ethernet */
	buf[15] = 0x01;
	buf[16] = 0x08;		/* Protocal type: IP */
	buf[17] = 0x00;
	buf[18] = 0x06;		/* Hardware size    */
	buf[19] = 0x04;		/* Protocol size    */
	buf[20] = 0x00;		/* Opcode: request  */
	buf[21] = 0x01;

	for (i = 0; i < 42; i++)
		buf[i + 22] = i;
	printf("--------Send 64 bytes......\n");
	bfin_EMAC_send(NULL, buf, 64);
	for (i = 0; i < 100; i++) {
		udelay(10000);
		if ((rxbuf[rxIdx]->StatusWord & RX_COMP) != 0) {
			value = 1;
			break;
		}
	}
	if (value == 0) {
		printf("--------EMAC can't receive any data\n");
		eth_halt();
		return -1;
	}
	length = rxbuf[rxIdx]->StatusWord & 0x000007FF - 4;
	for (i = 0; i < length; i++) {
		if (rxbuf[rxIdx]->FrmData->Dest[i] != buf[i]) {
			printf("--------EMAC receive error data!\n");
			eth_halt();
			return -1;
		}
	}
	printf("--------receive %d bytes, matched\n", length);
	bfin_EMAC_halt(NULL);
	return 0;
}
#endif

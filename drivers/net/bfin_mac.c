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
#include <command.h>
#include <malloc.h>

#include <asm/blackfin.h>
#include <asm/mach-common/bits/dma.h>
#include <asm/mach-common/bits/emac.h>
#include <asm/mach-common/bits/pll.h>

#include "bfin_mac.h"

#ifdef CONFIG_POST
#include <post.h>
#endif

#undef DEBUG_ETHERNET

#ifdef DEBUG_ETHERNET
#define DEBUGF(fmt, args...) printf(fmt, ##args)
#else
#define DEBUGF(fmt, args...)
#endif

#define RXBUF_BASE_ADDR		0xFF900000
#define TXBUF_BASE_ADDR		0xFF800000
#define TX_BUF_CNT		1

#define TOUT_LOOP 		1000000

ADI_ETHER_BUFFER *txbuf[TX_BUF_CNT];
ADI_ETHER_BUFFER *rxbuf[PKTBUFSRX];
static u16 txIdx;		/* index of the current RX buffer */
static u16 rxIdx;		/* index of the current TX buffer */

u16 PHYregs[NO_PHY_REGS];	/* u16 PHYADDR; */

/* DMAx_CONFIG values at DMA Restart */
const ADI_DMA_CONFIG_REG rxdmacfg = {
	.b_DMA_EN  = 1,	/* enabled */
	.b_WNR     = 1,	/* write to memory */
	.b_WDSIZE  = 2,	/* wordsize is 32 bits */
	.b_DMA2D   = 0,
	.b_RESTART = 0,
	.b_DI_SEL  = 0,
	.b_DI_EN   = 0,	/* no interrupt */
	.b_NDSIZE  = 5,	/* 5 half words is desc size */
	.b_FLOW    = 7	/* large desc flow */
};

const ADI_DMA_CONFIG_REG txdmacfg = {
	.b_DMA_EN  = 1,	/* enabled */
	.b_WNR     = 0,	/* read from memory */
	.b_WDSIZE  = 2,	/* wordsize is 32 bits */
	.b_DMA2D   = 0,
	.b_RESTART = 0,
	.b_DI_SEL  = 0,
	.b_DI_EN   = 0,	/* no interrupt */
	.b_NDSIZE  = 5,	/* 5 half words is desc size */
	.b_FLOW    = 7	/* large desc flow */
};

int bfin_EMAC_initialize(bd_t *bis)
{
	struct eth_device *dev;
	dev = (struct eth_device *)malloc(sizeof(*dev));
	if (dev == NULL)
		hang();

	memset(dev, 0, sizeof(*dev));
	sprintf(dev->name, "Blackfin EMAC");

	dev->iobase = 0;
	dev->priv = 0;
	dev->init = bfin_EMAC_init;
	dev->halt = bfin_EMAC_halt;
	dev->send = bfin_EMAC_send;
	dev->recv = bfin_EMAC_recv;

	eth_register(dev);

	return 1;
}

static int bfin_EMAC_send(struct eth_device *dev, volatile void *packet,
			  int length)
{
	int i;
	int result = 0;
	unsigned int *buf;
	buf = (unsigned int *)packet;

	if (length <= 0) {
		printf("Ethernet: bad packet size: %d\n", length);
		goto out;
	}

	if ((*pDMA2_IRQ_STATUS & DMA_ERR) != 0) {
		printf("Ethernet: tx DMA error\n");
		goto out;
	}

	for (i = 0; (*pDMA2_IRQ_STATUS & DMA_RUN) != 0; i++) {
		if (i > TOUT_LOOP) {
			puts("Ethernet: tx time out\n");
			goto out;
		}
	}
	txbuf[txIdx]->FrmData->NoBytes = length;
	memcpy(txbuf[txIdx]->FrmData->Dest, (void *)packet, length);
	txbuf[txIdx]->Dma[0].START_ADDR = (u32) txbuf[txIdx]->FrmData;
	*pDMA2_NEXT_DESC_PTR = &txbuf[txIdx]->Dma[0];
	*pDMA2_CONFIG = *(u16 *) (void *)(&txdmacfg);
	*pEMAC_OPMODE |= TE;

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
	DEBUGF("BFIN EMAC send: length = %d\n", length);
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
		NetRxPackets[rxIdx] =
		    (volatile uchar *)(rxbuf[rxIdx]->FrmData->Dest);
		NetReceive(NetRxPackets[rxIdx], length - 4);
		*pDMA1_IRQ_STATUS |= DMA_DONE | DMA_ERR;
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

static int bfin_EMAC_init(struct eth_device *dev, bd_t *bd)
{
	u32 opmode;
	int dat;
	int i;
	DEBUGF("Eth_init: ......\n");

	txIdx = 0;
	rxIdx = 0;

/* Initialize System Register */
	if (SetupSystemRegs(&dat) < 0)
		return -1;

/* Initialize EMAC address */
	bfin_EMAC_setup_addr(bd);

/* Initialize TX and RX buffer */
	for (i = 0; i < PKTBUFSRX; i++) {
		rxbuf[i] = SetupRxBuffer(i);
		if (i > 0) {
			rxbuf[i - 1]->Dma[1].NEXT_DESC_PTR =
			    &(rxbuf[i]->Dma[0]);
			if (i == (PKTBUFSRX - 1))
				rxbuf[i]->Dma[1].NEXT_DESC_PTR =
				    &(rxbuf[0]->Dma[0]);
		}
	}
	for (i = 0; i < TX_BUF_CNT; i++) {
		txbuf[i] = SetupTxBuffer(i);
		if (i > 0) {
			txbuf[i - 1]->Dma[1].NEXT_DESC_PTR =
			    &(txbuf[i]->Dma[0]);
			if (i == (TX_BUF_CNT - 1))
				txbuf[i]->Dma[1].NEXT_DESC_PTR =
				    &(txbuf[0]->Dma[0]);
		}
	}

	/* Set RX DMA */
	*pDMA1_NEXT_DESC_PTR = &rxbuf[0]->Dma[0];
	*pDMA1_CONFIG = *((u16 *) (void *)&rxbuf[0]->Dma[0].CONFIG);

	/* Wait MII done */
	PollMdcDone();

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
#ifdef CONFIG_BFIN_MAC_RMII
	opmode |= TE | RMII;
#endif
	/* Turn on the EMAC */
	*pEMAC_OPMODE = opmode;
	return 0;
}

static void bfin_EMAC_halt(struct eth_device *dev)
{
	DEBUGF("Eth_halt: ......\n");
	/* Turn off the EMAC */
	*pEMAC_OPMODE = 0x00000000;
	/* Turn off the EMAC RX DMA */
	*pDMA1_CONFIG = 0x0000;
	*pDMA2_CONFIG = 0x0000;

}

void bfin_EMAC_setup_addr(bd_t *bd)
{
	*pEMAC_ADDRLO =
		bd->bi_enetaddr[0] |
		bd->bi_enetaddr[1] << 8 |
		bd->bi_enetaddr[2] << 16 |
		bd->bi_enetaddr[3] << 24;
	*pEMAC_ADDRHI =
		bd->bi_enetaddr[4] |
		bd->bi_enetaddr[5] << 8;
}

static void PollMdcDone(void)
{
	/* poll the STABUSY bit */
	while (*pEMAC_STAADD & STABUSY) ;
}

static void WrPHYReg(u16 PHYAddr, u16 RegAddr, u16 Data)
{
	PollMdcDone();

	*pEMAC_STADAT = Data;

	*pEMAC_STAADD = SET_PHYAD(PHYAddr) | SET_REGAD(RegAddr) |
	    STAOP | STAIE | STABUSY;
}

/*********************************************************************************
 *		Read an off-chip register in a PHY through the MDC/MDIO port     *
 *********************************************************************************/
static u16 RdPHYReg(u16 PHYAddr, u16 RegAddr)
{
	u16 Data;

	PollMdcDone();

	*pEMAC_STAADD = SET_PHYAD(PHYAddr) | SET_REGAD(RegAddr) |
	    STAIE | STABUSY;

	PollMdcDone();

	Data = (u16) * pEMAC_STADAT;

	PHYregs[RegAddr] = Data;	/* save shadow copy */

	return Data;
}

#if 0 /* dead code ? */
static void SoftResetPHY(void)
{
	u16 phydat;
	/* set the reset bit */
	WrPHYReg(PHYADDR, PHY_MODECTL, PHY_RESET);
	/* and clear it again */
	WrPHYReg(PHYADDR, PHY_MODECTL, 0x0000);
	do {
		/* poll until reset is complete */
		phydat = RdPHYReg(PHYADDR, PHY_MODECTL);
	} while ((phydat & PHY_RESET) != 0);
}
#endif

static int SetupSystemRegs(int *opmode)
{
	u16 sysctl, phydat;
	int count = 0;
	/* Enable PHY output */
	*pVR_CTL |= CLKBUFOE;
	/* Set all the pins to peripheral mode */

#ifndef CONFIG_BFIN_MAC_RMII
	*pPORTH_FER = 0xFFFF;
#ifdef __ADSPBF52x__
	*pPORTH_MUX = PORT_x_MUX_0_FUNC_2 | PORT_x_MUX_1_FUNC_2 | PORT_x_MUX_2_FUNC_2;
#endif
#else
#if defined(__ADSPBF536__) || defined(__ADSPBF537__)
	*pPORTH_FER = 0xC373;
#endif
#ifdef __ADSPBF52x__
	*pPORTH_FER = 0x01FF;
	*pPORTH_MUX = PORT_x_MUX_0_FUNC_2 | PORT_x_MUX_1_FUNC_2;
#endif
#endif
	/* MDC  = 2.5 MHz */
	sysctl = SET_MDCDIV(24);
	/* Odd word alignment for Receive Frame DMA word */
	/* Configure checksum support and rcve frame word alignment */
	sysctl |= RXDWA | RXCKS;
	*pEMAC_SYSCTL = sysctl;
	/* auto negotiation on  */
	/* full duplex */
	/* 100 Mbps */
	phydat = PHY_ANEG_EN | PHY_DUPLEX | PHY_SPD_SET;
	WrPHYReg(PHYADDR, PHY_MODECTL, phydat);
	do {
		udelay(1000);
		phydat = RdPHYReg(PHYADDR, PHY_MODESTAT);
		if (count > 3000) {
			printf
			    ("Link is down, please check your network connection\n");
			return -1;
		}
		count++;
	} while (!(phydat & 0x0004));

	phydat = RdPHYReg(PHYADDR, PHY_ANLPAR);

	if ((phydat & 0x0100) || (phydat & 0x0040))
		*opmode = FDMODE;
	else
		*opmode = 0;

	*pEMAC_MMC_CTL = RSTC | CROLL;

	/* Initialize the TX DMA channel registers */
	*pDMA2_X_COUNT = 0;
	*pDMA2_X_MODIFY = 4;
	*pDMA2_Y_COUNT = 0;
	*pDMA2_Y_MODIFY = 0;

	/* Initialize the RX DMA channel registers */
	*pDMA1_X_COUNT = 0;
	*pDMA1_X_MODIFY = 4;
	*pDMA1_Y_COUNT = 0;
	*pDMA1_Y_MODIFY = 0;
	return 0;
}

ADI_ETHER_BUFFER *SetupRxBuffer(int no)
{
	ADI_ETHER_FRAME_BUFFER *frmbuf;
	ADI_ETHER_BUFFER *buf;
	int nobytes_buffer = sizeof(ADI_ETHER_BUFFER[2]) / 2;	/* ensure a multi. of 4 */
	int total_size = nobytes_buffer + RECV_BUFSIZE;

	buf = (ADI_ETHER_BUFFER *) (RXBUF_BASE_ADDR + no * total_size);
	frmbuf =
	    (ADI_ETHER_FRAME_BUFFER *) (RXBUF_BASE_ADDR + no * total_size +
					nobytes_buffer);

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
	buf->Dma[1].NEXT_DESC_PTR = &(buf->Dma[0]);
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

	buf = (ADI_ETHER_BUFFER *) (TXBUF_BASE_ADDR + no * total_size);
	frmbuf =
	    (ADI_ETHER_FRAME_BUFFER *) (TXBUF_BASE_ADDR + no * total_size +
					nobytes_buffer);

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

#if defined(CONFIG_POST) && defined(CFG_POST_ETHER)
int ether_post_test(int flags)
{
	uchar buf[64];
	int i, value = 0;
	int length;

	printf("\n--------");
	bfin_EMAC_init(NULL, NULL);
	/* construct the package */
	buf[0] = buf[6] = (unsigned char)(*pEMAC_ADDRLO & 0xFF);
	buf[1] = buf[7] = (unsigned char)((*pEMAC_ADDRLO & 0xFF00) >> 8);
	buf[2] = buf[8] = (unsigned char)((*pEMAC_ADDRLO & 0xFF0000) >> 16);
	buf[3] = buf[9] = (unsigned char)((*pEMAC_ADDRLO & 0xFF000000) >> 24);
	buf[4] = buf[10] = (unsigned char)(*pEMAC_ADDRHI & 0xFF);
	buf[5] = buf[11] = (unsigned char)((*pEMAC_ADDRHI & 0xFF00) >> 8);
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
	bfin_EMAC_send(NULL, (volatile void *)buf, 64);
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

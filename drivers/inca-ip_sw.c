/*
 * INCA-IP internal switch ethernet driver.
 *
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>

#if defined(CONFIG_CMD_NET) \
	&& defined(CONFIG_NET_MULTI) && defined(CONFIG_INCA_IP_SWITCH)

#include <malloc.h>
#include <net.h>
#include <asm/inca-ip.h>
#include <asm/addrspace.h>


#define NUM_RX_DESC	PKTBUFSRX
#define NUM_TX_DESC	3
#define TOUT_LOOP	1000000


#define DELAY	udelay(10000)
  /* Sometimes the store word instruction hangs while writing to one
   * of the Switch registers. Moving the instruction into a separate
   * function somehow makes the problem go away.
   */
static void SWORD(volatile u32 * reg, u32 value)
{
	*reg = value;
}

#define DMA_WRITE_REG(reg, value) *((volatile u32 *)reg) = (u32)value;
#define DMA_READ_REG(reg, value)    value = (u32)*((volatile u32*)reg)
#define SW_WRITE_REG(reg, value)   \
	SWORD(reg, value);\
	DELAY;\
	SWORD(reg, value);

#define SW_READ_REG(reg, value)	   \
	value = (u32)*((volatile u32*)reg);\
	DELAY;\
	value = (u32)*((volatile u32*)reg);

#define INCA_DMA_TX_POLLING_TIME	0x07
#define INCA_DMA_RX_POLLING_TIME	0x07

#define INCA_DMA_TX_HOLD		0x80000000
#define INCA_DMA_TX_EOP			0x40000000
#define INCA_DMA_TX_SOP			0x20000000
#define INCA_DMA_TX_ICPT		0x10000000
#define INCA_DMA_TX_IEOP		0x08000000

#define INCA_DMA_RX_C			0x80000000
#define INCA_DMA_RX_SOP			0x40000000
#define INCA_DMA_RX_EOP			0x20000000

#define INCA_SWITCH_PHY_SPEED_10H	0x1
#define INCA_SWITCH_PHY_SPEED_10F	0x5
#define INCA_SWITCH_PHY_SPEED_100H	0x2
#define INCA_SWITCH_PHY_SPEED_100F	0x6

/************************ Auto MDIX settings ************************/
#define INCA_IP_AUTO_MDIX_LAN_PORTS_DIR		INCA_IP_Ports_P1_DIR
#define INCA_IP_AUTO_MDIX_LAN_PORTS_ALTSEL	INCA_IP_Ports_P1_ALTSEL
#define INCA_IP_AUTO_MDIX_LAN_PORTS_OUT		INCA_IP_Ports_P1_OUT
#define INCA_IP_AUTO_MDIX_LAN_GPIO_PIN_RXTX	16

#define WAIT_SIGNAL_RETRIES			100
#define WAIT_LINK_RETRIES			100
#define LINK_RETRY_DELAY			2000  /* ms */
/********************************************************************/

typedef struct
{
	union {
		struct {
			volatile u32 HOLD		:1;
			volatile u32 ICpt		:1;
			volatile u32 IEop		:1;
			volatile u32 offset		:3;
			volatile u32 reserved0		:4;
			volatile u32 NFB		:22;
		}field;

		volatile u32 word;
	}params;

	volatile u32 nextRxDescPtr;

	volatile u32 RxDataPtr;

	union {
		struct {
			volatile u32 C			:1;
			volatile u32 Sop		:1;
			volatile u32 Eop		:1;
			volatile u32 reserved3		:12;
			volatile u32 NBT		:17;
		}field;

		volatile u32 word;
	}status;

} inca_rx_descriptor_t;


typedef struct
{
	union {
		struct {
			volatile u32 HOLD		:1;
			volatile u32 Eop		:1;
			volatile u32 Sop		:1;
			volatile u32 ICpt		:1;
			volatile u32 IEop		:1;
			volatile u32 reserved0		:5;
			volatile u32 NBA		:22;
		}field;

		volatile u32 word;
	}params;

	volatile u32 nextTxDescPtr;

	volatile u32 TxDataPtr;

	volatile u32 C			:1;
	volatile u32 reserved3		:31;

} inca_tx_descriptor_t;


static inca_rx_descriptor_t rx_ring[NUM_RX_DESC] __attribute__ ((aligned(16)));
static inca_tx_descriptor_t tx_ring[NUM_TX_DESC] __attribute__ ((aligned(16)));

static int tx_new, rx_new, tx_hold, rx_hold;
static int tx_old_hold = -1;
static int initialized	= 0;


static int inca_switch_init(struct eth_device *dev, bd_t * bis);
static int inca_switch_send(struct eth_device *dev, volatile void *packet, int length);
static int inca_switch_recv(struct eth_device *dev);
static void inca_switch_halt(struct eth_device *dev);
static void inca_init_switch_chip(void);
static void inca_dma_init(void);
static int inca_amdix(void);


int inca_switch_initialize(bd_t * bis)
{
	struct eth_device *dev;

#if 0
	printf("Entered inca_switch_initialize()\n");
#endif

	if (!(dev = (struct eth_device *) malloc (sizeof *dev))) {
		printf("Failed to allocate memory\n");
		return 0;
	}
	memset(dev, 0, sizeof(*dev));

	inca_dma_init();

	inca_init_switch_chip();

#if defined(CONFIG_INCA_IP_SWITCH_AMDIX)
	inca_amdix();
#endif

	sprintf(dev->name, "INCA-IP Switch");
	dev->init = inca_switch_init;
	dev->halt = inca_switch_halt;
	dev->send = inca_switch_send;
	dev->recv = inca_switch_recv;

	eth_register(dev);

#if 0
	printf("Leaving inca_switch_initialize()\n");
#endif

	return 1;
}


static int inca_switch_init(struct eth_device *dev, bd_t * bis)
{
	int i;
	u32 v, regValue;
	u16 wTmp;

#if 0
	printf("Entering inca_switch_init()\n");
#endif

	/* Set MAC address.
	 */
	wTmp = (u16)dev->enetaddr[0];
	regValue = (wTmp << 8) | dev->enetaddr[1];

	SW_WRITE_REG(INCA_IP_Switch_PMAC_SA1, regValue);

	wTmp = (u16)dev->enetaddr[2];
	regValue = (wTmp << 8) | dev->enetaddr[3];
	regValue = regValue << 16;
	wTmp = (u16)dev->enetaddr[4];
	regValue |= (wTmp<<8) | dev->enetaddr[5];

	SW_WRITE_REG(INCA_IP_Switch_PMAC_SA2, regValue);

	/* Initialize the descriptor rings.
	 */
	for (i = 0; i < NUM_RX_DESC; i++) {
		inca_rx_descriptor_t * rx_desc = KSEG1ADDR(&rx_ring[i]);
		memset(rx_desc, 0, sizeof(rx_ring[i]));

		/* Set maximum size of receive buffer.
		 */
		rx_desc->params.field.NFB = PKTSIZE_ALIGN;

		/* Set the offset of the receive buffer. Zero means
		 * that the offset mechanism is not used.
		 */
		rx_desc->params.field.offset = 0;

		/* Check if it is the last descriptor.
		 */
		if (i == (NUM_RX_DESC - 1)) {
			/* Let the last descriptor point to the first
			 * one.
			 */
			rx_desc->nextRxDescPtr = KSEG1ADDR((u32)rx_ring);
		} else {
			/* Set the address of the next descriptor.
			 */
			rx_desc->nextRxDescPtr = (u32)KSEG1ADDR(&rx_ring[i+1]);
		}

		rx_desc->RxDataPtr = (u32)KSEG1ADDR(NetRxPackets[i]);
	}

#if 0
	printf("rx_ring = 0x%08X 0x%08X\n", (u32)rx_ring, (u32)&rx_ring[0]);
	printf("tx_ring = 0x%08X 0x%08X\n", (u32)tx_ring, (u32)&tx_ring[0]);
#endif

	for (i = 0; i < NUM_TX_DESC; i++) {
		inca_tx_descriptor_t * tx_desc = KSEG1ADDR(&tx_ring[i]);

		memset(tx_desc, 0, sizeof(tx_ring[i]));

		tx_desc->params.word	   = 0;
		tx_desc->params.field.HOLD = 1;
		tx_desc->C		   = 1;

			/* Check if it is the last descriptor.
			 */
		if (i == (NUM_TX_DESC - 1)) {
				/* Let the last descriptor point to the
				 * first one.
				 */
			tx_desc->nextTxDescPtr = KSEG1ADDR((u32)tx_ring);
		} else {
				/* Set the address of the next descriptor.
				 */
			tx_desc->nextTxDescPtr = (u32)KSEG1ADDR(&tx_ring[i+1]);
		}
	}

	/* Initialize RxDMA.
	 */
	DMA_READ_REG(INCA_IP_DMA_DMA_RXISR, v);
#if 0
	printf("RX status = 0x%08X\n", v);
#endif

	/* Writing to the FRDA of CHANNEL.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_RXFRDA0, (u32)rx_ring);

	/* Writing to the COMMAND REG.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_RXCCR0, INCA_IP_DMA_DMA_RXCCR0_INIT);

	/* Initialize TxDMA.
	 */
	DMA_READ_REG(INCA_IP_DMA_DMA_TXISR, v);
#if 0
	printf("TX status = 0x%08X\n", v);
#endif

	/* Writing to the FRDA of CHANNEL.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXFRDA0, (u32)tx_ring);

	tx_new = rx_new = 0;

	tx_hold = NUM_TX_DESC - 1;
	rx_hold = NUM_RX_DESC - 1;

#if 0
	rx_ring[rx_hold].params.field.HOLD = 1;
#endif
	/* enable spanning tree forwarding, enable the CPU port */
	/* ST_PT:
	 *	CPS (CPU port status)	0x3 (forwarding)
	 *	LPS (LAN port status)	0x3 (forwarding)
	 *	PPS (PC port status)	0x3 (forwarding)
	 */
	SW_WRITE_REG(INCA_IP_Switch_ST_PT,0x3f);

#if 0
	printf("Leaving inca_switch_init()\n");
#endif

	return 0;
}


static int inca_switch_send(struct eth_device *dev, volatile void *packet, int length)
{
	int		       i;
	int		       res	= -1;
	u32		       command;
	u32		       regValue;
	inca_tx_descriptor_t * tx_desc	= KSEG1ADDR(&tx_ring[tx_new]);

#if 0
	printf("Entered inca_switch_send()\n");
#endif

	if (length <= 0) {
		printf ("%s: bad packet size: %d\n", dev->name, length);
		goto Done;
	}

	for(i = 0; tx_desc->C == 0; i++) {
		if (i >= TOUT_LOOP) {
			printf("%s: tx error buffer not ready\n", dev->name);
			goto Done;
		}
	}

	if (tx_old_hold >= 0) {
		KSEG1ADDR(&tx_ring[tx_old_hold])->params.field.HOLD = 1;
	}
	tx_old_hold = tx_hold;

	tx_desc->params.word =
			(INCA_DMA_TX_SOP | INCA_DMA_TX_EOP | INCA_DMA_TX_HOLD);

	tx_desc->C = 0;
	tx_desc->TxDataPtr = (u32)packet;
	tx_desc->params.field.NBA = length;

	KSEG1ADDR(&tx_ring[tx_hold])->params.field.HOLD = 0;

	tx_hold = tx_new;
	tx_new	= (tx_new + 1) % NUM_TX_DESC;


	if (! initialized) {
		command = INCA_IP_DMA_DMA_TXCCR0_INIT;
		initialized = 1;
	} else {
		command = INCA_IP_DMA_DMA_TXCCR0_HR;
	}

	DMA_READ_REG(INCA_IP_DMA_DMA_TXCCR0, regValue);
	regValue |= command;
#if 0
	printf("regValue = 0x%x\n", regValue);
#endif
	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXCCR0, regValue);

#if 1
	for(i = 0; KSEG1ADDR(&tx_ring[tx_hold])->C == 0; i++) {
		if (i >= TOUT_LOOP) {
			printf("%s: tx buffer not ready\n", dev->name);
			goto Done;
		}
	}
#endif
	res = length;
Done:
#if 0
	printf("Leaving inca_switch_send()\n");
#endif
	return res;
}


static int inca_switch_recv(struct eth_device *dev)
{
	int		       length  = 0;
	inca_rx_descriptor_t * rx_desc;

#if 0
	printf("Entered inca_switch_recv()\n");
#endif

	for (;;) {
		rx_desc = KSEG1ADDR(&rx_ring[rx_new]);

		if (rx_desc->status.field.C == 0) {
			break;
		}

#if 0
		rx_ring[rx_new].params.field.HOLD = 1;
#endif

		if (! rx_desc->status.field.Eop) {
			printf("Partly received packet!!!\n");
			break;
		}

		length = rx_desc->status.field.NBT;
		rx_desc->status.word &=
			 ~(INCA_DMA_RX_EOP | INCA_DMA_RX_SOP | INCA_DMA_RX_C);
#if 0
{
  int i;
  for (i=0;i<length - 4;i++) {
    if (i % 16 == 0) printf("\n%04x: ", i);
    printf("%02X ", NetRxPackets[rx_new][i]);
  }
  printf("\n");
}
#endif

		if (length) {
#if 0
			printf("Received %d bytes\n", length);
#endif
			NetReceive((void*)KSEG1ADDR(NetRxPackets[rx_new]), length - 4);
		} else {
#if 1
			printf("Zero length!!!\n");
#endif
		}


		KSEG1ADDR(&rx_ring[rx_hold])->params.field.HOLD = 0;

		rx_hold = rx_new;

		rx_new = (rx_new + 1) % NUM_RX_DESC;
	}

#if 0
	printf("Leaving inca_switch_recv()\n");
#endif

	return length;
}


static void inca_switch_halt(struct eth_device *dev)
{
#if 0
	printf("Entered inca_switch_halt()\n");
#endif

#if 1
	initialized = 0;
#endif
#if 1
	/* Disable forwarding to the CPU port.
	 */
	SW_WRITE_REG(INCA_IP_Switch_ST_PT,0xf);

	/* Close RxDMA channel.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_RXCCR0, INCA_IP_DMA_DMA_RXCCR0_OFF);

	/* Close TxDMA channel.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXCCR0, INCA_IP_DMA_DMA_TXCCR0_OFF);


#endif
#if 0
	printf("Leaving inca_switch_halt()\n");
#endif
}


static void inca_init_switch_chip(void)
{
	u32 regValue;

	/* To workaround a problem with collision counter
	 * (see Errata sheet).
	 */
	SW_WRITE_REG(INCA_IP_Switch_PC_TX_CTL, 0x00000001);
	SW_WRITE_REG(INCA_IP_Switch_LAN_TX_CTL, 0x00000001);

#if 1
	/* init MDIO configuration:
	 *	MDS (Poll speed):	0x01 (4ms)
	 *	PHY_LAN_ADDR:		0x06
	 *	PHY_PC_ADDR:		0x05
	 *	UEP (Use External PHY): 0x00 (Internal PHY is used)
	 *	PS (Port Select):	0x00 (PT/UMM for LAN)
	 *	PT (PHY Test):		0x00 (no test mode)
	 *	UMM (Use MDIO Mode):	0x00 (state machine is disabled)
	 */
	SW_WRITE_REG(INCA_IP_Switch_MDIO_CFG, 0x4c50);

	/* init PHY:
	 *	SL (Auto Neg. Speed for LAN)
	 *	SP (Auto Neg. Speed for PC)
	 *	LL (Link Status for LAN)
	 *	LP (Link Status for PC)
	 *	DL (Duplex Status for LAN)
	 *	DP (Duplex Status for PC)
	 *	PL (Auto Neg. Pause Status for LAN)
	 *	PP (Auto Neg. Pause Status for PC)
	 */
	SW_WRITE_REG (INCA_IP_Switch_EPHY, 0xff);

	/* MDIO_ACC:
	 *	RA (Request/Ack)  0x01 (Request)
	 *	RW (Read/Write)	  0x01 (Write)
	 *	PHY_ADDR	  0x05 (PC)
	 *	REG_ADDR	  0x00 (PHY_BCR: basic control register)
	 *	PHY_DATA	  0x8000
	 *		      Reset		      - software reset
	 *		      LB (loop back)	      - normal
	 *		      SS (speed select)	      - 10 Mbit/s
	 *		      ANE (auto neg. enable)  - enable
	 *		      PD (power down)	      - normal
	 *		      ISO (isolate)	      - normal
	 *		      RAN (restart auto neg.) - normal
	 *		      DM (duplex mode)	      - half duplex
	 *		      CT (collision test)     - enable
	 */
	SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC, 0xc0a09000);

	/* MDIO_ACC:
	 *	RA (Request/Ack)  0x01 (Request)
	 *	RW (Read/Write)	  0x01 (Write)
	 *	PHY_ADDR	  0x06 (LAN)
	 *	REG_ADDR	  0x00 (PHY_BCR: basic control register)
	 *	PHY_DATA	  0x8000
	 *		      Reset		      - software reset
	 *		      LB (loop back)	      - normal
	 *		      SS (speed select)	      - 10 Mbit/s
	 *		      ANE (auto neg. enable)  - enable
	 *		      PD (power down)	      - normal
	 *		      ISO (isolate)	      - normal
	 *		      RAN (restart auto neg.) - normal
	 *		      DM (duplex mode)	      - half duplex
	 *		      CT (collision test)     - enable
	 */
	SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC, 0xc0c09000);

#endif

	/* Make sure the CPU port is disabled for now. We
	 * don't want packets to get stacked for us until
	 * we enable DMA and are prepared to receive them.
	 */
	SW_WRITE_REG(INCA_IP_Switch_ST_PT,0xf);

	SW_READ_REG(INCA_IP_Switch_ARL_CTL, regValue);

	/* CRC GEN is enabled.
	 */
	regValue |= 0x00000200;
	SW_WRITE_REG(INCA_IP_Switch_ARL_CTL, regValue);

	/* ADD TAG is disabled.
	 */
	SW_READ_REG(INCA_IP_Switch_PMAC_HD_CTL, regValue);
	regValue &= ~0x00000002;
	SW_WRITE_REG(INCA_IP_Switch_PMAC_HD_CTL, regValue);
}


static void inca_dma_init(void)
{
	/* Switch off all DMA channels.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_RXCCR0, INCA_IP_DMA_DMA_RXCCR0_OFF);
	DMA_WRITE_REG(INCA_IP_DMA_DMA_RXCCR1, INCA_IP_DMA_DMA_RXCCR1_OFF);

	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXCCR0, INCA_IP_DMA_DMA_RXCCR0_OFF);
	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXCCR1, INCA_IP_DMA_DMA_TXCCR1_OFF);
	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXCCR2, INCA_IP_DMA_DMA_TXCCR2_OFF);

	/* Setup TX channel polling time.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXPOLL, INCA_DMA_TX_POLLING_TIME);

	/* Setup RX channel polling time.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_RXPOLL, INCA_DMA_RX_POLLING_TIME);

	/* ERRATA: write reset value into the DMA RX IMR register.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_RXIMR, 0xFFFFFFFF);

	/* Just in case: disable all transmit interrupts also.
	 */
	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXIMR, 0xFFFFFFFF);

	DMA_WRITE_REG(INCA_IP_DMA_DMA_TXISR, 0xFFFFFFFF);
	DMA_WRITE_REG(INCA_IP_DMA_DMA_RXISR, 0xFFFFFFFF);
}

#if defined(CONFIG_INCA_IP_SWITCH_AMDIX)
static int inca_amdix(void)
{
	u32 phyReg1 = 0;
	u32 phyReg4 = 0;
	u32 phyReg5 = 0;
	u32 phyReg6 = 0;
	u32 phyReg31 = 0;
	u32 regEphy = 0;
	int mdi_flag;
	int retries;

	/* Setup GPIO pins.
	 */
	*INCA_IP_AUTO_MDIX_LAN_PORTS_DIR    |= (1 << INCA_IP_AUTO_MDIX_LAN_GPIO_PIN_RXTX);
	*INCA_IP_AUTO_MDIX_LAN_PORTS_ALTSEL |= (1 << INCA_IP_AUTO_MDIX_LAN_GPIO_PIN_RXTX);

#if 0
	/* Wait for signal.
	 */
	retries = WAIT_SIGNAL_RETRIES;
	while (--retries) {
		SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC,
				(0x1 << 31) |	/* RA		*/
				(0x0 << 30) |	/* Read		*/
				(0x6 << 21) |	/* LAN		*/
				(17  << 16));	/* PHY_MCSR	*/
		do {
			SW_READ_REG(INCA_IP_Switch_MDIO_ACC, phyReg1);
		} while (phyReg1 & (1 << 31));

		if (phyReg1 & (1 << 1)) {
			/* Signal detected */
			break;
		}
	}

	if (!retries)
		goto Fail;
#endif

	/* Set MDI mode.
	 */
	*INCA_IP_AUTO_MDIX_LAN_PORTS_OUT &= ~(1 << INCA_IP_AUTO_MDIX_LAN_GPIO_PIN_RXTX);
	mdi_flag = 1;

	/* Wait for link.
	 */
	retries = WAIT_LINK_RETRIES;
	while (--retries) {
		udelay(LINK_RETRY_DELAY * 1000);
		SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC,
				(0x1 << 31) |	/* RA		*/
				(0x0 << 30) |	/* Read		*/
				(0x6 << 21) |	/* LAN		*/
				(1   << 16));	/* PHY_BSR	*/
		do {
			SW_READ_REG(INCA_IP_Switch_MDIO_ACC, phyReg1);
		} while (phyReg1 & (1 << 31));

		if (phyReg1 & (1 << 2)) {
			/* Link is up */
			break;
		} else if (mdi_flag) {
			/* Set MDIX mode */
			*INCA_IP_AUTO_MDIX_LAN_PORTS_OUT |= (1 << INCA_IP_AUTO_MDIX_LAN_GPIO_PIN_RXTX);
			mdi_flag = 0;
		} else {
			/* Set MDI mode */
			*INCA_IP_AUTO_MDIX_LAN_PORTS_OUT &= ~(1 << INCA_IP_AUTO_MDIX_LAN_GPIO_PIN_RXTX);
			mdi_flag = 1;
		}
	}

	if (!retries) {
		goto Fail;
	} else {
		SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC,
				(0x1 << 31) |	/* RA		*/
				(0x0 << 30) |	/* Read		*/
				(0x6 << 21) |	/* LAN		*/
				(1   << 16));	/* PHY_BSR	*/
		do {
			SW_READ_REG(INCA_IP_Switch_MDIO_ACC, phyReg1);
		} while (phyReg1 & (1 << 31));

		/* Auto-negotiation / Parallel detection complete
		 */
		if (phyReg1 & (1 << 5)) {
			SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC,
				(0x1 << 31) |	/* RA		*/
				(0x0 << 30) |	/* Read		*/
				(0x6 << 21) |	/* LAN		*/
				(31  << 16));	/* PHY_SCSR	*/
			do {
				SW_READ_REG(INCA_IP_Switch_MDIO_ACC, phyReg31);
			} while (phyReg31 & (1 << 31));

			switch ((phyReg31 >> 2) & 0x7) {
			case INCA_SWITCH_PHY_SPEED_10H:
				/* 10Base-T Half-duplex */
				regEphy = 0;
				break;
			case INCA_SWITCH_PHY_SPEED_10F:
				/* 10Base-T Full-duplex */
				regEphy = INCA_IP_Switch_EPHY_DL;
				break;
			case INCA_SWITCH_PHY_SPEED_100H:
				/* 100Base-TX Half-duplex */
				regEphy = INCA_IP_Switch_EPHY_SL;
				break;
			case INCA_SWITCH_PHY_SPEED_100F:
				/* 100Base-TX Full-duplex */
				regEphy = INCA_IP_Switch_EPHY_SL | INCA_IP_Switch_EPHY_DL;
				break;
			}

			/* In case of Auto-negotiation,
			 * update the negotiated PAUSE support status
			 */
			if (phyReg1 & (1 << 3)) {
				SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC,
					(0x1 << 31) |	/* RA		*/
					(0x0 << 30) |	/* Read		*/
					(0x6 << 21) |	/* LAN		*/
					(6   << 16));	/* PHY_ANER	*/
				do {
					SW_READ_REG(INCA_IP_Switch_MDIO_ACC, phyReg6);
				} while (phyReg6 & (1 << 31));

				/* We are Autoneg-able.
				 * Is Link partner also able to autoneg?
				 */
				if (phyReg6 & (1 << 0)) {
					SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC,
						(0x1 << 31) |	/* RA		*/
						(0x0 << 30) |	/* Read		*/
						(0x6 << 21) |	/* LAN		*/
						(4   << 16));	/* PHY_ANAR	*/
					do {
						SW_READ_REG(INCA_IP_Switch_MDIO_ACC, phyReg4);
					} while (phyReg4 & (1 << 31));

					/* We advertise PAUSE capab.
					 * Does link partner also advertise it?
					 */
					if (phyReg4 & (1 << 10)) {
						SW_WRITE_REG(INCA_IP_Switch_MDIO_ACC,
							(0x1 << 31) |	/* RA		*/
							(0x0 << 30) |	/* Read		*/
							(0x6 << 21) |	/* LAN		*/
							(5   << 16));	/* PHY_ANLPAR	*/
						do {
							SW_READ_REG(INCA_IP_Switch_MDIO_ACC, phyReg5);
						} while (phyReg5 & (1 << 31));

						/* Link partner is PAUSE capab.
						 */
						if (phyReg5 & (1 << 10)) {
							regEphy |= INCA_IP_Switch_EPHY_PL;
						}
					}
				}

			}

			/* Link is up */
			regEphy |= INCA_IP_Switch_EPHY_LL;

			SW_WRITE_REG(INCA_IP_Switch_EPHY, regEphy);
		}
	}

	return 0;

Fail:
	printf("No Link on LAN port\n");
	return -1;
}
#endif /* CONFIG_INCA_IP_SWITCH_AMDIX */

#endif

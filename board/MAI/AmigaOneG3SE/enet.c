/*
 * (C) Copyright 2002
 * Adam Kowalczyk, ACK Software Controls Inc. akowalczyk@cogeco.ca
 *
 * Some portions taken from 3c59x.c Written 1996-1999 by Donald Becker.
 *
 * Outline of the program based on eepro100.c which is
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <pci.h>

#include "articiaS.h"
#include "memio.h"

/* 3Com Ethernet PCI definitions*/

/* #define PCI_VENDOR_ID_3COM		0x10B7 */
#define PCI_DEVICE_ID_3COM_3C905C	0x9200

/* 3Com Commands, top 5 bits are command and bottom 11 bits are parameters */

#define TotalReset 		(0<<11)
#define SelectWindow 		(1<<11)
#define StartCoax 		(2<<11)
#define RxDisable 		(3<<11)
#define RxEnable 		(4<<11)
#define RxReset 		(5<<11)
#define UpStall 		(6<<11)
#define UpUnstall 		(6<<11)+1
#define DownStall 		(6<<11)+2
#define DownUnstall 		(6<<11)+3
#define RxDiscard 		(8<<11)
#define TxEnable 		(9<<11)
#define TxDisable 		(10<<11)
#define TxReset 		(11<<11)
#define FakeIntr 		(12<<11)
#define AckIntr 		(13<<11)
#define SetIntrEnb 		(14<<11)
#define SetStatusEnb 		(15<<11)
#define SetRxFilter 		(16<<11)
#define SetRxThreshold 		(17<<11)
#define SetTxThreshold 		(18<<11)
#define SetTxStart 		(19<<11)
#define StartDMAUp 		(20<<11)
#define StartDMADown 		(20<<11)+1
#define StatsEnable		(21<<11)
#define StatsDisable		(22<<11)
#define StopCoax 		(23<<11)
#define SetFilterBit 		(25<<11)

/* The SetRxFilter command accepts the following classes */

#define RxStation 		1
#define RxMulticast		2
#define RxBroadcast		4
#define RxProm 			8

/* 3Com status word defnitions */

#define IntLatch 		0x0001
#define HostError 		0x0002
#define TxComplete 		0x0004
#define TxAvailable 		0x0008
#define RxComplete 		0x0010
#define RxEarly 		0x0020
#define IntReq 			0x0040
#define StatsFull 		0x0080
#define DMADone 		(1<<8)
#define DownComplete 		(1<<9)
#define UpComplete 		(1<<10)
#define DMAInProgress 		(1<<11)			/* DMA controller is still busy.*/
#define CmdInProgress 		(1<<12)           	/* EL3_CMD is still busy.*/

/* Polling Registers */

#define DnPoll			0x2d
#define UpPoll			0x3d

/* Register window 0 offets */

#define Wn0EepromCmd 		10	          	/* Window 0: EEPROM command register. */
#define Wn0EepromData 		12             		/* Window 0: EEPROM results register. */
#define IntrStatus		0x0E	                /* Valid in all windows. */

/* Register window 0 EEPROM bits */

#define EEPROM_Read 		0x80
#define EEPROM_WRITE 		0x40
#define EEPROM_ERASE 		0xC0
#define EEPROM_EWENB 		0x30            	/* Enable erasing/writing for 10 msec. */
#define EEPROM_EWDIS 		0x00            	/* Disable EWENB before 10 msec timeout. */

/* EEPROM locations. */

#define PhysAddr01		0
#define PhysAddr23		1
#define PhysAddr45		2
#define ModelID			3
#define EtherLink3ID		7
#define IFXcvrIO		8
#define IRQLine			9
#define NodeAddr01		10
#define NodeAddr23		11
#define NodeAddr45		12
#define DriverTune		13
#define Checksum		15

/* Register window 1 offsets, the window used in normal operation */

#define TX_FIFO 		0x10
#define RX_FIFO 		0x10
#define RxErrors 		0x14
#define RxStatus 		0x18
#define Timer			0x1A
#define TxStatus 		0x1B
#define TxFree 			0x1C	 		/* Remaining free bytes in Tx buffer. */

/* Register Window 2 */

#define Wn2_ResetOptions	12

/* Register Window 3: MAC/config bits */

#define Wn3_Config		0			/* Internal Configuration */
#define Wn3_MAC_Ctrl		6
#define Wn3_Options		8

#define BFEXT(value, offset, bitcount)  					\
	((((unsigned long)(value)) >> (offset)) & ((1 << (bitcount)) - 1))

#define BFINS(lhs, rhs, offset, bitcount)                                       \
	(((lhs) & ~((((1 << (bitcount)) - 1)) << (offset))) |   		\
	(((rhs) & ((1 << (bitcount)) - 1)) << (offset)))

#define RAM_SIZE(v)             BFEXT(v, 0, 3)
#define RAM_WIDTH(v)    	BFEXT(v, 3, 1)
#define RAM_SPEED(v)    	BFEXT(v, 4, 2)
#define ROM_SIZE(v)             BFEXT(v, 6, 2)
#define RAM_SPLIT(v)    	BFEXT(v, 16, 2)
#define XCVR(v)                 BFEXT(v, 20, 4)
#define AUTOSELECT(v)   	BFEXT(v, 24, 1)

/* Register Window 4: Xcvr/media bits */

#define Wn4_FIFODiag 		4
#define Wn4_NetDiag 		6
#define Wn4_PhysicalMgmt	8
#define Wn4_Media 		10

#define Media_SQE 		0x0008     		/* Enable SQE error counting for AUI. */
#define Media_10TP 		0x00C0			/* Enable link beat and jabber for 10baseT. */
#define Media_Lnk 		0x0080			/* Enable just link beat for 100TX/100FX. */
#define Media_LnkBeat 		0x0800

/* Register Window 7: Bus Master control */

#define Wn7_MasterAddr 		0
#define Wn7_MasterLen 		6
#define Wn7_MasterStatus 	12

/* Boomerang bus master control registers. */

#define PktStatus 		0x20
#define DownListPtr		0x24
#define FragAddr 		0x28
#define FragLen 		0x2c
#define TxFreeThreshold 	0x2f
#define UpPktStatus 		0x30
#define UpListPtr 		0x38

/* The Rx and Tx descriptor lists. */

#define LAST_FRAG       0x80000000                      /* Last Addr/Len pair in descriptor. */
#define DN_COMPLETE     0x00010000                      /* This packet has been downloaded */

struct rx_desc_3com {
	u32 next;                                       /* Last entry points to 0	   	*/
	u32 status;					/* FSH -> Frame Start Header 		*/
	u32 addr;                                       /* Up to 63 addr/len pairs possible 	*/
	u32 length;                                     /* Set LAST_FRAG to indicate last pair	*/
};

/* Values for the Rx status entry. */

#define RxDComplete		0x00008000
#define RxDError		0x4000
#define IPChksumErr		(1<<25)
#define TCPChksumErr		(1<<26)
#define UDPChksumErr		(1<<27)
#define IPChksumValid		(1<<29)
#define TCPChksumValid		(1<<30)
#define UDPChksumValid		(1<<31)

struct tx_desc_3com {
	u32 next;                                       /* Last entry points to 0		*/
	u32 status;                                     /* bits 0:12 length, others see below	*/
	u32 addr;
	u32 length;
};

/* Values for the Tx status entry. */

#define CRCDisable		0x2000
#define TxDComplete		0x8000
#define AddIPChksum		0x02000000
#define AddTCPChksum		0x04000000
#define AddUDPChksum		0x08000000
#define TxIntrUploaded		0x80000000              /* IRQ when in FIFO, but maybe not sent. */

/* XCVR Types */

#define XCVR_10baseT		0
#define XCVR_AUI		1
#define XCVR_10baseTOnly	2
#define XCVR_10base2		3
#define XCVR_100baseTx		4
#define XCVR_100baseFx		5
#define XCVR_MII		6
#define XCVR_NWAY		8
#define XCVR_ExtMII		9
#define XCVR_Default		10			/* I don't think this is correct -> should have been 0x10 if Auto Negotiate */

struct descriptor {			    		/* A generic descriptor. */
	u32 next;                                       /* Last entry points to 0	   	*/
	u32 status;					/* FSH -> Frame Start Header 		*/
	u32 addr;                                       /* Up to 63 addr/len pairs possible 	*/
	u32 length;                                     /* Set LAST_FRAG to indicate last pair	*/
};

/* Misc. definitions */

#define NUM_RX_DESC 		PKTBUFSRX * 10
#define NUM_TX_DESC 		1            /* Number of TX descriptors   */

#define TOUT_LOOP		1000000

#define ETH_ALEN		6

#define EL3WINDOW(dev, win_num) ETH_OUTW(dev, SelectWindow + (win_num), EL3_CMD)
#define EL3_CMD 0x0e
#define EL3_STATUS 0x0e


#undef ETH_DEBUG

#ifdef ETH_DEBUG
#define PRINTF(fmt,args...)     printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif


static struct rx_desc_3com *rx_ring;		 	/* RX descriptor ring         		*/
static struct tx_desc_3com *tx_ring;		 	/* TX descriptor ring         		*/
static u8 rx_buffer[NUM_RX_DESC][PKTSIZE_ALIGN];	/* storage for the incoming messages 	*/
static int rx_next = 0;                      		/* RX descriptor ring pointer 		*/
static int tx_next = 0;                      		/* TX descriptor ring pointer 		*/
static int tx_threshold;

static void  init_rx_ring(struct eth_device* dev);
static void  purge_tx_ring(struct eth_device* dev);

static void  read_hw_addr(struct eth_device* dev, bd_t * bis);

static int eth_3com_init(struct eth_device* dev, bd_t *bis);
static int eth_3com_send(struct eth_device* dev, volatile void *packet, int length);
static int eth_3com_recv(struct eth_device* dev);
static void eth_3com_halt(struct eth_device* dev);

#define io_to_phys(a)	pci_io_to_phys((pci_dev_t)dev->priv, a)
#define phys_to_io(a)	pci_phys_to_io((pci_dev_t)dev->priv, a)
#define mem_to_phys(a)	pci_mem_to_phys((pci_dev_t)dev->priv, a)
#define phys_to_mem(a)	pci_phys_to_mem((pci_dev_t)dev->priv, a)

static inline int ETH_INL(struct eth_device* dev, u_long addr)
{
    __asm volatile ("eieio");
	return le32_to_cpu(*(volatile u32 *)io_to_phys(addr + dev->iobase));
}

static inline int ETH_INW(struct eth_device* dev, u_long addr)
{
    __asm volatile ("eieio");
	return le16_to_cpu(*(volatile u16 *)io_to_phys(addr + dev->iobase));
}

static inline int ETH_INB(struct eth_device* dev, u_long addr)
{
    __asm volatile ("eieio");
	return *(volatile u8 *)io_to_phys(addr + dev->iobase);
}

static inline void ETH_OUTB(struct eth_device* dev, int command, u_long addr)
{
	*(volatile u8 *)io_to_phys(addr + dev->iobase) = command;
    __asm volatile ("eieio");
}

static inline void ETH_OUTW(struct eth_device* dev, int command, u_long addr)
{
	*(volatile u16 *)io_to_phys(addr + dev->iobase) = cpu_to_le16(command);
    __asm volatile ("eieio");
}

static inline void ETH_OUTL(struct eth_device* dev, int command, u_long addr)
{
	*(volatile u32 *)io_to_phys(addr + dev->iobase) = cpu_to_le32(command);
    __asm volatile ("eieio");
}

static inline int ETH_STATUS(struct eth_device* dev)
{
    __asm volatile ("eieio");
	return le16_to_cpu(*(volatile u16 *)io_to_phys(EL3_STATUS + dev->iobase));
}

static inline void ETH_CMD(struct eth_device* dev, int command)
{
	*(volatile u16 *)io_to_phys(EL3_CMD + dev->iobase) = cpu_to_le16(command);
    __asm volatile ("eieio");
}

/* Command register is always in the same spot in all the register windows */
/* This function issues a command and waits for it so complete by checking the CmdInProgress bit */

static int issue_and_wait(struct eth_device* dev, int command)
{

	int i, status;

	ETH_CMD(dev, command);
	for (i = 0; i < 2000; i++) {
		status = ETH_STATUS(dev);
		/*printf ("Issue: status 0x%4x.\n", status); */
		if (!(status & CmdInProgress))
			return 1;
	}

	/* OK, that didn't work.  Do it the slow way.  One second */
	for (i = 0; i < 100000; i++) {
		status = ETH_STATUS(dev);
		/*printf ("Issue: status 0x%4x.\n", status); */
			return 1;
		udelay(10);
	}
	PRINTF("Ethernet command: 0x%4x did not complete! Status: 0x%4x\n", command, ETH_STATUS(dev) );
	return 0;
}

/* Determine network media type and set up 3com accordingly           */
/* I think I'm going to start with something known first like 10baseT */

static int auto_negotiate(struct eth_device* dev)
{
    int i;

    EL3WINDOW(dev, 1);

    /* Wait for Auto negotiation to complete */
    for (i = 0; i <= 1000; i++)
    {
	if (ETH_INW(dev, 2) & 0x04)
	    break;
	udelay(100);

	if (i == 1000)
	{
	    PRINTF("Error: Auto negotiation failed\n");
	    return 0;
	}
    }


    return 1;
}

void eth_interrupt(struct eth_device *dev)
{
    u16 status = ETH_STATUS(dev);

    printf("eth0: status = 0x%04x\n", status);

    if (!(status & IntLatch))
	return;

    if (status & (1<<6))
    {
	ETH_CMD(dev, AckIntr | (1<<6));
	printf("Acknowledged Interrupt command\n");
    }

    if (status & DownComplete)
    {
	ETH_CMD(dev, AckIntr | DownComplete);
	printf("Acknowledged DownComplete\n");
    }

    if (status & UpComplete)
    {
	ETH_CMD(dev, AckIntr | UpComplete);
	printf("Acknowledged UpComplete\n");
    }

    ETH_CMD(dev, AckIntr | IntLatch);
    printf("Acknowledged IntLatch\n");
}

int eth_3com_initialize(bd_t *bis)
{
	u32 eth_iobase = 0, status;
	int card_number = 0, ret;
	struct eth_device* dev;
	pci_dev_t devno;
	char *s;

	s = getenv("3com_base");

	/* Find ethernet controller on the PCI bus */

	if ((devno = pci_find_device(PCI_VENDOR_ID_3COM, PCI_DEVICE_ID_3COM_3C905C, 0)) < 0)
	{
		PRINTF("Error: Cannot find the ethernet device on the PCI bus\n");
		goto Done;
	}

	if (s)
	{
	    unsigned long base = atoi(s);
	    pci_write_config_dword(devno, PCI_BASE_ADDRESS_0, base | 0x01);
	}

	ret = pci_read_config_dword(devno, PCI_BASE_ADDRESS_0, &eth_iobase);
	eth_iobase &= ~0xf;

	PRINTF("eth: 3Com Found at Address: 0x%x\n", eth_iobase);

	pci_write_config_dword(devno, PCI_COMMAND, PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

	 /* Check if I/O accesses and Bus Mastering are enabled */

	ret = pci_read_config_dword(devno, PCI_COMMAND, &status);

	if (!(status & PCI_COMMAND_IO))
	{
		printf("Error: Cannot enable IO access.\n");
		goto Done;
	}

	if (!(status & PCI_COMMAND_MEMORY))
	{
		printf("Error: Cannot enable MEMORY access.\n");
		goto Done;
	}

	if (!(status & PCI_COMMAND_MASTER))
	{
		printf("Error: Cannot enable Bus Mastering.\n");
		goto Done;
	}

	dev = (struct eth_device*) malloc(sizeof(*dev)); /*struct eth_device)); */

	sprintf(dev->name, "3Com 3c920c#%d", card_number);
	dev->iobase = eth_iobase;
	dev->priv   = (void*) devno;
	dev->init   = eth_3com_init;
	dev->halt   = eth_3com_halt;
	dev->send   = eth_3com_send;
	dev->recv   = eth_3com_recv;

	eth_register(dev);

/* 	{ */
/* 	    char interrupt; */
/* 	    devno = pci_find_device(PCI_VENDOR_ID_3COM, PCI_DEVICE_ID_3COM_3C905C, 0); */
/* 	    pci_read_config_byte(devno, PCI_INTERRUPT_LINE, &interrupt); */

/* 	    printf("Installing eth0 interrupt handler to %d\n", interrupt); */
/* 	    irq_install_handler(interrupt, eth_interrupt, dev); */
/* 	} */

	card_number++;

	/* Set the latency timer for value */
	s = getenv("3com_latency");
	if (s)
	{
	    ret = pci_write_config_byte(devno, PCI_LATENCY_TIMER, (unsigned char)atoi(s));
	}
	else ret = pci_write_config_byte(devno, PCI_LATENCY_TIMER, 0x0a);

	read_hw_addr(dev, bis); 				/* get the MAC address from Window 2*/

	/* Reset the ethernet controller */

	PRINTF ("Issuing reset command....\n");
	if (!issue_and_wait(dev, TotalReset))
	{
		printf("Error: Cannot reset ethernet controller.\n");
		goto Done;
	}
	else
		PRINTF ("Ethernet controller reset.\n");

	/* allocate memory for rx and tx rings */

	if(!(rx_ring = memalign(sizeof(struct rx_desc_3com) * NUM_RX_DESC, 16)))
	{
		PRINTF ("Cannot allocate memory for RX_RING.....\n");
		goto Done;
	}

	if (!(tx_ring = memalign(sizeof(struct tx_desc_3com) * NUM_TX_DESC, 16)))
	{
		PRINTF ("Cannot allocate memory for TX_RING.....\n");
		goto Done;
	}

Done:
	return status;
}


static int eth_3com_init(struct eth_device* dev, bd_t *bis)
{
	int i, status = 0;
	int tx_cur, loop;
	u16 status_enable, intr_enable;
	struct descriptor *ias_cmd;

	/* Determine what type of network the machine is connected to	*/
	/* presently drops the connect to 10Mbps			*/

	if (!auto_negotiate(dev))
	{
		printf("Error: Cannot determine network media.\n");
		goto Done;
	}

	issue_and_wait(dev, TxReset);
	issue_and_wait(dev, RxReset|0x04);

	/* Switch to register set 7 for normal use. */
	EL3WINDOW(dev, 7);

	/* Initialize Rx and Tx rings */

	init_rx_ring(dev);
	purge_tx_ring(dev);

	ETH_CMD(dev, SetRxFilter | RxStation | RxBroadcast | RxProm);

	issue_and_wait(dev,SetTxStart|0x07ff);

	/* Below sets which indication bits to be seen. */

	status_enable = SetStatusEnb | HostError | DownComplete | UpComplete | (1<<6);
	ETH_CMD(dev, status_enable);

	/* Below sets no bits are to cause an interrupt since this is just polling */

	intr_enable   = SetIntrEnb;
/*	intr_enable = SetIntrEnb | (1<<9) | (1<<10) | (1<<6); */
	ETH_CMD(dev, intr_enable);
	ETH_OUTB(dev, 127, UpPoll);

	/* Ack all pending events, and set active indicator mask */

	ETH_CMD(dev, AckIntr | IntLatch | TxAvailable | RxEarly | IntReq);
	ETH_CMD(dev, intr_enable);

	/* Tell the adapter where the RX ring is located */

	issue_and_wait(dev,UpStall);				/* Stall and set the UplistPtr 		*/
	ETH_OUTL(dev, (u32)&rx_ring[rx_next], UpListPtr);
	ETH_CMD(dev, RxEnable); 				/* Enable the receiver. 		*/
	issue_and_wait(dev,UpUnstall);

	/* Send the Individual Address Setup frame */

	tx_cur       = tx_next;
	tx_next      = ((tx_next+1) % NUM_TX_DESC);

	ias_cmd             = (struct descriptor *)&tx_ring[tx_cur];
	ias_cmd->status     = cpu_to_le32(1<<31);		/* set DnIndicate bit. 			*/
	ias_cmd->next       = 0;
	ias_cmd->addr       = cpu_to_le32((u32)&bis->bi_enetaddr[0]);
	ias_cmd->length     = cpu_to_le32(6 | LAST_FRAG);

	/* Tell the adapter where the TX ring is located */

	ETH_CMD(dev, TxEnable); 				/* Enable transmitter. 			*/
	issue_and_wait(dev, DownStall);				/* Stall and set the DownListPtr. 	*/
	ETH_OUTL(dev, (u32)&tx_ring[tx_cur], DownListPtr);
	issue_and_wait(dev, DownUnstall);
	for (i=0; !(ETH_STATUS(dev) & DownComplete); i++)
	{
		if (i >= TOUT_LOOP)
		{
			PRINTF("TX Ring status (Init):  0x%4x\n", le32_to_cpu(tx_ring[tx_cur].status));
			PRINTF("ETH_STATUS: 0x%x\n", ETH_STATUS(dev));
			goto Done;
		}
	}
	if (ETH_STATUS(dev) & DownComplete)			/* If DownLoad Complete ACK the bit 	*/
	{
		ETH_CMD(dev, AckIntr | DownComplete);		/* acknowledge the indication bit	*/
		issue_and_wait(dev, DownStall);			/* stall and clear DownListPtr 		*/
		ETH_OUTL(dev, 0, DownListPtr);
		issue_and_wait(dev, DownUnstall);
	}
	status = 1;

Done:
	return status;
}

int eth_3com_send(struct eth_device* dev, volatile void *packet, int length)
{
	int i, status = 0;
	int tx_cur;

	if (length <= 0)
	{
		PRINTF("eth: bad packet size: %d\n", length);
		goto Done;
	}

	tx_cur  = tx_next;
	tx_next = (tx_next+1) % NUM_TX_DESC;

	tx_ring[tx_cur].status  = cpu_to_le32(1<<31);		/* set DnIndicate bit 			*/
	tx_ring[tx_cur].next    = 0;
	tx_ring[tx_cur].addr    = cpu_to_le32(((u32) packet));
	tx_ring[tx_cur].length  = cpu_to_le32(length | LAST_FRAG);

	/* Send the packet */

	issue_and_wait(dev, DownStall);				/* stall and set the DownListPtr 	*/
	ETH_OUTL(dev, (u32) &tx_ring[tx_cur], DownListPtr);
	issue_and_wait(dev, DownUnstall);

	for (i=0; !(ETH_STATUS(dev) & DownComplete); i++)
	{
		if (i >= TOUT_LOOP)
		{
			PRINTF("TX Ring status (send): 0x%4x\n", le32_to_cpu(tx_ring[tx_cur].status));
			goto Done;
		}
	}
	if (ETH_STATUS(dev) & DownComplete)			/* If DownLoad Complete ACK the bit 	*/
	{
		ETH_CMD(dev, AckIntr | DownComplete);		/* acknowledge the indication bit	*/
		issue_and_wait(dev, DownStall);			/* stall and clear DownListPtr 		*/
		ETH_OUTL(dev, 0, DownListPtr);
		issue_and_wait(dev, DownUnstall);
	}
	status=1;
 Done:
	return status;
}

void PrintPacket (uchar *packet, int length)
{
int loop;
uchar *ptr;

	printf ("Printing packet of length %x.\n\n", length);
	ptr = packet;
	for (loop = 1; loop <= length; loop++)
	{
		printf ("%2x ", *ptr++);
		if ((loop % 40)== 0)
			printf ("\n");
	}
}

int eth_3com_recv(struct eth_device* dev)
{
	u16 stat = 0;
	u32 status;
	int rx_prev, length = 0;

	while (!(ETH_STATUS(dev) & UpComplete))			/* wait on receipt of packet 	*/
		;

	status = le32_to_cpu(rx_ring[rx_next].status);		/* packet status		*/

	while (status & (1<<15))
	{
		/* A packet has been received */

		if (status & (1<<15))
		{
			/* A valid frame received  */

			length = le32_to_cpu(rx_ring[rx_next].status) & 0x1fff;		/* length is in bits 0 - 12 	*/

			/* Pass the packet up to the protocol layers */

			NetReceive((uchar *)le32_to_cpu(rx_ring[rx_next].addr), length);
			rx_ring[rx_next].status = 0;					/* clear the status word 	*/
			ETH_CMD(dev, AckIntr | UpComplete);
			issue_and_wait(dev, UpUnstall);
		}
		else
		if (stat & HostError)
		{
			/* There was an error */

			printf("Rx error status:  0x%4x\n", stat);
			init_rx_ring(dev);
			goto Done;
		}

		rx_prev = rx_next;
		rx_next = (rx_next + 1) % NUM_RX_DESC;
		stat = ETH_STATUS(dev);					/* register status 	*/
		status = le32_to_cpu(rx_ring[rx_next].status);		/* packet status 	*/
	}

Done:
	return length;
}

void eth_3com_halt(struct eth_device* dev)
{
	if (!(dev->iobase))
	{
		goto Done;
	}

	issue_and_wait(dev, DownStall);		/* shut down transmit and receive */
	issue_and_wait(dev, UpStall);
	issue_and_wait(dev, RxDisable);
	issue_and_wait(dev, TxDisable);

/*	free(tx_ring);				/###* release memory allocated to the DPD and UPD rings */
/*	free(rx_ring); */

Done:
	return;
}

static void init_rx_ring(struct eth_device* dev)
{
	int i;

	PRINTF("Initializing rx_ring. rx_buffer = %p\n", rx_buffer);
	issue_and_wait(dev, UpStall);

	for (i = 0; i < NUM_RX_DESC; i++)
	{
		rx_ring[i].next    = cpu_to_le32(((u32) &rx_ring[(i+1) % NUM_RX_DESC]));
		rx_ring[i].status  = 0;
		rx_ring[i].addr    = cpu_to_le32(((u32) &rx_buffer[i][0]));
		rx_ring[i].length  = cpu_to_le32(PKTSIZE_ALIGN | LAST_FRAG);
	}
	rx_next = 0;
}

static void purge_tx_ring(struct eth_device* dev)
{
	int i;

	PRINTF("Purging tx_ring.\n");

	tx_next      = 0;

	for (i = 0; i < NUM_TX_DESC; i++)
	{
		tx_ring[i].next    = 0;
		tx_ring[i].status  = 0;
		tx_ring[i].addr    = 0;
		tx_ring[i].length  = 0;
	}
}

static void read_hw_addr(struct eth_device* dev, bd_t *bis)
{
	u8 hw_addr[ETH_ALEN];
	unsigned int eeprom[0x40];
	unsigned int checksum = 0;
	int i, j, timer;

	/* Read the station address from the EEPROM. */

	EL3WINDOW(dev, 0);
	for (i = 0; i < 0x40; i++)
	{
		ETH_OUTW(dev, EEPROM_Read + i, Wn0EepromCmd);
		/* Pause for at least 162 us. for the read to take place. */
		for (timer = 10; timer >= 0; timer--)
		{
			udelay(162);
			if ((ETH_INW(dev, Wn0EepromCmd) & 0x8000) == 0)
				break;
		}
		eeprom[i] = ETH_INW(dev, Wn0EepromData);
	}

	/* Checksum calculation.  I'm not sure about this part and there seems to be a bug on the 3com side of things */

	for (i = 0; i < 0x21; i++)
		checksum  ^= eeprom[i];
	checksum = (checksum ^ (checksum >> 8)) & 0xff;

	if (checksum != 0xbb)
		printf(" *** INVALID EEPROM CHECKSUM %4.4x *** \n", checksum);

	for (i = 0, j = 0; i < 3; i++)
	{
		hw_addr[j++] = (u8)((eeprom[i+10] >> 8) & 0xff);
		hw_addr[j++] = (u8)(eeprom[i+10] & 0xff);
	}

	/*  MAC Address is in window 2, write value from EEPROM to window 2 */

	EL3WINDOW(dev, 2);
	for (i = 0; i < 6; i++)
		ETH_OUTB(dev, hw_addr[i], i);

	for (j = 0; j < ETH_ALEN; j+=2)
	{
		hw_addr[j]   = (u8)(ETH_INW(dev, j) & 0xff);
		hw_addr[j+1] = (u8)((ETH_INW(dev, j) >> 8) & 0xff);
	}

	for (i=0;i<ETH_ALEN;i++)
	{
		if (hw_addr[i] != bis->bi_enetaddr[i])
		{
/* 			printf("Warning: HW address don't match:\n"); */
/* 			printf("Address in 3Com Window 2 is         " */
/* 			       "%02X:%02X:%02X:%02X:%02X:%02X\n", */
/* 			       hw_addr[0], hw_addr[1], hw_addr[2], */
/* 			       hw_addr[3], hw_addr[4], hw_addr[5]); */
/* 			printf("Address used by U-Boot is " */
/* 			       "%02X:%02X:%02X:%02X:%02X:%02X\n", */
/* 			       bis->bi_enetaddr[0], bis->bi_enetaddr[1],  */
/* 			       bis->bi_enetaddr[2], bis->bi_enetaddr[3],  */
/* 			       bis->bi_enetaddr[4], bis->bi_enetaddr[5]); */
/* 			goto Done; */
		    char buffer[256];
		    if (bis->bi_enetaddr[0] == 0 && bis->bi_enetaddr[1] == 0 &&
			bis->bi_enetaddr[2] == 0 && bis->bi_enetaddr[3] == 0 &&
			bis->bi_enetaddr[4] == 0 && bis->bi_enetaddr[5] == 0)
		    {

			sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
				hw_addr[0], hw_addr[1], hw_addr[2],
				hw_addr[3], hw_addr[4], hw_addr[5]);
			setenv("ethaddr", buffer);
		    }
		}
	}

	for(i=0; i<ETH_ALEN; i++) dev->enetaddr[i] = hw_addr[i];

Done:
	return;
}

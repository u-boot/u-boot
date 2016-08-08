/* Only eth0 supported for now
 *
 * (C) Copyright 2003
 * Thomas.Lange@corelatus.se
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <config.h>

#if defined(CONFIG_SYS_DISCOVER_PHY)
#error "PHY not supported yet"
/* We just assume that we are running 100FD for now */
/* We all use switches, right? ;-) */
#endif

/* I assume ethernet behaves like au1000 */

#ifdef CONFIG_SOC_AU1000
/* Base address differ between cpu:s */
#define ETH0_BASE AU1000_ETH0_BASE
#define MAC0_ENABLE AU1000_MAC0_ENABLE
#else
#ifdef CONFIG_SOC_AU1100
#define ETH0_BASE AU1100_ETH0_BASE
#define MAC0_ENABLE AU1100_MAC0_ENABLE
#else
#ifdef CONFIG_SOC_AU1500
#define ETH0_BASE AU1500_ETH0_BASE
#define MAC0_ENABLE AU1500_MAC0_ENABLE
#else
#ifdef CONFIG_SOC_AU1550
#define ETH0_BASE AU1550_ETH0_BASE
#define MAC0_ENABLE AU1550_MAC0_ENABLE
#else
#error "No valid cpu set"
#endif
#endif
#endif
#endif

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <asm/io.h>
#include <mach/au1x00.h>

#if defined(CONFIG_CMD_MII)
#include <miiphy.h>
#endif

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH  1520
#define PKT_MAXBUF_SIZE		1518

static char txbuf[DBUF_LENGTH];

static int next_tx;
static int next_rx;

/* 4 rx and 4 tx fifos */
#define NO_OF_FIFOS 4

typedef struct{
	u32 status;
	u32 addr;
	u32 len; /* Only used for tx */
	u32 not_used;
} mac_fifo_t;

mac_fifo_t mac_fifo[NO_OF_FIFOS];

#define MAX_WAIT 1000

#if defined(CONFIG_CMD_MII)
int au1x00_miiphy_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	unsigned short value = 0;
	volatile u32 *mii_control_reg = (volatile u32*)(ETH0_BASE+MAC_MII_CNTRL);
	volatile u32 *mii_data_reg = (volatile u32*)(ETH0_BASE+MAC_MII_DATA);
	u32 mii_control;
	unsigned int timedout = 20;

	while (*mii_control_reg & MAC_MII_BUSY) {
		udelay(1000);
		if (--timedout == 0) {
			printf("au1x00_eth: miiphy_read busy timeout!!\n");
			return -1;
		}
	}

	mii_control = MAC_SET_MII_SELECT_REG(reg) |
		MAC_SET_MII_SELECT_PHY(addr) | MAC_MII_READ;

	*mii_control_reg = mii_control;

	timedout = 20;
	while (*mii_control_reg & MAC_MII_BUSY) {
		udelay(1000);
		if (--timedout == 0) {
			printf("au1x00_eth: miiphy_read busy timeout!!\n");
			return -1;
		}
	}
	value = *mii_data_reg;
	return value;
}

int au1x00_miiphy_write(struct mii_dev *bus, int addr, int devad, int reg,
			u16 value)
{
	volatile u32 *mii_control_reg = (volatile u32*)(ETH0_BASE+MAC_MII_CNTRL);
	volatile u32 *mii_data_reg = (volatile u32*)(ETH0_BASE+MAC_MII_DATA);
	u32 mii_control;
	unsigned int timedout = 20;

	while (*mii_control_reg & MAC_MII_BUSY) {
		udelay(1000);
		if (--timedout == 0) {
			printf("au1x00_eth: miiphy_write busy timeout!!\n");
			return -1;
		}
	}

	mii_control = MAC_SET_MII_SELECT_REG(reg) |
		MAC_SET_MII_SELECT_PHY(addr) | MAC_MII_WRITE;

	*mii_data_reg = value;
	*mii_control_reg = mii_control;
	return 0;
}
#endif

static int au1x00_send(struct eth_device *dev, void *packet, int length)
{
	volatile mac_fifo_t *fifo_tx =
		(volatile mac_fifo_t*)(MAC0_TX_DMA_ADDR+MAC_TX_BUFF0_STATUS);
	int i;
	int res;

	/* tx fifo should always be idle */
	fifo_tx[next_tx].len = length;
	fifo_tx[next_tx].addr = (virt_to_phys(packet))|TX_DMA_ENABLE;
	au_sync();

	udelay(1);
	i=0;
	while(!(fifo_tx[next_tx].addr&TX_T_DONE)){
		if(i>MAX_WAIT){
			printf("TX timeout\n");
			break;
		}
		udelay(1);
		i++;
	}

	/* Clear done bit */
	fifo_tx[next_tx].addr = 0;
	fifo_tx[next_tx].len = 0;
	au_sync();

	res = fifo_tx[next_tx].status;

	next_tx++;
	if(next_tx>=NO_OF_FIFOS){
		next_tx=0;
	}
	return(res);
}

static int au1x00_recv(struct eth_device* dev){
	volatile mac_fifo_t *fifo_rx =
		(volatile mac_fifo_t*)(MAC0_RX_DMA_ADDR+MAC_RX_BUFF0_STATUS);

	int length;
	u32 status;

	for(;;){
		if(!(fifo_rx[next_rx].addr&RX_T_DONE)){
			/* Nothing has been received */
			return(-1);
		}

		status = fifo_rx[next_rx].status;

		length = status&0x3FFF;

		if(status&RX_ERROR){
			printf("Rx error 0x%x\n", status);
		} else {
			/* Pass the packet up to the protocol layers. */
			net_process_received_packet(net_rx_packets[next_rx],
						    length - 4);
		}

		fifo_rx[next_rx].addr =
			(virt_to_phys(net_rx_packets[next_rx])) | RX_DMA_ENABLE;

		next_rx++;
		if(next_rx>=NO_OF_FIFOS){
			next_rx=0;
		}
	} /* for */

	return(0); /* Does anyone use this? */
}

static int au1x00_init(struct eth_device* dev, bd_t * bd){

	volatile u32 *macen = (volatile u32*)MAC0_ENABLE;
	volatile u32 *mac_ctrl = (volatile u32*)(ETH0_BASE+MAC_CONTROL);
	volatile u32 *mac_addr_high = (volatile u32*)(ETH0_BASE+MAC_ADDRESS_HIGH);
	volatile u32 *mac_addr_low = (volatile u32*)(ETH0_BASE+MAC_ADDRESS_LOW);
	volatile u32 *mac_mcast_high = (volatile u32*)(ETH0_BASE+MAC_MCAST_HIGH);
	volatile u32 *mac_mcast_low = (volatile u32*)(ETH0_BASE+MAC_MCAST_LOW);
	volatile mac_fifo_t *fifo_tx =
		(volatile mac_fifo_t*)(MAC0_TX_DMA_ADDR+MAC_TX_BUFF0_STATUS);
	volatile mac_fifo_t *fifo_rx =
		(volatile mac_fifo_t*)(MAC0_RX_DMA_ADDR+MAC_RX_BUFF0_STATUS);
	int i;

	next_tx = TX_GET_DMA_BUFFER(fifo_tx[0].addr);
	next_rx = RX_GET_DMA_BUFFER(fifo_rx[0].addr);

	/* We have to enable clocks before releasing reset */
	*macen = MAC_EN_CLOCK_ENABLE;
	udelay(10);

	/* Enable MAC0 */
	/* We have to release reset before accessing registers */
	*macen = MAC_EN_CLOCK_ENABLE|MAC_EN_RESET0|
		MAC_EN_RESET1|MAC_EN_RESET2;
	udelay(10);

	for(i=0;i<NO_OF_FIFOS;i++){
		fifo_tx[i].len = 0;
		fifo_tx[i].addr = virt_to_phys(&txbuf[0]);
		fifo_rx[i].addr = (virt_to_phys(net_rx_packets[i])) |
			RX_DMA_ENABLE;
	}

	/* Put mac addr in little endian */
#define ea eth_get_ethaddr()
	*mac_addr_high	=	(ea[5] <<  8) | (ea[4]	    ) ;
	*mac_addr_low	=	(ea[3] << 24) | (ea[2] << 16) |
		(ea[1] <<  8) | (ea[0]	    ) ;
#undef ea
	*mac_mcast_low = 0;
	*mac_mcast_high = 0;

	/* Make sure the MAC buffer is in the correct endian mode */
#ifdef __LITTLE_ENDIAN
	*mac_ctrl = MAC_FULL_DUPLEX;
	udelay(1);
	*mac_ctrl = MAC_FULL_DUPLEX|MAC_RX_ENABLE|MAC_TX_ENABLE;
#else
	*mac_ctrl = MAC_BIG_ENDIAN|MAC_FULL_DUPLEX;
	udelay(1);
	*mac_ctrl = MAC_BIG_ENDIAN|MAC_FULL_DUPLEX|MAC_RX_ENABLE|MAC_TX_ENABLE;
#endif

	return(1);
}

static void au1x00_halt(struct eth_device* dev){
	volatile u32 *macen = (volatile u32*)MAC0_ENABLE;

	/* Put MAC0 in reset */
	*macen = 0;
}

int au1x00_enet_initialize(bd_t *bis){
	struct eth_device* dev;

	if ((dev = (struct eth_device*)malloc(sizeof *dev)) == NULL) {
		puts ("malloc failed\n");
		return -1;
	}

	memset(dev, 0, sizeof *dev);

	strcpy(dev->name, "Au1X00 ethernet");
	dev->iobase = 0;
	dev->priv   = 0;
	dev->init   = au1x00_init;
	dev->halt   = au1x00_halt;
	dev->send   = au1x00_send;
	dev->recv   = au1x00_recv;

	eth_register(dev);

#if defined(CONFIG_CMD_MII)
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = au1x00_miiphy_read;
	mdiodev->write = au1x00_miiphy_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
#endif

	return 1;
}

int cpu_eth_init(bd_t *bis)
{
	au1x00_enet_initialize(bis);
	return 0;
}

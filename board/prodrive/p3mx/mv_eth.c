/*
 * (C) Copyright 2003
 * Ingo Assmus <ingo.assmus@keymile.com>
 *
 * based on - Driver for MV64460X ethernet ports
 * Copyright (C) 2002 rabeeh@galileo.co.il
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * mv_eth.c - header file for the polled mode GT ethernet driver
 */
#include <common.h>
#include <net.h>
#include <malloc.h>
#include <miiphy.h>

#include "mv_eth.h"

/* enable Debug outputs */

#undef DEBUG_MV_ETH

#ifdef DEBUG_MV_ETH
#define DEBUG
#define DP(x) x
#else
#define DP(x)
#endif

/* PHY DFCDL Registers */
#define ETH_PHY_DFCDL_CONFIG0_REG	0x2100
#define ETH_PHY_DFCDL_CONFIG1_REG	0x2104
#define ETH_PHY_DFCDL_ADDR_REG		0x2110
#define ETH_PHY_DFCDL_DATA0_REG		0x2114

#define PHY_AUTONEGOTIATE_TIMEOUT	4000	/* 4000 ms autonegotiate timeout */
#define PHY_UPDATE_TIMEOUT		10000

#undef MV64460_CHECKSUM_OFFLOAD
/*************************************************************************
*  The first part is the high level driver of the gigE ethernet ports.	 *
*************************************************************************/

/* Definition for configuring driver */
/* #define UPDATE_STATS_BY_SOFTWARE */
#undef MV64460_RX_QUEUE_FILL_ON_TASK

/* Constants */
#define MAGIC_ETH_RUNNING		8031971
#define MV64460_INTERNAL_SRAM_SIZE	_256K
#define EXTRA_BYTES 32
#define WRAP	   ETH_HLEN + 2 + 4 + 16
#define BUFFER_MTU dev->mtu + WRAP
#define INT_CAUSE_UNMASK_ALL		0x0007ffff
#define INT_CAUSE_UNMASK_ALL_EXT	0x0011ffff
#ifdef MV64460_RX_FILL_ON_TASK
#define INT_CAUSE_MASK_ALL		0x00000000
#define INT_CAUSE_CHECK_BITS		INT_CAUSE_UNMASK_ALL
#define INT_CAUSE_CHECK_BITS_EXT	INT_CAUSE_UNMASK_ALL_EXT
#endif

/* Read/Write to/from MV64460 internal registers */
#define MV_REG_READ(offset) my_le32_to_cpu(* (volatile unsigned int *) (INTERNAL_REG_BASE_ADDR + offset))
#define MV_REG_WRITE(offset,data) *(volatile unsigned int *) (INTERNAL_REG_BASE_ADDR + offset) = my_cpu_to_le32 (data)
#define MV_SET_REG_BITS(regOffset,bits) ((*((volatile unsigned int*)((INTERNAL_REG_BASE_ADDR) + (regOffset)))) |= ((unsigned int)my_cpu_to_le32(bits)))
#define MV_RESET_REG_BITS(regOffset,bits) ((*((volatile unsigned int*)((INTERNAL_REG_BASE_ADDR) + (regOffset)))) &= ~((unsigned int)my_cpu_to_le32(bits)))

#define my_cpu_to_le32(x) my_le32_to_cpu((x))

/* Static function declarations */
static int mv64460_eth_real_open (struct eth_device *eth);
static int mv64460_eth_real_stop (struct eth_device *eth);
static struct net_device_stats *mv64460_eth_get_stats (struct eth_device
						       *dev);
static void eth_port_init_mac_tables (ETH_PORT eth_port_num);
static void mv64460_eth_update_stat (struct eth_device *dev);
bool db64460_eth_start (struct eth_device *eth);
unsigned int eth_read_mib_counter (ETH_PORT eth_port_num,
				   unsigned int mib_offset);
int mv64460_eth_receive (struct eth_device *dev);

int mv64460_eth_xmit (struct eth_device *, volatile void *packet, int length);

int mv_miiphy_read(const char *devname, unsigned char phy_addr,
		   unsigned char phy_reg, unsigned short *value);
int mv_miiphy_write(const char *devname, unsigned char phy_addr,
		    unsigned char phy_reg, unsigned short value);

int phy_setup_aneg (char *devname, unsigned char addr);

#ifndef	 UPDATE_STATS_BY_SOFTWARE
static void mv64460_eth_print_stat (struct eth_device *dev);
#endif

extern unsigned int INTERNAL_REG_BASE_ADDR;

unsigned long my_le32_to_cpu (unsigned long x)
{
	return (((x & 0x000000ffU) << 24) |
		((x & 0x0000ff00U) << 8) |
		((x & 0x00ff0000U) >> 8) | ((x & 0xff000000U) >> 24));
}

/*************************************************
 *Helper functions - used inside the driver only *
 *************************************************/
#ifdef DEBUG_MV_ETH
void print_globals (struct eth_device *dev)
{
	printf ("Ethernet PRINT_Globals-Debug function\n");
	printf ("Base Address for ETH_PORT_INFO:	%08x\n",
		(unsigned int) dev->priv);
	printf ("Base Address for mv64460_eth_priv:	%08x\n",
		(unsigned int) &(((ETH_PORT_INFO *) dev->priv)->
				 port_private));

	printf ("GT Internal Base Address:	%08x\n",
		INTERNAL_REG_BASE_ADDR);
	printf ("Base Address for TX-DESCs:	%08x	Number of allocated Buffers %d\n",
		(unsigned int) ((ETH_PORT_INFO *) dev->priv)->p_tx_desc_area_base[0], MV64460_TX_QUEUE_SIZE);
	printf ("Base Address for RX-DESCs:	%08x	Number of allocated Buffers %d\n",
		(unsigned int) ((ETH_PORT_INFO *) dev->priv)->p_rx_desc_area_base[0], MV64460_RX_QUEUE_SIZE);
	printf ("Base Address for RX-Buffer:	%08x	allocated Bytes %d\n",
		(unsigned int) ((ETH_PORT_INFO *) dev->priv)->
		p_rx_buffer_base[0],
		(MV64460_RX_QUEUE_SIZE * MV64460_RX_BUFFER_SIZE) + 32);
	printf ("Base Address for TX-Buffer:	%08x	allocated Bytes %d\n",
		(unsigned int) ((ETH_PORT_INFO *) dev->priv)->
		p_tx_buffer_base[0],
		(MV64460_TX_QUEUE_SIZE * MV64460_TX_BUFFER_SIZE) + 32);
}
#endif

/**********************************************************************
 * mv64460_eth_print_phy_status
 *
 * Prints gigabit ethenret phy status
 *
 * Input : pointer to ethernet interface network device structure
 * Output : N/A
 **********************************************************************/
void mv64460_eth_print_phy_status (struct eth_device *dev)
{
	struct mv64460_eth_priv *port_private;
	unsigned int port_num;
	ETH_PORT_INFO *ethernet_private = (ETH_PORT_INFO *) dev->priv;
	unsigned int port_status, phy_reg_data;

	port_private =
		(struct mv64460_eth_priv *) ethernet_private->port_private;
	port_num = port_private->port_num;

	/* Check Link status on phy */
	eth_port_read_smi_reg (port_num, 1, &phy_reg_data);
	if (!(phy_reg_data & 0x20)) {
		printf ("Ethernet port changed link status to DOWN\n");
	} else {
		port_status =
			MV_REG_READ (MV64460_ETH_PORT_STATUS_REG (port_num));
		printf ("Ethernet status port %d: Link up", port_num);
		printf (", %s",
			(port_status & BIT2) ? "Full Duplex" : "Half Duplex");
		if (port_status & BIT4)
			printf (", Speed 1 Gbps");
		else
			printf (", %s",
				(port_status & BIT5) ? "Speed 100 Mbps" :
				"Speed 10 Mbps");
		printf ("\n");
	}
}

/**********************************************************************
 * u-boot entry functions for mv64460_eth
 *
 **********************************************************************/
int db64460_eth_probe (struct eth_device *dev)
{
	return ((int) db64460_eth_start (dev));
}

int db64460_eth_poll (struct eth_device *dev)
{
	return mv64460_eth_receive (dev);
}

int db64460_eth_transmit(struct eth_device *dev, void *packet, int length)
{
	mv64460_eth_xmit (dev, packet, length);
	return 0;
}

void db64460_eth_disable (struct eth_device *dev)
{
	mv64460_eth_stop (dev);
}

#define DFCDL(write,read)   ((write << 6) | read)
unsigned int  ethDfcdls[] = {
	DFCDL(0,0),	DFCDL(1,1),	DFCDL(2,2),	DFCDL(3,3),
	DFCDL(4,4),	DFCDL(5,5),	DFCDL(6,6),	DFCDL(7,7),
	DFCDL(8,8),	DFCDL(9,9),	DFCDL(10,10),	DFCDL(11,11),
	DFCDL(12,12),	DFCDL(13,13),	DFCDL(14,14),	DFCDL(15,15),
	DFCDL(16,16),	DFCDL(17,17),	DFCDL(18,18),	DFCDL(19,19),
	DFCDL(20,20),	DFCDL(21,21),	DFCDL(22,22),	DFCDL(23,23),
	DFCDL(24,24),	DFCDL(25,25),	DFCDL(26,26),	DFCDL(27,27),
	DFCDL(28,28),	DFCDL(29,29),	DFCDL(30,30),	DFCDL(31,31),
	DFCDL(32,32),	DFCDL(33,33),	DFCDL(34,34),	DFCDL(35,35),
	DFCDL(36,36),	DFCDL(37,37),	DFCDL(38,38),	DFCDL(39,39),
	DFCDL(40,40),	DFCDL(41,41),	DFCDL(42,42),	DFCDL(43,43),
	DFCDL(44,44),	DFCDL(45,45),	DFCDL(46,46),	DFCDL(47,47),
	DFCDL(48,48),	DFCDL(49,49),	DFCDL(50,50),	DFCDL(51,51),
	DFCDL(52,52),	DFCDL(53,53),	DFCDL(54,54),	DFCDL(55,55),
	DFCDL(56,56),	DFCDL(57,57),	DFCDL(58,58),	DFCDL(59,59),
	DFCDL(60,60),	DFCDL(61,61),	DFCDL(62,62),	DFCDL(63,63),
};

void mv_eth_phy_init (void)
{
	int i;

	MV_REG_WRITE (ETH_PHY_DFCDL_ADDR_REG, 0);

	for (i = 0; i < 64; i++) {
		MV_REG_WRITE (ETH_PHY_DFCDL_DATA0_REG, ethDfcdls[i]);
	}

	MV_REG_WRITE (ETH_PHY_DFCDL_CONFIG0_REG, 0x300000);
}

void mv6446x_eth_initialize (bd_t * bis)
{
	struct eth_device *dev;
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	int devnum, x, temp;
	char *s, *e, buf[64];

	/* P3M750 only
	 * Set RGMII clock drives strength
	 */
	temp = MV_REG_READ(0x20A0);
	temp |= 0x04000080;
	MV_REG_WRITE(0x20A0, temp);

	mv_eth_phy_init();

	for (devnum = 0; devnum < MV_ETH_DEVS; devnum++) {
		dev = calloc (sizeof (*dev), 1);
		if (!dev) {
			printf ("%s: mv_enet%d allocation failure, %s\n",
				__FUNCTION__, devnum, "eth_device structure");
			return;
		}

		/* must be less than sizeof(dev->name) */
		sprintf (dev->name, "mv_enet%d", devnum);

#ifdef DEBUG
		printf ("Initializing %s\n", dev->name);
#endif

		/* Extract the MAC address from the environment */
		switch (devnum) {
		case 0:
			s = "ethaddr";
			break;
		case 1:
			s = "eth1addr";
			break;
		case 2:
			s = "eth2addr";
			break;
		default:	/* this should never happen */
			printf ("%s: Invalid device number %d\n",
				__FUNCTION__, devnum);
			return;
		}

		temp = getenv_f(s, buf, sizeof (buf));
		s = (temp > 0) ? buf : NULL;

#ifdef DEBUG
		printf ("Setting MAC %d to %s\n", devnum, s);
#endif
		for (x = 0; x < 6; ++x) {
			dev->enetaddr[x] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}
		/* ronen - set the MAC addr in the HW */
		eth_port_uc_addr_set (devnum, dev->enetaddr, 0);

		dev->init = (void *) db64460_eth_probe;
		dev->halt = (void *) ethernet_phy_reset;
		dev->send = (void *) db64460_eth_transmit;
		dev->recv = (void *) db64460_eth_poll;

		ethernet_private = calloc (sizeof (*ethernet_private), 1);
		dev->priv = (void *)ethernet_private;
		if (!ethernet_private) {
			printf ("%s: %s allocation failure, %s\n",
				__FUNCTION__, dev->name,
				"Private Device Structure");
			free (dev);
			return;
		}
		/* start with an zeroed ETH_PORT_INFO */
		memset (ethernet_private, 0, sizeof (ETH_PORT_INFO));
		memcpy (ethernet_private->port_mac_addr, dev->enetaddr, 6);

		/* set pointer to memory for stats data structure etc... */
		port_private = calloc (sizeof (*ethernet_private), 1);
		ethernet_private->port_private = (void *)port_private;
		if (!port_private) {
			printf ("%s: %s allocation failure, %s\n",
				__FUNCTION__, dev->name,
				"Port Private Device Structure");

			free (ethernet_private);
			free (dev);
			return;
		}

		port_private->stats =
			calloc (sizeof (struct net_device_stats), 1);
		if (!port_private->stats) {
			printf ("%s: %s allocation failure, %s\n",
				__FUNCTION__, dev->name,
				"Net stat Structure");

			free (port_private);
			free (ethernet_private);
			free (dev);
			return;
		}
		memset (ethernet_private->port_private, 0,
			sizeof (struct mv64460_eth_priv));
		switch (devnum) {
		case 0:
			ethernet_private->port_num = ETH_0;
			break;
		case 1:
			ethernet_private->port_num = ETH_1;
			break;
		case 2:
			ethernet_private->port_num = ETH_2;
			break;
		default:
			printf ("Invalid device number %d\n", devnum);
			break;
		};

		port_private->port_num = devnum;
		/*
		 * Read MIB counter on the GT in order to reset them,
		 * then zero all the stats fields in memory
		 */
		mv64460_eth_update_stat (dev);
		memset (port_private->stats, 0,
			sizeof (struct net_device_stats));
		/* Extract the MAC address from the environment */
		switch (devnum) {
		case 0:
			s = "ethaddr";
			break;
		case 1:
			s = "eth1addr";
			break;
		case 2:
			s = "eth2addr";
			break;
		default:	/* this should never happen */
			printf ("%s: Invalid device number %d\n",
				__FUNCTION__, devnum);
			return;
		}

		temp = getenv_f(s, buf, sizeof (buf));
		s = (temp > 0) ? buf : NULL;

#ifdef DEBUG
		printf ("Setting MAC %d to %s\n", devnum, s);
#endif
		for (x = 0; x < 6; ++x) {
			dev->enetaddr[x] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}

		DP (printf ("Allocating descriptor and buffer rings\n"));

		ethernet_private->p_rx_desc_area_base[0] =
			(ETH_RX_DESC *) memalign (16,
						  RX_DESC_ALIGNED_SIZE *
						  MV64460_RX_QUEUE_SIZE + 1);
		ethernet_private->p_tx_desc_area_base[0] =
			(ETH_TX_DESC *) memalign (16,
						  TX_DESC_ALIGNED_SIZE *
						  MV64460_TX_QUEUE_SIZE + 1);

		ethernet_private->p_rx_buffer_base[0] =
			(char *) memalign (16,
					   MV64460_RX_QUEUE_SIZE *
					   MV64460_TX_BUFFER_SIZE + 1);
		ethernet_private->p_tx_buffer_base[0] =
			(char *) memalign (16,
					   MV64460_RX_QUEUE_SIZE *
					   MV64460_TX_BUFFER_SIZE + 1);

#ifdef DEBUG_MV_ETH
		/* DEBUG OUTPUT prints adresses of globals */
		print_globals (dev);
#endif
		eth_register (dev);

		miiphy_register(dev->name, mv_miiphy_read, mv_miiphy_write);
	}
	DP (printf ("%s: exit\n", __FUNCTION__));

}

/**********************************************************************
 * mv64460_eth_open
 *
 * This function is called when openning the network device. The function
 * should initialize all the hardware, initialize cyclic Rx/Tx
 * descriptors chain and buffers and allocate an IRQ to the network
 * device.
 *
 * Input : a pointer to the network device structure
 * / / ronen - changed the output to match  net/eth.c needs
 * Output : nonzero of success , zero if fails.
 * under construction
 **********************************************************************/

int mv64460_eth_open (struct eth_device *dev)
{
	return (mv64460_eth_real_open (dev));
}

/* Helper function for mv64460_eth_open */
static int mv64460_eth_real_open (struct eth_device *dev)
{

	unsigned int queue;
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	unsigned int port_num;
	ushort reg_short;
	int speed;
	int duplex;
	int i;
	int reg;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	/* ronen - when we update the MAC env params we only update dev->enetaddr
	   see ./net/eth.c eth_set_enetaddr() */
	memcpy (ethernet_private->port_mac_addr, dev->enetaddr, 6);

	port_private = (struct mv64460_eth_priv *) ethernet_private->port_private;
	port_num = port_private->port_num;

	/* Stop RX Queues */
	MV_REG_WRITE (MV64460_ETH_RECEIVE_QUEUE_COMMAND_REG (port_num), 0x0000ff00);

	/* Clear the ethernet port interrupts */
	MV_REG_WRITE (MV64460_ETH_INTERRUPT_CAUSE_REG (port_num), 0);
	MV_REG_WRITE (MV64460_ETH_INTERRUPT_CAUSE_EXTEND_REG (port_num), 0);

	/* Unmask RX buffer and TX end interrupt */
	MV_REG_WRITE (MV64460_ETH_INTERRUPT_MASK_REG (port_num),
		      INT_CAUSE_UNMASK_ALL);

	/* Unmask phy and link status changes interrupts */
	MV_REG_WRITE (MV64460_ETH_INTERRUPT_EXTEND_MASK_REG (port_num),
		      INT_CAUSE_UNMASK_ALL_EXT);

	/* Set phy address of the port */
	ethernet_private->port_phy_addr = 0x1 + (port_num << 1);
	reg = ethernet_private->port_phy_addr;

	/* Activate the DMA channels etc */
	eth_port_init (ethernet_private);

	/* "Allocate" setup TX rings */

	for (queue = 0; queue < MV64460_TX_QUEUE_NUM; queue++) {
		unsigned int size;

		port_private->tx_ring_size[queue] = MV64460_TX_QUEUE_SIZE;
		size = (port_private->tx_ring_size[queue] * TX_DESC_ALIGNED_SIZE);	/*size = no of DESCs times DESC-size */
		ethernet_private->tx_desc_area_size[queue] = size;

		/* first clear desc area completely */
		memset ((void *) ethernet_private->p_tx_desc_area_base[queue],
			0, ethernet_private->tx_desc_area_size[queue]);

		/* initialize tx desc ring with low level driver */
		if (ether_init_tx_desc_ring
		    (ethernet_private, ETH_Q0,
		     port_private->tx_ring_size[queue],
		     MV64460_TX_BUFFER_SIZE /* Each Buffer is 1600 Byte */ ,
		     (unsigned int) ethernet_private->
		     p_tx_desc_area_base[queue],
		     (unsigned int) ethernet_private->
		     p_tx_buffer_base[queue]) == false)
			printf ("### Error initializing TX Ring\n");
	}

	/* "Allocate" setup RX rings */
	for (queue = 0; queue < MV64460_RX_QUEUE_NUM; queue++) {
		unsigned int size;

		/* Meantime RX Ring are fixed - but must be configurable by user */
		port_private->rx_ring_size[queue] = MV64460_RX_QUEUE_SIZE;
		size = (port_private->rx_ring_size[queue] *
			RX_DESC_ALIGNED_SIZE);
		ethernet_private->rx_desc_area_size[queue] = size;

		/* first clear desc area completely */
		memset ((void *) ethernet_private->p_rx_desc_area_base[queue],
			0, ethernet_private->rx_desc_area_size[queue]);
		if ((ether_init_rx_desc_ring
		     (ethernet_private, ETH_Q0,
		      port_private->rx_ring_size[queue],
		      MV64460_RX_BUFFER_SIZE /* Each Buffer is 1600 Byte */ ,
		      (unsigned int) ethernet_private->
		      p_rx_desc_area_base[queue],
		      (unsigned int) ethernet_private->
		      p_rx_buffer_base[queue])) == false)
			printf ("### Error initializing RX Ring\n");
	}

	eth_port_start (ethernet_private);

	/* Set maximum receive buffer to 9700 bytes */
	MV_REG_WRITE (MV64460_ETH_PORT_SERIAL_CONTROL_REG (port_num),
		      (0x5 << 17) |
		      (MV_REG_READ
		       (MV64460_ETH_PORT_SERIAL_CONTROL_REG (port_num))
		       & 0xfff1ffff));

	/*
	 * Set ethernet MTU for leaky bucket mechanism to 0 - this will
	 * disable the leaky bucket mechanism .
	 */

	MV_REG_WRITE (MV64460_ETH_MAXIMUM_TRANSMIT_UNIT (port_num), 0);
	MV_REG_READ (MV64460_ETH_PORT_STATUS_REG (port_num));

#if defined(CONFIG_PHY_RESET)
	/*
	 * Reset the phy, only if its the first time through
	 * otherwise, just check the speeds & feeds
	 */
	if (port_private->first_init == 0) {
		port_private->first_init = 1;
		ethernet_phy_reset (port_num);

		/* Start/Restart autonegotiation */
		phy_setup_aneg (dev->name, reg);
		udelay (1000);
	}
#endif /* defined(CONFIG_PHY_RESET) */

	miiphy_read (dev->name, reg, MII_BMSR, &reg_short);

	/*
	 * Wait if PHY is capable of autonegotiation and autonegotiation is not complete
	 */
	if ((reg_short & BMSR_ANEGCAPABLE)
	    && !(reg_short & BMSR_ANEGCOMPLETE)) {
		puts ("Waiting for PHY auto negotiation to complete");
		i = 0;
		while (!(reg_short & BMSR_ANEGCOMPLETE)) {
			/*
			 * Timeout reached ?
			 */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts (" TIMEOUT !\n");
				break;
			}

			if ((i++ % 1000) == 0) {
				putc ('.');
			}
			udelay (1000);	/* 1 ms */
			miiphy_read (dev->name, reg, MII_BMSR, &reg_short);

		}
		puts (" done\n");
		udelay (500000);	/* another 500 ms (results in faster booting) */
	}

	speed = miiphy_speed (dev->name, reg);
	duplex = miiphy_duplex (dev->name, reg);

	printf ("ENET Speed is %d Mbps - %s duplex connection\n",
		(int) speed, (duplex == HALF) ? "HALF" : "FULL");

	port_private->eth_running = MAGIC_ETH_RUNNING;
	return 1;
}

static int mv64460_eth_free_tx_rings (struct eth_device *dev)
{
	unsigned int queue;
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	unsigned int port_num;
	volatile ETH_TX_DESC *p_tx_curr_desc;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	port_private =
		(struct mv64460_eth_priv *) ethernet_private->port_private;
	port_num = port_private->port_num;

	/* Stop Tx Queues */
	MV_REG_WRITE (MV64460_ETH_TRANSMIT_QUEUE_COMMAND_REG (port_num),
		      0x0000ff00);

	/* Free TX rings */
	DP (printf ("Clearing previously allocated TX queues... "));
	for (queue = 0; queue < MV64460_TX_QUEUE_NUM; queue++) {
		/* Free on TX rings */
		for (p_tx_curr_desc =
		     ethernet_private->p_tx_desc_area_base[queue];
		     ((unsigned int) p_tx_curr_desc <= (unsigned int)
		      ethernet_private->p_tx_desc_area_base[queue] +
		      ethernet_private->tx_desc_area_size[queue]);
		     p_tx_curr_desc =
		     (ETH_TX_DESC *) ((unsigned int) p_tx_curr_desc +
				      TX_DESC_ALIGNED_SIZE)) {
			/* this is inside for loop */
			if (p_tx_curr_desc->return_info != 0) {
				p_tx_curr_desc->return_info = 0;
				DP (printf ("freed\n"));
			}
		}
		DP (printf ("Done\n"));
	}
	return 0;
}

static int mv64460_eth_free_rx_rings (struct eth_device *dev)
{
	unsigned int queue;
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	unsigned int port_num;
	volatile ETH_RX_DESC *p_rx_curr_desc;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	port_private =
		(struct mv64460_eth_priv *) ethernet_private->port_private;
	port_num = port_private->port_num;

	/* Stop RX Queues */
	MV_REG_WRITE (MV64460_ETH_RECEIVE_QUEUE_COMMAND_REG (port_num),
		      0x0000ff00);

	/* Free RX rings */
	DP (printf ("Clearing previously allocated RX queues... "));
	for (queue = 0; queue < MV64460_RX_QUEUE_NUM; queue++) {
		/* Free preallocated skb's on RX rings */
		for (p_rx_curr_desc =
		     ethernet_private->p_rx_desc_area_base[queue];
		     (((unsigned int) p_rx_curr_desc <
		       ((unsigned int) ethernet_private->
			p_rx_desc_area_base[queue] +
			ethernet_private->rx_desc_area_size[queue])));
		     p_rx_curr_desc =
		     (ETH_RX_DESC *) ((unsigned int) p_rx_curr_desc +
				      RX_DESC_ALIGNED_SIZE)) {
			if (p_rx_curr_desc->return_info != 0) {
				p_rx_curr_desc->return_info = 0;
				DP (printf ("freed\n"));
			}
		}
		DP (printf ("Done\n"));
	}
	return 0;
}

/**********************************************************************
 * mv64460_eth_stop
 *
 * This function is used when closing the network device.
 * It updates the hardware,
 * release all memory that holds buffers and descriptors and release the IRQ.
 * Input : a pointer to the device structure
 * Output : zero if success , nonzero if fails
 *********************************************************************/

int mv64460_eth_stop (struct eth_device *dev)
{
	/* Disable all gigE address decoder */
	MV_REG_WRITE (MV64460_ETH_BASE_ADDR_ENABLE_REG, 0x3f);
	DP (printf ("%s Ethernet stop called ... \n", __FUNCTION__));
	mv64460_eth_real_stop (dev);

	return 0;
};

/* Helper function for mv64460_eth_stop */

static int mv64460_eth_real_stop (struct eth_device *dev)
{
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	unsigned int port_num;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	port_private =
		(struct mv64460_eth_priv *) ethernet_private->port_private;
	port_num = port_private->port_num;

	mv64460_eth_free_tx_rings (dev);
	mv64460_eth_free_rx_rings (dev);

	eth_port_reset (ethernet_private->port_num);
	/* Disable ethernet port interrupts */
	MV_REG_WRITE (MV64460_ETH_INTERRUPT_CAUSE_REG (port_num), 0);
	MV_REG_WRITE (MV64460_ETH_INTERRUPT_CAUSE_EXTEND_REG (port_num), 0);
	/* Mask RX buffer and TX end interrupt */
	MV_REG_WRITE (MV64460_ETH_INTERRUPT_MASK_REG (port_num), 0);
	/* Mask phy and link status changes interrupts */
	MV_REG_WRITE (MV64460_ETH_INTERRUPT_EXTEND_MASK_REG (port_num), 0);
	MV_RESET_REG_BITS (MV64460_CPU_INTERRUPT0_MASK_HIGH,
			   BIT0 << port_num);
	/* Print Network statistics */
#ifndef	 UPDATE_STATS_BY_SOFTWARE
	/*
	 * Print statistics (only if ethernet is running),
	 * then zero all the stats fields in memory
	 */
	if (port_private->eth_running == MAGIC_ETH_RUNNING) {
		port_private->eth_running = 0;
		mv64460_eth_print_stat (dev);
	}
	memset (port_private->stats, 0, sizeof (struct net_device_stats));
#endif
	DP (printf ("\nEthernet stopped ... \n"));
	return 0;
}

/**********************************************************************
 * mv64460_eth_start_xmit
 *
 * This function is queues a packet in the Tx descriptor for
 * required port.
 *
 * Input : skb - a pointer to socket buffer
 *	   dev - a pointer to the required port
 *
 * Output : zero upon success
 **********************************************************************/

int mv64460_eth_xmit (struct eth_device *dev, volatile void *dataPtr,
		      int dataSize)
{
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	PKT_INFO pkt_info;
	ETH_FUNC_RET_STATUS status;
	struct net_device_stats *stats;
	ETH_FUNC_RET_STATUS release_result;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	port_private =
		(struct mv64460_eth_priv *) ethernet_private->port_private;

	stats = port_private->stats;

	/* Update packet info data structure */
	pkt_info.cmd_sts = ETH_TX_FIRST_DESC | ETH_TX_LAST_DESC;	/* DMA owned, first last */
	pkt_info.byte_cnt = dataSize;
	pkt_info.buf_ptr = (unsigned int) dataPtr;
	pkt_info.return_info = 0;

	status = eth_port_send (ethernet_private, ETH_Q0, &pkt_info);
	if ((status == ETH_ERROR) || (status == ETH_QUEUE_FULL)) {
		printf ("Error on transmitting packet ..");
		if (status == ETH_QUEUE_FULL)
			printf ("ETH Queue is full. \n");
		if (status == ETH_QUEUE_LAST_RESOURCE)
			printf ("ETH Queue: using last available resource. \n");
		return 1;
	}

	/* Update statistics and start of transmittion time */
	stats->tx_bytes += dataSize;
	stats->tx_packets++;

	/* Check if packet(s) is(are) transmitted correctly (release everything) */
	do {
		release_result =
			eth_tx_return_desc (ethernet_private, ETH_Q0,
					    &pkt_info);
		switch (release_result) {
		case ETH_OK:
			DP (printf ("descriptor released\n"));
			if (pkt_info.cmd_sts & BIT0) {
				printf ("Error in TX\n");
				stats->tx_errors++;
			}
			break;
		case ETH_RETRY:
			DP (printf ("transmission still in process\n"));
			break;

		case ETH_ERROR:
			printf ("routine can not access Tx desc ring\n");
			break;

		case ETH_END_OF_JOB:
			DP (printf ("the routine has nothing to release\n"));
			break;
		default:	/* should not happen */
			break;
		}
	} while (release_result == ETH_OK);

	return 0;	/* success */
}

/**********************************************************************
 * mv64460_eth_receive
 *
 * This function is forward packets that are received from the port's
 * queues toward kernel core or FastRoute them to another interface.
 *
 * Input : dev - a pointer to the required interface
 *	   max - maximum number to receive (0 means unlimted)
 *
 * Output : number of served packets
 **********************************************************************/

int mv64460_eth_receive (struct eth_device *dev)
{
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	PKT_INFO pkt_info;
	struct net_device_stats *stats;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	port_private = (struct mv64460_eth_priv *) ethernet_private->port_private;
	stats = port_private->stats;

	while ((eth_port_receive (ethernet_private, ETH_Q0, &pkt_info) == ETH_OK)) {
#ifdef DEBUG_MV_ETH
		if (pkt_info.byte_cnt != 0) {
			printf ("%s: Received %d byte Packet @ 0x%x\n",
				__FUNCTION__, pkt_info.byte_cnt,
				pkt_info.buf_ptr);
			if(pkt_info.buf_ptr != 0){
				for(i=0; i < pkt_info.byte_cnt; i++){
					if((i % 4) == 0){
						printf("\n0x");
					}
					printf("%02x", ((char*)pkt_info.buf_ptr)[i]);
				}
				printf("\n");
			}
		}
#endif
		/* Update statistics. Note byte count includes 4 byte CRC count */
		stats->rx_packets++;
		stats->rx_bytes += pkt_info.byte_cnt;

		/*
		 * In case received a packet without first / last bits on OR the error
		 * summary bit is on, the packets needs to be dropeed.
		 */
		if (((pkt_info.
		      cmd_sts & (ETH_RX_FIRST_DESC | ETH_RX_LAST_DESC)) !=
		     (ETH_RX_FIRST_DESC | ETH_RX_LAST_DESC))
		    || (pkt_info.cmd_sts & ETH_ERROR_SUMMARY)) {
			stats->rx_dropped++;

			printf ("Received packet spread on multiple descriptors\n");

			/* Is this caused by an error ? */
			if (pkt_info.cmd_sts & ETH_ERROR_SUMMARY) {
				stats->rx_errors++;
			}

			/* free these descriptors again without forwarding them to the higher layers */
			pkt_info.buf_ptr &= ~0x7;	/* realign buffer again */
			pkt_info.byte_cnt = 0x0000;	/* Reset Byte count */

			if (eth_rx_return_buff
			    (ethernet_private, ETH_Q0, &pkt_info) != ETH_OK) {
				printf ("Error while returning the RX Desc to Ring\n");
			} else {
				DP (printf ("RX Desc returned to Ring\n"));
			}
			/* /free these descriptors again */
		} else {

/* !!! call higher layer processing */
#ifdef DEBUG_MV_ETH
			printf ("\nNow send it to upper layer protocols (NetReceive) ...\n");
#endif
			/* let the upper layer handle the packet */
			NetReceive ((uchar *) pkt_info.buf_ptr,
				    (int) pkt_info.byte_cnt);

/* **************************************************************** */
/* free descriptor  */
			pkt_info.buf_ptr &= ~0x7;	/* realign buffer again */
			pkt_info.byte_cnt = 0x0000;	/* Reset Byte count */
			DP (printf ("RX: pkt_info.buf_ptr =	%x\n", pkt_info.buf_ptr));
			if (eth_rx_return_buff
			    (ethernet_private, ETH_Q0, &pkt_info) != ETH_OK) {
				printf ("Error while returning the RX Desc to Ring\n");
			} else {
				DP (printf ("RX: Desc returned to Ring\n"));
			}

/* **************************************************************** */

		}
	}
	mv64460_eth_get_stats (dev);	/* update statistics */
	return 1;
}

/**********************************************************************
 * mv64460_eth_get_stats
 *
 * Returns a pointer to the interface statistics.
 *
 * Input : dev - a pointer to the required interface
 *
 * Output : a pointer to the interface's statistics
 **********************************************************************/

static struct net_device_stats *mv64460_eth_get_stats (struct eth_device *dev)
{
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	port_private =
		(struct mv64460_eth_priv *) ethernet_private->port_private;

	mv64460_eth_update_stat (dev);

	return port_private->stats;
}

/**********************************************************************
 * mv64460_eth_update_stat
 *
 * Update the statistics structure in the private data structure
 *
 * Input : pointer to ethernet interface network device structure
 * Output : N/A
 **********************************************************************/

static void mv64460_eth_update_stat (struct eth_device *dev)
{
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	struct net_device_stats *stats;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	port_private =
		(struct mv64460_eth_priv *) ethernet_private->port_private;
	stats = port_private->stats;

	/* These are false updates */
	stats->rx_packets += (unsigned long)
		eth_read_mib_counter (ethernet_private->port_num,
				      ETH_MIB_GOOD_FRAMES_RECEIVED);
	stats->tx_packets += (unsigned long)
		eth_read_mib_counter (ethernet_private->port_num,
				      ETH_MIB_GOOD_FRAMES_SENT);
	stats->rx_bytes += (unsigned long)
		eth_read_mib_counter (ethernet_private->port_num,
				      ETH_MIB_GOOD_OCTETS_RECEIVED_LOW);
	/*
	 * Ideally this should be as follows -
	 *
	 *   stats->rx_bytes   += stats->rx_bytes +
	 * ((unsigned long) ethReadMibCounter (ethernet_private->port_num ,
	 * ETH_MIB_GOOD_OCTETS_RECEIVED_HIGH) << 32);
	 *
	 * But the unsigned long in PowerPC and MIPS are 32bit. So the next read
	 * is just a dummy read for proper work of the GigE port
	 */
	(void)eth_read_mib_counter (ethernet_private->port_num,
				      ETH_MIB_GOOD_OCTETS_RECEIVED_HIGH);
	stats->tx_bytes += (unsigned long)
		eth_read_mib_counter (ethernet_private->port_num,
				      ETH_MIB_GOOD_OCTETS_SENT_LOW);
	(void)eth_read_mib_counter (ethernet_private->port_num,
				      ETH_MIB_GOOD_OCTETS_SENT_HIGH);
	stats->rx_errors += (unsigned long)
		eth_read_mib_counter (ethernet_private->port_num,
				      ETH_MIB_MAC_RECEIVE_ERROR);

	/* Rx dropped is for received packet with CRC error */
	stats->rx_dropped +=
		(unsigned long) eth_read_mib_counter (ethernet_private->
						      port_num,
						      ETH_MIB_BAD_CRC_EVENT);
	stats->multicast += (unsigned long)
		eth_read_mib_counter (ethernet_private->port_num,
				      ETH_MIB_MULTICAST_FRAMES_RECEIVED);
	stats->collisions +=
		(unsigned long) eth_read_mib_counter (ethernet_private->
						      port_num,
						      ETH_MIB_COLLISION) +
		(unsigned long) eth_read_mib_counter (ethernet_private->
						      port_num,
						      ETH_MIB_LATE_COLLISION);
	/* detailed rx errors */
	stats->rx_length_errors +=
		(unsigned long) eth_read_mib_counter (ethernet_private->
						      port_num,
						      ETH_MIB_UNDERSIZE_RECEIVED)
		+
		(unsigned long) eth_read_mib_counter (ethernet_private->
						      port_num,
						      ETH_MIB_OVERSIZE_RECEIVED);
	/* detailed tx errors */
}

#ifndef	 UPDATE_STATS_BY_SOFTWARE
/**********************************************************************
 * mv64460_eth_print_stat
 *
 * Update the statistics structure in the private data structure
 *
 * Input : pointer to ethernet interface network device structure
 * Output : N/A
 **********************************************************************/

static void mv64460_eth_print_stat (struct eth_device *dev)
{
	ETH_PORT_INFO *ethernet_private;
	struct mv64460_eth_priv *port_private;
	struct net_device_stats *stats;

	ethernet_private = (ETH_PORT_INFO *) dev->priv;
	port_private =
		(struct mv64460_eth_priv *) ethernet_private->port_private;
	stats = port_private->stats;

	/* These are false updates */
	printf ("\n### Network statistics: ###\n");
	printf ("--------------------------\n");
	printf (" Packets received:		%ld\n", stats->rx_packets);
	printf (" Packets send:			%ld\n", stats->tx_packets);
	printf (" Received bytes:		%ld\n", stats->rx_bytes);
	printf (" Send bytes:			%ld\n", stats->tx_bytes);
	if (stats->rx_errors != 0)
		printf (" Rx Errors:			%ld\n",
			stats->rx_errors);
	if (stats->rx_dropped != 0)
		printf (" Rx dropped (CRC Errors):	%ld\n",
			stats->rx_dropped);
	if (stats->multicast != 0)
		printf (" Rx mulicast frames:		%ld\n",
			stats->multicast);
	if (stats->collisions != 0)
		printf (" No. of collisions:		%ld\n",
			stats->collisions);
	if (stats->rx_length_errors != 0)
		printf (" Rx length errors:		%ld\n",
			stats->rx_length_errors);
}
#endif

/**************************************************************************
 *network_start - Network Kick Off Routine UBoot
 *Inputs :
 *Outputs :
 **************************************************************************/

bool db64460_eth_start (struct eth_device *dev)
{
	return (mv64460_eth_open (dev));	/* calls real open */
}

/*************************************************************************
**************************************************************************
**************************************************************************
*  The second part is the low level driver of the gigE ethernet ports.	 *
**************************************************************************
**************************************************************************
*************************************************************************/
/*
 * based on Linux code
 * arch/powerpc/galileo/EVB64460/mv64460_eth.c - Driver for MV64460X ethernet ports
 * Copyright (C) 2002 rabeeh@galileo.co.il

 * SPDX-License-Identifier:	GPL-2.0+
 */

/********************************************************************************
 * Marvell's Gigabit Ethernet controller low level driver
 *
 * DESCRIPTION:
 *	 This file introduce low level API to Marvell's Gigabit Ethernet
 *		controller. This Gigabit Ethernet Controller driver API controls
 *		1) Operations (i.e. port init, start, reset etc').
 *		2) Data flow (i.e. port send, receive etc').
 *		Each Gigabit Ethernet port is controlled via ETH_PORT_INFO
 *		struct.
 *		This struct includes user configuration information as well as
 *		driver internal data needed for its operations.
 *
 *		Supported Features:
 *		- This low level driver is OS independent. Allocating memory for
 *		  the descriptor rings and buffers are not within the scope of
 *		  this driver.
 *		- The user is free from Rx/Tx queue managing.
 *		- This low level driver introduce functionality API that enable
 *		  the to operate Marvell's Gigabit Ethernet Controller in a
 *		  convenient way.
 *		- Simple Gigabit Ethernet port operation API.
 *		- Simple Gigabit Ethernet port data flow API.
 *		- Data flow and operation API support per queue functionality.
 *		- Support cached descriptors for better performance.
 *		- Enable access to all four DRAM banks and internal SRAM memory
 *		  spaces.
 *		- PHY access and control API.
 *		- Port control register configuration API.
 *		- Full control over Unicast and Multicast MAC configurations.
 *
 *		Operation flow:
 *
 *		Initialization phase
 *		This phase complete the initialization of the ETH_PORT_INFO
 *		struct.
 *		User information regarding port configuration has to be set
 *		prior to calling the port initialization routine. For example,
 *		the user has to assign the port_phy_addr field which is board
 *		depended parameter.
 *		In this phase any port Tx/Rx activity is halted, MIB counters
 *		are cleared, PHY address is set according to user parameter and
 *		access to DRAM and internal SRAM memory spaces.
 *
 *		Driver ring initialization
 *		Allocating memory for the descriptor rings and buffers is not
 *		within the scope of this driver. Thus, the user is required to
 *		allocate memory for the descriptors ring and buffers. Those
 *		memory parameters are used by the Rx and Tx ring initialization
 *		routines in order to curve the descriptor linked list in a form
 *		of a ring.
 *		Note: Pay special attention to alignment issues when using
 *		cached descriptors/buffers. In this phase the driver store
 *		information in the ETH_PORT_INFO struct regarding each queue
 *		ring.
 *
 *		Driver start
 *		This phase prepares the Ethernet port for Rx and Tx activity.
 *		It uses the information stored in the ETH_PORT_INFO struct to
 *		initialize the various port registers.
 *
 *		Data flow:
 *		All packet references to/from the driver are done using PKT_INFO
 *		struct.
 *		This struct is a unified struct used with Rx and Tx operations.
 *		This way the user is not required to be familiar with neither
 *		Tx nor Rx descriptors structures.
 *		The driver's descriptors rings are management by indexes.
 *		Those indexes controls the ring resources and used to indicate
 *		a SW resource error:
 *		'current'
 *		This index points to the current available resource for use. For
 *		example in Rx process this index will point to the descriptor
 *		that will be passed to the user upon calling the receive routine.
 *		In Tx process, this index will point to the descriptor
 *		that will be assigned with the user packet info and transmitted.
 *		'used'
 *		This index points to the descriptor that need to restore its
 *		resources. For example in Rx process, using the Rx buffer return
 *		API will attach the buffer returned in packet info to the
 *		descriptor pointed by 'used'. In Tx process, using the Tx
 *		descriptor return will merely return the user packet info with
 *		the command status of  the transmitted buffer pointed by the
 *		'used' index. Nevertheless, it is essential to use this routine
 *		to update the 'used' index.
 *		'first'
 *		This index supports Tx Scatter-Gather. It points to the first
 *		descriptor of a packet assembled of multiple buffers. For example
 *		when in middle of Such packet we have a Tx resource error the
 *		'curr' index get the value of 'first' to indicate that the ring
 *		returned to its state before trying to transmit this packet.
 *
 *		Receive operation:
 *		The eth_port_receive API set the packet information struct,
 *		passed by the caller, with received information from the
 *		'current' SDMA descriptor.
 *		It is the user responsibility to return this resource back
 *		to the Rx descriptor ring to enable the reuse of this source.
 *		Return Rx resource is done using the eth_rx_return_buff API.
 *
 *		Transmit operation:
 *		The eth_port_send API supports Scatter-Gather which enables to
 *		send a packet spanned over multiple buffers. This means that
 *		for each packet info structure given by the user and put into
 *		the Tx descriptors ring, will be transmitted only if the 'LAST'
 *		bit will be set in the packet info command status field. This
 *		API also consider restriction regarding buffer alignments and
 *		sizes.
 *		The user must return a Tx resource after ensuring the buffer
 *		has been transmitted to enable the Tx ring indexes to update.
 *
 *		BOARD LAYOUT
 *		This device is on-board.  No jumper diagram is necessary.
 *
 *		EXTERNAL INTERFACE
 *
 *	 Prior to calling the initialization routine eth_port_init() the user
 *	 must set the following fields under ETH_PORT_INFO struct:
 *	 port_num	      User Ethernet port number.
 *	 port_phy_addr		    User PHY address of Ethernet port.
 *	 port_mac_addr[6]	    User defined port MAC address.
 *	 port_config	      User port configuration value.
 *	 port_config_extend    User port config extend value.
 *	 port_sdma_config      User port SDMA config value.
 *	 port_serial_control   User port serial control value.
 *	 *port_virt_to_phys ()	User function to cast virtual addr to CPU bus addr.
 *	 *port_private	      User scratch pad for user specific data structures.
 *
 *	 This driver introduce a set of default values:
 *	 PORT_CONFIG_VALUE	     Default port configuration value
 *	 PORT_CONFIG_EXTEND_VALUE    Default port extend configuration value
 *	 PORT_SDMA_CONFIG_VALUE	     Default sdma control value
 *	 PORT_SERIAL_CONTROL_VALUE   Default port serial control value
 *
 *		This driver data flow is done using the PKT_INFO struct which is
 *		a unified struct for Rx and Tx operations:
 *		byte_cnt	Tx/Rx descriptor buffer byte count.
 *		l4i_chk		CPU provided TCP Checksum. For Tx operation only.
 *		cmd_sts		Tx/Rx descriptor command status.
 *		buf_ptr		Tx/Rx descriptor buffer pointer.
 *		return_info	Tx/Rx user resource return information.
 *
 *
 *		EXTERNAL SUPPORT REQUIREMENTS
 *
 *		This driver requires the following external support:
 *
 *		D_CACHE_FLUSH_LINE (address, address offset)
 *
 *		This macro applies assembly code to flush and invalidate cache
 *		line.
 *		address	       - address base.
 *		address offset - address offset
 *
 *
 *		CPU_PIPE_FLUSH
 *
 *		This macro applies assembly code to flush the CPU pipeline.
 *
 *******************************************************************************/
/* includes */

/* defines */
/* SDMA command macros */
#define ETH_ENABLE_TX_QUEUE(tx_queue, eth_port) \
 MV_REG_WRITE(MV64460_ETH_TRANSMIT_QUEUE_COMMAND_REG(eth_port), (1 << tx_queue))

#define ETH_DISABLE_TX_QUEUE(tx_queue, eth_port) \
 MV_REG_WRITE(MV64460_ETH_TRANSMIT_QUEUE_COMMAND_REG(eth_port),\
 (1 << (8 + tx_queue)))

#define ETH_ENABLE_RX_QUEUE(rx_queue, eth_port) \
MV_REG_WRITE(MV64460_ETH_RECEIVE_QUEUE_COMMAND_REG(eth_port), (1 << rx_queue))

#define ETH_DISABLE_RX_QUEUE(rx_queue, eth_port) \
MV_REG_WRITE(MV64460_ETH_RECEIVE_QUEUE_COMMAND_REG(eth_port), (1 << (8 + rx_queue)))

#define CURR_RFD_GET(p_curr_desc, queue) \
 ((p_curr_desc) = p_eth_port_ctrl->p_rx_curr_desc_q[queue])

#define CURR_RFD_SET(p_curr_desc, queue) \
 (p_eth_port_ctrl->p_rx_curr_desc_q[queue] = (p_curr_desc))

#define USED_RFD_GET(p_used_desc, queue) \
 ((p_used_desc) = p_eth_port_ctrl->p_rx_used_desc_q[queue])

#define USED_RFD_SET(p_used_desc, queue)\
(p_eth_port_ctrl->p_rx_used_desc_q[queue] = (p_used_desc))


#define CURR_TFD_GET(p_curr_desc, queue) \
 ((p_curr_desc) = p_eth_port_ctrl->p_tx_curr_desc_q[queue])

#define CURR_TFD_SET(p_curr_desc, queue) \
 (p_eth_port_ctrl->p_tx_curr_desc_q[queue] = (p_curr_desc))

#define USED_TFD_GET(p_used_desc, queue) \
 ((p_used_desc) = p_eth_port_ctrl->p_tx_used_desc_q[queue])

#define USED_TFD_SET(p_used_desc, queue) \
 (p_eth_port_ctrl->p_tx_used_desc_q[queue] = (p_used_desc))

#define FIRST_TFD_GET(p_first_desc, queue) \
 ((p_first_desc) = p_eth_port_ctrl->p_tx_first_desc_q[queue])

#define FIRST_TFD_SET(p_first_desc, queue) \
 (p_eth_port_ctrl->p_tx_first_desc_q[queue] = (p_first_desc))


/* Macros that save access to desc in order to find next desc pointer  */
#define RX_NEXT_DESC_PTR(p_rx_desc, queue) (ETH_RX_DESC*)(((((unsigned int)p_rx_desc - (unsigned int)p_eth_port_ctrl->p_rx_desc_area_base[queue]) + RX_DESC_ALIGNED_SIZE) % p_eth_port_ctrl->rx_desc_area_size[queue]) + (unsigned int)p_eth_port_ctrl->p_rx_desc_area_base[queue])

#define TX_NEXT_DESC_PTR(p_tx_desc, queue) (ETH_TX_DESC*)(((((unsigned int)p_tx_desc - (unsigned int)p_eth_port_ctrl->p_tx_desc_area_base[queue]) + TX_DESC_ALIGNED_SIZE) % p_eth_port_ctrl->tx_desc_area_size[queue]) + (unsigned int)p_eth_port_ctrl->p_tx_desc_area_base[queue])

#define LINK_UP_TIMEOUT		100000
#define PHY_BUSY_TIMEOUT    10000000

/* locals */

/* PHY routines */
static void ethernet_phy_set (ETH_PORT eth_port_num, int phy_addr);
static int ethernet_phy_get (ETH_PORT eth_port_num);

/* Ethernet Port routines */
static void eth_set_access_control (ETH_PORT eth_port_num,
				    ETH_WIN_PARAM * param);
static bool eth_port_uc_addr (ETH_PORT eth_port_num, unsigned char uc_nibble,
			      ETH_QUEUE queue, int option);
#if 0				/* FIXME */
static bool eth_port_smc_addr (ETH_PORT eth_port_num,
			       unsigned char mc_byte,
			       ETH_QUEUE queue, int option);
static bool eth_port_omc_addr (ETH_PORT eth_port_num,
			       unsigned char crc8,
			       ETH_QUEUE queue, int option);
#endif

static void eth_b_copy (unsigned int src_addr, unsigned int dst_addr,
			int byte_count);

void eth_dbg (ETH_PORT_INFO * p_eth_port_ctrl);


typedef enum _memory_bank { BANK0, BANK1, BANK2, BANK3 } MEMORY_BANK;
u32 mv_get_dram_bank_base_addr (MEMORY_BANK bank)
{
	u32 result = 0;
	u32 enable = MV_REG_READ (MV64460_BASE_ADDR_ENABLE);

	if (enable & (1 << bank))
		return 0;
	if (bank == BANK0)
		result = MV_REG_READ (MV64460_CS_0_BASE_ADDR);
	if (bank == BANK1)
		result = MV_REG_READ (MV64460_CS_1_BASE_ADDR);
	if (bank == BANK2)
		result = MV_REG_READ (MV64460_CS_2_BASE_ADDR);
	if (bank == BANK3)
		result = MV_REG_READ (MV64460_CS_3_BASE_ADDR);
	result &= 0x0000ffff;
	result = result << 16;
	return result;
}

u32 mv_get_dram_bank_size (MEMORY_BANK bank)
{
	u32 result = 0;
	u32 enable = MV_REG_READ (MV64460_BASE_ADDR_ENABLE);

	if (enable & (1 << bank))
		return 0;
	if (bank == BANK0)
		result = MV_REG_READ (MV64460_CS_0_SIZE);
	if (bank == BANK1)
		result = MV_REG_READ (MV64460_CS_1_SIZE);
	if (bank == BANK2)
		result = MV_REG_READ (MV64460_CS_2_SIZE);
	if (bank == BANK3)
		result = MV_REG_READ (MV64460_CS_3_SIZE);
	result += 1;
	result &= 0x0000ffff;
	result = result << 16;
	return result;
}

u32 mv_get_internal_sram_base (void)
{
	u32 result;

	result = MV_REG_READ (MV64460_INTEGRATED_SRAM_BASE_ADDR);
	result &= 0x0000ffff;
	result = result << 16;
	return result;
}

/*******************************************************************************
* eth_port_init - Initialize the Ethernet port driver
*
* DESCRIPTION:
*	This function prepares the ethernet port to start its activity:
*	1) Completes the ethernet port driver struct initialization toward port
*	    start routine.
*	2) Resets the device to a quiescent state in case of warm reboot.
*	3) Enable SDMA access to all four DRAM banks as well as internal SRAM.
*	4) Clean MAC tables. The reset status of those tables is unknown.
*	5) Set PHY address.
*	Note: Call this routine prior to eth_port_start routine and after setting
*	user values in the user fields of Ethernet port control struct (i.e.
*	port_phy_addr).
*
* INPUT:
*	ETH_PORT_INFO	*p_eth_port_ctrl       Ethernet port control struct
*
* OUTPUT:
*	See description.
*
* RETURN:
*	None.
*
*******************************************************************************/
static void eth_port_init (ETH_PORT_INFO * p_eth_port_ctrl)
{
	int queue;
	ETH_WIN_PARAM win_param;

	p_eth_port_ctrl->port_config = PORT_CONFIG_VALUE;
	p_eth_port_ctrl->port_config_extend = PORT_CONFIG_EXTEND_VALUE;
	p_eth_port_ctrl->port_sdma_config = PORT_SDMA_CONFIG_VALUE;
	p_eth_port_ctrl->port_serial_control = PORT_SERIAL_CONTROL_VALUE;

	p_eth_port_ctrl->port_rx_queue_command = 0;
	p_eth_port_ctrl->port_tx_queue_command = 0;

	/* Zero out SW structs */
	for (queue = 0; queue < MAX_RX_QUEUE_NUM; queue++) {
		CURR_RFD_SET ((ETH_RX_DESC *) 0x00000000, queue);
		USED_RFD_SET ((ETH_RX_DESC *) 0x00000000, queue);
		p_eth_port_ctrl->rx_resource_err[queue] = false;
	}

	for (queue = 0; queue < MAX_TX_QUEUE_NUM; queue++) {
		CURR_TFD_SET ((ETH_TX_DESC *) 0x00000000, queue);
		USED_TFD_SET ((ETH_TX_DESC *) 0x00000000, queue);
		FIRST_TFD_SET ((ETH_TX_DESC *) 0x00000000, queue);
		p_eth_port_ctrl->tx_resource_err[queue] = false;
	}

	eth_port_reset (p_eth_port_ctrl->port_num);

	/* Set access parameters for DRAM bank 0 */
	win_param.win = ETH_WIN0;	/* Use Ethernet window 0 */
	win_param.target = ETH_TARGET_DRAM;	/* Window target - DDR	*/
	win_param.attributes = EBAR_ATTR_DRAM_CS0;	/* Enable DRAM bank   */
#ifndef CONFIG_NOT_COHERENT_CACHE
	win_param.attributes |= EBAR_ATTR_DRAM_CACHE_COHERENCY_WB;
#endif
	win_param.high_addr = 0;
	/* Get bank base */
	win_param.base_addr = mv_get_dram_bank_base_addr (BANK0);
	win_param.size = mv_get_dram_bank_size (BANK0); /* Get bank size */
	if (win_param.size == 0)
		win_param.enable = 0;
	else
		win_param.enable = 1;	/* Enable the access */
	win_param.access_ctrl = EWIN_ACCESS_FULL;	/* Enable full access */

	/* Set the access control for address window (EPAPR) READ & WRITE */
	eth_set_access_control (p_eth_port_ctrl->port_num, &win_param);

	/* Set access parameters for DRAM bank 1 */
	win_param.win = ETH_WIN1;	/* Use Ethernet window 1 */
	win_param.target = ETH_TARGET_DRAM;	/* Window target - DDR */
	win_param.attributes = EBAR_ATTR_DRAM_CS1;	/* Enable DRAM bank */
#ifndef CONFIG_NOT_COHERENT_CACHE
	win_param.attributes |= EBAR_ATTR_DRAM_CACHE_COHERENCY_WB;
#endif
	win_param.high_addr = 0;
	/* Get bank base */
	win_param.base_addr = mv_get_dram_bank_base_addr (BANK1);
	win_param.size = mv_get_dram_bank_size (BANK1); /* Get bank size */
	if (win_param.size == 0)
		win_param.enable = 0;
	else
		win_param.enable = 1;	/* Enable the access */
	win_param.access_ctrl = EWIN_ACCESS_FULL;	/* Enable full access */

	/* Set the access control for address window (EPAPR) READ & WRITE */
	eth_set_access_control (p_eth_port_ctrl->port_num, &win_param);

	/* Set access parameters for DRAM bank 2 */
	win_param.win = ETH_WIN2;	/* Use Ethernet window 2 */
	win_param.target = ETH_TARGET_DRAM;	/* Window target - DDR */
	win_param.attributes = EBAR_ATTR_DRAM_CS2;	/* Enable DRAM bank */
#ifndef CONFIG_NOT_COHERENT_CACHE
	win_param.attributes |= EBAR_ATTR_DRAM_CACHE_COHERENCY_WB;
#endif
	win_param.high_addr = 0;
	/* Get bank base */
	win_param.base_addr = mv_get_dram_bank_base_addr (BANK2);
	win_param.size = mv_get_dram_bank_size (BANK2); /* Get bank size */
	if (win_param.size == 0)
		win_param.enable = 0;
	else
		win_param.enable = 1;	/* Enable the access */
	win_param.access_ctrl = EWIN_ACCESS_FULL;	/* Enable full access */

	/* Set the access control for address window (EPAPR) READ & WRITE */
	eth_set_access_control (p_eth_port_ctrl->port_num, &win_param);

	/* Set access parameters for DRAM bank 3 */
	win_param.win = ETH_WIN3;	/* Use Ethernet window 3 */
	win_param.target = ETH_TARGET_DRAM;	/* Window target - DDR */
	win_param.attributes = EBAR_ATTR_DRAM_CS3;	/* Enable DRAM bank */
#ifndef CONFIG_NOT_COHERENT_CACHE
	win_param.attributes |= EBAR_ATTR_DRAM_CACHE_COHERENCY_WB;
#endif
	win_param.high_addr = 0;
	/* Get bank base */
	win_param.base_addr = mv_get_dram_bank_base_addr (BANK3);
	win_param.size = mv_get_dram_bank_size (BANK3); /* Get bank size */
	if (win_param.size == 0)
		win_param.enable = 0;
	else
		win_param.enable = 1;	/* Enable the access */
	win_param.access_ctrl = EWIN_ACCESS_FULL;	/* Enable full access */

	/* Set the access control for address window (EPAPR) READ & WRITE */
	eth_set_access_control (p_eth_port_ctrl->port_num, &win_param);

	/* Set access parameters for Internal SRAM */
	win_param.win = ETH_WIN4;	/* Use Ethernet window 0 */
	win_param.target = EBAR_TARGET_CBS;	/* Target - Internal SRAM */
	win_param.attributes = EBAR_ATTR_CBS_SRAM | EBAR_ATTR_CBS_SRAM_BLOCK0;
	win_param.high_addr = 0;
	win_param.base_addr = mv_get_internal_sram_base ();	/* Get base addr */
	win_param.size = MV64460_INTERNAL_SRAM_SIZE;	/* Get bank size */
	win_param.enable = 1;	/* Enable the access */
	win_param.access_ctrl = EWIN_ACCESS_FULL;	/* Enable full access */

	/* Set the access control for address window (EPAPR) READ & WRITE */
	eth_set_access_control (p_eth_port_ctrl->port_num, &win_param);

	eth_port_init_mac_tables (p_eth_port_ctrl->port_num);

	ethernet_phy_set (p_eth_port_ctrl->port_num,
			  p_eth_port_ctrl->port_phy_addr);

	return;

}

/*******************************************************************************
* eth_port_start - Start the Ethernet port activity.
*
* DESCRIPTION:
*	This routine prepares the Ethernet port for Rx and Tx activity:
*	1. Initialize Tx and Rx Current Descriptor Pointer for each queue that
*	    has been initialized a descriptor's ring (using ether_init_tx_desc_ring
*	    for Tx and ether_init_rx_desc_ring for Rx)
*	2. Initialize and enable the Ethernet configuration port by writing to
*	    the port's configuration and command registers.
*	3. Initialize and enable the SDMA by writing to the SDMA's
*    configuration and command registers.
*	After completing these steps, the ethernet port SDMA can starts to
*	perform Rx and Tx activities.
*
*	Note: Each Rx and Tx queue descriptor's list must be initialized prior
*	to calling this function (use ether_init_tx_desc_ring for Tx queues and
*	ether_init_rx_desc_ring for Rx queues).
*
* INPUT:
*	ETH_PORT_INFO	*p_eth_port_ctrl       Ethernet port control struct
*
* OUTPUT:
*	Ethernet port is ready to receive and transmit.
*
* RETURN:
*	false if the port PHY is not up.
*	true otherwise.
*
*******************************************************************************/
static bool eth_port_start (ETH_PORT_INFO * p_eth_port_ctrl)
{
	int queue;
	volatile ETH_TX_DESC *p_tx_curr_desc;
	volatile ETH_RX_DESC *p_rx_curr_desc;
	unsigned int phy_reg_data;
	ETH_PORT eth_port_num = p_eth_port_ctrl->port_num;

	/* Assignment of Tx CTRP of given queue */
	for (queue = 0; queue < MAX_TX_QUEUE_NUM; queue++) {
		CURR_TFD_GET (p_tx_curr_desc, queue);
		MV_REG_WRITE ((MV64460_ETH_TX_CURRENT_QUEUE_DESC_PTR_0
			       (eth_port_num)
			       + (4 * queue)),
			      ((unsigned int) p_tx_curr_desc));

	}

	/* Assignment of Rx CRDP of given queue */
	for (queue = 0; queue < MAX_RX_QUEUE_NUM; queue++) {
		CURR_RFD_GET (p_rx_curr_desc, queue);
		MV_REG_WRITE ((MV64460_ETH_RX_CURRENT_QUEUE_DESC_PTR_0
			       (eth_port_num)
			       + (4 * queue)),
			      ((unsigned int) p_rx_curr_desc));

		if (p_rx_curr_desc != NULL)
			/* Add the assigned Ethernet address to the port's address table */
			eth_port_uc_addr_set (p_eth_port_ctrl->port_num,
					      p_eth_port_ctrl->port_mac_addr,
					      queue);
	}

	/* Assign port configuration and command. */
	MV_REG_WRITE (MV64460_ETH_PORT_CONFIG_REG (eth_port_num),
		      p_eth_port_ctrl->port_config);

	MV_REG_WRITE (MV64460_ETH_PORT_CONFIG_EXTEND_REG (eth_port_num),
		      p_eth_port_ctrl->port_config_extend);

	MV_REG_WRITE (MV64460_ETH_PORT_SERIAL_CONTROL_REG (eth_port_num),
		      p_eth_port_ctrl->port_serial_control);

	MV_SET_REG_BITS (MV64460_ETH_PORT_SERIAL_CONTROL_REG (eth_port_num),
			 ETH_SERIAL_PORT_ENABLE);

	/* Assign port SDMA configuration */
	MV_REG_WRITE (MV64460_ETH_SDMA_CONFIG_REG (eth_port_num),
		      p_eth_port_ctrl->port_sdma_config);

	MV_REG_WRITE (MV64460_ETH_TX_QUEUE_0_TOKEN_BUCKET_COUNT
		      (eth_port_num), 0x3fffffff);
	MV_REG_WRITE (MV64460_ETH_TX_QUEUE_0_TOKEN_BUCKET_CONFIG
		      (eth_port_num), 0x03fffcff);
	/* Turn off the port/queue bandwidth limitation */
	MV_REG_WRITE (MV64460_ETH_MAXIMUM_TRANSMIT_UNIT (eth_port_num), 0x0);

	/* Enable port Rx. */
	MV_REG_WRITE (MV64460_ETH_RECEIVE_QUEUE_COMMAND_REG (eth_port_num),
		      p_eth_port_ctrl->port_rx_queue_command);

	/* Check if link is up */
	eth_port_read_smi_reg (eth_port_num, 1, &phy_reg_data);

	if (!(phy_reg_data & 0x20))
		return false;

	return true;
}

/*******************************************************************************
* eth_port_uc_addr_set - This function Set the port Unicast address.
*
* DESCRIPTION:
*		This function Set the port Ethernet MAC address.
*
* INPUT:
*	ETH_PORT eth_port_num	  Port number.
*	char *	      p_addr		Address to be set
*	ETH_QUEUE	  queue		Rx queue number for this MAC address.
*
* OUTPUT:
*	Set MAC address low and high registers. also calls eth_port_uc_addr()
*	To set the unicast table with the proper information.
*
* RETURN:
*	N/A.
*
*******************************************************************************/
static void eth_port_uc_addr_set (ETH_PORT eth_port_num,
				  unsigned char *p_addr, ETH_QUEUE queue)
{
	unsigned int mac_h;
	unsigned int mac_l;

	mac_l = (p_addr[4] << 8) | (p_addr[5]);
	mac_h = (p_addr[0] << 24) | (p_addr[1] << 16) |
		(p_addr[2] << 8) | (p_addr[3] << 0);

	MV_REG_WRITE (MV64460_ETH_MAC_ADDR_LOW (eth_port_num), mac_l);
	MV_REG_WRITE (MV64460_ETH_MAC_ADDR_HIGH (eth_port_num), mac_h);

	/* Accept frames of this address */
	eth_port_uc_addr (eth_port_num, p_addr[5], queue, ACCEPT_MAC_ADDR);

	return;
}

/*******************************************************************************
* eth_port_uc_addr - This function Set the port unicast address table
*
* DESCRIPTION:
*	This function locates the proper entry in the Unicast table for the
*	specified MAC nibble and sets its properties according to function
*	parameters.
*
* INPUT:
*	ETH_PORT	eth_port_num	  Port number.
*	unsigned char uc_nibble		Unicast MAC Address last nibble.
*	ETH_QUEUE		 queue		Rx queue number for this MAC address.
*	int			option	    0 = Add, 1 = remove address.
*
* OUTPUT:
*	This function add/removes MAC addresses from the port unicast address
*	table.
*
* RETURN:
*	true is output succeeded.
*	false if option parameter is invalid.
*
*******************************************************************************/
static bool eth_port_uc_addr (ETH_PORT eth_port_num,
			      unsigned char uc_nibble,
			      ETH_QUEUE queue, int option)
{
	unsigned int unicast_reg;
	unsigned int tbl_offset;
	unsigned int reg_offset;

	/* Locate the Unicast table entry */
	uc_nibble = (0xf & uc_nibble);
	tbl_offset = (uc_nibble / 4) * 4;	/* Register offset from unicast table base */
	reg_offset = uc_nibble % 4;	/* Entry offset within the above register */

	switch (option) {
	case REJECT_MAC_ADDR:
		/* Clear accepts frame bit at specified unicast DA table entry */
		unicast_reg =
			MV_REG_READ ((MV64460_ETH_DA_FILTER_UNICAST_TABLE_BASE
				      (eth_port_num)
				      + tbl_offset));

		unicast_reg &= (0x0E << (8 * reg_offset));

		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_UNICAST_TABLE_BASE
			       (eth_port_num)
			       + tbl_offset), unicast_reg);
		break;

	case ACCEPT_MAC_ADDR:
		/* Set accepts frame bit at unicast DA filter table entry */
		unicast_reg =
			MV_REG_READ ((MV64460_ETH_DA_FILTER_UNICAST_TABLE_BASE
				      (eth_port_num)
				      + tbl_offset));

		unicast_reg |= ((0x01 | queue) << (8 * reg_offset));

		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_UNICAST_TABLE_BASE
			       (eth_port_num)
			       + tbl_offset), unicast_reg);

		break;

	default:
		return false;
	}
	return true;
}

#if 0				/* FIXME */
/*******************************************************************************
* eth_port_mc_addr - Multicast address settings.
*
* DESCRIPTION:
*	This API controls the MV device MAC multicast support.
*	The MV device supports multicast using two tables:
*	1) Special Multicast Table for MAC addresses of the form
*	   0x01-00-5E-00-00-XX (where XX is between 0x00 and 0x_fF).
*	   The MAC DA[7:0] bits are used as a pointer to the Special Multicast
*	   Table entries in the DA-Filter table.
*	   In this case, the function calls eth_port_smc_addr() routine to set the
*	   Special Multicast Table.
*	2) Other Multicast Table for multicast of another type. A CRC-8bit
*	   is used as an index to the Other Multicast Table entries in the
*	   DA-Filter table.
*	   In this case, the function calculates the CRC-8bit value and calls
*	   eth_port_omc_addr() routine to set the Other Multicast Table.
* INPUT:
*	ETH_PORT	eth_port_num	  Port number.
*	unsigned char	*p_addr		Unicast MAC Address.
*	ETH_QUEUE		 queue		Rx queue number for this MAC address.
*	int			option	    0 = Add, 1 = remove address.
*
* OUTPUT:
*	See description.
*
* RETURN:
*	true is output succeeded.
*	false if add_address_table_entry( ) failed.
*
*******************************************************************************/
static void eth_port_mc_addr (ETH_PORT eth_port_num,
			      unsigned char *p_addr,
			      ETH_QUEUE queue, int option)
{
	unsigned int mac_h;
	unsigned int mac_l;
	unsigned char crc_result = 0;
	int mac_array[48];
	int crc[8];
	int i;

	if ((p_addr[0] == 0x01) &&
	    (p_addr[1] == 0x00) &&
	    (p_addr[2] == 0x5E) && (p_addr[3] == 0x00) && (p_addr[4] == 0x00)) {

		eth_port_smc_addr (eth_port_num, p_addr[5], queue, option);
	} else {
		/* Calculate CRC-8 out of the given address */
		mac_h = (p_addr[0] << 8) | (p_addr[1]);
		mac_l = (p_addr[2] << 24) | (p_addr[3] << 16) |
			(p_addr[4] << 8) | (p_addr[5] << 0);

		for (i = 0; i < 32; i++)
			mac_array[i] = (mac_l >> i) & 0x1;
		for (i = 32; i < 48; i++)
			mac_array[i] = (mac_h >> (i - 32)) & 0x1;

		crc[0] = mac_array[45] ^ mac_array[43] ^ mac_array[40] ^
			mac_array[39] ^ mac_array[35] ^ mac_array[34] ^
			mac_array[31] ^ mac_array[30] ^ mac_array[28] ^
			mac_array[23] ^ mac_array[21] ^ mac_array[19] ^
			mac_array[18] ^ mac_array[16] ^ mac_array[14] ^
			mac_array[12] ^ mac_array[8] ^ mac_array[7] ^
			mac_array[6] ^ mac_array[0];

		crc[1] = mac_array[46] ^ mac_array[45] ^ mac_array[44] ^
			mac_array[43] ^ mac_array[41] ^ mac_array[39] ^
			mac_array[36] ^ mac_array[34] ^ mac_array[32] ^
			mac_array[30] ^ mac_array[29] ^ mac_array[28] ^
			mac_array[24] ^ mac_array[23] ^ mac_array[22] ^
			mac_array[21] ^ mac_array[20] ^ mac_array[18] ^
			mac_array[17] ^ mac_array[16] ^ mac_array[15] ^
			mac_array[14] ^ mac_array[13] ^ mac_array[12] ^
			mac_array[9] ^ mac_array[6] ^ mac_array[1] ^
			mac_array[0];

		crc[2] = mac_array[47] ^ mac_array[46] ^ mac_array[44] ^
			mac_array[43] ^ mac_array[42] ^ mac_array[39] ^
			mac_array[37] ^ mac_array[34] ^ mac_array[33] ^
			mac_array[29] ^ mac_array[28] ^ mac_array[25] ^
			mac_array[24] ^ mac_array[22] ^ mac_array[17] ^
			mac_array[15] ^ mac_array[13] ^ mac_array[12] ^
			mac_array[10] ^ mac_array[8] ^ mac_array[6] ^
			mac_array[2] ^ mac_array[1] ^ mac_array[0];

		crc[3] = mac_array[47] ^ mac_array[45] ^ mac_array[44] ^
			mac_array[43] ^ mac_array[40] ^ mac_array[38] ^
			mac_array[35] ^ mac_array[34] ^ mac_array[30] ^
			mac_array[29] ^ mac_array[26] ^ mac_array[25] ^
			mac_array[23] ^ mac_array[18] ^ mac_array[16] ^
			mac_array[14] ^ mac_array[13] ^ mac_array[11] ^
			mac_array[9] ^ mac_array[7] ^ mac_array[3] ^
			mac_array[2] ^ mac_array[1];

		crc[4] = mac_array[46] ^ mac_array[45] ^ mac_array[44] ^
			mac_array[41] ^ mac_array[39] ^ mac_array[36] ^
			mac_array[35] ^ mac_array[31] ^ mac_array[30] ^
			mac_array[27] ^ mac_array[26] ^ mac_array[24] ^
			mac_array[19] ^ mac_array[17] ^ mac_array[15] ^
			mac_array[14] ^ mac_array[12] ^ mac_array[10] ^
			mac_array[8] ^ mac_array[4] ^ mac_array[3] ^
			mac_array[2];

		crc[5] = mac_array[47] ^ mac_array[46] ^ mac_array[45] ^
			mac_array[42] ^ mac_array[40] ^ mac_array[37] ^
			mac_array[36] ^ mac_array[32] ^ mac_array[31] ^
			mac_array[28] ^ mac_array[27] ^ mac_array[25] ^
			mac_array[20] ^ mac_array[18] ^ mac_array[16] ^
			mac_array[15] ^ mac_array[13] ^ mac_array[11] ^
			mac_array[9] ^ mac_array[5] ^ mac_array[4] ^
			mac_array[3];

		crc[6] = mac_array[47] ^ mac_array[46] ^ mac_array[43] ^
			mac_array[41] ^ mac_array[38] ^ mac_array[37] ^
			mac_array[33] ^ mac_array[32] ^ mac_array[29] ^
			mac_array[28] ^ mac_array[26] ^ mac_array[21] ^
			mac_array[19] ^ mac_array[17] ^ mac_array[16] ^
			mac_array[14] ^ mac_array[12] ^ mac_array[10] ^
			mac_array[6] ^ mac_array[5] ^ mac_array[4];

		crc[7] = mac_array[47] ^ mac_array[44] ^ mac_array[42] ^
			mac_array[39] ^ mac_array[38] ^ mac_array[34] ^
			mac_array[33] ^ mac_array[30] ^ mac_array[29] ^
			mac_array[27] ^ mac_array[22] ^ mac_array[20] ^
			mac_array[18] ^ mac_array[17] ^ mac_array[15] ^
			mac_array[13] ^ mac_array[11] ^ mac_array[7] ^
			mac_array[6] ^ mac_array[5];

		for (i = 0; i < 8; i++)
			crc_result = crc_result | (crc[i] << i);

		eth_port_omc_addr (eth_port_num, crc_result, queue, option);
	}
	return;
}

/*******************************************************************************
* eth_port_smc_addr - Special Multicast address settings.
*
* DESCRIPTION:
*	This routine controls the MV device special MAC multicast support.
*	The Special Multicast Table for MAC addresses supports MAC of the form
*	0x01-00-5E-00-00-XX (where XX is between 0x00 and 0x_fF).
*	The MAC DA[7:0] bits are used as a pointer to the Special Multicast
*	Table entries in the DA-Filter table.
*	This function set the Special Multicast Table appropriate entry
*	according to the argument given.
*
* INPUT:
*	ETH_PORT	eth_port_num	  Port number.
*	unsigned char	mc_byte		Multicast addr last byte (MAC DA[7:0] bits).
*	ETH_QUEUE		 queue		Rx queue number for this MAC address.
*	int			option	    0 = Add, 1 = remove address.
*
* OUTPUT:
*	See description.
*
* RETURN:
*	true is output succeeded.
*	false if option parameter is invalid.
*
*******************************************************************************/
static bool eth_port_smc_addr (ETH_PORT eth_port_num,
			       unsigned char mc_byte,
			       ETH_QUEUE queue, int option)
{
	unsigned int smc_table_reg;
	unsigned int tbl_offset;
	unsigned int reg_offset;

	/* Locate the SMC table entry */
	tbl_offset = (mc_byte / 4) * 4; /* Register offset from SMC table base */
	reg_offset = mc_byte % 4;	/* Entry offset within the above register */
	queue &= 0x7;

	switch (option) {
	case REJECT_MAC_ADDR:
		/* Clear accepts frame bit at specified Special DA table entry */
		smc_table_reg =
			MV_REG_READ ((MV64460_ETH_DA_FILTER_SPECIAL_MULTICAST_TABLE_BASE (eth_port_num) + tbl_offset));
		smc_table_reg &= (0x0E << (8 * reg_offset));

		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_SPECIAL_MULTICAST_TABLE_BASE (eth_port_num) + tbl_offset), smc_table_reg);
		break;

	case ACCEPT_MAC_ADDR:
		/* Set accepts frame bit at specified Special DA table entry */
		smc_table_reg =
			MV_REG_READ ((MV64460_ETH_DA_FILTER_SPECIAL_MULTICAST_TABLE_BASE (eth_port_num) + tbl_offset));
		smc_table_reg |= ((0x01 | queue) << (8 * reg_offset));

		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_SPECIAL_MULTICAST_TABLE_BASE (eth_port_num) + tbl_offset), smc_table_reg);
		break;

	default:
		return false;
	}
	return true;
}

/*******************************************************************************
* eth_port_omc_addr - Multicast address settings.
*
* DESCRIPTION:
*	This routine controls the MV device Other MAC multicast support.
*	The Other Multicast Table is used for multicast of another type.
*	A CRC-8bit is used as an index to the Other Multicast Table entries
*	in the DA-Filter table.
*	The function gets the CRC-8bit value from the calling routine and
*      set the Other Multicast Table appropriate entry according to the
*	CRC-8 argument given.
*
* INPUT:
*	ETH_PORT	eth_port_num	  Port number.
*	unsigned char	  crc8		A CRC-8bit (Polynomial: x^8+x^2+x^1+1).
*	ETH_QUEUE		 queue		Rx queue number for this MAC address.
*	int			option	    0 = Add, 1 = remove address.
*
* OUTPUT:
*	See description.
*
* RETURN:
*	true is output succeeded.
*	false if option parameter is invalid.
*
*******************************************************************************/
static bool eth_port_omc_addr (ETH_PORT eth_port_num,
			       unsigned char crc8,
			       ETH_QUEUE queue, int option)
{
	unsigned int omc_table_reg;
	unsigned int tbl_offset;
	unsigned int reg_offset;

	/* Locate the OMC table entry */
	tbl_offset = (crc8 / 4) * 4;	/* Register offset from OMC table base */
	reg_offset = crc8 % 4;	/* Entry offset within the above register */
	queue &= 0x7;

	switch (option) {
	case REJECT_MAC_ADDR:
		/* Clear accepts frame bit at specified Other DA table entry */
		omc_table_reg =
			MV_REG_READ ((MV64460_ETH_DA_FILTER_OTHER_MULTICAST_TABLE_BASE (eth_port_num) + tbl_offset));
		omc_table_reg &= (0x0E << (8 * reg_offset));

		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_OTHER_MULTICAST_TABLE_BASE (eth_port_num) + tbl_offset), omc_table_reg);
		break;

	case ACCEPT_MAC_ADDR:
		/* Set accepts frame bit at specified Other DA table entry */
		omc_table_reg =
			MV_REG_READ ((MV64460_ETH_DA_FILTER_OTHER_MULTICAST_TABLE_BASE (eth_port_num) + tbl_offset));
		omc_table_reg |= ((0x01 | queue) << (8 * reg_offset));

		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_OTHER_MULTICAST_TABLE_BASE (eth_port_num) + tbl_offset), omc_table_reg);
		break;

	default:
		return false;
	}
	return true;
}
#endif

/*******************************************************************************
* eth_port_init_mac_tables - Clear all entrance in the UC, SMC and OMC tables
*
* DESCRIPTION:
*	Go through all the DA filter tables (Unicast, Special Multicast & Other
*	Multicast) and set each entry to 0.
*
* INPUT:
*	ETH_PORT    eth_port_num   Ethernet Port number. See ETH_PORT enum.
*
* OUTPUT:
*	Multicast and Unicast packets are rejected.
*
* RETURN:
*	None.
*
*******************************************************************************/
static void eth_port_init_mac_tables (ETH_PORT eth_port_num)
{
	int table_index;

	/* Clear DA filter unicast table (Ex_dFUT) */
	for (table_index = 0; table_index <= 0xC; table_index += 4)
		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_UNICAST_TABLE_BASE
			       (eth_port_num) + table_index), 0);

	for (table_index = 0; table_index <= 0xFC; table_index += 4) {
		/* Clear DA filter special multicast table (Ex_dFSMT) */
		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_SPECIAL_MULTICAST_TABLE_BASE (eth_port_num) + table_index), 0);
		/* Clear DA filter other multicast table (Ex_dFOMT) */
		MV_REG_WRITE ((MV64460_ETH_DA_FILTER_OTHER_MULTICAST_TABLE_BASE (eth_port_num) + table_index), 0);
	}
}

/*******************************************************************************
* eth_clear_mib_counters - Clear all MIB counters
*
* DESCRIPTION:
*	This function clears all MIB counters of a specific ethernet port.
*	A read from the MIB counter will reset the counter.
*
* INPUT:
*	ETH_PORT    eth_port_num   Ethernet Port number. See ETH_PORT enum.
*
* OUTPUT:
*	After reading all MIB counters, the counters resets.
*
* RETURN:
*	MIB counter value.
*
*******************************************************************************/
static void eth_clear_mib_counters (ETH_PORT eth_port_num)
{
	int i;

	/* Perform dummy reads from MIB counters */
	for (i = ETH_MIB_GOOD_OCTETS_RECEIVED_LOW; i < ETH_MIB_LATE_COLLISION;
	     i += 4) {
		(void)MV_REG_READ ((MV64460_ETH_MIB_COUNTERS_BASE
				      (eth_port_num) + i));
	}

	return;
}

/*******************************************************************************
* eth_read_mib_counter - Read a MIB counter
*
* DESCRIPTION:
*	This function reads a MIB counter of a specific ethernet port.
*	NOTE - If read from ETH_MIB_GOOD_OCTETS_RECEIVED_LOW, then the
*	following read must be from ETH_MIB_GOOD_OCTETS_RECEIVED_HIGH
*	register. The same applies for ETH_MIB_GOOD_OCTETS_SENT_LOW and
*	ETH_MIB_GOOD_OCTETS_SENT_HIGH
*
* INPUT:
*	ETH_PORT    eth_port_num   Ethernet Port number. See ETH_PORT enum.
*	unsigned int mib_offset	  MIB counter offset (use ETH_MIB_... macros).
*
* OUTPUT:
*	After reading the MIB counter, the counter resets.
*
* RETURN:
*	MIB counter value.
*
*******************************************************************************/
unsigned int eth_read_mib_counter (ETH_PORT eth_port_num,
				   unsigned int mib_offset)
{
	return (MV_REG_READ (MV64460_ETH_MIB_COUNTERS_BASE (eth_port_num)
			     + mib_offset));
}

/*******************************************************************************
* ethernet_phy_set - Set the ethernet port PHY address.
*
* DESCRIPTION:
*	This routine set the ethernet port PHY address according to given
*	parameter.
*
* INPUT:
*		ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
*
* OUTPUT:
*	Set PHY Address Register with given PHY address parameter.
*
* RETURN:
*	None.
*
*******************************************************************************/
static void ethernet_phy_set (ETH_PORT eth_port_num, int phy_addr)
{
	unsigned int reg_data;

	reg_data = MV_REG_READ (MV64460_ETH_PHY_ADDR_REG);

	reg_data &= ~(0x1F << (5 * eth_port_num));
	reg_data |= (phy_addr << (5 * eth_port_num));

	MV_REG_WRITE (MV64460_ETH_PHY_ADDR_REG, reg_data);

	return;
}

/*******************************************************************************
 * ethernet_phy_get - Get the ethernet port PHY address.
 *
 * DESCRIPTION:
 *	 This routine returns the given ethernet port PHY address.
 *
 * INPUT:
 *		ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *
 * OUTPUT:
 *	 None.
 *
 * RETURN:
 *	 PHY address.
 *
 *******************************************************************************/
static int ethernet_phy_get (ETH_PORT eth_port_num)
{
	unsigned int reg_data;

	reg_data = MV_REG_READ (MV64460_ETH_PHY_ADDR_REG);

	return ((reg_data >> (5 * eth_port_num)) & 0x1f);
}

/***********************************************************/
/* (Re)start autonegotiation				   */
/***********************************************************/
int phy_setup_aneg (char *devname, unsigned char addr)
{
	unsigned short ctl, adv;

	/* Setup standard advertise */
	miiphy_read (devname, addr, MII_ADVERTISE, &adv);
	adv |= (LPA_LPACK | LPA_RFAULT | LPA_100BASE4 |
		LPA_100FULL | LPA_100HALF | LPA_10FULL |
		LPA_10HALF);
	miiphy_write (devname, addr, MII_ADVERTISE, adv);

	miiphy_read (devname, addr, MII_CTRL1000, &adv);
	adv |= (0x0300);
	miiphy_write (devname, addr, MII_CTRL1000, adv);

	/* Start/Restart aneg */
	miiphy_read (devname, addr, MII_BMCR, &ctl);
	ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);
	miiphy_write (devname, addr, MII_BMCR, ctl);

	return 0;
}

/*******************************************************************************
 * ethernet_phy_reset - Reset Ethernet port PHY.
 *
 * DESCRIPTION:
 *	 This routine utilize the SMI interface to reset the ethernet port PHY.
 *	 The routine waits until the link is up again or link up is timeout.
 *
 * INPUT:
 *	ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *
 * OUTPUT:
 *	 The ethernet port PHY renew its link.
 *
 * RETURN:
 *	 None.
 *
 *******************************************************************************/
static bool ethernet_phy_reset (ETH_PORT eth_port_num)
{
	unsigned int time_out = 50;
	unsigned int phy_reg_data;

	eth_port_read_smi_reg (eth_port_num, 20, &phy_reg_data);
	phy_reg_data |= 0x0083; /* Set bit 7 to 1 for different RGMII timing */
	eth_port_write_smi_reg (eth_port_num, 20, phy_reg_data);

	/* Reset the PHY */
	eth_port_read_smi_reg (eth_port_num, 0, &phy_reg_data);
	phy_reg_data |= 0x8000; /* Set bit 15 to reset the PHY */
	eth_port_write_smi_reg (eth_port_num, 0, phy_reg_data);

	/* Poll on the PHY LINK */
	do {
		eth_port_read_smi_reg (eth_port_num, 1, &phy_reg_data);

		if (time_out-- == 0)
			return false;
	}
	while (!(phy_reg_data & 0x20));

	return true;
}

/*******************************************************************************
 * eth_port_reset - Reset Ethernet port
 *
 * DESCRIPTION:
 *	This routine resets the chip by aborting any SDMA engine activity and
 *	clearing the MIB counters. The Receiver and the Transmit unit are in
 *	idle state after this command is performed and the port is disabled.
 *
 * INPUT:
 *	ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *
 * OUTPUT:
 *	 Channel activity is halted.
 *
 * RETURN:
 *	 None.
 *
 *******************************************************************************/
static void eth_port_reset (ETH_PORT eth_port_num)
{
	unsigned int reg_data;

	/* Stop Tx port activity. Check port Tx activity. */
	reg_data =
		MV_REG_READ (MV64460_ETH_TRANSMIT_QUEUE_COMMAND_REG
			     (eth_port_num));

	if (reg_data & 0xFF) {
		/* Issue stop command for active channels only */
		MV_REG_WRITE (MV64460_ETH_TRANSMIT_QUEUE_COMMAND_REG
			      (eth_port_num), (reg_data << 8));

		/* Wait for all Tx activity to terminate. */
		do {
			/* Check port cause register that all Tx queues are stopped */
			reg_data =
				MV_REG_READ
				(MV64460_ETH_TRANSMIT_QUEUE_COMMAND_REG
				 (eth_port_num));
		}
		while (reg_data & 0xFF);
	}

	/* Stop Rx port activity. Check port Rx activity. */
	reg_data =
		MV_REG_READ (MV64460_ETH_RECEIVE_QUEUE_COMMAND_REG
			     (eth_port_num));

	if (reg_data & 0xFF) {
		/* Issue stop command for active channels only */
		MV_REG_WRITE (MV64460_ETH_RECEIVE_QUEUE_COMMAND_REG
			      (eth_port_num), (reg_data << 8));

		/* Wait for all Rx activity to terminate. */
		do {
			/* Check port cause register that all Rx queues are stopped */
			reg_data =
				MV_REG_READ
				(MV64460_ETH_RECEIVE_QUEUE_COMMAND_REG
				 (eth_port_num));
		}
		while (reg_data & 0xFF);
	}

	/* Clear all MIB counters */
	eth_clear_mib_counters (eth_port_num);

	/* Reset the Enable bit in the Configuration Register */
	reg_data =
		MV_REG_READ (MV64460_ETH_PORT_SERIAL_CONTROL_REG
			     (eth_port_num));
	reg_data &= ~ETH_SERIAL_PORT_ENABLE;
	MV_REG_WRITE (MV64460_ETH_PORT_SERIAL_CONTROL_REG (eth_port_num),
		      reg_data);

	return;
}

#if 0				/* Not needed here */
/*******************************************************************************
 * ethernet_set_config_reg - Set specified bits in configuration register.
 *
 * DESCRIPTION:
 *	 This function sets specified bits in the given ethernet
 *	 configuration register.
 *
 * INPUT:
 *	ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *	unsigned int	value	32 bit value.
 *
 * OUTPUT:
 *	The set bits in the value parameter are set in the configuration
 *	register.
 *
 * RETURN:
 *	None.
 *
 *******************************************************************************/
static void ethernet_set_config_reg (ETH_PORT eth_port_num,
				     unsigned int value)
{
	unsigned int eth_config_reg;

	eth_config_reg =
		MV_REG_READ (MV64460_ETH_PORT_CONFIG_REG (eth_port_num));
	eth_config_reg |= value;
	MV_REG_WRITE (MV64460_ETH_PORT_CONFIG_REG (eth_port_num),
		      eth_config_reg);

	return;
}
#endif

#if 0				/* FIXME */
/*******************************************************************************
 * ethernet_reset_config_reg - Reset specified bits in configuration register.
 *
 * DESCRIPTION:
 *	 This function resets specified bits in the given Ethernet
 *	 configuration register.
 *
 * INPUT:
 *	ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *	unsigned int	value	32 bit value.
 *
 * OUTPUT:
 *	The set bits in the value parameter are reset in the configuration
 *	register.
 *
 * RETURN:
 *	None.
 *
 *******************************************************************************/
static void ethernet_reset_config_reg (ETH_PORT eth_port_num,
				       unsigned int value)
{
	unsigned int eth_config_reg;

	eth_config_reg = MV_REG_READ (MV64460_ETH_PORT_CONFIG_EXTEND_REG
				      (eth_port_num));
	eth_config_reg &= ~value;
	MV_REG_WRITE (MV64460_ETH_PORT_CONFIG_EXTEND_REG (eth_port_num),
		      eth_config_reg);

	return;
}
#endif

#if 0				/* Not needed here */
/*******************************************************************************
 * ethernet_get_config_reg - Get the port configuration register
 *
 * DESCRIPTION:
 *	 This function returns the configuration register value of the given
 *	 ethernet port.
 *
 * INPUT:
 *	ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *
 * OUTPUT:
 *	 None.
 *
 * RETURN:
 *	 Port configuration register value.
 *
 *******************************************************************************/
static unsigned int ethernet_get_config_reg (ETH_PORT eth_port_num)
{
	unsigned int eth_config_reg;

	eth_config_reg = MV_REG_READ (MV64460_ETH_PORT_CONFIG_EXTEND_REG
				      (eth_port_num));
	return eth_config_reg;
}

#endif

/*******************************************************************************
 * eth_port_read_smi_reg - Read PHY registers
 *
 * DESCRIPTION:
 *	 This routine utilize the SMI interface to interact with the PHY in
 *	 order to perform PHY register read.
 *
 * INPUT:
 *	ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *	 unsigned int	phy_reg	  PHY register address offset.
 *	 unsigned int	*value	 Register value buffer.
 *
 * OUTPUT:
 *	 Write the value of a specified PHY register into given buffer.
 *
 * RETURN:
 *	 false if the PHY is busy or read data is not in valid state.
 *	 true otherwise.
 *
 *******************************************************************************/
static bool eth_port_read_smi_reg (ETH_PORT eth_port_num,
				   unsigned int phy_reg, unsigned int *value)
{
	unsigned int reg_value;
	unsigned int time_out = PHY_BUSY_TIMEOUT;
	int phy_addr;

	phy_addr = ethernet_phy_get (eth_port_num);

	/* first check that it is not busy */
	do {
		reg_value = MV_REG_READ (MV64460_ETH_SMI_REG);
		if (time_out-- == 0) {
			return false;
		}
	}
	while (reg_value & ETH_SMI_BUSY);

	/* not busy */

	MV_REG_WRITE (MV64460_ETH_SMI_REG,
		      (phy_addr << 16) | (phy_reg << 21) |
		      ETH_SMI_OPCODE_READ);

	time_out = PHY_BUSY_TIMEOUT;	/* initialize the time out var again */

	do {
		reg_value = MV_REG_READ (MV64460_ETH_SMI_REG);
		if (time_out-- == 0) {
			return false;
		}
	}
	while ((reg_value & ETH_SMI_READ_VALID) != ETH_SMI_READ_VALID); /* Bit set equ operation done */

	/* Wait for the data to update in the SMI register */
#define PHY_UPDATE_TIMEOUT	10000
	for (time_out = 0; time_out < PHY_UPDATE_TIMEOUT; time_out++);

	reg_value = MV_REG_READ (MV64460_ETH_SMI_REG);

	*value = reg_value & 0xffff;

	return true;
}

int mv_miiphy_read(const char *devname, unsigned char phy_addr,
		   unsigned char phy_reg, unsigned short *value)
{
	unsigned int reg_value;
	unsigned int time_out = PHY_BUSY_TIMEOUT;

	/* first check that it is not busy */
	do {
		reg_value = MV_REG_READ (MV64460_ETH_SMI_REG);
		if (time_out-- == 0) {
			return false;
		}
	}
	while (reg_value & ETH_SMI_BUSY);

	/* not busy */
	MV_REG_WRITE (MV64460_ETH_SMI_REG,
		      (phy_addr << 16) | (phy_reg << 21) |
		      ETH_SMI_OPCODE_READ);

	time_out = PHY_BUSY_TIMEOUT;	/* initialize the time out var again */

	do {
		reg_value = MV_REG_READ (MV64460_ETH_SMI_REG);
		if (time_out-- == 0) {
			return false;
		}
	}
	while ((reg_value & ETH_SMI_READ_VALID) != ETH_SMI_READ_VALID); /* Bit set equ operation done */

	/* Wait for the data to update in the SMI register */
	for (time_out = 0; time_out < PHY_UPDATE_TIMEOUT; time_out++);

	reg_value = MV_REG_READ (MV64460_ETH_SMI_REG);

	*value = reg_value & 0xffff;

	return 0;
}

/*******************************************************************************
 * eth_port_write_smi_reg - Write to PHY registers
 *
 * DESCRIPTION:
 *	 This routine utilize the SMI interface to interact with the PHY in
 *	 order to perform writes to PHY registers.
 *
 * INPUT:
 *	ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *	unsigned int   phy_reg	 PHY register address offset.
 *	unsigned int	value	Register value.
 *
 * OUTPUT:
 *	Write the given value to the specified PHY register.
 *
 * RETURN:
 *	false if the PHY is busy.
 *	true otherwise.
 *
 *******************************************************************************/
static bool eth_port_write_smi_reg (ETH_PORT eth_port_num,
				    unsigned int phy_reg, unsigned int value)
{
	unsigned int reg_value;
	unsigned int time_out = PHY_BUSY_TIMEOUT;
	int phy_addr;

	phy_addr = ethernet_phy_get (eth_port_num);

	/* first check that it is not busy */
	do {
		reg_value = MV_REG_READ (MV64460_ETH_SMI_REG);
		if (time_out-- == 0) {
			return false;
		}
	}
	while (reg_value & ETH_SMI_BUSY);

	/* not busy */
	MV_REG_WRITE (MV64460_ETH_SMI_REG,
		      (phy_addr << 16) | (phy_reg << 21) |
		      ETH_SMI_OPCODE_WRITE | (value & 0xffff));
	return true;
}

int mv_miiphy_write(const char *devname, unsigned char phy_addr,
		    unsigned char phy_reg, unsigned short value)
{
	unsigned int reg_value;
	unsigned int time_out = PHY_BUSY_TIMEOUT;

	/* first check that it is not busy */
	do {
		reg_value = MV_REG_READ (MV64460_ETH_SMI_REG);
		if (time_out-- == 0) {
			return false;
		}
	}
	while (reg_value & ETH_SMI_BUSY);

	/* not busy */
	MV_REG_WRITE (MV64460_ETH_SMI_REG,
		      (phy_addr << 16) | (phy_reg << 21) |
		      ETH_SMI_OPCODE_WRITE | (value & 0xffff));
	return 0;
}

/*******************************************************************************
 * eth_set_access_control - Config address decode parameters for Ethernet unit
 *
 * DESCRIPTION:
 *	 This function configures the address decode parameters for the Gigabit
 *	 Ethernet Controller according the given parameters struct.
 *
 * INPUT:
 *	ETH_PORT   eth_port_num	  Ethernet Port number. See ETH_PORT enum.
 *	 ETH_WIN_PARAM	*param	 Address decode parameter struct.
 *
 * OUTPUT:
 *	 An access window is opened using the given access parameters.
 *
 * RETURN:
 *	 None.
 *
 *******************************************************************************/
static void eth_set_access_control (ETH_PORT eth_port_num,
				    ETH_WIN_PARAM * param)
{
	unsigned int access_prot_reg;

	/* Set access control register */
	access_prot_reg = MV_REG_READ (MV64460_ETH_ACCESS_PROTECTION_REG
				       (eth_port_num));
	access_prot_reg &= (~(3 << (param->win * 2)));	/* clear window permission */
	access_prot_reg |= (param->access_ctrl << (param->win * 2));
	MV_REG_WRITE (MV64460_ETH_ACCESS_PROTECTION_REG (eth_port_num),
		      access_prot_reg);

	/* Set window Size reg (SR) */
	MV_REG_WRITE ((MV64460_ETH_SIZE_REG_0 +
		       (ETH_SIZE_REG_GAP * param->win)),
		      (((param->size / 0x10000) - 1) << 16));

	/* Set window Base address reg (BA) */
	MV_REG_WRITE ((MV64460_ETH_BAR_0 + (ETH_BAR_GAP * param->win)),
		      (param->target | param->attributes | param->base_addr));
	/* High address remap reg (HARR) */
	if (param->win < 4)
		MV_REG_WRITE ((MV64460_ETH_HIGH_ADDR_REMAP_REG_0 +
			       (ETH_HIGH_ADDR_REMAP_REG_GAP * param->win)),
			      param->high_addr);

	/* Base address enable reg (BARER) */
	if (param->enable == 1)
		MV_RESET_REG_BITS (MV64460_ETH_BASE_ADDR_ENABLE_REG,
				   (1 << param->win));
	else
		MV_SET_REG_BITS (MV64460_ETH_BASE_ADDR_ENABLE_REG,
				 (1 << param->win));
}

/*******************************************************************************
 * ether_init_rx_desc_ring - Curve a Rx chain desc list and buffer in memory.
 *
 * DESCRIPTION:
 *	 This function prepares a Rx chained list of descriptors and packet
 *	 buffers in a form of a ring. The routine must be called after port
 *	 initialization routine and before port start routine.
 *	 The Ethernet SDMA engine uses CPU bus addresses to access the various
 *	 devices in the system (i.e. DRAM). This function uses the ethernet
 *	 struct 'virtual to physical' routine (set by the user) to set the ring
 *	 with physical addresses.
 *
 * INPUT:
 *	ETH_PORT_INFO	*p_eth_port_ctrl   Ethernet Port Control srtuct.
 *	ETH_QUEUE	rx_queue	 Number of Rx queue.
 *	int			rx_desc_num	  Number of Rx descriptors
 *	int			rx_buff_size	  Size of Rx buffer
 *	unsigned int	rx_desc_base_addr  Rx descriptors memory area base addr.
 *	unsigned int	rx_buff_base_addr  Rx buffer memory area base addr.
 *
 * OUTPUT:
 *	The routine updates the Ethernet port control struct with information
 *	regarding the Rx descriptors and buffers.
 *
 * RETURN:
 *	false if the given descriptors memory area is not aligned according to
 *	Ethernet SDMA specifications.
 *	true otherwise.
 *
 *******************************************************************************/
static bool ether_init_rx_desc_ring (ETH_PORT_INFO * p_eth_port_ctrl,
				     ETH_QUEUE rx_queue,
				     int rx_desc_num,
				     int rx_buff_size,
				     unsigned int rx_desc_base_addr,
				     unsigned int rx_buff_base_addr)
{
	ETH_RX_DESC *p_rx_desc;
	ETH_RX_DESC *p_rx_prev_desc;	/* pointer to link with the last descriptor */
	unsigned int buffer_addr;
	int ix;			/* a counter */

	p_rx_desc = (ETH_RX_DESC *) rx_desc_base_addr;
	p_rx_prev_desc = p_rx_desc;
	buffer_addr = rx_buff_base_addr;

	/* Rx desc Must be 4LW aligned (i.e. Descriptor_Address[3:0]=0000). */
	if (rx_buff_base_addr & 0xF)
		return false;

	/* Rx buffers are limited to 64K bytes and Minimum size is 8 bytes  */
	if ((rx_buff_size < 8) || (rx_buff_size > RX_BUFFER_MAX_SIZE))
		return false;

	/* Rx buffers must be 64-bit aligned.	    */
	if ((rx_buff_base_addr + rx_buff_size) & 0x7)
		return false;

	/* initialize the Rx descriptors ring */
	for (ix = 0; ix < rx_desc_num; ix++) {
		p_rx_desc->buf_size = rx_buff_size;
		p_rx_desc->byte_cnt = 0x0000;
		p_rx_desc->cmd_sts =
			ETH_BUFFER_OWNED_BY_DMA | ETH_RX_ENABLE_INTERRUPT;
		p_rx_desc->next_desc_ptr =
			((unsigned int) p_rx_desc) + RX_DESC_ALIGNED_SIZE;
		p_rx_desc->buf_ptr = buffer_addr;
		p_rx_desc->return_info = 0x00000000;
		D_CACHE_FLUSH_LINE (p_rx_desc, 0);
		buffer_addr += rx_buff_size;
		p_rx_prev_desc = p_rx_desc;
		p_rx_desc = (ETH_RX_DESC *)
			((unsigned int) p_rx_desc + RX_DESC_ALIGNED_SIZE);
	}

	/* Closing Rx descriptors ring */
	p_rx_prev_desc->next_desc_ptr = (rx_desc_base_addr);
	D_CACHE_FLUSH_LINE (p_rx_prev_desc, 0);

	/* Save Rx desc pointer to driver struct. */
	CURR_RFD_SET ((ETH_RX_DESC *) rx_desc_base_addr, rx_queue);
	USED_RFD_SET ((ETH_RX_DESC *) rx_desc_base_addr, rx_queue);

	p_eth_port_ctrl->p_rx_desc_area_base[rx_queue] =
		(ETH_RX_DESC *) rx_desc_base_addr;
	p_eth_port_ctrl->rx_desc_area_size[rx_queue] =
		rx_desc_num * RX_DESC_ALIGNED_SIZE;

	p_eth_port_ctrl->port_rx_queue_command |= (1 << rx_queue);

	return true;
}

/*******************************************************************************
 * ether_init_tx_desc_ring - Curve a Tx chain desc list and buffer in memory.
 *
 * DESCRIPTION:
 *	 This function prepares a Tx chained list of descriptors and packet
 *	 buffers in a form of a ring. The routine must be called after port
 *	 initialization routine and before port start routine.
 *	 The Ethernet SDMA engine uses CPU bus addresses to access the various
 *	 devices in the system (i.e. DRAM). This function uses the ethernet
 *	 struct 'virtual to physical' routine (set by the user) to set the ring
 *	 with physical addresses.
 *
 * INPUT:
 *	ETH_PORT_INFO	*p_eth_port_ctrl   Ethernet Port Control srtuct.
 *	ETH_QUEUE	tx_queue	 Number of Tx queue.
 *	int			tx_desc_num	  Number of Tx descriptors
 *	int			tx_buff_size	  Size of Tx buffer
 *	unsigned int	tx_desc_base_addr  Tx descriptors memory area base addr.
 *	unsigned int	tx_buff_base_addr  Tx buffer memory area base addr.
 *
 * OUTPUT:
 *	The routine updates the Ethernet port control struct with information
 *	regarding the Tx descriptors and buffers.
 *
 * RETURN:
 *	false if the given descriptors memory area is not aligned according to
 *	Ethernet SDMA specifications.
 *	true otherwise.
 *
 *******************************************************************************/
static bool ether_init_tx_desc_ring (ETH_PORT_INFO * p_eth_port_ctrl,
				     ETH_QUEUE tx_queue,
				     int tx_desc_num,
				     int tx_buff_size,
				     unsigned int tx_desc_base_addr,
				     unsigned int tx_buff_base_addr)
{

	ETH_TX_DESC *p_tx_desc;
	ETH_TX_DESC *p_tx_prev_desc;
	unsigned int buffer_addr;
	int ix;			/* a counter */

	/* save the first desc pointer to link with the last descriptor */
	p_tx_desc = (ETH_TX_DESC *) tx_desc_base_addr;
	p_tx_prev_desc = p_tx_desc;
	buffer_addr = tx_buff_base_addr;

	/* Tx desc Must be 4LW aligned (i.e. Descriptor_Address[3:0]=0000). */
	if (tx_buff_base_addr & 0xF)
		return false;

	/* Tx buffers are limited to 64K bytes and Minimum size is 8 bytes  */
	if ((tx_buff_size > TX_BUFFER_MAX_SIZE)
	    || (tx_buff_size < TX_BUFFER_MIN_SIZE))
		return false;

	/* Initialize the Tx descriptors ring */
	for (ix = 0; ix < tx_desc_num; ix++) {
		p_tx_desc->byte_cnt = 0x0000;
		p_tx_desc->l4i_chk = 0x0000;
		p_tx_desc->cmd_sts = 0x00000000;
		p_tx_desc->next_desc_ptr =
			((unsigned int) p_tx_desc) + TX_DESC_ALIGNED_SIZE;

		p_tx_desc->buf_ptr = buffer_addr;
		p_tx_desc->return_info = 0x00000000;
		D_CACHE_FLUSH_LINE (p_tx_desc, 0);
		buffer_addr += tx_buff_size;
		p_tx_prev_desc = p_tx_desc;
		p_tx_desc = (ETH_TX_DESC *)
			((unsigned int) p_tx_desc + TX_DESC_ALIGNED_SIZE);

	}
	/* Closing Tx descriptors ring */
	p_tx_prev_desc->next_desc_ptr = tx_desc_base_addr;
	D_CACHE_FLUSH_LINE (p_tx_prev_desc, 0);
	/* Set Tx desc pointer in driver struct. */
	CURR_TFD_SET ((ETH_TX_DESC *) tx_desc_base_addr, tx_queue);
	USED_TFD_SET ((ETH_TX_DESC *) tx_desc_base_addr, tx_queue);

	/* Init Tx ring base and size parameters */
	p_eth_port_ctrl->p_tx_desc_area_base[tx_queue] =
		(ETH_TX_DESC *) tx_desc_base_addr;
	p_eth_port_ctrl->tx_desc_area_size[tx_queue] =
		(tx_desc_num * TX_DESC_ALIGNED_SIZE);

	/* Add the queue to the list of Tx queues of this port */
	p_eth_port_ctrl->port_tx_queue_command |= (1 << tx_queue);

	return true;
}

/*******************************************************************************
 * eth_port_send - Send an Ethernet packet
 *
 * DESCRIPTION:
 *	This routine send a given packet described by p_pktinfo parameter. It
 *	supports transmitting of a packet spaned over multiple buffers. The
 *	routine updates 'curr' and 'first' indexes according to the packet
 *	segment passed to the routine. In case the packet segment is first,
 *	the 'first' index is update. In any case, the 'curr' index is updated.
 *	If the routine get into Tx resource error it assigns 'curr' index as
 *	'first'. This way the function can abort Tx process of multiple
 *	descriptors per packet.
 *
 * INPUT:
 *	ETH_PORT_INFO	*p_eth_port_ctrl   Ethernet Port Control srtuct.
 *	ETH_QUEUE	tx_queue	 Number of Tx queue.
 *	PKT_INFO	*p_pkt_info	  User packet buffer.
 *
 * OUTPUT:
 *	Tx ring 'curr' and 'first' indexes are updated.
 *
 * RETURN:
 *	ETH_QUEUE_FULL in case of Tx resource error.
 *	ETH_ERROR in case the routine can not access Tx desc ring.
 *	ETH_QUEUE_LAST_RESOURCE if the routine uses the last Tx resource.
 *	ETH_OK otherwise.
 *
 *******************************************************************************/
static ETH_FUNC_RET_STATUS eth_port_send (ETH_PORT_INFO * p_eth_port_ctrl,
					  ETH_QUEUE tx_queue,
					  PKT_INFO * p_pkt_info)
{
	volatile ETH_TX_DESC *p_tx_desc_first;
	volatile ETH_TX_DESC *p_tx_desc_curr;
	volatile ETH_TX_DESC *p_tx_next_desc_curr;
	volatile ETH_TX_DESC *p_tx_desc_used;
	unsigned int command_status;

	/* Do not process Tx ring in case of Tx ring resource error */
	if (p_eth_port_ctrl->tx_resource_err[tx_queue] == true)
		return ETH_QUEUE_FULL;

	/* Get the Tx Desc ring indexes */
	CURR_TFD_GET (p_tx_desc_curr, tx_queue);
	USED_TFD_GET (p_tx_desc_used, tx_queue);

	if (p_tx_desc_curr == NULL)
		return ETH_ERROR;

	/* The following parameters are used to save readings from memory */
	p_tx_next_desc_curr = TX_NEXT_DESC_PTR (p_tx_desc_curr, tx_queue);
	command_status = p_pkt_info->cmd_sts | ETH_ZERO_PADDING | ETH_GEN_CRC;

	if (command_status & (ETH_TX_FIRST_DESC)) {
		/* Update first desc */
		FIRST_TFD_SET (p_tx_desc_curr, tx_queue);
		p_tx_desc_first = p_tx_desc_curr;
	} else {
		FIRST_TFD_GET (p_tx_desc_first, tx_queue);
		command_status |= ETH_BUFFER_OWNED_BY_DMA;
	}

	/* Buffers with a payload smaller than 8 bytes must be aligned to 64-bit */
	/* boundary. We use the memory allocated for Tx descriptor. This memory	 */
	/* located in TX_BUF_OFFSET_IN_DESC offset within the Tx descriptor. */
	if (p_pkt_info->byte_cnt <= 8) {
		printf ("You have failed in the < 8 bytes errata - fixme\n");	/* RABEEH - TBD */
		return ETH_ERROR;

		p_tx_desc_curr->buf_ptr =
			(unsigned int) p_tx_desc_curr + TX_BUF_OFFSET_IN_DESC;
		eth_b_copy (p_pkt_info->buf_ptr, p_tx_desc_curr->buf_ptr,
			    p_pkt_info->byte_cnt);
	} else
		p_tx_desc_curr->buf_ptr = p_pkt_info->buf_ptr;

	p_tx_desc_curr->byte_cnt = p_pkt_info->byte_cnt;
	p_tx_desc_curr->return_info = p_pkt_info->return_info;

	if (p_pkt_info->cmd_sts & (ETH_TX_LAST_DESC)) {
		/* Set last desc with DMA ownership and interrupt enable. */
		p_tx_desc_curr->cmd_sts = command_status |
			ETH_BUFFER_OWNED_BY_DMA | ETH_TX_ENABLE_INTERRUPT;

		if (p_tx_desc_curr != p_tx_desc_first)
			p_tx_desc_first->cmd_sts |= ETH_BUFFER_OWNED_BY_DMA;

		/* Flush CPU pipe */

		D_CACHE_FLUSH_LINE ((unsigned int) p_tx_desc_curr, 0);
		D_CACHE_FLUSH_LINE ((unsigned int) p_tx_desc_first, 0);
		CPU_PIPE_FLUSH;

		/* Apply send command */
		ETH_ENABLE_TX_QUEUE (tx_queue, p_eth_port_ctrl->port_num);

		/* Finish Tx packet. Update first desc in case of Tx resource error */
		p_tx_desc_first = p_tx_next_desc_curr;
		FIRST_TFD_SET (p_tx_desc_first, tx_queue);

	} else {
		p_tx_desc_curr->cmd_sts = command_status;
		D_CACHE_FLUSH_LINE ((unsigned int) p_tx_desc_curr, 0);
	}

	/* Check for ring index overlap in the Tx desc ring */
	if (p_tx_next_desc_curr == p_tx_desc_used) {
		/* Update the current descriptor */
		CURR_TFD_SET (p_tx_desc_first, tx_queue);

		p_eth_port_ctrl->tx_resource_err[tx_queue] = true;
		return ETH_QUEUE_LAST_RESOURCE;
	} else {
		/* Update the current descriptor */
		CURR_TFD_SET (p_tx_next_desc_curr, tx_queue);
		return ETH_OK;
	}
}

/*******************************************************************************
 * eth_tx_return_desc - Free all used Tx descriptors
 *
 * DESCRIPTION:
 *	This routine returns the transmitted packet information to the caller.
 *	It uses the 'first' index to support Tx desc return in case a transmit
 *	of a packet spanned over multiple buffer still in process.
 *	In case the Tx queue was in "resource error" condition, where there are
 *	no available Tx resources, the function resets the resource error flag.
 *
 * INPUT:
 *	ETH_PORT_INFO	*p_eth_port_ctrl   Ethernet Port Control srtuct.
 *	ETH_QUEUE	tx_queue	 Number of Tx queue.
 *	PKT_INFO	*p_pkt_info	  User packet buffer.
 *
 * OUTPUT:
 *	Tx ring 'first' and 'used' indexes are updated.
 *
 * RETURN:
 *	ETH_ERROR in case the routine can not access Tx desc ring.
 *	ETH_RETRY in case there is transmission in process.
 *	ETH_END_OF_JOB if the routine has nothing to release.
 *	ETH_OK otherwise.
 *
 *******************************************************************************/
static ETH_FUNC_RET_STATUS eth_tx_return_desc (ETH_PORT_INFO *
					       p_eth_port_ctrl,
					       ETH_QUEUE tx_queue,
					       PKT_INFO * p_pkt_info)
{
	volatile ETH_TX_DESC *p_tx_desc_used = NULL;
	volatile ETH_TX_DESC *p_tx_desc_first = NULL;
	unsigned int command_status;

	/* Get the Tx Desc ring indexes */
	USED_TFD_GET (p_tx_desc_used, tx_queue);
	FIRST_TFD_GET (p_tx_desc_first, tx_queue);

	/* Sanity check */
	if (p_tx_desc_used == NULL)
		return ETH_ERROR;

	command_status = p_tx_desc_used->cmd_sts;

	/* Still transmitting... */
	if (command_status & (ETH_BUFFER_OWNED_BY_DMA)) {
		D_CACHE_FLUSH_LINE ((unsigned int) p_tx_desc_used, 0);
		return ETH_RETRY;
	}

	/* Stop release. About to overlap the current available Tx descriptor */
	if ((p_tx_desc_used == p_tx_desc_first) &&
	    (p_eth_port_ctrl->tx_resource_err[tx_queue] == false)) {
		D_CACHE_FLUSH_LINE ((unsigned int) p_tx_desc_used, 0);
		return ETH_END_OF_JOB;
	}

	/* Pass the packet information to the caller */
	p_pkt_info->cmd_sts = command_status;
	p_pkt_info->return_info = p_tx_desc_used->return_info;
	p_tx_desc_used->return_info = 0;

	/* Update the next descriptor to release. */
	USED_TFD_SET (TX_NEXT_DESC_PTR (p_tx_desc_used, tx_queue), tx_queue);

	/* Any Tx return cancels the Tx resource error status */
	if (p_eth_port_ctrl->tx_resource_err[tx_queue] == true)
		p_eth_port_ctrl->tx_resource_err[tx_queue] = false;

	D_CACHE_FLUSH_LINE ((unsigned int) p_tx_desc_used, 0);

	return ETH_OK;

}

/*******************************************************************************
 * eth_port_receive - Get received information from Rx ring.
 *
 * DESCRIPTION:
 *	This routine returns the received data to the caller. There is no
 *	data copying during routine operation. All information is returned
 *	using pointer to packet information struct passed from the caller.
 *	If the routine exhausts Rx ring resources then the resource error flag
 *	is set.
 *
 * INPUT:
 *	ETH_PORT_INFO	*p_eth_port_ctrl   Ethernet Port Control srtuct.
 *	ETH_QUEUE	rx_queue	 Number of Rx queue.
 *	PKT_INFO	*p_pkt_info	  User packet buffer.
 *
 * OUTPUT:
 *	Rx ring current and used indexes are updated.
 *
 * RETURN:
 *	ETH_ERROR in case the routine can not access Rx desc ring.
 *	ETH_QUEUE_FULL if Rx ring resources are exhausted.
 *	ETH_END_OF_JOB if there is no received data.
 *	ETH_OK otherwise.
 *
 *******************************************************************************/
static ETH_FUNC_RET_STATUS eth_port_receive (ETH_PORT_INFO * p_eth_port_ctrl,
					     ETH_QUEUE rx_queue,
					     PKT_INFO * p_pkt_info)
{
	volatile ETH_RX_DESC *p_rx_curr_desc;
	volatile ETH_RX_DESC *p_rx_next_curr_desc;
	volatile ETH_RX_DESC *p_rx_used_desc;
	unsigned int command_status;

	/* Do not process Rx ring in case of Rx ring resource error */
	if (p_eth_port_ctrl->rx_resource_err[rx_queue] == true) {
		printf ("\nRx Queue is full ...\n");
		return ETH_QUEUE_FULL;
	}

	/* Get the Rx Desc ring 'curr and 'used' indexes */
	CURR_RFD_GET (p_rx_curr_desc, rx_queue);
	USED_RFD_GET (p_rx_used_desc, rx_queue);

	/* Sanity check */
	if (p_rx_curr_desc == NULL)
		return ETH_ERROR;

	/* The following parameters are used to save readings from memory */
	p_rx_next_curr_desc = RX_NEXT_DESC_PTR (p_rx_curr_desc, rx_queue);
	command_status = p_rx_curr_desc->cmd_sts;

	/* Nothing to receive... */
	if (command_status & (ETH_BUFFER_OWNED_BY_DMA)) {
/*	DP(printf("Rx: command_status: %08x\n", command_status)); */
		D_CACHE_FLUSH_LINE ((unsigned int) p_rx_curr_desc, 0);
/*	DP(printf("\nETH_END_OF_JOB ...\n"));*/
		return ETH_END_OF_JOB;
	}

	p_pkt_info->byte_cnt = (p_rx_curr_desc->byte_cnt) - RX_BUF_OFFSET;
	p_pkt_info->cmd_sts = command_status;
	p_pkt_info->buf_ptr = (p_rx_curr_desc->buf_ptr) + RX_BUF_OFFSET;
	p_pkt_info->return_info = p_rx_curr_desc->return_info;
	p_pkt_info->l4i_chk = p_rx_curr_desc->buf_size; /* IP fragment indicator */

	/* Clean the return info field to indicate that the packet has been */
	/* moved to the upper layers					    */
	p_rx_curr_desc->return_info = 0;

	/* Update 'curr' in data structure */
	CURR_RFD_SET (p_rx_next_curr_desc, rx_queue);

	/* Rx descriptors resource exhausted. Set the Rx ring resource error flag */
	if (p_rx_next_curr_desc == p_rx_used_desc)
		p_eth_port_ctrl->rx_resource_err[rx_queue] = true;

	D_CACHE_FLUSH_LINE ((unsigned int) p_rx_curr_desc, 0);
	CPU_PIPE_FLUSH;

	return ETH_OK;
}

/*******************************************************************************
 * eth_rx_return_buff - Returns a Rx buffer back to the Rx ring.
 *
 * DESCRIPTION:
 *	This routine returns a Rx buffer back to the Rx ring. It retrieves the
 *	next 'used' descriptor and attached the returned buffer to it.
 *	In case the Rx ring was in "resource error" condition, where there are
 *	no available Rx resources, the function resets the resource error flag.
 *
 * INPUT:
 *	ETH_PORT_INFO	*p_eth_port_ctrl   Ethernet Port Control srtuct.
 *	ETH_QUEUE	rx_queue	 Number of Rx queue.
 *	PKT_INFO	*p_pkt_info	  Information on the returned buffer.
 *
 * OUTPUT:
 *	New available Rx resource in Rx descriptor ring.
 *
 * RETURN:
 *	ETH_ERROR in case the routine can not access Rx desc ring.
 *	ETH_OK otherwise.
 *
 *******************************************************************************/
static ETH_FUNC_RET_STATUS eth_rx_return_buff (ETH_PORT_INFO *
					       p_eth_port_ctrl,
					       ETH_QUEUE rx_queue,
					       PKT_INFO * p_pkt_info)
{
	volatile ETH_RX_DESC *p_used_rx_desc;	/* Where to return Rx resource */

	/* Get 'used' Rx descriptor */
	USED_RFD_GET (p_used_rx_desc, rx_queue);

	/* Sanity check */
	if (p_used_rx_desc == NULL)
		return ETH_ERROR;

	p_used_rx_desc->buf_ptr = p_pkt_info->buf_ptr;
	p_used_rx_desc->return_info = p_pkt_info->return_info;
	p_used_rx_desc->byte_cnt = p_pkt_info->byte_cnt;
	p_used_rx_desc->buf_size = MV64460_RX_BUFFER_SIZE;	/* Reset Buffer size */

	/* Flush the write pipe */
	CPU_PIPE_FLUSH;

	/* Return the descriptor to DMA ownership */
	p_used_rx_desc->cmd_sts =
		ETH_BUFFER_OWNED_BY_DMA | ETH_RX_ENABLE_INTERRUPT;

	/* Flush descriptor and CPU pipe */
	D_CACHE_FLUSH_LINE ((unsigned int) p_used_rx_desc, 0);
	CPU_PIPE_FLUSH;

	/* Move the used descriptor pointer to the next descriptor */
	USED_RFD_SET (RX_NEXT_DESC_PTR (p_used_rx_desc, rx_queue), rx_queue);

	/* Any Rx return cancels the Rx resource error status */
	if (p_eth_port_ctrl->rx_resource_err[rx_queue] == true)
		p_eth_port_ctrl->rx_resource_err[rx_queue] = false;

	return ETH_OK;
}

/*******************************************************************************
 * eth_port_set_rx_coal - Sets coalescing interrupt mechanism on RX path
 *
 * DESCRIPTION:
 *	This routine sets the RX coalescing interrupt mechanism parameter.
 *	This parameter is a timeout counter, that counts in 64 t_clk
 *	chunks ; that when timeout event occurs a maskable interrupt
 *	occurs.
 *	The parameter is calculated using the tClk of the MV-643xx chip
 *	, and the required delay of the interrupt in usec.
 *
 * INPUT:
 *	ETH_PORT eth_port_num	   Ethernet port number
 *	unsigned int t_clk	  t_clk of the MV-643xx chip in HZ units
 *	unsigned int delay	 Delay in usec
 *
 * OUTPUT:
 *	Interrupt coalescing mechanism value is set in MV-643xx chip.
 *
 * RETURN:
 *	The interrupt coalescing value set in the gigE port.
 *
 *******************************************************************************/
#if 0				/* FIXME */
static unsigned int eth_port_set_rx_coal (ETH_PORT eth_port_num,
					  unsigned int t_clk,
					  unsigned int delay)
{
	unsigned int coal;

	coal = ((t_clk / 1000000) * delay) / 64;
	/* Set RX Coalescing mechanism */
	MV_REG_WRITE (MV64460_ETH_SDMA_CONFIG_REG (eth_port_num),
		      ((coal & 0x3fff) << 8) |
		      (MV_REG_READ
		       (MV64460_ETH_SDMA_CONFIG_REG (eth_port_num))
		       & 0xffc000ff));
	return coal;
}

#endif
/*******************************************************************************
 * eth_port_set_tx_coal - Sets coalescing interrupt mechanism on TX path
 *
 * DESCRIPTION:
 *	This routine sets the TX coalescing interrupt mechanism parameter.
 *	This parameter is a timeout counter, that counts in 64 t_clk
 *	chunks ; that when timeout event occurs a maskable interrupt
 *	occurs.
 *	The parameter is calculated using the t_cLK frequency of the
 *	MV-643xx chip and the required delay in the interrupt in uSec
 *
 * INPUT:
 *	ETH_PORT eth_port_num	   Ethernet port number
 *	unsigned int t_clk	  t_clk of the MV-643xx chip in HZ units
 *	unsigned int delay	 Delay in uSeconds
 *
 * OUTPUT:
 *	Interrupt coalescing mechanism value is set in MV-643xx chip.
 *
 * RETURN:
 *	The interrupt coalescing value set in the gigE port.
 *
 *******************************************************************************/
#if 0				/* FIXME */
static unsigned int eth_port_set_tx_coal (ETH_PORT eth_port_num,
					  unsigned int t_clk,
					  unsigned int delay)
{
	unsigned int coal;

	coal = ((t_clk / 1000000) * delay) / 64;
	/* Set TX Coalescing mechanism */
	MV_REG_WRITE (MV64460_ETH_TX_FIFO_URGENT_THRESHOLD_REG (eth_port_num),
		      coal << 4);
	return coal;
}
#endif

/*******************************************************************************
 * eth_b_copy - Copy bytes from source to destination
 *
 * DESCRIPTION:
 *	 This function supports the eight bytes limitation on Tx buffer size.
 *	 The routine will zero eight bytes starting from the destination address
 *	 followed by copying bytes from the source address to the destination.
 *
 * INPUT:
 *	 unsigned int src_addr	  32 bit source address.
 *	 unsigned int dst_addr	  32 bit destination address.
 *	 int	    byte_count	  Number of bytes to copy.
 *
 * OUTPUT:
 *	 See description.
 *
 * RETURN:
 *	 None.
 *
 *******************************************************************************/
static void eth_b_copy (unsigned int src_addr, unsigned int dst_addr,
			int byte_count)
{
	/* Zero the dst_addr area */
	*(unsigned int *) dst_addr = 0x0;

	while (byte_count != 0) {
		*(char *) dst_addr = *(char *) src_addr;
		dst_addr++;
		src_addr++;
		byte_count--;
	}
}

/**************************************************************************
Etherboot -  BOOTP/TFTP Bootstrap Program
Skeleton NIC driver for Etherboot
***************************************************************************/

/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This file is a modified version from the Galileo polled mode
 * network driver for the ethernet contained within the GT64260
 * chip. It has been modified to fit into the U-Boot framework, from
 * the original (etherboot) setup.  Also, additional cleanup and features
 * were added.
 *
 * - Josh Huber <huber@mclx.com>
 */

#include <common.h>
#include <malloc.h>
#include <galileo/gt64260R.h>
#include <galileo/core.h>
#include <asm/cache.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>

#include "eth.h"
#include "eth_addrtbl.h"

#if defined(CONFIG_CMD_NET)

#define GT6426x_ETH_BUF_SIZE	1536

/* if you like verbose output, turn this on! */
#undef DEBUG

/* Restart autoneg if we detect link is up on phy init. */

/*
 * The GT doc's say that after Rst is deasserted, and the PHY
 * reports autoneg complete, it runs through its autoneg
 * procedures. This doesn't seem to be the case for MII
 * PHY's. To work around this check for link up && autoneg
 * complete when initilizing the port. If they are both set,
 * then restart PHY autoneg. Of course, it may be something
 * completly different.
 */
#ifdef CONFIG_ETHER_PORT_MII
# define RESTART_AUTONEG
#endif

/* do this if you dont want to use snooping */
#define USE_SOFTWARE_CACHE_MANAGEMENT

#ifdef USE_SOFTWARE_CACHE_MANAGEMENT
#define FLUSH_DCACHE(a,b)                if(dcache_status()){clean_dcache_range((u32)(a),(u32)(b));}
#define FLUSH_AND_INVALIDATE_DCACHE(a,b) if(dcache_status()){flush_dcache_range((u32)(a),(u32)(b));}
#define INVALIDATE_DCACHE(a,b)           if(dcache_status()){invalidate_dcache_range((u32)(a),(u32)(b));}
#else
/* bummer - w/o flush, nothing works, even with snooping - FIXME */
/* #define FLUSH_DCACHE(a,b) */
#define FLUSH_DCACHE(a,b)                if(dcache_status()){clean_dcache_range((u32)(a),(u32)(b));}
#define FLUSH_AND_INVALIDATE_DCACHE(a,b)
#define INVALIDATE_DCACHE(a,b)
#endif
struct eth_dev_s {
	eth0_tx_desc_single *eth_tx_desc;
	eth0_rx_desc_single *eth_rx_desc;
	char *eth_tx_buffer;
	char *eth_rx_buffer[NR];
	int tdn, rdn;
	int dev;
	unsigned int reg_base;
};


#ifdef CONFIG_INTEL_LXT97X
/* for intel LXT972 */
static const char ether_port_phy_addr[3]={0,1,2};
#else
static const char ether_port_phy_addr[3]={4,5,6};
#endif

/* MII PHY access routines are common for all i/f, use gal_ent0 */
#define GT6426x_MII_DEVNAME	"gal_enet0"

int gt6426x_miiphy_read(const char *devname, unsigned char phy,
		unsigned char reg, unsigned short *val);

static inline unsigned short
miiphy_read_ret(unsigned short phy, unsigned short reg)
{
    unsigned short val;
    gt6426x_miiphy_read(GT6426x_MII_DEVNAME,phy,reg,&val);
    return val;
}


/**************************************************************************
RESET - Reset adapter
***************************************************************************/
void
gt6426x_eth_reset(void *v)
{
	/*  we should do something here...
	struct eth_device *wp = (struct eth_device *)v;
	struct eth_dev_s *p = wp->priv;
	*/

	printf ("RESET\n");
	/* put the card in its initial state */
}

static void gt6426x_handle_SMI(struct eth_dev_s *p, unsigned int icr)
{
#ifdef DEBUG
    printf("SMI interrupt: ");

    if(icr&0x20000000) {
	printf("SMI done\n");
    }
#endif

    if(icr&0x10000000) {
#ifdef DEBUG
	unsigned int psr;

	psr=GTREGREAD(ETHERNET0_PORT_STATUS_REGISTER + p->reg_base);
	printf("PHY state change:\n"
	       "  GT:%s:%s:%s:%s\n",
		psr & 1 ? "100" : " 10",
		psr & 8 ? " Link" : "nLink",
		psr & 2 ? "FD" : "HD",
		psr & 4 ? " FC" : "nFC");

#ifdef CONFIG_INTEL_LXT97X /* non-standard mii reg (intel lxt972a) */
	{
		unsigned short mii_11;
		mii_11 = miiphy_read_ret(ether_port_phy_addr[p->dev], 0x11);

		printf(" mii:%s:%s:%s:%s %s:%s %s\n",
			mii_11 & (1 << 14) ? "100" : " 10",
			mii_11 & (1 << 10) ? " Link" : "nLink",
			mii_11 & (1 << 9) ? "FD" : "HD",
			mii_11 & (1 << 4) ? " FC" : "nFC",

			mii_11 & (1 << 7) ? "ANc" : "ANnc",
			mii_11 & (1 << 8) ? "AN" : "Manual",
			""
			);
	}
#endif /* CONFIG_INTEL_LXT97X */
#endif /* DEBUG */
    }
}

static int
gt6426x_eth_receive(struct eth_dev_s *p,unsigned int icr)
{
	int eth_len=0;
	char *eth_data;

	eth0_rx_desc_single *rx = &p->eth_rx_desc[(p->rdn)];

	INVALIDATE_DCACHE((unsigned int)rx,(unsigned int)(rx+1));

	if (rx->command_status & 0x80000000) {
		return 0; /* No packet received */
	}

	eth_len = (unsigned int)
		(rx->buff_size_byte_count) & 0x0000ffff;
	eth_data = (char *) p->eth_rx_buffer[p->rdn];

#ifdef DEBUG
	if (eth_len) {
		printf ("%s: Recived %d byte Packet @ 0x%p\n",
			__FUNCTION__, eth_len, eth_data);
	}
#endif
	/*
	 * packet is now in:
	 * eth0_rx_buffer[RDN_ETH0];
	 */

	/* let the upper layer handle the packet */
	NetReceive ((uchar *)eth_data, eth_len);

	rx->buff_size_byte_count = GT6426x_ETH_BUF_SIZE<<16;


	/* GT96100 Owner */
	rx->command_status = 0x80000000;

	FLUSH_DCACHE((unsigned int)rx,(unsigned int)(rx+1));

	p->rdn ++;
	if (p->rdn == NR) {p->rdn = 0;}

	sync();

	/* Start Rx*/
	GT_REG_WRITE (ETHERNET0_SDMA_COMMAND_REGISTER + p->reg_base, 0x00000080);

#ifdef DEBUG
	{
	    int i;
	    for (i=0;i<12;i++) {
		printf(" %02x", eth_data[i]);
	    }
	}
	printf(": %d bytes\n", eth_len);
#endif
	INVALIDATE_DCACHE((unsigned int)eth_data,
		(unsigned int)eth_data+eth_len);
	return eth_len;
}

/**************************************************************************
POLL - look for an rx frame, handle other conditions
***************************************************************************/
int
gt6426x_eth_poll(void *v)
{
	struct eth_device *wp = (struct eth_device *)v;
	struct eth_dev_s *p = wp->priv;
	unsigned int icr=GTREGREAD(ETHERNET0_INTERRUPT_CAUSE_REGISTER + p->reg_base);

	if(icr) {
	    GT_REG_WRITE(ETHERNET0_INTERRUPT_CAUSE_REGISTER +p->reg_base, 0);
#ifdef DEBUG
	    printf("poll got ICR %08x\n", icr);
#endif
	    /* SMI done or PHY state change*/
	    if(icr&0x30000000) gt6426x_handle_SMI(p, icr);
	}
	/* always process. We aren't using RX interrupts */
	return gt6426x_eth_receive(p, icr);
}

/**************************************************************************
TRANSMIT - Transmit a frame
***************************************************************************/
int gt6426x_eth_transmit(void *v, char *p, unsigned int s)
{
	struct eth_device *wp = (struct eth_device *)v;
	struct eth_dev_s *dev = (struct eth_dev_s *)wp->priv;
#ifdef DEBUG
	unsigned int old_command_stat,old_psr;
#endif
	eth0_tx_desc_single *tx = &dev->eth_tx_desc[dev->tdn];

	/* wait for tx to be ready */
	INVALIDATE_DCACHE((unsigned int)tx,(unsigned int)(tx+1));
	while (tx->command_status & 0x80000000) {
	    int i;
	    for(i=0;i<1000;i++);
			INVALIDATE_DCACHE((unsigned int)tx,(unsigned int)(tx+1));
	}

	GT_REG_WRITE (ETHERNET0_CURRENT_TX_DESCRIPTOR_POINTER0 + dev->reg_base,
		      (unsigned int)tx);

#ifdef DEBUG
	printf("copying to tx_buffer [%p], length %x, desc = %p\n",
	       dev->eth_tx_buffer, s, dev->eth_tx_desc);
#endif
	memcpy(dev->eth_tx_buffer, (char *) p, s);

	tx->buff_pointer = (uchar *)dev->eth_tx_buffer;
	tx->bytecount_reserved = ((__u16)s) << 16;

	/*    31 - own
	 *    22 - gencrc
	 * 18:16 - pad, last, first */
	tx->command_status = (1<<31) | (1<<22) | (7<<16);
#if 0
	/* FEr #18 */
	tx->next_desc = NULL;
#else
	tx->next_desc =
		(struct eth0_tx_desc_struct *)
		&dev->eth_tx_desc[(dev->tdn+1)%NT].bytecount_reserved;

	/* cpu owned */
	dev->eth_tx_desc[(dev->tdn+1)%NT].command_status = (7<<16);	/* pad, last, first */
#endif

#ifdef DEBUG
	old_command_stat=tx->command_status,
	old_psr=GTREGREAD(ETHERNET0_PORT_STATUS_REGISTER + dev->reg_base);
#endif

	FLUSH_DCACHE((unsigned int)tx,
		(unsigned int)&dev->eth_tx_desc[(dev->tdn+2)%NT]);

	FLUSH_DCACHE((unsigned int)dev->eth_tx_buffer,(unsigned int)dev->eth_tx_buffer+s);

	GT_REG_WRITE(ETHERNET0_SDMA_COMMAND_REGISTER + dev->reg_base, 0x01000000);

#ifdef DEBUG
	{
	    unsigned int command_stat=0;
	    printf("cmd_stat: %08x PSR: %08x\n", old_command_stat, old_psr);
	    /* wait for tx to be ready */
	    do {
		unsigned int psr=GTREGREAD(ETHERNET0_PORT_STATUS_REGISTER + dev->reg_base);
		command_stat=tx->command_status;
		if(command_stat!=old_command_stat || psr !=old_psr) {
		    printf("cmd_stat: %08x PSR: %08x\n", command_stat, psr);
		    old_command_stat = command_stat;
		    old_psr = psr;
		}
		/* gt6426x_eth0_poll(); */
	    } while (command_stat & 0x80000000);

	    printf("sent %d byte frame\n", s);

	    if((command_stat & (3<<15)) == 3) {
		printf("frame had error (stat=%08x)\n", command_stat);
	    }
	}
#endif
	return 0;
}

/**************************************************************************
DISABLE - Turn off ethernet interface
***************************************************************************/
void
gt6426x_eth_disable(void *v)
{
	struct eth_device *wp = (struct eth_device *)v;
	struct eth_dev_s *p = (struct eth_dev_s *)wp->priv;

	GT_REG_WRITE(ETHERNET0_SDMA_COMMAND_REGISTER + p->reg_base, 0x80008000);
}

/**************************************************************************
MII utilities - write: write to an MII register via SMI
***************************************************************************/
int
gt6426x_miiphy_write(const char *devname, unsigned char phy,
		unsigned char reg, unsigned short data)
{
    unsigned int temp= (reg<<21) | (phy<<16) | data;

    while(GTREGREAD(ETHERNET_SMI_REGISTER) & (1<<28));	/* wait for !Busy */

    GT_REG_WRITE(ETHERNET_SMI_REGISTER, temp);
    return 0;
}

/**************************************************************************
MII utilities - read: read from an MII register via SMI
***************************************************************************/
int
gt6426x_miiphy_read(const char *devname, unsigned char phy,
		unsigned char reg, unsigned short *val)
{
    unsigned int temp= (reg<<21) | (phy<<16) | 1<<26;

    while(GTREGREAD(ETHERNET_SMI_REGISTER) & (1<<28));	/* wait for !Busy */

    GT_REG_WRITE(ETHERNET_SMI_REGISTER, temp);

    while(1) {
	temp=GTREGREAD(ETHERNET_SMI_REGISTER);
	if(temp & (1<<27)) break;		/* wait for ReadValid */
    }
    *val = temp & 0xffff;

    return 0;
}

#ifdef DEBUG
/**************************************************************************
MII utilities - dump mii registers
***************************************************************************/
static void
gt6426x_dump_mii(bd_t *bis, unsigned short phy)
{
	printf("mii reg 0 - 3:   %04x %04x %04x %04x\n",
		miiphy_read_ret(phy, 0x0),
		miiphy_read_ret(phy, 0x1),
		miiphy_read_ret(phy, 0x2),
		miiphy_read_ret(phy, 0x3)
		);
	printf("        4 - 7:   %04x %04x %04x %04x\n",
		miiphy_read_ret(phy, 0x4),
		miiphy_read_ret(phy, 0x5),
		miiphy_read_ret(phy, 0x6),
		miiphy_read_ret(phy, 0x7)
		);
	printf("        8:       %04x\n",
		miiphy_read_ret(phy, 0x8)
		);
	printf("        16-19:   %04x %04x %04x %04x\n",
		miiphy_read_ret(phy, 0x10),
		miiphy_read_ret(phy, 0x11),
		miiphy_read_ret(phy, 0x12),
		miiphy_read_ret(phy, 0x13)
		);
	printf("        20,30:   %04x %04x\n",
		miiphy_read_ret(phy, 20),
		miiphy_read_ret(phy, 30)
		);
}
#endif

#ifdef RESTART_AUTONEG

/* If link is up && autoneg compleate, and if
 * GT and PHY disagree about link capabilitys,
 * restart autoneg - something screwy with FD/HD
 * unless we do this. */
static void
check_phy_state(struct eth_dev_s *p)
{
	int bmsr = miiphy_read_ret(ether_port_phy_addr[p->dev], MII_BMSR);
	int psr = GTREGREAD(ETHERNET0_PORT_STATUS_REGISTER + p->reg_base);

	if ((psr & 1<<3) && (bmsr & BMSR_LSTATUS)) {
		int nego = miiphy_read_ret(ether_port_phy_addr[p->dev], MII_ADVERTISE) &
				miiphy_read_ret(ether_port_phy_addr[p->dev], MII_LPA);
		int want;

		if (nego & LPA_100FULL) {
			want = 0x3;
			printf("MII: 100Base-TX, Full Duplex\n");
		} else if (nego & LPA_100HALF) {
			want = 0x1;
			printf("MII: 100Base-TX, Half Duplex\n");
		} else if (nego & LPA_10FULL) {
			want = 0x2;
			printf("MII: 10Base-T, Full Duplex\n");
		} else if (nego & LPA_10HALF) {
			want = 0x0;
			printf("MII: 10Base-T, Half Duplex\n");
		} else {
			printf("MII: Unknown link-foo! %x\n", nego);
			return;
		}

		if ((psr & 0x3) != want) {
			printf("MII: GT thinks %x, PHY thinks %x, restarting autoneg..\n",
					psr & 0x3, want);
			miiphy_write(GT6426x_MII_DEVNAME,ether_port_phy_addr[p->dev],0,
					miiphy_read_ret(ether_port_phy_addr[p->dev],0) | (1<<9));
			udelay(10000);	/* the EVB's GT takes a while to notice phy
					   went down and up */
		}
	}
}
#endif

/**************************************************************************
PROBE - Look for an adapter, this routine's visible to the outside
***************************************************************************/
int
gt6426x_eth_probe(void *v, bd_t *bis)
{
	struct eth_device *wp = (struct eth_device *)v;
	struct eth_dev_s *p = (struct eth_dev_s *)wp->priv;
	int dev = p->dev;
	unsigned int reg_base = p->reg_base;
	unsigned long temp;
	int i;

	if (( dev < 0 ) || ( dev >= GAL_ETH_DEVS ))
	{	/* This should never happen */
		printf("%s: Invalid device %d\n", __FUNCTION__, dev );
		return 0;
	}

#ifdef DEBUG
	printf ("%s: initializing %s\n", __FUNCTION__, wp->name );
	printf ("\nCOMM_CONTROL = %08x , COMM_CONF = %08x\n",
		GTREGREAD(COMM_UNIT_ARBITER_CONTROL),
		GTREGREAD(COMM_UNIT_ARBITER_CONFIGURATION_REGISTER));
#endif

	/* clear MIB counters */
	for(i=0;i<255; i++)
	    temp=GTREGREAD(ETHERNET0_MIB_COUNTER_BASE + reg_base +i);

#ifdef CONFIG_INTEL_LXT97X
	/* for intel LXT972 */

	/* led 1: 0x1=txact
	   led 2: 0xc=link/rxact
	   led 3: 0x2=rxact (N/C)
	   strch: 0,2=30 ms, enable */
	miiphy_write(GT6426x_MII_DEVNAME,ether_port_phy_addr[p->dev], 20, 0x1c22);

	/* 2.7ns port rise time */
	/*miiphy_write(ether_port_phy_addr[p->dev], 30, 0x0<<10); */
#else
	/* already set up in mpsc.c */
	/*GT_REG_WRITE(MAIN_ROUTING_REGISTER, 0x7ffe38);	/  b400 */

	/* already set up in sdram_init.S... */
	/* MPSC0, MPSC1, RMII */
	/*GT_REG_WRITE(SERIAL_PORT_MULTIPLEX, 0x1102);		/  f010 */
#endif
	GT_REG_WRITE(ETHERNET_PHY_ADDRESS_REGISTER,
	     ether_port_phy_addr[0]     |
	    (ether_port_phy_addr[1]<<5) |
	    (ether_port_phy_addr[2]<<10));			/* 2000 */

	/* 13:12 -   10: 4x64bit burst	(cache line size = 32 bytes)
	 *    9  -    1: RIFB - interrupt on frame boundaries only
	 *  6:7  -   00: big endian rx and tx
	 *  5:2  - 1111: 15 retries */
	GT_REG_WRITE(ETHERNET0_SDMA_CONFIGURATION_REGISTER + reg_base,
		(2<<12) | (1<<9) | (0xf<<2) );			/* 2440 */

#ifndef USE_SOFTWARE_CACHE_MANAGEMENT
	/* enable rx/tx desc/buffer cache snoop */
	GT_REG_READ(ETHERNET_0_ADDRESS_CONTROL_LOW + dev*0x20,
		&temp);						/* f200 */
	temp|= (1<<6)| (1<<14)| (1<<22)| (1<<30);
	GT_REG_WRITE(ETHERNET_0_ADDRESS_CONTROL_LOW + dev*0x20,
		temp);
#endif

	/* 31  28 27  24 23  20 19  16
	 *  0000   0000   0000   0000	[0004]
	 * 15  12 11  8   7  4   3  0
	 *  1000   1101   0000   0000	[4d00]
	 *    20 - 0=MII 1=RMII
	 *    19 - 0=speed autoneg
	 * 15:14 - framesize 1536 (GT6426x_ETH_BUF_SIZE)
	 *    11 - no force link pass
	 *    10 - 1=disable fctl autoneg
	 *     8 - override prio ?? */
	temp = 0x00004d00;
#ifndef CONFIG_ETHER_PORT_MII
	temp |= (1<<20);	/* RMII */
#endif
	/* set En */
	GT_REG_WRITE(ETHERNET0_PORT_CONFIGURATION_EXTEND_REGISTER + reg_base,
		     temp);				/* 2408 */

	/* hardcode E1 also? */
	/* -- according to dox, this is safer due to extra pulldowns? */
	if (dev<2) {
	GT_REG_WRITE(ETHERNET0_PORT_CONFIGURATION_EXTEND_REGISTER + (dev+1) * 0x400,
		     temp);				/* 2408 */
	}

	/* wake up MAC */				 /* 2400 */
	GT_REG_READ(ETHERNET0_PORT_CONFIGURATION_REGISTER + reg_base, &temp);
	temp |= (1<<7);		/* enable port */
#ifdef CONFIG_GT_USE_MAC_HASH_TABLE
	temp |= (1<<12);	/* hash size 1/2k */
#else
	temp |= 1;		/* promisc */
#endif
	GT_REG_WRITE(ETHERNET0_PORT_CONFIGURATION_REGISTER + reg_base, temp);
							/* 2400 */

#ifdef RESTART_AUTONEG
	check_phy_state(p);
#endif

	printf("%s: Waiting for link up..\n", wp->name);
	temp = 10 * 1000;
	/* wait for link back up */
	while(!(GTREGREAD(ETHERNET0_PORT_STATUS_REGISTER + reg_base) & 8)
			&& (--temp > 0)){
	    udelay(1000);	/* wait 1 ms */
	}
	if ( temp == 0) {
		printf("%s: Failed!\n", wp->name);
		return (0);
	}

	printf("%s: OK!\n", wp->name);

	p->tdn = 0;
	p->rdn = 0;
	p->eth_tx_desc[p->tdn].command_status = 0;

	/* Initialize Rx Side */
	for (temp = 0; temp < NR; temp++) {
		p->eth_rx_desc[temp].buff_pointer = (uchar *)p->eth_rx_buffer[temp];
		p->eth_rx_desc[temp].buff_size_byte_count = GT6426x_ETH_BUF_SIZE<<16;

		/* GT96100 Owner */
		p->eth_rx_desc[temp].command_status = 0x80000000;
		p->eth_rx_desc[temp].next_desc =
			(struct eth0_rx_desc_struct *)
			&p->eth_rx_desc[(temp+1)%NR].buff_size_byte_count;
	}

	FLUSH_DCACHE((unsigned int)&p->eth_tx_desc[0],
		     (unsigned int)&p->eth_tx_desc[NR]);
	FLUSH_DCACHE((unsigned int)&p->eth_rx_desc[0],
		     (unsigned int)&p->eth_rx_desc[NR]);

	GT_REG_WRITE(ETHERNET0_CURRENT_TX_DESCRIPTOR_POINTER0 + reg_base,
		      (unsigned int) p->eth_tx_desc);
	GT_REG_WRITE(ETHERNET0_FIRST_RX_DESCRIPTOR_POINTER0 + reg_base,
		      (unsigned int) p->eth_rx_desc);
	GT_REG_WRITE(ETHERNET0_CURRENT_RX_DESCRIPTOR_POINTER0 + reg_base,
		      (unsigned int) p->eth_rx_desc);

#ifdef DEBUG
	printf ("\nRx descriptor pointer is %08x %08x\n",
		GTREGREAD(ETHERNET0_FIRST_RX_DESCRIPTOR_POINTER0 + reg_base),
		GTREGREAD(ETHERNET0_CURRENT_RX_DESCRIPTOR_POINTER0 + reg_base));
	printf ("\n\n%08x %08x\n",
		(unsigned int)p->eth_rx_desc,p->eth_rx_desc[0].command_status);

	printf ("Descriptor dump:\n");
	printf ("cmd status: %08x\n",p->eth_rx_desc[0].command_status);
	printf ("byte_count: %08x\n",p->eth_rx_desc[0].buff_size_byte_count);
	printf ("buff_ptr: %08x\n",(unsigned int)p->eth_rx_desc[0].buff_pointer);
	printf ("next_desc: %08x\n\n",(unsigned int)p->eth_rx_desc[0].next_desc);
	printf ("%08x\n",*(unsigned int *) ((unsigned int)p->eth_rx_desc + 0x0));
	printf ("%08x\n",*(unsigned int *) ((unsigned int)p->eth_rx_desc + 0x4));
	printf ("%08x\n",*(unsigned int *) ((unsigned int)p->eth_rx_desc + 0x8));
	printf ("%08x\n\n",
		*(unsigned int *) ((unsigned int)p->eth_rx_desc + 0xc));
#endif

#ifdef DEBUG
	gt6426x_dump_mii(bis,ether_port_phy_addr[p->dev]);
#endif

#ifdef CONFIG_GT_USE_MAC_HASH_TABLE
	{
		unsigned int hashtable_base;
	    u8 *b = (u8 *)(wp->enetaddr);
		u32 macH, macL;

		/* twist the MAC up into the way the discovery wants it */
		macH= (b[0]<<8) | b[1];
	    macL= (b[2]<<24) | (b[3]<<16) | (b[4]<<8) | b[5];

	    /* mode 0, size 0x800 */
	    hashtable_base =initAddressTable(dev,0,1);

	    if(!hashtable_base) {
			printf("initAddressTable failed\n");
			return 0;
	    }

	    addAddressTableEntry(dev, macH, macL, 1, 0);
	    GT_REG_WRITE(ETHERNET0_HASH_TABLE_POINTER_REGISTER + reg_base,
		    hashtable_base);
	}
#endif

	/* Start Rx*/
	GT_REG_WRITE(ETHERNET0_SDMA_COMMAND_REGISTER + reg_base, 0x00000080);
	printf("%s: gt6426x eth device %d init success \n", wp->name, dev );
	return 1;
}

/* enter all the galileo ethernet devs into MULTI-BOOT */
void
gt6426x_eth_initialize(bd_t *bis)
{
	struct eth_device *dev;
	struct eth_dev_s *p;
	int devnum, x, temp;
	char *s, *e, buf[64];

#ifdef DEBUG
	printf( "\n%s\n", __FUNCTION );
#endif

	for (devnum = 0; devnum < GAL_ETH_DEVS; devnum++) {
		dev = calloc(sizeof(*dev), 1);
		if (!dev) {
			printf( "%s: gal_enet%d allocation failure, %s\n",
					__FUNCTION__, devnum, "eth_device structure");
			return;
		}

		/* must be less than sizeof(dev->name) */
		sprintf(dev->name, "gal_enet%d", devnum);

#ifdef DEBUG
		printf( "Initializing %s\n", dev->name );
#endif

		/* Extract the MAC address from the environment */
		switch (devnum)
		{
			case 0: s = "ethaddr"; break;
#if (GAL_ETH_DEVS > 1)
			case 1: s = "eth1addr"; break;
#endif
#if (GAL_ETH_DEVS > 2)
			case 2: s = "eth2addr";	break;
#endif
			default: /* this should never happen */
				printf( "%s: Invalid device number %d\n",
						__FUNCTION__, devnum );
				return;
		}

		temp = getenv_f(s, buf, sizeof(buf));
		s = (temp > 0) ? buf : NULL;

#ifdef DEBUG
		printf ("Setting MAC %d to %s\n", devnum, s );
#endif
		for (x = 0; x < 6; ++x) {
			dev->enetaddr[x] = s ? simple_strtoul(s, &e, 16) : 0;
			if (s)
				s = (*e) ? e+1 : e;
		}

		dev->init = (void*)gt6426x_eth_probe;
		dev->halt = (void*)gt6426x_eth_reset;
		dev->send = (void*)gt6426x_eth_transmit;
		dev->recv = (void*)gt6426x_eth_poll;

		p = calloc( sizeof(*p), 1 );
		dev->priv = (void*)p;
		if (!p)
		{
			printf( "%s: %s allocation failure, %s\n",
					__FUNCTION__, dev->name, "Private Device Structure");
			free(dev);
			return;
		}

		p->dev = devnum;
		p->tdn=0;
		p->rdn=0;
		p->reg_base = devnum * ETHERNET_PORTS_DIFFERENCE_OFFSETS;

		p->eth_tx_desc =
			(eth0_tx_desc_single *)
			(((unsigned int) malloc(sizeof (eth0_tx_desc_single) *
						(NT+1)) & 0xfffffff0) + 0x10);
		if (!p)
		{
			printf( "%s: %s allocation failure, %s\n",
					__FUNCTION__, dev->name, "Tx Descriptor");
			free(dev);
			return;
		}

		p->eth_rx_desc =
			(eth0_rx_desc_single *)
			(((unsigned int) malloc(sizeof (eth0_rx_desc_single) *
						(NR+1)) & 0xfffffff0) + 0x10);
		if (!p->eth_rx_desc)
		{
			printf( "%s: %s allocation failure, %s\n",
					__FUNCTION__, dev->name, "Rx Descriptor");
			free(dev);
			free(p);
			return;
		}

		p->eth_tx_buffer =
			(char *) (((unsigned int) malloc(GT6426x_ETH_BUF_SIZE) & 0xfffffff0) + 0x10);
		if (!p->eth_tx_buffer)
		{
			printf( "%s: %s allocation failure, %s\n",
					__FUNCTION__, dev->name, "Tx Bufffer");
			free(dev);
			free(p);
			free(p->eth_rx_desc);
			return;
		}

		for (temp = 0 ; temp < NR ; temp ++) {
			p->eth_rx_buffer[temp] =
				(char *)
				(((unsigned int) malloc(GT6426x_ETH_BUF_SIZE) & 0xfffffff0) + 0x10);
			if (!p->eth_rx_buffer[temp])
			{
				printf( "%s: %s allocation failure, %s\n",
						__FUNCTION__, dev->name, "Rx Buffers");
				free(dev);
				free(p);
				free(p->eth_tx_buffer);
				free(p->eth_rx_desc);
				free(p->eth_tx_desc);
				while (temp >= 0)
					free(p->eth_rx_buffer[--temp]);
				return;
			}
		}


		eth_register(dev);
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
		miiphy_register(dev->name,
				gt6426x_miiphy_read, gt6426x_miiphy_write);
#endif
	}

}
#endif

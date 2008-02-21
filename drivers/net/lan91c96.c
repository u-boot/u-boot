/*------------------------------------------------------------------------
 * lan91c96.c
 * This is a driver for SMSC's LAN91C96 single-chip Ethernet device, based
 * on the SMC91111 driver from U-boot.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Rolf Offermanns <rof@sysgo.de>
 *
 * Copyright (C) 2001 Standard Microsystems Corporation (SMSC)
 *       Developed by Simple Network Magic Corporation (SNMC)
 * Copyright (C) 1996 by Erik Stahlman (ES)
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
 *
 * Information contained in this file was obtained from the LAN91C96
 * manual from SMC.  To get a copy, if you really want one, you can find
 * information under www.smsc.com.
 *
 *
 * "Features" of the SMC chip:
 *   6144 byte packet memory. ( for the 91C96 )
 *   EEPROM for configuration
 *   AUI/TP selection  ( mine has 10Base2/10BaseT select )
 *
 * Arguments:
 *	io	= for the base address
 *	irq	= for the IRQ
 *
 * author:
 *	Erik Stahlman				( erik@vt.edu )
 *	Daris A Nevil				( dnevil@snmc.com )
 *
 *
 * Hardware multicast code from Peter Cammaert ( pc@denkart.be )
 *
 * Sources:
 *    o   SMSC LAN91C96 databook (www.smsc.com)
 *    o   smc91111.c (u-boot driver)
 *    o   smc9194.c (linux kernel driver)
 *    o   lan91c96.c (Intel Diagnostic Manager driver)
 *
 * History:
 *	04/30/03  Mathijs Haarman	Modified smc91111.c (u-boot version)
 *					for lan91c96
 *---------------------------------------------------------------------------
 */

#include <common.h>
#include <command.h>
#include "lan91c96.h"
#include <net.h>

#ifdef CONFIG_DRIVER_LAN91C96

#if defined(CONFIG_CMD_NET)

/*------------------------------------------------------------------------
 *
 * Configuration options, for the experienced user to change.
 *
 -------------------------------------------------------------------------*/

/* Use power-down feature of the chip */
#define POWER_DOWN	0

/*
 * Wait time for memory to be free.  This probably shouldn't be
 * tuned that much, as waiting for this means nothing else happens
 * in the system
*/
#define MEMORY_WAIT_TIME 16

#define SMC_DEBUG 0

#if (SMC_DEBUG > 2 )
#define PRINTK3(args...) printf(args)
#else
#define PRINTK3(args...)
#endif

#if SMC_DEBUG > 1
#define PRINTK2(args...) printf(args)
#else
#define PRINTK2(args...)
#endif

#ifdef SMC_DEBUG
#define PRINTK(args...) printf(args)
#else
#define PRINTK(args...)
#endif


/*------------------------------------------------------------------------
 *
 * The internal workings of the driver.  If you are changing anything
 * here with the SMC stuff, you should have the datasheet and know
 * what you are doing.
 *
 *------------------------------------------------------------------------
 */
#define CARDNAME "LAN91C96"

#define SMC_BASE_ADDRESS CONFIG_LAN91C96_BASE

#define SMC_DEV_NAME "LAN91C96"
#define SMC_ALLOC_MAX_TRY 5
#define SMC_TX_TIMEOUT 30

#define ETH_ZLEN 60

#ifdef  CONFIG_LAN91C96_USE_32_BIT
#define USE_32_BIT  1
#else
#undef USE_32_BIT
#endif

/*-----------------------------------------------------------------
 *
 *  The driver can be entered at any of the following entry points.
 *
 *-----------------------------------------------------------------
 */

extern int eth_init (bd_t * bd);
extern void eth_halt (void);
extern int eth_rx (void);
extern int eth_send (volatile void *packet, int length);
#if 0
static int smc_hw_init (void);
#endif

/*
 * This is called by  register_netdev().  It is responsible for
 * checking the portlist for the SMC9000 series chipset.  If it finds
 * one, then it will initialize the device, find the hardware information,
 * and sets up the appropriate device parameters.
 * NOTE: Interrupts are *OFF* when this procedure is called.
 *
 * NB:This shouldn't be static since it is referred to externally.
 */
int smc_init (void);

/*
 * This is called by  unregister_netdev().  It is responsible for
 * cleaning up before the driver is finally unregistered and discarded.
 */
void smc_destructor (void);

/*
 * The kernel calls this function when someone wants to use the device,
 * typically 'ifconfig ethX up'.
 */
static int smc_open (bd_t *bd);


/*
 * This is called by the kernel in response to 'ifconfig ethX down'.  It
 * is responsible for cleaning up everything that the open routine
 * does, and maybe putting the card into a powerdown state.
 */
static int smc_close (void);

/*
 * This is a separate procedure to handle the receipt of a packet, to
 * leave the interrupt code looking slightly cleaner
 */
static int smc_rcv (void);

/* See if a MAC address is defined in the current environment. If so use it. If not
 . print a warning and set the environment and other globals with the default.
 . If an EEPROM is present it really should be consulted.
*/
int smc_get_ethaddr(bd_t *bd);
int get_rom_mac(unsigned char *v_rom_mac);

/* ------------------------------------------------------------
 * Internal routines
 * ------------------------------------------------------------
 */

static unsigned char smc_mac_addr[] = { 0xc0, 0x00, 0x00, 0x1b, 0x62, 0x9c };

/*
 * This function must be called before smc_open() if you want to override
 * the default mac address.
 */

void smc_set_mac_addr (const unsigned char *addr)
{
	int i;

	for (i = 0; i < sizeof (smc_mac_addr); i++) {
		smc_mac_addr[i] = addr[i];
	}
}

/*
 * smc_get_macaddr is no longer used. If you want to override the default
 * mac address, call smc_get_mac_addr as a part of the board initialisation.
 */

#if 0
void smc_get_macaddr (byte * addr)
{
	/* MAC ADDRESS AT FLASHBLOCK 1 / OFFSET 0x10 */
	unsigned char *dnp1110_mac = (unsigned char *) (0xE8000000 + 0x20010);
	int i;


	for (i = 0; i < 6; i++) {
		addr[0] = *(dnp1110_mac + 0);
		addr[1] = *(dnp1110_mac + 1);
		addr[2] = *(dnp1110_mac + 2);
		addr[3] = *(dnp1110_mac + 3);
		addr[4] = *(dnp1110_mac + 4);
		addr[5] = *(dnp1110_mac + 5);
	}
}
#endif /* 0 */

/***********************************************
 * Show available memory                       *
 ***********************************************/
void dump_memory_info (void)
{
	word mem_info;
	word old_bank;

	old_bank = SMC_inw (LAN91C96_BANK_SELECT) & 0xF;

	SMC_SELECT_BANK (0);
	mem_info = SMC_inw (LAN91C96_MIR);
	PRINTK2 ("Memory: %4d available\n", (mem_info >> 8) * 2048);

	SMC_SELECT_BANK (old_bank);
}

/*
 * A rather simple routine to print out a packet for debugging purposes.
 */
#if SMC_DEBUG > 2
static void print_packet (byte *, int);
#endif

/* #define tx_done(dev) 1 */


/* this does a soft reset on the device */
static void smc_reset (void);

/* Enable Interrupts, Receive, and Transmit */
static void smc_enable (void);

/* this puts the device in an inactive state */
static void smc_shutdown (void);


static int poll4int (byte mask, int timeout)
{
	int tmo = get_timer (0) + timeout * CFG_HZ;
	int is_timeout = 0;
	word old_bank = SMC_inw (LAN91C96_BANK_SELECT);

	PRINTK2 ("Polling...\n");
	SMC_SELECT_BANK (2);
	while ((SMC_inw (LAN91C96_INT_STATS) & mask) == 0) {
		if (get_timer (0) >= tmo) {
			is_timeout = 1;
			break;
		}
	}

	/* restore old bank selection */
	SMC_SELECT_BANK (old_bank);

	if (is_timeout)
		return 1;
	else
		return 0;
}

/*
 * Function: smc_reset( void )
 * Purpose:
 *	This sets the SMC91111 chip to its normal state, hopefully from whatever
 *	mess that any other DOS driver has put it in.
 *
 * Maybe I should reset more registers to defaults in here?  SOFTRST  should
 * do that for me.
 *
 * Method:
 *	1.  send a SOFT RESET
 *	2.  wait for it to finish
 *	3.  enable autorelease mode
 *	4.  reset the memory management unit
 *	5.  clear all interrupts
 *
*/
static void smc_reset (void)
{
	PRINTK2 ("%s:smc_reset\n", SMC_DEV_NAME);

	/* This resets the registers mostly to defaults, but doesn't
	   affect EEPROM.  That seems unnecessary */
	SMC_SELECT_BANK (0);
	SMC_outw (LAN91C96_RCR_SOFT_RST, LAN91C96_RCR);

	udelay (10);

	/* Disable transmit and receive functionality */
	SMC_outw (0, LAN91C96_RCR);
	SMC_outw (0, LAN91C96_TCR);

	/* set the control register */
	SMC_SELECT_BANK (1);
	SMC_outw (SMC_inw (LAN91C96_CONTROL) | LAN91C96_CTR_BIT_8,
			  LAN91C96_CONTROL);

	/* Disable all interrupts */
	SMC_outb (0, LAN91C96_INT_MASK);
}

/*
 * Function: smc_enable
 * Purpose: let the chip talk to the outside work
 * Method:
 *	1.  Initialize the Memory Configuration Register
 *	2.  Enable the transmitter
 *	3.  Enable the receiver
*/
static void smc_enable ()
{
	PRINTK2 ("%s:smc_enable\n", SMC_DEV_NAME);
	SMC_SELECT_BANK (0);

	/* Initialize the Memory Configuration Register. See page
	   49 of the LAN91C96 data sheet for details. */
	SMC_outw (LAN91C96_MCR_TRANSMIT_PAGES, LAN91C96_MCR);

	/* Initialize the Transmit Control Register */
	SMC_outw (LAN91C96_TCR_TXENA, LAN91C96_TCR);
	/* Initialize the Receive Control Register
	 * FIXME:
	 * The promiscuous bit set because I could not receive ARP reply
	 * packets from the server when I send a ARP request. It only works
	 * when I set the promiscuous bit
	 */
	SMC_outw (LAN91C96_RCR_RXEN | LAN91C96_RCR_PRMS, LAN91C96_RCR);
}

/*
 * Function: smc_shutdown
 * Purpose:  closes down the SMC91xxx chip.
 * Method:
 *	1. zero the interrupt mask
 *	2. clear the enable receive flag
 *	3. clear the enable xmit flags
 *
 * TODO:
 *   (1) maybe utilize power down mode.
 *	Why not yet?  Because while the chip will go into power down mode,
 *	the manual says that it will wake up in response to any I/O requests
 *	in the register space.   Empirical results do not show this working.
 */
static void smc_shutdown ()
{
	PRINTK2 (CARDNAME ":smc_shutdown\n");

	/* no more interrupts for me */
	SMC_SELECT_BANK (2);
	SMC_outb (0, LAN91C96_INT_MASK);

	/* and tell the card to stay away from that nasty outside world */
	SMC_SELECT_BANK (0);
	SMC_outb (0, LAN91C96_RCR);
	SMC_outb (0, LAN91C96_TCR);
}


/*
 * Function:  smc_hardware_send_packet(struct net_device * )
 * Purpose:
 *	This sends the actual packet to the SMC9xxx chip.
 *
 * Algorithm:
 *	First, see if a saved_skb is available.
 *		( this should NOT be called if there is no 'saved_skb'
 *	Now, find the packet number that the chip allocated
 *	Point the data pointers at it in memory
 *	Set the length word in the chip's memory
 *	Dump the packet to chip memory
 *	Check if a last byte is needed ( odd length packet )
 *		if so, set the control flag right
 *	Tell the card to send it
 *	Enable the transmit interrupt, so I know if it failed
 *	Free the kernel data if I actually sent it.
 */
static int smc_send_packet (volatile void *packet, int packet_length)
{
	byte packet_no;
	unsigned long ioaddr;
	byte *buf;
	int length;
	int numPages;
	int try = 0;
	int time_out;
	byte status;


	PRINTK3 ("%s:smc_hardware_send_packet\n", SMC_DEV_NAME);

	length = ETH_ZLEN < packet_length ? packet_length : ETH_ZLEN;

	/* allocate memory
	 ** The MMU wants the number of pages to be the number of 256 bytes
	 ** 'pages', minus 1 ( since a packet can't ever have 0 pages :) )
	 **
	 ** The 91C111 ignores the size bits, but the code is left intact
	 ** for backwards and future compatibility.
	 **
	 ** Pkt size for allocating is data length +6 (for additional status
	 ** words, length and ctl!)
	 **
	 ** If odd size then last byte is included in this header.
	 */
	numPages = ((length & 0xfffe) + 6);
	numPages >>= 8;				/* Divide by 256 */

	if (numPages > 7) {
		printf ("%s: Far too big packet error. \n", SMC_DEV_NAME);
		return 0;
	}

	/* now, try to allocate the memory */

	SMC_SELECT_BANK (2);
	SMC_outw (LAN91C96_MMUCR_ALLOC_TX | numPages, LAN91C96_MMU);

  again:
	try++;
	time_out = MEMORY_WAIT_TIME;
	do {
		status = SMC_inb (LAN91C96_INT_STATS);
		if (status & LAN91C96_IST_ALLOC_INT) {

			SMC_outb (LAN91C96_IST_ALLOC_INT, LAN91C96_INT_STATS);
			break;
		}
	} while (--time_out);

	if (!time_out) {
		PRINTK2 ("%s: memory allocation, try %d failed ...\n",
				 SMC_DEV_NAME, try);
		if (try < SMC_ALLOC_MAX_TRY)
			goto again;
		else
			return 0;
	}

	PRINTK2 ("%s: memory allocation, try %d succeeded ...\n",
			 SMC_DEV_NAME, try);

	/* I can send the packet now.. */

	ioaddr = SMC_BASE_ADDRESS;

	buf = (byte *) packet;

	/* If I get here, I _know_ there is a packet slot waiting for me */
	packet_no = SMC_inb (LAN91C96_ARR);
	if (packet_no & LAN91C96_ARR_FAILED) {
		/* or isn't there?  BAD CHIP! */
		printf ("%s: Memory allocation failed. \n", SMC_DEV_NAME);
		return 0;
	}

	/* we have a packet address, so tell the card to use it */
	SMC_outb (packet_no, LAN91C96_PNR);

	/* point to the beginning of the packet */
	SMC_outw (LAN91C96_PTR_AUTO_INCR, LAN91C96_POINTER);

	PRINTK3 ("%s: Trying to xmit packet of length %x\n",
			 SMC_DEV_NAME, length);

#if SMC_DEBUG > 2
	printf ("Transmitting Packet\n");
	print_packet (buf, length);
#endif

	/* send the packet length ( +6 for status, length and ctl byte )
	   and the status word ( set to zeros ) */
#ifdef USE_32_BIT
	SMC_outl ((length + 6) << 16, LAN91C96_DATA_HIGH);
#else
	SMC_outw (0, LAN91C96_DATA_HIGH);
	/* send the packet length ( +6 for status words, length, and ctl */
	SMC_outw ((length + 6), LAN91C96_DATA_HIGH);
#endif /* USE_32_BIT */

	/* send the actual data
	 * I _think_ it's faster to send the longs first, and then
	 * mop up by sending the last word.  It depends heavily
	 * on alignment, at least on the 486.  Maybe it would be
	 * a good idea to check which is optimal?  But that could take
	 * almost as much time as is saved?
	 */
#ifdef USE_32_BIT
	SMC_outsl (LAN91C96_DATA_HIGH, buf, length >> 2);
	if (length & 0x2)
		SMC_outw (*((word *) (buf + (length & 0xFFFFFFFC))),
				  LAN91C96_DATA_HIGH);
#else
	SMC_outsw (LAN91C96_DATA_HIGH, buf, (length) >> 1);
#endif /* USE_32_BIT */

	/* Send the last byte, if there is one.   */
	if ((length & 1) == 0) {
		SMC_outw (0, LAN91C96_DATA_HIGH);
	} else {
		SMC_outw (buf[length - 1] | 0x2000, LAN91C96_DATA_HIGH);
	}

	/* and let the chipset deal with it */
	SMC_outw (LAN91C96_MMUCR_ENQUEUE, LAN91C96_MMU);

	/* poll for TX INT */
	if (poll4int (LAN91C96_MSK_TX_INT, SMC_TX_TIMEOUT)) {
		/* sending failed */
		PRINTK2 ("%s: TX timeout, sending failed...\n", SMC_DEV_NAME);

		/* release packet */
		SMC_outw (LAN91C96_MMUCR_RELEASE_TX, LAN91C96_MMU);

		/* wait for MMU getting ready (low) */
		while (SMC_inw (LAN91C96_MMU) & LAN91C96_MMUCR_NO_BUSY) {
			udelay (10);
		}

		PRINTK2 ("MMU ready\n");


		return 0;
	} else {
		/* ack. int */
		SMC_outw (LAN91C96_IST_TX_INT, LAN91C96_INT_STATS);

		PRINTK2 ("%s: Sent packet of length %d \n", SMC_DEV_NAME, length);

		/* release packet */
		SMC_outw (LAN91C96_MMUCR_RELEASE_TX, LAN91C96_MMU);

		/* wait for MMU getting ready (low) */
		while (SMC_inw (LAN91C96_MMU) & LAN91C96_MMUCR_NO_BUSY) {
			udelay (10);
		}

		PRINTK2 ("MMU ready\n");
	}

	return length;
}

/*-------------------------------------------------------------------------
 * smc_destructor( struct net_device * dev )
 *   Input parameters:
 *	dev, pointer to the device structure
 *
 *   Output:
 *	None.
 *--------------------------------------------------------------------------
 */
void smc_destructor ()
{
	PRINTK2 (CARDNAME ":smc_destructor\n");
}


/*
 * Open and Initialize the board
 *
 * Set up everything, reset the card, etc ..
 *
 */
static int smc_open (bd_t *bd)
{
	int i, err;			/* used to set hw ethernet address */

	PRINTK2 ("%s:smc_open\n", SMC_DEV_NAME);

	/* reset the hardware */

	smc_reset ();
	smc_enable ();

	SMC_SELECT_BANK (1);

	err = smc_get_ethaddr (bd);	/* set smc_mac_addr, and sync it with u-boot globals */
	if (err < 0) {
		memset (bd->bi_enetaddr, 0, 6); /* hack to make error stick! upper code will abort if not set */
		return (-1);	/* upper code ignores this, but NOT bi_enetaddr */
	}
#ifdef USE_32_BIT
	for (i = 0; i < 6; i += 2) {
		word address;

		address = smc_mac_addr[i + 1] << 8;
		address |= smc_mac_addr[i];
		SMC_outw (address, LAN91C96_IA0 + i);
	}
#else
	for (i = 0; i < 6; i++)
		SMC_outb (smc_mac_addr[i], LAN91C96_IA0 + i);
#endif
	return 0;
}

/*-------------------------------------------------------------
 *
 * smc_rcv -  receive a packet from the card
 *
 * There is ( at least ) a packet waiting to be read from
 * chip-memory.
 *
 * o Read the status
 * o If an error, record it
 * o otherwise, read in the packet
 *-------------------------------------------------------------
 */
static int smc_rcv ()
{
	int packet_number;
	word status;
	word packet_length;
	int is_error = 0;

#ifdef USE_32_BIT
	dword stat_len;
#endif


	SMC_SELECT_BANK (2);
	packet_number = SMC_inw (LAN91C96_FIFO);

	if (packet_number & LAN91C96_FIFO_RXEMPTY) {
		return 0;
	}

	PRINTK3 ("%s:smc_rcv\n", SMC_DEV_NAME);
	/*  start reading from the start of the packet */
	SMC_outw (LAN91C96_PTR_READ | LAN91C96_PTR_RCV |
			  LAN91C96_PTR_AUTO_INCR, LAN91C96_POINTER);

	/* First two words are status and packet_length */
#ifdef USE_32_BIT
	stat_len = SMC_inl (LAN91C96_DATA_HIGH);
	status = stat_len & 0xffff;
	packet_length = stat_len >> 16;
#else
	status = SMC_inw (LAN91C96_DATA_HIGH);
	packet_length = SMC_inw (LAN91C96_DATA_HIGH);
#endif

	packet_length &= 0x07ff;	/* mask off top bits */

	PRINTK2 ("RCV: STATUS %4x LENGTH %4x\n", status, packet_length);

	if (!(status & FRAME_FILTER)) {
		/* Adjust for having already read the first two words */
		packet_length -= 4;		/*4; */


		/* set odd length for bug in LAN91C111, */
		/* which never sets RS_ODDFRAME */
		/* TODO ? */


#ifdef USE_32_BIT
		PRINTK3 (" Reading %d dwords (and %d bytes) \n",
			 packet_length >> 2, packet_length & 3);
		/* QUESTION:  Like in the TX routine, do I want
		   to send the DWORDs or the bytes first, or some
		   mixture.  A mixture might improve already slow PIO
		   performance  */
		SMC_insl (LAN91C96_DATA_HIGH, NetRxPackets[0], packet_length >> 2);
		/* read the left over bytes */
		if (packet_length & 3) {
			int i;

			byte *tail = (byte *) (NetRxPackets[0] + (packet_length & ~3));
			dword leftover = SMC_inl (LAN91C96_DATA_HIGH);

			for (i = 0; i < (packet_length & 3); i++)
				*tail++ = (byte) (leftover >> (8 * i)) & 0xff;
		}
#else
		PRINTK3 (" Reading %d words and %d byte(s) \n",
				 (packet_length >> 1), packet_length & 1);
		SMC_insw (LAN91C96_DATA_HIGH, NetRxPackets[0], packet_length >> 1);

#endif /* USE_32_BIT */

#if	SMC_DEBUG > 2
		printf ("Receiving Packet\n");
		print_packet (NetRxPackets[0], packet_length);
#endif
	} else {
		/* error ... */
		/* TODO ? */
		is_error = 1;
	}

	while (SMC_inw (LAN91C96_MMU) & LAN91C96_MMUCR_NO_BUSY)
		udelay (1);		/* Wait until not busy */

	/*  error or good, tell the card to get rid of this packet */
	SMC_outw (LAN91C96_MMUCR_RELEASE_RX, LAN91C96_MMU);

	while (SMC_inw (LAN91C96_MMU) & LAN91C96_MMUCR_NO_BUSY)
		udelay (1);		/* Wait until not busy */

	if (!is_error) {
		/* Pass the packet up to the protocol layers. */
		NetReceive (NetRxPackets[0], packet_length);
		return packet_length;
	} else {
		return 0;
	}

}

/*----------------------------------------------------
 * smc_close
 *
 * this makes the board clean up everything that it can
 * and not talk to the outside world.   Caused by
 * an 'ifconfig ethX down'
 *
 -----------------------------------------------------*/
static int smc_close ()
{
	PRINTK2 ("%s:smc_close\n", SMC_DEV_NAME);

	/* clear everything */
	smc_shutdown ();

	return 0;
}

#if SMC_DEBUG > 2
static void print_packet (byte * buf, int length)
{
#if 0
	int i;
	int remainder;
	int lines;

	printf ("Packet of length %d \n", length);

	lines = length / 16;
	remainder = length % 16;

	for (i = 0; i < lines; i++) {
		int cur;

		for (cur = 0; cur < 8; cur++) {
			byte a, b;

			a = *(buf++);
			b = *(buf++);
			printf ("%02x%02x ", a, b);
		}
		printf ("\n");
	}
	for (i = 0; i < remainder / 2; i++) {
		byte a, b;

		a = *(buf++);
		b = *(buf++);
		printf ("%02x%02x ", a, b);
	}
	printf ("\n");
#endif /* 0 */
}
#endif /* SMC_DEBUG > 2 */

int eth_init (bd_t * bd)
{
	return (smc_open(bd));
}

void eth_halt ()
{
	smc_close ();
}

int eth_rx ()
{
	return smc_rcv ();
}

int eth_send (volatile void *packet, int length)
{
	return smc_send_packet (packet, length);
}


#if 0
/*-------------------------------------------------------------------------
 * smc_hw_init()
 *
 *   Function:
 *      Reset and enable the device, check if the I/O space location
 *      is correct
 *
 *   Input parameters:
 *      None
 *
 *   Output:
 *	0 --> success
 *	1 --> error
 *--------------------------------------------------------------------------
 */
static int smc_hw_init ()
{
	unsigned short status_test;

	/* The attribute register of the LAN91C96 is located at address
	   0x0e000000 on the lubbock platform */
	volatile unsigned *attaddr = (unsigned *) (0x0e000000);

	/* first reset, then enable the device. Sequence is critical */
	attaddr[LAN91C96_ECOR] |= LAN91C96_ECOR_SRESET;
	udelay (100);
	attaddr[LAN91C96_ECOR] &= ~LAN91C96_ECOR_SRESET;
	attaddr[LAN91C96_ECOR] |= LAN91C96_ECOR_ENABLE;

	/* force 16-bit mode */
	attaddr[LAN91C96_ECSR] &= ~LAN91C96_ECSR_IOIS8;
	udelay (100);

	/* check if the I/O address is correct, the upper byte of the
	   bank select register should read 0x33 */

	status_test = SMC_inw (LAN91C96_BANK_SELECT);
	if ((status_test & 0xFF00) != 0x3300) {
		printf ("Failed to initialize ethernetchip\n");
		return 1;
	}
	return 0;
}
#endif /* 0 */

#endif /* CONFIG_CMD_NET */


/* smc_get_ethaddr (bd_t * bd)
 *
 * This checks both the environment and the ROM for an ethernet address. If
 * found, the environment takes precedence.
 */

int smc_get_ethaddr (bd_t * bd)
{
	int env_size = 0;
	int rom_valid = 0;
	int env_present = 0;
	int reg = 0;
	char *s = NULL;
	char *e = NULL;
	char *v_mac, es[] = "11:22:33:44:55:66";
	char s_env_mac[64];
	uchar v_env_mac[6];
	uchar v_rom_mac[6];

	env_size = getenv_r ("ethaddr", s_env_mac, sizeof (s_env_mac));
	if (env_size != sizeof(es)) {	/* Ignore if env is bad or not set */
		printf ("\n*** Warning: ethaddr is not set properly, ignoring!!\n");
	} else {
		env_present = 1;
		s = s_env_mac;

		for (reg = 0; reg < 6; ++reg) { /* turn string into mac value */
			v_env_mac[reg] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}
	}

	rom_valid = get_rom_mac (v_rom_mac);	/* get ROM mac value if any */

	if (!env_present) {	/* if NO env */
		if (rom_valid) {	/* but ROM is valid */
			v_mac = (char *)v_rom_mac;
			sprintf (s_env_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				 v_mac[0], v_mac[1], v_mac[2], v_mac[3],
				 v_mac[4], v_mac[5]);
			setenv ("ethaddr", s_env_mac);
		} else {	/* no env, bad ROM */
			printf ("\n*** ERROR: ethaddr is NOT set !!\n");
			return (-1);
		}
	} else {		/* good env, don't care ROM */
		v_mac = (char *)v_env_mac;	/* always use a good env over a ROM */
	}

	if (env_present && rom_valid) { /* if both env and ROM are good */
		if (memcmp (v_env_mac, v_rom_mac, 6) != 0) {
			printf ("\nWarning: MAC addresses don't match:\n");
			printf ("\tHW MAC address:  "
				"%02X:%02X:%02X:%02X:%02X:%02X\n",
				v_rom_mac[0], v_rom_mac[1],
				v_rom_mac[2], v_rom_mac[3],
				v_rom_mac[4], v_rom_mac[5] );
			printf ("\t\"ethaddr\" value: "
				"%02X:%02X:%02X:%02X:%02X:%02X\n",
				v_env_mac[0], v_env_mac[1],
				v_env_mac[2], v_env_mac[3],
				v_env_mac[4], v_env_mac[5]) ;
			debug ("### Set MAC addr from environment\n");
		}
	}
	memcpy (bd->bi_enetaddr, v_mac, 6);	/* update global address to match env (allows env changing) */
	smc_set_mac_addr ((unsigned char *)v_mac); /* use old function to update smc default */
	PRINTK("Using MAC Address %02X:%02X:%02X:%02X:%02X:%02X\n", v_mac[0], v_mac[1],
		v_mac[2], v_mac[3], v_mac[4], v_mac[5]);
	return (0);
}

/*
 * get_rom_mac()
 * Note, this has omly been tested for the OMAP730 P2.
 */

int get_rom_mac (unsigned char *v_rom_mac)
{
#ifdef HARDCODE_MAC	/* used for testing or to supress run time warnings */
	char hw_mac_addr[] = { 0x02, 0x80, 0xad, 0x20, 0x31, 0xb8 };

	memcpy (v_rom_mac, hw_mac_addr, 6);
	return (1);
#else
	int i;
	SMC_SELECT_BANK (1);
	for (i=0; i<6; i++)
	{
		v_rom_mac[i] = SMC_inb (LAN91C96_IA0 + i);
	}
	return (1);
#endif
}

#endif /* CONFIG_DRIVER_LAN91C96 */

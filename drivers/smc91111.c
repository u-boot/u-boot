/*------------------------------------------------------------------------
 . smc91111.c
 . This is a driver for SMSC's 91C111 single-chip Ethernet device.
 .
 . (C) Copyright 2002
 . Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 . Rolf Offermanns <rof@sysgo.de>
 .
 . Copyright (C) 2001 Standard Microsystems Corporation (SMSC)
 .	 Developed by Simple Network Magic Corporation (SNMC)
 . Copyright (C) 1996 by Erik Stahlman (ES)
 .
 . This program is free software; you can redistribute it and/or modify
 . it under the terms of the GNU General Public License as published by
 . the Free Software Foundation; either version 2 of the License, or
 . (at your option) any later version.
 .
 . This program is distributed in the hope that it will be useful,
 . but WITHOUT ANY WARRANTY; without even the implied warranty of
 . MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 . GNU General Public License for more details.
 .
 . You should have received a copy of the GNU General Public License
 . along with this program; if not, write to the Free Software
 . Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 .
 . Information contained in this file was obtained from the LAN91C111
 . manual from SMC.  To get a copy, if you really want one, you can find
 . information under www.smsc.com.
 .
 .
 . "Features" of the SMC chip:
 .   Integrated PHY/MAC for 10/100BaseT Operation
 .   Supports internal and external MII
 .   Integrated 8K packet memory
 .   EEPROM interface for configuration
 .
 . Arguments:
 .	io	= for the base address
 .	irq	= for the IRQ
 .
 . author:
 .	Erik Stahlman				( erik@vt.edu )
 .	Daris A Nevil				( dnevil@snmc.com )
 .
 .
 . Hardware multicast code from Peter Cammaert ( pc@denkart.be )
 .
 . Sources:
 .    o	  SMSC LAN91C111 databook (www.smsc.com)
 .    o	  smc9194.c by Erik Stahlman
 .    o	  skeleton.c by Donald Becker ( becker@cesdis.gsfc.nasa.gov )
 .
 . History:
 .	06/19/03  Richard Woodruff Made u-boot environment aware and added mac addr checks.
 .	10/17/01  Marco Hasewinkel Modify for DNP/1110
 .	07/25/01  Woojung Huh	   Modify for ADS Bitsy
 .	04/25/01  Daris A Nevil	   Initial public release through SMSC
 .	03/16/01  Daris A Nevil	   Modified smc9194.c for use with LAN91C111
 ----------------------------------------------------------------------------*/

#include <common.h>
#include <command.h>
#include <config.h>
#include "smc91111.h"
#include <net.h>

#ifdef CONFIG_DRIVER_SMC91111

/* Use power-down feature of the chip */
#define POWER_DOWN	0

#define NO_AUTOPROBE

#define SMC_DEBUG 0

#if SMC_DEBUG > 1
static const char version[] =
	"smc91111.c:v1.0 04/25/01 by Daris A Nevil (dnevil@snmc.com)\n";
#endif

/* Autonegotiation timeout in seconds */
#ifndef CONFIG_SMC_AUTONEG_TIMEOUT
#define CONFIG_SMC_AUTONEG_TIMEOUT 10
#endif

/*------------------------------------------------------------------------
 .
 . Configuration options, for the experienced user to change.
 .
 -------------------------------------------------------------------------*/

/*
 . Wait time for memory to be free.  This probably shouldn't be
 . tuned that much, as waiting for this means nothing else happens
 . in the system
*/
#define MEMORY_WAIT_TIME 16


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
 .
 . The internal workings of the driver.	 If you are changing anything
 . here with the SMC stuff, you should have the datasheet and know
 . what you are doing.
 .
 -------------------------------------------------------------------------*/
#define CARDNAME "LAN91C111"

/* Memory sizing constant */
#define LAN91C111_MEMORY_MULTIPLIER	(1024*2)

#ifndef CONFIG_SMC91111_BASE
#define CONFIG_SMC91111_BASE 0x20000300
#endif

#define SMC_BASE_ADDRESS CONFIG_SMC91111_BASE

#define SMC_DEV_NAME "SMC91111"
#define SMC_PHY_ADDR 0x0000
#define SMC_ALLOC_MAX_TRY 5
#define SMC_TX_TIMEOUT 30

#define SMC_PHY_CLOCK_DELAY 1000

#define ETH_ZLEN 60

#ifdef	CONFIG_SMC_USE_32_BIT
#define USE_32_BIT  1
#else
#undef USE_32_BIT
#endif
/*-----------------------------------------------------------------
 .
 .  The driver can be entered at any of the following entry points.
 .
 .------------------------------------------------------------------  */

extern int eth_init(bd_t *bd);
extern void eth_halt(void);
extern int eth_rx(void);
extern int eth_send(volatile void *packet, int length);


/*
 . This is called by  register_netdev().  It is responsible for
 . checking the portlist for the SMC9000 series chipset.  If it finds
 . one, then it will initialize the device, find the hardware information,
 . and sets up the appropriate device parameters.
 . NOTE: Interrupts are *OFF* when this procedure is called.
 .
 . NB:This shouldn't be static since it is referred to externally.
*/
int smc_init(void);

/*
 . This is called by  unregister_netdev().  It is responsible for
 . cleaning up before the driver is finally unregistered and discarded.
*/
void smc_destructor(void);

/*
 . The kernel calls this function when someone wants to use the device,
 . typically 'ifconfig ethX up'.
*/
static int smc_open(bd_t *bd);


/*
 . This is called by the kernel in response to 'ifconfig ethX down'.  It
 . is responsible for cleaning up everything that the open routine
 . does, and maybe putting the card into a powerdown state.
*/
static int smc_close(void);

/*
 . Configures the PHY through the MII Management interface
*/
#ifndef CONFIG_SMC91111_EXT_PHY
static void smc_phy_configure(void);
#endif /* !CONFIG_SMC91111_EXT_PHY */

/*
 . This is a separate procedure to handle the receipt of a packet, to
 . leave the interrupt code looking slightly cleaner
*/
static int smc_rcv(void);

/* See if a MAC address is defined in the current environment. If so use it. If not
 . print a warning and set the environment and other globals with the default.
 . If an EEPROM is present it really should be consulted.
*/
int smc_get_ethaddr(bd_t *bd);
int get_rom_mac(char *v_rom_mac);

/*
 ------------------------------------------------------------
 .
 . Internal routines
 .
 ------------------------------------------------------------
*/

#ifdef CONFIG_SMC_USE_IOFUNCS
/*
 * input and output functions
 *
 * Implemented due to inx,outx macros accessing the device improperly
 * and putting the device into an unkown state.
 *
 * For instance, on Sharp LPD7A400 SDK, affects were chip memory
 * could not be free'd (hence the alloc failures), duplicate packets,
 * packets being corrupt (shifted) on the wire, etc.  Switching to the
 * inx,outx functions fixed this problem.
 */
static inline word SMC_inw(dword offset);
static inline void SMC_outw(word value, dword offset);
static inline byte SMC_inb(dword offset);
static inline void SMC_outb(byte value, dword offset);
static inline void SMC_insw(dword offset, volatile uchar* buf, dword len);
static inline void SMC_outsw(dword offset, uchar* buf, dword len);

#define barrier() __asm__ __volatile__("": : :"memory")

static inline word SMC_inw(dword offset)
{
	word v;
	v = *((volatile word*)(SMC_BASE_ADDRESS+offset));
	barrier(); *(volatile u32*)(0xc0000000);
	return v;
}

static inline void SMC_outw(word value, dword offset)
{
	*((volatile word*)(SMC_BASE_ADDRESS+offset)) = value;
	barrier(); *(volatile u32*)(0xc0000000);
}

static inline byte SMC_inb(dword offset)
{
	word  _w;

	_w = SMC_inw(offset & ~((dword)1));
	return (offset & 1) ? (byte)(_w >> 8) : (byte)(_w);
}

static inline void SMC_outb(byte value, dword offset)
{
	word  _w;

	_w = SMC_inw(offset & ~((dword)1));
	if (offset & 1)
			*((volatile word*)(SMC_BASE_ADDRESS+(offset & ~((dword)1)))) = (value<<8) | (_w & 0x00ff);
	else
			*((volatile word*)(SMC_BASE_ADDRESS+offset)) = value | (_w & 0xff00);
}

static inline void SMC_insw(dword offset, volatile uchar* buf, dword len)
{
	while (len-- > 0) {
		*((word*)buf)++ = SMC_inw(offset);
		barrier(); *((volatile u32*)(0xc0000000));
	}
}

static inline void SMC_outsw(dword offset, uchar* buf, dword len)
{
	while (len-- > 0) {
		SMC_outw(*((word*)buf)++, offset);
		barrier(); *(volatile u32*)(0xc0000000);
	}
}
#endif  /* CONFIG_SMC_USE_IOFUNCS */

static char unsigned smc_mac_addr[6] = {0x02, 0x80, 0xad, 0x20, 0x31, 0xb8};

/*
 * This function must be called before smc_open() if you want to override
 * the default mac address.
 */

void smc_set_mac_addr(const char *addr) {
	int i;

	for (i=0; i < sizeof(smc_mac_addr); i++){
		smc_mac_addr[i] = addr[i];
	}
}

/*
 * smc_get_macaddr is no longer used. If you want to override the default
 * mac address, call smc_get_mac_addr as a part of the board initialization.
 */

#if 0
void smc_get_macaddr( byte *addr ) {
	/* MAC ADDRESS AT FLASHBLOCK 1 / OFFSET 0x10 */
	unsigned char *dnp1110_mac = (unsigned char *) (0xE8000000 + 0x20010);
	int i;


	for (i=0; i<6; i++) {
	    addr[0] = *(dnp1110_mac+0);
	    addr[1] = *(dnp1110_mac+1);
	    addr[2] = *(dnp1110_mac+2);
	    addr[3] = *(dnp1110_mac+3);
	    addr[4] = *(dnp1110_mac+4);
	    addr[5] = *(dnp1110_mac+5);
	}
}
#endif /* 0 */

/***********************************************
 * Show available memory		       *
 ***********************************************/
void dump_memory_info(void)
{
	word mem_info;
	word old_bank;

	old_bank = SMC_inw(BANK_SELECT)&0xF;

	SMC_SELECT_BANK(0);
	mem_info = SMC_inw( MIR_REG );
	PRINTK2("Memory: %4d available\n", (mem_info >> 8)*2048);

	SMC_SELECT_BANK(old_bank);
}
/*
 . A rather simple routine to print out a packet for debugging purposes.
*/
#if SMC_DEBUG > 2
static void print_packet( byte *, int );
#endif

#define tx_done(dev) 1


/* this does a soft reset on the device */
static void smc_reset( void );

/* Enable Interrupts, Receive, and Transmit */
static void smc_enable( void );

/* this puts the device in an inactive state */
static void smc_shutdown( void );

/* Routines to Read and Write the PHY Registers across the
   MII Management Interface
*/

#ifndef CONFIG_SMC91111_EXT_PHY
static word smc_read_phy_register(byte phyreg);
static void smc_write_phy_register(byte phyreg, word phydata);
#endif /* !CONFIG_SMC91111_EXT_PHY */


static int poll4int (byte mask, int timeout)
{
	int tmo = get_timer (0) + timeout * CFG_HZ;
	int is_timeout = 0;
	word old_bank = SMC_inw (BSR_REG);

	PRINTK2 ("Polling...\n");
	SMC_SELECT_BANK (2);
	while ((SMC_inw (SMC91111_INT_REG) & mask) == 0) {
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

/* Only one release command at a time, please */
static inline void smc_wait_mmu_release_complete (void)
{
	int count = 0;

	/* assume bank 2 selected */
	while (SMC_inw (MMU_CMD_REG) & MC_BUSY) {
		udelay (1);	/* Wait until not busy */
		if (++count > 200)
			break;
	}
}

/*
 . Function: smc_reset( void )
 . Purpose:
 .	This sets the SMC91111 chip to its normal state, hopefully from whatever
 .	mess that any other DOS driver has put it in.
 .
 . Maybe I should reset more registers to defaults in here?  SOFTRST  should
 . do that for me.
 .
 . Method:
 .	1.  send a SOFT RESET
 .	2.  wait for it to finish
 .	3.  enable autorelease mode
 .	4.  reset the memory management unit
 .	5.  clear all interrupts
 .
*/
static void smc_reset (void)
{
	PRINTK2 ("%s: smc_reset\n", SMC_DEV_NAME);

	/* This resets the registers mostly to defaults, but doesn't
	   affect EEPROM.  That seems unnecessary */
	SMC_SELECT_BANK (0);
	SMC_outw (RCR_SOFTRST, RCR_REG);

	/* Setup the Configuration Register */
	/* This is necessary because the CONFIG_REG is not affected */
	/* by a soft reset */

	SMC_SELECT_BANK (1);
#if defined(CONFIG_SMC91111_EXT_PHY)
	SMC_outw (CONFIG_DEFAULT | CONFIG_EXT_PHY, CONFIG_REG);
#else
	SMC_outw (CONFIG_DEFAULT, CONFIG_REG);
#endif


	/* Release from possible power-down state */
	/* Configuration register is not affected by Soft Reset */
	SMC_outw (SMC_inw (CONFIG_REG) | CONFIG_EPH_POWER_EN, CONFIG_REG);

	SMC_SELECT_BANK (0);

	/* this should pause enough for the chip to be happy */
	udelay (10);

	/* Disable transmit and receive functionality */
	SMC_outw (RCR_CLEAR, RCR_REG);
	SMC_outw (TCR_CLEAR, TCR_REG);

	/* set the control register */
	SMC_SELECT_BANK (1);
	SMC_outw (CTL_DEFAULT, CTL_REG);

	/* Reset the MMU */
	SMC_SELECT_BANK (2);
	smc_wait_mmu_release_complete ();
	SMC_outw (MC_RESET, MMU_CMD_REG);
	while (SMC_inw (MMU_CMD_REG) & MC_BUSY)
		udelay (1);	/* Wait until not busy */

	/* Note:  It doesn't seem that waiting for the MMU busy is needed here,
	   but this is a place where future chipsets _COULD_ break.  Be wary
	   of issuing another MMU command right after this */

	/* Disable all interrupts */
	SMC_outb (0, IM_REG);
}

/*
 . Function: smc_enable
 . Purpose: let the chip talk to the outside work
 . Method:
 .	1.  Enable the transmitter
 .	2.  Enable the receiver
 .	3.  Enable interrupts
*/
static void smc_enable()
{
	PRINTK2("%s: smc_enable\n", SMC_DEV_NAME);
	SMC_SELECT_BANK( 0 );
	/* see the header file for options in TCR/RCR DEFAULT*/
	SMC_outw( TCR_DEFAULT, TCR_REG );
	SMC_outw( RCR_DEFAULT, RCR_REG );

	/* clear MII_DIS */
/*	smc_write_phy_register(PHY_CNTL_REG, 0x0000); */
}

/*
 . Function: smc_shutdown
 . Purpose:  closes down the SMC91xxx chip.
 . Method:
 .	1. zero the interrupt mask
 .	2. clear the enable receive flag
 .	3. clear the enable xmit flags
 .
 . TODO:
 .   (1) maybe utilize power down mode.
 .	Why not yet?  Because while the chip will go into power down mode,
 .	the manual says that it will wake up in response to any I/O requests
 .	in the register space.	 Empirical results do not show this working.
*/
static void smc_shutdown()
{
	PRINTK2(CARDNAME ": smc_shutdown\n");

	/* no more interrupts for me */
	SMC_SELECT_BANK( 2 );
	SMC_outb( 0, IM_REG );

	/* and tell the card to stay away from that nasty outside world */
	SMC_SELECT_BANK( 0 );
	SMC_outb( RCR_CLEAR, RCR_REG );
	SMC_outb( TCR_CLEAR, TCR_REG );
}


/*
 . Function:  smc_hardware_send_packet(struct net_device * )
 . Purpose:
 .	This sends the actual packet to the SMC9xxx chip.
 .
 . Algorithm:
 .	First, see if a saved_skb is available.
 .		( this should NOT be called if there is no 'saved_skb'
 .	Now, find the packet number that the chip allocated
 .	Point the data pointers at it in memory
 .	Set the length word in the chip's memory
 .	Dump the packet to chip memory
 .	Check if a last byte is needed ( odd length packet )
 .		if so, set the control flag right
 .	Tell the card to send it
 .	Enable the transmit interrupt, so I know if it failed
 .	Free the kernel data if I actually sent it.
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
	byte saved_pnr;
	word saved_ptr;

	/* save PTR and PNR registers before manipulation */
	SMC_SELECT_BANK (2);
	saved_pnr = SMC_inb( PN_REG );
	saved_ptr = SMC_inw( PTR_REG );

	PRINTK3 ("%s: smc_hardware_send_packet\n", SMC_DEV_NAME);

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
	numPages >>= 8;		/* Divide by 256 */

	if (numPages > 7) {
		printf ("%s: Far too big packet error. \n", SMC_DEV_NAME);
		return 0;
	}

	/* now, try to allocate the memory */
	SMC_SELECT_BANK (2);
	SMC_outw (MC_ALLOC | numPages, MMU_CMD_REG);

	/* FIXME: the ALLOC_INT bit never gets set *
	 * so the following will always give a	   *
	 * memory allocation error.		   *
	 * same code works in armboot though	   *
	 * -ro
	 */

again:
	try++;
	time_out = MEMORY_WAIT_TIME;
	do {
		status = SMC_inb (SMC91111_INT_REG);
		if (status & IM_ALLOC_INT) {
			/* acknowledge the interrupt */
			SMC_outb (IM_ALLOC_INT, SMC91111_INT_REG);
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
	packet_no = SMC_inb (AR_REG);
	if (packet_no & AR_FAILED) {
		/* or isn't there?  BAD CHIP! */
		printf ("%s: Memory allocation failed. \n", SMC_DEV_NAME);
		return 0;
	}

	/* we have a packet address, so tell the card to use it */
#ifndef CONFIG_XAENIAX
	SMC_outb (packet_no, PN_REG);
#else
	/* On Xaeniax board, we can't use SMC_outb here because that way
	 * the Allocate MMU command will end up written to the command register
	 * as well, which will lead to a problem.
	 */
	SMC_outl (packet_no << 16, 0);
#endif
	/* do not write new ptr value if Write data fifo not empty */
	while ( saved_ptr & PTR_NOTEMPTY )
		printf ("Write data fifo not empty!\n");

	/* point to the beginning of the packet */
	SMC_outw (PTR_AUTOINC, PTR_REG);

	PRINTK3 ("%s: Trying to xmit packet of length %x\n",
		 SMC_DEV_NAME, length);

#if SMC_DEBUG > 2
	printf ("Transmitting Packet\n");
	print_packet (buf, length);
#endif

	/* send the packet length ( +6 for status, length and ctl byte )
	   and the status word ( set to zeros ) */
#ifdef USE_32_BIT
	SMC_outl ((length + 6) << 16, SMC91111_DATA_REG);
#else
	SMC_outw (0, SMC91111_DATA_REG);
	/* send the packet length ( +6 for status words, length, and ctl */
	SMC_outw ((length + 6), SMC91111_DATA_REG);
#endif

	/* send the actual data
	   . I _think_ it's faster to send the longs first, and then
	   . mop up by sending the last word.  It depends heavily
	   . on alignment, at least on the 486.	 Maybe it would be
	   . a good idea to check which is optimal?  But that could take
	   . almost as much time as is saved?
	 */
#ifdef USE_32_BIT
	SMC_outsl (SMC91111_DATA_REG, buf, length >> 2);
#ifndef CONFIG_XAENIAX
	if (length & 0x2)
		SMC_outw (*((word *) (buf + (length & 0xFFFFFFFC))),
			  SMC91111_DATA_REG);
#else
	/* On XANEIAX, we can only use 32-bit writes, so we need to handle
	 * unaligned tail part specially. The standard code doesn't work.
	 */
	if ((length & 3) == 3) {
		u16 * ptr = (u16*) &buf[length-3];
		SMC_outl((*ptr) | ((0x2000 | buf[length-1]) << 16),
				SMC91111_DATA_REG);
	} else if ((length & 2) == 2) {
		u16 * ptr = (u16*) &buf[length-2];
		SMC_outl(*ptr, SMC91111_DATA_REG);
	} else if (length & 1) {
		SMC_outl((0x2000 | buf[length-1]), SMC91111_DATA_REG);
	} else {
		SMC_outl(0, SMC91111_DATA_REG);
	}
#endif
#else
	SMC_outsw (SMC91111_DATA_REG, buf, (length) >> 1);
#endif /* USE_32_BIT */

#ifndef CONFIG_XAENIAX
	/* Send the last byte, if there is one.	  */
	if ((length & 1) == 0) {
		SMC_outw (0, SMC91111_DATA_REG);
	} else {
		SMC_outw (buf[length - 1] | 0x2000, SMC91111_DATA_REG);
	}
#endif

	/* and let the chipset deal with it */
	SMC_outw (MC_ENQUEUE, MMU_CMD_REG);

	/* poll for TX INT */
	/* if (poll4int (IM_TX_INT, SMC_TX_TIMEOUT)) { */
	/* poll for TX_EMPTY INT - autorelease enabled */
	if (poll4int(IM_TX_EMPTY_INT, SMC_TX_TIMEOUT)) {
		/* sending failed */
		PRINTK2 ("%s: TX timeout, sending failed...\n", SMC_DEV_NAME);

		/* release packet */
		/* no need to release, MMU does that now */
#ifdef CONFIG_XAENIAX
		 SMC_outw (MC_FREEPKT, MMU_CMD_REG);
#endif

		/* wait for MMU getting ready (low) */
		while (SMC_inw (MMU_CMD_REG) & MC_BUSY) {
			udelay (10);
		}

		PRINTK2 ("MMU ready\n");


		return 0;
	} else {
		/* ack. int */
		SMC_outb (IM_TX_EMPTY_INT, SMC91111_INT_REG);
		/* SMC_outb (IM_TX_INT, SMC91111_INT_REG); */
		PRINTK2 ("%s: Sent packet of length %d \n", SMC_DEV_NAME,
			 length);

		/* release packet */
		/* no need to release, MMU does that now */
#ifdef CONFIG_XAENIAX
		SMC_outw (MC_FREEPKT, MMU_CMD_REG);
#endif

		/* wait for MMU getting ready (low) */
		while (SMC_inw (MMU_CMD_REG) & MC_BUSY) {
			udelay (10);
		}

		PRINTK2 ("MMU ready\n");


	}

	/* restore previously saved registers */
#ifndef CONFIG_XAENIAX
	SMC_outb( saved_pnr, PN_REG );
#else
	/* On Xaeniax board, we can't use SMC_outb here because that way
	 * the Allocate MMU command will end up written to the command register
	 * as well, which will lead to a problem.
	 */
	SMC_outl(saved_pnr << 16, 0);
#endif
	SMC_outw( saved_ptr, PTR_REG );

	return length;
}

/*-------------------------------------------------------------------------
 |
 | smc_destructor( struct net_device * dev )
 |   Input parameters:
 |	dev, pointer to the device structure
 |
 |   Output:
 |	None.
 |
 ---------------------------------------------------------------------------
*/
void smc_destructor()
{
	PRINTK2(CARDNAME ": smc_destructor\n");
}


/*
 * Open and Initialize the board
 *
 * Set up everything, reset the card, etc ..
 *
 */
static int smc_open (bd_t * bd)
{
	int i, err;

	PRINTK2 ("%s: smc_open\n", SMC_DEV_NAME);

	/* reset the hardware */
	smc_reset ();
	smc_enable ();

	/* Configure the PHY */
#ifndef CONFIG_SMC91111_EXT_PHY
	smc_phy_configure ();
#endif

	/* conservative setting (10Mbps, HalfDuplex, no AutoNeg.) */
/*	SMC_SELECT_BANK(0); */
/*	SMC_outw(0, RPC_REG); */
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
		SMC_outw (address, (ADDR0_REG + i));
	}
#else
	for (i = 0; i < 6; i++)
		SMC_outb (smc_mac_addr[i], (ADDR0_REG + i));
#endif

	return 0;
}

/*-------------------------------------------------------------
 .
 . smc_rcv -  receive a packet from the card
 .
 . There is ( at least ) a packet waiting to be read from
 . chip-memory.
 .
 . o Read the status
 . o If an error, record it
 . o otherwise, read in the packet
 --------------------------------------------------------------
*/
static int smc_rcv()
{
	int	packet_number;
	word	status;
	word	packet_length;
	int	is_error = 0;
#ifdef USE_32_BIT
	dword stat_len;
#endif
	byte saved_pnr;
	word saved_ptr;

	SMC_SELECT_BANK(2);
	/* save PTR and PTR registers */
	saved_pnr = SMC_inb( PN_REG );
	saved_ptr = SMC_inw( PTR_REG );

	packet_number = SMC_inw( RXFIFO_REG );

	if ( packet_number & RXFIFO_REMPTY ) {

		return 0;
	}

	PRINTK3("%s: smc_rcv\n", SMC_DEV_NAME);
	/*  start reading from the start of the packet */
	SMC_outw( PTR_READ | PTR_RCV | PTR_AUTOINC, PTR_REG );

	/* First two words are status and packet_length */
#ifdef USE_32_BIT
	stat_len = SMC_inl(SMC91111_DATA_REG);
	status = stat_len & 0xffff;
	packet_length = stat_len >> 16;
#else
	status		= SMC_inw( SMC91111_DATA_REG );
	packet_length	= SMC_inw( SMC91111_DATA_REG );
#endif

	packet_length &= 0x07ff;  /* mask off top bits */

	PRINTK2("RCV: STATUS %4x LENGTH %4x\n", status, packet_length );

	if ( !(status & RS_ERRORS ) ){
		/* Adjust for having already read the first two words */
		packet_length -= 4; /*4; */


		/* set odd length for bug in LAN91C111, */
		/* which never sets RS_ODDFRAME */
		/* TODO ? */


#ifdef USE_32_BIT
		PRINTK3(" Reading %d dwords (and %d bytes) \n",
			packet_length >> 2, packet_length & 3 );
		/* QUESTION:  Like in the TX routine, do I want
		   to send the DWORDs or the bytes first, or some
		   mixture.  A mixture might improve already slow PIO
		   performance	*/
		SMC_insl( SMC91111_DATA_REG , NetRxPackets[0], packet_length >> 2 );
		/* read the left over bytes */
		if (packet_length & 3) {
			int i;

			byte *tail = (byte *)(NetRxPackets[0] + (packet_length & ~3));
			dword leftover = SMC_inl(SMC91111_DATA_REG);
			for (i=0; i<(packet_length & 3); i++)
				*tail++ = (byte) (leftover >> (8*i)) & 0xff;
		}
#else
		PRINTK3(" Reading %d words and %d byte(s) \n",
			(packet_length >> 1 ), packet_length & 1 );
		SMC_insw(SMC91111_DATA_REG , NetRxPackets[0], packet_length >> 1);

#endif /* USE_32_BIT */

#if	SMC_DEBUG > 2
		printf("Receiving Packet\n");
		print_packet( NetRxPackets[0], packet_length );
#endif
	} else {
		/* error ... */
		/* TODO ? */
		is_error = 1;
	}

	while ( SMC_inw( MMU_CMD_REG ) & MC_BUSY )
		udelay(1); /* Wait until not busy */

	/*  error or good, tell the card to get rid of this packet */
	SMC_outw( MC_RELEASE, MMU_CMD_REG );

	while ( SMC_inw( MMU_CMD_REG ) & MC_BUSY )
		udelay(1); /* Wait until not busy */

	/* restore saved registers */
#ifndef CONFIG_XAENIAX
	SMC_outb( saved_pnr, PN_REG );
#else
	/* On Xaeniax board, we can't use SMC_outb here because that way
	 * the Allocate MMU command will end up written to the command register
	 * as well, which will lead to a problem.
	 */
	SMC_outl( saved_pnr << 16, 0);
#endif
	SMC_outw( saved_ptr, PTR_REG );

	if (!is_error) {
		/* Pass the packet up to the protocol layers. */
		NetReceive(NetRxPackets[0], packet_length);
		return packet_length;
	} else {
		return 0;
	}

}


/*----------------------------------------------------
 . smc_close
 .
 . this makes the board clean up everything that it can
 . and not talk to the outside world.	Caused by
 . an 'ifconfig ethX down'
 .
 -----------------------------------------------------*/
static int smc_close()
{
	PRINTK2("%s: smc_close\n", SMC_DEV_NAME);

	/* clear everything */
	smc_shutdown();

	return 0;
}


#if 0
/*------------------------------------------------------------
 . Modify a bit in the LAN91C111 register set
 .-------------------------------------------------------------*/
static word smc_modify_regbit(int bank, int ioaddr, int reg,
	unsigned int bit, int val)
{
	word regval;

	SMC_SELECT_BANK( bank );

	regval = SMC_inw( reg );
	if (val)
		regval |= bit;
	else
		regval &= ~bit;

	SMC_outw( regval, 0 );
	return(regval);
}


/*------------------------------------------------------------
 . Retrieve a bit in the LAN91C111 register set
 .-------------------------------------------------------------*/
static int smc_get_regbit(int bank, int ioaddr, int reg, unsigned int bit)
{
	SMC_SELECT_BANK( bank );
	if ( SMC_inw( reg ) & bit)
		return(1);
	else
		return(0);
}


/*------------------------------------------------------------
 . Modify a LAN91C111 register (word access only)
 .-------------------------------------------------------------*/
static void smc_modify_reg(int bank, int ioaddr, int reg, word val)
{
	SMC_SELECT_BANK( bank );
	SMC_outw( val, reg );
}


/*------------------------------------------------------------
 . Retrieve a LAN91C111 register (word access only)
 .-------------------------------------------------------------*/
static int smc_get_reg(int bank, int ioaddr, int reg)
{
	SMC_SELECT_BANK( bank );
	return(SMC_inw( reg ));
}

#endif /* 0 */

/*---PHY CONTROL AND CONFIGURATION----------------------------------------- */

#if (SMC_DEBUG > 2 )

/*------------------------------------------------------------
 . Debugging function for viewing MII Management serial bitstream
 .-------------------------------------------------------------*/
static void smc_dump_mii_stream (byte * bits, int size)
{
	int i;

	printf ("BIT#:");
	for (i = 0; i < size; ++i) {
		printf ("%d", i % 10);
	}

	printf ("\nMDOE:");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDOE)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\nMDO :");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDO)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\nMDI :");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDI)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\n");
}
#endif

/*------------------------------------------------------------
 . Reads a register from the MII Management serial interface
 .-------------------------------------------------------------*/
#ifndef CONFIG_SMC91111_EXT_PHY
static word smc_read_phy_register (byte phyreg)
{
	int oldBank;
	int i;
	byte mask;
	word mii_reg;
	byte bits[64];
	int clk_idx = 0;
	int input_idx;
	word phydata;
	byte phyaddr = SMC_PHY_ADDR;

	/* 32 consecutive ones on MDO to establish sync */
	for (i = 0; i < 32; ++i)
		bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Start code <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Read command <10> */
	bits[clk_idx++] = MII_MDOE | MII_MDO;
	bits[clk_idx++] = MII_MDOE;

	/* Output the PHY address, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyaddr & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Output the phy register number, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyreg & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Tristate and turnaround (2 bit times) */
	bits[clk_idx++] = 0;
	/*bits[clk_idx++] = 0; */

	/* Input starts at this bit time */
	input_idx = clk_idx;

	/* Will input 16 bits */
	for (i = 0; i < 16; ++i)
		bits[clk_idx++] = 0;

	/* Final clock bit */
	bits[clk_idx++] = 0;

	/* Save the current bank */
	oldBank = SMC_inw (BANK_SELECT);

	/* Select bank 3 */
	SMC_SELECT_BANK (3);

	/* Get the current MII register value */
	mii_reg = SMC_inw (MII_REG);

	/* Turn off all MII Interface bits */
	mii_reg &= ~(MII_MDOE | MII_MCLK | MII_MDI | MII_MDO);

	/* Clock all 64 cycles */
	for (i = 0; i < sizeof bits; ++i) {
		/* Clock Low - output data */
		SMC_outw (mii_reg | bits[i], MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);


		/* Clock Hi - input data */
		SMC_outw (mii_reg | bits[i] | MII_MCLK, MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);
		bits[i] |= SMC_inw (MII_REG) & MII_MDI;
	}

	/* Return to idle state */
	/* Set clock to low, data to low, and output tristated */
	SMC_outw (mii_reg, MII_REG);
	udelay (SMC_PHY_CLOCK_DELAY);

	/* Restore original bank select */
	SMC_SELECT_BANK (oldBank);

	/* Recover input data */
	phydata = 0;
	for (i = 0; i < 16; ++i) {
		phydata <<= 1;

		if (bits[input_idx++] & MII_MDI)
			phydata |= 0x0001;
	}

#if (SMC_DEBUG > 2 )
	printf ("smc_read_phy_register(): phyaddr=%x,phyreg=%x,phydata=%x\n",
		phyaddr, phyreg, phydata);
	smc_dump_mii_stream (bits, sizeof bits);
#endif

	return (phydata);
}


/*------------------------------------------------------------
 . Writes a register to the MII Management serial interface
 .-------------------------------------------------------------*/
static void smc_write_phy_register (byte phyreg, word phydata)
{
	int oldBank;
	int i;
	word mask;
	word mii_reg;
	byte bits[65];
	int clk_idx = 0;
	byte phyaddr = SMC_PHY_ADDR;

	/* 32 consecutive ones on MDO to establish sync */
	for (i = 0; i < 32; ++i)
		bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Start code <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Write command <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Output the PHY address, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyaddr & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Output the phy register number, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyreg & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Tristate and turnaround (2 bit times) */
	bits[clk_idx++] = 0;
	bits[clk_idx++] = 0;

	/* Write out 16 bits of data, msb first */
	mask = 0x8000;
	for (i = 0; i < 16; ++i) {
		if (phydata & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Final clock bit (tristate) */
	bits[clk_idx++] = 0;

	/* Save the current bank */
	oldBank = SMC_inw (BANK_SELECT);

	/* Select bank 3 */
	SMC_SELECT_BANK (3);

	/* Get the current MII register value */
	mii_reg = SMC_inw (MII_REG);

	/* Turn off all MII Interface bits */
	mii_reg &= ~(MII_MDOE | MII_MCLK | MII_MDI | MII_MDO);

	/* Clock all cycles */
	for (i = 0; i < sizeof bits; ++i) {
		/* Clock Low - output data */
		SMC_outw (mii_reg | bits[i], MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);


		/* Clock Hi - input data */
		SMC_outw (mii_reg | bits[i] | MII_MCLK, MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);
		bits[i] |= SMC_inw (MII_REG) & MII_MDI;
	}

	/* Return to idle state */
	/* Set clock to low, data to low, and output tristated */
	SMC_outw (mii_reg, MII_REG);
	udelay (SMC_PHY_CLOCK_DELAY);

	/* Restore original bank select */
	SMC_SELECT_BANK (oldBank);

#if (SMC_DEBUG > 2 )
	printf ("smc_write_phy_register(): phyaddr=%x,phyreg=%x,phydata=%x\n",
		phyaddr, phyreg, phydata);
	smc_dump_mii_stream (bits, sizeof bits);
#endif
}
#endif /* !CONFIG_SMC91111_EXT_PHY */


/*------------------------------------------------------------
 . Waits the specified number of milliseconds - kernel friendly
 .-------------------------------------------------------------*/
#ifndef CONFIG_SMC91111_EXT_PHY
static void smc_wait_ms(unsigned int ms)
{
	udelay(ms*1000);
}
#endif /* !CONFIG_SMC91111_EXT_PHY */


/*------------------------------------------------------------
 . Configures the specified PHY using Autonegotiation. Calls
 . smc_phy_fixed() if the user has requested a certain config.
 .-------------------------------------------------------------*/
#ifndef CONFIG_SMC91111_EXT_PHY
static void smc_phy_configure ()
{
	int timeout;
	byte phyaddr;
	word my_phy_caps;	/* My PHY capabilities */
	word my_ad_caps;	/* My Advertised capabilities */
	word status = 0;	/*;my status = 0 */
	int failed = 0;

	PRINTK3 ("%s: smc_program_phy()\n", SMC_DEV_NAME);


	/* Get the detected phy address */
	phyaddr = SMC_PHY_ADDR;

	/* Reset the PHY, setting all other bits to zero */
	smc_write_phy_register (PHY_CNTL_REG, PHY_CNTL_RST);

	/* Wait for the reset to complete, or time out */
	timeout = 6;		/* Wait up to 3 seconds */
	while (timeout--) {
		if (!(smc_read_phy_register (PHY_CNTL_REG)
		      & PHY_CNTL_RST)) {
			/* reset complete */
			break;
		}

		smc_wait_ms (500);	/* wait 500 millisecs */
	}

	if (timeout < 1) {
		printf ("%s:PHY reset timed out\n", SMC_DEV_NAME);
		goto smc_phy_configure_exit;
	}

	/* Read PHY Register 18, Status Output */
	/* lp->lastPhy18 = smc_read_phy_register(PHY_INT_REG); */

	/* Enable PHY Interrupts (for register 18) */
	/* Interrupts listed here are disabled */
	smc_write_phy_register (PHY_MASK_REG, 0xffff);

	/* Configure the Receive/Phy Control register */
	SMC_SELECT_BANK (0);
	SMC_outw (RPC_DEFAULT, RPC_REG);

	/* Copy our capabilities from PHY_STAT_REG to PHY_AD_REG */
	my_phy_caps = smc_read_phy_register (PHY_STAT_REG);
	my_ad_caps = PHY_AD_CSMA;	/* I am CSMA capable */

	if (my_phy_caps & PHY_STAT_CAP_T4)
		my_ad_caps |= PHY_AD_T4;

	if (my_phy_caps & PHY_STAT_CAP_TXF)
		my_ad_caps |= PHY_AD_TX_FDX;

	if (my_phy_caps & PHY_STAT_CAP_TXH)
		my_ad_caps |= PHY_AD_TX_HDX;

	if (my_phy_caps & PHY_STAT_CAP_TF)
		my_ad_caps |= PHY_AD_10_FDX;

	if (my_phy_caps & PHY_STAT_CAP_TH)
		my_ad_caps |= PHY_AD_10_HDX;

	/* Update our Auto-Neg Advertisement Register */
	smc_write_phy_register (PHY_AD_REG, my_ad_caps);

	/* Read the register back.  Without this, it appears that when */
	/* auto-negotiation is restarted, sometimes it isn't ready and */
	/* the link does not come up. */
	smc_read_phy_register(PHY_AD_REG);

	PRINTK2 ("%s: phy caps=%x\n", SMC_DEV_NAME, my_phy_caps);
	PRINTK2 ("%s: phy advertised caps=%x\n", SMC_DEV_NAME, my_ad_caps);

	/* Restart auto-negotiation process in order to advertise my caps */
	smc_write_phy_register (PHY_CNTL_REG,
				PHY_CNTL_ANEG_EN | PHY_CNTL_ANEG_RST);

	/* Wait for the auto-negotiation to complete.  This may take from */
	/* 2 to 3 seconds. */
	/* Wait for the reset to complete, or time out */
	timeout = CONFIG_SMC_AUTONEG_TIMEOUT * 2;
	while (timeout--) {

		status = smc_read_phy_register (PHY_STAT_REG);
		if (status & PHY_STAT_ANEG_ACK) {
			/* auto-negotiate complete */
			break;
		}

		smc_wait_ms (500);	/* wait 500 millisecs */

		/* Restart auto-negotiation if remote fault */
		if (status & PHY_STAT_REM_FLT) {
			printf ("%s: PHY remote fault detected\n",
				SMC_DEV_NAME);

			/* Restart auto-negotiation */
			printf ("%s: PHY restarting auto-negotiation\n",
				SMC_DEV_NAME);
			smc_write_phy_register (PHY_CNTL_REG,
						PHY_CNTL_ANEG_EN |
						PHY_CNTL_ANEG_RST |
						PHY_CNTL_SPEED |
						PHY_CNTL_DPLX);
		}
	}

	if (timeout < 1) {
		printf ("%s: PHY auto-negotiate timed out\n", SMC_DEV_NAME);
		failed = 1;
	}

	/* Fail if we detected an auto-negotiate remote fault */
	if (status & PHY_STAT_REM_FLT) {
		printf ("%s: PHY remote fault detected\n", SMC_DEV_NAME);
		failed = 1;
	}

	/* Re-Configure the Receive/Phy Control register */
	SMC_outw (RPC_DEFAULT, RPC_REG);

smc_phy_configure_exit:	;

}
#endif /* !CONFIG_SMC91111_EXT_PHY */


#if SMC_DEBUG > 2
static void print_packet( byte * buf, int length )
{
	int i;
	int remainder;
	int lines;

	printf("Packet of length %d \n", length );

#if SMC_DEBUG > 3
	lines = length / 16;
	remainder = length % 16;

	for ( i = 0; i < lines ; i ++ ) {
		int cur;

		for ( cur = 0; cur < 8; cur ++ ) {
			byte a, b;

			a = *(buf ++ );
			b = *(buf ++ );
			printf("%02x%02x ", a, b );
		}
		printf("\n");
	}
	for ( i = 0; i < remainder/2 ; i++ ) {
		byte a, b;

		a = *(buf ++ );
		b = *(buf ++ );
		printf("%02x%02x ", a, b );
	}
	printf("\n");
#endif
}
#endif

int eth_init(bd_t *bd) {
	return (smc_open(bd));
}

void eth_halt() {
	smc_close();
}

int eth_rx() {
	return smc_rcv();
}

int eth_send(volatile void *packet, int length) {
	return smc_send_packet(packet, length);
}

int smc_get_ethaddr (bd_t * bd)
{
	int env_size, rom_valid, env_present = 0, reg;
	char *s = NULL, *e, *v_mac, es[] = "11:22:33:44:55:66";
	uchar s_env_mac[64], v_env_mac[6], v_rom_mac[6];

	env_size = getenv_r ("ethaddr", s_env_mac, sizeof (s_env_mac));
	if ((env_size > 0) && (env_size < sizeof (es))) {	/* exit if env is bad */
		printf ("\n*** ERROR: ethaddr is not set properly!!\n");
		return (-1);
	}

	if (env_size > 0) {
		env_present = 1;
		s = s_env_mac;
	}

	for (reg = 0; reg < 6; ++reg) { /* turn string into mac value */
		v_env_mac[reg] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	rom_valid = get_rom_mac (v_rom_mac);	/* get ROM mac value if any */

	if (!env_present) {	/* if NO env */
		if (rom_valid) {	/* but ROM is valid */
			v_mac = v_rom_mac;
			sprintf (s_env_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				 v_mac[0], v_mac[1], v_mac[2], v_mac[3],
				 v_mac[4], v_mac[5]);
			setenv ("ethaddr", s_env_mac);
		} else {	/* no env, bad ROM */
			printf ("\n*** ERROR: ethaddr is NOT set !!\n");
			return (-1);
		}
	} else {		/* good env, don't care ROM */
		v_mac = v_env_mac;	/* always use a good env over a ROM */
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
	smc_set_mac_addr (v_mac);	/* use old function to update smc default */
	PRINTK("Using MAC Address %02X:%02X:%02X:%02X:%02X:%02X\n", v_mac[0], v_mac[1],
		v_mac[2], v_mac[3], v_mac[4], v_mac[5]);
	return (0);
}

int get_rom_mac (char *v_rom_mac)
{
#ifdef HARDCODE_MAC	/* used for testing or to supress run time warnings */
	char hw_mac_addr[] = { 0x02, 0x80, 0xad, 0x20, 0x31, 0xb8 };

	memcpy (v_rom_mac, hw_mac_addr, 6);
	return (1);
#else
	int i;
	int valid_mac = 0;

	SMC_SELECT_BANK (1);
	for (i=0; i<6; i++)
	{
		v_rom_mac[i] = SMC_inb ((ADDR0_REG + i));
		valid_mac |= v_rom_mac[i];
	}

	return (valid_mac ? 1 : 0);
#endif
}
#endif /* CONFIG_DRIVER_SMC91111 */

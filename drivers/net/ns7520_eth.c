/***********************************************************************
 *
 * Copyright (C) 2005 by Videon Central, Inc.
 *
 * $Id$
 * @Author: Arthur Shipkowski
 * @Descr: Ethernet driver for the NS7520. Uses polled Ethernet, like
 *     the older netarmeth driver.  Note that attempting to filter
 *     broadcast and multicast out in the SAFR register will cause
 *     bad things due to released errata.
 * @References: [1] NS7520 Hardware Reference, December 2003
 *		[2] Intel LXT971 Datasheet #249414 Rev. 02
 *
 ***********************************************************************/

#include <common.h>

#include <net.h>		/* NetSendPacket */
#include <asm/arch/netarm_registers.h>
#include <asm/arch/netarm_dma_module.h>

#include "ns7520_eth.h"		/* for Ethernet and PHY */

/**
 * Send an error message to the terminal.
 */
#define ERROR(x) \
do { \
	char *__foo = strrchr(__FILE__, '/'); \
	\
	printf("%s: %d: %s(): ", (__foo == NULL ? __FILE__ : (__foo + 1)), \
			__LINE__, __FUNCTION__); \
	printf x; printf("\n"); \
} while (0);

/* some definition to make transistion to linux easier */

#define NS7520_DRIVER_NAME	"eth"
#define KERN_WARNING		"Warning:"
#define KERN_ERR		"Error:"
#define KERN_INFO		"Info:"

#if 1
# define DEBUG
#endif

#ifdef	DEBUG
# define printk			printf

# define DEBUG_INIT		0x0001
# define DEBUG_MINOR		0x0002
# define DEBUG_RX		0x0004
# define DEBUG_TX		0x0008
# define DEBUG_INT		0x0010
# define DEBUG_POLL		0x0020
# define DEBUG_LINK		0x0040
# define DEBUG_MII		0x0100
# define DEBUG_MII_LOW		0x0200
# define DEBUG_MEM		0x0400
# define DEBUG_ERROR		0x4000
# define DEBUG_ERROR_CRIT	0x8000

static int nDebugLvl = DEBUG_ERROR_CRIT;

# define DEBUG_ARGS0( FLG, a0 ) if( ( nDebugLvl & (FLG) ) == (FLG) ) \
		printf("%s: " a0, __FUNCTION__, 0, 0, 0, 0, 0, 0 )
# define DEBUG_ARGS1( FLG, a0, a1 ) if( ( nDebugLvl & (FLG) ) == (FLG)) \
		printf("%s: " a0, __FUNCTION__, (int)(a1), 0, 0, 0, 0, 0 )
# define DEBUG_ARGS2( FLG, a0, a1, a2 ) if( (nDebugLvl & (FLG)) ==(FLG))\
		printf("%s: " a0, __FUNCTION__, (int)(a1), (int)(a2), 0, 0,0,0 )
# define DEBUG_ARGS3( FLG, a0, a1, a2, a3 ) if((nDebugLvl &(FLG))==(FLG))\
		printf("%s: "a0,__FUNCTION__,(int)(a1),(int)(a2),(int)(a3),0,0,0)
# define DEBUG_FN( FLG ) if( (nDebugLvl & (FLG)) == (FLG) ) \
		printf("\r%s:line %d\n", (int)__FUNCTION__, __LINE__, 0,0,0,0);
# define ASSERT( expr, func ) if( !( expr ) ) { \
		printf( "Assertion failed! %s:line %d %s\n", \
		(int)__FUNCTION__,__LINE__,(int)(#expr),0,0,0); \
		func }
#else				/* DEBUG */
# define printk(...)
# define DEBUG_ARGS0( FLG, a0 )
# define DEBUG_ARGS1( FLG, a0, a1 )
# define DEBUG_ARGS2( FLG, a0, a1, a2 )
# define DEBUG_ARGS3( FLG, a0, a1, a2, a3 )
# define DEBUG_FN( n )
# define ASSERT(expr, func)
#endif				/* DEBUG */

#define NS7520_MII_NEG_DELAY		(5*CONFIG_SYS_HZ)	/* in s */
#define TX_TIMEOUT			(5*CONFIG_SYS_HZ)	/* in s */
#define RX_STALL_WORKAROUND_CNT 100

static int ns7520_eth_reset(void);

static void ns7520_link_auto_negotiate(void);
static void ns7520_link_update_egcr(void);
static void ns7520_link_print_changed(void);

/* the PHY stuff */

static char ns7520_mii_identify_phy(void);
static unsigned short ns7520_mii_read(unsigned short uiRegister);
static void ns7520_mii_write(unsigned short uiRegister,
			     unsigned short uiData);
static unsigned int ns7520_mii_get_clock_divisor(unsigned int
						 unMaxMDIOClk);
static unsigned int ns7520_mii_poll_busy(void);

static unsigned int nPhyMaxMdioClock = PHY_MDIO_MAX_CLK;
static unsigned int uiLastLinkStatus;
static PhyType phyDetected = PHY_NONE;

/***********************************************************************
 * @Function: eth_init
 * @Return: -1 on failure otherwise 0
 * @Descr: Initializes the ethernet engine and uses either FS Forth's default
 *	   MAC addr or the one in environment
 ***********************************************************************/

int eth_init(bd_t * pbis)
{
	unsigned char aucMACAddr[6];
	char *pcTmp = getenv("ethaddr");
	char *pcEnd;
	int i;

	DEBUG_FN(DEBUG_INIT);

	/* no need to check for hardware */

	if (!ns7520_eth_reset())
		return -1;

	if (NULL == pcTmp)
		return -1;

	for (i = 0; i < 6; i++) {
		aucMACAddr[i] =
		    pcTmp ? simple_strtoul(pcTmp, &pcEnd, 16) : 0;
		pcTmp = (*pcTmp) ? pcEnd + 1 : pcEnd;
	}

	/* configure ethernet address */

	*get_eth_reg_addr(NS7520_ETH_SA1) =
	    aucMACAddr[5] << 8 | aucMACAddr[4];
	*get_eth_reg_addr(NS7520_ETH_SA2) =
	    aucMACAddr[3] << 8 | aucMACAddr[2];
	*get_eth_reg_addr(NS7520_ETH_SA3) =
	    aucMACAddr[1] << 8 | aucMACAddr[0];

	/* enable hardware */

	*get_eth_reg_addr(NS7520_ETH_MAC1) = NS7520_ETH_MAC1_RXEN;
	*get_eth_reg_addr(NS7520_ETH_SUPP) = NS7520_ETH_SUPP_JABBER;
	*get_eth_reg_addr(NS7520_ETH_MAC1) = NS7520_ETH_MAC1_RXEN;

	/* the linux kernel may give packets < 60 bytes, for example arp */
	*get_eth_reg_addr(NS7520_ETH_MAC2) = NS7520_ETH_MAC2_CRCEN |
	    NS7520_ETH_MAC2_PADEN | NS7520_ETH_MAC2_HUGE;

	/* Broadcast/multicast allowed; if you don't set this even unicast chokes */
	/* Based on NS7520 errata documentation */
	*get_eth_reg_addr(NS7520_ETH_SAFR) =
	    NS7520_ETH_SAFR_BROAD | NS7520_ETH_SAFR_PRM;

	/* enable receive and transmit FIFO, use 10/100 Mbps MII */
	*get_eth_reg_addr(NS7520_ETH_EGCR) |=
	    NS7520_ETH_EGCR_ETXWM_75 |
	    NS7520_ETH_EGCR_ERX |
	    NS7520_ETH_EGCR_ERXREG |
	    NS7520_ETH_EGCR_ERXBR | NS7520_ETH_EGCR_ETX;

	return 0;
}

/***********************************************************************
 * @Function: eth_send
 * @Return: -1 on timeout otherwise 1
 * @Descr: sends one frame by DMA
 ***********************************************************************/

int eth_send(volatile void *pPacket, int nLen)
{
	int i, length32, retval = 1;
	char *pa;
	unsigned int *pa32, lastp = 0, rest;
	unsigned int status;

	pa = (char *) pPacket;
	pa32 = (unsigned int *) pPacket;
	length32 = nLen / 4;
	rest = nLen % 4;

	/* make sure there's no garbage in the last word */
	switch (rest) {
	case 0:
		lastp = pa32[length32 - 1];
		length32--;
		break;
	case 1:
		lastp = pa32[length32] & 0x000000ff;
		break;
	case 2:
		lastp = pa32[length32] & 0x0000ffff;
		break;
	case 3:
		lastp = pa32[length32] & 0x00ffffff;
		break;
	}

	while (((*get_eth_reg_addr(NS7520_ETH_EGSR)) &
		NS7520_ETH_EGSR_TXREGE)
	       == 0) {
	}

	/* write to the fifo */
	for (i = 0; i < length32; i++)
		*get_eth_reg_addr(NS7520_ETH_FIFO) = pa32[i];

	/* the last word is written to an extra register, this
	   starts the transmission */
	*get_eth_reg_addr(NS7520_ETH_FIFOL) = lastp;

	/* Wait for it to be done */
	while ((*get_eth_reg_addr(NS7520_ETH_EGSR) & NS7520_ETH_EGSR_TXBC)
	       == 0) {
	}
	status = (*get_eth_reg_addr(NS7520_ETH_ETSR));
	*get_eth_reg_addr(NS7520_ETH_EGSR) = NS7520_ETH_EGSR_TXBC;	/* Clear it now */

	if (status & NS7520_ETH_ETSR_TXOK) {
		retval = 0;	/* We're OK! */
	} else if (status & NS7520_ETH_ETSR_TXDEF) {
		printf("Deferred, we'll see.\n");
		retval = 0;
	} else if (status & NS7520_ETH_ETSR_TXAL) {
		printf("Late collision error, %d collisions.\n",
		       (*get_eth_reg_addr(NS7520_ETH_ETSR)) &
		       NS7520_ETH_ETSR_TXCOLC);
	} else if (status & NS7520_ETH_ETSR_TXAEC) {
		printf("Excessive collisions: %d\n",
		       (*get_eth_reg_addr(NS7520_ETH_ETSR)) &
		       NS7520_ETH_ETSR_TXCOLC);
	} else if (status & NS7520_ETH_ETSR_TXAED) {
		printf("Excessive deferral on xmit.\n");
	} else if (status & NS7520_ETH_ETSR_TXAUR) {
		printf("Packet underrun.\n");
	} else if (status & NS7520_ETH_ETSR_TXAJ) {
		printf("Jumbo packet error.\n");
	} else {
		printf("Error: Should never get here.\n");
	}

	return (retval);
}

/***********************************************************************
 * @Function: eth_rx
 * @Return: size of last frame in bytes or 0 if no frame available
 * @Descr: gives one frame to U-Boot which has been copied by DMA engine already
 *	   to NetRxPackets[ 0 ].
 ***********************************************************************/

int eth_rx(void)
{
	int i;
	unsigned short rxlen;
	unsigned short totrxlen = 0;
	unsigned int *addr;
	unsigned int rxstatus, lastrxlen;
	char *pa;

	/* If RXBR is 1, data block was received */
	while (((*get_eth_reg_addr(NS7520_ETH_EGSR)) &
		NS7520_ETH_EGSR_RXBR) == NS7520_ETH_EGSR_RXBR) {

		/* get status register and the length of received block */
		rxstatus = *get_eth_reg_addr(NS7520_ETH_ERSR);
		rxlen = (rxstatus & NS7520_ETH_ERSR_RXSIZE) >> 16;

		/* clear RXBR to make fifo available */
		*get_eth_reg_addr(NS7520_ETH_EGSR) = NS7520_ETH_EGSR_RXBR;

		if (rxstatus & NS7520_ETH_ERSR_ROVER) {
			printf("Receive overrun, resetting FIFO.\n");
			*get_eth_reg_addr(NS7520_ETH_EGCR) &=
			    ~NS7520_ETH_EGCR_ERX;
			udelay(20);
			*get_eth_reg_addr(NS7520_ETH_EGCR) |=
			    NS7520_ETH_EGCR_ERX;
		}
		if (rxlen == 0) {
			printf("Nothing.\n");
			return 0;
		}

		addr = (unsigned int *) NetRxPackets[0];
		pa = (char *) NetRxPackets[0];

		/* read the fifo */
		for (i = 0; i < rxlen / 4; i++) {
			*addr = *get_eth_reg_addr(NS7520_ETH_FIFO);
			addr++;
		}

		if ((*get_eth_reg_addr(NS7520_ETH_EGSR)) &
		    NS7520_ETH_EGSR_RXREGR) {
			/* RXFDB indicates wether the last word is 1,2,3 or 4 bytes long */
			lastrxlen =
			    ((*get_eth_reg_addr(NS7520_ETH_EGSR)) &
			     NS7520_ETH_EGSR_RXFDB_MA) >> 28;
			*addr = *get_eth_reg_addr(NS7520_ETH_FIFO);
			switch (lastrxlen) {
			case 1:
				*addr &= 0xff000000;
				break;
			case 2:
				*addr &= 0xffff0000;
				break;
			case 3:
				*addr &= 0xffffff00;
				break;
			}
		}

		/* Pass the packet up to the protocol layers. */
		NetReceive(NetRxPackets[0], rxlen - 4);
		totrxlen += rxlen - 4;
	}

	return totrxlen;
}

/***********************************************************************
 * @Function: eth_halt
 * @Return: n/a
 * @Descr: stops the ethernet engine
 ***********************************************************************/

void eth_halt(void)
{
	DEBUG_FN(DEBUG_INIT);

	*get_eth_reg_addr(NS7520_ETH_MAC1) &= ~NS7520_ETH_MAC1_RXEN;
	*get_eth_reg_addr(NS7520_ETH_EGCR) &= ~(NS7520_ETH_EGCR_ERX |
						NS7520_ETH_EGCR_ERXDMA |
						NS7520_ETH_EGCR_ERXREG |
						NS7520_ETH_EGCR_ERXBR |
						NS7520_ETH_EGCR_ETX |
						NS7520_ETH_EGCR_ETXDMA);
}

/***********************************************************************
 * @Function: ns7520_eth_reset
 * @Return: 0 on failure otherwise 1
 * @Descr: resets the ethernet interface and the PHY,
 *	   performs auto negotiation or fixed modes
 ***********************************************************************/

static int ns7520_eth_reset(void)
{
	DEBUG_FN(DEBUG_MINOR);

	/* Reset important registers */
	*get_eth_reg_addr(NS7520_ETH_EGCR) = 0;	/* Null it out! */
	*get_eth_reg_addr(NS7520_ETH_MAC1) &= NS7520_ETH_MAC1_SRST;
	*get_eth_reg_addr(NS7520_ETH_MAC2) = 0;
	/* Reset MAC */
	*get_eth_reg_addr(NS7520_ETH_EGCR) |= NS7520_ETH_EGCR_MAC_RES;
	udelay(5);
	*get_eth_reg_addr(NS7520_ETH_EGCR) &= ~NS7520_ETH_EGCR_MAC_RES;

	/* reset and initialize PHY */

	*get_eth_reg_addr(NS7520_ETH_MAC1) &= ~NS7520_ETH_MAC1_SRST;

	/* we don't support hot plugging of PHY, therefore we don't reset
	   phyDetected and nPhyMaxMdioClock here. The risk is if the setting is
	   incorrect the first open
	   may detect the PHY correctly but succeding will fail
	   For reseting the PHY and identifying we have to use the standard
	   MDIO CLOCK value 2.5 MHz only after hardware reset
	   After having identified the PHY we will do faster */

	*get_eth_reg_addr(NS7520_ETH_MCFG) =
	    ns7520_mii_get_clock_divisor(nPhyMaxMdioClock);

	/* reset PHY */
	ns7520_mii_write(MII_BMCR, BMCR_RESET);
	ns7520_mii_write(MII_BMCR, 0);

	udelay(3000);		/* [2] p.70 says at least 300us reset recovery time. */

	/* MII clock has been setup to default, ns7520_mii_identify_phy should
	   work for all */

	if (!ns7520_mii_identify_phy()) {
		printk(KERN_ERR NS7520_DRIVER_NAME
		       ": Unsupported PHY, aborting\n");
		return 0;
	}

	/* now take the highest MDIO clock possible after detection */
	*get_eth_reg_addr(NS7520_ETH_MCFG) =
	    ns7520_mii_get_clock_divisor(nPhyMaxMdioClock);

	/* PHY has been detected, so there can be no abort reason and we can
	   finish initializing ethernet */

	uiLastLinkStatus = 0xff;	/* undefined */

	ns7520_link_auto_negotiate();

	if (phyDetected == PHY_LXT971A)
		/* set LED2 to link mode */
		ns7520_mii_write(PHY_LXT971_LED_CFG,
				 (PHY_LXT971_LED_CFG_LINK_ACT <<
				  PHY_LXT971_LED_CFG_SHIFT_LED2) |
				 (PHY_LXT971_LED_CFG_TRANSMIT <<
				  PHY_LXT971_LED_CFG_SHIFT_LED1));

	return 1;
}

/***********************************************************************
 * @Function: ns7520_link_auto_negotiate
 * @Return: void
 * @Descr: performs auto-negotation of link.
 ***********************************************************************/

static void ns7520_link_auto_negotiate(void)
{
	unsigned long ulStartJiffies;
	unsigned short uiStatus;

	DEBUG_FN(DEBUG_LINK);

	/* run auto-negotation */
	/* define what we are capable of */
	ns7520_mii_write(MII_ADVERTISE,
			 LPA_100FULL |
			 LPA_100HALF |
			 LPA_10FULL |
			 LPA_10HALF |
			 PHY_ANLPAR_PSB_802_3);
	/* start auto-negotiation */
	ns7520_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART);

	/* wait for completion */

	ulStartJiffies = get_timer(0);
	while (get_timer(0) < ulStartJiffies + NS7520_MII_NEG_DELAY) {
		uiStatus = ns7520_mii_read(MII_BMSR);
		if ((uiStatus &
		     (BMSR_ANEGCOMPLETE | BMSR_LSTATUS)) ==
		    (BMSR_ANEGCOMPLETE | BMSR_LSTATUS)) {
			/* lucky we are, auto-negotiation succeeded */
			ns7520_link_print_changed();
			ns7520_link_update_egcr();
			return;
		}
	}

	DEBUG_ARGS0(DEBUG_LINK, "auto-negotiation timed out\n");
	/* ignore invalid link settings */
}

/***********************************************************************
 * @Function: ns7520_link_update_egcr
 * @Return: void
 * @Descr: updates the EGCR and MAC2 link status after mode change or
 *	   auto-negotation
 ***********************************************************************/

static void ns7520_link_update_egcr(void)
{
	unsigned int unEGCR;
	unsigned int unMAC2;
	unsigned int unIPGT;

	DEBUG_FN(DEBUG_LINK);

	unEGCR = *get_eth_reg_addr(NS7520_ETH_EGCR);
	unMAC2 = *get_eth_reg_addr(NS7520_ETH_MAC2);
	unIPGT =
	    *get_eth_reg_addr(NS7520_ETH_IPGT) & ~NS7520_ETH_IPGT_IPGT;

	unEGCR &= ~NS7520_ETH_EGCR_EFULLD;
	unMAC2 &= ~NS7520_ETH_MAC2_FULLD;
	if ((uiLastLinkStatus & PHY_LXT971_STAT2_DUPLEX_MODE)
	    == PHY_LXT971_STAT2_DUPLEX_MODE) {
		unEGCR |= NS7520_ETH_EGCR_EFULLD;
		unMAC2 |= NS7520_ETH_MAC2_FULLD;
		unIPGT |= 0x15;	/* see [1] p. 167 */
	} else
		unIPGT |= 0x12;	/* see [1] p. 167 */

	*get_eth_reg_addr(NS7520_ETH_MAC2) = unMAC2;
	*get_eth_reg_addr(NS7520_ETH_EGCR) = unEGCR;
	*get_eth_reg_addr(NS7520_ETH_IPGT) = unIPGT;
}

/***********************************************************************
 * @Function: ns7520_link_print_changed
 * @Return: void
 * @Descr: checks whether the link status has changed and if so prints
 *	   the new mode
 ***********************************************************************/

static void ns7520_link_print_changed(void)
{
	unsigned short uiStatus;
	unsigned short uiControl;

	DEBUG_FN(DEBUG_LINK);

	uiControl = ns7520_mii_read(MII_BMCR);

	if ((uiControl & BMCR_ANENABLE) == BMCR_ANENABLE) {
		/* BMSR_LSTATUS is only set on autonegotiation */
		uiStatus = ns7520_mii_read(MII_BMSR);

		if (!(uiStatus & BMSR_LSTATUS)) {
			printk(KERN_WARNING NS7520_DRIVER_NAME
			       ": link down\n");
			/* @TODO Linux: carrier_off */
		} else {
			/* @TODO Linux: carrier_on */
			if (phyDetected == PHY_LXT971A) {
				uiStatus =
				    ns7520_mii_read(PHY_LXT971_STAT2);
				uiStatus &=
				    (PHY_LXT971_STAT2_100BTX |
				     PHY_LXT971_STAT2_DUPLEX_MODE |
				     PHY_LXT971_STAT2_AUTO_NEG);

				/* mask out all uninteresting parts */
			}
			/* other PHYs must store there link information in
			   uiStatus as PHY_LXT971 */
		}
	} else {
		/* mode has been forced, so uiStatus should be the same as the
		   last link status, enforce printing */
		uiStatus = uiLastLinkStatus;
		uiLastLinkStatus = 0xff;
	}

	if (uiStatus != uiLastLinkStatus) {
		/* save current link status */
		uiLastLinkStatus = uiStatus;

		/* print new link status */

		printk(KERN_INFO NS7520_DRIVER_NAME
		       ": link mode %i Mbps %s duplex %s\n",
		       (uiStatus & PHY_LXT971_STAT2_100BTX) ? 100 : 10,
		       (uiStatus & PHY_LXT971_STAT2_DUPLEX_MODE) ? "full" :
		       "half",
		       (uiStatus & PHY_LXT971_STAT2_AUTO_NEG) ? "(auto)" :
		       "");
	}
}

/***********************************************************************
 * the MII low level stuff
 ***********************************************************************/

/***********************************************************************
 * @Function: ns7520_mii_identify_phy
 * @Return: 1 if supported PHY has been detected otherwise 0
 * @Descr: checks for supported PHY and prints the IDs.
 ***********************************************************************/

static char ns7520_mii_identify_phy(void)
{
	unsigned short uiID1;
	unsigned short uiID2;
	unsigned char *szName;
	char cRes = 0;

	DEBUG_FN(DEBUG_MII);

	phyDetected = (PhyType) uiID1 = ns7520_mii_read(MII_PHYSID1);

	switch (phyDetected) {
	case PHY_LXT971A:
		szName = "LXT971A";
		uiID2 = ns7520_mii_read(MII_PHYSID2);
		nPhyMaxMdioClock = PHY_LXT971_MDIO_MAX_CLK;
		cRes = 1;
		break;
	case PHY_NONE:
	default:
		/* in case uiID1 == 0 && uiID2 == 0 we may have the wrong
		   address or reset sets the wrong NS7520_ETH_MCFG_CLKS */

		uiID2 = 0;
		szName = "unknown";
		nPhyMaxMdioClock = PHY_MDIO_MAX_CLK;
		phyDetected = PHY_NONE;
	}

	printk(KERN_INFO NS7520_DRIVER_NAME
	       ": PHY (0x%x, 0x%x) = %s detected\n", uiID1, uiID2, szName);

	return cRes;
}

/***********************************************************************
 * @Function: ns7520_mii_read
 * @Return: the data read from PHY register uiRegister
 * @Descr: the data read may be invalid if timed out. If so, a message
 *	   is printed but the invalid data is returned.
 *	   The fixed device address is being used.
 ***********************************************************************/

static unsigned short ns7520_mii_read(unsigned short uiRegister)
{
	DEBUG_FN(DEBUG_MII_LOW);

	/* write MII register to be read */
	*get_eth_reg_addr(NS7520_ETH_MADR) =
	    CONFIG_PHY_ADDR << 8 | uiRegister;

	*get_eth_reg_addr(NS7520_ETH_MCMD) = NS7520_ETH_MCMD_READ;

	if (!ns7520_mii_poll_busy())
		printk(KERN_WARNING NS7520_DRIVER_NAME
		       ": MII still busy in read\n");
	/* continue to read */

	*get_eth_reg_addr(NS7520_ETH_MCMD) = 0;

	return (unsigned short) (*get_eth_reg_addr(NS7520_ETH_MRDD));
}

/***********************************************************************
 * @Function: ns7520_mii_write
 * @Return: nothing
 * @Descr: writes the data to the PHY register. In case of a timeout,
 *	   no special handling is performed but a message printed
 *	   The fixed device address is being used.
 ***********************************************************************/

static void ns7520_mii_write(unsigned short uiRegister,
			     unsigned short uiData)
{
	DEBUG_FN(DEBUG_MII_LOW);

	/* write MII register to be written */
	*get_eth_reg_addr(NS7520_ETH_MADR) =
	    CONFIG_PHY_ADDR << 8 | uiRegister;

	*get_eth_reg_addr(NS7520_ETH_MWTD) = uiData;

	if (!ns7520_mii_poll_busy()) {
		printf(KERN_WARNING NS7520_DRIVER_NAME
		       ": MII still busy in write\n");
	}
}

/***********************************************************************
 * @Function: ns7520_mii_get_clock_divisor
 * @Return: the clock divisor that should be used in NS7520_ETH_MCFG_CLKS
 * @Descr: if no clock divisor can be calculated for the
 *	   current SYSCLK and the maximum MDIO Clock, a warning is printed
 *	   and the greatest divisor is taken
 ***********************************************************************/

static unsigned int ns7520_mii_get_clock_divisor(unsigned int unMaxMDIOClk)
{
	struct {
		unsigned int unSysClkDivisor;
		unsigned int unClks;	/* field for NS7520_ETH_MCFG_CLKS */
	} PHYClockDivisors[] = {
		{
		4, NS7520_ETH_MCFG_CLKS_4}, {
		6, NS7520_ETH_MCFG_CLKS_6}, {
		8, NS7520_ETH_MCFG_CLKS_8}, {
		10, NS7520_ETH_MCFG_CLKS_10}, {
		14, NS7520_ETH_MCFG_CLKS_14}, {
		20, NS7520_ETH_MCFG_CLKS_20}, {
		28, NS7520_ETH_MCFG_CLKS_28}
	};

	int nIndexSysClkDiv;
	int nArraySize =
	    sizeof(PHYClockDivisors) / sizeof(PHYClockDivisors[0]);
	unsigned int unClks = NS7520_ETH_MCFG_CLKS_28;	/* defaults to
							   greatest div */

	DEBUG_FN(DEBUG_INIT);

	for (nIndexSysClkDiv = 0; nIndexSysClkDiv < nArraySize;
	     nIndexSysClkDiv++) {
		/* find first sysclock divisor that isn't higher than 2.5 MHz
		   clock */
		if (NETARM_XTAL_FREQ /
		    PHYClockDivisors[nIndexSysClkDiv].unSysClkDivisor <=
		    unMaxMDIOClk) {
			unClks = PHYClockDivisors[nIndexSysClkDiv].unClks;
			break;
		}
	}

	DEBUG_ARGS2(DEBUG_INIT,
		    "Taking MDIO Clock bit mask 0x%0x for max clock %i\n",
		    unClks, unMaxMDIOClk);

	/* return greatest divisor */
	return unClks;
}

/***********************************************************************
 * @Function: ns7520_mii_poll_busy
 * @Return: 0 if timed out otherwise the remaing timeout
 * @Descr: waits until the MII has completed a command or it times out
 *	   code may be interrupted by hard interrupts.
 *	   It is not checked what happens on multiple actions when
 *	   the first is still being busy and we timeout.
 ***********************************************************************/

static unsigned int ns7520_mii_poll_busy(void)
{
	unsigned int unTimeout = 1000;

	DEBUG_FN(DEBUG_MII_LOW);

	while (((*get_eth_reg_addr(NS7520_ETH_MIND) & NS7520_ETH_MIND_BUSY)
		== NS7520_ETH_MIND_BUSY) && unTimeout)
		unTimeout--;

	return unTimeout;
}

/* ----------------------------------------------------------------------------
 * Net+ARM ethernet MII functionality.
 */
#if defined(CONFIG_MII)

/**
 * Maximum MII address we support
 */
#define MII_ADDRESS_MAX			(31)

/**
 * Maximum MII register address we support
 */
#define MII_REGISTER_MAX		(31)

/**
 * Ethernet MII interface return values for public functions.
 */
enum mii_status {
	MII_STATUS_SUCCESS = 0,
	MII_STATUS_FAILURE = 1,
};

/**
 * Read a 16-bit value from an MII register.
 */
extern int ns7520_miiphy_read(const char *devname, unsigned char const addr,
		unsigned char const reg, unsigned short *const value)
{
	int ret = MII_STATUS_FAILURE;

	/* Parameter checks */
	if (addr > MII_ADDRESS_MAX) {
		ERROR(("invalid addr, 0x%02X", addr));
		goto miiphy_read_failed_0;
	}

	if (reg > MII_REGISTER_MAX) {
		ERROR(("invalid reg, 0x%02X", reg));
		goto miiphy_read_failed_0;
	}

	if (value == NULL) {
		ERROR(("NULL value"));
		goto miiphy_read_failed_0;
	}

	DEBUG_FN(DEBUG_MII_LOW);

	/* write MII register to be read */
	*get_eth_reg_addr(NS7520_ETH_MADR) = (addr << 8) | reg;

	*get_eth_reg_addr(NS7520_ETH_MCMD) = NS7520_ETH_MCMD_READ;

	if (!ns7520_mii_poll_busy())
		printk(KERN_WARNING NS7520_DRIVER_NAME
		       ": MII still busy in read\n");
	/* continue to read */

	*get_eth_reg_addr(NS7520_ETH_MCMD) = 0;

	*value = (*get_eth_reg_addr(NS7520_ETH_MRDD));
	ret = MII_STATUS_SUCCESS;
	/* Fall through */

      miiphy_read_failed_0:
	return (ret);
}

/**
 * Write a 16-bit value to an MII register.
 */
extern int ns7520_miiphy_write(const char *devname, unsigned char const addr,
		unsigned char const reg, unsigned short const value)
{
	int ret = MII_STATUS_FAILURE;

	/* Parameter checks */
	if (addr > MII_ADDRESS_MAX) {
		ERROR(("invalid addr, 0x%02X", addr));
		goto miiphy_write_failed_0;
	}

	if (reg > MII_REGISTER_MAX) {
		ERROR(("invalid reg, 0x%02X", reg));
		goto miiphy_write_failed_0;
	}

	/* write MII register to be written */
	*get_eth_reg_addr(NS7520_ETH_MADR) = (addr << 8) | reg;

	*get_eth_reg_addr(NS7520_ETH_MWTD) = value;

	if (!ns7520_mii_poll_busy()) {
		printf(KERN_WARNING NS7520_DRIVER_NAME
		       ": MII still busy in write\n");
	}

	ret = MII_STATUS_SUCCESS;
	/* Fall through */

      miiphy_write_failed_0:
	return (ret);
}
#endif				/* defined(CONFIG_MII) */

int ns7520_miiphy_initialize(bd_t *bis)
{
#if defined(CONFIG_MII)
	miiphy_register("ns7520phy", ns7520_miiphy_read, ns7520_miiphy_write);
#endif
	return 0;
}

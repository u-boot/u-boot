/*
 * (C) Copyright 2000
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
#include <malloc.h>
#include <commproc.h>
#include <net.h>
#include <command.h>

#undef	ET_DEBUG

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(FEC_ENET)

#ifdef CFG_DISCOVER_PHY
#include <miiphy.h>
static void mii_discover_phy(void);
#endif

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH  1520

#define TX_BUF_CNT 2

#define TOUT_LOOP 100

#define PKT_MAXBUF_SIZE		1518
#define PKT_MINBUF_SIZE		64
#define PKT_MAXBLR_SIZE		1520


static char txbuf[DBUF_LENGTH];

static uint rxIdx;	/* index of the current RX buffer */
static uint txIdx;	/* index of the current TX buffer */

/*
  * FEC Ethernet Tx and Rx buffer descriptors allocated at the
  *  immr->udata_bd address on Dual-Port RAM
  * Provide for Double Buffering
  */

typedef volatile struct CommonBufferDescriptor {
    cbd_t rxbd[PKTBUFSRX];		/* Rx BD */
    cbd_t txbd[TX_BUF_CNT];		/* Tx BD */
} RTXBD;

static RTXBD *rtx = NULL;

static int fec_send(struct eth_device* dev, volatile void *packet, int length);
static int fec_recv(struct eth_device* dev);
static int fec_init(struct eth_device* dev, bd_t * bd);
static void fec_halt(struct eth_device* dev);

int fec_initialize(bd_t *bis)
{
	struct eth_device* dev;

	dev = (struct eth_device*) malloc(sizeof *dev);
	memset(dev, 0, sizeof *dev);

	sprintf(dev->name, "FEC ETHERNET");
	dev->iobase = 0;
	dev->priv   = 0;
	dev->init   = fec_init;
	dev->halt   = fec_halt;
	dev->send   = fec_send;
	dev->recv   = fec_recv;

	eth_register(dev);

	return 1;
}

static int fec_send(struct eth_device* dev, volatile void *packet, int length)
{
	int j, rc;
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile fec_t *fecp = &(immr->im_cpm.cp_fec);

	/* section 16.9.23.3
	 * Wait for ready
	 */
	j = 0;
	while ((rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_READY) && (j<TOUT_LOOP)) {
		udelay(1);
		j++;
	}
	if (j>=TOUT_LOOP) {
		printf("TX not ready\n");
	}

	rtx->txbd[txIdx].cbd_bufaddr = (uint)packet;
	rtx->txbd[txIdx].cbd_datlen  = length;
	rtx->txbd[txIdx].cbd_sc |= BD_ENET_TX_READY | BD_ENET_TX_LAST;
	__asm__ ("eieio");

	/* Activate transmit Buffer Descriptor polling */
	fecp->fec_x_des_active = 0x01000000;	/* Descriptor polling active	*/

	j = 0;
	while ((rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_READY) && (j<TOUT_LOOP)) {
#if defined(CONFIG_ICU862)
		udelay(10);
#else
		udelay(1);
#endif
		j++;
	}
	if (j>=TOUT_LOOP) {
		printf("TX timeout\n");
	}
#ifdef ET_DEBUG
	printf("%s[%d] %s: cycles: %d    status: %x  retry cnt: %d\n",
	__FILE__,__LINE__,__FUNCTION__,j,rtx->txbd[txIdx].cbd_sc,
	(rtx->txbd[txIdx].cbd_sc & 0x003C)>>2);
#endif
	/* return only status bits */;
	rc = (rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_STATS);

	txIdx = (txIdx + 1) % TX_BUF_CNT;

	return rc;
}

static int fec_recv(struct eth_device* dev)
{
	int length;
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile fec_t *fecp = &(immr->im_cpm.cp_fec);

   for (;;) {
	/* section 16.9.23.2 */
	if (rtx->rxbd[rxIdx].cbd_sc & BD_ENET_RX_EMPTY) {
		length = -1;
		break;     /* nothing received - leave for() loop */
	}

	length = rtx->rxbd[rxIdx].cbd_datlen;

	if (rtx->rxbd[rxIdx].cbd_sc & 0x003f) {
#ifdef ET_DEBUG
		printf("%s[%d] err: %x\n",
		__FUNCTION__,__LINE__,rtx->rxbd[rxIdx].cbd_sc);
#endif
	} else {
		/* Pass the packet up to the protocol layers. */
		NetReceive(NetRxPackets[rxIdx], length - 4);
	}

	/* Give the buffer back to the FEC. */
	rtx->rxbd[rxIdx].cbd_datlen = 0;

	/* wrap around buffer index when necessary */
	if ((rxIdx + 1) >= PKTBUFSRX) {
		rtx->rxbd[PKTBUFSRX - 1].cbd_sc = (BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY);
		rxIdx = 0;
	} else {
		rtx->rxbd[rxIdx].cbd_sc = BD_ENET_RX_EMPTY;
		rxIdx++;
	}

	__asm__ ("eieio");

	/* Try to fill Buffer Descriptors */
	fecp->fec_r_des_active = 0x01000000;	/* Descriptor polling active	*/
   }

   return length;
}

/**************************************************************
 *
 * FEC Ethernet Initialization Routine
 *
 *************************************************************/

#define	FEC_ECNTRL_PINMUX	0x00000004
#define FEC_ECNTRL_ETHER_EN	0x00000002
#define FEC_ECNTRL_RESET	0x00000001

#define FEC_RCNTRL_BC_REJ	0x00000010
#define FEC_RCNTRL_PROM		0x00000008
#define FEC_RCNTRL_MII_MODE	0x00000004
#define FEC_RCNTRL_DRT		0x00000002
#define FEC_RCNTRL_LOOP		0x00000001

#define FEC_TCNTRL_FDEN		0x00000004
#define FEC_TCNTRL_HBC		0x00000002
#define FEC_TCNTRL_GTS		0x00000001

#define	FEC_RESET_DELAY		50

static int fec_init(struct eth_device* dev, bd_t * bd)
{

	int i;
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile fec_t *fecp = &(immr->im_cpm.cp_fec);

#if defined(CONFIG_FADS) && defined(CONFIG_MPC860T)
	/* configure FADS for fast (FEC) ethernet, half-duplex */
	/* The LXT970 needs about 50ms to recover from reset, so
	 * wait for it by discovering the PHY before leaving eth_init().
	 */
	{
		volatile uint *bcsr4 = (volatile uint *) BCSR4;
		*bcsr4 = (*bcsr4 & ~(BCSR4_FETH_EN | BCSR4_FETHCFG1))
			| (BCSR4_FETHCFG0 | BCSR4_FETHFDE | BCSR4_FETHRST);

		/* reset the LXT970 PHY */
		*bcsr4 &= ~BCSR4_FETHRST;
		udelay (10);
		*bcsr4 |= BCSR4_FETHRST;
		udelay (10);
	}
#endif
	/* Whack a reset.
	 * A delay is required between a reset of the FEC block and
	 * initialization of other FEC registers because the reset takes
	 * some time to complete. If you don't delay, subsequent writes
	 * to FEC registers might get killed by the reset routine which is
	 * still in progress.
	 */
	fecp->fec_ecntrl = FEC_ECNTRL_PINMUX | FEC_ECNTRL_RESET;
	for (i = 0;
	     (fecp->fec_ecntrl & FEC_ECNTRL_RESET) && (i < FEC_RESET_DELAY);
	     ++i) {
		udelay (1);
	}
	if (i == FEC_RESET_DELAY) {
		printf ("FEC_RESET_DELAY timeout\n");
		return 0;
	}

	/* We use strictly polling mode only
	 */
	fecp->fec_imask = 0;

	/* Clear any pending interrupt
	 */
	fecp->fec_ievent = 0xffc0;

	/* No need to set the IVEC register */

	/* Set station address
	 */
#define ea eth_get_dev()->enetaddr
	fecp->fec_addr_low   =	(ea[0] << 24) | (ea[1] << 16) |
				(ea[2] <<  8) | (ea[3]      ) ;
	fecp->fec_addr_high  =	(ea[4] <<  8) | (ea[5]	    ) ;
#undef ea

	/* Clear multicast address hash table
	 */
	fecp->fec_hash_table_high = 0;
	fecp->fec_hash_table_low  = 0;

	/* Set maximum receive buffer size.
	 */
	fecp->fec_r_buff_size = PKT_MAXBLR_SIZE;

	/* Set maximum frame length
	 */
	fecp->fec_r_hash = PKT_MAXBUF_SIZE;

	/*
	 * Setup Buffers and Buffer Desriptors
	 */
	rxIdx = 0;
	txIdx = 0;

	if (!rtx) {
#ifdef CFG_ALLOC_DPRAM
	    rtx = (RTXBD *) (immr->im_cpm.cp_dpmem + dpram_alloc_align(sizeof(RTXBD),8));
#else
	    rtx = (RTXBD *) (immr->im_cpm.cp_dpmem + CPM_FEC_BASE);
#endif
	}
	/*
	 * Setup Receiver Buffer Descriptors (13.14.24.18)
	 * Settings:
	 *     Empty, Wrap
	 */
	for (i = 0; i < PKTBUFSRX; i++) {
		rtx->rxbd[i].cbd_sc      = BD_ENET_RX_EMPTY;
		rtx->rxbd[i].cbd_datlen  = 0;	/* Reset */
		rtx->rxbd[i].cbd_bufaddr = (uint) NetRxPackets[i];
	}
	rtx->rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

	/*
	 * Setup Ethernet Transmitter Buffer Descriptors (13.14.24.19)
	 * Settings:
	 *    Last, Tx CRC
	 */
	for (i = 0; i < TX_BUF_CNT; i++) {
		rtx->txbd[i].cbd_sc      = BD_ENET_TX_LAST | BD_ENET_TX_TC;
		rtx->txbd[i].cbd_datlen  = 0;	/* Reset */
		rtx->txbd[i].cbd_bufaddr = (uint) (&txbuf[0]);
	}
	rtx->txbd[TX_BUF_CNT - 1].cbd_sc |= BD_ENET_TX_WRAP;

	/* Set receive and transmit descriptor base
	 */
	fecp->fec_r_des_start = (unsigned int) (&rtx->rxbd[0]);
	fecp->fec_x_des_start = (unsigned int) (&rtx->txbd[0]);

	/* Enable MII mode
	 */
#if 0	/* Full duplex mode */
	fecp->fec_r_cntrl = FEC_RCNTRL_MII_MODE;
	fecp->fec_x_cntrl = FEC_TCNTRL_FDEN;
#else	/* Half duplex mode */
	fecp->fec_r_cntrl = FEC_RCNTRL_MII_MODE | FEC_RCNTRL_DRT;
	fecp->fec_x_cntrl = 0;
#endif

	/* Enable big endian and don't care about SDMA FC.
	 */
	fecp->fec_fun_code = 0x78000000;

	/* Set MII speed to 2.5 MHz or slightly below.
	 * According to the MPC860T (Rev. D) Fast ethernet controller user
	 * manual (6.2.14),
	 * the MII management interface clock must be less than or equal
	 * to 2.5 MHz.
	 * This MDC frequency is equal to system clock / (2 * MII_SPEED).
	 * Then MII_SPEED = system_clock / 2 * 2,5 Mhz.
	 */
	fecp->fec_mii_speed = ((bd->bi_busfreq + 4999999) / 5000000) << 1;

#if !defined(CONFIG_ICU862) && !defined(CONFIG_IAD210)
	/* Configure all of port D for MII.
	 */
	immr->im_ioport.iop_pdpar = 0x1fff;

	/* Bits moved from Rev. D onward */
	if ((get_immr (0) & 0xffff) < 0x0501) {
		immr->im_ioport.iop_pddir = 0x1c58;	/* Pre rev. D */
	} else {
		immr->im_ioport.iop_pddir = 0x1fff;	/* Rev. D and later */
	}
#else
	/* Configure port A for MII.
	*/

#if defined(CONFIG_ICU862) && defined(CFG_DISCOVER_PHY)

	/* On the ICU862 board the MII-MDC pin is routed to PD8 pin
	 * of CPU, so for this board we need to configure Utopia and
	 * enable PD8 to MII-MDC function */
	immr->im_ioport.iop_pdpar |= 0x4080;
#endif

	/* Has Utopia been configured? */
	if (immr->im_ioport.iop_pdpar & (0x8000 >> 1)) {
		/*
		 * YES - Use MUXED mode for UTOPIA bus.
		 * This frees Port A for use by MII (see 862UM table 41-6).
		 */
		immr->im_ioport.utmode &= ~0x80;
	} else {
		/*
		 * NO - set SPLIT mode for UTOPIA bus.
		 *
		 * This doesn't really effect UTOPIA (which isn't
		 * enabled anyway) but just tells the 862
		 * to use port A for MII (see 862UM table 41-6).
		 */
		immr->im_ioport.utmode |= 0x80;
	}
#endif	/* !defined(CONFIG_ICU862) */

	rxIdx = 0;
	txIdx = 0;

	/* Now enable the transmit and receive processing
	 */
	fecp->fec_ecntrl = FEC_ECNTRL_PINMUX | FEC_ECNTRL_ETHER_EN;

#ifdef CFG_DISCOVER_PHY
	/* wait for the PHY to wake up after reset
	 */
	mii_discover_phy();
#endif

	/* And last, try to fill Rx Buffer Descriptors */
	fecp->fec_r_des_active = 0x01000000;	/* Descriptor polling active	*/

	return 1;
}



static void fec_halt(struct eth_device* dev)
{
#if 0
    volatile immap_t *immr = (immap_t *)CFG_IMMR;
    immr->im_cpm.cp_scc[SCC_ENET].scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);
#endif
}

#if 0
void restart(void)
{
   volatile immap_t *immr = (immap_t *)CFG_IMMR;
   immr->im_cpm.cp_scc[SCC_ENET].scc_gsmrl |= (SCC_GSMRL_ENR | SCC_GSMRL_ENT);
}
#endif

#if defined(CFG_DISCOVER_PHY) || (CONFIG_COMMANDS & CFG_CMD_MII)

static	int	phyaddr = -1;	/* didn't find a PHY yet */
static	uint	phytype;

/* Make MII read/write commands for the FEC.
*/

#define mk_mii_read(ADDR, REG)	(0x60020000 | ((ADDR << 23) | \
						(REG & 0x1f) << 18))

#define mk_mii_write(ADDR, REG, VAL)	(0x50020000 | ((ADDR << 23) | \
						(REG & 0x1f) << 18) | \
						(VAL & 0xffff))

/* Interrupt events/masks.
*/
#define FEC_ENET_HBERR	((uint)0x80000000)	/* Heartbeat error */
#define FEC_ENET_BABR	((uint)0x40000000)	/* Babbling receiver */
#define FEC_ENET_BABT	((uint)0x20000000)	/* Babbling transmitter */
#define FEC_ENET_GRA	((uint)0x10000000)	/* Graceful stop complete */
#define FEC_ENET_TXF	((uint)0x08000000)	/* Full frame transmitted */
#define FEC_ENET_TXB	((uint)0x04000000)	/* A buffer was transmitted */
#define FEC_ENET_RXF	((uint)0x02000000)	/* Full frame received */
#define FEC_ENET_RXB	((uint)0x01000000)	/* A buffer was received */
#define FEC_ENET_MII	((uint)0x00800000)	/* MII interrupt */
#define FEC_ENET_EBERR	((uint)0x00400000)	/* SDMA bus error */

/* PHY identification
 */
#define PHY_ID_LXT970		0x78100000	/* LXT970 */
#define PHY_ID_LXT971		0x001378e0	/* LXT971 and 972 */
#define PHY_ID_82555		0x02a80150	/* Intel 82555 */
#define PHY_ID_QS6612		0x01814400	/* QS6612 */
#define PHY_ID_AMD79C784	0x00225610	/* AMD 79C784 */
#define PHY_ID_LSI80225		0x0016f870	/* LSI 80225 */
#define PHY_ID_LSI80225B	0x0016f880	/* LSI 80225/B */


/* send command to phy using mii, wait for result */
static uint
mii_send(uint mii_cmd)
{
	uint mii_reply;
	volatile fec_t	*ep;

	ep = &(((immap_t *)CFG_IMMR)->im_cpm.cp_fec);

	ep->fec_mii_data = mii_cmd;	/* command to phy */

	/* wait for mii complete */
	while (!(ep->fec_ievent & FEC_ENET_MII))
		;	/* spin until done */
	mii_reply = ep->fec_mii_data;		/* result from phy */
	ep->fec_ievent = FEC_ENET_MII;		/* clear MII complete */
#if 0
	printf("%s[%d] %s: sent=0x%8.8x, reply=0x%8.8x\n",
		__FILE__,__LINE__,__FUNCTION__,mii_cmd,mii_reply);
#endif
	return (mii_reply & 0xffff);		/* data read from phy */
}
#endif /* CFG_DISCOVER_PHY || (CONFIG_COMMANDS & CFG_CMD_MII) */

#if defined(CFG_DISCOVER_PHY)
static void
mii_discover_phy(void)
{
#define MAX_PHY_PASSES 11
	uint phyno;
	int  pass;

	phyaddr = -1;	/* didn't find a PHY yet */
	for (pass = 1; pass <= MAX_PHY_PASSES && phyaddr < 0; ++pass) {
		if (pass > 1) {
			/* PHY may need more time to recover from reset.
			 * The LXT970 needs 50ms typical, no maximum is
			 * specified, so wait 10ms before try again.
			 * With 11 passes this gives it 100ms to wake up.
			 */
			udelay(10000);	/* wait 10ms */
		}
		for (phyno = 0; phyno < 32 && phyaddr < 0; ++phyno) {
			phytype = mii_send(mk_mii_read(phyno, PHY_PHYIDR1));
#ifdef ET_DEBUG
			printf("PHY type 0x%x pass %d type ", phytype, pass);
#endif
			if (phytype != 0xffff) {
				phyaddr = phyno;
				phytype <<= 16;
				phytype |= mii_send(mk_mii_read(phyno,
								PHY_PHYIDR2));

#ifdef ET_DEBUG
				printf("PHY @ 0x%x pass %d type ",phyno,pass);
				switch (phytype & 0xfffffff0) {
				case PHY_ID_LXT970:
					printf("LXT970\n");
					break;
				case PHY_ID_LXT971:
					printf("LXT971\n");
					break;
				case PHY_ID_82555:
					printf("82555\n");
					break;
				case PHY_ID_QS6612:
					printf("QS6612\n");
					break;
				case PHY_ID_AMD79C784:
					printf("AMD79C784\n");
					break;
				case PHY_ID_LSI80225B:
					printf("LSI L80225/B\n");
					break;
				default:
					printf("0x%08x\n", phytype);
					break;
				}
#endif
			}
		}
	}
	if (phyaddr < 0) {
		printf("No PHY device found.\n");
	}
}
#endif	/* CFG_DISCOVER_PHY */

#if (CONFIG_COMMANDS & CFG_CMD_MII) && !defined(CONFIG_BITBANGMII)

static int mii_init_done = 0;

/****************************************************************************
 * mii_init -- Initialize the MII for MII command without ethernet
 * This function is a subset of eth_init
 ****************************************************************************
 */
void mii_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile fec_t *fecp = &(immr->im_cpm.cp_fec);
	int i;

	if (mii_init_done != 0) {
		return;
	}

	/* Whack a reset.
	 * A delay is required between a reset of the FEC block and
	 * initialization of other FEC registers because the reset takes
	 * some time to complete. If you don't delay, subsequent writes
	 * to FEC registers might get killed by the reset routine which is
	 * still in progress.
	 */

	fecp->fec_ecntrl = FEC_ECNTRL_PINMUX | FEC_ECNTRL_RESET;
	for (i = 0;
	     (fecp->fec_ecntrl & FEC_ECNTRL_RESET) && (i < FEC_RESET_DELAY);
	     ++i) {
		udelay (1);
	}
	if (i == FEC_RESET_DELAY) {
		printf ("FEC_RESET_DELAY timeout\n");
		return;
	}

	/* We use strictly polling mode only
	 */
	fecp->fec_imask = 0;

	/* Clear any pending interrupt
	 */
	fecp->fec_ievent = 0xffc0;

	/* Set MII speed to 2.5 MHz or slightly below.
	 * According to the MPC860T (Rev. D) Fast ethernet controller user
	 * manual (6.2.14),
	 * the MII management interface clock must be less than or equal
	 * to 2.5 MHz.
	 * This MDC frequency is equal to system clock / (2 * MII_SPEED).
	 * Then MII_SPEED = system_clock / 2 * 2,5 Mhz.
	 */
	fecp->fec_mii_speed = ((bd->bi_busfreq + 4999999) / 5000000) << 1;

#if !defined(CONFIG_ICU862) && !defined(CONFIG_IAD210)
	/* Configure all of port D for MII.
	 */
	immr->im_ioport.iop_pdpar = 0x1fff;

	/* Bits moved from Rev. D onward */
	if ((get_immr (0) & 0xffff) < 0x0501) {
		immr->im_ioport.iop_pddir = 0x1c58;	/* Pre rev. D */
	} else {
		immr->im_ioport.iop_pddir = 0x1fff;	/* Rev. D and later */
	}
#else
	/* Configure port A for MII.
	*/

#if defined(CONFIG_ICU862)

	/* On the ICU862 board the MII-MDC pin is routed to PD8 pin
	 * of CPU, so for this board we need to configure Utopia and
	 * enable PD8 to MII-MDC function */
	immr->im_ioport.iop_pdpar |= 0x4080;
#endif

	/* Has Utopia been configured? */
	if (immr->im_ioport.iop_pdpar & (0x8000 >> 1)) {
		/*
		 * YES - Use MUXED mode for UTOPIA bus.
		 * This frees Port A for use by MII (see 862UM table 41-6).
		 */
		immr->im_ioport.utmode &= ~0x80;
	} else {
		/*
		 * NO - set SPLIT mode for UTOPIA bus.
		 *
		 * This doesn't really effect UTOPIA (which isn't
		 * enabled anyway) but just tells the 862
		 * to use port A for MII (see 862UM table 41-6).
		 */
		immr->im_ioport.utmode |= 0x80;
	}
#endif	/* !defined(CONFIG_ICU862) */
	/* Now enable the transmit and receive processing
	 */
	fecp->fec_ecntrl = FEC_ECNTRL_PINMUX | FEC_ECNTRL_ETHER_EN;

	mii_init_done = 1;
}
/*****************************************************************************
 * Read and write a MII PHY register, routines used by MII Utilities
 *
 * FIXME: These routines are expected to return 0 on success, but mii_send
 *	  does _not_ return an error code. Maybe 0xFFFF means error, i.e.
 *	  no PHY connected...
 *	  For now always return 0.
 * FIXME: These routines only work after calling eth_init() at least once!
 *	  Otherwise they hang in mii_send() !!! Sorry!
 *****************************************************************************/

int miiphy_read(unsigned char addr, unsigned char  reg, unsigned short *value)
{
	short rdreg;    /* register working value */

#ifdef MII_DEBUG
	printf ("miiphy_read(0x%x) @ 0x%x = ", reg, addr);
#endif
	rdreg = mii_send(mk_mii_read(addr, reg));

	*value = rdreg;

#ifdef MII_DEBUG
	printf ("0x%04x\n", *value);
#endif

	return 0;
}

int miiphy_write(unsigned char  addr, unsigned char  reg, unsigned short value)
{
	short rdreg;    /* register working value */

#ifdef MII_DEBUG
	printf ("miiphy_write(0x%x) @ 0x%x = ", reg, addr);
#endif

	rdreg = mii_send(mk_mii_write(addr, reg, value));

#ifdef MII_DEBUG
	printf ("0x%04x\n", value);
#endif

	return 0;
}
#endif /* (CONFIG_COMMANDS & CFG_CMD_MII) && !defined(CONFIG_BITBANGMII)*/

#endif	/* CFG_CMD_NET, FEC_ENET */

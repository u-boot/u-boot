/*-----------------------------------------------------------------------------+
 *
 *       This source code has been made available to you by IBM on an AS-IS
 *       basis.  Anyone receiving this source is licensed under IBM
 *       copyrights to use it in any way he or she deems fit, including
 *       copying it, modifying it, compiling it, and redistributing it either
 *       with or without modifications.  No license under IBM patents or
 *       patent applications is to be implied by the copyright license.
 *
 *       Any user of this software should understand that IBM cannot provide
 *       technical support for this software and will not be responsible for
 *       any consequences resulting from the use of this software.
 *
 *       Any person who transfers this source code or any derivative work
 *       must include the IBM copyright notice, this paragraph, and the
 *       preceding two paragraphs in the transferred software.
 *
 *       COPYRIGHT   I B M   CORPORATION 1995
 *       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 *-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------+
 *
 *  File Name:  enetemac.c
 *
 *  Function:   Device driver for the ethernet EMAC3 macro on the 405GP.
 *
 *  Author:     Mark Wisner
 *
 *  Change Activity-
 *
 *  Date        Description of Change                                       BY
 *  ---------   ---------------------                                       ---
 *  05-May-99   Created                                                     MKW
 *  27-Jun-99   Clean up                                                    JWB
 *  16-Jul-99   Added MAL error recovery and better IP packet handling      MKW
 *  29-Jul-99   Added Full duplex support                                   MKW
 *  06-Aug-99   Changed names for Mal CR reg                                MKW
 *  23-Aug-99   Turned off SYE when running at 10Mbs                        MKW
 *  24-Aug-99   Marked descriptor empty after call_xlc                      MKW
 *  07-Sep-99   Set MAL RX buffer size reg to ENET_MAX_MTU_ALIGNED / 16     MCG
 *              to avoid chaining maximum sized packets. Push starting
 *              RX descriptor address up to the next cache line boundary.
 *  16-Jan-00   Added support for booting with IP of 0x0                    MKW
 *  15-Mar-00   Updated enetInit() to enable broadcast addresses in the
 *	        EMAC_RXM register.                                          JWB
 *  12-Mar-01   anne-sophie.harnois@nextream.fr
 *               - Variables are compatible with those already defined in
 *                include/net.h
 *              - Receive buffer descriptor ring is used to send buffers
 *                to the user
 *              - Info print about send/received/handled packet number if
 *                INFO_405_ENET is set
 *  17-Apr-01   stefan.roese@esd-electronics.com
 *              - MAL reset in "eth_halt" included
 *              - Enet speed and duplex output now in one line
 *  08-May-01   stefan.roese@esd-electronics.com
 *              - MAL error handling added (eth_init called again)
 *  13-Nov-01   stefan.roese@esd-electronics.com
 *              - Set IST bit in EMAC_M1 reg upon 100MBit or full duplex
 *  04-Jan-02   stefan.roese@esd-electronics.com
 *              - Wait for PHY auto negotiation to complete added
 *  06-Feb-02   stefan.roese@esd-electronics.com
 *              - Bug fixed in waiting for auto negotiation to complete
 *  26-Feb-02   stefan.roese@esd-electronics.com
 *              - rx and tx buffer descriptors now allocated (no fixed address
 *                used anymore)
 *  17-Jun-02   stefan.roese@esd-electronics.com
 *              - MAL error debug printf 'M' removed (rx de interrupt may
 *                occur upon many incoming packets with only 4 rx buffers).
 *-----------------------------------------------------------------------------*/

#include <common.h>
#include <asm/processor.h>
#include <ppc4xx.h>
#include <commproc.h>
#include <405gp_enet.h>
#include <405_mal.h>
#include <miiphy.h>
#include <net.h>
#include <malloc.h>
#include "vecnum.h"

#if defined(CONFIG_405GP) || defined(CONFIG_440) || defined(CONFIG_405EP)

#define EMAC_RESET_TIMEOUT 1000	/* 1000 ms reset timeout */
#define PHY_AUTONEGOTIATE_TIMEOUT 2000	/* 2000 ms autonegotiate timeout */

#define NUM_TX_BUFF 1
/* AS.HARNOIS
 * Use PKTBUFSRX (include/net.h) instead of setting NUM_RX_BUFF again
 * These both variables are used to define the same thing!
 * #define NUM_RX_BUFF 4
 */
#define NUM_RX_BUFF PKTBUFSRX

/* Ethernet Transmit and Receive Buffers */
/* AS.HARNOIS
 * In the same way ENET_MAX_MTU and ENET_MAX_MTU_ALIGNED are set from
 * PKTSIZE and PKTSIZE_ALIGN (include/net.h)
 */
#define ENET_MAX_MTU           PKTSIZE
#define ENET_MAX_MTU_ALIGNED   PKTSIZE_ALIGN

static char *txbuf_ptr;

/* define the number of channels implemented */
#define EMAC_RXCHL      1
#define EMAC_TXCHL      1

/*-----------------------------------------------------------------------------+
 * Defines for MAL/EMAC interrupt conditions as reported in the UIC (Universal
 * Interrupt Controller).
 *-----------------------------------------------------------------------------*/
#define MAL_UIC_ERR ( UIC_MAL_SERR | UIC_MAL_TXDE  | UIC_MAL_RXDE)
#define MAL_UIC_DEF  (UIC_MAL_RXEOB | MAL_UIC_ERR)
#define EMAC_UIC_DEF UIC_ENET

/*-----------------------------------------------------------------------------+
 * Global variables. TX and RX descriptors and buffers.
 *-----------------------------------------------------------------------------*/
static volatile mal_desc_t *tx;
static volatile mal_desc_t *rx;
static mal_desc_t *alloc_tx_buf = NULL;
static mal_desc_t *alloc_rx_buf = NULL;

/* IER globals */
static unsigned long emac_ier;
static unsigned long mal_ier;


/* Statistic Areas */
#define MAX_ERR_LOG 10
struct emac_stats {
	int data_len_err;
	int rx_frames;
	int rx;
	int rx_prot_err;
};

static struct stats {			/* Statistic Block */
	struct emac_stats emac;
	int int_err;
	short tx_err_log[MAX_ERR_LOG];
	short rx_err_log[MAX_ERR_LOG];
} stats;

static int first_init = 0;

static int tx_err_index = 0;	/* Transmit Error Index for tx_err_log */
static int rx_err_index = 0;	/* Receive Error Index for rx_err_log */

static int rx_slot = 0;			/* MAL Receive Slot */
static int rx_i_index = 0;		/* Receive Interrupt Queue Index */
static int rx_u_index = 0;		/* Receive User Queue Index */
static int rx_ready[NUM_RX_BUFF];	/* Receive Ready Queue */

static int tx_slot = 0;			/* MAL Transmit Slot */
static int tx_i_index = 0;		/* Transmit Interrupt Queue Index */
static int tx_u_index = 0;		/* Transmit User Queue Index */
static int tx_run[NUM_TX_BUFF];	/* Transmit Running Queue */

#undef INFO_405_ENET 1
#ifdef INFO_405_ENET
static int packetSent = 0;
static int packetReceived = 0;
static int packetHandled = 0;
#endif

static char emac_hwd_addr[ENET_ADDR_LENGTH];

static bd_t *bis_save = NULL;	/* for eth_init upon mal error */

static int is_receiving = 0;	/* sync with eth interrupt */
static int print_speed = 1;	/* print speed message upon start */

/*-----------------------------------------------------------------------------+
 * Prototypes and externals.
 *-----------------------------------------------------------------------------*/
static void enet_rcv (unsigned long malisr);
static int  enetInt(void);
static void mal_err (unsigned long isr, unsigned long uic, unsigned long mal_def,
	      unsigned long mal_errr);
static void emac_err (unsigned long isr);

static void ppc_4xx_eth_halt (struct eth_device *dev)
{
	mtdcr (malier, 0x00000000);	/* disable mal interrupts */
	out32 (EMAC_IER, 0x00000000);	/* disable emac interrupts */

	/* 1st reset MAL */
	mtdcr (malmcr, MAL_CR_MMSR);

	/* wait for reset */
	while (mfdcr (malmcr) & MAL_CR_MMSR) {
	};

	/* EMAC RESET */
	out32 (EMAC_M0, EMAC_M0_SRST);

	print_speed = 1;		/* print speed message again next time */
}


static int ppc_4xx_eth_init (struct eth_device *dev, bd_t * bis)
{
	int i;
	unsigned long reg;
	unsigned long msr;
	unsigned long speed;
	unsigned long duplex;
	unsigned mode_reg;
	unsigned short reg_short;

	msr = mfmsr ();
	mtmsr (msr & ~(MSR_EE));	/* disable interrupts */

#ifdef INFO_405_ENET
	/* AS.HARNOIS
	 * We should have :
	 * packetHandled <=  packetReceived <= packetHandled+PKTBUFSRX
         * In the most cases packetHandled = packetReceived, but it
         * is possible that new packets (without relationship with
         * current transfer) have got the time to arrived before
         * netloop calls eth_halt
	 */
	printf ("About preceeding transfer:\n"
		"- Sent packet number %d\n"
		"- Received packet number %d\n"
		"- Handled packet number %d\n",
		packetSent, packetReceived, packetHandled);
	packetSent = 0;
	packetReceived = 0;
	packetHandled = 0;
#endif

	/* MAL RESET */
	mtdcr (malmcr, MAL_CR_MMSR);
	/* wait for reset */
	while (mfdcr (malmcr) & MAL_CR_MMSR) {
	};

	tx_err_index = 0;		/* Transmit Error Index for tx_err_log */
	rx_err_index = 0;		/* Receive Error Index for rx_err_log */

	rx_slot = 0;			/* MAL Receive Slot */
	rx_i_index = 0;			/* Receive Interrupt Queue Index */
	rx_u_index = 0;			/* Receive User Queue Index */

	tx_slot = 0;			/* MAL Transmit Slot */
	tx_i_index = 0;			/* Transmit Interrupt Queue Index */
	tx_u_index = 0;			/* Transmit User Queue Index */

#if defined(CONFIG_440)
        /* set RMII mode */
        out32 (ZMII_FER, ZMII_RMII | ZMII_MDI0);
#endif /* CONFIG_440 */

	/* EMAC RESET */
	out32 (EMAC_M0, EMAC_M0_SRST);

	/* wait for PHY to complete auto negotiation */
	reg_short = 0;
#ifndef CONFIG_CS8952_PHY
	miiphy_read (CONFIG_PHY_ADDR, PHY_BMSR, &reg_short);

	/*
	 * Wait if PHY is able of autonegotiation and autonegotiation is not complete
	 */
	if ((reg_short & PHY_BMSR_AUTN_ABLE)
	    && !(reg_short & PHY_BMSR_AUTN_COMP)) {
		puts ("Waiting for PHY auto negotiation to complete");
		i = 0;
		while (!(reg_short & PHY_BMSR_AUTN_COMP)) {
			if ((i++ % 100) == 0)
				putc ('.');
			udelay (10000);		/* 10 ms */
			miiphy_read (CONFIG_PHY_ADDR, PHY_BMSR, &reg_short);

			/*
			 * Timeout reached ?
			 */
			if (i * 10 > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts (" TIMEOUT !\n");
				break;
			}
		}
		puts (" done\n");
		udelay (500000);	/* another 500 ms (results in faster booting) */
	}
#endif
	speed = miiphy_speed (CONFIG_PHY_ADDR);
	duplex = miiphy_duplex (CONFIG_PHY_ADDR);
	if (print_speed) {
		print_speed = 0;
		printf ("ENET Speed is %d Mbps - %s duplex connection\n",
			(int) speed, (duplex == HALF) ? "HALF" : "FULL");
	}

	/* set the Mal configuration reg */
#if defined(CONFIG_440)
	/* Errata 1.12: MAL_1 -- Disable MAL bursting */
	if( get_pvr() == PVR_440GP_RB )
	    mtdcr (malmcr, MAL_CR_OPBBL | MAL_CR_LEA | MAL_CR_PLBLT_DEFAULT);
	else
#else
	mtdcr (malmcr, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA | MAL_CR_PLBLT_DEFAULT);
#endif

	/* Free "old" buffers */
	if (alloc_tx_buf) free(alloc_tx_buf);
	if (alloc_rx_buf) free(alloc_rx_buf);

	/*
	 * Malloc MAL buffer desciptors, make sure they are
	 * aligned on cache line boundary size
	 * (401/403/IOP480 = 16, 405 = 32)
	 * and doesn't cross cache block boundaries.
	 */
	alloc_tx_buf = (mal_desc_t *)malloc((sizeof(mal_desc_t) * NUM_TX_BUFF) +
					    ((2 * CFG_CACHELINE_SIZE) - 2));
	if (((int)alloc_tx_buf & CACHELINE_MASK) != 0) {
		tx = (mal_desc_t *)((int)alloc_tx_buf + CFG_CACHELINE_SIZE -
				    ((int)alloc_tx_buf & CACHELINE_MASK));
	} else {
		tx = alloc_tx_buf;
	}

	alloc_rx_buf = (mal_desc_t *)malloc((sizeof(mal_desc_t) * NUM_RX_BUFF) +
					    ((2 * CFG_CACHELINE_SIZE) - 2));
	if (((int)alloc_rx_buf & CACHELINE_MASK) != 0) {
		rx = (mal_desc_t *)((int)alloc_rx_buf + CFG_CACHELINE_SIZE -
				    ((int)alloc_rx_buf & CACHELINE_MASK));
	} else {
		rx = alloc_rx_buf;
	}

	for (i = 0; i < NUM_TX_BUFF; i++) {
		tx[i].ctrl = 0;
		tx[i].data_len = 0;
		if (first_init == 0)
			txbuf_ptr = (char *) malloc (ENET_MAX_MTU_ALIGNED);
		tx[i].data_ptr = txbuf_ptr;
		if ((NUM_TX_BUFF - 1) == i)
			tx[i].ctrl |= MAL_TX_CTRL_WRAP;
		tx_run[i] = -1;
#if 0
		printf ("TX_BUFF %d @ 0x%08lx\n", i, (ulong) tx[i].data_ptr);
#endif
	}

	for (i = 0; i < NUM_RX_BUFF; i++) {
		rx[i].ctrl = 0;
		rx[i].data_len = 0;
		/*       rx[i].data_ptr = (char *) &rx_buff[i]; */
		rx[i].data_ptr = (char *) NetRxPackets[i];
		if ((NUM_RX_BUFF - 1) == i)
			rx[i].ctrl |= MAL_RX_CTRL_WRAP;
		rx[i].ctrl |= MAL_RX_CTRL_EMPTY | MAL_RX_CTRL_INTR;
		rx_ready[i] = -1;
#if 0
		printf ("RX_BUFF %d @ 0x%08lx\n", i, (ulong) rx[i].data_ptr);
#endif
	}

	memcpy (emac_hwd_addr, bis->bi_enetaddr, ENET_ADDR_LENGTH);

	reg = 0x00000000;

	reg |= emac_hwd_addr[0];	/* set high address */
	reg = reg << 8;
	reg |= emac_hwd_addr[1];

	out32 (EMAC_IAH, reg);

	reg = 0x00000000;
	reg |= emac_hwd_addr[2];	/* set low address  */
	reg = reg << 8;
	reg |= emac_hwd_addr[3];
	reg = reg << 8;
	reg |= emac_hwd_addr[4];
	reg = reg << 8;
	reg |= emac_hwd_addr[5];

	out32 (EMAC_IAL, reg);

	/* setup MAL tx & rx channel pointers */
	mtdcr (maltxctp0r, tx);
	mtdcr (malrxctp0r, rx);

	/* Reset transmit and receive channels */
	mtdcr (malrxcarr, 0x80000000);	/* 2 channels */
	mtdcr (maltxcarr, 0x80000000);	/* 2 channels */

	/* Enable MAL transmit and receive channels */
	mtdcr (maltxcasr, 0x80000000);	/* 1 channel */
	mtdcr (malrxcasr, 0x80000000);	/* 1 channel */

	/* set RX buffer size */
	mtdcr (malrcbs0, ENET_MAX_MTU_ALIGNED / 16);

	/* set transmit enable & receive enable */
	out32 (EMAC_M0, EMAC_M0_TXE | EMAC_M0_RXE);

	/* set receive fifo to 4k and tx fifo to 2k */
	mode_reg = EMAC_M1_RFS_4K | EMAC_M1_TX_FIFO_2K;

	/* set speed */
	if (speed == _100BASET)
		mode_reg = mode_reg | EMAC_M1_MF_100MBPS | EMAC_M1_IST;
	else
		mode_reg = mode_reg & ~0x00C00000;	/* 10 MBPS */
	if (duplex == FULL)
		mode_reg = mode_reg | 0x80000000 | EMAC_M1_IST;

	out32 (EMAC_M1, mode_reg);

	/* Enable broadcast and indvidual address */
	out32 (EMAC_RXM, EMAC_RMR_BAE | EMAC_RMR_IAE
	       /*| EMAC_RMR_ARRP| EMAC_RMR_SFCS | EMAC_RMR_SP */ );

	/* we probably need to set the tx mode1 reg? maybe at tx time */

	/* set transmit request threshold register */
	out32 (EMAC_TRTR, 0x18000000);	/* 256 byte threshold */

	/* set receive  low/high water mark register */
#if defined(CONFIG_440)
	/* 440GP has a 64 byte burst length */
        out32 (EMAC_RX_HI_LO_WMARK, 0x80009000);
        out32 (EMAC_TXM1,           0xf8640000);
#else /* CONFIG_440 */
	/* 405s have a 16 byte burst length */
	out32 (EMAC_RX_HI_LO_WMARK, 0x0f002000);
#endif /* CONFIG_440 */

	/* Frame gap set */
	out32 (EMAC_I_FRAME_GAP_REG, 0x00000008);

	if (first_init == 0) {
		/*
		 * Connect interrupt service routines
		 */
		irq_install_handler (VECNUM_EWU0, (interrupt_handler_t *) enetInt, NULL);
		irq_install_handler (VECNUM_MS, (interrupt_handler_t *) enetInt, NULL);
		irq_install_handler (VECNUM_MTE, (interrupt_handler_t *) enetInt, NULL);
		irq_install_handler (VECNUM_MRE, (interrupt_handler_t *) enetInt, NULL);
		irq_install_handler (VECNUM_TXDE, (interrupt_handler_t *) enetInt, NULL);
		irq_install_handler (VECNUM_RXDE, (interrupt_handler_t *) enetInt, NULL);
		irq_install_handler (VECNUM_ETH0, (interrupt_handler_t *) enetInt, NULL);
	}

	/* set up interrupt handler */
	/* setup interrupt controler to take interrupts from the MAL &
	   EMAC */
	mtdcr (uicsr, 0xffffffff);	/* clear pending interrupts */
	mtdcr (uicer, mfdcr (uicer) | MAL_UIC_DEF | EMAC_UIC_DEF);

	/* set the MAL IER ??? names may change with new spec ??? */
	mal_ier = MAL_IER_DE | MAL_IER_NE | MAL_IER_TE | MAL_IER_OPBE |
		MAL_IER_PLBE;
	mtdcr (malesr, 0xffffffff);	/* clear pending interrupts */
	mtdcr (maltxdeir, 0xffffffff);	/* clear pending interrupts */
	mtdcr (malrxdeir, 0xffffffff);	/* clear pending interrupts */
	mtdcr (malier, mal_ier);

	/* Set EMAC IER */
	emac_ier = EMAC_ISR_PTLE | EMAC_ISR_BFCS |
		EMAC_ISR_PTLE | EMAC_ISR_ORE  | EMAC_ISR_IRE;
	if (speed == _100BASET)
		emac_ier = emac_ier | EMAC_ISR_SYE;

	out32 (EMAC_ISR, 0xffffffff);	/* clear pending interrupts */
	out32 (EMAC_IER, emac_ier);

	mtmsr (msr);				/* enable interrupts again */

	bis_save = bis;
	first_init = 1;

	return (1);
}


static int ppc_4xx_eth_send (struct eth_device *dev, volatile void *ptr, int len)
{
	struct enet_frame *ef_ptr;
	ulong time_start, time_now;
	unsigned long temp_txm0;

	ef_ptr = (struct enet_frame *) ptr;

	/*-----------------------------------------------------------------------+
	 *  Copy in our address into the frame.
	 *-----------------------------------------------------------------------*/
	(void) memcpy (ef_ptr->source_addr, emac_hwd_addr, ENET_ADDR_LENGTH);

	/*-----------------------------------------------------------------------+
	 * If frame is too long or too short, modify length.
	 *-----------------------------------------------------------------------*/
	if (len > ENET_MAX_MTU)
		len = ENET_MAX_MTU;

	/*   memcpy ((void *) &tx_buff[tx_slot], (const void *) ptr, len); */
	memcpy ((void *) txbuf_ptr, (const void *) ptr, len);

	/*-----------------------------------------------------------------------+
	 * set TX Buffer busy, and send it
	 *-----------------------------------------------------------------------*/
	tx[tx_slot].ctrl = (MAL_TX_CTRL_LAST |
			    EMAC_TX_CTRL_GFCS | EMAC_TX_CTRL_GP) &
		~(EMAC_TX_CTRL_ISA | EMAC_TX_CTRL_RSA);
	if ((NUM_TX_BUFF - 1) == tx_slot)
		tx[tx_slot].ctrl |= MAL_TX_CTRL_WRAP;

	tx[tx_slot].data_len = (short) len;
	tx[tx_slot].ctrl |= MAL_TX_CTRL_READY;

    __asm__ volatile ("eieio");
	out32 (EMAC_TXM0, in32 (EMAC_TXM0) | EMAC_TXM0_GNP0);
#ifdef INFO_405_ENET
	packetSent++;
#endif

	/*-----------------------------------------------------------------------+
	 * poll unitl the packet is sent and then make sure it is OK
	 *-----------------------------------------------------------------------*/
	time_start = get_timer (0);
	while (1) {
		temp_txm0 = in32 (EMAC_TXM0);
		/* loop until either TINT turns on or 3 seconds elapse */
		if ((temp_txm0 & EMAC_TXM0_GNP0) != 0) {
			/* transmit is done, so now check for errors
                         * If there is an error, an interrupt should
                         * happen when we return
			 */
			time_now = get_timer (0);
			if ((time_now - time_start) > 3000) {
				return (-1);
			}
		} else {
			return (len);
		}
	}
}


#if defined(CONFIG_440)
/*-----------------------------------------------------------------------------+
| EnetInt.
| EnetInt is the interrupt handler.  It will determine the
| cause of the interrupt and call the apporpriate servive
| routine.
+-----------------------------------------------------------------------------*/
int enetInt ()
{
	int serviced;
	int rc = -1;				/* default to not us */
	unsigned long mal_isr;
	unsigned long emac_isr = 0;
	unsigned long mal_rx_eob;
	unsigned long my_uic0msr, my_uic1msr;

	/* enter loop that stays in interrupt code until nothing to service */
	do {
		serviced = 0;

		my_uic0msr = mfdcr (uic0msr);
		my_uic1msr = mfdcr (uic1msr);

		if (!(my_uic0msr & UIC_MRE)
                    && !(my_uic1msr & (UIC_ETH0 | UIC_MS | UIC_MTDE | UIC_MRDE))) {
                        /* not for us */
			return (rc);
		}

		/* get and clear controller status interrupts */
		/* look at Mal and EMAC interrupts */
		if ((my_uic0msr & UIC_MRE)
                    || (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE))) {
                        /* we have a MAL interrupt */
			mal_isr = mfdcr (malesr);
			/* look for mal error */
			if (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE)) {
				mal_err (mal_isr, my_uic0msr, MAL_UIC_DEF, MAL_UIC_ERR);
				serviced = 1;
				rc = 0;
			}
		}
		if (UIC_ETH0 & my_uic1msr) {	/* look for EMAC errors */
			emac_isr = in32 (EMAC_ISR);
			if ((emac_ier & emac_isr) != 0) {
				emac_err (emac_isr);
				serviced = 1;
				rc = 0;
			}
		}
		if ((emac_ier & emac_isr)
                    || (my_uic1msr & (UIC_MS | UIC_MTDE | UIC_MRDE))) {
			mtdcr (uic0sr, UIC_MRE); /* Clear */
			mtdcr (uic1sr, UIC_ETH0 | UIC_MS | UIC_MTDE | UIC_MRDE); /* Clear */
			return (rc);		/* we had errors so get out */
		}

		/* handle MAL RX EOB  interupt from a receive */
		/* check for EOB on valid channels            */
		if (my_uic0msr & UIC_MRE) {
			mal_rx_eob = mfdcr (malrxeobisr);
			if ((mal_rx_eob & 0x80000000) != 0) {	/* call emac routine for channel 0 */
				/* clear EOB
				   mtdcr(malrxeobisr, mal_rx_eob); */
				enet_rcv (emac_isr);
				/* indicate that we serviced an interrupt */
				serviced = 1;
				rc = 0;
			}
		}
                mtdcr (uic0sr, UIC_MRE); /* Clear */
                mtdcr (uic1sr, UIC_ETH0 | UIC_MS | UIC_MTDE | UIC_MRDE); /* Clear */
	} while (serviced);

	return (rc);
}
#else /* CONFIG_440 */
/*-----------------------------------------------------------------------------+
 * EnetInt.
 * EnetInt is the interrupt handler.  It will determine the
 * cause of the interrupt and call the apporpriate servive
 * routine.
 *-----------------------------------------------------------------------------*/
int enetInt ()
{
	int serviced;
	int rc = -1;				/* default to not us */
	unsigned long mal_isr;
	unsigned long emac_isr = 0;
	unsigned long mal_rx_eob;
	unsigned long my_uicmsr;

	/* enter loop that stays in interrupt code until nothing to service */
	do {
		serviced = 0;

		my_uicmsr = mfdcr (uicmsr);
		if ((my_uicmsr & (MAL_UIC_DEF | EMAC_UIC_DEF)) == 0) {	/* not for us */
			return (rc);
		}


		/* get and clear controller status interrupts */
		/* look at Mal and EMAC interrupts */
		if ((MAL_UIC_DEF & my_uicmsr) != 0) {	/* we have a MAL interrupt */
			mal_isr = mfdcr (malesr);
			/* look for mal error */
			if ((my_uicmsr & MAL_UIC_ERR) != 0) {
				mal_err (mal_isr, my_uicmsr, MAL_UIC_DEF, MAL_UIC_ERR);
				serviced = 1;
				rc = 0;
			}
		}
		if ((EMAC_UIC_DEF & my_uicmsr) != 0) {	/* look for EMAC errors */
			emac_isr = in32 (EMAC_ISR);
			if ((emac_ier & emac_isr) != 0) {
				emac_err (emac_isr);
				serviced = 1;
				rc = 0;
			}
		}
		if (((emac_ier & emac_isr) != 0) | ((MAL_UIC_ERR & my_uicmsr) != 0)) {
			mtdcr (uicsr, MAL_UIC_DEF | EMAC_UIC_DEF); /* Clear */
			return (rc);		/* we had errors so get out */
		}


		/* handle MAL RX EOB  interupt from a receive */
		/* check for EOB on valid channels            */
		if ((my_uicmsr & UIC_MAL_RXEOB) != 0) {
			mal_rx_eob = mfdcr (malrxeobisr);
			if ((mal_rx_eob & 0x80000000) != 0) {	/* call emac routine for channel 0 */
				/* clear EOB
				   mtdcr(malrxeobisr, mal_rx_eob); */
				enet_rcv (emac_isr);
				/* indicate that we serviced an interrupt */
				serviced = 1;
				rc = 0;
			}
		}
		mtdcr (uicsr, MAL_UIC_DEF | EMAC_UIC_DEF);	/* Clear */
	}
	while (serviced);

	return (rc);
}
#endif /* CONFIG_440 */

/*-----------------------------------------------------------------------------+
 *  MAL Error Routine
 *-----------------------------------------------------------------------------*/
static void mal_err (unsigned long isr, unsigned long uic, unsigned long maldef,
	      unsigned long mal_errr)
{
	mtdcr (malesr, isr);		/* clear interrupt */

	/* clear DE interrupt */
	mtdcr (maltxdeir, 0xC0000000);
	mtdcr (malrxdeir, 0x80000000);

#ifdef INFO_405_ENET
	printf ("\nMAL error occured.... ISR = %lx UIC = = %lx  MAL_DEF = %lx  MAL_ERR= %lx \n",
		isr, uic, maldef, mal_errr);
#else
#if 0
	/*
	 * MAL error is RX DE error (out of rx buffers)! This is OK here, upon
	 * many incoming packets with only 4 rx buffers.
	 */
	printf ("M");			/* just to see something upon mal error */
#endif
#endif

	eth_init (bis_save);		/* start again... */
}

/*-----------------------------------------------------------------------------+
 *  EMAC Error Routine
 *-----------------------------------------------------------------------------*/
static void emac_err (unsigned long isr)
{
	printf ("EMAC error occured.... ISR = %lx\n", isr);
	out32 (EMAC_ISR, isr);
}

/*-----------------------------------------------------------------------------+
 *  enet_rcv() handles the ethernet receive data
 *-----------------------------------------------------------------------------*/
static void enet_rcv (unsigned long malisr)
{
	struct enet_frame *ef_ptr;
	unsigned long data_len;
	unsigned long rx_eob_isr;

	int handled = 0;
	int i;
	int loop_count = 0;

	rx_eob_isr = mfdcr (malrxeobisr);
	if ((0x80000000 >> (EMAC_RXCHL - 1)) & rx_eob_isr) {
		/* clear EOB */
		mtdcr (malrxeobisr, rx_eob_isr);

		/* EMAC RX done */
		while (1) {				/* do all */
			i = rx_slot;

			if ((MAL_RX_CTRL_EMPTY & rx[i].ctrl)
			    || (loop_count >= NUM_RX_BUFF))
				break;
			loop_count++;
			rx_slot++;
			if (NUM_RX_BUFF == rx_slot)
				rx_slot = 0;
			handled++;
			data_len = (unsigned long) rx[i].data_len;	/* Get len */
			if (data_len) {
				if (data_len > ENET_MAX_MTU)	/* Check len */
					data_len = 0;
				else {
					if (EMAC_RX_ERRORS & rx[i].ctrl) {	/* Check Errors */
						data_len = 0;
						stats.rx_err_log[rx_err_index] = rx[i].ctrl;
						rx_err_index++;
						if (rx_err_index == MAX_ERR_LOG)
							rx_err_index = 0;
					}	/* emac_erros         */
				}		/* data_len < max mtu */
			}			/* if data_len        */
			if (!data_len) {	/* no data */
				rx[i].ctrl |= MAL_RX_CTRL_EMPTY;	/* Free Recv Buffer */

				stats.emac.data_len_err++;	/* Error at Rx */
			}

			/* !data_len */
			/* AS.HARNOIS */
			/* Check if user has already eaten buffer */
			/* if not => ERROR */
			else if (rx_ready[rx_i_index] != -1) {
				if (is_receiving)
					printf ("ERROR : Receive buffers are full!\n");
				break;
			} else {
				stats.emac.rx_frames++;
				stats.emac.rx += data_len;
				ef_ptr = (struct enet_frame *) rx[i].data_ptr;
#ifdef INFO_405_ENET
				packetReceived++;
#endif
				/* AS.HARNOIS
				 * use ring buffer
				 */
				rx_ready[rx_i_index] = i;
				rx_i_index++;
				if (NUM_RX_BUFF == rx_i_index)
					rx_i_index = 0;

				/* printf("X");  /|* test-only *|/ */

				/*  AS.HARNOIS
				 * free receive buffer only when
				 * buffer has been handled (eth_rx)
				 rx[i].ctrl |= MAL_RX_CTRL_EMPTY;
				*/
			}			/* if data_len */
		}				/* while */
	}					/* if EMACK_RXCHL */
}


static int ppc_4xx_eth_rx (struct eth_device *dev)
{
	int length;
	int user_index;
	unsigned long msr;

	is_receiving = 1;			/* tell driver */

	for (;;) {
		/* AS.HARNOIS
		 * use ring buffer and
		 * get index from rx buffer desciptor queue
		 */
		user_index = rx_ready[rx_u_index];
		if (user_index == -1) {
			length = -1;
			break;	/* nothing received - leave for() loop */
		}

		msr = mfmsr ();
		mtmsr (msr & ~(MSR_EE));

		length = rx[user_index].data_len;

		/* Pass the packet up to the protocol layers. */
		/*       NetReceive(NetRxPackets[rxIdx], length - 4); */
		/*       NetReceive(NetRxPackets[i], length); */
		NetReceive (NetRxPackets[user_index], length - 4);
		/* Free Recv Buffer */
		rx[user_index].ctrl |= MAL_RX_CTRL_EMPTY;
		/* Free rx buffer descriptor queue */
		rx_ready[rx_u_index] = -1;
		rx_u_index++;
		if (NUM_RX_BUFF == rx_u_index)
			rx_u_index = 0;

#ifdef INFO_405_ENET
		packetHandled++;
#endif

		mtmsr (msr);			/* Enable IRQ's */
	}

	is_receiving = 0;			/* tell driver */

	return length;
}

#if defined(CONFIG_NET_MULTI)
int ppc_4xx_eth_initialize(bd_t *bis)
{
        struct eth_device *dev;
        int                eth_num = 0;

        dev = malloc (sizeof *dev);
        if (dev == NULL) {
                printf(__FUNCTION__ ": Cannot allocate eth_device\n");
                return (-1);
        }

        sprintf(dev->name, "ppc_4xx_eth%d", eth_num);
        dev->priv = (void *) eth_num;
        dev->init = ppc_4xx_eth_init;
        dev->halt = ppc_4xx_eth_halt;
        dev->send = ppc_4xx_eth_send;
        dev->recv = ppc_4xx_eth_rx;

        eth_register (dev);
}
#else /* !defined(CONFIG_NET_MULTI) */
void eth_halt (void)
{
        ppc_4xx_eth_halt(NULL);
}

int eth_init (bd_t *bis)
{
        return (ppc_4xx_eth_init(NULL, bis));
}
int eth_send(volatile void *packet, int length)
{
        return (ppc_4xx_eth_send(NULL, packet, length));
}

int eth_rx(void)
{
        return (ppc_4xx_eth_rx(NULL));
}
#endif /* !defined(CONFIG_NET_MULTI) */

#endif	/* CONFIG_405GP */

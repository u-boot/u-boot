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
#include <commproc.h>
#include <net.h>
#include <command.h>



/**************************************************************
 *
 * FEC Ethernet Initialization Routine
 *
 *************************************************************/

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

#define	FEC_RESET_DELAY		50000



/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH  1520

#define TX_BUF_CNT 2

#define TOUT_LOOP 100

#define PKT_MAXBUF_SIZE         1518
#define PKT_MINBUF_SIZE         64
#define PKT_MAXBLR_SIZE         1520



#ifdef CONFIG_M5272
#define FEC_ADDR 0x10000840
#endif
#ifdef CONFIG_M5282
#define FEC_ADDR 0x40001000
#endif

#undef	ET_DEBUG

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(FEC_ENET)



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

static RTXBD *rtx = 0x380000;


int eth_send(volatile void *packet, int length)
{
	int j, rc;
	volatile fec_t *fecp = FEC_ADDR;

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

	/* Activate transmit Buffer Descriptor polling */
	fecp->fec_x_des_active = 0x01000000;	/* Descriptor polling active	*/

	j = 0;
	while ((rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_READY) && (j<TOUT_LOOP)) {
		udelay(1);
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

int eth_rx(void)
{
	int length;
	volatile fec_t *fecp = FEC_ADDR;

   for (;;)
   {     
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
	
	/* Try to fill Buffer Descriptors */
	fecp->fec_r_des_active = 0x01000000;	/* Descriptor polling active	*/
   }

   return length;
}


int eth_init (bd_t * bd)
{

	int i;
	volatile fec_t *fecp = FEC_ADDR;
	
	/* Whack a reset.
	 * A delay is required between a reset of the FEC block and
	 * initialization of other FEC registers because the reset takes
	 * some time to complete. If you don't delay, subsequent writes
	 * to FEC registers might get killed by the reset routine which is
	 * still in progress.
	 */

	fecp->fec_ecntrl = FEC_ECNTRL_RESET;
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
	
	/* Clear any pending interrupt */
	fecp->fec_ievent = 0xffffffff;
	
	/* Set station address	 */
#define ea bd->bi_enetaddr
	fecp->fec_addr_low   =	(ea[0] << 24) | (ea[1] << 16) |
				(ea[2] <<  8) | (ea[3]        ) ;
	fecp->fec_addr_high  =	(ea[4] << 24) | (ea[5] << 16  ) ;
#undef ea

	/* Clear multicast address hash table
	 */
	fecp->fec_hash_table_high = 0;
	fecp->fec_hash_table_low  = 0;

	/* Set maximum receive buffer size.
	 */
	fecp->fec_r_buff_size = PKT_MAXBLR_SIZE;

	/*
	 * Setup Buffers and Buffer Desriptors
	 */
	rxIdx = 0;
	txIdx = 0;

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

	/* Half duplex mode */
	fecp->fec_r_cntrl = (PKT_MAXBUF_SIZE<<16) | FEC_RCNTRL_MII_MODE;
	fecp->fec_r_cntrl = (PKT_MAXBUF_SIZE<<16) | FEC_RCNTRL_MII_MODE;
	fecp->fec_x_cntrl = 0;

	fecp->fec_mii_speed = 0;

	/* Now enable the transmit and receive processing
	 */
	fecp->fec_ecntrl = FEC_ECNTRL_ETHER_EN;

	/* And last, try to fill Rx Buffer Descriptors */
	fecp->fec_r_des_active = 0x01000000;	/* Descriptor polling active	*/

	return 1;
}



void eth_halt(void)
{
	volatile fec_t *fecp = FEC_ADDR;
	fecp->fec_ecntrl = 0;
}

#endif

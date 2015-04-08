/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/*
 * Ethernet test
 *
 * The Serial Communication Controllers (SCC) listed in ctlr_list array below
 * are tested in the loopback ethernet mode.
 * The controllers are configured accordingly and several packets
 * are transmitted. The configurable test parameters are:
 *   MIN_PACKET_LENGTH - minimum size of packet to transmit
 *   MAX_PACKET_LENGTH - maximum size of packet to transmit
 *   TEST_NUM - number of tests
 */

#include <post.h>
#if CONFIG_POST & CONFIG_SYS_POST_ETHER
#if defined(CONFIG_8xx)
#include <commproc.h>
#elif defined(CONFIG_MPC8260)
#include <asm/cpm_8260.h>
#else
#error "Apparently a bad configuration, please fix."
#endif

#include <command.h>
#include <net.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

#define MIN_PACKET_LENGTH	64
#define MAX_PACKET_LENGTH	256
#define TEST_NUM		1

#define CTLR_SCC 0

extern void spi_init_f (void);
extern void spi_init_r (void);

/* The list of controllers to test */
#if defined(CONFIG_MPC823)
static int ctlr_list[][2] = { {CTLR_SCC, 1} };
#else
static int ctlr_list[][2] = { };
#endif

static struct {
	void (*init) (int index);
	void (*halt) (int index);
	int (*send) (int index, volatile void *packet, int length);
	int (*recv) (int index, void *packet, int length);
} ctlr_proc[1];

static char *ctlr_name[1] = { "SCC" };

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH  1520

#define TX_BUF_CNT 2

#define TOUT_LOOP 100

static char txbuf[DBUF_LENGTH];

static uint rxIdx;		/* index of the current RX buffer */
static uint txIdx;		/* index of the current TX buffer */

/*
  * SCC Ethernet Tx and Rx buffer descriptors allocated at the
  *  immr->udata_bd address on Dual-Port RAM
  * Provide for Double Buffering
  */

typedef volatile struct CommonBufferDescriptor {
	cbd_t rxbd[PKTBUFSRX];		/* Rx BD */
	cbd_t txbd[TX_BUF_CNT];		/* Tx BD */
} RTXBD;

static RTXBD *rtx;

  /*
   * SCC callbacks
   */

static void scc_init (int scc_index)
{
	uchar ea[6];

	static int proff[] = {
				PROFF_SCC1,
				PROFF_SCC2,
				PROFF_SCC3,
				PROFF_SCC4,
	};
	static unsigned int cpm_cr[] = {
				CPM_CR_CH_SCC1,
				CPM_CR_CH_SCC2,
				CPM_CR_CH_SCC3,
				CPM_CR_CH_SCC4,
	};

	int i;
	scc_enet_t *pram_ptr;

	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

	immr->im_cpm.cp_scc[scc_index].scc_gsmrl &=
			~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);

	pram_ptr = (scc_enet_t *) & (immr->im_cpm.cp_dparam[proff[scc_index]]);

	rxIdx = 0;
	txIdx = 0;

#ifdef CONFIG_SYS_ALLOC_DPRAM
	rtx = (RTXBD *) (immr->im_cpm.cp_dpmem +
					 dpram_alloc_align (sizeof (RTXBD), 8));
#else
	rtx = (RTXBD *) (immr->im_cpm.cp_dpmem + CPM_SCC_BASE);
#endif

#if 0

#if (defined(PA_ENET_RXD) && defined(PA_ENET_TXD))
	/* Configure port A pins for Txd and Rxd.
	 */
	immr->im_ioport.iop_papar |= (PA_ENET_RXD | PA_ENET_TXD);
	immr->im_ioport.iop_padir &= ~(PA_ENET_RXD | PA_ENET_TXD);
	immr->im_ioport.iop_paodr &= ~PA_ENET_TXD;
#elif (defined(PB_ENET_RXD) && defined(PB_ENET_TXD))
	/* Configure port B pins for Txd and Rxd.
	 */
	immr->im_cpm.cp_pbpar |= (PB_ENET_RXD | PB_ENET_TXD);
	immr->im_cpm.cp_pbdir &= ~(PB_ENET_RXD | PB_ENET_TXD);
	immr->im_cpm.cp_pbodr &= ~PB_ENET_TXD;
#else
#error Configuration Error: exactly ONE of PA_ENET_[RT]XD, PB_ENET_[RT]XD must be defined
#endif

#if defined(PC_ENET_LBK)
	/* Configure port C pins to disable External Loopback
	 */
	immr->im_ioport.iop_pcpar &= ~PC_ENET_LBK;
	immr->im_ioport.iop_pcdir |= PC_ENET_LBK;
	immr->im_ioport.iop_pcso &= ~PC_ENET_LBK;
	immr->im_ioport.iop_pcdat &= ~PC_ENET_LBK;	/* Disable Loopback */
#endif /* PC_ENET_LBK */

	/* Configure port C pins to enable CLSN and RENA.
	 */
	immr->im_ioport.iop_pcpar &= ~(PC_ENET_CLSN | PC_ENET_RENA);
	immr->im_ioport.iop_pcdir &= ~(PC_ENET_CLSN | PC_ENET_RENA);
	immr->im_ioport.iop_pcso |= (PC_ENET_CLSN | PC_ENET_RENA);

	/* Configure port A for TCLK and RCLK.
	 */
	immr->im_ioport.iop_papar |= (PA_ENET_TCLK | PA_ENET_RCLK);
	immr->im_ioport.iop_padir &= ~(PA_ENET_TCLK | PA_ENET_RCLK);

	/*
	 * Configure Serial Interface clock routing -- see section 16.7.5.3
	 * First, clear all SCC bits to zero, then set the ones we want.
	 */

	immr->im_cpm.cp_sicr &= ~SICR_ENET_MASK;
	immr->im_cpm.cp_sicr |= SICR_ENET_CLKRT;
#else
	/*
	 * SCC2 receive clock is BRG2
	 * SCC2 transmit clock is BRG3
	 */
	immr->im_cpm.cp_brgc2 = 0x0001000C;
	immr->im_cpm.cp_brgc3 = 0x0001000C;

	immr->im_cpm.cp_sicr &= ~0x00003F00;
	immr->im_cpm.cp_sicr |=  0x00000a00;
#endif /* 0 */


	/*
	 * Initialize SDCR -- see section 16.9.23.7
	 * SDMA configuration register
	 */
	immr->im_siu_conf.sc_sdcr = 0x01;


	/*
	 * Setup SCC Ethernet Parameter RAM
	 */

	pram_ptr->sen_genscc.scc_rfcr = 0x18;	/* Normal Operation and Mot byte ordering */
	pram_ptr->sen_genscc.scc_tfcr = 0x18;	/* Mot byte ordering, Normal access */

	pram_ptr->sen_genscc.scc_mrblr = DBUF_LENGTH;	/* max. ET package len 1520 */

	pram_ptr->sen_genscc.scc_rbase = (unsigned int) (&rtx->rxbd[0]);	/* Set RXBD tbl start at Dual Port */
	pram_ptr->sen_genscc.scc_tbase = (unsigned int) (&rtx->txbd[0]);	/* Set TXBD tbl start at Dual Port */

	/*
	 * Setup Receiver Buffer Descriptors (13.14.24.18)
	 * Settings:
	 *     Empty, Wrap
	 */

	for (i = 0; i < PKTBUFSRX; i++) {
		rtx->rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
		rtx->rxbd[i].cbd_datlen = 0;	/* Reset */
		rtx->rxbd[i].cbd_bufaddr = (uint) net_rx_packets[i];
	}

	rtx->rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

	/*
	 * Setup Ethernet Transmitter Buffer Descriptors (13.14.24.19)
	 * Settings:
	 *    Add PADs to Short FRAMES, Wrap, Last, Tx CRC
	 */

	for (i = 0; i < TX_BUF_CNT; i++) {
		rtx->txbd[i].cbd_sc =
				(BD_ENET_TX_PAD | BD_ENET_TX_LAST | BD_ENET_TX_TC);
		rtx->txbd[i].cbd_datlen = 0;	/* Reset */
		rtx->txbd[i].cbd_bufaddr = (uint) (&txbuf[0]);
	}

	rtx->txbd[TX_BUF_CNT - 1].cbd_sc |= BD_ENET_TX_WRAP;

	/*
	 * Enter Command:  Initialize Rx Params for SCC
	 */

	do {				/* Spin until ready to issue command    */
		__asm__ ("eieio");
	} while (immr->im_cpm.cp_cpcr & CPM_CR_FLG);
	/* Issue command */
	immr->im_cpm.cp_cpcr =
			((CPM_CR_INIT_RX << 8) | (cpm_cr[scc_index] << 4) |
			 CPM_CR_FLG);
	do {				/* Spin until command processed     */
		__asm__ ("eieio");
	} while (immr->im_cpm.cp_cpcr & CPM_CR_FLG);

	/*
	 * Ethernet Specific Parameter RAM
	 *     see table 13-16, pg. 660,
	 *     pg. 681 (example with suggested settings)
	 */

	pram_ptr->sen_cpres = ~(0x0);	/* Preset CRC */
	pram_ptr->sen_cmask = 0xdebb20e3;	/* Constant Mask for CRC */
	pram_ptr->sen_crcec = 0x0;	/* Error Counter CRC (unused) */
	pram_ptr->sen_alec = 0x0;	/* Alignment Error Counter (unused) */
	pram_ptr->sen_disfc = 0x0;	/* Discard Frame Counter (unused) */
	pram_ptr->sen_pads = 0x8888;	/* Short Frame PAD Characters */

	pram_ptr->sen_retlim = 15;	/* Retry Limit Threshold */
	pram_ptr->sen_maxflr = 1518;	/* MAX Frame Length Register */
	pram_ptr->sen_minflr = 64;	/* MIN Frame Length Register */

	pram_ptr->sen_maxd1 = DBUF_LENGTH;	/* MAX DMA1 Length Register */
	pram_ptr->sen_maxd2 = DBUF_LENGTH;	/* MAX DMA2 Length Register */

	pram_ptr->sen_gaddr1 = 0x0;	/* Group Address Filter 1 (unused) */
	pram_ptr->sen_gaddr2 = 0x0;	/* Group Address Filter 2 (unused) */
	pram_ptr->sen_gaddr3 = 0x0;	/* Group Address Filter 3 (unused) */
	pram_ptr->sen_gaddr4 = 0x0;	/* Group Address Filter 4 (unused) */

	eth_getenv_enetaddr("ethaddr", ea);
	pram_ptr->sen_paddrh = (ea[5] << 8) + ea[4];
	pram_ptr->sen_paddrm = (ea[3] << 8) + ea[2];
	pram_ptr->sen_paddrl = (ea[1] << 8) + ea[0];

	pram_ptr->sen_pper = 0x0;	/* Persistence (unused) */
	pram_ptr->sen_iaddr1 = 0x0;	/* Individual Address Filter 1 (unused) */
	pram_ptr->sen_iaddr2 = 0x0;	/* Individual Address Filter 2 (unused) */
	pram_ptr->sen_iaddr3 = 0x0;	/* Individual Address Filter 3 (unused) */
	pram_ptr->sen_iaddr4 = 0x0;	/* Individual Address Filter 4 (unused) */
	pram_ptr->sen_taddrh = 0x0;	/* Tmp Address (MSB) (unused) */
	pram_ptr->sen_taddrm = 0x0;	/* Tmp Address (unused) */
	pram_ptr->sen_taddrl = 0x0;	/* Tmp Address (LSB) (unused) */

	/*
	 * Enter Command:  Initialize Tx Params for SCC
	 */

	do {				/* Spin until ready to issue command    */
		__asm__ ("eieio");
	} while (immr->im_cpm.cp_cpcr & CPM_CR_FLG);
	/* Issue command */
	immr->im_cpm.cp_cpcr =
			((CPM_CR_INIT_TX << 8) | (cpm_cr[scc_index] << 4) |
			 CPM_CR_FLG);
	do {				/* Spin until command processed     */
		__asm__ ("eieio");
	} while (immr->im_cpm.cp_cpcr & CPM_CR_FLG);

	/*
	 * Mask all Events in SCCM - we use polling mode
	 */
	immr->im_cpm.cp_scc[scc_index].scc_sccm = 0;

	/*
	 * Clear Events in SCCE -- Clear bits by writing 1's
	 */

	immr->im_cpm.cp_scc[scc_index].scc_scce = ~(0x0);


	/*
	 * Initialize GSMR High 32-Bits
	 * Settings:  Normal Mode
	 */

	immr->im_cpm.cp_scc[scc_index].scc_gsmrh = 0;

	/*
	 * Initialize GSMR Low 32-Bits, but do not Enable Transmit/Receive
	 * Settings:
	 *     TCI = Invert
	 *     TPL =  48 bits
	 *     TPP = Repeating 10's
	 *     LOOP = Loopback
	 *     MODE = Ethernet
	 */

	immr->im_cpm.cp_scc[scc_index].scc_gsmrl = (SCC_GSMRL_TCI |
						    SCC_GSMRL_TPL_48 |
						    SCC_GSMRL_TPP_10 |
						    SCC_GSMRL_DIAG_LOOP |
						    SCC_GSMRL_MODE_ENET);

	/*
	 * Initialize the DSR -- see section 13.14.4 (pg. 513) v0.4
	 */

	immr->im_cpm.cp_scc[scc_index].scc_dsr = 0xd555;

	/*
	 * Initialize the PSMR
	 * Settings:
	 *  CRC = 32-Bit CCITT
	 *  NIB = Begin searching for SFD 22 bits after RENA
	 *  LPB = Loopback Enable (Needed when FDE is set)
	 */
	immr->im_cpm.cp_scc[scc_index].scc_psmr = SCC_PSMR_ENCRC |
			SCC_PSMR_NIB22 | SCC_PSMR_LPB;

	/*
	 * Set the ENT/ENR bits in the GSMR Low -- Enable Transmit/Receive
	 */

	immr->im_cpm.cp_scc[scc_index].scc_gsmrl |=
			(SCC_GSMRL_ENR | SCC_GSMRL_ENT);
}

static void scc_halt (int scc_index)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

	immr->im_cpm.cp_scc[scc_index].scc_gsmrl &=
			~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);
	immr->im_ioport.iop_pcso  &=  ~(PC_ENET_CLSN | PC_ENET_RENA);
}

static int scc_send (int index, volatile void *packet, int length)
{
	int i, j = 0;

	while ((rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_READY) && (j < TOUT_LOOP)) {
		udelay (1);		/* will also trigger Wd if needed */
		j++;
	}
	if (j >= TOUT_LOOP)
		printf ("TX not ready\n");
	rtx->txbd[txIdx].cbd_bufaddr = (uint) packet;
	rtx->txbd[txIdx].cbd_datlen = length;
	rtx->txbd[txIdx].cbd_sc |=
			(BD_ENET_TX_READY | BD_ENET_TX_LAST | BD_ENET_TX_WRAP);
	while ((rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_READY) && (j < TOUT_LOOP)) {
		udelay (1);		/* will also trigger Wd if needed */
		j++;
	}
	if (j >= TOUT_LOOP)
		printf ("TX timeout\n");
	i = (rtx->txbd[txIdx].
		 cbd_sc & BD_ENET_TX_STATS) /* return only status bits */ ;
	return i;
}

static int scc_recv (int index, void *packet, int max_length)
{
	int length = -1;

	if (rtx->rxbd[rxIdx].cbd_sc & BD_ENET_RX_EMPTY) {
		goto Done;		/* nothing received */
	}

	if (!(rtx->rxbd[rxIdx].cbd_sc & 0x003f)) {
		length = rtx->rxbd[rxIdx].cbd_datlen - 4;
		memcpy (packet,
			(void *)(net_rx_packets[rxIdx]),
			length < max_length ? length : max_length);
	}

	/* Give the buffer back to the SCC. */
	rtx->rxbd[rxIdx].cbd_datlen = 0;

	/* wrap around buffer index when necessary */
	if ((rxIdx + 1) >= PKTBUFSRX) {
		rtx->rxbd[PKTBUFSRX - 1].cbd_sc =
				(BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY);
		rxIdx = 0;
	} else {
		rtx->rxbd[rxIdx].cbd_sc = BD_ENET_RX_EMPTY;
		rxIdx++;
	}

Done:
	return length;
}

  /*
   * Test routines
   */

static void packet_fill (char *packet, int length)
{
	char c = (char) length;
	int i;

	packet[0] = 0xFF;
	packet[1] = 0xFF;
	packet[2] = 0xFF;
	packet[3] = 0xFF;
	packet[4] = 0xFF;
	packet[5] = 0xFF;

	for (i = 6; i < length; i++) {
		packet[i] = c++;
	}
}

static int packet_check (char *packet, int length)
{
	char c = (char) length;
	int i;

	for (i = 6; i < length; i++) {
		if (packet[i] != c++)
			return -1;
	}

	return 0;
}

static int test_ctlr (int ctlr, int index)
{
	int res = -1;
	char packet_send[MAX_PACKET_LENGTH];
	char packet_recv[MAX_PACKET_LENGTH];
	int length;
	int i;
	int l;

	ctlr_proc[ctlr].init (index);

	for (i = 0; i < TEST_NUM; i++) {
		for (l = MIN_PACKET_LENGTH; l <= MAX_PACKET_LENGTH; l++) {
			packet_fill (packet_send, l);

			ctlr_proc[ctlr].send (index, packet_send, l);

			length = ctlr_proc[ctlr].recv (index, packet_recv,
							MAX_PACKET_LENGTH);

			if (length != l || packet_check (packet_recv, length) < 0) {
				goto Done;
			}
		}
	}

	res = 0;

Done:

	ctlr_proc[ctlr].halt (index);

	/*
	 * SCC2 Ethernet parameter RAM space overlaps
	 * the SPI parameter RAM space. So we need to restore
	 * the SPI configuration after SCC2 ethernet test.
	 */
#if defined(CONFIG_SPI)
	if (ctlr == CTLR_SCC && index == 1) {
		spi_init_f ();
		spi_init_r ();
	}
#endif

	if (res != 0) {
		post_log ("ethernet %s%d test failed\n", ctlr_name[ctlr],
				  index + 1);
	}

	return res;
}

int ether_post_test (int flags)
{
	int res = 0;
	int i;

	ctlr_proc[CTLR_SCC].init = scc_init;
	ctlr_proc[CTLR_SCC].halt = scc_halt;
	ctlr_proc[CTLR_SCC].send = scc_send;
	ctlr_proc[CTLR_SCC].recv = scc_recv;

	for (i = 0; i < ARRAY_SIZE(ctlr_list); i++) {
		if (test_ctlr (ctlr_list[i][0], ctlr_list[i][1]) != 0) {
			res = -1;
		}
	}

#if !defined(CONFIG_8xx_CONS_NONE)
	serial_reinit_all ();
#endif
	return res;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_ETHER */

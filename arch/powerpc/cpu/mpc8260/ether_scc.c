/*
 * MPC8260 SCC Ethernet
 *
 * Copyright (c) 2000 MontaVista Software, Inc.   Dan Malek (dmalek@jlc.net)
 *
 * (C) Copyright 2000 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright (c) 2001
 * Advent Networks, Inc. <http://www.adventnetworks.com>
 * Jay Monkman <jtm@smoothsmoothie.com>
 *
 * Modified so that it plays nicely when more than one ETHERNET interface
 * is in use a la ether_fcc.c.
 * (C) Copyright 2008
 * DENX Software Engineerin GmbH
 * Gary Jennejohn <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/cpm_8260.h>
#include <mpc8260.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <config.h>

#if (CONFIG_ETHER_INDEX == 1)
#  define PROFF_ENET            PROFF_SCC1
#  define CPM_CR_ENET_PAGE      CPM_CR_SCC1_PAGE
#  define CPM_CR_ENET_SBLOCK    CPM_CR_SCC1_SBLOCK
#  define CMXSCR_MASK          (CMXSCR_SC1          |\
				CMXSCR_RS1CS_MSK    |\
				CMXSCR_TS1CS_MSK)

#elif (CONFIG_ETHER_INDEX == 2)
#  define PROFF_ENET            PROFF_SCC2
#  define CPM_CR_ENET_PAGE      CPM_CR_SCC2_PAGE
#  define CPM_CR_ENET_SBLOCK    CPM_CR_SCC2_SBLOCK
#  define CMXSCR_MASK          (CMXSCR_SC2          |\
				CMXSCR_RS2CS_MSK    |\
				CMXSCR_TS2CS_MSK)

#elif (CONFIG_ETHER_INDEX == 3)
#  define PROFF_ENET            PROFF_SCC3
#  define CPM_CR_ENET_PAGE      CPM_CR_SCC3_PAGE
#  define CPM_CR_ENET_SBLOCK    CPM_CR_SCC3_SBLOCK
#  define CMXSCR_MASK          (CMXSCR_SC3          |\
				CMXSCR_RS3CS_MSK    |\
				CMXSCR_TS3CS_MSK)
#elif (CONFIG_ETHER_INDEX == 4)
#  define PROFF_ENET            PROFF_SCC4
#  define CPM_CR_ENET_PAGE      CPM_CR_SCC4_PAGE
#  define CPM_CR_ENET_SBLOCK    CPM_CR_SCC4_SBLOCK
#  define CMXSCR_MASK          (CMXSCR_SC4          |\
				CMXSCR_RS4CS_MSK    |\
				CMXSCR_TS4CS_MSK)

#endif


/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH  1520

#define TX_BUF_CNT 2

#if !defined(CONFIG_SYS_SCC_TOUT_LOOP)
  #define CONFIG_SYS_SCC_TOUT_LOOP 1000000
#endif

static char txbuf[TX_BUF_CNT][ DBUF_LENGTH ];

static uint rxIdx;      /* index of the current RX buffer */
static uint txIdx;      /* index of the current TX buffer */

/*
 * SCC Ethernet Tx and Rx buffer descriptors allocated at the
 *  immr->udata_bd address on Dual-Port RAM
 * Provide for Double Buffering
 */

typedef volatile struct CommonBufferDescriptor {
    cbd_t rxbd[PKTBUFSRX];         /* Rx BD */
    cbd_t txbd[TX_BUF_CNT];        /* Tx BD */
} RTXBD;

static RTXBD *rtx;


static int sec_send(struct eth_device *dev, void *packet, int length)
{
    int i;
    int result = 0;

    if (length <= 0) {
	printf("scc: bad packet size: %d\n", length);
	goto out;
    }

    for(i=0; rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_READY; i++) {
	if (i >= CONFIG_SYS_SCC_TOUT_LOOP) {
	    puts ("scc: tx buffer not ready\n");
	    goto out;
	}
    }

    rtx->txbd[txIdx].cbd_bufaddr = (uint)packet;
    rtx->txbd[txIdx].cbd_datlen = length;
    rtx->txbd[txIdx].cbd_sc |= (BD_ENET_TX_READY | BD_ENET_TX_LAST |
				BD_ENET_TX_WRAP);

    for(i=0; rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_READY; i++) {
	if (i >= CONFIG_SYS_SCC_TOUT_LOOP) {
	    puts ("scc: tx error\n");
	    goto out;
	}
    }

    /* return only status bits */
    result = rtx->txbd[txIdx].cbd_sc & BD_ENET_TX_STATS;

 out:
    return result;
}


static int sec_rx(struct eth_device *dev)
{
    int length;

    for (;;)
    {
	if (rtx->rxbd[rxIdx].cbd_sc & BD_ENET_RX_EMPTY) {
	    length = -1;
	    break;     /* nothing received - leave for() loop */
	}

	length = rtx->rxbd[rxIdx].cbd_datlen;

	if (rtx->rxbd[rxIdx].cbd_sc & 0x003f)
	{
	    printf("err: %x\n", rtx->rxbd[rxIdx].cbd_sc);
	}
	else
	{
	    /* Pass the packet up to the protocol layers. */
	    net_process_received_packet(net_rx_packets[rxIdx], length - 4);
	}


	/* Give the buffer back to the SCC. */
	rtx->rxbd[rxIdx].cbd_datlen = 0;

	/* wrap around buffer index when necessary */
	if ((rxIdx + 1) >= PKTBUFSRX) {
	    rtx->rxbd[PKTBUFSRX - 1].cbd_sc = (BD_ENET_RX_WRAP |
					       BD_ENET_RX_EMPTY);
	    rxIdx = 0;
	}
	else {
	    rtx->rxbd[rxIdx].cbd_sc = BD_ENET_RX_EMPTY;
	    rxIdx++;
	}
    }
    return length;
}

/**************************************************************
 *
 * SCC Ethernet Initialization Routine
 *
 *************************************************************/

static int sec_init(struct eth_device *dev, bd_t *bis)
{
    int i;
    volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
    scc_enet_t *pram_ptr;
    uint dpaddr;
    uchar ea[6];

    rxIdx = 0;
    txIdx = 0;

    /*
     * Assign static pointer to BD area.
     * Avoid exhausting DPRAM, which would cause a panic.
     */
    if (rtx == NULL) {
	    dpaddr = m8260_cpm_dpalloc(sizeof(RTXBD) + 2, 16);
	    rtx = (RTXBD *)&immr->im_dprambase[dpaddr];
    }

    /* 24.21 - (1-3): ioports have been set up already */

    /* 24.21 - (4,5): connect SCC's tx and rx clocks, use NMSI for SCC */
    immr->im_cpmux.cmx_uar = 0;
    immr->im_cpmux.cmx_scr = ( (immr->im_cpmux.cmx_scr & ~CMXSCR_MASK) |
			       CONFIG_SYS_CMXSCR_VALUE);


    /* 24.21 (6) write RBASE and TBASE to parameter RAM */
    pram_ptr = (scc_enet_t *)&(immr->im_dprambase[PROFF_ENET]);
    pram_ptr->sen_genscc.scc_rbase = (unsigned int)(&rtx->rxbd[0]);
    pram_ptr->sen_genscc.scc_tbase = (unsigned int)(&rtx->txbd[0]);

    pram_ptr->sen_genscc.scc_rfcr = 0x18;  /* Nrml Ops and Mot byte ordering */
    pram_ptr->sen_genscc.scc_tfcr = 0x18;  /* Mot byte ordering, Nrml access */

    pram_ptr->sen_genscc.scc_mrblr = DBUF_LENGTH; /* max. package len 1520 */

    pram_ptr->sen_cpres  = ~(0x0);        /* Preset CRC */
    pram_ptr->sen_cmask  = 0xdebb20e3;    /* Constant Mask for CRC */


    /* 24.21 - (7): Write INIT RX AND TX PARAMETERS to CPCR */
    while(immr->im_cpm.cp_cpcr & CPM_CR_FLG);
    immr->im_cpm.cp_cpcr = mk_cr_cmd(CPM_CR_ENET_PAGE,
				     CPM_CR_ENET_SBLOCK,
				     0x0c,
				     CPM_CR_INIT_TRX) | CPM_CR_FLG;

    /* 24.21 - (8-18): Set up parameter RAM */
    pram_ptr->sen_crcec  = 0x0;           /* Error Counter CRC (unused) */
    pram_ptr->sen_alec   = 0x0;           /* Align Error Counter (unused) */
    pram_ptr->sen_disfc  = 0x0;           /* Discard Frame Counter (unused) */

    pram_ptr->sen_pads   = 0x8888;        /* Short Frame PAD Characters */

    pram_ptr->sen_retlim = 15;            /* Retry Limit Threshold */

    pram_ptr->sen_maxflr = 1518;  /* MAX Frame Length Register */
    pram_ptr->sen_minflr = 64;            /* MIN Frame Length Register */

    pram_ptr->sen_maxd1  = DBUF_LENGTH;   /* MAX DMA1 Length Register */
    pram_ptr->sen_maxd2  = DBUF_LENGTH;   /* MAX DMA2 Length Register */

    pram_ptr->sen_gaddr1 = 0x0;   /* Group Address Filter 1 (unused) */
    pram_ptr->sen_gaddr2 = 0x0;   /* Group Address Filter 2 (unused) */
    pram_ptr->sen_gaddr3 = 0x0;   /* Group Address Filter 3 (unused) */
    pram_ptr->sen_gaddr4 = 0x0;   /* Group Address Filter 4 (unused) */

    eth_getenv_enetaddr("ethaddr", ea);
    pram_ptr->sen_paddrh = (ea[5] << 8) + ea[4];
    pram_ptr->sen_paddrm = (ea[3] << 8) + ea[2];
    pram_ptr->sen_paddrl = (ea[1] << 8) + ea[0];

    pram_ptr->sen_pper   = 0x0;   /* Persistence (unused) */

    pram_ptr->sen_iaddr1 = 0x0;   /* Individual Address Filter 1 (unused) */
    pram_ptr->sen_iaddr2 = 0x0;   /* Individual Address Filter 2 (unused) */
    pram_ptr->sen_iaddr3 = 0x0;   /* Individual Address Filter 3 (unused) */
    pram_ptr->sen_iaddr4 = 0x0;   /* Individual Address Filter 4 (unused) */

    pram_ptr->sen_taddrh = 0x0;   /* Tmp Address (MSB) (unused) */
    pram_ptr->sen_taddrm = 0x0;   /* Tmp Address (unused) */
    pram_ptr->sen_taddrl = 0x0;   /* Tmp Address (LSB) (unused) */

    /* 24.21 - (19): Initialize RxBD */
    for (i = 0; i < PKTBUFSRX; i++)
    {
	rtx->rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
	rtx->rxbd[i].cbd_datlen = 0;                  /* Reset */
	rtx->rxbd[i].cbd_bufaddr = (uint)net_rx_packets[i];
    }

    rtx->rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

    /* 24.21 - (20): Initialize TxBD */
    for (i = 0; i < TX_BUF_CNT; i++)
    {
	rtx->txbd[i].cbd_sc = (BD_ENET_TX_PAD  |
			       BD_ENET_TX_LAST |
			       BD_ENET_TX_TC);
	rtx->txbd[i].cbd_datlen = 0;                  /* Reset */
	rtx->txbd[i].cbd_bufaddr = (uint)&txbuf[i][0];
    }

    rtx->txbd[TX_BUF_CNT - 1].cbd_sc |= BD_ENET_TX_WRAP;

    /* 24.21 - (21): Write 0xffff to SCCE */
    immr->im_scc[CONFIG_ETHER_INDEX-1].scc_scce = ~(0x0);

    /* 24.21 - (22): Write to SCCM to enable TXE, RXF, TXB events */
    immr->im_scc[CONFIG_ETHER_INDEX-1].scc_sccm = (SCCE_ENET_TXE |
						   SCCE_ENET_RXF |
						   SCCE_ENET_TXB);

    /* 24.21 - (23): we don't use ethernet interrupts */

    /* 24.21 - (24): Clear GSMR_H to enable normal operations */
    immr->im_scc[CONFIG_ETHER_INDEX-1].scc_gsmrh = 0;

    /* 24.21 - (25): Clear GSMR_L to enable normal operations */
    immr->im_scc[CONFIG_ETHER_INDEX-1].scc_gsmrl = (SCC_GSMRL_TCI        |
						    SCC_GSMRL_TPL_48     |
						    SCC_GSMRL_TPP_10     |
						    SCC_GSMRL_MODE_ENET);

    /* 24.21 - (26): Initialize DSR */
    immr->im_scc[CONFIG_ETHER_INDEX-1].scc_dsr = 0xd555;

    /* 24.21 - (27): Initialize PSMR2
     *
     * Settings:
     *	CRC = 32-Bit CCITT
     *	NIB = Begin searching for SFD 22 bits after RENA
     *	FDE = Full Duplex Enable
     *	BRO = Reject broadcast packets
     *	PROMISCOUS = Catch all packets regardless of dest. MAC adress
     */
    immr->im_scc[CONFIG_ETHER_INDEX-1].scc_psmr   =	SCC_PSMR_ENCRC	|
							SCC_PSMR_NIB22	|
#if defined(CONFIG_SCC_ENET_FULL_DUPLEX)
							SCC_PSMR_FDE	|
#endif
#if defined(CONFIG_SCC_ENET_NO_BROADCAST)
							SCC_PSMR_BRO	|
#endif
#if defined(CONFIG_SCC_ENET_PROMISCOUS)
							SCC_PSMR_PRO	|
#endif
							0;

    /* 24.21 - (28): Write to GSMR_L to enable SCC */
    immr->im_scc[CONFIG_ETHER_INDEX-1].scc_gsmrl |= (SCC_GSMRL_ENR |
						     SCC_GSMRL_ENT);

    return 0;
}


static void sec_halt(struct eth_device *dev)
{
    volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
    immr->im_scc[CONFIG_ETHER_INDEX-1].scc_gsmrl &= ~(SCC_GSMRL_ENR |
						      SCC_GSMRL_ENT);
}

#if 0
static void sec_restart(void)
{
    volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
    immr->im_cpm.cp_scc[CONFIG_ETHER_INDEX-1].scc_gsmrl |= (SCC_GSMRL_ENR |
							    SCC_GSMRL_ENT);
}
#endif

int mpc82xx_scc_enet_initialize(bd_t *bis)
{
	struct eth_device *dev;

	dev = (struct eth_device *) malloc(sizeof *dev);
	memset(dev, 0, sizeof *dev);

	strcpy(dev->name, "SCC");
	dev->init   = sec_init;
	dev->halt   = sec_halt;
	dev->send   = sec_send;
	dev->recv   = sec_rx;

	eth_register(dev);

	return 1;
}

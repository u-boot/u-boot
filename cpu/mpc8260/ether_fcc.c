/*
 * MPC8260 FCC Fast Ethernet
 *
 * Copyright (c) 2000 MontaVista Software, Inc.   Dan Malek (dmalek@jlc.net)
 *
 * (C) Copyright 2000 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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

/*
 * MPC8260 FCC Fast Ethernet
 * Basic ET HW initialization and packet RX/TX routines
 *
 * This code will not perform the IO port configuration. This should be
 * done in the iop_conf_t structure specific for the board.
 *
 * TODO:
 * add a PHY driver to do the negotiation
 * reflect negotiation results in FPSMR
 * look for ways to configure the board specific stuff elsewhere, eg.
 *    config_xxx.h or the board directory
 */

#include <common.h>
#include <malloc.h>
#include <asm/cpm_8260.h>
#include <mpc8260.h>
#include <command.h>
#include <config.h>
#include <net.h>

#if defined(CONFIG_ETHER_ON_FCC) && (CONFIG_COMMANDS & CFG_CMD_NET) && \
	defined(CONFIG_NET_MULTI)

static struct ether_fcc_info_s
{
	int ether_index;
	int proff_enet;
	ulong cpm_cr_enet_sblock;
	ulong cpm_cr_enet_page;
	ulong cmxfcr_mask;
	ulong cmxfcr_value;
}
	ether_fcc_info[] =
{
#ifdef CONFIG_ETHER_ON_FCC1
{
	0,
	PROFF_FCC1,
	CPM_CR_FCC1_SBLOCK,
	CPM_CR_FCC1_PAGE,
	CFG_CMXFCR_MASK1,
	CFG_CMXFCR_VALUE1
},
#endif

#ifdef CONFIG_ETHER_ON_FCC2
{
	1,
	PROFF_FCC2,
	CPM_CR_FCC2_SBLOCK,
	CPM_CR_FCC2_PAGE,
	CFG_CMXFCR_MASK2,
	CFG_CMXFCR_VALUE2
},
#endif

#ifdef CONFIG_ETHER_ON_FCC3
{
	2,
	PROFF_FCC3,
	CPM_CR_FCC3_SBLOCK,
	CPM_CR_FCC3_PAGE,
	CFG_CMXFCR_MASK3,
	CFG_CMXFCR_VALUE3
},
#endif
};

/*---------------------------------------------------------------------*/

/* Maximum input DMA size.  Must be a should(?) be a multiple of 4. */
#define PKT_MAXDMA_SIZE         1520

/* The FCC stores dest/src/type, data, and checksum for receive packets. */
#define PKT_MAXBUF_SIZE         1518
#define PKT_MINBUF_SIZE         64

/* Maximum input buffer size.  Must be a multiple of 32. */
#define PKT_MAXBLR_SIZE         1536

#define TOUT_LOOP 1000000

#define TX_BUF_CNT 2
#ifdef __GNUC__
static char txbuf[TX_BUF_CNT][PKT_MAXBLR_SIZE] __attribute__ ((aligned(8)));
#else
#error "txbuf must be 64-bit aligned"
#endif

static uint rxIdx;	/* index of the current RX buffer */
static uint txIdx;	/* index of the current TX buffer */

/*
 * FCC Ethernet Tx and Rx buffer descriptors.
 * Provide for Double Buffering
 * Note: PKTBUFSRX is defined in net.h
 */

typedef volatile struct rtxbd {
    cbd_t rxbd[PKTBUFSRX];
    cbd_t txbd[TX_BUF_CNT];
} RTXBD;

/*  Good news: the FCC supports external BDs! */
#ifdef __GNUC__
static RTXBD rtx __attribute__ ((aligned(8)));
#else
#error "rtx must be 64-bit aligned"
#endif

static int fec_send(struct eth_device* dev, volatile void *packet, int length)
{
    int i;
    int result = 0;

    if (length <= 0) {
	printf("fec: bad packet size: %d\n", length);
	goto out;
    }

    for(i=0; rtx.txbd[txIdx].cbd_sc & BD_ENET_TX_READY; i++) {
	if (i >= TOUT_LOOP) {
	    printf("fec: tx buffer not ready\n");
	    goto out;
	}
    }

    rtx.txbd[txIdx].cbd_bufaddr = (uint)packet;
    rtx.txbd[txIdx].cbd_datlen = length;
    rtx.txbd[txIdx].cbd_sc |= (BD_ENET_TX_READY | BD_ENET_TX_LAST |
			       BD_ENET_TX_WRAP);

    for(i=0; rtx.txbd[txIdx].cbd_sc & BD_ENET_TX_READY; i++) {
	if (i >= TOUT_LOOP) {
	    printf("fec: tx error\n");
	    goto out;
	}
    }

#ifdef ET_DEBUG
    printf("cycles: %d status: %04x\n", i, rtx.txbd[txIdx].cbd_sc);
#endif

    /* return only status bits */
    result = rtx.txbd[txIdx].cbd_sc & BD_ENET_TX_STATS;

out:
    return result;
}

static int fec_recv(struct eth_device* dev)
{
    int length;

    for (;;)
    {
	if (rtx.rxbd[rxIdx].cbd_sc & BD_ENET_RX_EMPTY) {
	    length = -1;
	    break;     /* nothing received - leave for() loop */
	}
	length = rtx.rxbd[rxIdx].cbd_datlen;

	if (rtx.rxbd[rxIdx].cbd_sc & 0x003f) {
	    printf("fec: rx error %04x\n", rtx.rxbd[rxIdx].cbd_sc);
	}
	else {
	    /* Pass the packet up to the protocol layers. */
	    NetReceive(NetRxPackets[rxIdx], length - 4);
	}


	/* Give the buffer back to the FCC. */
	rtx.rxbd[rxIdx].cbd_datlen = 0;

	/* wrap around buffer index when necessary */
	if ((rxIdx + 1) >= PKTBUFSRX) {
	    rtx.rxbd[PKTBUFSRX - 1].cbd_sc = (BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY);
	    rxIdx = 0;
	}
	else {
	    rtx.rxbd[rxIdx].cbd_sc = BD_ENET_RX_EMPTY;
	    rxIdx++;
	}
    }
    return length;
}


static int fec_init(struct eth_device* dev, bd_t *bis)
{
    struct ether_fcc_info_s * info = dev->priv;
    int i;
    volatile immap_t *immr = (immap_t *)CFG_IMMR;
    volatile cpm8260_t *cp = &(immr->im_cpm);
    fcc_enet_t *pram_ptr;
    unsigned long mem_addr;

#if 0
    mii_discover_phy();
#endif

    /* 28.9 - (1-2): ioports have been set up already */

    /* 28.9 - (3): connect FCC's tx and rx clocks */
    immr->im_cpmux.cmx_uar = 0;
    immr->im_cpmux.cmx_fcr = (immr->im_cpmux.cmx_fcr & ~info->cmxfcr_mask) |
    							info->cmxfcr_value;

    /* 28.9 - (4): GFMR: disable tx/rx, CCITT CRC, Mode Ethernet */
    immr->im_fcc[info->ether_index].fcc_gfmr =
      FCC_GFMR_MODE_ENET | FCC_GFMR_TCRC_32;

    /* 28.9 - (5): FPSMR: enable full duplex, select CCITT CRC for Ethernet */
    immr->im_fcc[info->ether_index].fcc_fpsmr = CFG_FCC_PSMR | FCC_PSMR_ENCRC;

    /* 28.9 - (6): FDSR: Ethernet Syn */
    immr->im_fcc[info->ether_index].fcc_fdsr = 0xD555;

    /* reset indeces to current rx/tx bd (see eth_send()/eth_rx()) */
    rxIdx = 0;
    txIdx = 0;

    /* Setup Receiver Buffer Descriptors */
    for (i = 0; i < PKTBUFSRX; i++)
    {
      rtx.rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
      rtx.rxbd[i].cbd_datlen = 0;
      rtx.rxbd[i].cbd_bufaddr = (uint)NetRxPackets[i];
    }
    rtx.rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

    /* Setup Ethernet Transmitter Buffer Descriptors */
    for (i = 0; i < TX_BUF_CNT; i++)
    {
      rtx.txbd[i].cbd_sc = (BD_ENET_TX_PAD | BD_ENET_TX_LAST | BD_ENET_TX_TC);
      rtx.txbd[i].cbd_datlen = 0;
      rtx.txbd[i].cbd_bufaddr = (uint)&txbuf[i][0];
    }
    rtx.txbd[TX_BUF_CNT - 1].cbd_sc |= BD_ENET_TX_WRAP;

    /* 28.9 - (7): initialise parameter ram */
    pram_ptr = (fcc_enet_t *)&(immr->im_dprambase[info->proff_enet]);

    /* clear whole structure to make sure all reserved fields are zero */
    memset((void*)pram_ptr, 0, sizeof(fcc_enet_t));

    /*
     * common Parameter RAM area
     *
     * Allocate space in the reserved FCC area of DPRAM for the
     * internal buffers.  No one uses this space (yet), so we
     * can do this.  Later, we will add resource management for
     * this area.
     */
    mem_addr = CPM_FCC_SPECIAL_BASE + ((info->ether_index) * 64);
    pram_ptr->fen_genfcc.fcc_riptr = mem_addr;
    pram_ptr->fen_genfcc.fcc_tiptr = mem_addr+32;
    /*
     * Set maximum bytes per receive buffer.
     * It must be a multiple of 32.
     */
    pram_ptr->fen_genfcc.fcc_mrblr = PKT_MAXBLR_SIZE;
    pram_ptr->fen_genfcc.fcc_rstate = (CPMFCR_GBL | CPMFCR_EB |
				       CFG_CPMFCR_RAMTYPE) << 24;
    pram_ptr->fen_genfcc.fcc_rbase = (unsigned int)(&rtx.rxbd[rxIdx]);
    pram_ptr->fen_genfcc.fcc_tstate = (CPMFCR_GBL | CPMFCR_EB |
				       CFG_CPMFCR_RAMTYPE) << 24;
    pram_ptr->fen_genfcc.fcc_tbase = (unsigned int)(&rtx.txbd[txIdx]);

    /* protocol-specific area */
    pram_ptr->fen_cmask = 0xdebb20e3;	/* CRC mask */
    pram_ptr->fen_cpres = 0xffffffff;	/* CRC preset */
    pram_ptr->fen_retlim = 15;		/* Retry limit threshold */
    pram_ptr->fen_mflr = PKT_MAXBUF_SIZE;   /* maximum frame length register */
    /*
     * Set Ethernet station address.
     *
     * This is supplied in the board information structure, so we
     * copy that into the controller.
     * So, far we have only been given one Ethernet address. We make
     * it unique by setting a few bits in the upper byte of the
     * non-static part of the address.
     */
#define ea eth_get_dev()->enetaddr
    pram_ptr->fen_paddrh = (ea[5] << 8) + ea[4];
    pram_ptr->fen_paddrm = (ea[3] << 8) + ea[2];
    pram_ptr->fen_paddrl = (ea[1] << 8) + ea[0];
#undef ea
    pram_ptr->fen_minflr = PKT_MINBUF_SIZE; /* minimum frame length register */
    /* pad pointer. use tiptr since we don't need a specific padding char */
    pram_ptr->fen_padptr = pram_ptr->fen_genfcc.fcc_tiptr;
    pram_ptr->fen_maxd1 = PKT_MAXDMA_SIZE;	/* maximum DMA1 length */
    pram_ptr->fen_maxd2 = PKT_MAXDMA_SIZE;	/* maximum DMA2 length */
    pram_ptr->fen_rfthr = 1;
    pram_ptr->fen_rfcnt = 1;
#if 0
    printf("pram_ptr->fen_genfcc.fcc_rbase %08lx\n",
	pram_ptr->fen_genfcc.fcc_rbase);
    printf("pram_ptr->fen_genfcc.fcc_tbase %08lx\n",
	pram_ptr->fen_genfcc.fcc_tbase);
#endif

    /* 28.9 - (8): clear out events in FCCE */
    immr->im_fcc[info->ether_index].fcc_fcce = ~0x0;

    /* 28.9 - (9): FCCM: mask all events */
    immr->im_fcc[info->ether_index].fcc_fccm = 0;

    /* 28.9 - (10-12): we don't use ethernet interrupts */

    /* 28.9 - (13)
     *
     * Let's re-initialize the channel now.  We have to do it later
     * than the manual describes because we have just now finished
     * the BD initialization.
     */
    cp->cp_cpcr = mk_cr_cmd(info->cpm_cr_enet_page,
			    info->cpm_cr_enet_sblock,
			    0x0c,
			    CPM_CR_INIT_TRX) | CPM_CR_FLG;
    do {
	__asm__ __volatile__ ("eieio");
    } while (cp->cp_cpcr & CPM_CR_FLG);

    /* 28.9 - (14): enable tx/rx in gfmr */
    immr->im_fcc[info->ether_index].fcc_gfmr |= FCC_GFMR_ENT | FCC_GFMR_ENR;

    return 1;
}

static void fec_halt(struct eth_device* dev)
{
    struct ether_fcc_info_s * info = dev->priv;
    volatile immap_t *immr = (immap_t *)CFG_IMMR;

    /* write GFMR: disable tx/rx */
    immr->im_fcc[info->ether_index].fcc_gfmr &=
						~(FCC_GFMR_ENT | FCC_GFMR_ENR);
}

int fec_initialize(bd_t *bis)
{
	struct eth_device* dev;
	int i;

	for (i = 0; i < sizeof(ether_fcc_info) / sizeof(ether_fcc_info[0]); i++)
	{
		dev = (struct eth_device*) malloc(sizeof *dev);
		memset(dev, 0, sizeof *dev);

		sprintf(dev->name, "FCC%d ETHERNET",
		        ether_fcc_info[i].ether_index + 1);
		dev->priv   = &ether_fcc_info[i];
		dev->init   = fec_init;
		dev->halt   = fec_halt;
		dev->send   = fec_send;
		dev->recv   = fec_recv;

		eth_register(dev);
	}

	return 1;
}

#endif	/* CONFIG_ETHER_ON_FCC && CFG_CMD_NET && CONFIG_NET_MULTI */

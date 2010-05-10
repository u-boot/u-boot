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

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#include <miiphy.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_ETHER_ON_FCC) && defined(CONFIG_CMD_NET) && \
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
	CONFIG_SYS_CMXFCR_MASK1,
	CONFIG_SYS_CMXFCR_VALUE1
},
#endif

#ifdef CONFIG_ETHER_ON_FCC2
{
	1,
	PROFF_FCC2,
	CPM_CR_FCC2_SBLOCK,
	CPM_CR_FCC2_PAGE,
	CONFIG_SYS_CMXFCR_MASK2,
	CONFIG_SYS_CMXFCR_VALUE2
},
#endif

#ifdef CONFIG_ETHER_ON_FCC3
{
	2,
	PROFF_FCC3,
	CPM_CR_FCC3_SBLOCK,
	CPM_CR_FCC3_PAGE,
	CONFIG_SYS_CMXFCR_MASK3,
	CONFIG_SYS_CMXFCR_VALUE3
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
	    puts ("fec: tx buffer not ready\n");
	    goto out;
	}
    }

    rtx.txbd[txIdx].cbd_bufaddr = (uint)packet;
    rtx.txbd[txIdx].cbd_datlen = length;
    rtx.txbd[txIdx].cbd_sc |= (BD_ENET_TX_READY | BD_ENET_TX_LAST |
			       BD_ENET_TX_WRAP);

    for(i=0; rtx.txbd[txIdx].cbd_sc & BD_ENET_TX_READY; i++) {
	if (i >= TOUT_LOOP) {
	    puts ("fec: tx error\n");
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
    volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
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
    immr->im_fcc[info->ether_index].fcc_fpsmr = CONFIG_SYS_FCC_PSMR | FCC_PSMR_ENCRC;

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
				       CONFIG_SYS_CPMFCR_RAMTYPE) << 24;
    pram_ptr->fen_genfcc.fcc_rbase = (unsigned int)(&rtx.rxbd[rxIdx]);
    pram_ptr->fen_genfcc.fcc_tstate = (CPMFCR_GBL | CPMFCR_EB |
				       CONFIG_SYS_CPMFCR_RAMTYPE) << 24;
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
    volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

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

#if (defined(CONFIG_MII) || defined(CONFIG_CMD_MII)) \
		&& defined(CONFIG_BITBANGMII)
		miiphy_register(dev->name,
				bb_miiphy_read,	bb_miiphy_write);
#endif
	}

	return 1;
}

#ifdef CONFIG_ETHER_LOOPBACK_TEST

#define ELBT_BUFSZ	1024	/* must be multiple of 32 */

#define ELBT_CRCSZ	4

#define ELBT_NRXBD	4	/* must be at least 2 */
#define ELBT_NTXBD	4

#define ELBT_MAXRXERR	32
#define ELBT_MAXTXERR	32

#define ELBT_CLSWAIT	1000	/* msec to wait for further input frames */

typedef
	struct {
		uint off;
		char *lab;
	}
elbt_prdesc;

typedef
	struct {
		uint _l, _f, m, bc, mc, lg, no, sh, cr, ov, cl;
		uint badsrc, badtyp, badlen, badbit;
	}
elbt_rxeacc;

static elbt_prdesc rxeacc_descs[] = {
	{ offsetof(elbt_rxeacc, _l),		"Not Last in Frame"	},
	{ offsetof(elbt_rxeacc, _f),		"Not First in Frame"	},
	{ offsetof(elbt_rxeacc, m),		"Address Miss"		},
	{ offsetof(elbt_rxeacc, bc),		"Broadcast Address"	},
	{ offsetof(elbt_rxeacc, mc),		"Multicast Address"	},
	{ offsetof(elbt_rxeacc, lg),		"Frame Length Violation"},
	{ offsetof(elbt_rxeacc, no),		"Non-Octet Alignment"	},
	{ offsetof(elbt_rxeacc, sh),		"Short Frame"		},
	{ offsetof(elbt_rxeacc, cr),		"CRC Error"		},
	{ offsetof(elbt_rxeacc, ov),		"Overrun"		},
	{ offsetof(elbt_rxeacc, cl),		"Collision"		},
	{ offsetof(elbt_rxeacc, badsrc),	"Bad Src Address"	},
	{ offsetof(elbt_rxeacc, badtyp),	"Bad Frame Type"	},
	{ offsetof(elbt_rxeacc, badlen),	"Bad Frame Length"	},
	{ offsetof(elbt_rxeacc, badbit),	"Data Compare Errors"	},
};
static int rxeacc_ndesc = sizeof (rxeacc_descs) / sizeof (rxeacc_descs[0]);

typedef
	struct {
		uint def, hb, lc, rl, rc, un, csl;
	}
elbt_txeacc;

static elbt_prdesc txeacc_descs[] = {
	{ offsetof(elbt_txeacc, def),		"Defer Indication"	},
	{ offsetof(elbt_txeacc, hb),		"Heartbeat"		},
	{ offsetof(elbt_txeacc, lc),		"Late Collision"	},
	{ offsetof(elbt_txeacc, rl),		"Retransmission Limit"	},
	{ offsetof(elbt_txeacc, rc),		"Retry Count"		},
	{ offsetof(elbt_txeacc, un),		"Underrun"		},
	{ offsetof(elbt_txeacc, csl),		"Carrier Sense Lost"	},
};
static int txeacc_ndesc = sizeof (txeacc_descs) / sizeof (txeacc_descs[0]);

typedef
	struct {
		uchar rxbufs[ELBT_NRXBD][ELBT_BUFSZ];
		uchar txbufs[ELBT_NTXBD][ELBT_BUFSZ];
		cbd_t rxbd[ELBT_NRXBD];
		cbd_t txbd[ELBT_NTXBD];
		enum { Idle, Running, Closing, Closed } state;
		int proff, page, sblock;
		uint clstime, nsent, ntxerr, nrcvd, nrxerr;
		ushort rxerrs[ELBT_MAXRXERR], txerrs[ELBT_MAXTXERR];
		elbt_rxeacc rxeacc;
		elbt_txeacc txeacc;
	} __attribute__ ((aligned(8)))
elbt_chan;

static uchar patbytes[ELBT_NTXBD] = {
	0xff, 0xaa, 0x55, 0x00
};
static uint patwords[ELBT_NTXBD] = {
	0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000
};

#ifdef __GNUC__
static elbt_chan elbt_chans[3] __attribute__ ((aligned(8)));
#else
#error "elbt_chans must be 64-bit aligned"
#endif

#define CPM_CR_GRACEFUL_STOP_TX	((ushort)0x0005)

static elbt_prdesc epram_descs[] = {
	{ offsetof(fcc_enet_t, fen_crcec),	"CRC Errors"		},
	{ offsetof(fcc_enet_t, fen_alec),	"Alignment Errors"	},
	{ offsetof(fcc_enet_t, fen_disfc),	"Discarded Frames"	},
	{ offsetof(fcc_enet_t, fen_octc),	"Octets"		},
	{ offsetof(fcc_enet_t, fen_colc),	"Collisions"		},
	{ offsetof(fcc_enet_t, fen_broc),	"Broadcast Frames"	},
	{ offsetof(fcc_enet_t, fen_mulc),	"Multicast Frames"	},
	{ offsetof(fcc_enet_t, fen_uspc),	"Undersize Frames"	},
	{ offsetof(fcc_enet_t, fen_frgc),	"Fragments"		},
	{ offsetof(fcc_enet_t, fen_ospc),	"Oversize Frames"	},
	{ offsetof(fcc_enet_t, fen_jbrc),	"Jabbers"		},
	{ offsetof(fcc_enet_t, fen_p64c),	"64 Octet Frames"	},
	{ offsetof(fcc_enet_t, fen_p65c),	"65-127 Octet Frames"	},
	{ offsetof(fcc_enet_t, fen_p128c),	"128-255 Octet Frames"	},
	{ offsetof(fcc_enet_t, fen_p256c),	"256-511 Octet Frames"	},
	{ offsetof(fcc_enet_t, fen_p512c),	"512-1023 Octet Frames"	},
	{ offsetof(fcc_enet_t, fen_p1024c),	"1024-1518 Octet Frames"},
};
static int epram_ndesc = sizeof (epram_descs) / sizeof (epram_descs[0]);

/*
 * given an elbt_prdesc array and an array of base addresses, print
 * each prdesc down the screen with the values fetched from each
 * base address across the screen
 */
static void
print_desc (elbt_prdesc descs[], int ndesc, uchar *bases[], int nbase)
{
	elbt_prdesc *dp = descs, *edp = dp + ndesc;
	int i;

	printf ("%32s", "");

	for (i = 0; i < nbase; i++)
		printf ("  Channel %d", i);

	putc ('\n');

	while (dp < edp) {

		printf ("%-32s", dp->lab);

		for (i = 0; i < nbase; i++) {
			uint val = *(uint *)(bases[i] + dp->off);

			printf (" %10u", val);
		}

		putc ('\n');

		dp++;
	}
}

/*
 * return number of bits that are set in a value; value contains
 * nbits (right-justified) bits.
 */
static uint __inline__
nbs (uint value, uint nbits)
{
	uint cnt = 0;
#if 1
	uint pos = sizeof (uint) * 8;

	__asm__ __volatile__ ("\
	mtctr	%2\n\
1:	rlwnm.	%2,%1,%4,31,31\n\
	beq	2f\n\
	addi	%0,%0,1\n\
2:	subi	%4,%4,1\n\
	bdnz	1b"
	: "=r"(cnt)
	: "r"(value), "r"(nbits), "r"(cnt), "r"(pos)
	: "ctr", "cc" );
#else
	uint mask = 1;

	do {
		if (value & mask)
			cnt++;
		mask <<= 1;
	} while (--nbits);
#endif

	return (cnt);
}

static ulong
badbits (uchar *bp, int n, ulong pat)
{
	ulong *lp, cnt = 0;
	int nl;

	while (n > 0 && ((ulong)bp & (sizeof (ulong) - 1)) != 0) {
		uchar diff;

		diff = *bp++ ^ (uchar)pat;

		if (diff)
			cnt += nbs ((ulong)diff, 8);

		n--;
	}

	lp = (ulong *)bp;
	nl = n / sizeof (ulong);
	n -= nl * sizeof (ulong);

	while (nl > 0) {
		ulong diff;

		diff = *lp++ ^ pat;

		if (diff)
			cnt += nbs (diff, 32);

		nl--;
	}

	bp = (uchar *)lp;

	while (n > 0) {
		uchar diff;

		diff = *bp++ ^ (uchar)pat;

		if (diff)
			cnt += nbs ((ulong)diff, 8);

		n--;
	}

	return (cnt);
}

static inline unsigned short
swap16 (unsigned short x)
{
	return (((x & 0xff) << 8) | ((x & 0xff00) >> 8));
}

/* broadcast is not an error - we send them like that */
#define BD_ENET_RX_ERRS	(BD_ENET_RX_STATS & ~BD_ENET_RX_BC)

void
eth_loopback_test (void)
{
	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8260_t *cp = &(immr->im_cpm);
	int c, nclosed;
	ulong runtime, nmsec;
	uchar *bases[3];

	puts ("FCC Ethernet External loopback test\n");

	eth_getenv_enetaddr("ethaddr", NetOurEther);

	/*
	 * global initialisations for all FCC channels
	 */

	/* 28.9 - (1-2): ioports have been set up already */

#if defined(CONFIG_HYMOD)
	/*
	 * Attention: this is board-specific
	 * 0, FCC1
	 * 1, FCC2
	 * 2, FCC3
	 */
#       define FCC_START_LOOP 0
#       define FCC_END_LOOP   2

	/*
	 * Attention: this is board-specific
	 * - FCC1 Rx-CLK is CLK10
	 * - FCC1 Tx-CLK is CLK11
	 * - FCC2 Rx-CLK is CLK13
	 * - FCC2 Tx-CLK is CLK14
	 * - FCC3 Rx-CLK is CLK15
	 * - FCC3 Tx-CLK is CLK16
	 */

	/* 28.9 - (3): connect FCC's tx and rx clocks */
	immr->im_cpmux.cmx_uar = 0;
	immr->im_cpmux.cmx_fcr = CMXFCR_RF1CS_CLK10|CMXFCR_TF1CS_CLK11|\
	    CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14|\
	    CMXFCR_RF3CS_CLK15|CMXFCR_TF3CS_CLK16;
#elif defined(CONFIG_SBC8260) || defined(CONFIG_SACSng)
	/*
	 * Attention: this is board-specific
	 * 1, FCC2
	 */
#       define FCC_START_LOOP 1
#       define FCC_END_LOOP   1

	/*
	 * Attention: this is board-specific
	 * - FCC2 Rx-CLK is CLK13
	 * - FCC2 Tx-CLK is CLK14
	 */

	/* 28.9 - (3): connect FCC's tx and rx clocks */
	immr->im_cpmux.cmx_uar = 0;
	immr->im_cpmux.cmx_fcr = CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14;
#else
#error "eth_loopback_test not supported on your board"
#endif

	puts ("Initialise FCC channels:");

	for (c = FCC_START_LOOP; c <= FCC_END_LOOP; c++) {
		elbt_chan *ecp = &elbt_chans[c];
		volatile fcc_t *fcp = &immr->im_fcc[c];
		volatile fcc_enet_t *fpp;
		int i;
		ulong addr;

		/*
		 * initialise channel data
		 */

		printf (" %d", c);

		memset ((void *)ecp, 0, sizeof (*ecp));

		ecp->state = Idle;

		switch (c) {

		case 0: /* FCC1 */
			ecp->proff = PROFF_FCC1;
			ecp->page = CPM_CR_FCC1_PAGE;
			ecp->sblock = CPM_CR_FCC1_SBLOCK;
			break;

		case 1: /* FCC2 */
			ecp->proff = PROFF_FCC2;
			ecp->page = CPM_CR_FCC2_PAGE;
			ecp->sblock = CPM_CR_FCC2_SBLOCK;
			break;

		case 2: /* FCC3 */
			ecp->proff = PROFF_FCC3;
			ecp->page = CPM_CR_FCC3_PAGE;
			ecp->sblock = CPM_CR_FCC3_SBLOCK;
			break;
		}

		/*
		 * set up tx buffers and bds
		 */

		for (i = 0; i < ELBT_NTXBD; i++) {
			cbd_t *bdp = &ecp->txbd[i];
			uchar *bp = &ecp->txbufs[i][0];

			bdp->cbd_bufaddr = (uint)bp;
			/* room for crc */
			bdp->cbd_datlen = ELBT_BUFSZ - ELBT_CRCSZ;
			bdp->cbd_sc = BD_ENET_TX_READY | BD_ENET_TX_PAD | \
				BD_ENET_TX_LAST | BD_ENET_TX_TC;

			memset ((void *)bp, patbytes[i], ELBT_BUFSZ);
			NetSetEther (bp, NetBcastAddr, 0x8000);
		}
		ecp->txbd[ELBT_NTXBD - 1].cbd_sc |= BD_ENET_TX_WRAP;

		/*
		 * set up rx buffers and bds
		 */

		for (i = 0; i < ELBT_NRXBD; i++) {
		    cbd_t *bdp = &ecp->rxbd[i];
		    uchar *bp = &ecp->rxbufs[i][0];

		    bdp->cbd_bufaddr = (uint)bp;
		    bdp->cbd_datlen = 0;
		    bdp->cbd_sc = BD_ENET_RX_EMPTY;

		    memset ((void *)bp, 0, ELBT_BUFSZ);
		}
		ecp->rxbd[ELBT_NRXBD - 1].cbd_sc |= BD_ENET_RX_WRAP;

		/*
		 * set up the FCC channel hardware
		 */

		/* 28.9 - (4): GFMR: disable tx/rx, CCITT CRC, Mode Ethernet */
		fcp->fcc_gfmr = FCC_GFMR_MODE_ENET | FCC_GFMR_TCRC_32;

		/* 28.9 - (5): FPSMR: fd, enet CRC, Promis, RMON, Rx SHort */
		fcp->fcc_fpsmr = FCC_PSMR_FDE | FCC_PSMR_LPB | \
			FCC_PSMR_ENCRC | FCC_PSMR_PRO | \
			FCC_PSMR_MON | FCC_PSMR_RSH;

		/* 28.9 - (6): FDSR: Ethernet Syn */
		fcp->fcc_fdsr = 0xD555;

		/* 29.9 - (7): initialise parameter ram */
		fpp = (fcc_enet_t *)&(immr->im_dprambase[ecp->proff]);

		/* clear whole struct to make sure all resv fields are zero */
		memset ((void *)fpp, 0, sizeof (fcc_enet_t));

		/*
		 * common Parameter RAM area
		 *
		 * Allocate space in the reserved FCC area of DPRAM for the
		 * internal buffers.  No one uses this space (yet), so we
		 * can do this.  Later, we will add resource management for
		 * this area.
		 */
		addr = CPM_FCC_SPECIAL_BASE + (c * 64);
		fpp->fen_genfcc.fcc_riptr = addr;
		fpp->fen_genfcc.fcc_tiptr = addr + 32;

		/*
		 * Set maximum bytes per receive buffer.
		 * It must be a multiple of 32.
		 * buffers are in 60x bus memory.
		 */
		fpp->fen_genfcc.fcc_mrblr = PKT_MAXBLR_SIZE;
		fpp->fen_genfcc.fcc_rstate = (CPMFCR_GBL | CPMFCR_EB) << 24;
		fpp->fen_genfcc.fcc_rbase = (unsigned int)(&ecp->rxbd[0]);
		fpp->fen_genfcc.fcc_tstate = (CPMFCR_GBL | CPMFCR_EB) << 24;
		fpp->fen_genfcc.fcc_tbase = (unsigned int)(&ecp->txbd[0]);

		/* protocol-specific area */
		fpp->fen_cmask = 0xdebb20e3;	/* CRC mask */
		fpp->fen_cpres = 0xffffffff;	/* CRC preset */
		fpp->fen_retlim = 15;		/* Retry limit threshold */
		fpp->fen_mflr = PKT_MAXBUF_SIZE;/* max frame length register */

		/*
		 * Set Ethernet station address.
		 *
		 * This is supplied in the board information structure, so we
		 * copy that into the controller.
		 * So, far we have only been given one Ethernet address. We use
		 * the same address for all channels
		 */
#define ea NetOurEther
		fpp->fen_paddrh = (ea[5] << 8) + ea[4];
		fpp->fen_paddrm = (ea[3] << 8) + ea[2];
		fpp->fen_paddrl = (ea[1] << 8) + ea[0];
#undef ea

		fpp->fen_minflr = PKT_MINBUF_SIZE; /* min frame len register */
		/*
		 * pad pointer. use tiptr since we don't need
		 * a specific padding char
		 */
		fpp->fen_padptr = fpp->fen_genfcc.fcc_tiptr;
		fpp->fen_maxd1 = PKT_MAXDMA_SIZE;	/* max DMA1 length */
		fpp->fen_maxd2 = PKT_MAXDMA_SIZE;	/* max DMA2 length */
		fpp->fen_rfthr = 1;
		fpp->fen_rfcnt = 1;

		/* 28.9 - (8): clear out events in FCCE */
		fcp->fcc_fcce = ~0x0;

		/* 28.9 - (9): FCCM: mask all events */
		fcp->fcc_fccm = 0;

		/* 28.9 - (10-12): we don't use ethernet interrupts */

		/* 28.9 - (13)
		 *
		 * Let's re-initialize the channel now.  We have to do it later
		 * than the manual describes because we have just now finished
		 * the BD initialization.
		 */
		cp->cp_cpcr = mk_cr_cmd (ecp->page, ecp->sblock, \
			0x0c, CPM_CR_INIT_TRX) | CPM_CR_FLG;
		do {
			__asm__ __volatile__ ("eieio");
		} while (cp->cp_cpcr & CPM_CR_FLG);
	}

	puts (" done\nStarting test... (Ctrl-C to Finish)\n");

	/*
	 * Note: don't want serial output from here until the end of the
	 * test - the delays would probably stuff things up.
	 */

	clear_ctrlc ();
	runtime = get_timer (0);

	do {
		nclosed = 0;

		for (c = FCC_START_LOOP; c <= FCC_END_LOOP; c++) {
			volatile fcc_t *fcp = &immr->im_fcc[c];
			elbt_chan *ecp = &elbt_chans[c];
			int i;

			switch (ecp->state) {

			case Idle:
				/*
				 * set the channel Running ...
				 */

				/* 28.9 - (14): enable tx/rx in gfmr */
				fcp->fcc_gfmr |= FCC_GFMR_ENT | FCC_GFMR_ENR;

				ecp->state = Running;
				break;

			case Running:
				/*
				 * (while Running only) check for
				 * termination of the test
				 */

				(void)ctrlc ();

				if (had_ctrlc ()) {
					/*
					 * initiate a "graceful stop transmit"
					 * on the channel
					 */
					cp->cp_cpcr = mk_cr_cmd (ecp->page, \
						ecp->sblock, 0x0c, \
						CPM_CR_GRACEFUL_STOP_TX) | \
						CPM_CR_FLG;
					do {
						__asm__ __volatile__ ("eieio");
					} while (cp->cp_cpcr & CPM_CR_FLG);

					ecp->clstime = get_timer (0);
					ecp->state = Closing;
				}
				/* fall through ... */

			case Closing:
				/*
				 * (while Running or Closing) poll the channel:
				 * - check for any non-READY tx buffers and
				 *   make them ready
				 * - check for any non-EMPTY rx buffers and
				 *   check that they were received correctly,
				 *   adjust counters etc, then make empty
				 */

				for (i = 0; i < ELBT_NTXBD; i++) {
					cbd_t *bdp = &ecp->txbd[i];
					ushort sc = bdp->cbd_sc;

					if ((sc & BD_ENET_TX_READY) != 0)
						continue;

					/*
					 * this frame has finished
					 * transmitting
					 */
					ecp->nsent++;

					if (sc & BD_ENET_TX_STATS) {
						ulong n;

						/*
						 * we had an error on
						 * the transmission
						 */
						n = ecp->ntxerr++;
						if (n < ELBT_MAXTXERR)
							ecp->txerrs[n] = sc;

						if (sc & BD_ENET_TX_DEF)
							ecp->txeacc.def++;
						if (sc & BD_ENET_TX_HB)
							ecp->txeacc.hb++;
						if (sc & BD_ENET_TX_LC)
							ecp->txeacc.lc++;
						if (sc & BD_ENET_TX_RL)
							ecp->txeacc.rl++;
						if (sc & BD_ENET_TX_RCMASK)
							ecp->txeacc.rc++;
						if (sc & BD_ENET_TX_UN)
							ecp->txeacc.un++;
						if (sc & BD_ENET_TX_CSL)
							ecp->txeacc.csl++;

						bdp->cbd_sc &= \
							~BD_ENET_TX_STATS;
					}

					if (ecp->state == Closing)
						ecp->clstime = get_timer (0);

					/* make it ready again */
					bdp->cbd_sc |= BD_ENET_TX_READY;
				}

				for (i = 0; i < ELBT_NRXBD; i++) {
					cbd_t *bdp = &ecp->rxbd[i];
					ushort sc = bdp->cbd_sc, mask;

					if ((sc & BD_ENET_RX_EMPTY) != 0)
						continue;

					/* we have a new frame in this buffer */
					ecp->nrcvd++;

					mask = BD_ENET_RX_LAST|BD_ENET_RX_FIRST;
					if ((sc & mask) != mask) {
						/* somethings wrong here ... */
						if (!(sc & BD_ENET_RX_LAST))
							ecp->rxeacc._l++;
						if (!(sc & BD_ENET_RX_FIRST))
							ecp->rxeacc._f++;
					}

					if (sc & BD_ENET_RX_ERRS) {
						ulong n;

						/*
						 * we had some sort of error
						 * on the frame
						 */
						n = ecp->nrxerr++;
						if (n < ELBT_MAXRXERR)
							ecp->rxerrs[n] = sc;

						if (sc & BD_ENET_RX_MISS)
							ecp->rxeacc.m++;
						if (sc & BD_ENET_RX_BC)
							ecp->rxeacc.bc++;
						if (sc & BD_ENET_RX_MC)
							ecp->rxeacc.mc++;
						if (sc & BD_ENET_RX_LG)
							ecp->rxeacc.lg++;
						if (sc & BD_ENET_RX_NO)
							ecp->rxeacc.no++;
						if (sc & BD_ENET_RX_SH)
							ecp->rxeacc.sh++;
						if (sc & BD_ENET_RX_CR)
							ecp->rxeacc.cr++;
						if (sc & BD_ENET_RX_OV)
							ecp->rxeacc.ov++;
						if (sc & BD_ENET_RX_CL)
							ecp->rxeacc.cl++;

						bdp->cbd_sc &= \
							~BD_ENET_RX_ERRS;
					}
					else {
						ushort datlen = bdp->cbd_datlen;
						Ethernet_t *ehp;
						ushort prot;
						int ours, tb, n, nbytes;

						ehp = (Ethernet_t *) \
							&ecp->rxbufs[i][0];

						ours = memcmp (ehp->et_src, \
							NetOurEther, 6);

						prot = swap16 (ehp->et_protlen);
						tb = prot & 0x8000;
						n = prot & 0x7fff;

						nbytes = ELBT_BUFSZ - \
							offsetof (Ethernet_t, \
								et_dsap) - \
							ELBT_CRCSZ;

						/* check the frame is correct */
						if (datlen != ELBT_BUFSZ)
							ecp->rxeacc.badlen++;
						else if (!ours)
							ecp->rxeacc.badsrc++;
						else if (!tb || n >= ELBT_NTXBD)
							ecp->rxeacc.badtyp++;
						else {
							ulong patword = \
								patwords[n];
							uint nbb;

							nbb = badbits ( \
								&ehp->et_dsap, \
								nbytes, \
								patword);

							ecp->rxeacc.badbit += \
								nbb;
						}
					}

					if (ecp->state == Closing)
					    ecp->clstime = get_timer (0);

					/* make it empty again */
					bdp->cbd_sc |= BD_ENET_RX_EMPTY;
				}

				if (ecp->state != Closing)
					break;

				/*
				 * (while Closing) check to see if
				 * waited long enough
				 */

				if (get_timer (ecp->clstime) >= ELBT_CLSWAIT) {
					/* write GFMR: disable tx/rx */
					fcp->fcc_gfmr &= \
						~(FCC_GFMR_ENT | FCC_GFMR_ENR);
					ecp->state = Closed;
				}

				break;

			case Closed:
				nclosed++;
				break;
			}
		}

	} while (nclosed < (FCC_END_LOOP - FCC_START_LOOP + 1));

	runtime = get_timer (runtime);
	if (runtime <= ELBT_CLSWAIT) {
		printf ("Whoops! somehow elapsed time (%ld) is wrong (<= %d)\n",
			runtime, ELBT_CLSWAIT);
		return;
	}
	nmsec = runtime - ELBT_CLSWAIT;

	printf ("Test Finished in %ldms (plus %dms close wait period)!\n\n",
		nmsec, ELBT_CLSWAIT);

	/*
	 * now print stats
	 */

	for (c = FCC_START_LOOP; c <= FCC_END_LOOP; c++) {
		elbt_chan *ecp = &elbt_chans[c];
		uint rxpps, txpps, nerr;

		rxpps = (ecp->nrcvd * 1000) / nmsec;
		txpps = (ecp->nsent * 1000) / nmsec;

		printf ("Channel %d: %d rcvd (%d pps, %d rxerrs), "
			"%d sent (%d pps, %d txerrs)\n\n", c,
			ecp->nrcvd, rxpps, ecp->nrxerr,
			ecp->nsent, txpps, ecp->ntxerr);

		if ((nerr = ecp->nrxerr) > 0) {
			ulong i;

			printf ("\tFirst %d rx errs:", nerr);
			for (i = 0; i < nerr; i++)
				printf (" %04x", ecp->rxerrs[i]);
			putc ('\n');
		}

		if ((nerr = ecp->ntxerr) > 0) {
			ulong i;

			printf ("\tFirst %d tx errs:", nerr);
			for (i = 0; i < nerr; i++)
				printf (" %04x", ecp->txerrs[i]);
			putc ('\n');
		}
	}

	puts ("Receive Error Counts:\n");
	for (c = FCC_START_LOOP; c <= FCC_END_LOOP; c++)
		bases[c] = (uchar *)&elbt_chans[c].rxeacc;
	print_desc (rxeacc_descs, rxeacc_ndesc, bases, 3);

	puts ("\nTransmit Error Counts:\n");
	for (c = FCC_START_LOOP; c <= FCC_END_LOOP; c++)
		bases[c] = (uchar *)&elbt_chans[c].txeacc;
	print_desc (txeacc_descs, txeacc_ndesc, bases, 3);

	puts ("\nRMON(-like) Counters:\n");
	for (c = FCC_START_LOOP; c <= FCC_END_LOOP; c++)
		bases[c] = (uchar *)&immr->im_dprambase[elbt_chans[c].proff];
	print_desc (epram_descs, epram_ndesc, bases, 3);
}

#endif /* CONFIG_ETHER_LOOPBACK_TEST */

#endif

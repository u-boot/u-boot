/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 *
 * Based in part on arch/powerpc/cpu/mpc8260/ether_scc.c.
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
#include <net.h>

#ifdef CONFIG_KEYMILE_HDLC_ENET

#include "../common/keymile_hdlc_enet.h"

char keymile_slot;	/* our slot number in the backplane */

/*
 * Since, except during initialization, ethact is always HDLC ETHERNET
 * while we're in the driver, just use serial_printf() everywhere for
 * output.  This avoids possible conflicts when netconsole is being
 * used.
 */
#define dprintf(fmt, args...)	serial_printf(fmt, ##args)

static int already_inited;

/*
  * SCC Ethernet Tx and Rx buffer descriptors allocated at the
  *  immr->udata_bd address on Dual-Port RAM
  * Provide for Double Buffering
  */
typedef volatile struct CommonBufferDescriptor {
    cbd_t txbd;			/* Tx BD */
    cbd_t rxbd[HDLC_PKTBUFSRX];	/* Rx BD */
} RTXBD;

static RTXBD *rtx;

int keymile_hdlc_enet_init(struct eth_device *, bd_t *);
void keymile_hdlc_enet_halt(struct eth_device *);
extern void keymile_hdlc_enet_init_bds(RTXBD *);
extern void initCachedNumbers(int);

/* Use SCC1 */
#define CPM_CR_SCC_PAGE		CPM_CR_SCC1_PAGE
#define CPM_CR_SCC_SBLOCK	CPM_CR_SCC1_SBLOCK
#define CMXSCR_MASK		(CMXSCR_GR1|CMXSCR_SC1|\
					CMXSCR_RS1CS_MSK|CMXSCR_TS1CS_MSK)
#define CMXSCR_VALUE		(CMXSCR_RS1CS_CLK11|CMXSCR_TS1CS_CLK11)
#define MGC_PROFF_HDLC	PROFF_SCC1
#define MGC_SCC_HDLC	0	/* Index, not number! */

int keymile_hdlc_enet_init(struct eth_device *dev, bd_t *bis)
{
	/* int i; */
	uint dpr;
	/* volatile cbd_t *bdp; */
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8260_t *cp = &(im->im_cpm);
	volatile scc_t *sccp;
	volatile scc_hdlc_t *hpr;
	volatile iop8260_t *iop;

	if (already_inited)
		return 0;

	hpr = (scc_hdlc_t *)(&im->im_dprambase[MGC_PROFF_HDLC]);
	sccp = (scc_t *)(&im->im_scc[MGC_SCC_HDLC]);
	iop = &im->im_ioport;

	/*
	 * Disable receive and transmit just in case.
	 */
	sccp->scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);

	/*
	 * Avoid exhausting DPRAM, which would cause a panic.
	 */
	if (rtx == NULL) {
		/* dpr is an offset into dpram */
		dpr = m8260_cpm_dpalloc(sizeof(RTXBD), 8);
		rtx = (RTXBD *)&im->im_dprambase[dpr];
	}

	/* We need the slot number for addressing. */
	keymile_slot = *(char *)(CONFIG_SYS_SLOT_ID_BASE +
		CONFIG_SYS_SLOT_ID_OFF) & CONFIG_SYS_SLOT_ID_MASK;
	/*
	 * Be consistent with the Linux driver and set
	 * only enetaddr[0].
	 *
	 * Always add 1 to the slot number so that
	 * there are no problems with an ethaddr which
	 * is all 0s.  This should be acceptable because
	 * a board should never have a slot number of 255,
	 * which is the broadcast address.  The HDLC addressing
	 * uses only the slot number.
	 */
	dev->enetaddr[0] = keymile_slot + 1;
#ifdef TEST_IT
	dprintf("slot %d\n", keymile_slot);
#endif

	/* use pd30, pd31 pins for TXD1, RXD1 respectively */
	iop->iop_ppard |= (0x80000000 >> 30) | (0x80000000 >> 31);
	iop->iop_pdird |= (0x80000000 >> 30);
	iop->iop_psord |= (0x80000000 >> 30);

	/* use pc21 as CLK11 */
	iop->iop_pparc |= (0x80000000 >> 21);
	iop->iop_pdirc &= ~(0x80000000 >> 21);
	iop->iop_psorc &= ~(0x80000000 >> 21);

	/* use pc15 as CTS1 */
	iop->iop_pparc |= (0x80000000 >> 15);
	iop->iop_pdirc &= ~(0x80000000 >> 15);
	iop->iop_psorc &= ~(0x80000000 >> 15);

	/*
	 * SI clock routing
	 * use CLK11
	 * this also connects SCC1 to NMSI
	 */
	im->im_cpmux.cmx_scr = (im->im_cpmux.cmx_scr & ~CMXSCR_MASK) |
		CMXSCR_VALUE;

	/* keymile_rxIdx = 0; */

	/*
	 * Initialize function code registers for big-endian.
	 */
	hpr->sh_genscc.scc_rfcr = CPMFCR_EB;
	hpr->sh_genscc.scc_tfcr = CPMFCR_EB;

	/*
	 * Set maximum bytes per receive buffer.
	 */
	hpr->sh_genscc.scc_mrblr = MAX_FRAME_LENGTH;

	/* Setup CRC generator values for HDLC */
	hpr->sh_cmask = 0x0000F0B8;
	hpr->sh_cpres = 0x0000FFFF;

	/* Initialize all error counters to 0 */
	hpr->sh_disfc = 0;
	hpr->sh_crcec = 0;
	hpr->sh_abtsc = 0;
	hpr->sh_nmarc = 0;
	hpr->sh_retrc = 0;

	/* Set maximum frame length size */
	hpr->sh_mflr = MAX_FRAME_LENGTH;

	/* set to 1 for per frame processing change later if needed */
	hpr->sh_rfthr = 1;

	hpr->sh_hmask = 0xff;

	hpr->sh_haddr2 = SET_HDLC_UUA(keymile_slot);
	hpr->sh_haddr3 = hpr->sh_haddr2;
	hpr->sh_haddr4 = hpr->sh_haddr2;
	/* broadcast */
	hpr->sh_haddr1 = HDLC_BCAST;

	hpr->sh_genscc.scc_rbase = (unsigned int) &rtx->rxbd[0];
	hpr->sh_genscc.scc_tbase = (unsigned int) &rtx->txbd;

#if 0
	/*
	 * Initialize the buffer descriptors.
	 */
	bdp = &rtx->txbd;
	bdp->cbd_sc = 0;
	bdp->cbd_bufaddr = 0;
	bdp->cbd_sc = BD_SC_WRAP;

	/*
	 *	Setup RX packet buffers, aligned correctly.
	 *	Borrowed from net/net.c.
	 */
	MyRxPackets[0] = &MyPktBuf[0] + (PKTALIGN - 1);
	MyRxPackets[0] -= (ulong)MyRxPackets[0] % PKTALIGN;
	for (i = 1; i < HDLC_PKTBUFSRX; i++)
		MyRxPackets[i] = MyRxPackets[0] + i * PKT_MAXBLR_SIZE;

	bdp = &rtx->rxbd[0];
	for (i = 0; i < HDLC_PKTBUFSRX; i++) {
		bdp->cbd_sc = BD_SC_EMPTY;
		/* Leave space at the start for INET header. */
		bdp->cbd_bufaddr = (unsigned int)(MyRxPackets[i] +
			INET_HDR_ALIGN);
		bdp++;
	}
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;
#else
	keymile_hdlc_enet_init_bds(rtx);
#endif

	/* Let's re-initialize the channel now.	 We have to do it later
	 * than the manual describes because we have just now finished
	 * the BD initialization.
	 */
	cp->cp_cpcr = mk_cr_cmd(CPM_CR_SCC_PAGE, CPM_CR_SCC_SBLOCK,
					0, CPM_CR_INIT_TRX) | CPM_CR_FLG;
	while (cp->cp_cpcr & CPM_CR_FLG);

	sccp->scc_gsmrl = SCC_GSMRL_MODE_HDLC;
	/* CTSS=1 */
	sccp->scc_gsmrh = SCC_GSMRH_CTSS;
	/* NOF=0, RTE=1, DRT=0, BUS=1 */
	sccp->scc_psmr = ((0x8000 >> 6) | (0x8000 >> 10));

/* loopback for local testing */
#ifdef GJTEST
	dprintf("LOOPBACK!\n");
	sccp->scc_gsmrl |= SCC_GSMRL_DIAG_LOOP;
#endif

	/*
	 * Disable all interrupts and clear all pending
	 * events.
	 */
	sccp->scc_sccm = 0;
	sccp->scc_scce = 0xffff;

	/*
	 * And last, enable the transmit and receive processing.
	 */
	sccp->scc_gsmrl |= (SCC_GSMRL_ENR | SCC_GSMRL_ENT);

	dprintf("%s: HDLC ENET Version 0.3 on SCC%d\n", dev->name,
		MGC_SCC_HDLC + 1);

	/*
	 * We may not get an ARP packet because ARP was already done on
	 * a different interface, so initialize the cached values now.
	 */
	initCachedNumbers(1);

	already_inited = 1;

	return 0;
}

void keymile_hdlc_enet_halt(struct eth_device *dev)
{
#if 0 /* just return, but keep this for reference */
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

	/* maybe should do a graceful stop here? */
	immr->im_scc[MGC_SCC_HDLC].scc_gsmrl &=
		~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);
#endif
}

#endif /* CONFIG_MGCOGE_HDLC_ENET */

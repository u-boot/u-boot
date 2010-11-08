/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 *
 * Based in part on arch/powerpc/cpu/mpc8xx/scc.c.
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

#include <common.h>	/* commproc.h is included here */
#include <malloc.h>
#include <net.h>

#ifdef CONFIG_KEYMILE_HDLC_ENET

#include "../common/keymile_hdlc_enet.h"

char keymile_slot;	/* our slot number in the backplane */

/*
 * Since, except during initialization, ethact is always HDLC
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

/* Use SCC4 */
#define MGS_CPM_CR_HDLC	CPM_CR_CH_SCC4
#define MGS_PROFF_HDLC	PROFF_SCC4
#define MGS_SCC_HDLC	3	/* Index, not number! */

int keymile_hdlc_enet_init(struct eth_device *dev, bd_t *bis)
{
	/* int i; */
	/* volatile cbd_t *bdp; */
	volatile cpm8xx_t *cp;
	volatile scc_t *sccp;
	volatile hdlc_pram_t *hpr;
	volatile iop8xx_t *iop;

	if (already_inited)
		return 0;

	cp = (cpm8xx_t *)&(((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm);
	hpr = (hdlc_pram_t *)(&cp->cp_dparam[MGS_PROFF_HDLC]);
	sccp = (volatile scc_t *)(&cp->cp_scc[MGS_SCC_HDLC]);
	iop = (iop8xx_t *)&(((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport);

	/*
	 * Disable receive and transmit just in case.
	 */
	sccp->scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);

#ifndef CONFIG_SYS_ALLOC_DPRAM
#error "CONFIG_SYS_ALLOC_DPRAM must be defined"
#else
	/*
	 * Avoid exhausting DPRAM, which would cause a panic.
	 * Actually this isn't really necessary, but leave it here
	 * for safety's sake.
	 */
	if (rtx == NULL) {
		rtx = (RTXBD *) (cp->cp_dpmem +
			 dpram_alloc_align(sizeof(RTXBD), 8));
		if (rtx == (RTXBD *)CPM_DP_NOSPACE)
			return -1;
		memset((void *)rtx, 0, sizeof(RTXBD));
	}
#endif /* !CONFIG_SYS_ALLOC_DPRAM */

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

	/* use pa8, pa9 pins for TXD4, RXD4 respectively */
	iop->iop_papar |= ((0x8000 >> 8) | (0x8000 >> 9));
	iop->iop_padir &= ~((0x8000 >> 8) | (0x8000 >> 9));
	iop->iop_paodr &= ~((0x8000 >> 8) | (0x8000 >> 9));

	/* also use pa0 as CLK8 */
	iop->iop_papar |= 0x8000;
	iop->iop_padir &= ~0x8000;
	iop->iop_paodr &= ~0x8000;

	/* use pc5 as CTS4 */
	iop->iop_pcpar &= ~(0x8000 >> 5);
	iop->iop_pcdir &= ~(0x8000 >> 5);
	iop->iop_pcso  |= (0x8000 >> 5);

	/*
	 * SI clock routing
	 * use CLK8
	 * this also connects SCC4 to NMSI
	 */
	cp->cp_sicr = (cp->cp_sicr & ~0xff000000) | 0x3f000000;

	/* keymile_rxIdx = 0; */

	/*
	 * Initialize function code registers for big-endian.
	 */
	hpr->rfcr = SCC_EB;
	hpr->tfcr = SCC_EB;

	/*
	 * Set maximum bytes per receive buffer.
	 */
	hpr->mrblr = MAX_FRAME_LENGTH;

	/* Setup CRC generator values for HDLC */
	hpr->c_mask = 0x0000F0B8;
	hpr->c_pres = 0x0000FFFF;

	/* Initialize all error counters to 0 */
	hpr->disfc = 0;
	hpr->crcec = 0;
	hpr->abtsc = 0;
	hpr->nmarc = 0;
	hpr->retrc = 0;

	/* Set maximum frame length size */
	hpr->mflr = MAX_FRAME_LENGTH;

	/* set to 1 for per frame processing change later if needed */
	hpr->rfthr = 1;

	hpr->hmask = 0xff;

	hpr->haddr2 = SET_HDLC_UUA(keymile_slot);
	hpr->haddr3 = hpr->haddr2;
	hpr->haddr4 = hpr->haddr2;
	/* broadcast */
	hpr->haddr1 = HDLC_BCAST;

	hpr->rbase = (unsigned int) &rtx->rxbd[0];
	hpr->tbase = (unsigned int) &rtx->txbd;

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
	cp->cp_cpcr = mk_cr_cmd(MGS_CPM_CR_HDLC, CPM_CR_INIT_TRX) | CPM_CR_FLG;
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
		MGS_SCC_HDLC + 1);

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
	immr->im_cpm.cp_scc[MGS_SCC_HDLC].scc_gsmrl &=
		~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);
#endif
}

#endif /* CONFIG_KEYMILE_HDLC_ENET */

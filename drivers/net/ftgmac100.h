/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Faraday FTGMAC100 Ethernet
 *
 * (C) Copyright 2010 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * (C) Copyright 2010 Andes Technology
 * Macpaul Lin <macpaul@andestech.com>
 */

#ifndef __FTGMAC100_H
#define __FTGMAC100_H

/* The registers offset table of ftgmac100 */
struct ftgmac100 {
	unsigned int	isr;		/* 0x00 */
	unsigned int	ier;		/* 0x04 */
	unsigned int	mac_madr;	/* 0x08 */
	unsigned int	mac_ladr;	/* 0x0c */
	unsigned int	maht0;		/* 0x10 */
	unsigned int	maht1;		/* 0x14 */
	unsigned int	txpd;		/* 0x18 */
	unsigned int	rxpd;		/* 0x1c */
	unsigned int	txr_badr;	/* 0x20 */
	unsigned int	rxr_badr;	/* 0x24 */
	unsigned int	hptxpd;		/* 0x28 */
	unsigned int	hptxpd_badr;	/* 0x2c */
	unsigned int	itc;		/* 0x30 */
	unsigned int	aptc;		/* 0x34 */
	unsigned int	dblac;		/* 0x38 */
	unsigned int	dmafifos;	/* 0x3c */
	unsigned int	revr;		/* 0x40 */
	unsigned int	fear;		/* 0x44 */
	unsigned int	tpafcr;		/* 0x48 */
	unsigned int	rbsr;		/* 0x4c */
	unsigned int	maccr;		/* 0x50 */
	unsigned int	macsr;		/* 0x54 */
	unsigned int	tm;		/* 0x58 */
	unsigned int	resv1;		/* 0x5c */ /* not defined in spec */
	unsigned int	phycr;		/* 0x60 */
	unsigned int	phydata;	/* 0x64 */
	unsigned int	fcr;		/* 0x68 */
	unsigned int	bpr;		/* 0x6c */
	unsigned int	wolcr;		/* 0x70 */
	unsigned int	wolsr;		/* 0x74 */
	unsigned int	wfcrc;		/* 0x78 */
	unsigned int	resv2;		/* 0x7c */ /* not defined in spec */
	unsigned int	wfbm1;		/* 0x80 */
	unsigned int	wfbm2;		/* 0x84 */
	unsigned int	wfbm3;		/* 0x88 */
	unsigned int	wfbm4;		/* 0x8c */
	unsigned int	nptxr_ptr;	/* 0x90 */
	unsigned int	hptxr_ptr;	/* 0x94 */
	unsigned int	rxr_ptr;	/* 0x98 */
	unsigned int	resv3;		/* 0x9c */ /* not defined in spec */
	unsigned int	tx;		/* 0xa0 */
	unsigned int	tx_mcol_scol;	/* 0xa4 */
	unsigned int	tx_ecol_fail;	/* 0xa8 */
	unsigned int	tx_lcol_und;	/* 0xac */
	unsigned int	rx;		/* 0xb0 */
	unsigned int	rx_bc;		/* 0xb4 */
	unsigned int	rx_mc;		/* 0xb8 */
	unsigned int	rx_pf_aep;	/* 0xbc */
	unsigned int	rx_runt;	/* 0xc0 */
	unsigned int	rx_crcer_ftl;	/* 0xc4 */
	unsigned int	rx_col_lost;	/* 0xc8 */
};

/*
 * Interrupt status register & interrupt enable register
 */
#define FTGMAC100_INT_RPKT_BUF		(1 << 0)
#define FTGMAC100_INT_RPKT_FIFO		(1 << 1)
#define FTGMAC100_INT_NO_RXBUF		(1 << 2)
#define FTGMAC100_INT_RPKT_LOST		(1 << 3)
#define FTGMAC100_INT_XPKT_ETH		(1 << 4)
#define FTGMAC100_INT_XPKT_FIFO		(1 << 5)
#define FTGMAC100_INT_NO_NPTXBUF	(1 << 6)
#define FTGMAC100_INT_XPKT_LOST		(1 << 7)
#define FTGMAC100_INT_AHB_ERR		(1 << 8)
#define FTGMAC100_INT_PHYSTS_CHG	(1 << 9)
#define FTGMAC100_INT_NO_HPTXBUF	(1 << 10)

/*
 * Interrupt timer control register
 */
#define FTGMAC100_ITC_RXINT_CNT(x)	(((x) & 0xf) << 0)
#define FTGMAC100_ITC_RXINT_THR(x)	(((x) & 0x7) << 4)
#define FTGMAC100_ITC_RXINT_TIME_SEL	(1 << 7)
#define FTGMAC100_ITC_TXINT_CNT(x)	(((x) & 0xf) << 8)
#define FTGMAC100_ITC_TXINT_THR(x)	(((x) & 0x7) << 12)
#define FTGMAC100_ITC_TXINT_TIME_SEL	(1 << 15)

/*
 * Automatic polling timer control register
 */
#define FTGMAC100_APTC_RXPOLL_CNT(x)	(((x) & 0xf) << 0)
#define FTGMAC100_APTC_RXPOLL_TIME_SEL	(1 << 4)
#define FTGMAC100_APTC_TXPOLL_CNT(x)	(((x) & 0xf) << 8)
#define FTGMAC100_APTC_TXPOLL_TIME_SEL	(1 << 12)

/*
 * DMA burst length and arbitration control register
 */
#define FTGMAC100_DBLAC_RXFIFO_LTHR(x)	(((x) & 0x7) << 0)
#define FTGMAC100_DBLAC_RXFIFO_HTHR(x)	(((x) & 0x7) << 3)
#define FTGMAC100_DBLAC_RX_THR_EN	(1 << 6)
#define FTGMAC100_DBLAC_RXBURST_SIZE(x)	(((x) & 0x3) << 8)
#define FTGMAC100_DBLAC_TXBURST_SIZE(x)	(((x) & 0x3) << 10)
#define FTGMAC100_DBLAC_RXDES_SIZE(x)	(((x) & 0xf) << 12)
#define FTGMAC100_DBLAC_TXDES_SIZE(x)	(((x) & 0xf) << 16)
#define FTGMAC100_DBLAC_IFG_CNT(x)	(((x) & 0x7) << 20)
#define FTGMAC100_DBLAC_IFG_INC		(1 << 23)

/*
 * DMA FIFO status register
 */
#define FTGMAC100_DMAFIFOS_RXDMA1_SM(dmafifos)	((dmafifos) & 0xf)
#define FTGMAC100_DMAFIFOS_RXDMA2_SM(dmafifos)	(((dmafifos) >> 4) & 0xf)
#define FTGMAC100_DMAFIFOS_RXDMA3_SM(dmafifos)	(((dmafifos) >> 8) & 0x7)
#define FTGMAC100_DMAFIFOS_TXDMA1_SM(dmafifos)	(((dmafifos) >> 12) & 0xf)
#define FTGMAC100_DMAFIFOS_TXDMA2_SM(dmafifos)	(((dmafifos) >> 16) & 0x3)
#define FTGMAC100_DMAFIFOS_TXDMA3_SM(dmafifos)	(((dmafifos) >> 18) & 0xf)
#define FTGMAC100_DMAFIFOS_RXFIFO_EMPTY		(1 << 26)
#define FTGMAC100_DMAFIFOS_TXFIFO_EMPTY		(1 << 27)
#define FTGMAC100_DMAFIFOS_RXDMA_GRANT		(1 << 28)
#define FTGMAC100_DMAFIFOS_TXDMA_GRANT		(1 << 29)
#define FTGMAC100_DMAFIFOS_RXDMA_REQ		(1 << 30)
#define FTGMAC100_DMAFIFOS_TXDMA_REQ		(1 << 31)

/*
 * Receive buffer size register
 */
#define FTGMAC100_RBSR_SIZE(x)		((x) & 0x3fff)

/*
 * MAC control register
 */
#define FTGMAC100_MACCR_TXDMA_EN	(1 << 0)
#define FTGMAC100_MACCR_RXDMA_EN	(1 << 1)
#define FTGMAC100_MACCR_TXMAC_EN	(1 << 2)
#define FTGMAC100_MACCR_RXMAC_EN	(1 << 3)
#define FTGMAC100_MACCR_RM_VLAN		(1 << 4)
#define FTGMAC100_MACCR_HPTXR_EN	(1 << 5)
#define FTGMAC100_MACCR_LOOP_EN		(1 << 6)
#define FTGMAC100_MACCR_ENRX_IN_HALFTX	(1 << 7)
#define FTGMAC100_MACCR_FULLDUP		(1 << 8)
#define FTGMAC100_MACCR_GIGA_MODE	(1 << 9)
#define FTGMAC100_MACCR_CRC_APD		(1 << 10)
#define FTGMAC100_MACCR_RX_RUNT		(1 << 12)
#define FTGMAC100_MACCR_JUMBO_LF	(1 << 13)
#define FTGMAC100_MACCR_RX_ALL		(1 << 14)
#define FTGMAC100_MACCR_HT_MULTI_EN	(1 << 15)
#define FTGMAC100_MACCR_RX_MULTIPKT	(1 << 16)
#define FTGMAC100_MACCR_RX_BROADPKT	(1 << 17)
#define FTGMAC100_MACCR_DISCARD_CRCERR	(1 << 18)
#define FTGMAC100_MACCR_FAST_MODE	(1 << 19)
#define FTGMAC100_MACCR_SW_RST		(1 << 31)

/*
 * PHY control register
 */
#define FTGMAC100_PHYCR_MDC_CYCTHR_MASK	0x3f
#define FTGMAC100_PHYCR_MDC_CYCTHR(x)	((x) & 0x3f)
#define FTGMAC100_PHYCR_PHYAD(x)	(((x) & 0x1f) << 16)
#define FTGMAC100_PHYCR_REGAD(x)	(((x) & 0x1f) << 21)
#define FTGMAC100_PHYCR_MIIRD		(1 << 26)
#define FTGMAC100_PHYCR_MIIWR		(1 << 27)

/*
 * PHY data register
 */
#define FTGMAC100_PHYDATA_MIIWDATA(x)		((x) & 0xffff)
#define FTGMAC100_PHYDATA_MIIRDATA(phydata)	(((phydata) >> 16) & 0xffff)

/*
 * Transmit descriptor, aligned to 16 bytes
 */
struct ftgmac100_txdes {
	unsigned int	txdes0;
	unsigned int	txdes1;
	unsigned int	txdes2;	/* not used by HW */
	unsigned int	txdes3;	/* TXBUF_BADR */
} __attribute__ ((aligned(16)));

#define FTGMAC100_TXDES0_TXBUF_SIZE(x)	((x) & 0x3fff)
#define FTGMAC100_TXDES0_EDOTR		(1 << 15)
#define FTGMAC100_TXDES0_CRC_ERR	(1 << 19)
#define FTGMAC100_TXDES0_LTS		(1 << 28)
#define FTGMAC100_TXDES0_FTS		(1 << 29)
#define FTGMAC100_TXDES0_TXDMA_OWN	(1 << 31)

#define FTGMAC100_TXDES1_VLANTAG_CI(x)	((x) & 0xffff)
#define FTGMAC100_TXDES1_INS_VLANTAG	(1 << 16)
#define FTGMAC100_TXDES1_TCP_CHKSUM	(1 << 17)
#define FTGMAC100_TXDES1_UDP_CHKSUM	(1 << 18)
#define FTGMAC100_TXDES1_IP_CHKSUM	(1 << 19)
#define FTGMAC100_TXDES1_LLC		(1 << 22)
#define FTGMAC100_TXDES1_TX2FIC		(1 << 30)
#define FTGMAC100_TXDES1_TXIC		(1 << 31)

/*
 * Receive descriptor, aligned to 16 bytes
 */
struct ftgmac100_rxdes {
	unsigned int	rxdes0;
	unsigned int	rxdes1;
	unsigned int	rxdes2;	/* not used by HW */
	unsigned int	rxdes3;	/* RXBUF_BADR */
} __attribute__ ((aligned(16)));

#define FTGMAC100_RXDES0_VDBC(x)	((x) & 0x3fff)
#define FTGMAC100_RXDES0_EDORR		(1 << 15)
#define FTGMAC100_RXDES0_MULTICAST	(1 << 16)
#define FTGMAC100_RXDES0_BROADCAST	(1 << 17)
#define FTGMAC100_RXDES0_RX_ERR		(1 << 18)
#define FTGMAC100_RXDES0_CRC_ERR	(1 << 19)
#define FTGMAC100_RXDES0_FTL		(1 << 20)
#define FTGMAC100_RXDES0_RUNT		(1 << 21)
#define FTGMAC100_RXDES0_RX_ODD_NB	(1 << 22)
#define FTGMAC100_RXDES0_FIFO_FULL	(1 << 23)
#define FTGMAC100_RXDES0_PAUSE_OPCODE	(1 << 24)
#define FTGMAC100_RXDES0_PAUSE_FRAME	(1 << 25)
#define FTGMAC100_RXDES0_LRS		(1 << 28)
#define FTGMAC100_RXDES0_FRS		(1 << 29)
#define FTGMAC100_RXDES0_RXPKT_RDY	(1 << 31)

#define FTGMAC100_RXDES1_VLANTAG_CI	0xffff
#define FTGMAC100_RXDES1_PROT_MASK	(0x3 << 20)
#define FTGMAC100_RXDES1_PROT_NONIP	(0x0 << 20)
#define FTGMAC100_RXDES1_PROT_IP	(0x1 << 20)
#define FTGMAC100_RXDES1_PROT_TCPIP	(0x2 << 20)
#define FTGMAC100_RXDES1_PROT_UDPIP	(0x3 << 20)
#define FTGMAC100_RXDES1_LLC		(1 << 22)
#define FTGMAC100_RXDES1_DF		(1 << 23)
#define FTGMAC100_RXDES1_VLANTAG_AVAIL	(1 << 24)
#define FTGMAC100_RXDES1_TCP_CHKSUM_ERR	(1 << 25)
#define FTGMAC100_RXDES1_UDP_CHKSUM_ERR	(1 << 26)
#define FTGMAC100_RXDES1_IP_CHKSUM_ERR	(1 << 27)

#endif /* __FTGMAC100_H */

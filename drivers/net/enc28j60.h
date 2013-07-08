/*
 * (X) extracted from enc28j60.c
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _enc28j60_h
#define _enc28j60_h

/*
 * SPI Commands
 *
 * Bits 7-5: Command
 * Bits 4-0: Register
 */
#define CMD_RCR(x)	(0x00+((x)&0x1f))	/* Read Control Register */
#define CMD_RBM		0x3a			/* Read Buffer Memory */
#define CMD_WCR(x)	(0x40+((x)&0x1f))	/* Write Control Register */
#define CMD_WBM		0x7a			/* Write Buffer Memory */
#define CMD_BFS(x)	(0x80+((x)&0x1f))	/* Bit Field Set */
#define CMD_BFC(x)	(0xa0+((x)&0x1f))	/* Bit Field Clear */
#define CMD_SRC		0xff			/* System Reset Command */

/* NEW: encode (bank number+1) in upper byte */

/* Common Control Registers accessible in all Banks */
#define CTL_REG_EIE		0x01B
#define CTL_REG_EIR		0x01C
#define CTL_REG_ESTAT		0x01D
#define CTL_REG_ECON2		0x01E
#define CTL_REG_ECON1		0x01F

/* Control Registers accessible in Bank 0 */
#define CTL_REG_ERDPTL		0x100
#define CTL_REG_ERDPTH		0x101
#define CTL_REG_EWRPTL		0x102
#define CTL_REG_EWRPTH		0x103
#define CTL_REG_ETXSTL		0x104
#define CTL_REG_ETXSTH		0x105
#define CTL_REG_ETXNDL		0x106
#define CTL_REG_ETXNDH		0x107
#define CTL_REG_ERXSTL		0x108
#define CTL_REG_ERXSTH		0x109
#define CTL_REG_ERXNDL		0x10A
#define CTL_REG_ERXNDH		0x10B
#define CTL_REG_ERXRDPTL	0x10C
#define CTL_REG_ERXRDPTH	0x10D
#define CTL_REG_ERXWRPTL	0x10E
#define CTL_REG_ERXWRPTH	0x10F
#define CTL_REG_EDMASTL		0x110
#define CTL_REG_EDMASTH		0x111
#define CTL_REG_EDMANDL		0x112
#define CTL_REG_EDMANDH		0x113
#define CTL_REG_EDMADSTL	0x114
#define CTL_REG_EDMADSTH	0x115
#define CTL_REG_EDMACSL		0x116
#define CTL_REG_EDMACSH		0x117

/* Control Registers accessible in Bank 1 */
#define CTL_REG_EHT0		0x200
#define CTL_REG_EHT1		0x201
#define CTL_REG_EHT2		0x202
#define CTL_REG_EHT3		0x203
#define CTL_REG_EHT4		0x204
#define CTL_REG_EHT5		0x205
#define CTL_REG_EHT6		0x206
#define CTL_REG_EHT7		0x207
#define CTL_REG_EPMM0		0x208
#define CTL_REG_EPMM1		0x209
#define CTL_REG_EPMM2		0x20A
#define CTL_REG_EPMM3		0x20B
#define CTL_REG_EPMM4		0x20C
#define CTL_REG_EPMM5		0x20D
#define CTL_REG_EPMM6		0x20E
#define CTL_REG_EPMM7		0x20F
#define CTL_REG_EPMCSL		0x210
#define CTL_REG_EPMCSH		0x211
#define CTL_REG_EPMOL		0x214
#define CTL_REG_EPMOH		0x215
#define CTL_REG_EWOLIE		0x216
#define CTL_REG_EWOLIR		0x217
#define CTL_REG_ERXFCON		0x218
#define CTL_REG_EPKTCNT		0x219

/* Control Registers accessible in Bank 2 */
#define CTL_REG_MACON1		0x300
#define CTL_REG_MACON2		0x301
#define CTL_REG_MACON3		0x302
#define CTL_REG_MACON4		0x303
#define CTL_REG_MABBIPG		0x304
#define CTL_REG_MAIPGL		0x306
#define CTL_REG_MAIPGH		0x307
#define CTL_REG_MACLCON1	0x308
#define CTL_REG_MACLCON2	0x309
#define CTL_REG_MAMXFLL		0x30A
#define CTL_REG_MAMXFLH		0x30B
#define CTL_REG_MAPHSUP		0x30D
#define CTL_REG_MICON		0x311
#define CTL_REG_MICMD		0x312
#define CTL_REG_MIREGADR	0x314
#define CTL_REG_MIWRL		0x316
#define CTL_REG_MIWRH		0x317
#define CTL_REG_MIRDL		0x318
#define CTL_REG_MIRDH		0x319

/* Control Registers accessible in Bank 3 */
#define CTL_REG_MAADR1		0x400
#define CTL_REG_MAADR0		0x401
#define CTL_REG_MAADR3		0x402
#define CTL_REG_MAADR2		0x403
#define CTL_REG_MAADR5		0x404
#define CTL_REG_MAADR4		0x405
#define CTL_REG_EBSTSD		0x406
#define CTL_REG_EBSTCON		0x407
#define CTL_REG_EBSTCSL		0x408
#define CTL_REG_EBSTCSH		0x409
#define CTL_REG_MISTAT		0x40A
#define CTL_REG_EREVID		0x412
#define CTL_REG_ECOCON		0x415
#define CTL_REG_EFLOCON		0x417
#define CTL_REG_EPAUSL		0x418
#define CTL_REG_EPAUSH		0x419

/* PHY Register */
#define PHY_REG_PHCON1		0x00
#define PHY_REG_PHSTAT1		0x01
#define PHY_REG_PHID1		0x02
#define PHY_REG_PHID2		0x03
#define PHY_REG_PHCON2		0x10
#define PHY_REG_PHSTAT2		0x11
#define PHY_REG_PHLCON		0x14

/* Receive Filter Register (ERXFCON) bits */
#define ENC_RFR_UCEN		0x80
#define ENC_RFR_ANDOR		0x40
#define ENC_RFR_CRCEN		0x20
#define ENC_RFR_PMEN		0x10
#define ENC_RFR_MPEN		0x08
#define ENC_RFR_HTEN		0x04
#define ENC_RFR_MCEN		0x02
#define ENC_RFR_BCEN		0x01

/* ECON1 Register Bits */
#define ENC_ECON1_TXRST		0x80
#define ENC_ECON1_RXRST		0x40
#define ENC_ECON1_DMAST		0x20
#define ENC_ECON1_CSUMEN	0x10
#define ENC_ECON1_TXRTS		0x08
#define ENC_ECON1_RXEN		0x04
#define ENC_ECON1_BSEL1		0x02
#define ENC_ECON1_BSEL0		0x01

/* ECON2 Register Bits */
#define ENC_ECON2_AUTOINC	0x80
#define ENC_ECON2_PKTDEC	0x40
#define ENC_ECON2_PWRSV		0x20
#define ENC_ECON2_VRPS		0x08

/* EIR Register Bits */
#define ENC_EIR_PKTIF		0x40
#define ENC_EIR_DMAIF		0x20
#define ENC_EIR_LINKIF		0x10
#define ENC_EIR_TXIF		0x08
#define ENC_EIR_WOLIF		0x04
#define ENC_EIR_TXERIF		0x02
#define ENC_EIR_RXERIF		0x01

/* ESTAT Register Bits */
#define ENC_ESTAT_INT		0x80
#define ENC_ESTAT_LATECOL	0x10
#define ENC_ESTAT_RXBUSY	0x04
#define ENC_ESTAT_TXABRT	0x02
#define ENC_ESTAT_CLKRDY	0x01

/* EIE Register Bits */
#define ENC_EIE_INTIE		0x80
#define ENC_EIE_PKTIE		0x40
#define ENC_EIE_DMAIE		0x20
#define ENC_EIE_LINKIE		0x10
#define ENC_EIE_TXIE		0x08
#define ENC_EIE_WOLIE		0x04
#define ENC_EIE_TXERIE		0x02
#define ENC_EIE_RXERIE		0x01

/* MACON1 Register Bits */
#define ENC_MACON1_LOOPBK	0x10
#define ENC_MACON1_TXPAUS	0x08
#define ENC_MACON1_RXPAUS	0x04
#define ENC_MACON1_PASSALL	0x02
#define ENC_MACON1_MARXEN	0x01

/* MACON2 Register Bits */
#define ENC_MACON2_MARST	0x80
#define ENC_MACON2_RNDRST	0x40
#define ENC_MACON2_MARXRST	0x08
#define ENC_MACON2_RFUNRST	0x04
#define ENC_MACON2_MATXRST	0x02
#define ENC_MACON2_TFUNRST	0x01

/* MACON3 Register Bits */
#define ENC_MACON3_PADCFG2	0x80
#define ENC_MACON3_PADCFG1	0x40
#define ENC_MACON3_PADCFG0	0x20
#define ENC_MACON3_TXCRCEN	0x10
#define ENC_MACON3_PHDRLEN	0x08
#define ENC_MACON3_HFRMEN	0x04
#define ENC_MACON3_FRMLNEN	0x02
#define ENC_MACON3_FULDPX	0x01

/* MACON4 Register Bits */
#define ENC_MACON4_DEFER	0x40

/* MICMD Register Bits */
#define ENC_MICMD_MIISCAN	0x02
#define ENC_MICMD_MIIRD		0x01

/* MISTAT Register Bits */
#define ENC_MISTAT_NVALID	0x04
#define ENC_MISTAT_SCAN		0x02
#define ENC_MISTAT_BUSY		0x01

/* PHID1 and PHID2 values */
#define ENC_PHID1_VALUE		0x0083
#define ENC_PHID2_VALUE		0x1400
#define ENC_PHID2_MASK		0xFC00

/* PHCON1 values */
#define	ENC_PHCON1_PDPXMD	0x0100

/* PHSTAT1 values */
#define	ENC_PHSTAT1_LLSTAT	0x0004

/* PHSTAT2 values */
#define	ENC_PHSTAT2_LSTAT	0x0400
#define	ENC_PHSTAT2_DPXSTAT	0x0200

#endif

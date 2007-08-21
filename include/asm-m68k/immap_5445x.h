/*
 * MCF5445x Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#ifndef __IMMAP_5445X__
#define __IMMAP_5445X__

/* Module Base Addresses */
#define MMAP_SCM1	0xFC000000
#define MMAP_XBS	0xFC004000
#define MMAP_FBCS	0xFC008000
#define MMAP_FEC0	0xFC030000
#define MMAP_FEC1	0xFC034000
#define MMAP_RTC	0xFC03C000
#define MMAP_EDMA	0xFC044000
#define MMAP_INTC0	0xFC048000
#define MMAP_INTC1	0xFC04C000
#define MMAP_IACK	0xFC054000
#define MMAP_I2C	0xFC058000
#define MMAP_DSPI	0xFC05C000
#define MMAP_UART0	0xFC060000
#define MMAP_UART1	0xFC064000
#define MMAP_UART2	0xFC068000
#define MMAP_DTMR0	0xFC070000
#define MMAP_DTMR1	0xFC074000
#define MMAP_DTMR2	0xFC078000
#define MMAP_DTMR3	0xFC07C000
#define MMAP_PIT0	0xFC080000
#define MMAP_PIT1	0xFC084000
#define MMAP_PIT2	0xFC088000
#define MMAP_PIT3	0xFC08C000
#define MMAP_EPORT	0xFC094000
#define MMAP_WTM	0xFC098000
#define MMAP_SBF	0xFC0A0000
#define MMAP_RCM	0xFC0A0000
#define MMAP_CCM	0xFC0A0000
#define MMAP_GPIO	0xFC0A4000
#define MMAP_PCI	0xFC0A8000
#define MMAP_PCIARB	0xFC0AC000
#define MMAP_RNG	0xFC0B4000
#define MMAP_SDRAM	0xFC0B8000
#define MMAP_SSI	0xFC0BC000
#define MMAP_PLL	0xFC0C4000
#define MMAP_ATA	0x90000000

/*********************************************************************
* ATA
*********************************************************************/

typedef struct atac {
	/* PIO */
	u8 toff;		/* 0x00 */
	u8 ton;			/* 0x01 */
	u8 t1;			/* 0x02 */
	u8 t2w;			/* 0x03 */
	u8 t2r;			/* 0x04 */
	u8 ta;			/* 0x05 */
	u8 trd;			/* 0x06 */
	u8 t4;			/* 0x07 */
	u8 t9;			/* 0x08 */

	/* DMA */
	u8 tm;			/* 0x09 */
	u8 tn;			/* 0x0A */
	u8 td;			/* 0x0B */
	u8 tk;			/* 0x0C */
	u8 tack;		/* 0x0D */
	u8 tenv;		/* 0x0E */
	u8 trp;			/* 0x0F */
	u8 tzah;		/* 0x10 */
	u8 tmli;		/* 0x11 */
	u8 tdvh;		/* 0x12 */
	u8 tdzfs;		/* 0x13 */
	u8 tdvs;		/* 0x14 */
	u8 tcvh;		/* 0x15 */
	u8 tss;			/* 0x16 */
	u8 tcyc;		/* 0x17 */

	/* FIFO */
	u32 fifo32;		/* 0x18 */
	u16 fifo16;		/* 0x1C */
	u8 rsvd0[2];
	u8 ffill;		/* 0x20 */
	u8 rsvd1[3];

	/* ATA */
	u8 cr;			/* 0x24 */
	u8 rsvd2[3];
	u8 isr;			/* 0x28 */
	u8 rsvd3[3];
	u8 ier;			/* 0x2C */
	u8 rsvd4[3];
	u8 icr;			/* 0x30 */
	u8 rsvd5[3];
	u8 falarm;		/* 0x34 */
	u8 rsvd6[106];
} atac_t;

/*********************************************************************
* Cross-bar switch (XBS)
*********************************************************************/

typedef struct xbs {
	u8 resv0[0x100];
	u32 prs1;		/* XBS Priority Register */
	u8 resv1[0xC];
	u32 crs1;		/* XBS Control Register */
	u8 resv2[0xEC];
	u32 prs2;		/* XBS Priority Register */
	u8 resv3[0xC];
	u32 crs2;		/* XBS Control Register */
	u8 resv4[0xEC];
	u32 prs3;		/* XBS Priority Register */
	u8 resv5[0xC];
	u32 crs3;		/* XBS Control Register */
	u8 resv6[0xEC];
	u32 prs4;		/* XBS Priority Register */
	u8 resv7[0xC];
	u32 crs4;		/* XBS Control Register */
	u8 resv8[0xEC];
	u32 prs5;		/* XBS Priority Register */
	u8 resv9[0xC];
	u32 crs5;		/* XBS Control Register */
	u8 resv10[0xEC];
	u32 prs6;		/* XBS Priority Register */
	u8 resv11[0xC];
	u32 crs6;		/* XBS Control Register */
	u8 resv12[0xEC];
	u32 prs7;		/* XBS Priority Register */
	u8 resv13[0xC];
	u32 crs7;		/* XBS Control Register */
} xbs_t;

/*********************************************************************
* FlexBus Chip Selects (FBCS)
*********************************************************************/

typedef struct fbcs {
	u32 csar0;		/* Chip-select Address Register */
	u32 csmr0;		/* Chip-select Mask Register */
	u32 cscr0;		/* Chip-select Control Register */
	u32 csar1;		/* Chip-select Address Register */
	u32 csmr1;		/* Chip-select Mask Register */
	u32 cscr1;		/* Chip-select Control Register */
	u32 csar2;		/* Chip-select Address Register */
	u32 csmr2;		/* Chip-select Mask Register */
	u32 cscr2;		/* Chip-select Control Register */
	u32 csar3;		/* Chip-select Address Register */
	u32 csmr3;		/* Chip-select Mask Register */
	u32 cscr3;		/* Chip-select Control Register */
} fbcs_t;

/*********************************************************************
* Enhanced DMA (EDMA)
*********************************************************************/

typedef struct edma {
	u32 cr;
	u32 es;
	u8 resv0[0x6];
	u16 erq;
	u8 resv1[0x6];
	u16 eei;
	u8 serq;
	u8 cerq;
	u8 seei;
	u8 ceei;
	u8 cint;
	u8 cerr;
	u8 ssrt;
	u8 cdne;
	u8 resv2[0x6];
	u16 intr;
	u8 resv3[0x6];
	u16 err;
	u8 resv4[0xD0];
	u8 dchpri0;
	u8 dchpri1;
	u8 dchpri2;
	u8 dchpri3;
	u8 dchpri4;
	u8 dchpri5;
	u8 dchpri6;
	u8 dchpri7;
	u8 dchpri8;
	u8 dchpri9;
	u8 dchpri10;
	u8 dchpri11;
	u8 dchpri12;
	u8 dchpri13;
	u8 dchpri14;
	u8 dchpri15;
	u8 resv5[0xEF0];
	u32 tcd0_saddr;
	u16 tcd0_attr;
	u16 tcd0_soff;
	u32 tcd0_nbytes;
	u32 tcd0_slast;
	u32 tcd0_daddr;
	union {
		u16 tcd0_citer_elink;
		u16 tcd0_citer;
	};
	u16 tcd0_doff;
	u32 tcd0_dlast_sga;
	union {
		u16 tcd0_biter_elink;
		u16 tcd0_biter;
	};
	u16 tcd0_csr;
	u32 tcd1_saddr;
	u16 tcd1_attr;
	u16 tcd1_soff;
	u32 tcd1_nbytes;
	u32 tcd1_slast;
	u32 tcd1_daddr;
	union {
		u16 tcd1_citer_elink;
		u16 tcd1_citer;
	};
	u16 tcd1_doff;
	u32 tcd1_dlast_sga;
	union {
		u16 tcd1_biter;
		u16 tcd1_biter_elink;
	};
	u16 tcd1_csr;
	u32 tcd2_saddr;
	u16 tcd2_attr;
	u16 tcd2_soff;
	u32 tcd2_nbytes;
	u32 tcd2_slast;
	u32 tcd2_daddr;
	union {
		u16 tcd2_citer;
		u16 tcd2_citer_elink;
	};
	u16 tcd2_doff;
	u32 tcd2_dlast_sga;
	union {
		u16 tcd2_biter_elink;
		u16 tcd2_biter;
	};
	u16 tcd2_csr;
	u32 tcd3_saddr;
	u16 tcd3_attr;
	u16 tcd3_soff;
	u32 tcd3_nbytes;
	u32 tcd3_slast;
	u32 tcd3_daddr;
	union {
		u16 tcd3_citer;
		u16 tcd3_citer_elink;
	};
	u16 tcd3_doff;
	u32 tcd3_dlast_sga;
	union {
		u16 tcd3_biter_elink;
		u16 tcd3_biter;
	};
	u16 tcd3_csr;
	u32 tcd4_saddr;
	u16 tcd4_attr;
	u16 tcd4_soff;
	u32 tcd4_nbytes;
	u32 tcd4_slast;
	u32 tcd4_daddr;
	union {
		u16 tcd4_citer;
		u16 tcd4_citer_elink;
	};
	u16 tcd4_doff;
	u32 tcd4_dlast_sga;
	union {
		u16 tcd4_biter;
		u16 tcd4_biter_elink;
	};
	u16 tcd4_csr;
	u32 tcd5_saddr;
	u16 tcd5_attr;
	u16 tcd5_soff;
	u32 tcd5_nbytes;
	u32 tcd5_slast;
	u32 tcd5_daddr;
	union {
		u16 tcd5_citer;
		u16 tcd5_citer_elink;
	};
	u16 tcd5_doff;
	u32 tcd5_dlast_sga;
	union {
		u16 tcd5_biter_elink;
		u16 tcd5_biter;
	};
	u16 tcd5_csr;
	u32 tcd6_saddr;
	u16 tcd6_attr;
	u16 tcd6_soff;
	u32 tcd6_nbytes;
	u32 tcd6_slast;
	u32 tcd6_daddr;
	union {
		u16 tcd6_citer;
		u16 tcd6_citer_elink;
	};
	u16 tcd6_doff;
	u32 tcd6_dlast_sga;
	union {
		u16 tcd6_biter_elink;
		u16 tcd6_biter;
	};
	u16 tcd6_csr;
	u32 tcd7_saddr;
	u16 tcd7_attr;
	u16 tcd7_soff;
	u32 tcd7_nbytes;
	u32 tcd7_slast;
	u32 tcd7_daddr;
	union {
		u16 tcd7_citer;
		u16 tcd7_citer_elink;
	};
	u16 tcd7_doff;
	u32 tcd7_dlast_sga;
	union {
		u16 tcd7_biter_elink;
		u16 tcd7_biter;
	};
	u16 tcd7_csr;
	u32 tcd8_saddr;
	u16 tcd8_attr;
	u16 tcd8_soff;
	u32 tcd8_nbytes;
	u32 tcd8_slast;
	u32 tcd8_daddr;
	union {
		u16 tcd8_citer;
		u16 tcd8_citer_elink;
	};
	u16 tcd8_doff;
	u32 tcd8_dlast_sga;
	union {
		u16 tcd8_biter_elink;
		u16 tcd8_biter;
	};
	u16 tcd8_csr;
	u32 tcd9_saddr;
	u16 tcd9_attr;
	u16 tcd9_soff;
	u32 tcd9_nbytes;
	u32 tcd9_slast;
	u32 tcd9_daddr;
	union {
		u16 tcd9_citer_elink;
		u16 tcd9_citer;
	};
	u16 tcd9_doff;
	u32 tcd9_dlast_sga;
	union {
		u16 tcd9_biter_elink;
		u16 tcd9_biter;
	};
	u16 tcd9_csr;
	u32 tcd10_saddr;
	u16 tcd10_attr;
	u16 tcd10_soff;
	u32 tcd10_nbytes;
	u32 tcd10_slast;
	u32 tcd10_daddr;
	union {
		u16 tcd10_citer_elink;
		u16 tcd10_citer;
	};
	u16 tcd10_doff;
	u32 tcd10_dlast_sga;
	union {
		u16 tcd10_biter;
		u16 tcd10_biter_elink;
	};
	u16 tcd10_csr;
	u32 tcd11_saddr;
	u16 tcd11_attr;
	u16 tcd11_soff;
	u32 tcd11_nbytes;
	u32 tcd11_slast;
	u32 tcd11_daddr;
	union {
		u16 tcd11_citer;
		u16 tcd11_citer_elink;
	};
	u16 tcd11_doff;
	u32 tcd11_dlast_sga;
	union {
		u16 tcd11_biter;
		u16 tcd11_biter_elink;
	};
	u16 tcd11_csr;
	u32 tcd12_saddr;
	u16 tcd12_attr;
	u16 tcd12_soff;
	u32 tcd12_nbytes;
	u32 tcd12_slast;
	u32 tcd12_daddr;
	union {
		u16 tcd12_citer;
		u16 tcd12_citer_elink;
	};
	u16 tcd12_doff;
	u32 tcd12_dlast_sga;
	union {
		u16 tcd12_biter;
		u16 tcd12_biter_elink;
	};
	u16 tcd12_csr;
	u32 tcd13_saddr;
	u16 tcd13_attr;
	u16 tcd13_soff;
	u32 tcd13_nbytes;
	u32 tcd13_slast;
	u32 tcd13_daddr;
	union {
		u16 tcd13_citer_elink;
		u16 tcd13_citer;
	};
	u16 tcd13_doff;
	u32 tcd13_dlast_sga;
	union {
		u16 tcd13_biter_elink;
		u16 tcd13_biter;
	};
	u16 tcd13_csr;
	u32 tcd14_saddr;
	u16 tcd14_attr;
	u16 tcd14_soff;
	u32 tcd14_nbytes;
	u32 tcd14_slast;
	u32 tcd14_daddr;
	union {
		u16 tcd14_citer;
		u16 tcd14_citer_elink;
	};
	u16 tcd14_doff;
	u32 tcd14_dlast_sga;
	union {
		u16 tcd14_biter_elink;
		u16 tcd14_biter;
	};
	u16 tcd14_csr;
	u32 tcd15_saddr;
	u16 tcd15_attr;
	u16 tcd15_soff;
	u32 tcd15_nbytes;
	u32 tcd15_slast;
	u32 tcd15_daddr;
	union {
		u16 tcd15_citer_elink;
		u16 tcd15_citer;
	};
	u16 tcd15_doff;
	u32 tcd15_dlast_sga;
	union {
		u16 tcd15_biter;
		u16 tcd15_biter_elink;
	};
	u16 tcd15_csr;
} edma_t;

/*********************************************************************
* Interrupt Controller (INTC)
*********************************************************************/

typedef struct int0_ctrl {
	u32 iprh0;		/* 0x00 Pending Register High */
	u32 iprl0;		/* 0x04 Pending Register Low */
	u32 imrh0;		/* 0x08 Mask Register High */
	u32 imrl0;		/* 0x0C Mask Register Low */
	u32 frch0;		/* 0x10 Force Register High */
	u32 frcl0;		/* 0x14 Force Register Low */
	u16 res1;		/* 0x18 - 0x19 */
	u16 icfg0;		/* 0x1A Configuration Register */
	u8 simr0;		/* 0x1C Set Interrupt Mask */
	u8 cimr0;		/* 0x1D Clear Interrupt Mask */
	u8 clmask0;		/* 0x1E Current Level Mask */
	u8 slmask;		/* 0x1F Saved Level Mask */
	u32 res2[8];		/* 0x20 - 0x3F */
	u8 icr0[64];		/* 0x40 - 0x7F Control registers */
	u32 res3[24];		/* 0x80 - 0xDF */
	u8 swiack0;		/* 0xE0 Software Interrupt Acknowledge */
	u8 res4[3];		/* 0xE1 - 0xE3 */
	u8 Lniack0_1;		/* 0xE4 Level n interrupt acknowledge resister */
	u8 res5[3];		/* 0xE5 - 0xE7 */
	u8 Lniack0_2;		/* 0xE8 Level n interrupt acknowledge resister */
	u8 res6[3];		/* 0xE9 - 0xEB */
	u8 Lniack0_3;		/* 0xEC Level n interrupt acknowledge resister */
	u8 res7[3];		/* 0xED - 0xEF */
	u8 Lniack0_4;		/* 0xF0 Level n interrupt acknowledge resister */
	u8 res8[3];		/* 0xF1 - 0xF3 */
	u8 Lniack0_5;		/* 0xF4 Level n interrupt acknowledge resister */
	u8 res9[3];		/* 0xF5 - 0xF7 */
	u8 Lniack0_6;		/* 0xF8 Level n interrupt acknowledge resister */
	u8 resa[3];		/* 0xF9 - 0xFB */
	u8 Lniack0_7;		/* 0xFC Level n interrupt acknowledge resister */
	u8 resb[3];		/* 0xFD - 0xFF */
} int0_t;

typedef struct int1_ctrl {
	/* Interrupt Controller 1 */
	u32 iprh1;		/* 0x00 Pending Register High */
	u32 iprl1;		/* 0x04 Pending Register Low */
	u32 imrh1;		/* 0x08 Mask Register High */
	u32 imrl1;		/* 0x0C Mask Register Low */
	u32 frch1;		/* 0x10 Force Register High */
	u32 frcl1;		/* 0x14 Force Register Low */
	u16 res1;		/* 0x18 */
	u16 icfg1;		/* 0x1A Configuration Register */
	u8 simr1;		/* 0x1C Set Interrupt Mask */
	u8 cimr1;		/* 0x1D Clear Interrupt Mask */
	u16 res2;		/* 0x1E - 0x1F */
	u32 res3[8];		/* 0x20 - 0x3F */
	u8 icr1[64];		/* 0x40 - 0x7F */
	u32 res4[24];		/* 0x80 - 0xDF */
	u8 swiack1;		/* 0xE0 Software Interrupt Acknowledge */
	u8 res5[3];		/* 0xE1 - 0xE3 */
	u8 Lniack1_1;		/* 0xE4 Level n interrupt acknowledge resister */
	u8 res6[3];		/* 0xE5 - 0xE7 */
	u8 Lniack1_2;		/* 0xE8 Level n interrupt acknowledge resister */
	u8 res7[3];		/* 0xE9 - 0xEB */
	u8 Lniack1_3;		/* 0xEC Level n interrupt acknowledge resister */
	u8 res8[3];		/* 0xED - 0xEF */
	u8 Lniack1_4;		/* 0xF0 Level n interrupt acknowledge resister */
	u8 res9[3];		/* 0xF1 - 0xF3 */
	u8 Lniack1_5;		/* 0xF4 Level n interrupt acknowledge resister */
	u8 resa[3];		/* 0xF5 - 0xF7 */
	u8 Lniack1_6;		/* 0xF8 Level n interrupt acknowledge resister */
	u8 resb[3];		/* 0xF9 - 0xFB */
	u8 Lniack1_7;		/* 0xFC Level n interrupt acknowledge resister */
	u8 resc[3];		/* 0xFD - 0xFF */
} int1_t;

/*********************************************************************
* Global Interrupt Acknowledge (IACK)
*********************************************************************/

typedef struct iack {
	u8 resv0[0xE0];
	u8 gswiack;
	u8 resv1[0x3];
	u8 gl1iack;
	u8 resv2[0x3];
	u8 gl2iack;
	u8 resv3[0x3];
	u8 gl3iack;
	u8 resv4[0x3];
	u8 gl4iack;
	u8 resv5[0x3];
	u8 gl5iack;
	u8 resv6[0x3];
	u8 gl6iack;
	u8 resv7[0x3];
	u8 gl7iack;
} iack_t;

/*********************************************************************
* DMA Serial Peripheral Interface (DSPI)
*********************************************************************/

typedef struct dspi {
	u32 dmcr;
	u8 resv0[0x4];
	u32 dtcr;
	u32 dctar0;
	u32 dctar1;
	u32 dctar2;
	u32 dctar3;
	u32 dctar4;
	u32 dctar5;
	u32 dctar6;
	u32 dctar7;
	u32 dsr;
	u32 dirsr;
	u32 dtfr;
	u32 drfr;
	u32 dtfdr0;
	u32 dtfdr1;
	u32 dtfdr2;
	u32 dtfdr3;
	u8 resv1[0x30];
	u32 drfdr0;
	u32 drfdr1;
	u32 drfdr2;
	u32 drfdr3;
} dspi_t;

/*********************************************************************
* Edge Port Module (EPORT)
*********************************************************************/

typedef struct eport {
	u16 eppar;
	u8 epddr;
	u8 epier;
	u8 epdr;
	u8 eppdr;
	u8 epfr;
} eport_t;

/*********************************************************************
* Watchdog Timer Modules (WTM)
*********************************************************************/

typedef struct wtm {
	u16 wcr;
	u16 wmr;
	u16 wcntr;
	u16 wsr;
} wtm_t;

/*********************************************************************
* Serial Boot Facility (SBF)
*********************************************************************/

typedef struct sbf {
	u8 resv0[0x18];
	u16 sbfsr;		/* Serial Boot Facility Status Register */
	u8 resv1[0x6];
	u16 sbfcr;		/* Serial Boot Facility Control Register */
} sbf_t;

/*********************************************************************
* Reset Controller Module (RCM)
*********************************************************************/

typedef struct rcm {
	u8 rcr;
	u8 rsr;
} rcm_t;

/*********************************************************************
* Chip Configuration Module (CCM)
*********************************************************************/

typedef struct ccm {
	u8 ccm_resv0[0x4];
	u16 ccr;		/* Chip Configuration Register (256 TEPBGA, Read-only) */
	u8 resv1[0x2];
	u16 rcon;		/* Reset Configuration (256 TEPBGA, Read-only) */
	u16 cir;		/* Chip Identification Register (Read-only) */
	u8 resv2[0x4];
	u16 misccr;		/* Miscellaneous Control Register */
	u16 cdr;		/* Clock Divider Register */
	u16 uocsr;		/* USB On-the-Go Controller Status Register */
} ccm_t;

/*********************************************************************
* General Purpose I/O Module (GPIO)
*********************************************************************/

typedef struct gpio {
	u8 podr_fec0h;		/* FEC0 High Port Output Data Register */
	u8 podr_fec0l;		/* FEC0 Low Port Output Data Register */
	u8 podr_ssi;		/* SSI Port Output Data Register */
	u8 podr_fbctl;		/* Flexbus Control Port Output Data Register */
	u8 podr_be;		/* Flexbus Byte Enable Port Output Data Register */
	u8 podr_cs;		/* Flexbus Chip-Select Port Output Data Register */
	u8 podr_dma;		/* DMA Port Output Data Register */
	u8 podr_feci2c;		/* FEC1 / I2C Port Output Data Register */
	u8 resv0[0x1];
	u8 podr_uart;		/* UART Port Output Data Register */
	u8 podr_dspi;		/* DSPI Port Output Data Register */
	u8 podr_timer;		/* Timer Port Output Data Register */
	u8 podr_pci;		/* PCI Port Output Data Register */
	u8 podr_usb;		/* USB Port Output Data Register */
	u8 podr_atah;		/* ATA High Port Output Data Register */
	u8 podr_atal;		/* ATA Low Port Output Data Register */
	u8 podr_fec1h;		/* FEC1 High Port Output Data Register */
	u8 podr_fec1l;		/* FEC1 Low Port Output Data Register */
	u8 resv1[0x2];
	u8 podr_fbadh;		/* Flexbus AD High Port Output Data Register */
	u8 podr_fbadmh;		/* Flexbus AD Med-High Port Output Data Register */
	u8 podr_fbadml;		/* Flexbus AD Med-Low Port Output Data Register */
	u8 podr_fbadl;		/* Flexbus AD Low Port Output Data Register */
	u8 pddr_fec0h;		/* FEC0 High Port Data Direction Register */
	u8 pddr_fec0l;		/* FEC0 Low Port Data Direction Register */
	u8 pddr_ssi;		/* SSI Port Data Direction Register */
	u8 pddr_fbctl;		/* Flexbus Control Port Data Direction Register */
	u8 pddr_be;		/* Flexbus Byte Enable Port Data Direction Register */
	u8 pddr_cs;		/* Flexbus Chip-Select Port Data Direction Register */
	u8 pddr_dma;		/* DMA Port Data Direction Register */
	u8 pddr_feci2c;		/* FEC1 / I2C Port Data Direction Register */
	u8 resv2[0x1];
	u8 pddr_uart;		/* UART Port Data Direction Register */
	u8 pddr_dspi;		/* DSPI Port Data Direction Register */
	u8 pddr_timer;		/* Timer Port Data Direction Register */
	u8 pddr_pci;		/* PCI Port Data Direction Register */
	u8 pddr_usb;		/* USB Port Data Direction Register */
	u8 pddr_atah;		/* ATA High Port Data Direction Register */
	u8 pddr_atal;		/* ATA Low Port Data Direction Register */
	u8 pddr_fec1h;		/* FEC1 High Port Data Direction Register */
	u8 pddr_fec1l;		/* FEC1 Low Port Data Direction Register */
	u8 resv3[0x2];
	u8 pddr_fbadh;		/* Flexbus AD High Port Data Direction Register */
	u8 pddr_fbadmh;		/* Flexbus AD Med-High Port Data Direction Register */
	u8 pddr_fbadml;		/* Flexbus AD Med-Low Port Data Direction Register */
	u8 pddr_fbadl;		/* Flexbus AD Low Port Data Direction Register */
	u8 ppdsdr_fec0h;	/* FEC0 High Port Pin Data/Set Data Register */
	u8 ppdsdr_fec0l;	/* FEC0 Low Port Clear Output Data Register */
	u8 ppdsdr_ssi;		/* SSI Port Pin Data/Set Data Register */
	u8 ppdsdr_fbctl;	/* Flexbus Control Port Pin Data/Set Data Register */
	u8 ppdsdr_be;		/* Flexbus Byte Enable Port Pin Data/Set Data Register */
	u8 ppdsdr_cs;		/* Flexbus Chip-Select Port Pin Data/Set Data Register */
	u8 ppdsdr_dma;		/* DMA Port Pin Data/Set Data Register */
	u8 ppdsdr_feci2c;	/* FEC1 / I2C Port Pin Data/Set Data Register */
	u8 resv4[0x1];
	u8 ppdsdr_uart;		/* UART Port Pin Data/Set Data Register */
	u8 ppdsdr_dspi;		/* DSPI Port Pin Data/Set Data Register */
	u8 ppdsdr_timer;	/* FTimer Port Pin Data/Set Data Register */
	u8 ppdsdr_pci;		/* PCI Port Pin Data/Set Data Register */
	u8 ppdsdr_usb;		/* USB Port Pin Data/Set Data Register */
	u8 ppdsdr_atah;		/* ATA High Port Pin Data/Set Data Register */
	u8 ppdsdr_atal;		/* ATA Low Port Pin Data/Set Data Register */
	u8 ppdsdr_fec1h;	/* FEC1 High Port Pin Data/Set Data Register */
	u8 ppdsdr_fec1l;	/* FEC1 Low Port Pin Data/Set Data Register */
	u8 resv5[0x2];
	u8 ppdsdr_fbadh;	/* Flexbus AD High Port Pin Data/Set Data Register */
	u8 ppdsdr_fbadmh;	/* Flexbus AD Med-High Port Pin Data/Set Data Register */
	u8 ppdsdr_fbadml;	/* Flexbus AD Med-Low Port Pin Data/Set Data Register */
	u8 ppdsdr_fbadl;	/* Flexbus AD Low Port Pin Data/Set Data Register */
	u8 pclrr_fec0h;		/* FEC0 High Port Clear Output Data Register */
	u8 pclrr_fec0l;		/* FEC0 Low Port Pin Data/Set Data Register */
	u8 pclrr_ssi;		/* SSI Port Clear Output Data Register */
	u8 pclrr_fbctl;		/* Flexbus Control Port Clear Output Data Register */
	u8 pclrr_be;		/* Flexbus Byte Enable Port Clear Output Data Register */
	u8 pclrr_cs;		/* Flexbus Chip-Select Port Clear Output Data Register */
	u8 pclrr_dma;		/* DMA Port Clear Output Data Register */
	u8 pclrr_feci2c;	/* FEC1 / I2C Port Clear Output Data Register */
	u8 resv6[0x1];
	u8 pclrr_uart;		/* UART Port Clear Output Data Register */
	u8 pclrr_dspi;		/* DSPI Port Clear Output Data Register */
	u8 pclrr_timer;		/* Timer Port Clear Output Data Register */
	u8 pclrr_pci;		/* PCI Port Clear Output Data Register */
	u8 pclrr_usb;		/* USB Port Clear Output Data Register */
	u8 pclrr_atah;		/* ATA High Port Clear Output Data Register */
	u8 pclrr_atal;		/* ATA Low Port Clear Output Data Register */
	u8 pclrr_fec1h;		/* FEC1 High Port Clear Output Data Register */
	u8 pclrr_fec1l;		/* FEC1 Low Port Clear Output Data Register */
	u8 resv7[0x2];
	u8 pclrr_fbadh;		/* Flexbus AD High Port Clear Output Data Register */
	u8 pclrr_fbadmh;	/* Flexbus AD Med-High Port Clear Output Data Register */
	u8 pclrr_fbadml;	/* Flexbus AD Med-Low Port Clear Output Data Register */
	u8 pclrr_fbadl;		/* Flexbus AD Low Port Clear Output Data Register */
	u8 par_fec;		/* FEC Pin Assignment Register */
	u8 par_dma;		/* DMA Pin Assignment Register */
	u8 par_fbctl;		/* Flexbus Control Pin Assignment Register */
	u8 par_dspi;		/* DSPI Pin Assignment Register */
	u8 par_be;		/* Flexbus Byte-Enable Pin Assignment Register */
	u8 par_cs;		/* Flexbus Chip-Select Pin Assignment Register */
	u8 par_timer;		/* Time Pin Assignment Register */
	u8 par_usb;		/* USB Pin Assignment Register */
	u8 resv8[0x1];
	u8 par_uart;		/* UART Pin Assignment Register */
	u16 par_feci2c;		/* FEC / I2C Pin Assignment Register */
	u16 par_ssi;		/* SSI Pin Assignment Register */
	u16 par_ata;		/* ATA Pin Assignment Register */
	u8 par_irq;		/* IRQ Pin Assignment Register */
	u8 resv9[0x1];
	u16 par_pci;		/* PCI Pin Assignment Register */
	u8 mscr_sdram;		/* SDRAM Mode Select Control Register */
	u8 mscr_pci;		/* PCI Mode Select Control Register */
	u8 resv10[0x2];
	u8 dscr_i2c;		/* I2C Drive Strength Control Register */
	u8 dscr_flexbus;	/* FLEXBUS Drive Strength Control Register */
	u8 dscr_fec;		/* FEC Drive Strength Control Register */
	u8 dscr_uart;		/* UART Drive Strength Control Register */
	u8 dscr_dspi;		/* DSPI Drive Strength Control Register */
	u8 dscr_timer;		/* TIMER Drive Strength Control Register */
	u8 dscr_ssi;		/* SSI Drive Strength Control Register */
	u8 dscr_dma;		/* DMA Drive Strength Control Register */
	u8 dscr_debug;		/* DEBUG Drive Strength Control Register */
	u8 dscr_reset;		/* RESET Drive Strength Control Register */
	u8 dscr_irq;		/* IRQ Drive Strength Control Register */
	u8 dscr_usb;		/* USB Drive Strength Control Register */
	u8 dscr_ata;		/* ATA Drive Strength Control Register */
} gpio_t;

/*********************************************************************
* Random Number Generator (RNG)
*********************************************************************/

typedef struct rng {
	u32 rngcr;
	u32 rngsr;
	u32 rnger;
	u32 rngout;
} rng_t;

/*********************************************************************
* SDRAM Controller (SDRAMC)
*********************************************************************/

typedef struct sdramc {
	u32 sdmr;		/* SDRAM Mode/Extended Mode Register */
	u32 sdcr;		/* SDRAM Control Register */
	u32 sdcfg1;		/* SDRAM Configuration Register 1 */
	u32 sdcfg2;		/* SDRAM Chip Select Register */
	u8 resv0[0x100];
	u32 sdcs0;		/* SDRAM Mode/Extended Mode Register */
	u32 sdcs1;		/* SDRAM Mode/Extended Mode Register */
} sdramc_t;

/*********************************************************************
* Synchronous Serial Interface (SSI)
*********************************************************************/

typedef struct ssi {
	u32 tx0;
	u32 tx1;
	u32 rx0;
	u32 rx1;
	u32 cr;
	u32 isr;
	u32 ier;
	u32 tcr;
	u32 rcr;
	u32 ccr;
	u8 resv0[0x4];
	u32 fcsr;
	u8 resv1[0x8];
	u32 acr;
	u32 acadd;
	u32 acdat;
	u32 atag;
	u32 tmask;
	u32 rmask;
} ssi_t;

/*********************************************************************
* Phase Locked Loop (PLL)
*********************************************************************/

typedef struct pll {
	u32 pcr;		/* PLL Control Register */
	u32 psr;		/* PLL Status Register */
} pll_t;

typedef struct pci {
	u32 idr;		/* 0x00 Device Id / Vendor Id Register */
	u32 scr;		/* 0x04 Status / command Register */
	u32 ccrir;		/* 0x08 Class Code / Revision Id Register */
	u32 cr1;		/* 0x0c Configuration 1 Register */
	u32 bar0;		/* 0x10 Base address register 0 Register */
	u32 bar1;		/* 0x14 Base address register 1 Register */
	u32 bar2;		/* 0x18 Base address register 2 Register */
	u32 bar3;		/* 0x1c Base address register 3 Register */
	u32 bar4;		/* 0x20 Base address register 4 Register */
	u32 bar5;		/* 0x24 Base address register 5 Register */
	u32 ccpr;		/* 0x28 Cardbus CIS Pointer Register */
	u32 sid;		/* 0x2c Subsystem ID / Subsystem Vendor ID Register */
	u32 erbar;		/* 0x30 Expansion ROM Base Address Register */
	u32 cpr;		/* 0x34 Capabilities Pointer Register */
	u32 rsvd1;		/* 0x38 */
	u32 cr2;		/* 0x3c Configuration Register 2 */
	u32 rsvd2[8];		/* 0x40 - 0x5f */

	/* General control / status registers */
	u32 gscr;		/* 0x60 Global Status / Control Register */
	u32 tbatr0a;		/* 0x64 Target Base Address Translation Register  0 */
	u32 tbatr1a;		/* 0x68 Target Base Address Translation Register  1 */
	u32 tcr1;		/* 0x6c Target Control 1 Register */
	u32 iw0btar;		/* 0x70 Initiator Window 0 Base/Translation addr */
	u32 iw1btar;		/* 0x74 Initiator Window 1 Base/Translation addr */
	u32 iw2btar;		/* 0x78 Initiator Window 2 Base/Translation addr */
	u32 rsvd3;		/* 0x7c */
	u32 iwcr;		/* 0x80 Initiator Window Configuration Register */
	u32 icr;		/* 0x84 Initiator Control Register */
	u32 isr;		/* 0x88 Initiator Status Register */
	u32 tcr2;		/* 0x8c Target Control 2 Register */
	u32 tbatr0;		/* 0x90 Target Base Address Translation Register  0 */
	u32 tbatr1;		/* 0x94 Target Base Address Translation Register  1 */
	u32 tbatr2;		/* 0x98 Target Base Address Translation Register  2 */
	u32 tbatr3;		/* 0x9c Target Base Address Translation Register  3 */
	u32 tbatr4;		/* 0xa0 Target Base Address Translation Register  4 */
	u32 tbatr5;		/* 0xa4 Target Base Address Translation Register  5 */
	u32 intr;		/* 0xa8 Interrupt Register */
	u32 rsvd4[19];		/* 0xac - 0xf7 */
	u32 car;		/* 0xf8 Configuration Address Register */
} pci_t;

typedef struct pci_arbiter {
	/* Pci Arbiter Registers */
	union {
		u32 acr;	/* Arbiter Control Register */
		u32 asr;	/* Arbiter Status Register */
	};
} pciarb_t;

/* Register read/write struct */
typedef struct scm1 {
	u32 mpr;		/* 0x00 Master Privilege Register */
	u32 rsvd1[7];
	u32 pacra;		/* 0x20 Peripheral Access Control Register A */
	u32 pacrb;		/* 0x24 Peripheral Access Control Register B */
	u32 pacrc;		/* 0x28 Peripheral Access Control Register C */
	u32 pacrd;		/* 0x2C Peripheral Access Control Register D */
	u32 rsvd2[4];
	u32 pacre;		/* 0x40 Peripheral Access Control Register E */
	u32 pacrf;		/* 0x44 Peripheral Access Control Register F */
	u32 pacrg;		/* 0x48 Peripheral Access Control Register G */
} scm1_t;
/********************************************************************/

typedef struct rtcex {
	u32 rsvd1[3];
	u32 gocu;
	u32 gocl;
} rtcex_t;
#endif				/* __IMMAP_5445X__ */

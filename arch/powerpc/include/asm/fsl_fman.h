/*
 * MPC85xx Internal Memory Map
 *
 * Copyright 2010 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __FSL_FMAN_H__
#define __FSL_FMAN_H__

#include <asm/types.h>

typedef struct fm_bmi_common {
	u32	fmbm_init;	/* BMI initialization */
	u32	fmbm_cfg1;	/* BMI configuration1 */
	u32	fmbm_cfg2;	/* BMI configuration2 */
	u32	res0[0x5];
	u32	fmbm_ievr;	/* interrupt event register */
	u32	fmbm_ier;	/* interrupt enable register */
	u32	fmbm_ifr;	/* interrupt force register */
	u32	res1[0x5];
	u32	fmbm_arb[0x8];	/* BMI arbitration */
	u32	res2[0x28];
	u32	fmbm_gde;	/* global debug enable */
	u32	fmbm_pp[0x3f];	/* BMI port parameters */
	u32	res3;
	u32	fmbm_pfs[0x3f];	/* BMI port FIFO size */
	u32	res4;
	u32	fmbm_ppid[0x3f];/* port partition ID */
} fm_bmi_common_t;

typedef struct fm_qmi_common {
	u32	fmqm_gc;	/* general configuration register */
	u32	res0;
	u32	fmqm_eie;	/* error interrupt event register */
	u32	fmqm_eien;	/* error interrupt enable register */
	u32	fmqm_eif;	/* error interrupt force register */
	u32	fmqm_ie;	/* interrupt event register */
	u32	fmqm_ien;	/* interrupt enable register */
	u32	fmqm_if;	/* interrupt force register */
	u32	fmqm_gs;	/* global status register */
	u32	fmqm_ts;	/* task status register */
	u32	fmqm_etfc;	/* enqueue total frame counter */
	u32	fmqm_dtfc;	/* dequeue total frame counter */
	u32	fmqm_dc0;	/* dequeue counter 0 */
	u32	fmqm_dc1;	/* dequeue counter 1 */
	u32	fmqm_dc2;	/* dequeue counter 2 */
	u32	fmqm_dc3;	/* dequeue counter 3 */
	u32	fmqm_dfnoc;	/* dequeue FQID not override counter */
	u32	fmqm_dfcc;	/* dequeue FQID from context counter */
	u32	fmqm_dffc;	/* dequeue FQID from FD counter */
	u32	fmqm_dcc;	/* dequeue confirm counter */
	u32	res1[0xc];
	u32	fmqm_dtrc;	/* debug trap configuration register */
	u32	fmqm_efddd;	/* enqueue frame descriptor dynamic debug */
	u32	res3[0x2];
	u32	res4[0xdc];	/* missing debug regs */
} fm_qmi_common_t;

typedef struct fm_bmi {
	u8	res[1024];
} fm_bmi_t;

typedef struct fm_qmi {
	u8	res[1024];
} fm_qmi_t;

typedef struct fm_parser {
	u8	res[1024];
} fm_parser_t;

typedef struct fm_policer {
	u8	res[4*1024];
} fm_policer_t;

typedef struct fm_keygen {
	u8	res[4*1024];
} fm_keygen_t;

typedef struct fm_dma {
	u32	fmdmsr;		/* status register */
	u32	fmdmmr;		/* mode register */
	u32	fmdmtr;		/* bus threshold register */
	u32	fmdmhy;		/* bus hysteresis register */
	u32	fmdmsetr;	/* SOS emergency threshold register */
	u32	fmdmtah;	/* transfer bus address high register */
	u32	fmdmtal;	/* transfer bus address low register */
	u32	fmdmtcid;	/* transfer bus communication ID register */
	u32	fmdmra;		/* DMA bus internal ram address register */
	u32	fmdmrd;		/* DMA bus internal ram data register */
	u32	res0[0xb];
	u32	fmdmdcr;	/* debug counter */
	u32	fmdmemsr;	/* emrgency smoother register */
	u32	res1;
	u32	fmdmplr[32];	/* FM DMA PID-LIODN # register */
	u32	res[0x3c8];
} fm_dma_t;

typedef struct fm_fpm {
	u32	fpmtnc;		/* TNUM control */
	u32	fpmprc;		/* Port_ID control */
	u32	res0;
	u32	fpmflc;		/* flush control */
	u32	fpmdis1;	/* dispatch thresholds1 */
	u32	fpmdis2;	/* dispatch thresholds2 */
	u32	fmepi;		/* error pending interrupts */
	u32	fmrie;		/* rams interrupt enable */
	u32	fpmfcevent[0x4];/* FMan controller event 0-3 */
	u32	res1[0x4];
	u32	fpmfcmask[0x4];	/* FMan controller mask 0-3 */
	u32	res2[0x4];
	u32	fpmtsc1;	/* timestamp control1 */
	u32	fpmtsc2;	/* timestamp control2 */
	u32	fpmtsp;		/* time stamp */
	u32	fpmtsf;		/* time stamp fraction */
	u32	fpmrcr;		/* rams control and event */
	u32	res3[0x3];
	u32	fpmdrd[0x4];	/* data_ram data 0-3 */
	u32	res4[0xc];
	u32	fpmdra;		/* data ram access */
	u32	fm_ip_rev_1;	/* IP block revision 1 */
	u32	fm_ip_rev_2;	/* IP block revision 2 */
	u32	fmrstc;		/* reset command */
	u32	fmcld;		/* classifier debug control */
	u32	fmnpi;		/* normal pending interrupts */
	u32	res5;
	u32	fmnee;		/* event and enable */
	u32	fpmcev[0x4];	/* CPU event 0-3 */
	u32	res6[0x4];
	u32	fmfp_ps[0x40];	/* port status */
	u32	res7[0x260];
	u32	fpmts[0x80];	/* task status */
	u32	res8[0xa0];
} fm_fpm_t;

typedef struct fm_imem {
	u8	res[4*1024];
} fm_imem_t;

typedef struct fm_soft_parser {
	u8	res[4*1024];
} fm_soft_parser_t;

typedef struct fm_dtesc {
	u8	res[4*1024];
} fm_dtsec_t;

typedef struct fm_mdio {
	u8	res[4*1024];
} fm_mdio_t;

typedef struct fm_10gec {
	u8	res[4*1024];
} fm_10gec_t;

typedef struct fm_10gec_mdio {
	u8	res[4*1024];
} fm_10gec_mdio_t;

typedef struct fm_1588 {
	u8	res[4*1024];
} fm_1588_t;

typedef struct ccsr_fman {
	u8			muram[0x80000];
	fm_bmi_common_t		fm_bmi_common;
	fm_qmi_common_t		fm_qmi_common;
	u8			res0[2048];
	struct {
		fm_bmi_t	fm_bmi;
		fm_qmi_t	fm_qmi;
		fm_parser_t	fm_parser;
		u8		res[1024];
	} port[63];
	fm_policer_t		fm_policer;
	fm_keygen_t		fm_keygen;
	fm_dma_t		fm_dma;
	fm_fpm_t		fm_fpm;
	fm_imem_t		fm_imem;
	u8			res1[8*1024];
	fm_soft_parser_t	fm_soft_parser;
	u8			res2[96*1024];
	struct {
		fm_dtsec_t	fm_dtesc;
		fm_mdio_t	fm_mdio;
	} mac[4];
	u8			res3[32*1024];
	fm_10gec_t		fm_10gec;
	fm_10gec_mdio_t		fm_10gec_mdio;
	u8			res4[48*1024];
	fm_1588_t		fm_1588;
	u8			res5[4*1024];
} ccsr_fman_t;

#endif /*__FSL_FMAN_H__*/

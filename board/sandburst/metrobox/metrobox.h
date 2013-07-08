#ifndef __METROBOX_H__
#define __METROBOX_H__
/*
 * (C) Copyright 2005
 * Travis B. Sawyer, Sandburst Corporation, tsawyer@sandburst.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

typedef struct metrobox_board_id_s {
	const char name[40];
} METROBOX_BOARD_ID_ST, *METROBOX_BOARD_ID_PST;


/* Metrobox Opto-FPGA registers and definitions */
#include "hal_xc_auto.h"
typedef struct opto_fpga_regs_s {
	volatile unsigned long revision_ul;	/* Read Only  */
	volatile unsigned long reset_ul;	/* Read/Write */
	volatile unsigned long status_ul;	/* Read Only  */
	volatile unsigned long interrupt_ul;	/* Read Only  */
	volatile unsigned long mask_ul;	/* Read/Write */
	volatile unsigned long scratch_ul;	/* Read/Write */
	volatile unsigned long scrmask_ul;	/* Read/Write */
	volatile unsigned long control_ul;	/* Read/Write */
	volatile unsigned long boardinfo_ul;	/* Read Only  */
} __attribute__ ((packed)) OPTO_FPGA_REGS_ST , *OPTO_FPGA_REGS_PST;

#endif /* __METROBOX_H__ */

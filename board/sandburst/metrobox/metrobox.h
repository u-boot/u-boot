#ifndef __METROBOX_H__
#define __METROBOX_H__
/*
 * (C) Copyright 2005
 * Travis B. Sawyer, Sandburst Corporation, tsawyer@sandburst.com
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

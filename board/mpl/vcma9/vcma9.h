/*
 * (C) Copyright 2002, 2003
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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
 *
 */
 /****************************************************************************
 * Global routines used for VCMA9
 *****************************************************************************/

#include <asm/arch/s3c24x0_cpu.h>

extern void vcma9_print_info(void);
extern int do_mplcommon(cmd_tbl_t *cmdtp, int flag,
			int argc, char *const argv[]);

/* VCMA9 PLD registers */
enum vcma9_pld_regs {
	VCMA9_PLD_ID,
	VCMA9_PLD_NIC,
	VCMA9_PLD_CAN,
	VCMA9_PLD_MISC,
	VCMA9_PLD_GPCD,
	VCMA9_PLD_BOARD,
	VCMA9_PLD_SDRAM
};

#define VCMA9_PLD_BASE	(0x2C000100)

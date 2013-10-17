/*
 * (C) Copyright 2002, 2003
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

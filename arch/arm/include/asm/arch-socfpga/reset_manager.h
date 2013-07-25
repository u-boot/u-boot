/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	_RESET_MANAGER_H_
#define	_RESET_MANAGER_H_

void reset_cpu(ulong addr);
void reset_deassert_peripherals_handoff(void);

struct socfpga_reset_manager {
	u32	padding1;
	u32	ctrl;
	u32	padding2;
	u32	padding3;
	u32	mpu_mod_reset;
	u32	per_mod_reset;
	u32	per2_mod_reset;
	u32	brg_mod_reset;
};

#define RSTMGR_CTRL_SWWARMRSTREQ_LSB 1

#endif /* _RESET_MANAGER_H_ */

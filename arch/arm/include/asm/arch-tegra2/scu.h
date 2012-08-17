/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
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

#ifndef _SCU_H_
#define _SCU_H_

/* ARM Snoop Control Unit (SCU) registers */
struct scu_ctlr {
	uint scu_ctrl;		/* SCU Control Register, offset 00 */
	uint scu_cfg;		/* SCU Config Register, offset 04 */
	uint scu_cpu_pwr_stat;	/* SCU CPU Power Status Register, offset 08 */
	uint scu_inv_all;	/* SCU Invalidate All Register, offset 0C */
	uint scu_reserved0[12];	/* reserved, offset 10-3C */
	uint scu_filt_start;	/* SCU Filtering Start Address Reg, offset 40 */
	uint scu_filt_end;	/* SCU Filtering End Address Reg, offset 44 */
	uint scu_reserved1[2];	/* reserved, offset 48-4C */
	uint scu_acc_ctl;	/* SCU Access Control Register, offset 50 */
	uint scu_ns_acc_ctl;	/* SCU Non-secure Access Cntrl Reg, offset 54 */
};

#define SCU_CTRL_ENABLE		(1 << 0)

#endif	/* SCU_H */

/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>, Contributor: Mahavir Jain <mjain@marvell.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _ARMADA100CPU_H
#define _ARMADA100CPU_H

#include <asm/io.h>
#include <asm/system.h>

/*
 * CPU Interface Registers
 * Refer Datasheet Appendix A.2
 */
struct armd1cpu_registers {
	u32 chip_id;		/* Chip Id Reg */
	u32 pad;
	u32 cpu_conf;		/* CPU Conf Reg */
	u32 pad1;
	u32 cpu_sram_spd;	/* CPU SRAM Speed Reg */
	u32 pad2;
	u32 cpu_l2c_spd;	/* CPU L2cache Speed Conf */
	u32 mcb_conf;		/* MCB Conf Reg */
	u32 sys_boot_ctl;	/* Sytem Boot Control */
};

/*
 * Functions
 */
u32 armd1_sdram_base(int);
u32 armd1_sdram_size(int);

#endif /* _ARMADA100CPU_H */

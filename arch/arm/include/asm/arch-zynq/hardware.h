/*
 * Copyright (c) 2013 Xilinx Inc.
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

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H

#define XPSS_SYS_CTRL_BASEADDR		0xF8000000

/* Reflect slcr offsets */
struct slcr_regs {
	u32 scl; /* 0x0 */
	u32 slcr_lock; /* 0x4 */
	u32 slcr_unlock; /* 0x8 */
	u32 reserved1[125];
	u32 pss_rst_ctrl; /* 0x200 */
	u32 reserved2[21];
	u32 reboot_status; /* 0x258 */
};

#define slcr_base ((struct slcr_regs *) XPSS_SYS_CTRL_BASEADDR)

#endif /* _ASM_ARCH_HARDWARE_H */

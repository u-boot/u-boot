/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.aribaud@free.fr>
 *
 * Based on original Kirorion5x_ood support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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

#ifndef _ORION5X_CPU_H
#define _ORION5X_CPU_H

#include <asm/system.h>

#ifndef __ASSEMBLY__

#define ORION5X_CPU_WIN_CTRL_DATA(size, target, attr, en) (en | (target << 4) \
			| (attr << 8) | (orion5x_winctrl_calcsize(size) << 16))

#define ORION5XGBE_PORT_SERIAL_CONTROL1_REG(_x)	\
		((_x ? ORION5X_EGIGA0_BASE : ORION5X_EGIGA1_BASE) + 0x44c)

enum memory_bank {
	BANK0,
	BANK1,
	BANK2,
	BANK3
};

enum orion5x_cpu_winen {
	ORION5X_WIN_DISABLE,
	ORION5X_WIN_ENABLE
};

enum orion5x_cpu_target {
	ORION5X_TARGET_DRAM = 0,
	ORION5X_TARGET_DEVICE = 1,
	ORION5X_TARGET_PCI = 3,
	ORION5X_TARGET_PCIE = 4,
	ORION5X_TARGET_SASRAM = 9
};

enum orion5x_cpu_attrib {
	ORION5X_ATTR_DRAM_CS0 = 0x0e,
	ORION5X_ATTR_DRAM_CS1 = 0x0d,
	ORION5X_ATTR_DRAM_CS2 = 0x0b,
	ORION5X_ATTR_DRAM_CS3 = 0x07,
	ORION5X_ATTR_PCI_MEM = 0x59,
	ORION5X_ATTR_PCI_IO = 0x51,
	ORION5X_ATTR_PCIE_MEM = 0x59,
	ORION5X_ATTR_PCIE_IO = 0x51,
	ORION5X_ATTR_SASRAM = 0x00,
	ORION5X_ATTR_DEV_CS0 = 0x1e,
	ORION5X_ATTR_DEV_CS1 = 0x1d,
	ORION5X_ATTR_DEV_CS2 = 0x1b,
	ORION5X_ATTR_BOOTROM = 0x0f
};

/*
 * Default Device Address MAP BAR values
 */
#define ORION5X_DEFADR_PCIE_MEM	0x90000000
#define ORION5X_DEFADR_PCIE_MEM_REMAP_LO	0x90000000
#define ORION5X_DEFADR_PCIE_MEM_REMAP_HI	0
#define ORION5X_DEFSZ_PCIE_MEM	(128*1024*1024)

#define ORION5X_DEFADR_PCIE_IO	0xf0000000
#define ORION5X_DEFADR_PCIE_IO_REMAP_LO	0x90000000
#define ORION5X_DEFADR_PCIE_IO_REMAP_HI	0
#define ORION5X_DEFSZ_PCIE_IO	(64*1024)

#define ORION5X_DEFADR_PCI_MEM	0x98000000
#define ORION5X_DEFSZ_PCI_MEM	(128*1024*1024)

#define ORION5X_DEFADR_PCI_IO	0xf0100000
#define ORION5X_DEFSZ_PCI_IO	(64*1024)

#define ORION5X_DEFADR_DEV_CS0	0xfa000000
#define ORION5X_DEFSZ_DEV_CS0	(2*1024*1024)

#define ORION5X_DEFADR_DEV_CS1	0xf8000000
#define ORION5X_DEFSZ_DEV_CS1	(32*1024*1024)

#define ORION5X_DEFADR_DEV_CS2	0xfa800000
#define ORION5X_DEFSZ_DEV_CS2	(1*1024*1024)

#define ORION5X_DEFADR_BOOTROM	0xFFF80000
#define ORION5X_DEFSZ_BOOTROM	(512*1024)

/*
 * PCIE registers are used for SoC device ID and revision
 */
#define PCIE_DEV_ID_OFF         (ORION5X_REG_PCIE_BASE + 0x0000)
#define PCIE_DEV_REV_OFF        (ORION5X_REG_PCIE_BASE + 0x0008)

/*
 * The following definitions are intended for identifying
 * the real device and revision on which u-boot is running
 * even if it was compiled only for a specific one. Thus,
 * these constants must not be considered chip-specific.
 */

/* Orion-1 (88F5181) and Orion-VoIP (88F5181L) */
#define MV88F5181_DEV_ID        0x5181
#define MV88F5181_REV_B1        3
#define MV88F5181L_REV_A0       8
#define MV88F5181L_REV_A1       9
/* Orion-NAS (88F5182) */
#define MV88F5182_DEV_ID        0x5182
#define MV88F5182_REV_A2        2
/* Orion-2 (88F5281) */
#define MV88F5281_DEV_ID        0x5281
#define MV88F5281_REV_D0        4
#define MV88F5281_REV_D1        5
#define MV88F5281_REV_D2        6
/* Orion-1-90 (88F6183) */
#define MV88F6183_DEV_ID        0x6183
#define MV88F6183_REV_B0        3

/*
 * read feroceon core extra feature register
 * using co-proc instruction
 */
static inline unsigned int readfr_extra_feature_reg(void)
{
	unsigned int val;
	asm volatile ("mrc p15, 1, %0, c15, c1, 0 @ readfr exfr" : "=r"
			(val) : : "cc");
	return val;
}

/*
 * write feroceon core extra feature register
 * using co-proc instruction
 */
static inline void writefr_extra_feature_reg(unsigned int val)
{
	asm volatile ("mcr p15, 1, %0, c15, c1, 0 @ writefr exfr" : : "r"
			(val) : "cc");
	isb();
}

/*
 * AHB to Mbus Bridge Registers
 * Source: 88F5182 User Manual, Appendix A, section A.4
 * Note: only windows 0 and 1 have remap capability.
 */
struct orion5x_win_registers {
	u32 ctrl;
	u32 base;
	u32 remap_lo;
	u32 remap_hi;
};

/*
 * CPU control and status Registers
 * Source: 88F5182 User Manual, Appendix A, section A.4
 */
struct orion5x_cpu_registers {
	u32 config;	/*0x20100 */
	u32 ctrl_stat;	/*0x20104 */
	u32 rstoutn_mask; /* 0x20108 */
	u32 sys_soft_rst; /* 0x2010C */
	u32 ahb_mbus_cause_irq; /* 0x20110 */
	u32 ahb_mbus_mask_irq; /* 0x20114 */
};

/*
 * DDR SDRAM Controller Address Decode Registers
 * Source: 88F5182 User Manual, Appendix A, section A.5.1
 */
struct orion5x_ddr_addr_decode_registers {
	u32 base;
	u32 size;
};

/*
 * functions
 */
void reset_cpu(unsigned long ignored);
u32 orion5x_device_id(void);
u32 orion5x_device_rev(void);
unsigned int orion5x_winctrl_calcsize(unsigned int sizeval);
#endif /* __ASSEMBLY__ */
#endif /* _ORION5X_CPU_H */

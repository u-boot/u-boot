/*
 * (C) Copyright 2006-2010
 * Texas Instruments, <www.ti.com>
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
 *
 */

#ifndef _CPU_H
#define _CPU_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct gptimer {
	u32 tidr;		/* 0x00 r */
	u8 res[0xc];
	u32 tiocp_cfg;		/* 0x10 rw */
	u32 tistat;		/* 0x14 r */
	u32 tisr;		/* 0x18 rw */
	u32 tier;		/* 0x1c rw */
	u32 twer;		/* 0x20 rw */
	u32 tclr;		/* 0x24 rw */
	u32 tcrr;		/* 0x28 rw */
	u32 tldr;		/* 0x2c rw */
	u32 ttgr;		/* 0x30 rw */
	u32 twpc;		/* 0x34 r */
	u32 tmar;		/* 0x38 rw */
	u32 tcar1;		/* 0x3c r */
	u32 tcicr;		/* 0x40 rw */
	u32 tcar2;		/* 0x44 r */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* enable sys_clk NO-prescale /1 */
#define GPT_EN			((0x0 << 2) | (0x1 << 1) | (0x1 << 0))

/* Watchdog */
#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct watchdog {
	u8 res1[0x34];
	u32 wwps;		/* 0x34 r */
	u8 res2[0x10];
	u32 wspr;		/* 0x48 rw */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#define WD_UNLOCK1		0xAAAA
#define WD_UNLOCK2		0x5555

#define SYSCLKDIV_1		(0x1 << 6)
#define SYSCLKDIV_2		(0x1 << 7)

#define CLKSEL_GPT1		(0x1 << 0)

#define EN_GPT1			(0x1 << 0)
#define EN_32KSYNC		(0x1 << 2)

#define ST_WDT2			(0x1 << 5)

#define RESETDONE		(0x1 << 0)

#define TCLR_ST			(0x1 << 0)
#define TCLR_AR			(0x1 << 1)
#define TCLR_PRE		(0x1 << 5)

/* I2C base */
#define I2C_BASE1		(OMAP44XX_L4_PER_BASE + 0x70000)
#define I2C_BASE2		(OMAP44XX_L4_PER_BASE + 0x72000)
#define I2C_BASE3		(OMAP44XX_L4_PER_BASE + 0x60000)

#endif /* _CPU_H */

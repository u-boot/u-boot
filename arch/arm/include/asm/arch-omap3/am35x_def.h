/*
 * am35x_def.h - TI's AM35x specific definitions.
 *
 * Based on arch/arm/include/asm/arch-omap3/cpu.h
 *
 * Author: Ajay Kumar Gupta <ajay.gupta@ti.com>
 *
 * Copyright (c) 2010 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _AM35X_DEF_H_
#define _AM35X_DEF_H_

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__

/* IP_SW_RESET bits */
#define CPGMACSS_SW_RST		(1 << 1)	/* reset CPGMAC */

/* General register mappings of system control module */
#define AM35X_SCM_GEN_BASE	0x48002270
struct am35x_scm_general {
	u32 res1[0xC4];		/* 0x000 - 0x30C */
	u32 devconf2;		/* 0x310 */
	u32 devconf3;		/* 0x314 */
	u32 res2[0x2];		/* 0x318 - 0x31C */
	u32 cba_priority;	/* 0x320 */
	u32 lvl_intr_clr;	/* 0x324 */
	u32 ip_sw_reset;	/* 0x328 */
	u32 ipss_clk_ctrl;	/* 0x32C */
};
#define am35x_scm_general_regs ((struct am35x_scm_general *)AM35X_SCM_GEN_BASE)

#endif /*__ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#endif /* _AM35X_DEF_H_ */

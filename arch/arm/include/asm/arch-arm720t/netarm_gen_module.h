/*
 * include/asm-armnommu/arch-netarm/netarm_gen_module.h
 *
 * Copyright (C) 2005
 * Art Shipkowski, Videon Central, Inc., <art@videon-central.com>
 *
 * Copyright (C) 2000, 2001 NETsilicon, Inc.
 * Copyright (C) 2000, 2001 Red Hat, Inc.
 *
 * This software is copyrighted by Red Hat. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall Red Hat
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Joe deBlaquiere
 *
 * Modified to support NS7520 by Art Shipkowski.
 */

#ifndef __NETARM_GEN_MODULE_REGISTERS_H
#define __NETARM_GEN_MODULE_REGISTERS_H

/* GEN unit register offsets */

#define NETARM_GEN_MODULE_BASE		(0xFFB00000)

#define get_gen_reg_addr(c) ((volatile unsigned int *)(NETARM_GEN_MODULE_BASE + (c)))

#define NETARM_GEN_SYSTEM_CONTROL	(0x00)
#define NETARM_GEN_STATUS_CONTROL	(0x04)
#define NETARM_GEN_PLL_CONTROL		(0x08)
#define NETARM_GEN_SOFTWARE_SERVICE	(0x0c)

#define NETARM_GEN_TIMER1_CONTROL	(0x10)
#define NETARM_GEN_TIMER1_STATUS	(0x14)
#define NETARM_GEN_TIMER2_CONTROL	(0x18)
#define NETARM_GEN_TIMER2_STATUS	(0x1c)

#define NETARM_GEN_PORTA		(0x20)
#ifndef CONFIG_NETARM_NS7520
#define NETARM_GEN_PORTB		(0x24)
#endif
#define NETARM_GEN_PORTC		(0x28)

#define NETARM_GEN_INTR_ENABLE		(0x30)
#define NETARM_GEN_INTR_ENABLE_SET	(0x34)
#define NETARM_GEN_INTR_ENABLE_CLR	(0x38)
#define NETARM_GEN_INTR_STATUS_EN	(0x34)
#define NETARM_GEN_INTR_STATUS_RAW	(0x38)

#define NETARM_GEN_CACHE_CONTROL1	(0x40)
#define NETARM_GEN_CACHE_CONTROL2	(0x44)

/* select bitfield definitions */

/* System Control Register ( 0xFFB0_0000 ) */

#define NETARM_GEN_SYS_CFG_LENDIAN	(0x80000000)
#define NETARM_GEN_SYS_CFG_BENDIAN	(0x00000000)

#define NETARM_GEN_SYS_CFG_BUSQRTR	(0x00000000)
#define NETARM_GEN_SYS_CFG_BUSHALF	(0x20000000)
#define NETARM_GEN_SYS_CFG_BUSFULL	(0x40000000)

#define NETARM_GEN_SYS_CFG_BCLK_DISABLE (0x10000000)

#define NETARM_GEN_SYS_CFG_WDOG_EN	(0x01000000)
#define NETARM_GEN_SYS_CFG_WDOG_IRQ	(0x00000000)
#define NETARM_GEN_SYS_CFG_WDOG_FIQ	(0x00400000)
#define NETARM_GEN_SYS_CFG_WDOG_RST	(0x00800000)
#define NETARM_GEN_SYS_CFG_WDOG_24	(0x00000000)
#define NETARM_GEN_SYS_CFG_WDOG_26	(0x00100000)
#define NETARM_GEN_SYS_CFG_WDOG_28	(0x00200000)
#define NETARM_GEN_SYS_CFG_WDOG_29	(0x00300000)

#define NETARM_GEN_SYS_CFG_BUSMON_EN	(0x00040000)
#define NETARM_GEN_SYS_CFG_BUSMON_128	(0x00000000)
#define NETARM_GEN_SYS_CFG_BUSMON_64	(0x00010000)
#define NETARM_GEN_SYS_CFG_BUSMON_32	(0x00020000)
#define NETARM_GEN_SYS_CFG_BUSMON_16	(0x00030000)

#define NETARM_GEN_SYS_CFG_USER_EN	(0x00008000)
#define NETARM_GEN_SYS_CFG_BUSER_EN	(0x00004000)

#define NETARM_GEN_SYS_CFG_BUSARB_INT	(0x00002000)
#define NETARM_GEN_SYS_CFG_BUSARB_EXT	(0x00000000)

#define NETARM_GEN_SYS_CFG_DMATST	(0x00001000)

#define NETARM_GEN_SYS_CFG_TEALAST	(0x00000800)

#define NETARM_GEN_SYS_CFG_ALIGN_ABORT	(0x00000400)

#define NETARM_GEN_SYS_CFG_CACHE_EN	(0x00000200)

#define NETARM_GEN_SYS_CFG_WRI_BUF_EN	(0x00000100)

#define NETARM_GEN_SYS_CFG_CACHE_INIT	(0x00000080)

/* PLL Control Register ( 0xFFB0_0008 ) */

#define NETARM_GEN_PLL_CTL_PLLCNT_MASK	(0x0F000000)

#define NETARM_GEN_PLL_CTL_PLLCNT(x)	(((x)<<24) & \
					 NETARM_GEN_PLL_CTL_PLLCNT_MASK)

/* Defaults for POLTST and ICP Fields in PLL CTL */
#define NETARM_GEN_PLL_CTL_OUTDIV(x)	(x)
#define NETARM_GEN_PLL_CTL_INDIV(x)	((x)<<6)
#define NETARM_GEN_PLL_CTL_POLTST_DEF	(0x00000E00)
#define NETARM_GEN_PLL_CTL_ICP_DEF	(0x0000003C)


/* Software Service Register ( 0xFFB0_000C ) */

#define NETARM_GEN_SW_SVC_RESETA	(0x123)
#define NETARM_GEN_SW_SVC_RESETB	(0x321)

/* PORT C Register ( 0xFFB0_0028 ) */

#ifndef CONFIG_NETARM_NS7520
#define NETARM_GEN_PORT_MODE(x)		(((x)<<24) + (0xFF00))
#define NETARM_GEN_PORT_DIR(x)		(((x)<<16) + (0xFF00))
#else
#define NETARM_GEN_PORT_MODE(x)		((x)<<24)
#define NETARM_GEN_PORT_DIR(x)		((x)<<16)
#define NETARM_GEN_PORT_CSF(x)		((x)<<8)
#endif

/* Timer Registers ( 0xFFB0_0010 0xFFB0_0018 ) */

#define NETARM_GEN_TCTL_ENABLE		(0x80000000)
#define NETARM_GEN_TCTL_INT_ENABLE	(0x40000000)

#define NETARM_GEN_TCTL_USE_IRQ		(0x00000000)
#define NETARM_GEN_TCTL_USE_FIQ		(0x20000000)

#define NETARM_GEN_TCTL_USE_PRESCALE	(0x10000000)
#define NETARM_GEN_TCTL_INIT_COUNT(x)	((x) & 0x1FF)

#define NETARM_GEN_TSTAT_INTPEN		(0x40000000)
#if ~defined(CONFIG_NETARM_NS7520)
#define NETARM_GEN_TSTAT_CTC_MASK	(0x000001FF)
#else
#define NETARM_GEN_TSTAT_CTC_MASK	(0x0FFFFFFF)
#endif

/* prescale to msecs conversion */

#if !defined(CONFIG_NETARM_PLL_BYPASS)
#define NETARM_GEN_TIMER_MSEC_P(x)	( ( ( 20480 ) * ( 0x1FF - ( (x) &	    \
					    NETARM_GEN_TSTAT_CTC_MASK ) +   \
					    1 ) ) / (NETARM_XTAL_FREQ/1000) )

#define NETARM_GEN_TIMER_SET_HZ(x)	( ( ((NETARM_XTAL_FREQ/(20480*(x)))-1) & \
					  NETARM_GEN_TSTAT_CTC_MASK ) | \
					  NETARM_GEN_TCTL_USE_PRESCALE )

#else
#define NETARM_GEN_TIMER_MSEC_P(x)	( ( ( 4096 ) * ( 0x1FF - ( (x) &    \
					    NETARM_GEN_TSTAT_CTC_MASK ) +   \
					    1 ) ) / (NETARM_XTAL_FREQ/1000) )

#define NETARM_GEN_TIMER_SET_HZ(x)	( ( ((NETARM_XTAL_FREQ/(4096*(x)))-1) & \
					  NETARM_GEN_TSTAT_CTC_MASK ) | \
					  NETARM_GEN_TCTL_USE_PRESCALE )
#endif

#endif

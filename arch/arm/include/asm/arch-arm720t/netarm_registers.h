/*
 * linux/include/asm-arm/arch-netarm/netarm_registers.h
 *
 * Copyright (C) 2005
 * Art Shipkowski, Videon Central, Inc., <art@videon-central.com>
 *
 * Copyright (C) 2000, 2001 NETsilicon, Inc.
 * Copyright (C) 2000, 2001 WireSpeed Communications Corporation
 *
 * This software is copyrighted by WireSpeed. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall WireSpeed
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

#ifndef __NET_ARM_REGISTERS_H
#define __NET_ARM_REGISTERS_H

#include <config.h>

/* fundamental constants : */
/* the input crystal/clock frequency ( in Hz ) */
#define	NETARM_XTAL_FREQ_25MHz		(18432000)
#define	NETARM_XTAL_FREQ_33MHz		(23698000)
#define	NETARM_XTAL_FREQ_48MHz		(48000000)
#define	NETARM_XTAL_FREQ_55MHz		(55000000)
#define NETARM_XTAL_FREQ_EMLIN1		(20000000)

/* the frequency of SYS_CLK */
#if defined(CONFIG_NETARM_EMLIN)

/* EMLIN board:  33 MHz (exp.) */
#define	NETARM_PLL_COUNT_VAL		6
#define NETARM_XTAL_FREQ		NETARM_XTAL_FREQ_25MHz

#elif defined(CONFIG_NETARM_NET40_REV2)

/* NET+40 Rev2 boards:  33 MHz (with NETARM_XTAL_FREQ_25MHz) */
#define	NETARM_PLL_COUNT_VAL		6
#define	NETARM_XTAL_FREQ		NETARM_XTAL_FREQ_25MHz

#elif defined(CONFIG_NETARM_NET40_REV4)

/* NET+40 Rev4 boards with EDO must clock slower: 25 MHz (with
   NETARM_XTAL_FREQ_25MHz) 4 */
#define	NETARM_PLL_COUNT_VAL		4
#define	NETARM_XTAL_FREQ		NETARM_XTAL_FREQ_25MHz

#elif defined(CONFIG_NETARM_NET50)

/* NET+50 boards:  40 MHz (with NETARM_XTAL_FREQ_25MHz) */
#define NETARM_PLL_COUNT_VAL		8
#define	NETARM_XTAL_FREQ		NETARM_XTAL_FREQ_25MHz

#else	/* CONFIG_NETARM_NS7520 */

#define	NETARM_PLL_COUNT_VAL		0

#if defined(CONFIG_BOARD_UNC20)
#define	NETARM_XTAL_FREQ		NETARM_XTAL_FREQ_48MHz
#else
#define	NETARM_XTAL_FREQ		NETARM_XTAL_FREQ_55MHz
#endif

#endif

/* #include "arm_registers.h" */
#include <asm/arch/netarm_gen_module.h>
#include <asm/arch/netarm_mem_module.h>
#include <asm/arch/netarm_ser_module.h>
#include <asm/arch/netarm_eni_module.h>
#include <asm/arch/netarm_dma_module.h>
#include <asm/arch/netarm_eth_module.h>

#endif

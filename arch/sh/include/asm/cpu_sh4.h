/*
 * (C) Copyright 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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

#ifndef _ASM_CPU_SH4_H_
#define _ASM_CPU_SH4_H_

/* cache control */
#define CCR_CACHE_STOP   0x00000808
#define CCR_CACHE_ENABLE 0x00000101
#define CCR_CACHE_ICI    0x00000800

#define CACHE_OC_ADDRESS_ARRAY	0xf4000000

#if defined (CONFIG_CPU_SH7750) || \
	defined(CONFIG_CPU_SH7751)
#define CACHE_OC_WAY_SHIFT	14
#define CACHE_OC_NUM_ENTRIES	512
#else
#define CACHE_OC_WAY_SHIFT	13
#define CACHE_OC_NUM_ENTRIES	256
#endif
#define CACHE_OC_ENTRY_SHIFT	5

#if defined (CONFIG_CPU_SH7750) || \
	defined(CONFIG_CPU_SH7751)
# include <asm/cpu_sh7750.h>
#elif defined (CONFIG_CPU_SH7722)
# include <asm/cpu_sh7722.h>
#elif defined (CONFIG_CPU_SH7723)
# include <asm/cpu_sh7723.h>
#elif defined (CONFIG_CPU_SH7724)
# include <asm/cpu_sh7724.h>
#elif defined (CONFIG_CPU_SH7734)
# include <asm/cpu_sh7734.h>
#elif defined (CONFIG_CPU_SH7752)
# include <asm/cpu_sh7752.h>
#elif defined (CONFIG_CPU_SH7757)
# include <asm/cpu_sh7757.h>
#elif defined (CONFIG_CPU_SH7763)
# include <asm/cpu_sh7763.h>
#elif defined (CONFIG_CPU_SH7780)
# include <asm/cpu_sh7780.h>
#elif defined (CONFIG_CPU_SH7785)
# include <asm/cpu_sh7785.h>
#else
# error "Unknown SH4 variant"
#endif

#if defined(CONFIG_SH_32BIT)
#define PMB_ADDR_ARRAY		0xf6100000
#define PMB_ADDR_ENTRY		8
#define PMB_VPN			24

#define PMB_DATA_ARRAY		0xf7100000
#define PMB_DATA_ENTRY		8
#define PMB_PPN			24
#define PMB_UB			9		/* Buffered write */
#define PMB_V			8		/* Valid */
#define PMB_SZ1			7		/* Page size (upper bit) */
#define PMB_SZ0			4		/* Page size (lower bit) */
#define PMB_C			3		/* Cacheability */
#define PMB_WT			0		/* Write-through */

#define PMB_ADDR_BASE(entry)	(PMB_ADDR_ARRAY + (entry << PMB_ADDR_ENTRY))
#define PMB_DATA_BASE(entry)	(PMB_DATA_ARRAY + (entry << PMB_DATA_ENTRY))
#define mk_pmb_addr_val(vpn)	((vpn << PMB_VPN))
#define mk_pmb_data_val(ppn, ub, v, sz1, sz0, c, wt)	\
				((ppn << PMB_PPN) | (ub << PMB_UB) | \
				 (v << PMB_V) | (sz1 << PMB_SZ1) | \
				 (sz0 << PMB_SZ0) | (c << PMB_C) | \
				 (wt << PMB_WT))
#endif

#endif	/* _ASM_CPU_SH4_H_ */

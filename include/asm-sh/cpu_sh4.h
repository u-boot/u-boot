/*
 * (C) Copyright 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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
#define CACHE_OC_WAY_SHIFT	14
#define CACHE_OC_NUM_ENTRIES	512
#define CACHE_OC_ENTRY_SHIFT	5

#if defined (CONFIG_CPU_SH7750)
#include <asm/cpu_sh7750.h>
#elif defined (CONFIG_CPU_SH7722)
#include <asm/cpu_sh7722.h>
#else
#error "Unknown SH4 variant"
#endif

#endif	/* _ASM_CPU_SH4_H_ */

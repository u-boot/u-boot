/*
 * (C) Copyright 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * (C) Copyright 2007 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
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

#ifndef _ASM_CPU_SH3_H_
#define _ASM_CPU_SH3_H_

/* cache control */
#define CCR_CACHE_STOP   0x00000008
#define CCR_CACHE_ENABLE 0x00000005
#define CCR_CACHE_ICI    0x00000008

#define CACHE_OC_ADDRESS_ARRAY	0xf0000000
#define CACHE_OC_WAY_SHIFT	13
#define CACHE_OC_NUM_ENTRIES	256
#define CACHE_OC_ENTRY_SHIFT	4

#if defined(CONFIG_CPU_SH7710)
#include <asm/cpu_sh7710.h>
#elif defined(CONFIG_CPU_SH7720)
#include <asm/cpu_sh7720.h>
#else
#error "Unknown SH3 variant"
#endif

#endif	/* _ASM_CPU_SH3_H_ */

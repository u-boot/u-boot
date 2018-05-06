/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2009 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * (C) Copyright 2007 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
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

#if defined(CONFIG_CPU_SH7706)
#include <asm/cpu_sh7706.h>
#elif defined(CONFIG_CPU_SH7710)
#include <asm/cpu_sh7710.h>
#elif defined(CONFIG_CPU_SH7720)
#include <asm/cpu_sh7720.h>
#else
#error "Unknown SH3 variant"
#endif

#endif	/* _ASM_CPU_SH3_H_ */

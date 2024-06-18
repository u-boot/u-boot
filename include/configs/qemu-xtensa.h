/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007-2013 Tensilica, Inc.
 * Copyright (C) 2014 - 2016 Cadence Design Systems Inc.
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/core.h>
#include <asm/addrspace.h>
#include <asm/config.h>

#if XCHAL_HAVE_PTP_MMU
#define CFG_SYS_MEMORY_BASE		\
	(XCHAL_VECBASE_RESET_VADDR - XCHAL_VECBASE_RESET_PADDR)
#define CFG_SYS_IO_BASE			0xf0000000
#define CFG_SYS_SDRAM_SIZE		0x80000000 /* xtensa.sysram0 */
#else
#define CFG_SYS_MEMORY_BASE		0x60000000
#define CFG_SYS_SDRAM_SIZE		0x08000000 /* xtensa.sysram0 */
#endif

#define CFG_SYS_SDRAM_BASE		MEMADDR(0x00000000)

#if defined(CFG_MAX_MEM_MAPPED) && \
	CFG_MAX_MEM_MAPPED < CFG_SYS_SDRAM_SIZE
#define XTENSA_SYS_TEXT_ADDR		\
	(MEMADDR(CFG_MAX_MEM_MAPPED) - CONFIG_SYS_MONITOR_LEN)
#else
#define XTENSA_SYS_TEXT_ADDR		\
	(MEMADDR(CFG_SYS_SDRAM_SIZE) - CONFIG_SYS_MONITOR_LEN)
#endif

#endif /* __CONFIG_H */

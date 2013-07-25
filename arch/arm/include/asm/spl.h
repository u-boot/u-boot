/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_ASM_SPL_H_
#define	_ASM_SPL_H_

/* Platform-specific defines */
#include <asm/arch/spl.h>

/* Linker symbols. */
extern char __bss_start[], __bss_end[];

extern gd_t gdata;

#endif

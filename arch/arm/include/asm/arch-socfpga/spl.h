/*
 *  Copyright (C) 2012 Pavel Machek <pavel@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SOCFPGA_SPL_H_
#define _SOCFPGA_SPL_H_

/* Symbols from linker script */
extern char __malloc_start, __malloc_end, __stack_start;

#define BOOT_DEVICE_RAM 1

#endif

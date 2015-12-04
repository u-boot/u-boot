/*
 * Copyright 2015,
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_CONFIG_H_
#define _ASM_CONFIG_H_

#define CONFIG_SYS_GENERIC_GLOBAL_DATA
#define CONFIG_NEEDS_MANUAL_RELOC

#define CONFIG_LMB
#define CONFIG_SYS_BOOT_RAMDISK_HIGH

#define CONFIG_SYS_TIMER_RATE		1000000		/* 1MHz */
#define CONFIG_SYS_TIMER_COUNTER	gd->arch.timer
#define CONFIG_SYS_TIMER_COUNTS_DOWN

#endif

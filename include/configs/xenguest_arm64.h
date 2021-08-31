/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2020 EPAM Systemc Inc.
 */
#ifndef __XENGUEST_ARM64_H
#define __XENGUEST_ARM64_H

#ifndef __ASSEMBLY__
#include <linux/types.h>
#endif

#define CONFIG_EXTRA_ENV_SETTINGS

#undef CONFIG_NR_DRAM_BANKS
#undef CONFIG_SYS_SDRAM_BASE

#define CONFIG_NR_DRAM_BANKS          1

/*
 * This can be any arbitrary address as we are using PIE, but
 * please note, that CONFIG_SYS_TEXT_BASE must match the below.
 */
#define CONFIG_LNX_KRNL_IMG_TEXT_OFFSET_BASE    CONFIG_SYS_LOAD_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE             1024
#define CONFIG_SYS_MAXARGS            64
#define CONFIG_SYS_BARGSIZE           CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE             (CONFIG_SYS_CBSIZE + \
				      sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_OF_SYSTEM_SETUP

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"loadimage=ext4load pvblock 0 0x90000000 /boot/Image;\0" \
	"pvblockboot=run loadimage;" \
		"booti 0x90000000 - 0x88000000;\0"

#endif /* __XENGUEST_ARM64_H */

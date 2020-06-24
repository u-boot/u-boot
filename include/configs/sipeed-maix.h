/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */

#ifndef CONFIGS_SIPEED_MAIX_H
#define CONFIGS_SIPEED_MAIX_H

#include <linux/sizes.h>

#define CONFIG_SYS_LOAD_ADDR 0x80000000
/* Start just below the second bank so we don't clobber it during reloc */
#define CONFIG_SYS_INIT_SP_ADDR 0x803FFFFF
#define CONFIG_SYS_MALLOC_LEN SZ_128K
#define CONFIG_SYS_CACHELINE_SIZE 64

#define CONFIG_SYS_SDRAM_BASE 0x80000000
/* Don't relocate into AI ram since it isn't set up yet */
#define CONFIG_SYS_SDRAM_SIZE (SZ_4M + SZ_2M)

/* For early init */
#define K210_SYSCTL_BASE 0x50440000

#endif /* CONFIGS_SIPEED_MAIX_H */

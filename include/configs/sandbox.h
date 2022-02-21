/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifndef CONFIG_SPL_BUILD
#define CONFIG_IO_TRACE
#endif

#define CONFIG_MALLOC_F_ADDR		0x0010000

#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */

/* Size of our emulated memory */
#define SB_CONCAT(x, y) x ## y
#define SB_TO_UL(s) SB_CONCAT(s, UL)
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE \
		(SB_TO_UL(CONFIG_SANDBOX_RAM_SIZE_MB) << 20)
#define CONFIG_SYS_MONITOR_BASE	0

#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

#ifndef SANDBOX_NO_SDL
#define CONFIG_SANDBOX_SDL
#endif

#endif

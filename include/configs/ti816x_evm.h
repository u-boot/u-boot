/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ti816x_evm.h
 *
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Antoine Tenart, <atenart@adeneo-embedded.com>
 */

#ifndef __CONFIG_TI816X_EVM_H
#define __CONFIG_TI816X_EVM_H

#include <configs/ti_armv7_omap.h>
#include <asm/arch/omap.h>

#define CONFIG_EXTRA_ENV_SETTINGS	\
	DEFAULT_LINUX_BOOT_ENV

/* Clock Defines */
#define V_OSCK          24000000    /* Clock output from T2 */
#define V_SCLK          (V_OSCK >> 1)

#define CONFIG_MAX_RAM_BANK_SIZE	(2048 << 20)	/* 2048MB */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

/**
 * Platform/Board specific defs
 */
#define CONFIG_SYS_TIMERBASE    0x4802E000

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE (-4)
#define CONFIG_SYS_NS16550_CLK      (48000000)
#define CONFIG_SYS_NS16550_COM1     0x48024000  /* Base EVM has UART2 */

/* allow overwriting serial config and ethaddr */


/*
 * GPMC NAND block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#define CONFIG_SYS_NAND_BASE		0x8000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* NAND: SPL related configs */

/* NAND: device related configs */
/* NAND: driver related configs */
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14

/* SPL */
/* Defines for SPL */

#endif

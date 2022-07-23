/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the SAMA5D3xEK board.
 *
 * Copyright (C) 2012 - 2013 Atmel
 *
 * based on at91sam9m10g45ek.h by:
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

/*
 * This needs to be defined for the OHCI code to work but it is defined as
 * ATMEL_ID_UHPHS in the CPU specific header files.
 */
#define ATMEL_ID_UHP			32

/*
 * Specify the clock enable bit in the PMC_SCER register.
 */
#define ATMEL_PMC_UHP			(1 <<  6)

/* board specific (not enough SRAM) */
#define CONFIG_SAMA5D3_LCD_BASE		0x23E00000

/* NOR flash */
#ifdef CONFIG_MTD_NOR_FLASH
#define CONFIG_SYS_FLASH_BASE		0x10000000
#endif

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE           0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x20000000

/* SerialFlash */

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x60000000
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#endif

/* SPL */

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#endif

/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX7ULP EVK board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MX7ULP_EVK_CONFIG_H
#define __MX7ULP_EVK_CONFIG_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

/*Uncomment it to use secure boot*/
/*#define CONFIG_SECURE_BOOT*/

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE			0x4000
#endif
#endif

#define CONFIG_BOARD_POSTCLK_INIT
#define CONFIG_SYS_BOOTM_LEN		0x1000000

#define SRC_BASE_ADDR			CMC1_RBASE
#define IRAM_BASE_ADDR			OCRAM_0_BASE
#define IOMUXC_BASE_ADDR		IOMUXC1_RBASE

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			SZ_8K

#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG1_RBASE

#define CONFIG_SYS_ARCH_TIMER
#define CONFIG_SYS_HZ_CLOCK		1000000 /* Fixed at 1Mhz from TSTMR */

#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
/*#define CONFIG_REVISION_TAG*/

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(8 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F

/* UART */
#define LPUART_BASE			LPUART4_RBASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

#undef CONFIG_CMD_IMLS
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE

#define CONFIG_SYS_CACHELINE_SIZE      64

/* Miscellaneous configurable options */
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		512

/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		256
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_CMDLINE_EDITING
#define CONFIG_STACKSIZE		SZ_8K

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_TEXT_BASE		0x67800000
#define PHYS_SDRAM			0x60000000
#define PHYS_SDRAM_SIZE			SZ_1G
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_CMD_BOOTZ

#define CONFIG_LOADADDR             0x60800000

#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_END      0x9E000000

#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	SZ_256K

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

#endif	/* __CONFIG_H */

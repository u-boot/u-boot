/*
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * Configuration settings for the ATSTK1002 CPU daughterboard
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/hardware.h>

#define CONFIG_AT32AP
#define CONFIG_AT32AP7000
#define CONFIG_ATSTK1002
#define CONFIG_ATSTK1000

/*
 * Set up the PLL to run at 140 MHz, the CPU to run at the PLL
 * frequency, the HSB and PBB at 1/2, and the PBA to run at 1/4 the
 * PLL frequency.
 * (CONFIG_SYS_OSC0_HZ * CONFIG_SYS_PLL0_MUL) / CONFIG_SYS_PLL0_DIV = PLL MHz
 */
#define CONFIG_PLL
#define CONFIG_SYS_POWER_MANAGER
#define CONFIG_SYS_OSC0_HZ			20000000
#define CONFIG_SYS_PLL0_DIV			1
#define CONFIG_SYS_PLL0_MUL			7
#define CONFIG_SYS_PLL0_SUPPRESS_CYCLES	16
/*
 * Set the CPU running at:
 * PLL / (2^CONFIG_SYS_CLKDIV_CPU) = CPU MHz
 */
#define CONFIG_SYS_CLKDIV_CPU			0
/*
 * Set the HSB running at:
 * PLL / (2^CONFIG_SYS_CLKDIV_HSB) = HSB MHz
 */
#define CONFIG_SYS_CLKDIV_HSB			1
/*
 * Set the PBA running at:
 * PLL / (2^CONFIG_SYS_CLKDIV_PBA) = PBA MHz
 */
#define CONFIG_SYS_CLKDIV_PBA			2
/*
 * Set the PBB running at:
 * PLL / (2^CONFIG_SYS_CLKDIV_PBB) = PBB MHz
 */
#define CONFIG_SYS_CLKDIV_PBB			1

/* Reserve VM regions for SDRAM and NOR flash */
#define CONFIG_SYS_NR_VM_REGIONS		2

/*
 * The PLLOPT register controls the PLL like this:
 *   icp = PLLOPT<2>
 *   ivco = PLLOPT<1:0>
 *
 * We want icp=1 (default) and ivco=0 (80-160 MHz) or ivco=2 (150-240MHz).
 */
#define CONFIG_SYS_PLL0_OPT			0x04

#define CONFIG_USART_BASE		ATMEL_BASE_USART1
#define CONFIG_USART_ID			1

/* User serviceable stuff */
#define CONFIG_DOS_PARTITION

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_STACKSIZE		(2048)

#define CONFIG_BAUDRATE			115200
#define CONFIG_BOOTARGS							\
	"console=ttyS0 root=/dev/mmcblk0p1 fbmem=600k rootwait=1"

#define CONFIG_BOOTCOMMAND						\
	"fsload; bootm $(fileaddr)"

#define CONFIG_BOOTDELAY		1

/*
 * After booting the board for the first time, new ethernet addresses
 * should be generated and assigned to the environment variables
 * "ethaddr" and "eth1addr". This is normally done during production.
 */
#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY

/* generic board */
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R

/*
 * Command line configuration.
 */
#define CONFIG_CMD_JFFS2

#define CONFIG_ATMEL_USART
#define CONFIG_MACB
#define CONFIG_PORTMUX_PIO
#define CONFIG_SYS_NR_PIOS			5
#define CONFIG_SYS_HSDRAMC
#define CONFIG_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#define CONFIG_GENERIC_MMC

#define CONFIG_SYS_DCACHE_LINESZ		32
#define CONFIG_SYS_ICACHE_LINESZ		32

#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER

#define CONFIG_SYS_FLASH_BASE			0x00000000
#define CONFIG_SYS_FLASH_SIZE			0x800000
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_MAX_FLASH_SECT		135

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_TEXT_BASE		0x00000000

#define CONFIG_SYS_INTRAM_BASE			INTERNAL_SRAM_BASE
#define CONFIG_SYS_INTRAM_SIZE			INTERNAL_SRAM_SIZE
#define CONFIG_SYS_SDRAM_BASE			EBI_SDRAM_BASE

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SIZE			65536
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_SIZE - CONFIG_ENV_SIZE)

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INTRAM_BASE + CONFIG_SYS_INTRAM_SIZE)

#define CONFIG_SYS_MALLOC_LEN			(256*1024)

/* Allow 4MB for the kernel run-time image */
#define CONFIG_SYS_LOAD_ADDR			(EBI_SDRAM_BASE + 0x00400000)
#define CONFIG_SYS_BOOTPARAMS_LEN		(16 * 1024)

/* Other configuration settings that shouldn't have to change all that often */
#define CONFIG_SYS_CBSIZE			256
#define CONFIG_SYS_MAXARGS			16
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP

#define CONFIG_SYS_MEMTEST_START		EBI_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END			(CONFIG_SYS_MEMTEST_START + 0x700000)
#define CONFIG_SYS_BAUDRATE_TABLE { 115200, 38400, 19200, 9600, 2400 }

#endif /* __CONFIG_H */

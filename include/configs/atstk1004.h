/*
 * Copyright (C) 2007 Atmel Corporation
 *
 * Configuration settings for the ATSTK1003 CPU daughterboard
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/memory-map.h>

#define CONFIG_AVR32			1
#define CONFIG_AT32AP			1
#define CONFIG_AT32AP7002		1
#define CONFIG_ATSTK1004		1
#define CONFIG_ATSTK1000		1

#define CONFIG_ATSTK1000_EXT_FLASH	1

/*
 * Timer clock frequency. We're using the CPU-internal COUNT register
 * for this, so this is equivalent to the CPU core clock frequency
 */
#define CONFIG_SYS_HZ				1000

/*
 * Set up the PLL to run at 140 MHz, the CPU to run at the PLL
 * frequency, the HSB and PBB at 1/2, and the PBA to run at 1/4 the
 * PLL frequency.
 * (CONFIG_SYS_OSC0_HZ * CONFIG_SYS_PLL0_MUL) / CONFIG_SYS_PLL0_DIV = PLL MHz
 */
#define CONFIG_PLL			1
#define CONFIG_SYS_POWER_MANAGER		1
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

/*
 * The PLLOPT register controls the PLL like this:
 *   icp = PLLOPT<2>
 *   ivco = PLLOPT<1:0>
 *
 * We want icp=1 (default) and ivco=0 (80-160 MHz) or ivco=2 (150-240MHz).
 */
#define CONFIG_SYS_PLL0_OPT			0x04

#undef CONFIG_USART0
#define CONFIG_USART1			1
#undef CONFIG_USART2
#undef CONFIG_USART3

/* User serviceable stuff */
#define CONFIG_DOS_PARTITION		1

#define CONFIG_CMDLINE_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

#define CONFIG_STACKSIZE		(2048)

#define CONFIG_BAUDRATE			115200
#define CONFIG_BOOTARGS							\
	"console=ttyS0 root=/dev/mmcblk0p1 rootwait"

#define CONFIG_BOOTCOMMAND						\
	"mmcinit; ext2load mmc 0:1 0x10200000 /boot/uImage; bootm"

/*
 * Only interrupt autoboot if <space> is pressed. Otherwise, garbage
 * data on the serial line may interrupt the boot sequence.
 */
#define CONFIG_BOOTDELAY		1
#define CONFIG_AUTOBOOT			1
#define CONFIG_AUTOBOOT_KEYED		1
#define CONFIG_AUTOBOOT_PROMPT		\
	"Press SPACE to abort autoboot in %d seconds\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR	"d"
#define CONFIG_AUTOBOOT_STOP_STR	" "

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MMC

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_XIMG

#define CONFIG_ATMEL_USART		1
#define CONFIG_PORTMUX_PIO		1
#define CONFIG_SYS_HSDRAMC			1
#define CONFIG_MMC			1
#define CONFIG_ATMEL_MCI		1

#define CONFIG_SYS_DCACHE_LINESZ		32
#define CONFIG_SYS_ICACHE_LINESZ		32

#define CONFIG_NR_DRAM_BANKS		1

/* External flash on STK1000 */
#if 0
#define CONFIG_SYS_FLASH_CFI			1
#define CONFIG_FLASH_CFI_DRIVER		1
#endif

#define CONFIG_SYS_FLASH_BASE			0x00000000
#define CONFIG_SYS_FLASH_SIZE			0x800000
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_MAX_FLASH_SECT		135

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_INTRAM_BASE			INTERNAL_SRAM_BASE
#define CONFIG_SYS_INTRAM_SIZE			INTERNAL_SRAM_SIZE
#define CONFIG_SYS_SDRAM_BASE			EBI_SDRAM_BASE

#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SIZE			65536
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_SIZE - CONFIG_ENV_SIZE)

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INTRAM_BASE + CONFIG_SYS_INTRAM_SIZE)

#define CONFIG_SYS_MALLOC_LEN			(256*1024)

/* Allow 2MB for the kernel run-time image */
#define CONFIG_SYS_LOAD_ADDR			(EBI_SDRAM_BASE + 0x00200000)
#define CONFIG_SYS_BOOTPARAMS_LEN		(16 * 1024)

/* Other configuration settings that shouldn't have to change all that often */
#define CONFIG_SYS_PROMPT			"U-Boot> "
#define CONFIG_SYS_CBSIZE			256
#define CONFIG_SYS_MAXARGS			16
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP			1

#define CONFIG_SYS_MEMTEST_START		EBI_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END			(CONFIG_SYS_MEMTEST_START + 0x700000)
#define CONFIG_SYS_BAUDRATE_TABLE { 115200, 38400, 19200, 9600, 2400 }

#endif /* __CONFIG_H */

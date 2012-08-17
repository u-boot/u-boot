/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * Configuration settings for the AVR32 Network Gateway
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

#include <asm/arch/hardware.h>

#define CONFIG_AVR32
#define CONFIG_AT32AP
#define CONFIG_AT32AP7000
#define CONFIG_MIMC200

#define CONFIG_MIMC200_EXT_FLASH

#define CONFIG_SYS_HZ				1000

/*
 * Set up the PLL to run at 140 MHz, the CPU to run at the PLL
 * frequency, the HSB and PBB busses to run at 1/2 the PLL frequency
 * and the PBA bus to run at 1/4 the PLL frequency.
 */
#define CONFIG_PLL
#define CONFIG_SYS_POWER_MANAGER
#define CONFIG_SYS_OSC0_HZ			10000000
#define CONFIG_SYS_PLL0_DIV			1
#define CONFIG_SYS_PLL0_MUL			15
#define CONFIG_SYS_PLL0_SUPPRESS_CYCLES	16
#define CONFIG_SYS_CLKDIV_CPU			0
#define CONFIG_SYS_CLKDIV_HSB			1
#define CONFIG_SYS_CLKDIV_PBA			2
#define CONFIG_SYS_CLKDIV_PBB			1

/* Reserve VM regions for SDRAM, NOR flash and FRAM */
#define CONFIG_SYS_NR_VM_REGIONS		3

/*
 * The PLLOPT register controls the PLL like this:
 *   icp = PLLOPT<2>
 *   ivco = PLLOPT<1:0>
 *
 * We want icp=1 (default) and ivco=0 (80-160 MHz) or ivco=2 (150-240MHz).
 */
#define CONFIG_SYS_PLL0_OPT			0x04

#define CONFIG_USART_BASE			ATMEL_BASE_USART1
#define CONFIG_USART_ID				1

#define CONFIG_MIMC200_DBGLINK		1

/* User serviceable stuff */
#define CONFIG_DOS_PARTITION

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_STACKSIZE		(2048)

#define CONFIG_BAUDRATE			115200
#define CONFIG_BOOTARGS							\
	"root=/dev/mtdblock1 rootfstype=jffs2 fbmem=512k console=ttyS1"
#define CONFIG_BOOTCOMMAND						\
	"fsload boot/uImage; bootm"

#define CONFIG_SILENT_CONSOLE       /* enable silent startup */
#define CONFIG_DISABLE_CONSOLE      /* disable console */
#define CONFIG_SYS_DEVICE_NULLDEV   /* include nulldev device */

#define CONFIG_LCD			1

/*
 * Only interrupt autoboot if <space> is pressed. Otherwise, garbage
 * data on the serial line may interrupt the boot sequence.
 */
#define CONFIG_BOOTDELAY		0
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_AUTOBOOT

/*
 * After booting the board for the first time, new ethernet addresses
 * should be generated and assigned to the environment variables
 * "ethaddr" and "eth1addr". This is normally done during production.
 */
#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*
 * BOOTP/DHCP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NET

#define CONFIG_ATMEL_USART
#define CONFIG_MACB
#define CONFIG_PORTMUX_PIO
#define CONFIG_SYS_NR_PIOS			5
#define CONFIG_SYS_HSDRAMC
#define CONFIG_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#define CONFIG_GENERIC_MMC
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 1

#if defined(CONFIG_LCD)
#define CONFIG_CMD_BMP
#define CONFIG_ATMEL_LCD		1
#define LCD_BPP				LCD_COLOR16
#define CONFIG_BMP_16BPP		1
#define CONFIG_FB_ADDR			0x10600000
#define CONFIG_WHITE_ON_BLACK		1
#define CONFIG_VIDEO_BMP_GZIP 		1
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE		262144
#define CONFIG_ATMEL_LCD_BGR555		1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1
#define CONFIG_SPLASH_SCREEN		1
#endif

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

#define CONFIG_SYS_FRAM_BASE			0x08000000
#define CONFIG_SYS_FRAM_SIZE			0x20000

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SIZE			65536
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_SIZE - CONFIG_ENV_SIZE)

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INTRAM_BASE + CONFIG_SYS_INTRAM_SIZE)

#define CONFIG_SYS_MALLOC_LEN			(1024*1024)
#define CONFIG_SYS_DMA_ALLOC_LEN		(16384)

/* Allow 4MB for the kernel run-time image */
#define CONFIG_SYS_LOAD_ADDR			(EBI_SDRAM_BASE + 0x00400000)
#define CONFIG_SYS_BOOTPARAMS_LEN		(16 * 1024)

/* Other configuration settings that shouldn't have to change all that often */
#define CONFIG_SYS_PROMPT			"U-Boot> "
#define CONFIG_SYS_CBSIZE			256
#define CONFIG_SYS_MAXARGS			16
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP

#define CONFIG_SYS_MEMTEST_START		EBI_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END			(CONFIG_SYS_MEMTEST_START + 0x1f00000)

#define CONFIG_SYS_BAUDRATE_TABLE { 115200, 38400, 19200, 9600, 2400 }

#endif /* __CONFIG_H */

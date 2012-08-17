/*
 * (c) 2011 Graf-Syteco, Matthias Weisser
 * <weisserm@arcor.de>
 *
 * Configuation settings for the zmx25 board
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARM926EJS			/* arm926ejs CPU core */
#define CONFIG_MX25
#define CONFIG_MX25_CLK32		32768	/* OSC32K frequency */
#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_TEXT_BASE		0xA0000000

#define CONFIG_MACH_TYPE	MACH_TYPE_ZMX25
/*
 * Environment settings
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"gs_fast_boot=setenv bootdelay 5\0" \
	"gs_slow_boot=setenv bootdelay 10\0" \
	"bootcmd=dcache off; mw.l 0x81000000 0 1024; usb start;" \
		"fatls usb 0; fatload usb 0 0x81000000 zmx25-init.bin;" \
		"bootm 0x81000000; bootelf 0x81000000\0"

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_BOARD_LATE_INIT

/*
 * Compressions
 */
#define CONFIG_LZO

/*
 * Hardware drivers
 */

/*
 * GPIO
 */
#define CONFIG_MXC_GPIO

/*
 * Serial
 */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART2_BASE
#define CONFIG_CONS_INDEX	1	/* use UART2 for console */
#define CONFIG_BAUDRATE		115200	/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Ethernet
 */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_PHYADDR		0x00
#define CONFIG_MII

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_NET
#define CONFIG_CMD_CACHE

#define CONFIG_SYS_64BIT_VSPRINTF

/*
 * Additional command
 */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "

/*
 * USB
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI			/* Enable EHCI USB support */
#define CONFIG_USB_EHCI_MXC
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORT	2
#define CONFIG_MXC_USB_PORTSC	0xC0000000
#define CONFIG_MXC_USB_FLAGS	0
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT
#endif /* CONFIG_CMD_USB */

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM		0x80000000	/* start address of LPDDRRAM */
#define PHYS_SDRAM_SIZE		0x04000000	/* 64 megs */

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM
#define CONFIG_SYS_INIT_SP_ADDR	0x78020000	/* end of internal SRAM */

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		0xA0000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	256

#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x00040000)
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SECT_SIZE		(128 * 1024)
#define CONFIG_ENV_SIZE			(128 * 1024)

/*
 * CFI FLASH driver setup
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	/* ~10x faster */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM + (512*1024))
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM + PHYS_SDRAM_SIZE)

#define CONFIG_SYS_PROMPT	"zmx25> "
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
				sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING

#define CONFIG_PREBOOT  ""

#define CONFIG_BOOTDELAY	5
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT "boot in %d s\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR	"delaygs"
#define CONFIG_AUTOBOOT_STOP_STR	"stopgs"

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(0x400000 - 0x8000)
#define CONFIG_STACKSIZE		(32*1024)	/* regular stack */

#endif	/* __CONFIG_H */

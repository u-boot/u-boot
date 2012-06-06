/*
 * Configuation settings for the sh7757lcr board
 *
 * Copyright (C) 2011 Renesas Solutions Corp.
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

#ifndef __SH7757LCR_H
#define __SH7757LCR_H

#undef DEBUG
#define CONFIG_SH		1
#define CONFIG_SH4A		1
#define CONFIG_SH_32BIT		1
#define CONFIG_CPU_SH7757	1
#define CONFIG_SH7757LCR	1

#define CONFIG_SYS_TEXT_BASE	0x8ef80000
#define CONFIG_SYS_LDSCRIPT	"board/renesas/sh7757lcr/u-boot.lds"

#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_NFS
#define CONFIG_CMD_DFL
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SF
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_MD5SUM
#define CONFIG_MD5
#define CONFIG_CMD_LOADS

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"console=ttySC2,115200 root=/dev/nfs ip=dhcp"

#define CONFIG_VERSION_VARIABLE
#undef	CONFIG_SHOW_BOOT_PROGRESS

/* MEMORY */
#define SH7757LCR_SDRAM_BASE		(0x80000000)
#define SH7757LCR_SDRAM_SIZE		(240 * 1024 * 1024)
#define SH7757LCR_SDRAM_ECC_SETTING	0x0f000000	/* 240MByte */
#define SH7757LCR_SDRAM_DVC_SIZE	(16 * 1024 * 1024)

#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		512
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }

/* SCIF */
#define CONFIG_SCIF_CONSOLE	1
#define CONFIG_CONS_SCIF2	1
#undef	CONFIG_SYS_CONSOLE_INFO_QUIET
#undef	CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#undef	CONFIG_SYS_CONSOLE_ENV_OVERWRITE

#define CONFIG_SYS_MEMTEST_START	(SH7757LCR_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					 224 * 1024 * 1024)
#undef	CONFIG_SYS_ALT_MEMTEST
#undef	CONFIG_SYS_MEMTEST_SCRATCH
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_SDRAM_BASE		(SH7757LCR_SDRAM_BASE)
#define CONFIG_SYS_SDRAM_SIZE		(SH7757LCR_SDRAM_SIZE)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + \
					 (128 + 16) * 1024 * 1024)

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

/* FLASH */
#define CONFIG_SYS_NO_FLASH

/* Ether */
#define CONFIG_NET_MULTI		1
#define CONFIG_SH_ETHER			1
#define CONFIG_SH_ETHER_USE_PORT	0
#define CONFIG_SH_ETHER_PHY_ADDR	1
#define CONFIG_SH_ETHER_CACHE_WRITEBACK	1

#define SH7757LCR_ETHERNET_MAC_BASE_SPI	0x000b0000
#define SH7757LCR_SPI_SECTOR_SIZE	(64 * 1024)
#define SH7757LCR_ETHERNET_MAC_BASE	SH7757LCR_ETHERNET_MAC_BASE_SPI
#define SH7757LCR_ETHERNET_MAC_SIZE	17
#define SH7757LCR_ETHERNET_NUM_CH	2
#define BOARD_LATE_INIT			1

/* Gigabit Ether */
#define SH7757LCR_GIGA_ETHERNET_NUM_CH	2

/* SPI */
#define CONFIG_SH_SPI			1
#define CONFIG_SH_SPI_BASE		0xfe002000
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO	1

/* SH7757 board */
#define SH7757LCR_SDRAM_PHYS_TOP	0x40000000
#define SH7757LCR_GRA_OFFSET		0x1f000000
#define SH7757LCR_PCIEBRG_ADDR_B0	0x000a0000
#define SH7757LCR_PCIEBRG_SIZE_B0	(64 * 1024)
#define SH7757LCR_PCIEBRG_ADDR		0x00090000
#define SH7757LCR_PCIEBRG_SIZE		(96 * 1024)

/* ENV setting */
#define CONFIG_ENV_IS_EMBEDDED
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE	(64 * 1024)
#define CONFIG_ENV_ADDR		(0x00080000)
#define CONFIG_ENV_OFFSET	(CONFIG_ENV_ADDR)
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SECT_SIZE)
#define CONFIG_EXTRA_ENV_SETTINGS				\
		"netboot=bootp; bootm\0"

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	48000000
#define CONFIG_SYS_TMU_CLK_DIV	4
#define CONFIG_SYS_HZ		1000
#endif	/* __SH7757LCR_H */

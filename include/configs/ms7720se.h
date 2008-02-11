/*
 * Configuation settings for the Hitachi Solution Engine 7720
 *
 * Copyright (C) 2007 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
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

#ifndef __MS7720SE_H
#define __MS7720SE_H

#define CONFIG_SH		1
#define CONFIG_SH3		1
#define CONFIG_CPU_SH7720	1
#define CONFIG_MS7720SE		1

#define CONFIG_CMD_FLASH
#define CONFIG_CMD_ENV
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_PCMCIA
#define CONFIG_CMD_IDE
#define CONFIG_CMD_EXT2

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTARGS		"console=ttySC0,115200"
#define CONFIG_BOOTFILE		/boot/zImage
#define CONFIG_LOADADDR		0x8E000000

#define CONFIG_VERSION_VARIABLE
#undef  CONFIG_SHOW_BOOT_PROGRESS

/* MEMORY */
#define MS7720SE_SDRAM_BASE		0x8C000000
#define MS7720SE_FLASH_BASE_1		0xA0000000
#define MS7720SE_FLASH_BANK_SIZE	(8 * 1024 * 1024)

#define CFG_LONGHELP		/* undef to save memory	*/
#define CFG_PROMPT	"=> "	/* Monitor Command Prompt */
#define CFG_CBSIZE	256	/* Buffer size for input from the Console */
#define CFG_PBSIZE	256	/* Buffer size for Console output */
#define CFG_MAXARGS	16	/* max args accepted for monitor commands */
/* Buffer size for Boot Arguments passed to kernel */
#define CFG_BARGSIZE	512
/* List of legal baudrate settings for this board */
#define CFG_BAUDRATE_TABLE	{ 115200 }

/* SCIF */
#define CFG_SCIF_CONSOLE	1
#define CONFIG_CONS_SCIF0	1

#define CFG_MEMTEST_START	MS7720SE_SDRAM_BASE
#define CFG_MEMTEST_END		(CFG_MEMTEST_START + (60 * 1024 * 1024))

#define CFG_SDRAM_BASE		MS7720SE_SDRAM_BASE
#define CFG_SDRAM_SIZE		(64 * 1024 * 1024)

#define CFG_LOAD_ADDR		(CFG_SDRAM_BASE + 32 * 1024 * 1024)
#define CFG_MONITOR_BASE	MS7720SE_FLASH_BASE_1
#define CFG_MONITOR_LEN		(128 * 1024)
#define CFG_MALLOC_LEN		(256 * 1024)
#define CFG_GBL_DATA_SIZE	256
#define CFG_BOOTMAPSZ		(8 * 1024 * 1024)


/* FLASH */
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#undef  CFG_FLASH_QUIET_TEST
#define CFG_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */

#define CFG_FLASH_BASE		MS7720SE_FLASH_BASE_1

#define CFG_MAX_FLASH_SECT	150
#define CFG_MAX_FLASH_BANKS	1
#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	(64 * 1024)
#define CFG_ENV_SIZE		CFG_ENV_SECT_SIZE
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_FLASH_ERASE_TOUT  	120000
#define CFG_FLASH_WRITE_TOUT	500

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	33333333
#define TMU_CLK_DIVIDER		4	/* 4 (default), 16, 64, 256 or 1024 */
#define CFG_HZ			(CONFIG_SYS_CLK_FREQ / TMU_CLK_DIVIDER)

/* PCMCIA */
#define CONFIG_IDE_PCMCIA	1
#define CONFIG_MARUBUN_PCCARD	1
#define CONFIG_PCMCIA_SLOT_A	1
#define CFG_IDE_MAXDEVICE	1
#define CFG_MARUBUN_MRSHPC	0xb83fffe0
#define CFG_MARUBUN_MW1		0xb8400000
#define CFG_MARUBUN_MW2		0xb8500000
#define CFG_MARUBUN_IO		0xb8600000

#define CFG_PIO_MODE		1
#define CFG_IDE_MAXBUS		1
#define CONFIG_DOS_PARTITION	1
#define CFG_ATA_BASE_ADDR	CFG_MARUBUN_IO	/* base address */
#define CFG_ATA_IDE0_OFFSET	0x01F0		/* ide0 offste */
#define CFG_ATA_DATA_OFFSET	0		/* data reg offset */
#define CFG_ATA_REG_OFFSET	0		/* reg offset */
#define CFG_ATA_ALT_OFFSET	0x200		/* alternate register offset */

#endif	/* __MS7720SE_H */

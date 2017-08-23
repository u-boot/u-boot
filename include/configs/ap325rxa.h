/*
 * Configuation settings for the Renesas Solutions AP-325RXA board
 *
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AP325RXA_H
#define __AP325RXA_H

#define CONFIG_CPU_SH7723	1
#define CONFIG_AP325RXA	1

#define CONFIG_DISPLAY_BOARDINFO
#undef  CONFIG_SHOW_BOOT_PROGRESS

/* SMC9118 */
#define CONFIG_SMC911X 1
#define CONFIG_SMC911X_32_BIT 1
#define CONFIG_SMC911X_BASE 0xB6080000

/* MEMORY */
#define AP325RXA_SDRAM_BASE		(0x88000000)
#define AP325RXA_FLASH_BASE_1		(0xA0000000)
#define AP325RXA_FLASH_BANK_SIZE	(128 * 1024 * 1024)

#define CONFIG_SYS_TEXT_BASE	0x8FFC0000

/* undef to save memory	*/
#define CONFIG_SYS_LONGHELP
/* Monitor Command Prompt */
/* Buffer size for Console output */
#define CONFIG_SYS_PBSIZE		256
/* List of legal baudrate settings for this board */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 38400 }

/* SCIF */
#define CONFIG_SCIF_A		1 /* SH7723 has SCIF and SCIFA */
#define CONFIG_CONS_SCIF5	1

/* Suppress display of console information at boot */

#define CONFIG_SYS_MEMTEST_START	(AP325RXA_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (60 * 1024 * 1024))

/* Enable alternate, more extensive, memory test */
#undef  CONFIG_SYS_ALT_MEMTEST
/* Scratch address used by the alternate memory test */
#undef  CONFIG_SYS_MEMTEST_SCRATCH

/* Enable temporary baudrate change while serial download */
#undef  CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_SDRAM_BASE	(AP325RXA_SDRAM_BASE)
/* maybe more, but if so u-boot doesn't know about it... */
#define CONFIG_SYS_SDRAM_SIZE	(128 * 1024 * 1024)
/* default load address for scripts ?!? */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 * 1024)

/* Address of u-boot image in Flash (NOT run time address in SDRAM) ?!? */
#define CONFIG_SYS_MONITOR_BASE	(AP325RXA_FLASH_BASE_1)
/* Monitor size */
#define CONFIG_SYS_MONITOR_LEN	(128 * 1024)
/* Size of DRAM reserved for malloc() use */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024)
#define CONFIG_SYS_BOOTMAPSZ	(8 * 1024 * 1024)

/* FLASH */
#define CONFIG_FLASH_CFI_DRIVER 1
#define CONFIG_SYS_FLASH_CFI
#undef  CONFIG_SYS_FLASH_QUIET_TEST
/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_EMPTY_INFO
/* Physical start address of Flash memory */
#define CONFIG_SYS_FLASH_BASE	(AP325RXA_FLASH_BASE_1)
/* Max number of sectors on each Flash chip */
#define CONFIG_SYS_MAX_FLASH_SECT	512

/*
 * IDE support
 */
#define CONFIG_IDE_RESET	1
#define CONFIG_SYS_PIO_MODE		1
#define CONFIG_SYS_IDE_MAXBUS		1	/* IDE bus */
#define CONFIG_SYS_IDE_MAXDEVICE	1
#define CONFIG_SYS_ATA_BASE_ADDR	0xB4180000
#define CONFIG_SYS_ATA_STRIDE		2	/* 1bit shift */
#define CONFIG_SYS_ATA_DATA_OFFSET	0x200	/* data reg offset */
#define CONFIG_SYS_ATA_REG_OFFSET	0x200	/* reg offset */
#define CONFIG_SYS_ATA_ALT_OFFSET	0x210	/* alternate register offset */
#define CONFIG_IDE_SWAP_IO

/* if you use all NOR Flash , you change dip-switch. Please see Manual. */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE + (0 * AP325RXA_FLASH_BANK_SIZE)}

/* Timeout for Flash erase operations (in ms) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(3 * 1000)
/* Timeout for Flash write operations (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(3 * 1000)
/* Timeout for Flash set sector lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_LOCK_TOUT	(3 * 1000)
/* Timeout for Flash clear lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	(3 * 1000)

/*
 * Use hardware flash sectors protection instead
 * of U-Boot software protection
 */
#undef  CONFIG_SYS_FLASH_PROTECTION
#undef  CONFIG_SYS_DIRECT_FLASH_TFTP

/* ENV setting */
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
/* Offset of env Flash sector relative to CONFIG_SYS_FLASH_BASE */
#define CONFIG_ENV_OFFSET		(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SECT_SIZE)

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	33333333
#define CONFIG_SH_TMU_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define CONFIG_SH_SCIF_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_TMU_CLK_DIV		(4)	/* 4 (default), 16, 64, 256 or 1024 */

#endif	/* __AP325RXA_H */

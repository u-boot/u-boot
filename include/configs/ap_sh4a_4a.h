/*
 * Configuation settings for the Alpha Project AP-SH4A-4A board
 *
 * Copyright (C) 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AP_SH4A_4A_H
#define __AP_SH4A_4A_H

#define CONFIG_CPU_SH7734	1
#define CONFIG_AP_SH4A_4A	1
#define CONFIG_400MHZ_MODE	1
/* #define CONFIG_533MHZ_MODE	1 */

#define CONFIG_SYS_TEXT_BASE 0x8BFC0000

#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_ENV

#define CONFIG_BOOTARGS		"console=ttySC4,115200"

#define CONFIG_DISPLAY_BOARDINFO
#undef  CONFIG_SHOW_BOOT_PROGRESS

/* Ether */
#define CONFIG_SH_ETHER 1
#define CONFIG_SH_ETHER_USE_PORT (0)
#define CONFIG_SH_ETHER_PHY_ADDR (0x0)
#define CONFIG_SH_ETHER_PHY_MODE (PHY_INTERFACE_MODE_GMII)
#define CONFIG_SH_ETHER_SH7734_MII (0x02) /* GMII */
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL 1
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* I2C */
#define CONFIG_SH_SH7734_I2C	1
#define CONFIG_HARD_I2C			1
#define CONFIG_I2C_MULTI_BUS	1
#define CONFIG_SYS_MAX_I2C_BUS	2
#define CONFIG_SYS_I2C_MODULE	0
#define CONFIG_SYS_I2C_SPEED	400000 /* 400 kHz */
#define CONFIG_SYS_I2C_SLAVE	0x50
#define CONFIG_SH_I2C_DATA_HIGH	4
#define CONFIG_SH_I2C_DATA_LOW	5
#define CONFIG_SH_I2C_CLOCK		500000000
#define CONFIG_SH_I2C_BASE0		0xFFC70000
#define CONFIG_SH_I2C_BASE1		0xFFC71000

/* undef to save memory	*/
#define CONFIG_SYS_LONGHELP
/* Monitor Command Prompt */
/* Buffer size for input from the Console */
#define CONFIG_SYS_CBSIZE		256
/* Buffer size for Console output */
#define CONFIG_SYS_PBSIZE		256
/* max args accepted for monitor commands */
#define CONFIG_SYS_MAXARGS		16
/* Buffer size for Boot Arguments passed to kernel */
#define CONFIG_SYS_BARGSIZE	512
/* List of legal baudrate settings for this board */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }

/* SCIF */
#define CONFIG_SCIF_CONSOLE	1
#define CONFIG_SCIF			1
#define CONFIG_CONS_SCIF4	1

/* Suppress display of console information at boot */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE	(0x88000000)
#define CONFIG_SYS_SDRAM_SIZE	(64 * 1024 * 1024)
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 * 1024)

#define CONFIG_SYS_MEMTEST_START (CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END	 (CONFIG_SYS_MEMTEST_START + CONFIG_SYS_SDRAM_SIZE)
/* Enable alternate, more extensive, memory test */
#undef  CONFIG_SYS_ALT_MEMTEST
/* Scratch address used by the alternate memory test */
#undef  CONFIG_SYS_MEMTEST_SCRATCH

/* Enable temporary baudrate change while serial download */
#undef  CONFIG_SYS_LOADS_BAUD_CHANGE

/* FLASH */
#define CONFIG_FLASH_CFI_DRIVER 1
#define CONFIG_SYS_FLASH_CFI
#undef  CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_BASE	(0xA0000000)
#define CONFIG_SYS_MAX_FLASH_SECT	512

/* if you use all NOR Flash , you change dip-switch. Please see Manual. */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE }

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

/* Address of u-boot image in Flash (NOT run time address in SDRAM) ?!? */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE)
/* Monitor size */
#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)
/* Size of DRAM reserved for malloc() use */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024)
#define CONFIG_SYS_BOOTMAPSZ	(8 * 1024 * 1024)

/* ENV setting */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
/* Offset of env Flash sector relative to CONFIG_SYS_FLASH_BASE */
#define CONFIG_ENV_OFFSET	(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SECT_SIZE)

/* Board Clock */
#if defined(CONFIG_400MHZ_MODE)
#define CONFIG_SYS_CLK_FREQ 50000000
#else
#define CONFIG_SYS_CLK_FREQ 44444444
#endif
#define CONFIG_SH_TMU_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define CONFIG_SH_SCIF_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_TMU_CLK_DIV      4

#endif	/* __AP_SH4A_4A_H */

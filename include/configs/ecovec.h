/*
 * Configuation settings for the Renesas Solutions ECOVEC board
 *
 * Copyright (C) 2009 - 2011 Renesas Solutions Corp.
 * Copyright (C) 2009 Kuninori Morimoto <morimoto.kuninori@renesas.com>
 * Copyright (C) 2010, 2011 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ECOVEC_H
#define __ECOVEC_H

/*
 *  Address      Interface        BusWidth
 *-----------------------------------------
 *  0x0000_0000  U-Boot           16bit
 *  0x0004_0000  Linux romImage   16bit
 *  0x0014_0000  MTD for Linux    16bit
 *  0x0400_0000  Internal I/O     16/32bit
 *  0x0800_0000  DRAM             32bit
 *  0x1800_0000  MFI              16bit
 */

#define CONFIG_CPU_SH7724	1
#define CONFIG_ECOVEC		1

#define CONFIG_ECOVEC_ROMIMAGE_ADDR 0xA0040000
#define CONFIG_SYS_TEXT_BASE 0x8FFC0000

#define CONFIG_CMD_SDRAM

#define CONFIG_BOOTARGS		"console=ttySC0,115200"

#define CONFIG_DISPLAY_BOARDINFO
#undef  CONFIG_SHOW_BOOT_PROGRESS

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SH
#define CONFIG_SYS_I2C_SLAVE	0x7F
#define CONFIG_SYS_I2C_SH_NUM_CONTROLLERS 2
#define CONFIG_SYS_I2C_SH_BASE0	0xA4470000
#define CONFIG_SYS_I2C_SH_SPEED0	100000
#define CONFIG_SYS_I2C_SH_BASE1	0xA4750000
#define CONFIG_SYS_I2C_SH_SPEED1	100000
#define CONFIG_SH_I2C_DATA_HIGH	4
#define CONFIG_SH_I2C_DATA_LOW 	5
#define CONFIG_SH_I2C_CLOCK  	41666666

/* Ether */
#define CONFIG_SH_ETHER 1
#define CONFIG_SH_ETHER_USE_PORT (0)
#define CONFIG_SH_ETHER_PHY_ADDR (0x1f)
#define CONFIG_PHY_SMSC 1
#define CONFIG_PHYLIB
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI
#define CONFIG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_MII

/* USB / R8A66597 */
#define CONFIG_USB_R8A66597_HCD
#define CONFIG_R8A66597_BASE_ADDR   0xA4D80000
#define CONFIG_R8A66597_XTAL        0x0000  /* 12MHz */
#define CONFIG_R8A66597_LDRV        0x8000  /* 3.3V */
#define CONFIG_R8A66597_ENDIAN      0x0000  /* little */
#define CONFIG_SUPERH_ON_CHIP_R8A66597

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
#define CONFIG_SCIF		1
#define CONFIG_CONS_SCIF0	1

/* Suppress display of console information at boot */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE	(0x88000000)
#define CONFIG_SYS_SDRAM_SIZE	(256 * 1024 * 1024)
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 * 1024)

#define CONFIG_SYS_MEMTEST_START (CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END	 (CONFIG_SYS_MEMTEST_START + 200 * 1024 * 1024)
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
#define CONFIG_SYS_CLK_FREQ 41666666
#define CONFIG_SH_TMU_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define CONFIG_SH_SCIF_CLK_FREQ CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_TMU_CLK_DIV      4

#endif	/* __ECOVEC_H */

/*
 * (C) Copyright 2013 Atmel Corporation.
 * Josh Wu <josh.wu@atmel.com>
 *
 * Configuation settings for the AT91SAM9N12-EK boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AT91SAM9N12_CONFIG_H_
#define __AT91SAM9N12_CONFIG_H_

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

#define CONFIG_SYS_TEXT_BASE		0x26f00000

#define CONFIG_ARM926EJS
#define CONFIG_AT91FAMILY

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	16000000	/* main clock xtal */
#define CONFIG_SYS_HZ			1000

/* Misc CPU related */
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_OF_LIBFDT

/* general purpose I/O */
#define CONFIG_AT91_GPIO

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200

/* LCD */
#define CONFIG_LCD
#define LCD_BPP				LCD_COLOR16
#define LCD_OUTPUT_BPP			24
#define CONFIG_LCD_LOGO
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_ATMEL_HLCD
#define CONFIG_ATMEL_LCD_RGB565
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_BOOTDELAY		3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_FPGA

#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NAND
#define CONFIG_CMD_SF
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT

#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x08000000

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
# define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM + 0x1000 - GENERATED_GBL_DATA_SIZE)

/* DataFlash */
#ifdef CONFIG_CMD_SF
#define CONFIG_ATMEL_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SF_DEFAULT_SPEED		30000000
#define CONFIG_ENV_SPI_MODE		SPI_MODE_3
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#endif

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIO_PORTD, 4
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIO_PORTD, 5

/* PMECC & PMERRLOC */
#define CONFIG_ATMEL_NAND_HWECC
#define CONFIG_ATMEL_NAND_HW_PMECC
#define CONFIG_PMECC_CAP		2
#define CONFIG_PMECC_SECTOR_SIZE	512
#define CONFIG_PMECC_INDEX_TABLE_OFFSET	0x8000

#define CONFIG_CMD_NAND_TRIMFFS

#endif

#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT			"nand0=atmel_nand"
#define MTDPARTS_DEFAULT						\
	"mtdparts=atmel_nand:256k(bootstrap)ro,512k(uboot)ro,"		\
	"256k(env),256k(env_redundant),256k(spare),"			\
	"512k(dtb),6M(kernel)ro,-(rootfs)"

#define CONFIG_EXTRA_ENV_SETTINGS                                       \
	"console=console=ttyS0,115200\0"                                \
	"mtdparts="MTDPARTS_DEFAULT"\0"					\
	"bootargs_nand=rootfstype=ubifs ubi.mtd=7 root=ubi0:rootfs rw\0"\
	"bootargs_mmc=root=/dev/mmcblk0p2 rw rootfstype=ext4 rootwait\0"

/* MMC */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#endif

/* FAT */
#ifdef CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif

/* Ethernet */
#define CONFIG_KS8851_MLL
#define CONFIG_KS8851_MLL_BASEADDR	0x30000000 /* use NCS2 */

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x26e00000

#ifdef CONFIG_SYS_USE_SPIFLASH

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET		0x5000
#define CONFIG_ENV_SIZE			0x3000
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_nand};"	\
	"sf probe 0; sf read 0x22000000 0x100000 0x300000; "		\
	"bootm 0x22000000"

#elif defined(CONFIG_SYS_USE_NANDFLASH)

/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0xc0000
#define CONFIG_ENV_OFFSET_REDUND	0x100000
#define CONFIG_ENV_SIZE			0x20000		/* 1 sector = 128 kB */
#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_nand};"	\
	"nand read 0x21000000 0x180000 0x080000;"			\
	"nand read 0x22000000 0x200000 0x400000;"			\
	"bootm 0x22000000 - 0x21000000"

#else /* CONFIG_SYS_USE_MMC */

/* bootstrap + u-boot + env + linux in mmc */
#define CONFIG_ENV_IS_IN_MMC
/* For FAT system, most cases it should be in the reserved sector */
#define CONFIG_ENV_OFFSET		0x2000
#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_mmc};"	\
	"fatload mmc 0:1 0x21000000 dtb;"				\
	"fatload mmc 0:1 0x22000000 uImage;"				\
	"bootm 0x22000000 - 0x21000000"

#endif

#define CONFIG_SYS_PROMPT	"U-Boot> "
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) \
					+ 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(4 * 1024 * 1024)
#define CONFIG_STACKSIZE	(32 * 1024)	/* regular stack */

#endif

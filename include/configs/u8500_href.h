/*
 * Copyright (C) ST-Ericsson SA 2009
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_U8500

#define CONFIG_SYS_MEMTEST_START	0x00000000
#define CONFIG_SYS_MEMTEST_END	0x1FFFFFFF

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

/*
 * Size of malloc() pool
 */
#ifdef CONFIG_BOOT_SRAM
#define CONFIG_ENV_SIZE		(32*1024)
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 64*1024)
#else
#define CONFIG_ENV_SIZE		(128*1024)
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 256*1024)
#endif

/*
 * PL011 Configuration
 */
#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_SERIAL_RLCR
#define CONFIG_PL011_SERIAL_FLUSH_ON_INIT

/*
 * U8500 UART registers base for 3 serial devices
 */
#define CFG_UART0_BASE		0x80120000
#define CFG_UART1_BASE		0x80121000
#define CFG_UART2_BASE		0x80007000
#define CFG_SERIAL0		CFG_UART0_BASE
#define CFG_SERIAL1		CFG_UART1_BASE
#define CFG_SERIAL2		CFG_UART2_BASE
#define CONFIG_PL011_CLOCK	38400000
#define CONFIG_PL01x_PORTS	{ (void *)CFG_SERIAL0, (void *)CFG_SERIAL1, \
				  (void *)CFG_SERIAL2 }
#define CONFIG_CONS_INDEX	2
#define CONFIG_BAUDRATE		115200

/*
 * Devices and file systems
 */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DOS_PARTITION

/*
 * Commands
 */
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_I2C

#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	1
#endif
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#undef CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND	"run emmcboot"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=n\0"							\
	"loadaddr=0x00100000\0"						\
	"console=ttyAMA2,115200n8\0"					\
	"memargs256=mem=96M@0 mem_modem=32M@96M mem=30M@128M "		\
		"pmem=22M@158M pmem_hwb=44M@180M mem_mali=32@224M\0"	\
	"memargs512=mem=96M@0 mem_modem=32M@96M mem=44M@128M "		\
		"pmem=22M@172M mem=30M@194M mem_mali=32M@224M "		\
		"pmem_hwb=54M@256M mem=202M@310M\0"			\
	"commonargs=setenv bootargs cachepolicy=writealloc noinitrd "	\
		"init=init "						\
		"board_id=${board_id} "					\
		"logo.${logo} "						\
		"startup_graphics=${startup_graphics}\0"		\
	"emmcargs=setenv bootargs ${bootargs} "				\
		"root=/dev/mmcblk0p2 "					\
		"rootdelay=1\0"						\
	"addcons=setenv bootargs ${bootargs} "				\
		"console=${console}\0"					\
	"emmcboot=echo Booting from eMMC ...; "				\
		"run commonargs emmcargs addcons memargs;"		\
		"mmc read 0 ${loadaddr} 0xA0000 0x4000;"		\
		"bootm ${loadaddr}\0"					\
	"flash=mmc init 1;fatload mmc 1 ${loadaddr} flash.scr;"		\
		"source ${loadaddr}\0"					\
	"loaduimage=mmc init 1;fatload mmc 1 ${loadaddr} uImage\0"	\
	"usbtty=cdc_acm\0"						\
	"stdout=serial,usbtty\0"					\
	"stdin=serial,usbtty\0"						\
	"stderr=serial,usbtty\0"

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_LONGHELP			/* undef to save memory     */
#define CONFIG_SYS_PROMPT	"U8500 $ "	/* Monitor Command Prompt   */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size  */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Arg Buffer Size */

#define CONFIG_SYS_LOAD_ADDR		0x00100000 /* default load address */
#define CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING

#define CONFIG_SETUP_MEMORY_TAGS	2
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs  */

/*
 * I2C
 */
#define CONFIG_U8500_I2C
#undef	CONFIG_HARD_I2C			/* I2C with hardware support */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0	/* slave addr of controller */
#define CONFIG_SYS_U8500_I2C0_BASE		0x80004000
#define CONFIG_SYS_U8500_I2C1_BASE		0x80122000
#define CONFIG_SYS_U8500_I2C2_BASE		0x80128000
#define CONFIG_SYS_U8500_I2C3_BASE		0x80110000
#define CONFIG_SYS_U8500_I2C_BUS_MAX		4

#define CONFIG_SYS_I2C_GPIOE_ADDR	0x42	/* GPIO expander chip addr */
#define CONFIG_TC35892_GPIO

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x00000000	/* DDR-SDRAM Bank #1 */
#define PHYS_SDRAM_SIZE_1		0x20000000	/* 512 MB */

/*
 * additions for new relocation code
 */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_GBL_DATA_OFFSET

/* landing address before relocation */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE            0x0
#endif

/*
 * MMC related configs
 * NB Only externa SD slot is currently supported
 */
#define MMC_BLOCK_SIZE			512
#define CONFIG_ARM_PL180_MMCI
#define CONFIG_ARM_PL180_MMCI_BASE	0x80126000	/* MMC base for 8500  */
#define CONFIG_ARM_PL180_MMCI_CLOCK_FREQ 6250000
#define CONFIG_MMC_DEV_NUM		1

#define CONFIG_CMD_ENV
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		0x13F80000
#define CONFIG_SYS_MMC_ENV_DEV          0               /* SLOT2: eMMC */

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

/*
 * base register values for U8500
 */
#define CFG_PRCMU_BASE		0x80157000	/* Power, reset and clock
						   management unit */
#define CFG_FSMC_BASE		0x80000000	/* FSMC Controller */

#endif	/* __CONFIG_H */

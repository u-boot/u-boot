/*
 * Copyright (C) ST-Ericsson SA 2009
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * #define DEBUG 1
 */

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SNOWBALL
#define CONFIG_SYS_ICACHE_OFF
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_BOARD_LATE_INIT

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_U8500

#define CONFIG_SYS_MEMTEST_START	0x00000000
#define CONFIG_SYS_MEMTEST_END	0x1FFFFFFF

/*-----------------------------------------------------------------------
 * Size of environment and malloc() pool
 */
/*
 * If you use U-Boot as crash kernel, make sure that it does not overwrite
 * information saved by kexec during panic. Kexec expects the start
 * address of the executable 32K above "crashkernel" address.
 */
/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE		(8*1024)
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 256*1024)

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_CMD_ENV
#define CONFIG_ENV_OFFSET		0x0118000
#define CONFIG_SYS_MMC_ENV_DEV          0              /* SLOT2: eMMC */

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

#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	1
#endif
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#undef CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND \
"mmc dev 1; "								\
	"if run loadbootscript; "					\
		"then run bootscript; "					\
	"else "								\
		"if run mmcload; "					\
			"then run mmcboot; "				\
		"else "							\
			"mmc dev 0; "					\
			"if run emmcloadbootscript; "			\
				"then run bootscript; "			\
			"else "						\
				"if run emmcload; "			\
					"then run emmcboot; "		\
				"else "					\
					"echo No media to boot from; "	\
				"fi; "					\
			"fi; "						\
		"fi; "							\
	"fi; "

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=n\0"							\
	"loadaddr=0x00100000\0"						\
	"console=ttyAMA2,115200n8\0"					\
	"loadbootscript=fatload mmc 1:1 ${loadaddr} boot.scr\0"		\
	"emmcloadbootscript=fatload mmc 0:2 ${loadaddr} boot.scr\0"	\
	"bootscript=echo Running bootscript "				\
		"from mmc ...; source ${loadaddr}\0"			\
	"memargs256=mem=96M@0 mem_modem=32M@96M mem=32M@128M "		\
		"hwmem=22M@160M pmem_hwb=42M@182M mem_mali=32@224M\0"	\
	"memargs512=mem=96M@0 mem_modem=32M@96M hwmem=32M@128M "	\
		"mem=64M@160M mem_mali=32M@224M "			\
		"pmem_hwb=128M@256M mem=128M@384M\0"			\
	"memargs1024=mem=128M@0 mali.mali_mem=32M@128M "		\
		"hwmem=168M@M160M mem=48M@328M "			\
		"mem_issw=1M@383M mem=640M@384M\0"			\
	"memargs=setenv bootargs ${bootargs} ${memargs1024}\0"		\
	"emmcload=fatload mmc 0:2 ${loadaddr} uImage\0"			\
	"mmcload=fatload mmc 1:1 ${loadaddr} uImage\0"			\
	"commonargs=setenv bootargs console=${console} "		\
	"vmalloc=300M\0"						\
	"emmcargs=setenv bootargs ${bootargs} "				\
		"root=/dev/mmcblk0p3 "					\
		"rootwait\0"						\
	"addcons=setenv bootargs ${bootargs} "				\
		"console=${console}\0"					\
	"emmcboot=echo Booting from eMMC ...; "				\
		"run commonargs emmcargs memargs; "			\
		"bootm ${loadaddr}\0"					\
	"mmcargs=setenv bootargs ${bootargs} "				\
		"root=/dev/mmcblk1p2 "					\
		"rootwait earlyprintk\0"				\
	"mmcboot=echo Booting from external MMC ...; "			\
		"run commonargs mmcargs memargs; "			\
		"bootm ${loadaddr}\0"					\
	"fdt_high=0x2BC00000\0"						\
	"stdout=serial,usbtty\0"					\
	"stdin=serial,usbtty\0"						\
	"stderr=serial,usbtty\0"

/*-----------------------------------------------------------------------
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
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1

#define CONFIG_SYS_HUSH_PARSER		1
#define CONFIG_CMDLINE_EDITING

#define CONFIG_SETUP_MEMORY_TAGS	2
#define CONFIG_INITRD_TAG		1
#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs  */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x00000000	/* DDR-SDRAM Bank #1 */

/*
 * additions for new relocation code
 */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_MAX_RAM_SIZE	0x40000000
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
 */
#define CONFIG_ARM_PL180_MMCI
#define MMC_BLOCK_SIZE			512
#define CFG_EMMC_BASE                   0x80114000
#define CFG_MMC_BASE                    0x80126000

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

/*
 * base register values for U8500
 */
#define CFG_PRCMU_BASE		0x80157000	/* Power, reset and clock */


/*
 * U8500 GPIO register base for 9 banks
 */
#define CONFIG_DB8500_GPIO
#define CFG_GPIO_0_BASE			0x8012E000
#define CFG_GPIO_1_BASE			0x8012E080
#define CFG_GPIO_2_BASE			0x8000E000
#define CFG_GPIO_3_BASE			0x8000E080
#define CFG_GPIO_4_BASE			0x8000E100
#define CFG_GPIO_5_BASE			0x8000E180
#define CFG_GPIO_6_BASE			0x8011E000
#define CFG_GPIO_7_BASE			0x8011E080
#define CFG_GPIO_8_BASE			0xA03FE000

#define CFG_FSMC_BASE		0x80000000	/* FSMC Controller */

#endif	/* __CONFIG_H */

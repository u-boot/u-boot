/*
 * ti816x_evm.h
 *
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Antoine Tenart, <atenart@adeneo-embedded.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TI816X_EVM_H
#define __CONFIG_TI816X_EVM_H

#define CONFIG_TI81XX
#define CONFIG_TI816X

#define CONFIG_ARCH_CPU_INIT

#include <asm/arch/omap.h>

#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (32 * 1024))
#define CONFIG_SYS_LONGHELP		/* undef save memory */
#define CONFIG_MACH_TYPE		MACH_TYPE_TI8168EVM

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG		/* required for ramdisk support */

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"loadaddr=0x81000000\0"		\

#define CONFIG_BOOTCOMMAND			\
	"mmc rescan;"				\
	"fatload mmc 0 ${loadaddr} uImage;"	\
	"bootm ${loadaddr}"			\

#define CONFIG_BOOTARGS	"console=ttyO2,115200n8 noinitrd earlyprintk"

/* Clock Defines */
#define V_OSCK          24000000    /* Clock output from T2 */
#define V_SCLK          (V_OSCK >> 1)

#define CONFIG_SYS_MAXARGS	32
#define CONFIG_SYS_CBSIZE	512 /* console I/O buffer size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE \
		+ sizeof(CONFIG_SYS_PROMPT) + 16) /* print buffer size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* boot arg buffer size */

#define CONFIG_SYS_LOAD_ADDR		0x81000000 /* Default load address */

#define CONFIG_CMD_ASKENV

#define CONFIG_NR_DRAM_BANKS	2		/* we have 2 banks of DRAM */
#define PHYS_DRAM_1		0x80000000	/* DRAM Bank #1 */
#define PHYS_DRAM_1_SIZE        0x40000000	/* 1 GB */
#define PHYS_DRAM_2		0xC0000000	/* DRAM Bank #2 */
#define PHYS_DRAM_2_SIZE	0x40000000	/* 1 GB */

#define CONFIG_MAX_RAM_BANK_SIZE	(2048 << 20)	/* 2048MB */
#define CONFIG_SYS_SDRAM_BASE		PHYS_DRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(NON_SECURE_SRAM_END - \
		GENERATED_GBL_DATA_SIZE)

/**
 * Platform/Board specific defs
 */
#define CONFIG_SYS_CLK_FREQ     27000000
#define CONFIG_SYS_TIMERBASE    0x4802E000
#define CONFIG_SYS_PTV          2   /* Divisor: 2^(PTV+1) => 8 */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE (-4)
#define CONFIG_SYS_NS16550_CLK      (48000000)
#define CONFIG_SYS_NS16550_COM1     0x48024000  /* Base EVM has UART2 */

/* allow overwriting serial config and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_SERIAL1
#define CONFIG_SERIAL2
#define CONFIG_SERIAL3
#define CONFIG_CONS_INDEX	1

/*
 * GPMC NAND block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#define CONFIG_SYS_NAND_BASE		0x8000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* NAND: SPL related configs */
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_NAND_AM33XX_BCH
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

/* NAND: device related configs */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
/* NAND: driver related configs */
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_NAND_OMAP_GPMC_PREFETCH
#define CONFIG_NAND_OMAP_ELM
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW
#define MTDIDS_DEFAULT			"nand0=nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=nand.0:" \
					"128k(NAND.SPL)," \
					"128k(NAND.SPL.backup1)," \
					"128k(NAND.SPL.backup2)," \
					"128k(NAND.SPL.backup3)," \
					"256k(NAND.u-boot-spl-os)," \
					"1m(NAND.u-boot)," \
					"128k(NAND.u-boot-env)," \
					"128k(NAND.u-boot-env.backup1)," \
					"8m(NAND.kernel)," \
					"-(NAND.file-system)"
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x000c0000
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x001c0000
#define CONFIG_ENV_OFFSET_REDUND	0x001e0000
#define CONFIG_SYS_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE

/* SPL */
/* Defines for SPL */
#define CONFIG_SPL_NAND_AM33XX_BCH	/* ELM support */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE    0x40400000
#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)

#define CONFIG_SPL_BSS_START_ADDR   0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE     0x80000     /* 512 KB */

#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION     1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME        "u-boot.img"

#define CONFIG_SYS_SPI_U_BOOT_OFFS  0x20000
#define CONFIG_SYS_SPI_U_BOOT_SIZE  0x40000
#define CONFIG_SPL_LDSCRIPT     "arch/arm/mach-omap2/u-boot-spl.lds"

#define CONFIG_SYS_TEXT_BASE        0x80800000
#define CONFIG_SYS_SPL_MALLOC_START 0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE  0x100000

/* Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

#endif

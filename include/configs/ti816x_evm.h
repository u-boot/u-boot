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
#define CONFIG_SYS_NO_FLASH
#define CONFIG_OMAP
#define CONFIG_OMAP_COMMON

#define CONFIG_ARCH_CPU_INIT

#include <asm/arch/omap.h>

#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (32 * 1024))
#define CONFIG_SYS_LONGHELP		/* undef save memory */
#define CONFIG_MACH_TYPE		MACH_TYPE_TI8168EVM

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG		/* required for ramdisk support */

#define CONFIG_DISPLAY_CPUINFO

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

#define CONFIG_CMD_ASKEN
#define CONFIG_OMAP_GPIO
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_DOS_PARTITION

#define CONFIG_FS_FAT

/*
 * Only one of the following two options (DDR3/DDR2) should be enabled
 * CONFIG_TI816X_EVM_DDR2
 * CONFIG_TI816X_EVM_DDR3
 */
#define CONFIG_TI816X_EVM_DDR3

/*
 * Supported values: 400, 531, 675 or 796 MHz
 */
#define CONFIG_TI816X_DDR_PLL_796

#define CONFIG_TI816X_USE_EMIF0	1
#define CONFIG_TI816X_USE_EMIF1	1

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

#undef CONFIG_NAND_OMAP_GPMC

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE (-4)
#define CONFIG_SYS_NS16550_CLK      (48000000)
#define CONFIG_SYS_NS16550_COM1     0x48024000  /* Base EVM has UART2 */

#define CONFIG_BAUDRATE     115200

/* allow overwriting serial config and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_SERIAL1
#define CONFIG_SERIAL2
#define CONFIG_SERIAL3
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_CONSOLE_INFO_QUIET

#define CONFIG_ENV_IS_NOWHERE

/* SPL */
/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE    0x40400000
#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)

#define CONFIG_SPL_BSS_START_ADDR   0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE     0x80000     /* 512 KB */

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR 0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS      0x200 /* 256 KB */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION     1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME        "u-boot.img"
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_YMODEM_SUPPORT
#define CONFIG_SYS_SPI_U_BOOT_OFFS  0x20000
#define CONFIG_SYS_SPI_U_BOOT_SIZE  0x40000
#define CONFIG_SPL_LDSCRIPT     "$(CPUDIR)/omap-common/u-boot-spl.lds"

#define CONFIG_SPL_BOARD_INIT

#define CONFIG_SYS_TEXT_BASE        0x80800000
#define CONFIG_SYS_SPL_MALLOC_START 0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE  0x100000

/* Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/* Unsupported features */
#undef CONFIG_USE_IRQ

#endif

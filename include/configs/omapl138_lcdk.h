/*
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on davinci_dvevm.h. Original Copyrights follow:
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Board
 */
#define CONFIG_DRIVER_TI_EMAC
#undef CONFIG_USE_SPIFLASH
#undef	CONFIG_SYS_USE_NOR
#define	CONFIG_USE_NAND

/*
 * SoC Configuration
 */
#define CONFIG_MACH_OMAPL138_LCDK
#define CONFIG_ARM926EJS		/* arm926ejs CPU core */
#define CONFIG_SOC_DA8XX		/* TI DA8xx SoC */
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)
#define CONFIG_SYS_HZ			1000
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_TEXT_BASE		0xc1080000

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 1*1024*1024) /* malloc() len */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(128 << 20) /* SDRAM size 128MB */
#define CONFIG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/

/* memtest start addr */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + 0x2000000)

/* memtest will be run on 16MB */
#define CONFIG_SYS_MEMTEST_END 	(PHYS_SDRAM_1 + 0x2000000 + 16*1024*1024)

#define CONFIG_NR_DRAM_BANKS	1 /* we have 1 bank of DRAM */
#define CONFIG_STACKSIZE	(256*1024) /* regular stack */

#define CONFIG_SYS_DA850_SYSCFG_SUSPSRC (	\
	DAVINCI_SYSCFG_SUSPSRC_TIMER0 |		\
	DAVINCI_SYSCFG_SUSPSRC_SPI1 |		\
	DAVINCI_SYSCFG_SUSPSRC_UART2 |		\
	DAVINCI_SYSCFG_SUSPSRC_EMAC |		\
	DAVINCI_SYSCFG_SUSPSRC_I2C)

/*
 * PLL configuration
 */
#define CONFIG_SYS_DV_CLKMODE          0
#define CONFIG_SYS_DA850_PLL0_POSTDIV  1
#define CONFIG_SYS_DA850_PLL0_PLLDIV1  0x8000
#define CONFIG_SYS_DA850_PLL0_PLLDIV2  0x8001
#define CONFIG_SYS_DA850_PLL0_PLLDIV3  0x8002
#define CONFIG_SYS_DA850_PLL0_PLLDIV4  0x8003
#define CONFIG_SYS_DA850_PLL0_PLLDIV5  0x8002
#define CONFIG_SYS_DA850_PLL0_PLLDIV6  CONFIG_SYS_DA850_PLL0_PLLDIV1
#define CONFIG_SYS_DA850_PLL0_PLLDIV7  0x8005

#define CONFIG_SYS_DA850_PLL1_POSTDIV  1
#define CONFIG_SYS_DA850_PLL1_PLLDIV1  0x8000
#define CONFIG_SYS_DA850_PLL1_PLLDIV2  0x8001
#define CONFIG_SYS_DA850_PLL1_PLLDIV3  0x8003

#define CONFIG_SYS_DA850_PLL0_PLLM     24
#define CONFIG_SYS_DA850_PLL1_PLLM     21

/*
 * Serial Driver info
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size */
#define CONFIG_SYS_NS16550_COM1	DAVINCI_UART2_BASE /* Base address of UART2 */
#define CONFIG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_SPI
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_DAVINCI_SPI
#define CONFIG_SYS_SPI_BASE		DAVINCI_SPI1_BASE
#define CONFIG_SYS_SPI_CLK		clk_get(DAVINCI_SPI1_CLKID)
#define CONFIG_SF_DEFAULT_SPEED		30000000
#define CONFIG_ENV_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED

#ifdef CONFIG_USE_SPIFLASH
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x8000
#define CONFIG_SYS_SPI_U_BOOT_SIZE	0x30000
#endif

/*
 * I2C Configuration
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DAVINCI
#define CONFIG_SYS_DAVINCI_I2C_SPEED	25000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE	10 /* Bogus, master-only in U-Boot */
#define CONFIG_SYS_I2C_EXPANDER_ADDR	0x20

/*
 * Flash & Environment
 */
#ifdef CONFIG_USE_NAND
#undef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_NAND_DAVINCI
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_IN_NAND		/* U-Boot env in NAND Flash  */
#define CONFIG_ENV_OFFSET		0x0 /* Block 0--not used by bootcode */
#define CONFIG_ENV_SIZE			(128 << 9)
#define	CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define	CONFIG_SYS_NAND_PAGE_2K
#define	CONFIG_SYS_NAND_BUSWIDTH_16_BIT
#define CONFIG_SYS_NAND_CS		3
#define CONFIG_SYS_NAND_BASE		DAVINCI_ASYNC_EMIF_DATA_CE3_BASE
#define CONFIG_SYS_CLE_MASK		0x10
#define CONFIG_SYS_ALE_MASK		0x8
#undef CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1 /* Max number of NAND devices */
#define NAND_MAX_CHIPS			1
#endif

#ifdef CONFIG_SYS_USE_NOR
#define CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_SYS_NO_FLASH
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* max number of flash banks */
#define CONFIG_SYS_FLASH_SECT_SZ	(128 << 10) /* 128KB */
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_FLASH_SECT_SZ * 3)
#define CONFIG_ENV_SIZE			(128 << 10)
#define CONFIG_SYS_FLASH_BASE		DAVINCI_ASYNC_EMIF_DATA_CE2_BASE
#define PHYS_FLASH_SIZE			(8 << 20) /* Flash size 8MB */
#define CONFIG_SYS_MAX_FLASH_SECT ((PHYS_FLASH_SIZE/CONFIG_SYS_FLASH_SECT_SZ)\
	       + 3)
#define CONFIG_ENV_SECT_SIZE		CONFIG_SYS_FLASH_SECT_SZ
#endif

#ifdef CONFIG_USE_SPIFLASH
#undef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE			(64 << 10)
#define CONFIG_ENV_OFFSET		(256 << 10)
#define CONFIG_ENV_SECT_SIZE		(64 << 10)
#define CONFIG_SYS_NO_FLASH
#endif

/*
 * Network & Ethernet Configuration
 */
#ifdef CONFIG_DRIVER_TI_EMAC
#define CONFIG_EMAC_MDIO_PHY_NUM	7
#define CONFIG_MII
#undef	CONFIG_DRIVER_TI_EMAC_USE_RMII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#endif

/*
 * U-Boot general configuration
 */
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_MISC_INIT_R
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOOTFILE		"uImage" /* Boot file name */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16 /* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR	(PHYS_SDRAM_1 + 0x700000)
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
#define CONFIG_OF_LIBFDT

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTARGS		"console=ttyS2,115200n8 root=/dev/mmcblk0p2 rw rootwait ip=off"
#define CONFIG_BOOTCOMMAND	"if mmc rescan 0; then if fatload mmc 0 0xc0600000 boot.scr; then source 0xc0600000; else fatload mmc 0 0xc0700000 uImage; bootm c0700000; fi; else sf probe 0; sf read 0xc0700000 0x80000 0x220000; bootm 0xc0700000; fi"
#define CONFIG_BOOTDELAY	3

/*
 * U-Boot commands
 */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES
#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#ifndef CONFIG_DRIVER_TI_EMAC
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_MII
#undef CONFIG_CMD_PING
#endif

#ifdef CONFIG_USE_NAND
#define CONFIG_CMD_NAND

#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_LZO
#define CONFIG_RBTREE
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#endif

#ifdef CONFIG_USE_SPIFLASH
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#endif

#if !defined(CONFIG_USE_NAND) && \
	!defined(CONFIG_SYS_USE_NOR) && \
	!defined(CONFIG_USE_SPIFLASH)
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_SIZE		(16 << 10)
#undef CONFIG_CMD_ENV
#endif

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DAVINCI_MMC

#ifdef CONFIG_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MMC
#undef CONFIG_ENV_IS_IN_MMC
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_SIZE		(16 << 10)	/* 16 KiB */
#define CONFIG_ENV_OFFSET	(51 << 9)	/* Sector 51 */
#undef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_IN_NAND
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#endif

#ifndef CONFIG_DIRECT_NOR_BOOT
/* defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SYS_TEXT_BASE - \
						CONFIG_SYS_MALLOC_LEN)
#define CONFIG_SYS_SPL_MALLOC_SIZE	CONFIG_SYS_MALLOC_LEN
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_LDSCRIPT	"board/$(BOARDDIR)/u-boot-spl-da850evm.lds"
#define CONFIG_SPL_STACK	0x8001ff00
#define CONFIG_SPL_TEXT_BASE	0x80000000
#define CONFIG_SPL_MAX_FOOTPRINT	32768
#define CONFIG_SPL_PAD_TO	32768
#endif

/* additions for new relocation code, must added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0xc0000000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - /* Fix this */ \
					GENERATED_GBL_DATA_SIZE)
#endif /* __CONFIG_H */

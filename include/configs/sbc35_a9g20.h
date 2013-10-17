/*
 * Copyright (C) 2009
 * Albin Tonnerre, Free Electrons <albin.tonnerre@free-electrons.com>
 *
 * Configuation settings for the Calao SBC35-A9G20 board
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SoC type is defined in boards.cfg */
#include <asm/hardware.h>
#include <asm/sizes.h>

#if defined(CONFIG_SYS_USE_NANDFLASH)
#define CONFIG_ENV_IS_IN_NAND
#else
#define CONFIG_ENV_IS_IN_EEPROM
#endif

#define MACH_TYPE_SBC35_A9G20		1848
#define CONFIG_MACH_TYPE		MACH_TYPE_SBC35_A9G20

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	12000000	/* 12.000 MHz crystal */
#define CONFIG_SYS_HZ		        1000

#define CONFIG_ARCH_CPU_INIT

#define CONFIG_CMDLINE_TAG              /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT

/* GPIO */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO

/* Serial */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE               ATMEL_BASE_DBGU
#define CONFIG_USART_ID                 ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200

#define CONFIG_BOOTDELAY	3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_SOURCE

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_USB

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS	1
#define CONFIG_SYS_SDRAM_BASE	ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE	0x04000000	/* 64 megs */
#define CONFIG_SYS_INIT_SP_ADDR	(ATMEL_BASE_SRAM1 + 0x1000 - \
				 GENERATED_GBL_DATA_SIZE)

/* SPI EEPROM */
#define CONFIG_SPI
#define CONFIG_CMD_SPI
#define CONFIG_ATMEL_SPI
#define CONFIG_SYS_SPI_WRITE_TOUT	(5 * CONFIG_SYS_HZ)

#define CONFIG_CMD_EEPROM
#define CONFIG_SPI_M95XXX
#define CONFIG_SYS_EEPROM_SIZE 0x10000
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 5

/* SPI RTC */
#define CONFIG_CMD_DATE
#define CONFIG_RTC_M41T94
#define CONFIG_M41T94_SPI_BUS 0
#define CONFIG_M41T94_SPI_CS 0

/* NAND flash */
#define CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC13

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH			1

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R
#define CONFIG_MACB_SEARCH_PHY

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	0x00500000	/* AT91SAM9260_UHP_BASE */
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_FAT

#define CONFIG_SYS_LOAD_ADDR		0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x23e00000

/* Env in EEPROM, bootstrap + u-boot in NAND*/
#ifdef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET	0x20
#define CONFIG_ENV_SIZE		0x1000
#endif

/* Env, bootstrap and u-boot in NAND */
#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_ENV_OFFSET_REDUND	0x80000
#define CONFIG_ENV_SIZE			0x20000
#endif

#define CONFIG_BOOTCOMMAND	"nboot 0x21000000 0 400000"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 " \
				"root=/dev/mtdblock1 " \
				"mtdparts=atmel_nand:16M(kernel)ro," \
				"120M(rootfs),-(other) " \
				"rw rootfstype=jffs2"


#define CONFIG_SYS_PROMPT	"U-Boot> "
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP	1
#define CONFIG_CMDLINE_EDITING	1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024, 0x1000)

#endif

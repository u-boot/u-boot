/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9261EK board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* 18.432 MHz crystal */
#define CONFIG_SYS_HZ			1000

#ifdef CONFIG_AT91SAM9G10
#define CONFIG_AT91SAM9G10EK		/* It's an Atmel AT91SAM9G10 EK*/
#else
#define CONFIG_AT91SAM9261EK		/* It's an Atmel AT91SAM9261 EK*/
#endif

#include <asm/hardware.h>

#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_ATMEL_LEGACY
#define CONFIG_SYS_TEXT_BASE		0x21f00000

/*
 * Hardware drivers
 */

/* gpio */
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP		1

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200, 57600, 38400, 19200, 9600}

/* LCD */
#define CONFIG_LCD
#define LCD_BPP				LCD_COLOR8
#define CONFIG_LCD_LOGO
#undef LCD_TEST_PATTERN
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_ATMEL_LCD
#ifdef CONFIG_AT91SAM9261EK
#define CONFIG_ATMEL_LCD_BGR555
#endif

#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/* LED */
#define CONFIG_AT91_LED
#define	CONFIG_RED_LED		AT91_PIN_PA23	/* this is the power led */
#define	CONFIG_GREEN_LED	AT91_PIN_PA13	/* this is the user1 led */
#define	CONFIG_YELLOW_LED	AT91_PIN_PA14	/* this is the user2 led */

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
#define CONFIG_CMD_NAND
#define CONFIG_CMD_USB

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x04000000
#define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM + 0x1000 - GENERATED_GBL_DATA_SIZE)

/* DataFlash */
#define CONFIG_ATMEL_DATAFLASH_SPI
#define CONFIG_HAS_DATAFLASH
#define CONFIG_SYS_SPI_WRITE_TOUT		(5*CONFIG_SYS_HZ)
#define CONFIG_SYS_MAX_DATAFLASH_BANKS		2
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* CS0 */
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000	/* CS3 */
#define AT91_SPI_CLK				15000000
#define DATAFLASH_TCSS				(0x1a << 16)
#define DATAFLASH_TCHS				(0x1 << 24)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8
/* our ALE is AD22 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 22)
/* our CLE is AD21 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 21)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC15

#endif

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH

/* Ethernet */
#define CONFIG_DRIVER_DM9000
#define CONFIG_DM9000_BASE		0x30000000
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE + 4)
#define CONFIG_DM9000_USE_16BIT
#define CONFIG_DM9000_NO_SROM
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00500000	/* AT91SAM9261_UHP_BASE */
#ifdef CONFIG_AT91SAM9G10EK
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9g10"
#else
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9261"
#endif
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_FAT

#define CONFIG_SYS_LOAD_ADDR			0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END			0x23e00000

#ifdef CONFIG_SYS_USE_DATAFLASH_CS0

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_DATAFLASH
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + 0x8400)
#define CONFIG_ENV_OFFSET	0x4200
#define CONFIG_ENV_ADDR		(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x4200
#define CONFIG_BOOTCOMMAND	"cp.b 0xC0042000 0x22000000 0x210000; bootm"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 "			\
				"root=/dev/mtdblock0 "			\
				"mtdparts=atmel_nand:-(root) "		\
				"rw rootfstype=jffs2"

#elif CONFIG_SYS_USE_DATAFLASH_CS3

/* bootstrap + u-boot + env + linux in dataflash on CS3 */
#define CONFIG_ENV_IS_IN_DATAFLASH
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS3 + 0x8400)
#define CONFIG_ENV_OFFSET	0x4200
#define CONFIG_ENV_ADDR		(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS3 + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x4200
#define CONFIG_BOOTCOMMAND	"cp.b 0xD0042000 0x22000000 0x210000; bootm"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 "			\
				"root=/dev/mtdblock0 "			\
				"mtdparts=atmel_nand:-(root) "		\
				"rw rootfstype=jffs2"

#else /* CONFIG_SYS_USE_NANDFLASH */

/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_ENV_OFFSET_REDUND	0x80000
#define CONFIG_ENV_SIZE		0x20000		/* 1 sector = 128 kB */
#define CONFIG_BOOTCOMMAND	"nand read 0x22000000 0xA0000 0x200000; bootm"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 "			\
				"root=/dev/mtdblock5 "			\
				"mtdparts=atmel_nand:128k(bootstrap)ro,"	\
				"256k(uboot)ro,128k(env1)ro,"		\
				"128k(env2)ro,2M(linux),-(root) "	\
				"rw rootfstype=jffs2"

#endif

#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)

#define CONFIG_STACKSIZE		(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif

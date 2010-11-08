/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
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

#define CONFIG_AT91_LEGACY

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* 18.432 MHz crystal */
#define CONFIG_SYS_HZ		1000

#define CONFIG_ARM926EJS	1	/* This is an ARM926EJS Core	*/
#ifdef CONFIG_AT91SAM9G10EK
#define CONFIG_AT91SAM9G10	1	/* It's an Atmel AT91SAM9G10 SoC*/
#else
#define CONFIG_AT91SAM9261	1	/* It's an Atmel AT91SAM9261 SoC*/
#endif
#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT

/*
 * Hardware drivers
 */
#define CONFIG_AT91_GPIO	1
#define CONFIG_ATMEL_USART	1
#undef CONFIG_USART0
#undef CONFIG_USART1
#undef CONFIG_USART2
#define CONFIG_USART3		1	/* USART 3 is DBGU */

/* LCD */
#define CONFIG_LCD			1
#define LCD_BPP				LCD_COLOR8
#define CONFIG_LCD_LOGO			1
#undef LCD_TEST_PATTERN
#define CONFIG_LCD_INFO			1
#define CONFIG_LCD_INFO_BELOW_LOGO	1
#define CONFIG_SYS_WHITE_ON_BLACK		1
#define CONFIG_ATMEL_LCD		1
#ifdef CONFIG_AT91SAM9261EK
#define CONFIG_ATMEL_LCD_BGR555		1
#else
#define	CONFIG_AT91SAM9G10_LCD_BASE		0x23E00000	/* LCD is no more in SRAM */
#endif
#define CONFIG_SYS_CONSOLE_IS_IN_ENV		1

/* LED */
#define CONFIG_AT91_LED
#define	CONFIG_RED_LED		AT91_PIN_PA23	/* this is the power led */
#define	CONFIG_GREEN_LED	AT91_PIN_PA13	/* this is the user1 led */
#define	CONFIG_YELLOW_LED	AT91_PIN_PA14	/* this is the user2 led */

#define CONFIG_BOOTDELAY	3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE	1
#define CONFIG_BOOTP_BOOTPATH		1
#define CONFIG_BOOTP_GATEWAY		1
#define CONFIG_BOOTP_HOSTNAME		1

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

#define CONFIG_CMD_PING		1
#define CONFIG_CMD_DHCP		1
#define CONFIG_CMD_NAND		1
#define CONFIG_CMD_USB		1

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			0x20000000
#define PHYS_SDRAM_SIZE			0x04000000	/* 64 megs */

/* DataFlash */
#define CONFIG_ATMEL_DATAFLASH_SPI
#define CONFIG_HAS_DATAFLASH		1
#define CONFIG_SYS_SPI_WRITE_TOUT		(5*CONFIG_SYS_HZ)
#define CONFIG_SYS_MAX_DATAFLASH_BANKS		2
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* CS0 */
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000	/* CS3 */
#define AT91_SPI_CLK			15000000
#define DATAFLASH_TCSS			(0x1a << 16)
#define DATAFLASH_TCHS			(0x1 << 24)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8			1
/* our ALE is AD22 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 22)
/* our CLE is AD21 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 21)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC15

#endif

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH			1

/* Ethernet */
#define CONFIG_NET_MULTI		1
#define CONFIG_DRIVER_DM9000		1
#define CONFIG_DM9000_BASE		0x30000000
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE + 4)
#define CONFIG_DM9000_USE_16BIT		1
#define CONFIG_DM9000_NO_SROM		1
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R		1

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW		1
#define CONFIG_DOS_PARTITION		1
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00500000	/* AT91SAM9261_UHP_BASE */
#ifdef CONFIG_AT91SAM9G10EK
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9g10"
#else
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9261"
#endif
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE		1
#define CONFIG_CMD_FAT			1

#define CONFIG_SYS_LOAD_ADDR			0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			0x23e00000

#ifdef CONFIG_SYS_USE_DATAFLASH_CS0

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_DATAFLASH	1
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
#define CONFIG_ENV_IS_IN_DATAFLASH	1
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
#define CONFIG_ENV_IS_IN_NAND	1
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

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }

#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_CMDLINE_EDITING	1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* 128 bytes for initial data */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif

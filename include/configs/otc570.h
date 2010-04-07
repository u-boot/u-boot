/*
 * (C) Copyright 2010
 * Daniel Gorsulowski <daniel.gorsulowski@esd.eu>
 * esd electronic system design gmbh <www.esd.eu>
 *
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the esd OTC570 board.
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

/* Common stuff */
#define CONFIG_OTC570			1	/* Board is esd OTC570 */
#define CONFIG_ARM926EJS		1	/* This is an ARM926EJS Core */
#define CONFIG_AT91SAM9263		1	/* It's an AT91SAM9263 SoC */
#define CONFIG_SYS_HZ			1000	/* decrementer freq */
#define CONFIG_DISPLAY_BOARDINFO	1
#define CONFIG_DISPLAY_CPUINFO		1	/* display cpu info and speed */
#define CONFIG_PREBOOT				/* enable preboot variable */
#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_SERIAL_TAG		1
#define CONFIG_REVISION_TAG		1
#undef CONFIG_USE_IRQ				/* don't need IRQ/FIQ stuff */

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT
#define CONFIG_MISC_INIT_R		1	/* Call misc_init_r */

#define CONFIG_ARCH_CPU_INIT

/*
 * Hardware drivers
 */
#define CONFIG_AT91_GPIO		1

/* Console output */
#define CONFIG_ATMEL_USART		1
#undef CONFIG_USART0
#undef CONFIG_USART1
#undef CONFIG_USART2
#define CONFIG_USART3			1	/* USART 3 is DBGU */

#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK	1

/* LCD */
#define CONFIG_LCD			1
#define LCD_BPP				LCD_COLOR8

#undef CONFIG_SPLASH_SCREEN

#ifndef CONFIG_SPLASH_SCREEN
#define CONFIG_LCD_LOGO			1
#define CONFIG_LCD_INFO			1
#undef CONFIG_LCD_INFO_BELOW_LOGO
#endif /* CONFIG_SPLASH_SCREEN */

#undef LCD_TEST_PATTERN
#define CONFIG_SYS_WHITE_ON_BLACK	1
#define CONFIG_ATMEL_LCD		1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1
#define CONFIG_OTC570_LCD_BASE		0x23E00000	/* LCD is in SDRAM */
#define CONFIG_CMD_BMP			1

/* RTC and I2C stuff */
#define CONFIG_RTC_DS1338		1
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
#undef CONFIG_HARD_I2C
#define CONFIG_SOFT_I2C			1
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0x7F

#ifdef CONFIG_SOFT_I2C
#define CONFIG_I2C_CMD_TREE		1
#define CONFIG_I2C_MULTI_BUS		1
/* Configure data and clock pins for pio */
#define I2C_INIT { \
	at91_set_pio_output(AT91_PIO_PORTB, 4, 0); \
	at91_set_pio_output(AT91_PIO_PORTB, 5, 0); \
}
#define I2C_SOFT_DECLARATIONS
/* Configure data pin as output */
#define I2C_ACTIVE		at91_set_pio_output(AT91_PIO_PORTB, 4, 0)
/* Configure data pin as input */
#define I2C_TRISTATE		at91_set_pio_input(AT91_PIO_PORTB, 4, 0)
/* Read data pin */
#define I2C_READ		at91_get_pio_value(AT91_PIO_PORTB, 4)
/* Set data pin */
#define I2C_SDA(bit)		at91_set_pio_value(AT91_PIO_PORTB, 4, bit)
/* Set clock pin */
#define I2C_SCL(bit)		at91_set_pio_value(AT91_PIO_PORTB, 5, bit)
#define I2C_DELAY		udelay(2) /* 1/4 I2C clock duration */
#endif /* CONFIG_SOFT_I2C */

#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK	1

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
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_IMLS

#define CONFIG_CMD_PING			1
#define CONFIG_CMD_DHCP			1
#define CONFIG_CMD_NAND			1
#define CONFIG_CMD_USB			1
#define CONFIG_CMD_I2C			1
#define CONFIG_CMD_DATE			1

/* LED */
#define CONFIG_AT91_LED			1

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS			1
#define PHYS_SDRAM				0x20000000

/* DataFlash */
#define CONFIG_ATMEL_DATAFLASH_SPI
#define CONFIG_HAS_DATAFLASH			1
#define CONFIG_SYS_SPI_WRITE_TOUT		(5 * CONFIG_SYS_HZ)
#define CONFIG_SYS_MAX_DATAFLASH_BANKS		1
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* CS0 */
#define AT91_SPI_CLK				15000000
#define DATAFLASH_TCSS				(0x1a << 16)
#define DATAFLASH_TCHS				(0x1 << 24)

/* NOR flash is not populated, disable it */
#define CONFIG_SYS_NO_FLASH			1

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8			1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIO_PORTD, 15
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIO_PORTA, 22
#define CONFIG_SYS_64BIT_VSPRINTF		/* needed for nand_util.c */
#endif

/* Ethernet */
#define CONFIG_MACB				1
#define CONFIG_RMII				1
#define CONFIG_NET_MULTI			1
#define CONFIG_NET_RETRY_COUNT			20
#undef CONFIG_RESET_PHY_R

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW			1
#define CONFIG_DOS_PARTITION			1
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00a00000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9263"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE			1
#define CONFIG_CMD_FAT				1

#define CONFIG_SYS_LOAD_ADDR			0x22000000 /* load address */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			0x23e00000

#define CONFIG_SYS_USE_DATAFLASH		1
#undef CONFIG_SYS_USE_NANDFLASH

/* CAN */
#define CONFIG_AT91_CAN				1

/* hw-controller addresses */
#define CONFIG_ET1100_BASE			0x70000000

/* bootstrap + u-boot + env in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_DATAFLASH	1
#define CONFIG_SYS_MONITOR_BASE		(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + \
					0x8400)
#define CONFIG_ENV_OFFSET		0x4200
#define CONFIG_ENV_ADDR			(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + \
					CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE			0x4200

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }

#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_CMDLINE_EDITING		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + \
					128*1024, 0x1000)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* 128 bytes for initial data */

#define CONFIG_STACKSIZE		(32 * 1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif

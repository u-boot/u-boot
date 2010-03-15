/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2009
 * Daniel Gorsulowski <daniel.gorsulowski@esd.eu>
 * esd electronic system design gmbh <www.esd.eu>
 *
 * Configuation settings for the esd MEESC board.
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

/* Common stuff */
#define CONFIG_SYS_HZ			1000	/* decrementer freq */
#define CONFIG_MEESC			1	/* Board is esd MEESC */
#define CONFIG_ARM926EJS		1	/* This is an ARM926EJS Core */
#define CONFIG_AT91SAM9263		1	/* It's an AT91SAM9263 SoC */
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
#define CONFIG_MISC_INIT_R			/* Call misc_init_r */

#define CONFIG_ARCH_CPU_INIT

/*
 * Hardware drivers
 */

/* Console output */
#define CONFIG_AT91_GPIO			1
#define CONFIG_ATMEL_USART			1
#undef CONFIG_USART0
#undef CONFIG_USART1
#undef CONFIG_USART2
#define CONFIG_USART3				1	/* USART 3 is DBGU */

#define CONFIG_BOOTDELAY			3
#define CONFIG_ZERO_BOOTDELAY_CHECK		1

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE		1
#define CONFIG_BOOTP_BOOTPATH			1
#define CONFIG_BOOTP_GATEWAY			1
#define CONFIG_BOOTP_HOSTNAME			1

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_AUTOSCRIPT
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_USB

#define CONFIG_CMD_PING				1
#define CONFIG_CMD_DHCP				1
#define CONFIG_CMD_NAND				1

/* LED */
#define CONFIG_AT91_LED				1

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
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PD15
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PA22

#endif

/* Ethernet */
#define CONFIG_MACB				1
#define CONFIG_RMII				1
#define CONFIG_NET_MULTI			1
#define CONFIG_NET_RETRY_COUNT			20
#undef CONFIG_RESET_PHY_R

#define CONFIG_SYS_LOAD_ADDR			0x22000000 /* load address */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			0x21e00000

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

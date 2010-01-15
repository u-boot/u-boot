/*
 * (C) Copyright 2009
 * Vipin Kumar, STMicroelectronics, <vipin.kumar@st.com>
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

#ifndef _SPEAR_COMMON_H
#define _SPEAR_COMMON_H
/*
 * Common configurations used for both spear3xx as well as spear6xx
 */

/* USBD driver configuration */
#define CONFIG_SPEARUDC
#define CONFIG_USB_DEVICE
#define CONFIG_USB_TTY

#define CONFIG_USBD_PRODUCT_NAME		"SPEAr SoC"
#define CONFIG_USBD_MANUFACTURER		"ST Microelectronics"

#define CONFIG_EXTRA_ENV_USBTTY			"usbtty=cdc_acm\0"

/* I2C driver configuration */
#define CONFIG_HARD_I2C
#define CONFIG_SPEAR_I2C
#define CONFIG_SYS_I2C_SPEED			400000
#define CONFIG_SYS_I2C_SLAVE			0x02

#define CONFIG_I2C_CHIPADDRESS			0x50

/* Timer, HZ specific defines */
#define CONFIG_SYS_HZ				(1000)
#define CONFIG_SYS_HZ_CLOCK			(8300000)

/* Flash configuration */
#if defined(CONFIG_FLASH_PNOR)
#define CONFIG_SPEAR_EMI			1
#else
#define CONFIG_SPEARSMI				1
#endif

#if defined(CONFIG_SPEARSMI)

#define CONFIG_SYS_MAX_FLASH_BANKS		2
#define CONFIG_SYS_FLASH_BASE			(0xF8000000)
#define CONFIG_SYS_CS1_FLASH_BASE		(0xF9000000)
#define CONFIG_SYS_FLASH_BANK_SIZE		(0x01000000)
#define CONFIG_SYS_FLASH_ADDR_BASE		{CONFIG_SYS_FLASH_BASE, \
						CONFIG_SYS_CS1_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_SECT		128

#define CONFIG_SYS_FLASH_EMPTY_INFO		1
#define CONFIG_SYS_FLASH_ERASE_TOUT		(3 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT		(3 * CONFIG_SYS_HZ)

#endif

/*
 * Serial Configuration (PL011)
 * CONFIG_PL01x_PORTS is defined in specific files
 */
#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_CLOCK			(48 * 1000 * 1000)
#define CONFIG_CONS_INDEX			0
#define CONFIG_BAUDRATE				115200
#define CONFIG_SYS_BAUDRATE_TABLE		{ 9600, 19200, 38400, \
						57600, 115200 }

#define CONFIG_SYS_LOADS_BAUD_CHANGE

/* NAND FLASH Configuration */
#define CONFIG_NAND_SPEAR			1
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_MTD_NAND_VERIFY_WRITE		1

/*
 * Command support defines
 */
#define CONFIG_CMD_I2C
#define CONFIG_CMD_NAND
#define CONFIG_CMD_ENV
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVES

/* This must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <config_cmd_default.h>
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

/*
 * Default Environment Varible definitions
 */
#if defined(CONFIG_SPEAR_USBTTY)
#define CONFIG_BOOTDELAY			-1
#else
#define CONFIG_BOOTDELAY			1
#endif

#define CONFIG_ENV_OVERWRITE

/*
 * U-Boot Environment placing definitions.
 */
#if defined(CONFIG_ENV_IS_IN_FLASH)
#ifdef CONFIG_SPEARSMI
/*
 * Environment is in serial NOR flash
 */
#define CONFIG_SYS_MONITOR_LEN			0x00040000
#define CONFIG_ENV_SECT_SIZE			0x00010000
#define CONFIG_FSMTDBLK				"/dev/mtdblock8 "

#define CONFIG_BOOTCOMMAND			"bootm 0xf8050000"

#elif defined(CONFIG_SPEAR_EMI)
/*
 * Environment is in parallel NOR flash
 */
#define CONFIG_SYS_MONITOR_LEN			0x00060000
#define CONFIG_ENV_SECT_SIZE			0x00020000
#define CONFIG_FSMTDBLK				"/dev/mtdblock3 "

#define CONFIG_BOOTCOMMAND			"cp.b 0x50080000 0x1600000 " \
						"0x4C0000; bootm 0x1600000"
#endif

#define CONFIG_SYS_MONITOR_BASE			CONFIG_SYS_FLASH_BASE
#define CONFIG_ENV_ADDR				(CONFIG_SYS_MONITOR_BASE + \
						CONFIG_SYS_MONITOR_LEN)
#elif defined(CONFIG_ENV_IS_IN_NAND)
/*
 * Environment is in NAND
 */

#define CONFIG_ENV_OFFSET			0x60000
#define CONFIG_ENV_RANGE			0x10000
#define CONFIG_FSMTDBLK				"/dev/mtdblock12 "

#define CONFIG_BOOTCOMMAND			"nand read.jffs2 0x1600000 " \
						"0x80000 0x4C0000; " \
						"bootm 0x1600000"
#endif

#define CONFIG_BOOTARGS_NFS			"root=/dev/nfs ip=dhcp " \
						"console=ttyS0 init=/bin/sh"
#define CONFIG_BOOTARGS				"console=ttyS0 mem=128M "  \
						"root="CONFIG_FSMTDBLK \
						"rootfstype=jffs2"

#define CONFIG_ENV_SIZE				0x02000

/* Miscellaneous configurable options */
#define CONFIG_BOOT_PARAMS_ADDR			0x00000100
#define CONFIG_CMDLINE_TAG			1
#define CONFIG_SETUP_MEMORY_TAGS		1
#define CONFIG_MISC_INIT_R			1
#define CONFIG_ZERO_BOOTDELAY_CHECK		1
#define CONFIG_AUTOBOOT_KEYED			1
#define CONFIG_AUTOBOOT_STOP_STR		" "
#define CONFIG_AUTOBOOT_PROMPT			\
		"Hit SPACE in %d seconds to stop autoboot.\n", bootdelay

#define CONFIG_SYS_MEMTEST_START		0x00800000
#define CONFIG_SYS_MEMTEST_END			0x04000000
#define CONFIG_SYS_MALLOC_LEN			(1024*1024)
#define CONFIG_SYS_GBL_DATA_SIZE		128
#define CONFIG_IDENT_STRING			"-SPEAr"
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT			"u-boot> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_CBSIZE			256
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS			16
#define CONFIG_SYS_BARGSIZE			CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LOAD_ADDR			0x00800000
#define CONFIG_SYS_CONSOLE_INFO_QUIET		1
#define CONFIG_SYS_64BIT_VSPRINTF		1

#define CONFIG_EXTRA_ENV_SETTINGS		CONFIG_EXTRA_ENV_USBTTY

/* Stack sizes */
#define CONFIG_STACKSIZE			(128*1024)

#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ			(4*1024)
#define CONFIG_STACKSIZE_FIQ			(4*1024)
#endif

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS			1
#define PHYS_SDRAM_1				0x00000000
#define PHYS_SDRAM_1_MAXSIZE			0x40000000

#endif

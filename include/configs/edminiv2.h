/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.aribaud@free.fr>
 *
 * Based on original Kirkwood support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _CONFIG_EDMINIV2_H
#define _CONFIG_EDMINIV2_H

/*
 * Version number information
 */

#define CONFIG_IDENT_STRING	" EDMiniV2"

/*
 * High Level Configuration Options (easy to change)
 */

#define CONFIG_MARVELL		1
#define CONFIG_ARM926EJS	1	/* Basic Architecture */
#define CONFIG_FEROCEON		1	/* CPU Core subversion */
#define CONFIG_ORION5X		1	/* SOC Family Name */
#define CONFIG_88F5182		1	/* SOC Name */
#define CONFIG_MACH_EDMINIV2	1	/* Machine type */

/*
 * CLKs configurations
 */

#define CONFIG_SYS_HZ		1000

/*
 * Board-specific values for Orion5x MPP low level init:
 * - MPPs 12 to 15 are SATA LEDs (mode 5)
 * - Others are GPIO/unused (mode 3 for MPP0, mode 5 for
 *   MPP16 to MPP19, mode 0 for others
 */

#define ORION5X_MPP0_7		0x00000003
#define ORION5X_MPP8_15		0x55550000
#define ORION5X_MPP16_23	0x00000000

/*
 * Board-specific values for Orion5x GPIO low level init:
 * - GPIO3 is input (RTC interrupt)
 * - GPIO16 is Power LED control (0 = on, 1 = off)
 * - GPIO17 is Power LED source select (0 = CPLD, 1 = GPIO16)
 * - GPIO18 is Power Button status (0 = Released, 1 = Pressed)
 * - Last GPIO is 26, further bits are supposed to be 0.
 * Enable mask has ones for INPUT, 0 for OUTPUT.
 * Default is LED ON.
 */

#define ORION5X_GPIO_OUT_ENABLE	0x03fcffff
#define ORION5X_GPIO_OUT_VALUE	0x03fcffff

/*
 * NS16550 Configuration
 */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#define CONFIG_SYS_NS16550_COM1		ORION5X_UART0_BASE

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define CONFIG_CONS_INDEX	1	/*Console on UART0 */
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE \
	{ 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 }

/*
 * FLASH configuration
 */

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_FLASH_CFI_LEGACY
#define CONFIG_SYS_MAX_FLASH_BANKS	1  /* max num of flash banks       */
#define CONFIG_SYS_MAX_FLASH_SECT	11 /* max num of sects on one chip */
#define CONFIG_SYS_FLASH_BASE		0xfff80000
#define CONFIG_SYS_FLASH_SECTSZ \
	{16384, 8192, 8192, 32768, \
	 65536, 65536, 65536, 65536, 65536, 65536, 65536}

/* auto boot */
#define CONFIG_BOOTDELAY	3	/* default enable autoboot */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG	1	/* enable INITRD tag */
#define CONFIG_SETUP_MEMORY_TAGS 1	/* enable memory tag */

#define	CONFIG_SYS_PROMPT	"EDMiniV2> "	/* Command Prompt */
#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buff Size */
#define	CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE \
		+sizeof(CONFIG_SYS_PROMPT) + 16)	/* Print Buff */
/*
 * Commands configuration - using default command set for now
 */
#include <config_cmd_default.h>
/*
 * Disabling some default commands for staggered bring-up
 */
#undef CONFIG_CMD_BOOTD	/* no bootd since no net */
#undef CONFIG_CMD_NET	/* no net since no eth */
#undef CONFIG_CMD_NFS	/* no NFS since no net */

/*
 *  Environment variables configurations
 */
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SECT_SIZE		0x2000	/* 16K */
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_OFFSET		0x4000	/* env starts here */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(1024 * 128) /* 128kB for malloc() */
/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

/*
 * Other required minimal configurations
 */
#define CONFIG_CONSOLE_INFO_QUIET	/* some code reduction */
#define CONFIG_ARCH_CPU_INIT		/* call arch_cpu_init() */
#define CONFIG_ARCH_MISC_INIT		/* call arch_misc_init() */
#define CONFIG_DISPLAY_CPUINFO		/* Display cpu info */
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_STACKSIZE		0x00100000
#define CONFIG_SYS_LOAD_ADDR		0x00800000
#define CONFIG_SYS_MEMTEST_START	0x00400000
#define CONFIG_SYS_MEMTEST_END		0x007fffff
#define CONFIG_SYS_RESET_ADDRESS	0xffff0000
#define CONFIG_SYS_MAXARGS		16

#endif /* _CONFIG_EDMINIV2_H */

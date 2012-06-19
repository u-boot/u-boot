/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * This file contains the configuration parameters for qemu-mips target.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1	/* MIPS32 CPU core */
#define CONFIG_QEMU_MIPS	1
#define CONFIG_MISC_INIT_R

/*IP address is default used by Qemu*/
#define CONFIG_IPADDR		10.0.2.15	/* Our IP address */
#define CONFIG_SERVERIP		10.0.2.2	/* Server IP address */

#define CONFIG_BOOTDELAY	10	/* autoboot after 10 seconds */

#define CONFIG_BAUDRATE		115200

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */
#undef CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"addmisc=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate} "				\
		"panic=1\0"						\
	"bootfile=/tftpboot/vmlinux\0"				\
	"load=tftp 80500000 ${u-boot}\0"				\
	""

#define CONFIG_BOOTCOMMAND	"bootp;bootelf"

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

#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#define CONFIG_CMD_DHCP

#define CONFIG_DRIVER_NE2000
#define CONFIG_DRIVER_NE2000_BASE	(0xb4000300)

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		115200
#define CONFIG_SYS_NS16550_COM1	(0xb40003f8)
#define CONFIG_CONS_INDEX	1

#define CONFIG_CMD_IDE
#define CONFIG_DOS_PARTITION

#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_ATA_IDE0_OFFSET	(0x1f0)
#define CONFIG_SYS_ATA_IDE1_OFFSET	(0x170)
#define CONFIG_SYS_ATA_DATA_OFFSET	(0)
#define CONFIG_SYS_ATA_REG_OFFSET	(0)
#define CONFIG_SYS_ATA_BASE_ADDR	(0xb4000000)

#define CONFIG_SYS_IDE_MAXDEVICE	(4)

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory */

#define CONFIG_SYS_PROMPT		"qemu-mips # "	/* Monitor Command Prompt */

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_HUSH_PARSER

#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)  /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */

#define CONFIG_SYS_MALLOC_LEN		128*1024

#define CONFIG_SYS_BOOTPARAMS_LEN	128*1024

#define CONFIG_SYS_MHZ			132

#define CONFIG_SYS_MIPS_TIMER_FREQ	(CONFIG_SYS_MHZ * 1000000)

#define CONFIG_SYS_HZ			1000

#define CONFIG_SYS_SDRAM_BASE		0x80000000	/* Cached addr */

#define CONFIG_SYS_LOAD_ADDR		0x81000000	/* default load address */

#define CONFIG_SYS_MEMTEST_START	0x80100000
#define CONFIG_SYS_MEMTEST_END		0x80800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* The following #defines are needed to get flash environment right */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 << 10)

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

/* We boot from this flash, selected with dip switch */
#define CONFIG_SYS_FLASH_BASE		0xbfc00000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	128
#define CONFIG_SYS_FLASH_CFI		1	/* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER	1
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)

/* Address and size of Primary Environment Sector */
#define CONFIG_ENV_SIZE		0x8000

#define CONFIG_ENV_OVERWRITE	1

#define MEM_SIZE		128

#undef CONFIG_MEMSIZE_IN_BYTES

#define CONFIG_LZMA

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	32

#endif /* __CONFIG_H */

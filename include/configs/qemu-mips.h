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
 * This file contains the configuration parameters for the dbau1x00 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1  /* MIPS32 CPU core	*/
#define CONFIG_QEMU_MIPS        1
#define CONFIG_MISC_INIT_R

#undef DEBUG

/*IP address is default used by Qemu*/
#define CONFIG_IPADDR		10.0.2.15    	     /* Our IP address */
#define CONFIG_SERVERIP		10.0.2.2	     /* Server IP address*/

#define CONFIG_BOOTDELAY	10	/* autoboot after 10 seconds	*/

#define CONFIG_BAUDRATE		115200

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */
#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
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
#undef  CONFIG_CMD_IMLS
#undef  CONFIG_CMD_FLASH
#undef  CONFIG_CMD_LOADB
#undef  CONFIG_CMD_LOADS
#define CONFIG_CMD_DHCP

#define CONFIG_DRIVER_NE2000
#define CONFIG_DRIVER_NE2000_BASE	(0xb4000300)

#define CFG_NO_FLASH
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE    1
#define CFG_NS16550_CLK         115200
#define CFG_NS16550_COM1        (0xb40003f8)
#define CONFIG_CONS_INDEX	1

#define CONFIG_CMD_IDE
#define CONFIG_DOS_PARTITION

#define CFG_IDE_MAXBUS	2
#define CFG_ATA_IDE0_OFFSET	(0x1f0)
#define CFG_ATA_IDE1_OFFSET	(0x170)
#define CFG_ATA_DATA_OFFSET	(0)
#define CFG_ATA_REG_OFFSET	(0)
#define CFG_ATA_BASE_ADDR	(0xb4000000)

#define CFG_IDE_MAXDEVICE	(4)

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory      */

#define	CFG_PROMPT		"qemu-mips # "	/* Monitor Command Prompt    */

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "

#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args*/

#define CFG_MALLOC_LEN		128*1024

#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_MHZ			132

#define CFG_HZ                  (CFG_MHZ * 1000000)

#define CFG_SDRAM_BASE		0x80000000     /* Cached addr */

#define	CFG_LOAD_ADDR		0x81000000     /* default load address	*/

#define CFG_MEMTEST_START	0x80100000
#define CFG_MEMTEST_END		0x80800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* The following #defines are needed to get flash environment right */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#define CFG_INIT_SP_OFFSET	0x400000

/* We boot from this flash, selected with dip switch */
#define CFG_FLASH_BASE		0xbfc00000

#define	CFG_ENV_IS_NOWHERE	1

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_SIZE		0x10000
#undef CONFIG_NET_MULTI

#define MEM_SIZE 128

#undef CONFIG_MEMSIZE_IN_BYTES

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32

#endif	/* __CONFIG_H */

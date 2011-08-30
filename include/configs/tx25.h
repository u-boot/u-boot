/*
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H


/*
 * KARO TX25 board - SoC Configuration
 */
#define CONFIG_MX25
#define CONFIG_MX25_CLK32		32000	/* OSC32K frequency */
#define CONFIG_SYS_HZ			1000

#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* 256 kB for U-Boot */

/* NAND BOOT is the only boot method */
#define CONFIG_NAND_U_BOOT

#ifdef CONFIG_NAND_SPL
/* Start copying real U-boot from the second page */
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x800
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x30000

#define CONFIG_SYS_NAND_U_BOOT_DST      (0x81200000)
#define CONFIG_SYS_NAND_U_BOOT_START    CONFIG_SYS_NAND_U_BOOT_DST

#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_SPARE_SIZE	64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_SIZE		(128 * 1024 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#else
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Memory Info
 */
/* malloc() len */
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)	/* 1 MiB */
/*
 * Board has 2 32MB banks of DRAM but there is a bug when using
 * both so only the first is configured
 */
#define CONFIG_NR_DRAM_BANKS	1

#define PHYS_SDRAM_1		0x80000000
#define PHYS_SDRAM_1_SIZE	0x02000000
#if (CONFIG_NR_DRAM_BANKS == 2)
#define PHYS_SDRAM_2		0x90000000
#define PHYS_SDRAM_2_SIZE	0x02000000
#endif
/* 8MB DRAM test */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1+0x0800000)
#define CONFIG_STACKSIZE	(256 * 1024)	/* regular stack */

/*
 * Serial Info
 */
#define CONFIG_MXC_UART
#define CONFIG_SYS_MX25_UART1
#define CONFIG_CONS_INDEX	1	/* use UART0 for console */
#define CONFIG_BAUDRATE		115200	/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_MXC_GPIO

/*
 * Flash & Environment
 */
/* No NOR flash present */
#define CONFIG_SYS_NO_FLASH
#define	CONFIG_ENV_IS_IN_NAND
#define	CONFIG_ENV_OFFSET	CONFIG_SYS_MONITOR_LEN
#define CONFIG_ENV_SIZE		(128 * 1024)	/* 128 kB NAND block size */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)

/* NAND */
#define CONFIG_NAND_MXC
#define CONFIG_NAND_MXC_V1_1
#define CONFIG_MXC_NAND_REGS_BASE	(0xBB000000)
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		(0xBB000000)
#define CONFIG_JFFS2_NAND
#define CONFIG_MXC_NAND_HWECC
#define CONFIG_SYS_NAND_LARGEPAGE

#define CONFIG_SYS_64BIT_VSPRINTF

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT	"=> "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size  */
/* Print buffer sz */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
		sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

/* U-Boot commands */
#include <config_cmd_default.h>
#define CONFIG_CMD_NAND
#define CONFIG_CMD_CACHE

/*
 * Ethernet
 */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_PHYADDR		0x1f
#define CONFIG_MII
#define CONFIG_CMD_NET
#define CONFIG_NET_MULTI
#define BOARD_LATE_INIT
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BOOTDELAY	5

#define CONFIG_LOADADDR		0x81000000	/* loadaddr env var */
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

#define xstr(s)	str(s)
#define str(s)	#s

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=ttymxc0,${baudrate}\0"			\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addmisc=setenv bootargs ${bootargs}\0"				\
	"u-boot=tx25/u-boot.bin\0"					\
	"kernel_addr_r=" xstr(CONFIG_LOADADDR) "\0"			\
	"hostname=tx25\0"						\
	"bootfile=tx25/uImage\0"					\
	"rootpath=/opt/eldk/arm\0"					\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm\0"						\
	"bootcmd=run net_nfs\0"						\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"update=nand erase 0 40000;nand write ${loadaddr} 0 40000\0"	\
	"upd=run load update\0"						\

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - /* Fix this */ \
					GENERATED_GBL_DATA_SIZE)

#endif /* __CONFIG_H */

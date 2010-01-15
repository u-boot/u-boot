/*
 * Copyright (C) 2009 Ilya Yanok <yanok@emcraft.com>
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
 * SoC Configuration
 */
#define CONFIG_ARM926EJS			/* arm926ejs CPU core */
#define CONFIG_MX27
#define CONFIG_IMX27LITE
#define CONFIG_MX27_CLK32	32768		/* OSC32K frequency */
#define CONFIG_SYS_HZ		1000

#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Lowlevel configuration
 */
#define SDRAM_ESDCFG_REGISTER_VAL(cas)	\
		(ESDCFG_TRC(10) |	\
		ESDCFG_TRCD(3) |	\
		ESDCFG_TCAS(cas) |	\
		ESDCFG_TRRD(1) |	\
		ESDCFG_TRAS(5) |	\
		ESDCFG_TWR |		\
		ESDCFG_TMRD(2) |	\
		ESDCFG_TRP(2) |		\
		ESDCFG_TXP(3))

#define SDRAM_ESDCTL_REGISTER_VAL	\
		(ESDCTL_PRCT(0) |	\
		 ESDCTL_BL |		\
		 ESDCTL_PWDT(0) |	\
		 ESDCTL_SREFR(3) |	\
		 ESDCTL_DSIZ_32 |	\
		 ESDCTL_COL10 |		\
		 ESDCTL_ROW13 |		\
		 ESDCTL_SDE)

#define SDRAM_ALL_VAL		0xf00

#define SDRAM_MODE_REGISTER_VAL	0x33	/* BL: 8, CAS: 3 */
#define SDRAM_EXT_MODE_REGISTER_VAL	0x1000000

#define MPCTL0_VAL	0x1ef15d5

#define SPCTL0_VAL	0x043a1c09

#define CSCR_VAL	0x33f08107

#define PCDR0_VAL	0x120470c3
#define PCDR1_VAL	0x03030303
#define PCCR0_VAL	0xffffffff
#define PCCR1_VAL	0xfffffffc

#define AIPI1_PSR0_VAL	0x20040304
#define AIPI1_PSR1_VAL	0xdffbfcfb
#define AIPI2_PSR0_VAL	0x07ffc200
#define AIPI2_PSR1_VAL	0xffffffff

/*
 * Memory Info
 */
/* malloc() len */
#define CONFIG_SYS_MALLOC_LEN		(0x10000 + 512 * 1024)
/* reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128
/* memtest start address */
#define CONFIG_SYS_MEMTEST_START	0xA0000000
#define CONFIG_SYS_MEMTEST_END		0xA1000000	/* 16MB RAM test */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define CONFIG_STACKSIZE	(256 * 1024)	/* regular stack */
#define PHYS_SDRAM_1		0xA0000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE	0x08000000	/* DDR size 128MB */

/*
 * Serial Driver info
 */
#define CONFIG_MXC_UART
#define CONFIG_SYS_MX27_UART1
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Flash & Environment
 */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
/* Use buffered writes (~10x faster) */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
/* Use hardware sector protection */
#define CONFIG_SYS_FLASH_PROTECTION		1
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of flash banks */
#define CONFIG_SYS_FLASH_SECT_SZ	0x2000	/* 8KB sect size Intel Flash */
/* end of flash */
#define CONFIG_ENV_OFFSET		(PHYS_FLASH_SIZE - 0x20000)
/* CS2 Base address */
#define PHYS_FLASH_1			0xc0000000
/* Flash Base for U-Boot */
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
/* Flash size 2MB */
#define PHYS_FLASH_SIZE			0x200000
#define CONFIG_SYS_MAX_FLASH_SECT	(PHYS_FLASH_SIZE / \
		CONFIG_SYS_FLASH_SECT_SZ)
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		0x40000		/* Reserve 256KiB */
#define CONFIG_ENV_SECT_SIZE		0x10000		/* Env sector Size */
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

/*
 * Ethernet
 */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_PHYADDR		0x1f
#define CONFIG_MII
#define CONFIG_NET_MULTI

/*
 * MTD
 */
#define CONFIG_FLASH_CFI_MTD
#define CONFIG_MTD_DEVICE

/*
 * NAND
 */
#define CONFIG_NAND_MXC
#define CONFIG_MXC_NAND_REGS_BASE	0xd8000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0xd8000000
#define CONFIG_JFFS2_NAND
#define CONFIG_MXC_NAND_HWECC
#define CONFIG_SYS_64BIT_VSPRINTF	/* needed for nand_util.c */

/*
 * SD/MMC
 */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_MXC_MMC
#define CONFIG_MXC_MCI_REGS_BASE	0x10014000
#define CONFIG_DOS_PARTITION

/*
 * MTD partitions
 */
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor0=physmap-flash.0,nand0=mxc_nand.0"
#define MTDPARTS_DEFAULT			\
	"mtdparts="				\
		"physmap-flash.0:"		\
			"256k(U-Boot),"		\
			"1664k(user),"		\
			"64k(env1),"		\
			"64k(env2);"		\
		"mxc_nand.0:"			\
			"128k(IPL-SPL),"	\
			"4m(kernel),"		\
			"22m(rootfs),"		\
			"-(userfs)"

/*
 * U-Boot general configuration
 */
#define CONFIG_BOOTFILE		"uImage"	/* Boot file name */
#define CONFIG_SYS_PROMPT	"=> "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size  */
/* Print buffer sz */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
		sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

/*
 * U-Boot commands
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_FAT
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PING

#define CONFIG_BOOTDELAY	5

#define CONFIG_LOADADDR		0xa0800000	/* loadaddr env var */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

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
	"u-boot=imx27/u-boot.bin\0"					\
	"kernel_addr_r=a0800000\0"					\
	"hostname=imx27\0"						\
	"bootfile=imx27/uImage\0"					\
	"rootpath=/opt/eldk-4.2-arm/arm\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm\0"						\
	"bootcmd=run net_nfs\0"					\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"update=protect off " xstr(CONFIG_SYS_MONITOR_BASE)		\
		" +${filesize};era " xstr(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};cp.b ${fileaddr} "			\
		xstr(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0"		\
	"upd=run load update\0"						\

#endif /* __CONFIG_H */

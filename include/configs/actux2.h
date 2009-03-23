/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
 * Configuration settings for the AcTux-2 board.
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

#define CONFIG_IXP425			1
#define CONFIG_ACTUX2			1

#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

#define CONFIG_IXP_SERIAL
#define CONFIG_SYS_IXP425_CONSOLE		IXP425_UART2
#define CONFIG_BAUDRATE			115200
#define CONFIG_BOOTDELAY		5
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

/***************************************************************
 * U-boot generic defines start here.
 ***************************************************************/
#undef CONFIG_USE_IRQ

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN			(CONFIG_ENV_SIZE + 128*1024)
/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE		128

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Command line configuration. */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#undef CONFIG_CMD_PCI
#undef CONFIG_PCI

#define CONFIG_BOOTCOMMAND		"run boot_flash"
/* enable passing of ATAGs */
#define CONFIG_CMDLINE_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1

#if defined(CONFIG_CMD_KGDB)
# define CONFIG_KGDB_BAUDRATE		230400
/* which serial port to use */
# define CONFIG_KGDB_SER_INDEX		1
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT			"=> "
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE			256
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS			16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE			CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START		0x00400000
#define CONFIG_SYS_MEMTEST_END			0x00800000

/* spec says 66.666 MHz, but it appears to be 33 */
#define CONFIG_SYS_HZ				3333333

/* default load address */
#define CONFIG_SYS_LOAD_ADDR			0x00010000

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE		{ 9600, 19200, 38400, 57600, \
					  115200, 230400 }
#define CONFIG_SERIAL_RTS_ACTIVE	1

/*
 * Stack sizes
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE		(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
# define CONFIG_STACKSIZE_IRQ		(4*1024)	/* IRQ stack */
# define CONFIG_STACKSIZE_FIQ		(4*1024)	/* FIQ stack */
#endif

/* Expansion bus settings */
#define CONFIG_SYS_EXP_CS0			0xbd113042

/* SDRAM settings */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x00000000
#define CONFIG_SYS_DRAM_BASE			0x00000000

/* 16MB SDRAM */
#define CONFIG_SYS_SDR_CONFIG			0x3A
#define PHYS_SDRAM_1_SIZE		0x01000000
#define CONFIG_SYS_SDRAM_REFRESH_CNT		0x81a
#define CONFIG_SYS_SDR_MODE_CONFIG		0x1
#define CONFIG_SYS_DRAM_SIZE			0x01000000

/* FLASH organization */
#define CONFIG_SYS_MAX_FLASH_BANKS		1
/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT		140
#define PHYS_FLASH_1			0x50000000
#define CONFIG_SYS_FLASH_BANKS_LIST		{ PHYS_FLASH_1 }

#define CONFIG_SYS_FLASH_BASE			PHYS_FLASH_1
#define CONFIG_SYS_MONITOR_BASE		PHYS_FLASH_1
#define CONFIG_SYS_MONITOR_LEN			(256 << 10)

/* Use common CFI driver */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
/* no byte writes on IXP4xx */
#define CONFIG_SYS_FLASH_CFI_WIDTH		FLASH_CFI_16BIT

/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_EMPTY_INFO

/* Ethernet */

/* include IXP4xx NPE support */
#define CONFIG_IXP4XX_NPE		1
#define CONFIG_NET_MULTI		1
/* NPE0 PHY address */
#define	CONFIG_PHY_ADDR			0x00
/* MII PHY management */
#define CONFIG_MII			1
/* Number of ethernet rx buffers & descriptors */
#define CONFIG_SYS_RX_ETH_BUFFER		16
#define CONFIG_RESET_PHY_R		1
/* ethernet switch connected to MII port */
#define CONFIG_MII_ETHSWITCH		1

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#undef  CONFIG_CMD_NFS

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* Cache Configuration */
#define CONFIG_SYS_CACHELINE_SIZE		32

/*
 * environment organization:
 * one flash sector, embedded in uboot area (bottom bootblock flash)
 */
#define	CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_ADDR			(PHYS_FLASH_1 + 0x4000)
#define CONFIG_SYS_USE_PPCENV			1

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"npe_ucode=50040000\0"						\
	"mtd=IXP4XX-Flash.0:256k(uboot),64k(ucode),1152k(linux),-(root)\0" \
	"kerneladdr=50050000\0"						\
	"rootaddr=50170000\0"						\
	"loadaddr=10000\0"						\
	"updateboot_ser=mw.b 10000 ff 40000;"				\
	" loady ${loadaddr};"						\
	" run eraseboot writeboot\0"					\
	"updateboot_net=mw.b 10000 ff 40000;"				\
	" tftp ${loadaddr} u-boot.bin;"					\
	" run eraseboot writeboot\0"					\
	"eraseboot=protect off 50000000 50003fff;"			\
	" protect off 50006000 5003ffff;"				\
	" erase 50000000 50003fff;"					\
	" erase 50006000 5003ffff\0"					\
	"writeboot=cp.b 10000 50000000 4000;"				\
	" cp.b 16000 50006000 3a000\0"					\
	"eraseenv=protect off 50004000 50005fff;"			\
	" erase 50004000 50005fff\0"					\
	"updateroot=tftp ${loadaddr} ${rootfile};"			\
	" era ${rootaddr} +${filesize};"				\
	" cp.b ${loadaddr} ${rootaddr} ${filesize}\0"			\
	"updatekern=tftp ${loadaddr} ${kernelfile};"			\
	" era ${kerneladdr} +${filesize};"				\
	" cp.b ${loadaddr} ${kerneladdr} ${filesize}\0"			\
	"flashargs=setenv bootargs mtdparts=${mtd} root=/dev/mtdblock3"	\
	" rootfstype=squashfs,jffs2 init=/etc/preinit\0"		\
	"netargs=setenv bootargs mtdparts=${mtd} root=/dev/mtdblock3"	\
	" rootfstype=squashfs,jffs2 init=/etc/preinit\0"		\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
	"addeth=setenv bootargs ${bootargs} ethaddr=${ethaddr}\0"	\
	"boot_flash=run flashargs addtty addeth;"			\
	" bootm ${kerneladdr}\0"					\
	"boot_net=run netargs addtty addeth;"				\
	" tftpboot ${loadaddr} ${kernelfile};"				\
	" bootm\0"

#endif /* __CONFIG_H */

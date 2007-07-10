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
#define CONFIG_PB1X00		1
#define CONFIG_AU1X00		1  /* alchemy series cpu */

#ifdef CONFIG_PB1000
#define CONFIG_AU1000		1
#else
#ifdef CONFIG_PB1100
#define CONFIG_AU1100		1
#else
#ifdef CONFIG_PB1500
#define CONFIG_AU1500		1
#else
#error "No valid board set"
#endif
#endif
#endif

#define CONFIG_ETHADDR		DE:AD:BE:EF:01:01    /* Ethernet address */

#define CONFIG_BOOTDELAY	2	/* autoboot after 2 seconds	*/

#define CONFIG_BAUDRATE		115200

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */
#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"addmisc=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate} "				\
		"panic=1\0"						\
	"bootfile=/vmlinux.img\0"				\
	"load=tftp 80500000 ${u-boot}\0"				\
	""
/* Boot from NFS root */
#define CONFIG_BOOTCOMMAND	"bootp; setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; bootm"

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory      */
#define	CFG_PROMPT		"Pb1x00 # "	/* Monitor Command Prompt    */
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args*/

#define CFG_MALLOC_LEN		128*1024

#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_HZ			396000000      /* FIXME causes overflow in net.c */

#define CFG_SDRAM_BASE		0x80000000     /* Cached addr */

#define	CFG_LOAD_ADDR		0x81000000     /* default load address	*/

#define CFG_MEMTEST_START	0x80100000
#undef CFG_MEMTEST_START
#define CFG_MEMTEST_START       0x80200000
#define CFG_MEMTEST_END		0x83800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(128)	/* max number of sectors on one chip */

#define PHYS_FLASH_1		0xbec00000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0xbfc00000 /* Flash Bank #2 */

/* The following #defines are needed to get flash environment right */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#define CFG_INIT_SP_OFFSET	0x4000000

/* We boot from this flash, selected with dip switch */
#define CFG_FLASH_BASE		PHYS_FLASH_2

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_NOWHERE	1

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		0xB0030000
#define CFG_ENV_SIZE		0x10000

#define CONFIG_FLASH_16BIT

#define CONFIG_NR_DRAM_BANKS	2

#define CONFIG_NET_MULTI

#define CONFIG_MEMSIZE_IN_BYTES


/*---USB -------------------------------------------*/
#if 0
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#endif

/*---ATA PCMCIA ------------------------------------*/
#if 0
#define CFG_PCMCIA_MEM_SIZE 0x4000000 /* Offset to slot 1 FIXME!!! */
#define CFG_PCMCIA_MEM_ADDR 0x20000000
#define CONFIG_PCMCIA_SLOT_A

#define CONFIG_ATAPI 1
#define CONFIG_MAC_PARTITION 1

/* We run CF in "true ide" mode or a harddrive via pcmcia */
#define CONFIG_IDE_PCMCIA 1

/* We only support one slot for now */
#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR       CFG_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET     8

/* Offset for normal register accesses  */
#define CFG_ATA_REG_OFFSET      0

/* Offset for alternate registers       */
#define CFG_ATA_ALT_OFFSET      0x0100

#endif
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32


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

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING

#undef CONFIG_CMD_ENV
#undef CONFIG_CMD_FAT
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IDE
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_RUN
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_ELF
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_BEDBUG

#endif	/* __CONFIG_H */

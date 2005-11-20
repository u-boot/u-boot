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
#define CONFIG_DBAU1X00		1
#define CONFIG_AU1X00		1  /* alchemy series cpu */

#ifdef CONFIG_DBAU1000
/* Also known as Merlot */
#define CONFIG_AU1000		1
#else
#ifdef CONFIG_DBAU1100
#define CONFIG_AU1100		1
#else
#ifdef CONFIG_DBAU1500
#define CONFIG_AU1500		1
#else
#ifdef CONFIG_DBAU1550
/* Cabernet */
#define CONFIG_AU1550           1
#else
#error "No valid board set"
#endif
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
	"bootfile=/tftpboot/vmlinux.srec\0"				\
	"load=tftp 80500000 ${u-boot}\0"				\
	""

#ifdef CONFIG_DBAU1550
/* Boot from flash by default, revert to bootp */
#define CONFIG_BOOTCOMMAND	"bootm 0xbfc20000; bootp; bootm"

#define CONFIG_COMMANDS		((CONFIG_CMD_DFL | CFG_CMD_FLASH | CFG_CMD_LOADB | CFG_CMD_NET) & \
				 ~(CFG_CMD_ENV | CFG_CMD_FAT | CFG_CMD_FPGA | CFG_CMD_IDE | \
				   CFG_CMD_MII | CFG_CMD_RUN | CFG_CMD_BDI | CFG_CMD_BEDBUG | \
				   CFG_CMD_NFS | CFG_CMD_ELF | CFG_CMD_PCMCIA | CFG_CMD_I2C))
#else /* CONFIG_DBAU1550 */
/* Boot from Compact flash partition 2 as default */
#define CONFIG_BOOTCOMMAND	"ide reset;disk 0x81000000 0:2;bootm"

#define CONFIG_COMMANDS		((CONFIG_CMD_DFL | CFG_CMD_IDE | CFG_CMD_DHCP | CFG_CMD_ELF) & \
				 ~(CFG_CMD_ENV | CFG_CMD_FAT | CFG_CMD_FLASH | CFG_CMD_FPGA | \
				   CFG_CMD_MII | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB | \
				   CFG_CMD_ELF | CFG_CMD_BDI | CFG_CMD_BEDBUG))
#endif /* CONFIG_DBAU1550 */

#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory      */

#define	CFG_PROMPT		"DbAu1xx0 # "	/* Monitor Command Prompt    */

#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args*/

#define CFG_MALLOC_LEN		128*1024

#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_MHZ			396

#if (CFG_MHZ % 12) != 0
#error "Invalid CPU frequency - must be multiple of 12!"
#endif

#define CFG_HZ                  (CFG_MHZ * 1000000) /* FIXME causes overflow in net.c */

#define CFG_SDRAM_BASE		0x80000000     /* Cached addr */

#define	CFG_LOAD_ADDR		0x81000000     /* default load address	*/

#define CFG_MEMTEST_START	0x80100000
#define CFG_MEMTEST_END		0x80800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#ifdef CONFIG_DBAU1550

#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(512)	/* max number of sectors on one chip */

#define PHYS_FLASH_1		0xb8000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0xbc000000 /* Flash Bank #2 */

#define CFG_FLASH_BANKS_LIST {PHYS_FLASH_1, PHYS_FLASH_2}

#else /* CONFIG_DBAU1550 */

#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(128)	/* max number of sectors on one chip */

#define PHYS_FLASH_1		0xbec00000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0xbfc00000 /* Flash Bank #2 */

#endif /* CONFIG_DBAU1550 */

#define CFG_FLASH_CFI           1
#define CFG_FLASH_CFI_DRIVER    1

/* The following #defines are needed to get flash environment right */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#define CFG_INIT_SP_OFFSET	0x400000

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

#ifdef CONFIG_DBAU1550
#define MEM_SIZE 192
#else
#define MEM_SIZE 64
#endif

#define CONFIG_MEMSIZE_IN_BYTES

#ifndef CONFIG_DBAU1550
/*---ATA PCMCIA ------------------------------------*/
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
#endif /* CONFIG_DBAU1550 */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32

#endif	/* __CONFIG_H */

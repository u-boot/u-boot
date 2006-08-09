/*
 * (C) Copyright 2005
 * Thomas.Lange@corelatus.se
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * This file contains the configuration parameters for the gth2 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1  /* MIPS32 CPU core	*/
#define CONFIG_GTH2		1
#define CONFIG_AU1X00		1  /* alchemy series cpu */

#define CONFIG_AU1000		1

#define CONFIG_MISC_INIT_R	1

#define CONFIG_ETHADDR		DE:AD:BE:EF:01:02    /* Ethernet address */

#define CONFIG_BOOTDELAY	1	/* autoboot after 1 seconds	*/

#define CONFIG_ENV_OVERWRITE 1 /* Allow change of ethernet address */

#define CONFIG_BOOT_RETRY_TIME 5 /* Retry boot in 5 secs */

#define CONFIG_RESET_TO_RETRY 1 /* If timeout waiting for command, perform a reset */

#define CONFIG_BAUDRATE		115200

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 115200 }

/* Only interrupt boot if space is pressed */
/* If a long serial cable is connected but */
/* other end is dead, garbage will be read */
#define CONFIG_AUTOBOOT_KEYED 1
#define CONFIG_AUTOBOOT_PROMPT "Press space to abort autoboot in %d second\n"
#define CONFIG_AUTOBOOT_DELAY_STR "d"
#define CONFIG_AUTOBOOT_STOP_STR " "

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */
#define CONFIG_BOOTARGS "panic=1"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"addmisc=setenv bootargs $(bootargs) "				\
		"ethaddr=$(ethaddr) \0"					\
	"netboot=bootp;run addmisc;bootm\0"				\
		""

/* Boot from Compact flash partition 2 as default */
#define CONFIG_BOOTCOMMAND	"ide reset;disk 0x81000000 0:2;run addmisc;bootm"

#define CONFIG_COMMANDS ((CONFIG_CMD_DFL | CFG_CMD_IDE | CFG_CMD_DHCP ) & \
 ~(CFG_CMD_ENV | CFG_CMD_FAT | CFG_CMD_FLASH | CFG_CMD_FPGA | \
   CFG_CMD_MII | CFG_CMD_LOADS	| CFG_CMD_LOADB | CFG_CMD_ELF | \
   CFG_CMD_BDI | CFG_CMD_BEDBUG | CFG_CMD_NFS | CFG_CMD_AUTOSCRIPT ))

#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory	     */
#define CFG_PROMPT		"GTH2 # "	/* Monitor Command Prompt    */
#define CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args*/

#define CFG_MALLOC_LEN		128*1024

#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_MHZ			500

#define CFG_HZ			(CFG_MHZ * 1000000) /* FIXME causes overflow in net.c */

#define CFG_SDRAM_BASE		0x80000000     /* Cached addr */

#define CFG_LOAD_ADDR		0x81000000     /* default load address	*/

#define CFG_MEMTEST_START	0x80100000
#define CFG_MEMTEST_END		0x83000000

#define CONFIG_HW_WATCHDOG	1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(128)	/* max number of sectors on one chip */

#define PHYS_FLASH		0xbfc00000 /* Flash Bank #1 */

/* The following #defines are needed to get flash environment right */
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 << 10)

#define CFG_INIT_SP_OFFSET	0x400000

/* We boot from this flash, selected with dip switch */
#define CFG_FLASH_BASE		PHYS_FLASH

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

#define CFG_ENV_IS_NOWHERE	1

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		0xB0030000
#define CFG_ENV_SIZE		0x10000

#define CONFIG_FLASH_16BIT

#define CONFIG_NR_DRAM_BANKS	2

#define CONFIG_NET_MULTI

#define CONFIG_MEMSIZE_IN_BYTES

/*---ATA PCMCIA ------------------------------------*/
#define CFG_PCMCIA_MEM_SIZE 0x4000000 /* Offset to slot 1 FIXME!!! */

#define CFG_PCMCIA_MEM_ADDR  0x20000000
#define CFG_PCMCIA_IO_BASE   0x28000000
#define CFG_PCMCIA_ATTR_BASE 0x30000000

#define CONFIG_PCMCIA_SLOT_A

#define CONFIG_ATAPI 1
#define CONFIG_MAC_PARTITION 1

/* We run CF in "true ide" mode or a harddrive via pcmcia */
#define CONFIG_IDE_PCMCIA 1

/* We only support one slot for now */
#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#undef	CONFIG_IDE_LED			/* LED	 for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CFG_ATA_IDE0_OFFSET	0

#define CFG_ATA_BASE_ADDR	CFG_PCMCIA_IO_BASE

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	0

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	0

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	0x0200

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32

#define GPIO_CACONFIG  (1<<0)
#define GPIO_DPACONFIG (1<<6)
#define GPIO_ERESET    (1<<11)
#define GPIO_EEDQ      (1<<17)
#define GPIO_WDI       (1<<18)
#define GPIO_RJ1LY     (1<<22)
#define GPIO_RJ1LG     (1<<23)
#define GPIO_LEDCLK    (1<<29)
#define GPIO_LEDD      (1<<30)
#define GPIO_CPU_LED   (1<<31)

#endif	/* __CONFIG_H */

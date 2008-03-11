/*
 * Configuation settings for the Renesas R7780MP board
 *
 * Copyright (C) 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * Copyright (C) 2008 Yusuke Goda <goda.yusuke@renesas.com>
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

#ifndef __R7780RP_H
#define __R7780RP_H

#undef DEBUG
#define CONFIG_SH		1
#define CONFIG_SH4A		1
#define CONFIG_CPU_SH7780	1
#define CONFIG_R7780MP		1
#define __LITTLE_ENDIAN 1

/*
 * Command line configuration.
 */
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_PCI
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_ENV
#define CONFIG_CMD_NFS
#define CONFIG_CMD_IDE
#define CONFIG_CMD_EXT2
#define CONFIG_DOS_PARTITION

#define CFG_SCIF_CONSOLE	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_CONS_SCIF0	1

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"console=ttySC0,115200"
#define CONFIG_ENV_OVERWRITE	1

/* check for keypress on bootdelay==0 */
/*#define CONFIG_ZERO_BOOTDELAY_CHECK*/

/* Network setting */
#define CONFIG_NETMASK		255.0.0.0
#define CONFIG_IPADDR		10.0.192.82
#define CONFIG_SERVERIP		10.0.0.1
#define CONFIG_GATEWAYIP	10.0.0.1

#define CFG_SDRAM_BASE		(0x08000000)
#define CFG_SDRAM_SIZE		(128 * 1024 * 1024)

#define CFG_LONGHELP
#define CFG_PROMPT		"=> "
#define CFG_CBSIZE		256
#define CFG_PBSIZE		256
#define CFG_MAXARGS		16
#define CFG_BARGSIZE	512
/* List of legal baudrate settings for this board */
#define CFG_BAUDRATE_TABLE	{ 115200, 57600, 38400, 19200, 9600 }

#define CFG_MEMTEST_START	(CFG_SDRAM_BASE)
#define CFG_MEMTEST_END		(TEXT_BASE - 0x100000)

/* NOR Flash (S29PL127J60TFI130) */
#define CFG_FLASH_BASE		(0xA0000000)
#define CFG_FLASH_CFI_WIDTH FLASH_CFI_32BIT
#define CFG_MAX_FLASH_BANKS (2)
#define CFG_MAX_FLASH_SECT  270
#define CFG_FLASH_BANKS_LIST    { CFG_FLASH_BASE,\
				CFG_FLASH_BASE + 0x100000,\
				CFG_FLASH_BASE + 0x400000,\
				CFG_FLASH_BASE + 0x700000, }

#define CFG_LOAD_ADDR		(CFG_SDRAM_BASE + 4 * 1024 * 1024)
/* Address of u-boot image in Flash */
#define CFG_MONITOR_BASE	(CFG_FLASH_BASE)
#define CFG_MONITOR_LEN		(112 * 1024)
/* Size of DRAM reserved for malloc() use */
#define CFG_MALLOC_LEN		(256 * 1024)

/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_SIZE	(256)
#define CFG_BOOTMAPSZ		(8 * 1024 * 1024)
#define CFG_RX_ETH_BUFFER	(8)

#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#undef CFG_FLASH_CFI_BROKEN_TABLE
#undef  CFG_FLASH_QUIET_TEST
/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_EMPTY_INFO

#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	(16 * 1024)
#define CFG_ENV_SIZE		(CFG_ENV_SECT_SIZE)
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_FLASH_ERASE_TOUT  	120000
#define CFG_FLASH_WRITE_TOUT	500

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	33333333
#define TMU_CLK_DIVIDER		4
#define CFG_HZ	(CONFIG_SYS_CLK_FREQ / TMU_CLK_DIVIDER)

/* PCI Controller */
#if defined(CONFIG_CMD_PCI)
#define CONFIG_PCI
#define CONFIG_SH4_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW	1
#define __io
#define __mem_pci

#define CONFIG_PCI_MEM_BUS	0xFD000000	/* Memory space base addr */
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x01000000	/* Size of Memory window */

#define CONFIG_PCI_IO_BUS	0xFE200000	/* IO space base address */
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x00200000	/* Size of IO window */
#endif /* CONFIG_CMD_PCI */

#if defined(CONFIG_CMD_NET)
/* #define CONFIG_NET_MULTI
   #define CONFIG_RTL8169 */
/* AX88696L Support(NE2000 base chip) */
#define CONFIG_DRIVER_NE2000
#define CONFIG_DRIVER_AX88796L
#define CONFIG_DRIVER_NE2000_BASE	0xA4100000
#endif

/* Compact flash Support */
#if defined(CONFIG_CMD_IDE)
#define CONFIG_IDE_RESET        1
#define CFG_PIO_MODE            1
#define CFG_IDE_MAXBUS          1   /* IDE bus */
#define CFG_IDE_MAXDEVICE       1
#define CFG_ATA_BASE_ADDR       0xb4000000
#define CFG_ATA_STRIDE          2               /* 1bit shift */
#define CFG_ATA_DATA_OFFSET     0x1000          /* data reg offset */
#define CFG_ATA_REG_OFFSET      0x1000          /* reg offset */
#define CFG_ATA_ALT_OFFSET      0x800           /* alternate register offset */
#endif /* CONFIG_CMD_IDE */

#endif /* __R7780RP_H */

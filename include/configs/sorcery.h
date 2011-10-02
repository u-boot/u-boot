/*
 * (C) Copyright 2004
 * TsiChung Liew, Freescale Software Engineering, Tsi-Chung.Liew@freescale.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC8220		1
#define CONFIG_SORCERY		1	/* Sorcery board */

#define	CONFIG_SYS_TEXT_BASE	0xfff00000

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* Input clock running at 60Mhz, read Hid1 for the CPU multiplier to
   determine the CPU speed. */
#define CONFIG_SYS_MPC8220_CLKIN	60000000 /* ... running at 60MHz */
#define CONFIG_SYS_MPC8220_SYSPLL_VCO_MULTIPLIER 8 /* VCO multiplier can't be read from any register */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC */

#define CONFIG_BAUDRATE		115200	    /* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/* PCI */
#define CONFIG_PCI              1
#define CONFIG_PCI_PNP          1

#define CONFIG_PCI_MEM_BUS      0x80000000
#define CONFIG_PCI_MEM_PHYS     CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE     0x10000000

#define CONFIG_PCI_IO_BUS	0x71000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CONFIG_PCI_CFG_BUS	0x70000000
#define CONFIG_PCI_CFG_PHYS	CONFIG_PCI_CFG_BUS
#define CONFIG_PCI_CFG_SIZE	0x01000000


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

#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP


/*
 * Default Environment
 */
#define CONFIG_BOOTDELAY	5    /* autoboot after 5 seconds */
#define CONFIG_HOSTNAME		sorcery

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=$serverip:$rootpath\0"				\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs $bootargs "				\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask"		\
		":$hostname:$netdev:off panic=1\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm $kernel_addr\0"					\
	"flash_self=run ramargs addip;"					\
		"bootm $kernel_addr $ramdisk_addr\0"			\
	"net_nfs=tftp 200000 $bootfile;run nfsargs addip;bootm\0"	\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"bootfile=/tftpboot/sorcery/uImage\0"				\
	"kernel_addr=FFE00000\0"					\
	"ramdisk_addr=FFB00000\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

#define CONFIG_EEPRO100

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1
#define CONFIG_SYS_I2C_MODULE		1
#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif

/*
 * Flexbus Chipselect configuration
 * Beware: Some CS# seem to be mandatory (if these CS# are not set,
 * board can hang-up in unpredictable place).
 * Sorcery_Memory_Map v0.3 is possibly wrong with CPLD CS#
 */

/* Flash */
#define CONFIG_SYS_CS0_BASE		0xf800
#define CONFIG_SYS_CS0_MASK		0x08000000 /* 128 MB (two chips) */
#define CONFIG_SYS_CS0_CTRL		0x001019c0

/* NVM */
#define CONFIG_SYS_CS1_BASE		0xf7e8
#define CONFIG_SYS_CS1_MASK		0x00040000 /* 256K */
#define CONFIG_SYS_CS1_CTRL		0x00101940 /* 8bit port size */

/* Atlas2 + Gemini */
#define CONFIG_SYS_CS2_BASE		0xf7e7
#define CONFIG_SYS_CS2_MASK		0x00010000 /* 64K*/
#define CONFIG_SYS_CS2_CTRL		0x001011c0 /* 16bit port size */

/* CAN Controller */
#define CONFIG_SYS_CS3_BASE		0xf7e6
#define CONFIG_SYS_CS3_MASK		0x00010000 /* 64K */
#define CONFIG_SYS_CS3_CTRL		0x00102140 /* 8Bit port size */

/* Foreign interface */
#define CONFIG_SYS_CS4_BASE		0xf7e5
#define CONFIG_SYS_CS4_MASK		0x00010000 /* 64K */
#define CONFIG_SYS_CS4_CTRL		0x00101dc0 /* 16bit port size */

/* CPLD */
#define CONFIG_SYS_CS5_BASE		0xf7e4
#define CONFIG_SYS_CS5_MASK		0x00010000 /* 64K */
#define CONFIG_SYS_CS5_CTRL		0x001000c0 /* 16bit port size */

#define CONFIG_SYS_FLASH0_BASE		(CONFIG_SYS_CS0_BASE << 16)
#define CONFIG_SYS_FLASH_BASE		(CONFIG_SYS_FLASH0_BASE)

#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max num of flash banks */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max num of sects on one chip */

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE,  \
				CONFIG_SYS_FLASH_BASE+0x04000000 } /* two banks */

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x8000000 - 0x40000)
#define CONFIG_ENV_SIZE		0x4000                       /* 16K */
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + 0x20000)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

#define CONFIG_ENV_OVERWRITE	1

#if defined CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_IN_NVRAM
#undef CONFIG_ENV_IS_IN_EEPROM
#elif defined CONFIG_ENV_IS_IN_NVRAM
#undef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_IN_EEPROM
#elif defined CONFIG_ENV_IS_IN_EEPROM
#undef CONFIG_ENV_IS_IN_NVRAM
#undef CONFIG_ENV_IS_IN_FLASH
#endif

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000
#define CONFIG_SYS_SRAM_BASE		(CONFIG_SYS_MBAR + 0x20000)
#define CONFIG_SYS_SRAM_SIZE		0x8000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_MBAR + 0x20000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000	/* Size of used area in DPRAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(256 << 10) /* Reserve 256 kB for Monitor   */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10) /* Reserve 128 kB for malloc()  */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)   /* Initial Memory map for Linux */

/* SDRAM configuration (for SPD) */
#define CONFIG_SYS_SDRAM_TOTAL_BANKS		1
#define CONFIG_SYS_SDRAM_SPD_I2C_ADDR		0x50		/* 7bit */
#define CONFIG_SYS_SDRAM_SPD_SIZE		0x100
#define CONFIG_SYS_SDRAM_CAS_LATENCY		5		/* (CL=2.5)x2 */

/* SDRAM drive strength register (for SSTL_2 class II)*/
#define CONFIG_SYS_SDRAM_DRIVE_STRENGTH	((DRIVE_STRENGTH_HIGH << SDRAMDS_SBE_SHIFT) | \
					 (DRIVE_STRENGTH_HIGH << SDRAMDS_SBC_SHIFT) | \
					 (DRIVE_STRENGTH_HIGH << SDRAMDS_SBA_SHIFT) | \
					 (DRIVE_STRENGTH_HIGH << SDRAMDS_SBS_SHIFT) | \
					 (DRIVE_STRENGTH_HIGH << SDRAMDS_SBD_SHIFT))

/*
 * Ethernet configuration
 */
#define CONFIG_MPC8220_FEC	1
#define CONFIG_FEC_10MBIT	1 /* Workaround for FEC 100Mbit problem */
#define CONFIG_PHY_ADDR		0x1F
#define CONFIG_MII		1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			    /* undef to save memory	*/
#define CONFIG_SYS_PROMPT		"=> "	    /* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	    /* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE		256	    /* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	    /* max number of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE  /* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START	0x00100000  /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000  /* 1 ... 15 MB in DRAM  */

#define CONFIG_SYS_LOAD_ADDR		0x100000    /* default load address */

#define CONFIG_SYS_HZ			1000	    /* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC8220 CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		0
#define CONFIG_SYS_HID0_FINAL		0

/*
#define CONFIG_SYS_HID0_INIT           HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL          HID0_ICE
*/

#endif /* __CONFIG_H */

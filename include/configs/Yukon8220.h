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
#define CONFIG_YUKON8220	1	/* ... on Yukon board	*/

/* Input clock running at 30Mhz, read Hid1 for the CPU multiplier to
   determine the CPU speed. */
#define CFG_MPC8220_CLKIN	30000000/* ... running at 30MHz */
#define CFG_MPC8220_SYSPLL_VCO_MULTIPLIER 16 /* VCO multiplier can't be read from any register */

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot	*/

/*
 * Serial console configuration
 */

/* Define this for PSC console
#define CONFIG_PSC_CONSOLE	1
*/

#define CONFIG_EXTUART_CONSOLE	1

#ifdef CONFIG_EXTUART_CONSOLE
#   define CONFIG_CONS_INDEX	1
#   define CFG_NS16550_SERIAL
#   define CFG_NS16550
#   define CFG_NS16550_REG_SIZE 1
#   define CFG_NS16550_COM1	(CFG_CPLD_BASE + 0x1008)
#   define CFG_NS16550_CLK	18432000
#endif

#define CONFIG_BAUDRATE		115200	    /* ... at 115200 bps */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_TIMESTAMP			/* Print image info with timestamp */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP


#define CONFIG_NET_MULTI
#define CONFIG_MII

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5    /* autoboot after 5 seconds */
#define CONFIG_BOOTARGS		"root=/dev/ram rw"
#define CONFIG_ETHADDR		00:e0:0c:bc:e0:60
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		00:e0:0c:bc:e0:61
#define CONFIG_IPADDR		192.162.1.2
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_SERVERIP		192.162.1.1
#define CONFIG_GATEWAYIP	192.162.1.1
#define CONFIG_HOSTNAME		yukon
#define CONFIG_OVERWRITE_ETHADDR_ONCE


/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1
#define CFG_I2C_MODULE		1

#define CFG_I2C_SPEED		100000 /* 100 kHz */
#define CFG_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CFG_I2C_EEPROM_ADDR		0x52	/* 1011000xb */
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	70
/*
#define CFG_ENV_IS_IN_EEPROM	1
#define CFG_ENV_OFFSET		0
#define CFG_ENV_SIZE		256
*/

/* If CFG_AMD_BOOT is defined, the the system will boot from AMD.
   else undefined it will boot from Intel Strata flash */
#define CFG_AMD_BOOT		1

/*
 * Flexbus Chipselect configuration
 */
#if defined (CFG_AMD_BOOT)
#define CFG_CS0_BASE		0xfff0
#define CFG_CS0_MASK		0x00080000  /* 512 KB */
#define CFG_CS0_CTRL		0x003f0d40

#define CFG_CS1_BASE		0xfe00
#define CFG_CS1_MASK		0x01000000  /* 16 MB */
#define CFG_CS1_CTRL		0x003f1540
#else
#define CFG_CS0_BASE		0xff00
#define CFG_CS0_MASK		0x01000000  /* 16 MB */
#define CFG_CS0_CTRL		0x003f1540

#define CFG_CS1_BASE		0xfe08
#define CFG_CS1_MASK		0x00080000  /* 512 KB */
#define CFG_CS1_CTRL		0x003f0d40
#endif

#define CFG_CS2_BASE		0xf100
#define CFG_CS2_MASK		0x00040000
#define CFG_CS2_CTRL		0x003f1140

#define CFG_CS3_BASE		0xf200
#define CFG_CS3_MASK		0x00040000
#define CFG_CS3_CTRL		0x003f1100


#define CFG_FLASH0_BASE		(CFG_CS0_BASE << 16)
#define CFG_FLASH1_BASE		(CFG_CS1_BASE << 16)

#if defined (CFG_AMD_BOOT)
#define CFG_AMD_BASE		CFG_FLASH0_BASE
#define CFG_INTEL_BASE		CFG_FLASH1_BASE + 0xf00000
#define CFG_FLASH_BASE		CFG_AMD_BASE
#else
#define CFG_INTEL_BASE		CFG_FLASH0_BASE + 0xf00000
#define CFG_AMD_BASE		CFG_FLASH1_BASE
#define CFG_FLASH_BASE		CFG_INTEL_BASE
#endif

#define CFG_CPLD_BASE		(CFG_CS2_BASE << 16)
#define CFG_FPGA_BASE		(CFG_CS3_BASE << 16)


#define CFG_MAX_FLASH_BANKS	4	/* max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/
#define CFG_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CFG_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */
#define CFG_FLASH_PROTECTION		/* "Real" (hardware) sectors protection */

#define PHYS_AMD_SECT_SIZE	0x00010000 /*  64 KB sectors (x2) */
#define PHYS_INTEL_SECT_SIZE	0x00020000 /* 128 KB sectors (x2) */

#define CFG_FLASH_CHECKSUM
/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#if defined (CFG_AMD_BOOT)
#define CFG_ENV_ADDR		(CFG_FLASH0_BASE + CFG_CS0_MASK - PHYS_AMD_SECT_SIZE)
#define CFG_ENV_SIZE		PHYS_AMD_SECT_SIZE
#define CFG_ENV_SECT_SIZE	PHYS_AMD_SECT_SIZE
#define CFG_ENV1_ADDR		(CFG_FLASH1_BASE + CFG_CS1_MASK - PHYS_INTEL_SECT_SIZE)
#define CFG_ENV1_SIZE		PHYS_INTEL_SECT_SIZE
#define CFG_ENV1_SECT_SIZE	PHYS_INTEL_SECT_SIZE
#else
#define CFG_ENV_ADDR		(CFG_FLASH0_BASE + CFG_CS0_MASK - PHYS_INTEL_SECT_SIZE)
#define CFG_ENV_SIZE		PHYS_INTEL_SECT_SIZE
#define CFG_ENV_SECT_SIZE	PHYS_INTEL_SECT_SIZE
#define CFG_ENV1_ADDR		(CFG_FLASH1_BASE + CFG_CS1_MASK - PHYS_AMD_SECT_SIZE)
#define CFG_ENV1_SIZE		PHYS_AMD_SECT_SIZE
#define CFG_ENV1_SECT_SIZE	PHYS_AMD_SECT_SIZE
#endif

#define CONFIG_ENV_OVERWRITE	1

#if defined CFG_ENV_IS_IN_FLASH
#undef CFG_ENV_IS_IN_NVRAM
#undef CFG_ENV_IS_IN_EEPROM
#elif defined CFG_ENV_IS_IN_NVRAM
#undef CFG_ENV_IS_IN_FLASH
#undef CFG_ENV_IS_IN_EEPROM
#elif defined CFG_ENV_IS_IN_EEPROM
#undef CFG_ENV_IS_IN_NVRAM
#undef CFG_ENV_IS_IN_FLASH
#endif

#ifndef CFG_JFFS2_FIRST_SECTOR
#define CFG_JFFS2_FIRST_SECTOR	0
#endif
#ifndef CFG_JFFS2_FIRST_BANK
#define CFG_JFFS2_FIRST_BANK	0
#endif
#ifndef CFG_JFFS2_NUM_BANKS
#define CFG_JFFS2_NUM_BANKS	1
#endif
#define CFG_JFFS2_LAST_BANK (CFG_JFFS2_FIRST_BANK + CFG_JFFS2_NUM_BANKS - 1)

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000
#define CFG_SRAM_BASE		(CFG_MBAR + 0x20000)
#define CFG_SRAM_SIZE		0x8000

/* Use SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	(CFG_MBAR + 0x20000)
#define CFG_INIT_RAM_END	0x8000	/* End of used area in DPRAM */

#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE	TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(256 << 10) /* Reserve 256 kB for Monitor   */
#define CFG_MALLOC_LEN		(128 << 10) /* Reserve 128 kB for malloc()  */
#define CFG_BOOTMAPSZ		(8 << 20)   /* Initial Memory map for Linux */

/* SDRAM configuration */
#define CFG_SDRAM_TOTAL_BANKS		2
#define CFG_SDRAM_SPD_I2C_ADDR		0x51		/* 7bit */
#define CFG_SDRAM_SPD_SIZE		0x40
#define CFG_SDRAM_CAS_LATENCY		4		/* (CL=2)x2 */

/* SDRAM drive strength register */
#define CFG_SDRAM_DRIVE_STRENGTH	((DRIVE_STRENGTH_LOW  << SDRAMDS_SBE_SHIFT) | \
					 (DRIVE_STRENGTH_HIGH << SDRAMDS_SBC_SHIFT) | \
					 (DRIVE_STRENGTH_LOW  << SDRAMDS_SBA_SHIFT) | \
					 (DRIVE_STRENGTH_OFF  << SDRAMDS_SBS_SHIFT) | \
					 (DRIVE_STRENGTH_LOW  << SDRAMDS_SBD_SHIFT))

/*
 * Ethernet configuration
 */
#define CONFIG_MPC8220_FEC	1
#define CONFIG_FEC_10MBIT	1 /* Workaround for FEC 100Mbit problem */
#define CONFIG_PHY_ADDR		0x18


/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			    /* undef to save memory	*/
#define CFG_PROMPT		"=> "	    /* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	    /* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256	    /* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16	    /* max number of command args   */
#define CFG_BARGSIZE		CFG_CBSIZE  /* Boot Argument Buffer Size    */

#define CFG_MEMTEST_START	0x00100000  /* memtest works on */
#define CFG_MEMTEST_END		0x00f00000  /* 1 ... 15 MB in DRAM  */

#define CFG_LOAD_ADDR		0x100000    /* default load address */

#define CFG_HZ			1000	    /* decrementer freq: 1 ms ticks */

#define CFG_CACHELINE_SIZE	32	/* For MPC8220 CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5   /* log base 2 of the above value */
#endif

/*
 * Various low-level settings
 */
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE

#endif /* __CONFIG_H */

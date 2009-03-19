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
#define CONFIG_ALASKA8220	1	/* ... on Alaska board	*/

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* Input clock running at 30Mhz, read Hid1 for the CPU multiplier to
   determine the CPU speed. */
#define CONFIG_SYS_MPC8220_CLKIN	30000000/* ... running at 30MHz */
#define CONFIG_SYS_MPC8220_SYSPLL_VCO_MULTIPLIER 16 /* VCO multiplier can't be read from any register */

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
#   define CONFIG_SYS_NS16550_SERIAL
#   define CONFIG_SYS_NS16550
#   define CONFIG_SYS_NS16550_REG_SIZE 1
#   define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CPLD_BASE + 0x1008)
#   define CONFIG_SYS_NS16550_CLK	18432000
#endif

#define CONFIG_BAUDRATE		115200	    /* ... at 115200 bps */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_TIMESTAMP			/* Print image info with timestamp */


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
#define CONFIG_HOSTNAME		Alaska
#define CONFIG_OVERWRITE_ETHADDR_ONCE


/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1
#define CONFIG_SYS_I2C_MODULE		1

#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x52	/* 1011000xb */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	70
/*
#define CONFIG_ENV_IS_IN_EEPROM	1
#define CONFIG_ENV_OFFSET		0
#define CONFIG_ENV_SIZE		256
*/

/* If CONFIG_SYS_AMD_BOOT is defined, the the system will boot from AMD.
   else undefined it will boot from Intel Strata flash */
#define CONFIG_SYS_AMD_BOOT		1

/*
 * Flexbus Chipselect configuration
 */
#if defined (CONFIG_SYS_AMD_BOOT)
#define CONFIG_SYS_CS0_BASE		0xfff0
#define CONFIG_SYS_CS0_MASK		0x00080000  /* 512 KB */
#define CONFIG_SYS_CS0_CTRL		0x003f0d40

#define CONFIG_SYS_CS1_BASE		0xfe00
#define CONFIG_SYS_CS1_MASK		0x01000000  /* 16 MB */
#define CONFIG_SYS_CS1_CTRL		0x003f1540
#else
#define CONFIG_SYS_CS0_BASE		0xff00
#define CONFIG_SYS_CS0_MASK		0x01000000  /* 16 MB */
#define CONFIG_SYS_CS0_CTRL		0x003f1540

#define CONFIG_SYS_CS1_BASE		0xfe08
#define CONFIG_SYS_CS1_MASK		0x00080000  /* 512 KB */
#define CONFIG_SYS_CS1_CTRL		0x003f0d40
#endif

#define CONFIG_SYS_CS2_BASE		0xf100
#define CONFIG_SYS_CS2_MASK		0x00040000
#define CONFIG_SYS_CS2_CTRL		0x003f1140

#define CONFIG_SYS_CS3_BASE		0xf200
#define CONFIG_SYS_CS3_MASK		0x00040000
#define CONFIG_SYS_CS3_CTRL		0x003f1100


#define CONFIG_SYS_FLASH0_BASE		(CONFIG_SYS_CS0_BASE << 16)
#define CONFIG_SYS_FLASH1_BASE		(CONFIG_SYS_CS1_BASE << 16)

#if defined (CONFIG_SYS_AMD_BOOT)
#define CONFIG_SYS_AMD_BASE		CONFIG_SYS_FLASH0_BASE
#define CONFIG_SYS_INTEL_BASE		CONFIG_SYS_FLASH1_BASE + 0xf00000
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_AMD_BASE
#else
#define CONFIG_SYS_INTEL_BASE		CONFIG_SYS_FLASH0_BASE + 0xf00000
#define CONFIG_SYS_AMD_BASE		CONFIG_SYS_FLASH1_BASE
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_INTEL_BASE
#endif

#define CONFIG_SYS_CPLD_BASE		(CONFIG_SYS_CS2_BASE << 16)
#define CONFIG_SYS_FPGA_BASE		(CONFIG_SYS_CS3_BASE << 16)


#define CONFIG_SYS_MAX_FLASH_BANKS	4	/* max num of memory banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/
#define CONFIG_SYS_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */
#define CONFIG_SYS_FLASH_PROTECTION		/* "Real" (hardware) sectors protection */

#define PHYS_AMD_SECT_SIZE	0x00010000 /*  64 KB sectors (x2) */
#define PHYS_INTEL_SECT_SIZE	0x00020000 /* 128 KB sectors (x2) */

#define CONFIG_SYS_FLASH_CHECKSUM
/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#if defined (CONFIG_SYS_AMD_BOOT)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH0_BASE + CONFIG_SYS_CS0_MASK - PHYS_AMD_SECT_SIZE)
#define CONFIG_ENV_SIZE		PHYS_AMD_SECT_SIZE
#define CONFIG_ENV_SECT_SIZE	PHYS_AMD_SECT_SIZE
#define CONFIG_ENV1_ADDR		(CONFIG_SYS_FLASH1_BASE + CONFIG_SYS_CS1_MASK - PHYS_INTEL_SECT_SIZE)
#define CONFIG_ENV1_SIZE		PHYS_INTEL_SECT_SIZE
#define CONFIG_ENV1_SECT_SIZE	PHYS_INTEL_SECT_SIZE
#else
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH0_BASE + CONFIG_SYS_CS0_MASK - PHYS_INTEL_SECT_SIZE)
#define CONFIG_ENV_SIZE		PHYS_INTEL_SECT_SIZE
#define CONFIG_ENV_SECT_SIZE	PHYS_INTEL_SECT_SIZE
#define CONFIG_ENV1_ADDR		(CONFIG_SYS_FLASH1_BASE + CONFIG_SYS_CS1_MASK - PHYS_AMD_SECT_SIZE)
#define CONFIG_ENV1_SIZE		PHYS_AMD_SECT_SIZE
#define CONFIG_ENV1_SECT_SIZE	PHYS_AMD_SECT_SIZE
#endif

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
#define CONFIG_SYS_INIT_RAM_END	0x8000	/* End of used area in DPRAM */

#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(256 << 10) /* Reserve 256 kB for Monitor   */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10) /* Reserve 128 kB for malloc()  */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)   /* Initial Memory map for Linux */

/* SDRAM configuration */
#define CONFIG_SYS_SDRAM_TOTAL_BANKS		2
#define CONFIG_SYS_SDRAM_SPD_I2C_ADDR		0x51		/* 7bit */
#define CONFIG_SYS_SDRAM_SPD_SIZE		0x40
#define CONFIG_SYS_SDRAM_CAS_LATENCY		4		/* (CL=2)x2 */

/* SDRAM drive strength register */
#define CONFIG_SYS_SDRAM_DRIVE_STRENGTH	((DRIVE_STRENGTH_LOW  << SDRAMDS_SBE_SHIFT) | \
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
#  define CONFIG_SYS_CACHELINE_SHIFT	5   /* log base 2 of the above value */
#endif

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

/*
 * JFFS2 partitions
 */

/* No command line, one static partition */
/*
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0x00400000
#define CONFIG_JFFS2_PART_OFFSET	0x00000000
*/

/* mtdparts command line support */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor0=alaska-0"
#define MTDPARTS_DEFAULT	"mtdparts=alaska-0:4m(user)"
*/

#endif /* __CONFIG_H */

/*
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * (C) Copyright 2003-2005
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

#define	CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU		*/
#define	CONFIG_MPC5200		1	/* (more precisely an MPC5200 CPU)	*/
#define	CONFIG_MUCMC52		1	/* MUCMC52 board			*/

#define	CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz		*/

#define	BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define	BOOTFLAG_WARM		0x02	/* Software reboot			*/

#define	CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs			*/
#if (CONFIG_COMMANDS & CONFIG_SYS_CMD_KGDB)
#  define	CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

#define	CONFIG_BOARD_EARLY_INIT_R

#define	CONFIG_LAST_STAGE_INIT

#define	CONFIG_HIGH_BATS	1	/* High BATs supported			*/
/*
 * Serial console configuration
 */
#define	CONFIG_PSC_CONSOLE	1	/* console is on PSC1	*/
#define	CONFIG_BAUDRATE		38400	/* ... at 38400 bps	*/
#define	CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/* Partitions */
#define	CONFIG_DOS_PARTITION

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define	CONFIG_CMD_DATE
#define	CONFIG_CMD_DISPLAY
#define	CONFIG_CMD_DHCP
#define	CONFIG_CMD_EEPROM
#define	CONFIG_CMD_FAT
#define	CONFIG_CMD_I2C
#define	CONFIG_CMD_DTT
#define	CONFIG_CMD_IDE
#define	CONFIG_CMD_MII
#define	CONFIG_CMD_NFS
#define	CONFIG_CMD_PCI
#define	CONFIG_CMD_PING
#define	CONFIG_CMD_SNTP

#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */

#if (TEXT_BASE == 0xFFF00000) /* Boot low */
#   define	CONFIG_SYS_LOWBOOT		1
#endif

/*
 * Autobooting
 */
#define	CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define	CONFIG_PREBOOT	"echo;" \
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip;bootm\0"	\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	""

#define	CONFIG_BOOTCOMMAND	"run net_nfs"

#define	CONFIG_MISC_INIT_R	1

/*
 * IPB Bus clocking configuration.
 */
#undef	CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * I2C configuration
 */
#define	CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define	CONFIG_SYS_I2C_MODULE		2	/* Select I2C module #1 or #2 */

#define	CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define	CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define	CONFIG_SYS_I2C_EEPROM_ADDR		0x58
#define	CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define	CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define	CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/*
 * RTC configuration
 */
#define	CONFIG_RTC_PCF8563
#define	CONFIG_SYS_I2C_RTC_ADDR		0x51

/* I2C SYSMON (LM75) */
#define	CONFIG_DTT_LM81			1	/* ON Semi's LM75		*/
#define	CONFIG_DTT_SENSORS		{0}	/* Sensor addresses		*/
#define	CONFIG_SYS_DTT_MAX_TEMP		70
#define	CONFIG_SYS_DTT_LOW_TEMP		-30
#define	CONFIG_SYS_DTT_HYSTERESIS		3

/*
 * Flash configuration
 */
#define	CONFIG_SYS_FLASH_BASE		0xFF800000

#define	CONFIG_SYS_FLASH_SIZE		0x00800000 /* 8 MByte */
#define	CONFIG_SYS_MAX_FLASH_SECT	67	/* max num of sects on one chip */

#define	CONFIG_ENV_ADDR		(TEXT_BASE+0x40000) /* second sector */
#define	CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of flash banks
					   (= chip selects) */
#define	CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)	*/
#define	CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

#define	CONFIG_FLASH_CFI_DRIVER
#define	CONFIG_SYS_FLASH_CFI
#define	CONFIG_SYS_FLASH_EMPTY_INFO
#define	CONFIG_SYS_FLASH_CFI_AMD_RESET

/*
 * Environment settings
 */
#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_SIZE		0x4000
#define	CONFIG_ENV_SECT_SIZE	0x20000
#define	CONFIG_ENV_OFFSET_REDUND   (CONFIG_ENV_OFFSET+CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE_REDUND     (CONFIG_ENV_SIZE)

/*
 * Memory map
 */
#define	CONFIG_SYS_MBAR		0xF0000000
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define	CONFIG_SYS_DEFAULT_MBAR	0x80000000
#define	CONFIG_SYS_DISPLAY_BASE	0x80600000
#define	CONFIG_SYS_STATUS1_BASE	0x80600200
#define	CONFIG_SYS_STATUS2_BASE	0x80600300
#define	CONFIG_SYS_PMI_UNI_BASE	0x80800000
#define	CONFIG_SYS_PMI_BROAD_BASE	0x80810000

/* Settings for XLB = 132 MHz */
#define	SDRAM_DDR	 1
#define	SDRAM_MODE      0x018D0000
#define	SDRAM_EMODE     0x40090000
#define	SDRAM_CONTROL   0x714f0f00
#define	SDRAM_CONFIG1   0x73722930
#define	SDRAM_CONFIG2   0x47770000
#define	SDRAM_TAPDELAY  0x10000000

/* Use ON-Chip SRAM until RAM will be available */
#define	CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
#define	CONFIG_SYS_INIT_RAM_END	MPC5XXX_SRAM_POST_SIZE
#else
#define	CONFIG_SYS_INIT_RAM_END	MPC5XXX_SRAM_SIZE
#endif

#define	CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define	CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define	CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define	CONFIG_SYS_RAMBOOT	1
#endif

#define	CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define	CONFIG_SYS_MALLOC_LEN		(512 << 10)	/* Reserve 128 kB for malloc()	*/
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define	CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
#define	CONFIG_PHY_ADDR		0x00
#define	CONFIG_MII		1		/* MII PHY management		*/

/*
 * GPIO configuration
 */
#define	CONFIG_SYS_GPS_PORT_CONFIG	0x8D550644

/*use  Hardware WDT */
#define CONFIG_HW_WATCHDOG

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define	CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */
#if (CONFIG_COMMANDS & CONFIG_SYS_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define	CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16	/* max number of command args	*/
#define	CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

/* Enable an alternate, more extensive memory test */
#define	CONFIG_SYS_ALT_MEMTEST

#define	CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define	CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define	CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Enable loopw commando. This has only affect, if CONFIG_SYS_CMD_MEM is defined,
 * which is normally part of the default commands (CFV_CMD_DFL)
 */
#define	CONFIG_LOOPW

/*
 * Various low-level settings
 */
#if defined(CONFIG_MPC5200)
#define	CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define	CONFIG_SYS_HID0_FINAL		HID0_ICE
#else
#define	CONFIG_SYS_HID0_INIT		0
#define	CONFIG_SYS_HID0_FINAL		0
#endif

#define	CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define	CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define	CONFIG_SYS_BOOTCS_CFG		0x0004FB00
#define	CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define	CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

/* 8Mbit SRAM @0x80100000 */
#define	CONFIG_SYS_CS1_START		0x80100000
#define	CONFIG_SYS_CS1_SIZE		0x00100000
#define	CONFIG_SYS_CS1_CFG		0x00019B00

/* FRAM 32Kbyte @0x80700000 */
#define	CONFIG_SYS_CS2_START		0x80700000
#define	CONFIG_SYS_CS2_SIZE		0x00008000
#define	CONFIG_SYS_CS2_CFG		0x00019800

/* Display H1, Status Inputs, EPLD @0x80600000 */
#define	CONFIG_SYS_CS3_START		0x80600000
#define	CONFIG_SYS_CS3_SIZE		0x00100000
#define	CONFIG_SYS_CS3_CFG		0x00019800

/* PMI Unicast 32Kbyte @0x80800000 */
#define	CONFIG_SYS_CS6_START		CONFIG_SYS_PMI_UNI_BASE
#define	CONFIG_SYS_CS6_SIZE		0x00008000
#define	CONFIG_SYS_CS6_CFG		0xFFFFF930

/* PMI Broadcast 32Kbyte @0x80810000 */
#define	CONFIG_SYS_CS7_START		CONFIG_SYS_PMI_BROAD_BASE
#define	CONFIG_SYS_CS7_SIZE		0x00008000
#define	CONFIG_SYS_CS7_CFG		0xFF00F930

#define	CONFIG_SYS_CS_BURST		0x00000000
#define	CONFIG_SYS_CS_DEADCYCLE	0x33333333

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef	CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define	CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define	CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 2 drives per IDE bus	*/

#define	CONFIG_IDE_PREINIT	1

#define	CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define	CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O			*/
#define	CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses	*/
#define	CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)

/* Offset for alternate registers	*/
#define	CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)

/* Interval between registers           */
#define	CONFIG_SYS_ATA_STRIDE          4

#define	CONFIG_ATAPI            1

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define	CONFIG_PCI		1
#define	CONFIG_PCI_PNP		1
#define	CONFIG_PCI_SCAN_SHOW	1
#define	CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define	CONFIG_PCI_MEM_BUS	0x40000000
#define	CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define	CONFIG_PCI_MEM_SIZE	0x10000000

#define	CONFIG_PCI_IO_BUS	0x50000000
#define	CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define	CONFIG_PCI_IO_SIZE	0x01000000

#define	CONFIG_SYS_ISA_IO		CONFIG_PCI_IO_BUS

/*---------------------------------------------------------------------*/
/* Display addresses						       */
/*---------------------------------------------------------------------*/

#define	CONFIG_SYS_DISP_CHR_RAM	(CONFIG_SYS_DISPLAY_BASE + 0x38)
#define	CONFIG_SYS_DISP_CWORD		(CONFIG_SYS_DISPLAY_BASE + 0x30)

#endif /* __CONFIG_H */

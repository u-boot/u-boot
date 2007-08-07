/*
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

#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU		*/
#define CONFIG_MPC5200		1	/* (more precisely an MPC5200 CPU)	*/
#define CONFIG_HMI1001		1	/* HMI1001 board			*/

#define CFG_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz		*/

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#define CONFIG_BOARD_EARLY_INIT_R

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1	*/
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps	*/
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/* Partitions */
#define CONFIG_DOS_PARTITION


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

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DISPLAY
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SNTP


#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */

#if (TEXT_BASE == 0xFFF00000) /* Boot low */
#   define CFG_LOWBOOT		1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
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

#define CONFIG_BOOTCOMMAND	"run net_nfs"

#define CONFIG_MISC_INIT_R	1

/*
 * IPB Bus clocking configuration.
 */
#undef CFG_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_MODULE		2	/* Select I2C module #1 or #2 */

#define CFG_I2C_SPEED		100000 /* 100 kHz */
#define CFG_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CFG_I2C_EEPROM_ADDR		0x58
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_BITS	4
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10

/*
 * RTC configuration
 */
#define CONFIG_RTC_PCF8563
#define CFG_I2C_RTC_ADDR		0x51

/*
 * Flash configuration
 */
#define CFG_FLASH_BASE		0xFF800000

#define CFG_FLASH_SIZE		0x00800000 /* 8 MByte */
#define CFG_MAX_FLASH_SECT	67	/* max num of sects on one chip */

#define CFG_ENV_ADDR		(TEXT_BASE+0x40000) /* second sector */
#define CFG_MAX_FLASH_BANKS	1	/* max num of flash banks
					   (= chip selects) */
#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_CFI_AMD_RESET

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x4000
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_OFFSET_REDUND   (CFG_ENV_OFFSET+CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND     (CFG_ENV_SIZE)

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000
#define CFG_DISPLAY_BASE	0x80600000
#define CFG_STATUS1_BASE	0x80600200
#define CFG_STATUS2_BASE	0x80600300

/* Settings for XLB = 132 MHz */
#define SDRAM_DDR	 1
#define SDRAM_MODE      0x018D0000
#define SDRAM_EMODE     0x40090000
#define SDRAM_CONTROL   0x714f0f00
#define SDRAM_CONFIG1   0x73722930
#define SDRAM_CONFIG2   0x47770000
#define SDRAM_TAPDELAY  0x10000000

/* Use ON-Chip SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_POST_SIZE
#else
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE
#endif


#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CFG_MALLOC_LEN		(512 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_PHY_ADDR		0x00
#define CONFIG_MII		1		/* MII PHY management		*/

/*
 * GPIO configuration
 */
#define CFG_GPS_PORT_CONFIG	0x01051004

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/* Enable an alternate, more extensive memory test */
#define CFG_ALT_MEMTEST

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Enable loopw command.
 */
#define CONFIG_LOOPW

/*
 * Various low-level settings
 */
#if defined(CONFIG_MPC5200)
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE
#else
#define CFG_HID0_INIT		0
#define CFG_HID0_FINAL		0
#endif

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x0004FB00
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

/* 8Mbit SRAM @0x80100000 */
#define CFG_CS1_START		0x80100000
#define CFG_CS1_SIZE		0x00100000
#define CFG_CS1_CFG		0x19B00

/* FRAM 32Kbyte @0x80700000 */
#define CFG_CS2_START		0x80700000
#define CFG_CS2_SIZE		0x00008000
#define CFG_CS2_CFG		0x19800

/* Display H1, Status Inputs, EPLD @0x80600000 */
#define CFG_CS3_START		0x80600000
#define CFG_CS3_SIZE		0x00100000
#define CFG_CS3_CFG		0x00019800

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	2	/* max. 2 drives per IDE bus	*/

#define CONFIG_IDE_PREINIT	1

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	(CFG_ATA_DATA_OFFSET)

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	(0x005C)

/* Interval between registers                                                */
#define CFG_ATA_STRIDE          4

#define CONFIG_ATAPI            1

#define CONFIG_VIDEO_SMI_LYNXEM
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_LOGO

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
#define CONFIG_PCI_SCAN_SHOW	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CFG_ISA_IO		CONFIG_PCI_IO_BUS

/*---------------------------------------------------------------------*/
/* Display addresses						       */
/*---------------------------------------------------------------------*/

#define CFG_DISP_CHR_RAM	(CFG_DISPLAY_BASE + 0x38)
#define CFG_DISP_CWORD		(CFG_DISPLAY_BASE + 0x30)

#endif /* __CONFIG_H */

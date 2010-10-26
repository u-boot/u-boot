/*
 * (C) Copyright 2003-2004
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


/*************************************************************************
 * (c) 2005 esd gmbh Hannover
 *
 *
 * from IceCube.h file
 * by Reinhard Arlt reinhard.arlt@esd-electronics.com
 *
 *************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC5200		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_ICECUBE		1	/* ... on IceCube board */
#define CONFIG_MECP5200		1	/* ... on MECP5200  board */
#define CONFIG_MPC5200_DDR      1       /* ... use DDR RAM      */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFF00000
#endif

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#if 0 /* test-only */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#else
#define CONFIG_BAUDRATE		9600	/* ... at 115200 bps */
#endif
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_MII
#if 0 /* test-only !!! */
#define CONFIG_NET_MULTI	1
#define CONFIG_EEPRO100		1
#define CONFIG_SYS_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */
#define CONFIG_NS8382X		1
#endif

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/* USB */
#if 0
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE
#endif


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

#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_BSP
#define CONFIG_CMD_ELF


#if (CONFIG_SYS_TEXT_BASE == 0xFF000000)		/* Boot low with 16 MB Flash */
#   define CONFIG_SYS_LOWBOOT		1
#   define CONFIG_SYS_LOWBOOT16	1
#endif
#if (CONFIG_SYS_TEXT_BASE == 0xFF800000)		/* Boot low with  8 MB Flash */
#   define CONFIG_SYS_LOWBOOT		1
#   define CONFIG_SYS_LOWBOOT08	1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	3	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Welcome to CBX-CPU5200 (mecp5200);" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=eth0\0" \
	"flash_vxworks0=run ata_vxworks_args;setenv loadaddr ff000000;bootvx\0" \
	"flash_vxworks1=run ata_vxworks_args;setenv loadaddr ff200000:bootvx\0" \
	"net_vxworks=tftp $(loadaddr) $(image);run vxworks_args;bootvx\0" \
	"vxworks_args=setenv bootargs fec(0,0)$(host):$(image) h=$(serverip) e=$(ipaddr) g=$(gatewayip) u=$(user) $(pass) tn=$(target) s=$(script)\0" \
	"ata_vxworks_args=setenv bootargs /ata0/vxWorks h=$(serverip) e=$(ipaddr) g=$(gatewayip) u=$(user) $(pass) tn=$(target) s=$(script) o=fec0 \0" \
	"loadaddr=01000000\0" \
	"serverip=192.168.2.99\0" \
	"gatewayip=10.0.0.79\0" \
	"user=mu\0" \
	"target=mecp5200.esd\0" \
	"script=mecp5200.bat\0" \
	"image=/tftpboot/vxWorks_mecp5200\0" \
	"ipaddr=10.0.13.196\0" \
	"netmask=255.255.0.0\0" \
	""

#define CONFIG_BOOTCOMMAND	"run flash_vxworks0"

/*
 * IPB Bus clocking configuration.
 */
#undef CONFIG_SYS_IPBSPEED_133			/* define for 133MHz speed */
/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE		2	/* Select I2C module #1 or #2 */

#define CONFIG_SYS_I2C_SPEED		86000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20
#define CONFIG_SYS_I2C_MULTI_EEPROMS		1
/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_BASE		0xFFC00000
#define CONFIG_SYS_FLASH_SIZE		0x00400000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x003E0000)
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks      */
#define CONFIG_SYS_MAX_FLASH_SECT	512

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

/*
 * Environment settings
 */
#if 1 /* test-only */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_OVERWRITE	1
#else
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET		0x0000	/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x0400	/* 8192 bytes may be used for env vars*/
				   /* total size of a CAT24WC32 is 8192 bytes */
#define CONFIG_ENV_OVERWRITE	1
#endif

#define CONFIG_FLASH_CFI_DRIVER	1	   /* Flash is CFI conformant		*/
#define CONFIG_SYS_FLASH_CFI		1	   /* Flash is CFI conformant		*/
#define CONFIG_SYS_FLASH_PROTECTION	1	   /* use hardware protection		*/
#if 0
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1       /* use buffered writes (20x faster)  */
#endif
#define CONFIG_SYS_FLASH_INCREMENT	0x00400000 /* size of  flash bank		*/
#define CONFIG_SYS_FLASH_BANKS_LIST  { CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_FLASH_EMPTY_INFO	1	   /* show if bank is empty		*/


/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_SIZE	MPC5XXX_SRAM_SIZE	/* Size of used area in DPRAM */


#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
/*
 * Define CONFIG_MPC5xxx_FEC_MII10 to force FEC at 10Mb
 */
/* #define CONFIG_MPC5xxx_FEC_MII10 */
#define CONFIG_PHY_ADDR		0x00
#define CONFIG_UDP_CHECKSUM     1


/*
 * GPIO configuration
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x01052444

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ			1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_VXWORKS_MAC_PTR	0x00000000	/* Pass Ethernet MAC to VxWorks */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x00085d00

#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

#define CONFIG_SYS_CS1_START		0xfd000000
#define CONFIG_SYS_CS1_SIZE		0x00010000
#define CONFIG_SYS_CS1_CFG		0x10101410

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x33333333

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00001000

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define	CONFIG_IDE_RESET		/* reset for ide supported	*/
#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)

/* Interval between registers		*/
#define CONFIG_SYS_ATA_STRIDE		4

#endif /* __CONFIG_H */

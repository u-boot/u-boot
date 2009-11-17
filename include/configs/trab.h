/*
 * (C) Copyright 2002-2005
 * Gary Jennejohn <garyj@denx.de>
 *
 * Configuation settings for the TRAB board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Default configuration is with 8 MB Flash, 32 MB RAM
 */
#if (!defined(CONFIG_FLASH_8MB)) && (!defined(CONFIG_FLASH_16MB))
# define CONFIG_FLASH_8MB	/*  8 MB Flash	*/
#endif
#if (!defined(CONFIG_RAM_16MB)) && (!defined(CONFIG_RAM_32MB))
# define CONFIG_RAM_32MB	/* 32 MB SDRAM	*/
#endif

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T	1	/* This is an arm920t CPU	*/
#define CONFIG_S3C24X0	1	/* in a SAMSUNG S3C24x0-type SoC	*/
#define CONFIG_S3C2400	1	/* specifically a SAMSUNG S3C2400 SoC	*/
#define CONFIG_TRAB	1	/* on a TRAB Board		*/
#undef CONFIG_TRAB_50MHZ	/* run the CPU at 50 MHz	*/

/* automatic software updates (see board/trab/auto_update.c) */
#define CONFIG_AUTO_UPDATE	1

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000 /* TRAB has 12 MHz input clock */

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1

#define CONFIG_SYS_DEVICE_NULLDEV	 1	/* enble null device		*/
#define CONFIG_SILENT_CONSOLE	 1	/* enable silent startup	*/

#define CONFIG_VERSION_VARIABLE	1       /* include version env variable */

/***********************************************************
 * I2C stuff:
 * the TRAB is equipped with an ATMEL 24C04 EEPROM at
 * address 0x54 with 8bit addressing
 ***********************************************************/
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed */
#define CONFIG_SYS_I2C_SLAVE		0x7F	/* I2C slave addr */

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x54	/* EEPROM address */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1	/* 1 address byte */

#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW 0x01
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3	/* 8 bytes page write mode on 24C04 */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

/* USB stuff */
#define CONFIG_USB_OHCI_NEW	1
#define CONFIG_USB_STORAGE	1
#define CONFIG_DOS_PARTITION	1

#undef CONFIG_SYS_USB_OHCI_BOARD_INIT
#define CONFIG_SYS_USB_OHCI_CPU_INIT	1

#define CONFIG_SYS_USB_OHCI_REGS_BASE	0x14200000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"s3c2400"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_NET_MULTI
#define CONFIG_CS8900		/* we have a CS8900 on-board */
#define CONFIG_CS8900_BASE	0x07000300 /* agrees with WIN CE PA */
#define CONFIG_CS8900_BUS16	/* the Linux driver does accesses as shorts */

#define CONFIG_DRIVER_S3C24X0_I2C 1	/* we use the buildin I2C controller */

#define CONFIG_VFD		1	/* VFD linear frame buffer driver */
#define VFD_TEST_LOGO		1	/* output a test logo to the VFDs */

/*
 * select serial console configuration
 */
#define CONFIG_S3C24X0_SERIAL
#define CONFIG_SERIAL1		1	/* we use SERIAL 1 on TRAB */

#define CONFIG_HWFLOW			/* include RTS/CTS flow control support	*/

#define CONFIG_MODEM_SUPPORT	1	/* enable modem initialization stuff */

#define	CONFIG_MODEM_KEY_MAGIC	"23"	/* hold down these keys to enable modem */

/*
 * The following enables modem debugging stuff. The dbg() and
 * 'char screen[1024]' are used for debug printfs. Unfortunately,
 * it is usable only from BDI
 */
#undef CONFIG_MODEM_SUPPORT_DEBUG

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

#define	CONFIG_TIMESTAMP	1	/* Print timestamp info for images */

/* Use s3c2400's RTC */
#define CONFIG_RTC_S3C24X0	1


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

#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_FAT
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_USB

#ifdef CONFIG_HWFLOW
    #define CONFIG_CMD_HWFLOW
#endif

#ifdef	CONFIG_VFD
    #define CONFIG_CMD_VFD
#endif

#ifdef CONFIG_DRIVER_S3C24X0_I2C
    #define CONFIG_CMD_EEPROM
    #define CONFIG_CMD_I2C
#endif

#ifndef USE_920T_MMU
    #undef CONFIG_CMD_CACHE
#endif


/* moved up */
#define CONFIG_SYS_HUSH_PARSER		1	/* use "hush" command parser	*/

#define CONFIG_BOOTDELAY	5
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* allow to break in always */
#define CONFIG_PREBOOT		"echo;echo *** booting ***;echo"
#define CONFIG_BOOTARGS		"console=ttyS0"
#define CONFIG_NETMASK		255.255.0.0
#define CONFIG_IPADDR		192.168.3.68
#define CONFIG_HOSTNAME		trab
#define CONFIG_SERVERIP		192.168.3.1
#define CONFIG_BOOTCOMMAND	"burn_in"

#ifndef CONFIG_FLASH_8MB	/* current config: 16 MB flash */
#ifdef CONFIG_SYS_HUSH_PARSER
#define	CONFIG_EXTRA_ENV_SETTINGS	\
	"nfs_args=setenv bootargs root=/dev/nfs rw " \
		"nfsroot=$serverip:$rootpath\0" \
	"rootpath=/opt/eldk/arm_920TDI\0" \
	"ram_args=setenv bootargs root=/dev/ram rw\0" \
	"add_net=setenv bootargs $bootargs ethaddr=$ethaddr " \
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname::off\0" \
	"add_misc=setenv bootargs $bootargs console=ttyS0 panic=1\0" \
	"u-boot=/tftpboot/TRAB/u-boot.bin\0" \
	"load=tftp C100000 ${u-boot}\0" \
	"update=protect off 0 5FFFF;era 0 5FFFF;" \
		"cp.b C100000 0 $filesize\0" \
	"loadfile=/tftpboot/TRAB/uImage\0" \
	"loadaddr=c400000\0" \
	"net_load=tftpboot $loadaddr $loadfile\0" \
	"net_nfs=run net_load nfs_args add_net add_misc;bootm\0" \
	"kernel_addr=00060000\0" \
	"flash_nfs=run nfs_args add_net add_misc;bootm $kernel_addr\0" \
	"mdm_init1=ATZ\0" \
	"mdm_init2=ATS0=1\0" \
	"mdm_flow_control=rts/cts\0"
#else /* !CONFIG_SYS_HUSH_PARSER */
#define	CONFIG_EXTRA_ENV_SETTINGS	\
	"nfs_args=setenv bootargs root=/dev/nfs rw " \
		"nfsroot=${serverip}:${rootpath}\0" \
	"rootpath=/opt/eldk/arm_920TDI\0" \
	"ram_args=setenv bootargs root=/dev/ram rw\0" \
	"add_net=setenv bootargs ${bootargs} ethaddr=${ethaddr} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off\0" \
	"add_misc=setenv bootargs ${bootargs} console=ttyS0 panic=1\0" \
	"u-boot=/tftpboot/TRAB/u-boot.bin\0" \
	"load=tftp C100000 ${u-boot}\0" \
	"update=protect off 0 5FFFF;era 0 5FFFF;" \
		"cp.b C100000 0 ${filesize}\0" \
	"loadfile=/tftpboot/TRAB/uImage\0" \
	"loadaddr=c400000\0" \
	"net_load=tftpboot ${loadaddr} ${loadfile}\0" \
	"net_nfs=run net_load nfs_args add_net add_misc;bootm\0" \
	"kernel_addr=000C0000\0" \
	"flash_nfs=run nfs_args add_net add_misc;bootm ${kernel_addr}\0" \
	"mdm_init1=ATZ\0" \
	"mdm_init2=ATS0=1\0" \
	"mdm_flow_control=rts/cts\0"
#endif	/* CONFIG_SYS_HUSH_PARSER */
#else	/* CONFIG_FLASH_8MB	 => 8 MB flash */
#ifdef CONFIG_SYS_HUSH_PARSER
#define	CONFIG_EXTRA_ENV_SETTINGS	\
	"nfs_args=setenv bootargs root=/dev/nfs rw " \
		"nfsroot=$serverip:$rootpath\0" \
	"rootpath=/opt/eldk/arm_920TDI\0" \
	"ram_args=setenv bootargs root=/dev/ram rw\0" \
	"add_net=setenv bootargs $bootargs ethaddr=$ethaddr " \
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname::off\0" \
	"add_misc=setenv bootargs $bootargs console=ttyS0 panic=1\0" \
	"u-boot=/tftpboot/TRAB/u-boot.bin\0" \
	"load=tftp C100000 ${u-boot}\0" \
	"update=protect off 0 3FFFF;era 0 3FFFF;" \
		"cp.b C100000 0 $filesize;" \
		"setenv filesize;saveenv\0" \
	"loadfile=/tftpboot/TRAB/uImage\0" \
	"loadaddr=C400000\0" \
	"net_load=tftpboot $loadaddr $loadfile\0" \
	"net_nfs=run net_load nfs_args add_net add_misc;bootm\0" \
	"kernel_addr=000C0000\0" \
	"flash_nfs=run nfs_args add_net add_misc;bootm $kernel_addr\0" \
	"mdm_init1=ATZ\0" \
	"mdm_init2=ATS0=1\0" \
	"mdm_flow_control=rts/cts\0"
#else /* !CONFIG_SYS_HUSH_PARSER */
#define	CONFIG_EXTRA_ENV_SETTINGS	\
	"nfs_args=setenv bootargs root=/dev/nfs rw " \
		"nfsroot=${serverip}:${rootpath}\0" \
	"rootpath=/opt/eldk/arm_920TDI\0" \
	"ram_args=setenv bootargs root=/dev/ram rw\0" \
	"add_net=setenv bootargs ${bootargs} ethaddr=${ethaddr} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off\0" \
	"add_misc=setenv bootargs ${bootargs} console=ttyS0 panic=1\0" \
	"u-boot=/tftpboot/TRAB/u-boot.bin\0" \
	"load=tftp C100000 ${u-boot}\0" \
	"update=protect off 0 3FFFF;era 0 3FFFF;" \
		"cp.b C100000 0 ${filesize};" \
		"setenv filesize;saveenv\0" \
	"loadfile=/tftpboot/TRAB/uImage\0" \
	"loadaddr=C400000\0" \
	"net_load=tftpboot ${loadaddr} ${loadfile}\0" \
	"net_nfs=run net_load nfs_args add_net add_misc;bootm\0" \
	"kernel_addr=000C0000\0" \
	"flash_nfs=run nfs_args add_net add_misc;bootm ${kernel_addr}\0" \
	"mdm_init1=ATZ\0" \
	"mdm_init2=ATS0=1\0" \
	"mdm_flow_control=rts/cts\0"
#endif /* CONFIG_SYS_HUSH_PARSER */
#endif	/* CONFIG_FLASH_8MB */

#if 1	/* feel free to disable for development */
#define	CONFIG_AUTOBOOT_KEYED		/* Enable password protection	*/
#define CONFIG_AUTOBOOT_PROMPT		\
	"\nEnter password - autoboot in %d sec...\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR	"R"	/* 1st "password"	*/
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"TRAB # "	/* Monitor Command Prompt	*/
#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0C000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0D000000	/* 16 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x0CF00000	/* default load address	*/

#define	CONFIG_SYS_HZ			1000

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_MISC_INIT_R		/* have misc_init_r() function	*/

/*-----------------------------------------------------------------------
 * burn-in test stuff.
 *
 * BURN_IN_CYCLE_DELAY defines the seconds to wait between each burn-in cycle
 * Because the burn-in test itself causes also an delay of about 4 seconds,
 * this time must be subtracted from the desired overall burn-in cycle time.
 */
#define BURN_IN_CYCLE_DELAY	296	/* seconds between burn-in cycles */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x0C000000	/* SDRAM Bank #1 */
#ifndef CONFIG_RAM_16MB
#define PHYS_SDRAM_1_SIZE	0x02000000	/* 32 MB */
#else
#define PHYS_SDRAM_1_SIZE	0x01000000	/* 16 MB */
#endif

#define CONFIG_SYS_FLASH_BASE		0x00000000	/* Flash Bank #1 */

/* The following #defines are needed to get flash environment right */
#define	CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)

/* Dynamic MTD partition support */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=0"

/* production flash layout */
#define MTDPARTS_DEFAULT	"mtdparts=0:16k(Firmware1)ro,"		\
						"16k(Env1),"		\
						"16k(Env2),"		\
						"336k(Firmware2)ro,"	\
						"896k(Kernel),"		\
						"5376k(Root-FS),"	\
						"1408k(JFFS2),"		\
						"-(VFD)"

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#ifndef CONFIG_FLASH_8MB
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max number of sectors on one chip */
#else
#define CONFIG_SYS_MAX_FLASH_SECT	71	/* max number of sectors on one chip */
#endif

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(15*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Write */

#define	CONFIG_ENV_IS_IN_FLASH	1

/* Address and size of Primary Environment Sector	*/
#ifndef CONFIG_FLASH_8MB
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x60000)
#define CONFIG_ENV_SIZE		0x4000
#define CONFIG_ENV_SECT_SIZE	0x20000
#else
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x4000)
#define CONFIG_ENV_SIZE		0x4000
#define CONFIG_ENV_SECT_SIZE	0x4000
#endif

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define	CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

/* Initial value of the on-board touch screen brightness */
#define CONFIG_SYS_BRIGHTNESS 0x20

#endif	/* __CONFIG_H */

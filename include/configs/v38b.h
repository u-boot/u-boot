/*
 * (C) Copyright 2003-2006 Wolfgang Denk, DENX Software Engineering,
 * wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this project.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC5xxx			1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5200			1	/* This is an MPC5200 CPU */
#define CONFIG_V38B			1	/* ...on V38B board */
#define CFG_MPC5XXX_CLKIN	33000000	/* ...running at 33.000000MHz */

#define CONFIG_RTC_PCF8563		1	/* has PCF8563 RTC */
#define CONFIG_MPC5200_DDR		1	/* has DDR SDRAM */

#undef CONFIG_HW_WATCHDOG			/* don't use watchdog */

#define CONFIG_NETCONSOLE		1

#define CONFIG_BOARD_EARLY_INIT_R	1	/* do board-specific init */
#define CONFIG_BOARD_EARLY_INIT_F	1	/* do board-specific init */

#define CFG_XLB_PIPELINING		1	/* gives better performance */

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM		0x02	/* Software reboot */

#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * DDR
 */
#define SDRAM_DDR		1	/* is DDR */
/* Settings for XLB = 132 MHz */
#define SDRAM_MODE		0x018D0000
#define SDRAM_EMODE		0x40090000
#define SDRAM_CONTROL		0x704f0f00
#define SDRAM_CONFIG1		0x73722930
#define SDRAM_CONFIG2		0x47770000
#define SDRAM_TAPDELAY		0x10000000

/*
 * PCI - no suport
 */
#undef CONFIG_PCI

/*
 * Partitions
 */
#define CONFIG_MAC_PARTITION	1
#define CONFIG_DOS_PARTITION	1

/*
 * USB
 */
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00001000

/*
 * Supported commands
 */
#define CONFIG_COMMANDS		(CONFIG_CMD_DFL	| \
				 CFG_CMD_FAT	| \
				 CFG_CMD_I2C	| \
				 CFG_CMD_IDE	| \
				 CFG_CMD_PING	| \
				 CFG_CMD_DHCP	| \
				 CFG_CMD_DIAG	| \
				 CFG_CMD_IRQ	| \
				 CFG_CMD_JFFS2	| \
				 CFG_CMD_MII	| \
				 CFG_CMD_SDRAM	| \
				 CFG_CMD_DATE	| \
				 CFG_CMD_USB	| \
				 CFG_CMD_FAT)

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Boot low with 16 MB Flash
 */
#define CFG_LOWBOOT		1
#define CFG_LOWBOOT16		1

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootcmd=run net_nfs\0"						\
	"bootdelay=3\0"							\
	"baudrate=115200\0"						\
	"preboot=echo;echo Type \"run flash_nfs\" to mount root "	\
		"filesystem over NFS; echo\0"				\
	"netdev=eth0\0"							\
	"ramargs=setenv bootargs root=/dev/ram rw wdt=off \0"		\
	"addip=setenv bootargs $(bootargs) "				\
		"ip=$(ipaddr):$(serverip):$(gatewayip):"		\
		"$(netmask):$(hostname):$(netdev):off panic=1\0"	\
	"flash_nfs=run nfsargs addip;bootm $(kernel_addr)\0"		\
	"flash_self=run ramargs addip;bootm $(kernel_addr) "		\
		"$(ramdisk_addr)\0"					\
	"net_nfs=tftp 200000 $(bootfile);run nfsargs addip;bootm\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=$(serverip):$(rootpath) wdt=off\0"		\
	"hostname=v38b\0"						\
	"ethact=FEC ETHERNET\0"						\
	"rootpath=/opt/eldk-3.1.1/ppc_6xx\0"				\
	"update=prot off ff000000 ff03ffff; era ff000000 ff03ffff; "	\
		"cp.b 200000 ff000000 $(filesize);"			\
		"prot on ff000000 ff03ffff\0"				\
	"load=tftp 200000 $(u-boot)\0"					\
	"netmask=255.255.0.0\0"						\
	"ipaddr=192.168.160.18\0"					\
	"serverip=192.168.1.1\0"					\
	"ethaddr=00:e0:ee:00:05:2e\0"					\
	"bootfile=/tftpboot/v38b/uImage\0"				\
	"u-boot=/tftpboot/v38b/u-boot.bin\0"				\
	""

#define CONFIG_BOOTCOMMAND	"run net_nfs"

#if defined(CONFIG_MPC5200)
/*
 * IPB Bus clocking configuration.
 */
#undef CFG_IPBCLK_EQUALS_XLBCLK			/* define for 133MHz speed */
#endif

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_MODULE		2	/* Select I2C module #1 or #2 */
#define CFG_I2C_SPEED		100000	/* 100 kHz */
#define CFG_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CFG_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * RTC configuration
 */
#define CFG_I2C_RTC_ADDR		0x51

/*
 * Flash configuration - use CFI driver
 */
#define CFG_FLASH_CFI		1		/* Flash is CFI conformant */
#define CFG_FLASH_CFI_DRIVER	1		/* Use the common driver */
#define CFG_FLASH_CFI_AMD_RESET	1
#define CFG_FLASH_BASE		0xFF000000
#define CFG_MAX_FLASH_BANKS	1		/* max num of flash banks */
#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }
#define CFG_FLASH_SIZE		0x01000000	/* 16 MiB */
#define CFG_MAX_FLASH_SECT	256		/* max num of sects on one chip */
#define CFG_FLASH_USE_BUFFER_WRITE	1	/* flash write speed-up */

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x00040000)
#define CFG_ENV_SIZE		0x10000
#define CFG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE	/* End of used area in DPRAM */

#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE	TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256kB for Monitor */
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128kB for malloc() */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Linux initial memory map */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_PHY_ADDR		0x00
#define CONFIG_MII		1

/*
 * GPIO configuration
 */
#define CFG_GPS_PORT_CONFIG	0x90001404

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM */

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Various low-level settings
 */
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x00047801
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333

#define CFG_RESET_ADDRESS	0xff000000

/*
 * IDE/ATA (supports IDE harddisk)
 */
#undef CONFIG_IDE_8xx_PCCARD		/* Don't use IDE with PC Card Adapter */
#undef CONFIG_IDE_8xx_DIRECT		/* Direct IDE not supported */
#undef CONFIG_IDE_LED			/* LED for ide not supported */

#define CONFIG_IDE_RESET		/* reset for ide supported */
#define CONFIG_IDE_PREINIT

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus */
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus */

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	MPC5XXX_ATA

#define CFG_ATA_DATA_OFFSET	(0x0060)	/* data I/O offset */

#define CFG_ATA_REG_OFFSET	(CFG_ATA_DATA_OFFSET)	/* normal register accesses offset */

#define CFG_ATA_ALT_OFFSET	(0x005C)	/* alternate registers offset */

#define CFG_ATA_STRIDE		4		/* Interval between registers */

/*
 * Status LED
 */
#define  CONFIG_STATUS_LED		/* Status LED enabled */
#define  CONFIG_BOARD_SPECIFIC_LED	/* version has board specific leds */

#define CFG_LED_BASE	MPC5XXX_GPT7_ENABLE	/* Timer 7 GPIO */
#ifndef __ASSEMBLY__
typedef unsigned int led_id_t;

#define __led_toggle(_msk) \
	do { \
		*((volatile long *) (CFG_LED_BASE)) ^= (_msk); \
	} while(0)

#define __led_set(_msk, _st) \
	do { \
		if ((_st)) \
			*((volatile long *) (CFG_LED_BASE)) &= ~(_msk); \
		else \
			*((volatile long *) (CFG_LED_BASE)) |= (_msk); \
	} while(0)

#define __led_init(_msk, st) \
	do { \
		*((volatile long *) (CFG_LED_BASE)) |= 0x34; \
	} while(0)
#endif /* __ASSEMBLY__ */

#endif /* __CONFIG_H */

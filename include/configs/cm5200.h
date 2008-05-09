/*
 * (C) Copyright 2003-2007
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
 */
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5200		1	/* (more precisely an MPC5200 CPU) */
#define CONFIG_CM5200		1	/* ... on CM5200 platform */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/*
 * Supported commands
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_USB

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		57600	/* ... at 57600 bps */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }
#define CONFIG_SILENT_CONSOLE	1	/* needed to silence i2c_init() */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_PHY_ADDR		0x00
#define CONFIG_ENV_OVERWRITE	1	/* allow overwriting of ethaddr */
/* use misc_init_r() to read ethaddr from I2C EEPROM (see CFG_I2C_EEPROM) */
#define CONFIG_MISC_INIT_R	1
#define CONFIG_MAC_OFFSET	0x35	/* MAC address offset in I2C EEPROM */

/*
 * POST support
 */
#define CONFIG_POST		(CFG_POST_MEMORY | CFG_POST_CPU | CFG_POST_I2C)
#define MPC5XXX_SRAM_POST_SIZE	(MPC5XXX_SRAM_SIZE - 4)
/* List of I2C addresses to be verified by POST */
#define I2C_ADDR_LIST		{ CFG_I2C_SLAVE, CFG_I2C_IO, CFG_I2C_EEPROM }

/* display image timestamps */
#define CONFIG_TIMESTAMP	1

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CONFIG_PREBOOT	"echo;" \
	"echo Type \"run net_nfs_fdt\" to mount root filesystem over NFS;" \
	"echo"
#undef CONFIG_BOOTARGS

/*
 * Default environment settings
 */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"netmask=255.255.0.0\0"						\
	"ipaddr=192.168.160.33\0"					\
	"serverip=192.168.1.1\0"					\
	"gatewayip=192.168.1.1\0"					\
	"console=ttyPSC0\0"						\
	"u-boot_addr=100000\0"						\
	"kernel_addr=200000\0"						\
	"kernel_addr_flash=fc0c0000\0"					\
	"fdt_addr=400000\0"						\
	"fdt_addr_flash=fc0a0000\0"					\
	"ramdisk_addr=500000\0"						\
	"rootpath=/opt/eldk-4.1/ppc_6xx\0"				\
	"u-boot=/tftpboot/cm5200/u-boot.bin\0"				\
	"bootfile_fdt=/tftpboot/cm5200/uImage\0"			\
	"fdt_file=/tftpboot/cm5200/cm5200.dtb\0"			\
	"load=tftp ${u-boot_addr} ${u-boot}\0"				\
	"update=prot off fc000000 +${filesize}; "			\
		"era fc000000 +${filesize}; "				\
		"cp.b ${u-boot_addr} fc000000 ${filesize}; "		\
		"prot on fc000000 +${filesize}\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"flashargs=setenv bootargs root=/dev/mtdblock5 rw\0"		\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addinit=setenv bootargs ${bootargs} init=/linuxrc\0"		\
	"addcons=setenv bootargs ${bootargs} "				\
		"console=${console},${baudrate}\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
		"${netmask}:${hostname}:${netdev}:off panic=1\0"	\
	"flash_flash=run flashargs addinit addip addcons;"		\
		"bootm ${kernel_addr_flash} - ${fdt_addr_flash}\0"	\
	"net_nfs_fdt=tftp ${kernel_addr} ${bootfile_fdt}; "		\
		"tftp ${fdt_addr} ${fdt_file}; run nfsargs addip "	\
		"addcons; bootm ${kernel_addr} - ${fdt_addr}\0"		\
	""
#define CONFIG_BOOTCOMMAND	"run flash_flash"

/*
 * Low level configuration
 */

/*
 * Clock configuration
 */
#define CFG_MPC5XXX_CLKIN	33000000	/* SYS_XTAL_IN = 33MHz */
#define CFG_IPBCLK_EQUALS_XLBCLK	1	/* IPB = 133MHz */

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000

#define CFG_LOWBOOT		1

/* Use ON-Chip SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_POST_SIZE
#else
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE
#endif

#define CFG_GBL_DATA_SIZE	128	/* size in bytes for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CONFIG_BOARD_TYPES	1	/* we use board_type */

#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(384 << 10)	/* 384 kB for Monitor */
#define CFG_MALLOC_LEN		(256 << 10)	/* 256 kB for malloc() */
#define CFG_BOOTMAPSZ		(8 << 20)	/* initial mem map for Linux */

/*
 * Flash configuration
 */
#define CFG_FLASH_CFI		1
#define CFG_FLASH_CFI_DRIVER	1
#define CFG_FLASH_BASE		0xfc000000
/* we need these despite using CFI */
#define CFG_MAX_FLASH_BANKS	1	/* max num of flash banks */
#define CFG_MAX_FLASH_SECT	256	/* max num of sectors on one chip */
#define CFG_FLASH_SIZE		0x02000000 /* 32 MiB */


#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT		1
#undef CFG_LOWBOOT
#endif


/*
 * Chip selects configuration
 */
/* Boot Chipselect */
#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x00087D31	/* for pci_clk = 33 MHz */
/* use board_early_init_r to enable flash write in CS_BOOT */
#define CONFIG_BOARD_EARLY_INIT_R

/* Flash memory addressing */
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

/* No burst, dead cycle = 1 for CS0 (Flash) */
#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x00000001

/*
 * SDRAM configuration
 * settings for k4s561632E-xx75, assuming XLB = 132 MHz
 */
#define SDRAM_MODE	0x00CD0000	/* CASL 3, burst length 8 */
#define SDRAM_CONTROL	0x514F0000
#define SDRAM_CONFIG1	0xE2333900
#define SDRAM_CONFIG2	0x8EE70000

/*
 * MTD configuration
 */
#define CONFIG_JFFS2_CMDLINE	1
#define MTDIDS_DEFAULT		"nor0=cm5200-0"
#define MTDPARTS_DEFAULT	"mtdparts=cm5200-0:"			\
					"384k(uboot),128k(env),"	\
					"128k(redund_env),128k(dtb),"	\
					"2m(kernel),27904k(rootfs),"	\
					"-(config)"

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_MODULE		2	/* Select I2C module #2 */
#define CFG_I2C_SPEED		40000	/* 40 kHz */
#define CFG_I2C_SLAVE		0x0
#define CFG_I2C_IO		0x38	/* PCA9554AD I2C I/O port address */
#define CFG_I2C_EEPROM		0x53	/* I2C EEPROM device address */

/*
 * RTC configuration
 */
#define CONFIG_RTC_MPC5200	1	/* use internal MPC5200 RTC */

/*
 * USB configuration
 */
#define CONFIG_USB_OHCI		1
#define CONFIG_USB_STORAGE	1
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00001000
/* Partitions (for USB) */
#define CONFIG_MAC_PARTITION	1
#define CONFIG_DOS_PARTITION	1
#define CONFIG_ISO_PARTITION	1

/*
 * Invoke our last_stage_init function - needed by fwupdate
 */
#define CONFIG_LAST_STAGE_INIT	1

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x10000
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + CFG_MONITOR_LEN)
/* Configuration of redundant environment */
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*
 * Pin multiplexing configuration
 */

/*
 * CS1/GPIO_WKUP_6: GPIO (default)
 * ALTs: CAN1 on I2C1, CAN2 on TIMER0/1
 * IRDA/PSC6: UART
 * Ether: Ethernet 100Mbit with MD
 * PCI_DIS: PCI controller disabled
 * USB: USB
 * PSC3: SPI with UART3
 * PSC2: UART
 * PSC1: UART
 */
#define CFG_GPS_PORT_CONFIG	0x10559C44

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		1	/* undef to save memory */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt */
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size */
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CFG_ALT_MEMTEST		1
#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x03f00000	/* 1 .. 63 MiB in SDRAM */

#define CONFIG_LOOPW		1

#define CFG_LOAD_ADDR		0x100000	/* default load address */
#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Various low-level settings
 */
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot */

#define CFG_XLB_PIPELINING	1	/* enable transaction pipeling */

/*
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#ifdef CONFIG_CMD_KGDB
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Flat Device Tree support
 */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc5200@f0000000/serial@2000"

#endif /* __CONFIG_H */

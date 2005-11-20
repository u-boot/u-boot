/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@freescale.com.
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
 * Check valid setting of revision define.
 * Total5100 and Total5200 Rev.1 are identical except for the processor.
 */
#if (CONFIG_TOTAL5200_REV!=1 && CONFIG_TOTAL5200_REV!=2)
#error CONFIG_TOTAL5200_REV must be 1 or 2
#endif

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_TOTAL5200	1	/* ... on Total5200 board */

#define CFG_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz */

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM		0x02	/* Software reboot	     */

#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	3	/* console is on PSC3 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Video console
 */
#if 1
#define CONFIG_VIDEO
#define CONFIG_VIDEO_SED13806
#define CONFIG_VIDEO_SED13806_16BPP

#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
/* #define CONFIG_VIDEO_BMP_LOGO */
#define CONFIG_CONSOLE_EXTRA_INFO
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_SPLASH_SCREEN

#define ADD_VIDEO_CMD	CFG_CMD_BMP
#else
#define ADD_VIDEO_CMD	0
#endif

#ifdef CONFIG_MPC5200	/* MGT5100 PCI is not supported yet. */
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

#define CONFIG_NET_MULTI	1
#define CONFIG_MII		1
#define CONFIG_EEPRO100		1
#define CFG_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */
#define CONFIG_NS8382X		1

#define ADD_PCI_CMD 		CFG_CMD_PCI

#else	/* MGT5100 */

#define CONFIG_MII		1
#define ADD_PCI_CMD		0  /* no CFG_CMD_PCI */

#endif

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/* USB */
#if 1
#define CONFIG_USB_OHCI
#define ADD_USB_CMD             CFG_CMD_USB | CFG_CMD_FAT
#define CONFIG_USB_STORAGE
#else
#define ADD_USB_CMD             0
#endif

/*
 * Supported commands
 */
#define CONFIG_COMMANDS		(CONFIG_CMD_DFL	| \
				 CFG_CMD_PING	| \
				 CFG_CMD_I2C	| \
				 CFG_CMD_EEPROM	| \
				 CFG_CMD_FAT	| \
				 CFG_CMD_IDE	| \
				 ADD_VIDEO_CMD  | \
				 ADD_PCI_CMD	| \
				 ADD_USB_CMD)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#if (TEXT_BASE == 0xFE000000)		/* Boot low */
#   define CFG_LOWBOOT		1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	\
	"setenv stdout serial;setenv stderr serial;" \
	"echo;" \
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
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip;bootm\0"	\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"bootfile=/tftpboot/MPC5200/uImage\0"				\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

#if defined(CONFIG_MPC5200)
/*
 * IPB Bus clocking configuration.
 */
#undef CFG_IPBSPEED_133   		/* define for 133MHz speed */
#endif

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_MODULE		1	/* Select I2C module #1 or #2 */

#define CFG_I2C_SPEED		100000 /* 100 kHz */
#define CFG_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CFG_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * Flash configuration
 */
#define CFG_FLASH_CFI		1	/* Flash is CFI conformant */
#define CFG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#if CONFIG_TOTAL5200_REV==2
#   define CFG_MAX_FLASH_BANKS	3	/* max num of flash banks */
#   define CFG_FLASH_BANKS_LIST { CFG_CS5_START, CFG_CS4_START, CFG_BOOTCS_START }
#else
#   define CFG_MAX_FLASH_BANKS	1	/* max num of flash banks  */
#   define CFG_FLASH_BANKS_LIST { CFG_BOOTCS_START }
#endif
#define CFG_FLASH_EMPTY_INFO
#define CFG_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#if CONFIG_TOTAL5200_REV==1
#   define CFG_FLASH_BASE	0xFE000000
#   define CFG_FLASH_SIZE	0x02000000
#elif CONFIG_TOTAL5200_REV==2
#   define CFG_FLASH_BASE	0xFA000000
#   define CFG_FLASH_SIZE	0x06000000
#endif /* CONFIG_TOTAL5200_REV */

#if defined(CFG_LOWBOOT)
#   define CFG_ENV_ADDR		0xFE040000
#else	/* CFG_LOWBOOT */
#   define CFG_ENV_ADDR		0xFFF40000
#endif	/* CFG_LOWBOOT */

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x40000
#define CFG_ENV_SECT_SIZE	0x40000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000
#define CFG_MBAR		0xF0000000	/*   64 kB */
#define CFG_FPGA_BASE		0xF0010000	/*   64 kB */
#define CFG_CPLD_BASE		0xF0020000	/*   64 kB */
#define CFG_LCD_BASE		0xF1000000	/* 4096 kB */

/* Use SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE	/* End of used area in DPRAM */

#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
/* dummy, 7-wire FEC does not have phy address */
#define CONFIG_PHY_ADDR		0x00

/*
 * GPIO configuration
 *
 * CS1:   SDRAM CS1 disabled, gpio_wkup_6 enabled                0
 * Reserved                                                      0
 * ALTs:  CAN1/2 on PSC2, SPI on PSC3                            00
 * CS7:   Interrupt GPIO on PSC3_5                               0
 * CS8:   Interrupt GPIO on PSC3_4                               0
 * ATA:   reset default, changed in ATA driver                   00
 * IR_USB_CLK: IrDA/USB 48MHz clock gen. int., pin is GPIO       0
 * IRDA:  reset default, changed in IrDA driver                  000
 * ETHER: reset default, changed in Ethernet driver              0000
 * PCI_DIS: reset default, changed in PCI driver                 0
 * USB_SE: reset default, changed in USB driver                  0
 * USB:   reset default, changed in USB driver                   00
 * PSC3:  SPI and UART functionality without CD                  1100
 * Reserved                                                      0
 * PSC2:  CAN1/2                                                 001
 * Reserved                                                      0
 * PSC1:  reset default, changed in AC'97 driver                 000
 *
 */
#define CFG_GPS_PORT_CONFIG	0x00000C10

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

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

#if defined (CONFIG_MGT5100)
#   define CONFIG_BOARD_EARLY_INIT_R	/* switch from CS_BOOT to CS0 */
#endif

#if CONFIG_TOTAL5200_REV==1
#   define CFG_BOOTCS_START	CFG_FLASH_BASE
#   define CFG_BOOTCS_SIZE	0x02000000	/* 32 MB */
#   define CFG_BOOTCS_CFG	0x0004DF00	/* 4WS, MX, AL, CE, AS_25, DS_32 */
#   define CFG_CS0_START	CFG_FLASH_BASE
#   define CFG_CS0_SIZE		0x02000000	/* 32 MB */
#else
#   define CFG_BOOTCS_START	(CFG_CS4_START + CFG_CS4_SIZE)
#   define CFG_BOOTCS_SIZE	0x02000000	/* 32 MB */
#   define CFG_BOOTCS_CFG	0x0004DF00	/* 4WS, MX, AL, CE, AS_25, DS_32 */
#   define CFG_CS4_START	(CFG_CS5_START + CFG_CS5_SIZE)
#   define CFG_CS4_SIZE		0x02000000	/* 32 MB */
#   define CFG_CS4_CFG		0x0004DF00	/* 4WS, MX, AL, CE, AS_25, DS_32 */
#   define CFG_CS5_START	CFG_FLASH_BASE
#   define CFG_CS5_SIZE		0x02000000	/* 32 MB */
#   define CFG_CS5_CFG		0x0004DF00	/* 4WS, MX, AL, CE, AS_25, DS_32 */
#endif

#define CFG_CS1_START		CFG_FPGA_BASE
#define CFG_CS1_SIZE		0x00010000	/* 64 kB */
#define CFG_CS1_CFG		0x0019FF00	/* 25WS, MX, AL, AA, CE, AS_25, DS_32 */

#define CFG_CS2_START		CFG_LCD_BASE
#define CFG_CS2_SIZE		0x00400000	/* 4096 kB */
#define CFG_CS2_CFG		0x0032FD0C	/* 50WS, MX, AL, AA, CE, AS_25, DS_16, endian swapping */

#if CONFIG_TOTAL5200_REV==1
#   define CFG_CS3_START	CFG_CPLD_BASE
#   define CFG_CS3_SIZE		0x00010000	/* 64 kB */
#   define CFG_CS3_CFG		0x000ADF00	/* 10WS, MX, AL, CE, AS_25, DS_32 */
#else
#   define CFG_CS3_START	CFG_CPLD_BASE
#   define CFG_CS3_SIZE		0x00010000	/* 64 kB */
#   define CFG_CS3_CFG		0x000AD800	/* 10WS, MX, AL, CE, AS_24, DS_8 */
#endif

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333

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

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

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

#endif /* __CONFIG_H */

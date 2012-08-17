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
#define CONFIG_MPC5200		1	/* (more precisely a MPC5200 CPU) */
#define CONFIG_TOTAL5200	1	/* ... on Total5200 board */

/*
 * Valid values for CONFIG_SYS_TEXT_BASE are:
 * 0xFFF00000	boot high (standard configuration)
 * 0xFE000000	boot low
 * 0x00100000	boot from RAM (for testing only)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_TEXT_BASE	0xFFF00000
#endif

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	3	/* console is on PSC3 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Video console
 */
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


/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
#define CONFIG_PCI_SCAN_SHOW	1
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CONFIG_MII		1
#define CONFIG_EEPRO100		1
#define CONFIG_SYS_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */
#define CONFIG_NS8382X		1

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/* USB */
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE


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

#define CONFIG_CMD_PCI

#define CONFIG_CMD_BMP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB


#if (CONFIG_SYS_TEXT_BASE == 0xFE000000)		/* Boot low */
#   define CONFIG_SYS_LOWBOOT		1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	\
	"setenv stdout serial;setenv stderr serial;" \
	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
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

/*
 * IPB Bus clocking configuration.
 */
#undef CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE		1	/* Select I2C module #1 or #2 */

#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#if CONFIG_TOTAL5200_REV==2
#   define CONFIG_SYS_MAX_FLASH_BANKS	3	/* max num of flash banks */
#   define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_CS5_START, CONFIG_SYS_CS4_START, CONFIG_SYS_BOOTCS_START }
#else
#   define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of flash banks  */
#   define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_BOOTCS_START }
#endif
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#if CONFIG_TOTAL5200_REV==1
#   define CONFIG_SYS_FLASH_BASE	0xFE000000
#   define CONFIG_SYS_FLASH_SIZE	0x02000000
#elif CONFIG_TOTAL5200_REV==2
#   define CONFIG_SYS_FLASH_BASE	0xFA000000
#   define CONFIG_SYS_FLASH_SIZE	0x06000000
#endif /* CONFIG_TOTAL5200_REV */

#if defined(CONFIG_SYS_LOWBOOT)
#   define CONFIG_ENV_ADDR		0xFE040000
#else	/* CONFIG_SYS_LOWBOOT */
#   define CONFIG_ENV_ADDR		0xFFF40000
#endif	/* CONFIG_SYS_LOWBOOT */

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x40000
#define CONFIG_ENV_SECT_SIZE	0x40000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000
#define CONFIG_SYS_MBAR		0xF0000000	/*   64 kB */
#define CONFIG_SYS_FPGA_BASE		0xF0010000	/*   64 kB */
#define CONFIG_SYS_CPLD_BASE		0xF0020000	/*   64 kB */
#define CONFIG_SYS_LCD_BASE		0xF1000000	/* 4096 kB */

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
#define CONFIG_MPC5xxx_FEC_SEVENWIRE
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
#define CONFIG_SYS_GPS_PORT_CONFIG	0x00000C10

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

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif


/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#if CONFIG_TOTAL5200_REV==1
#   define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#   define CONFIG_SYS_BOOTCS_SIZE	0x02000000	/* 32 MB */
#   define CONFIG_SYS_BOOTCS_CFG	0x0004DF00	/* 4WS, MX, AL, CE, AS_25, DS_32 */
#   define CONFIG_SYS_CS0_START	CONFIG_SYS_FLASH_BASE
#   define CONFIG_SYS_CS0_SIZE		0x02000000	/* 32 MB */
#else
#   define CONFIG_SYS_BOOTCS_START	(CONFIG_SYS_CS4_START + CONFIG_SYS_CS4_SIZE)
#   define CONFIG_SYS_BOOTCS_SIZE	0x02000000	/* 32 MB */
#   define CONFIG_SYS_BOOTCS_CFG	0x0004DF00	/* 4WS, MX, AL, CE, AS_25, DS_32 */
#   define CONFIG_SYS_CS4_START	(CONFIG_SYS_CS5_START + CONFIG_SYS_CS5_SIZE)
#   define CONFIG_SYS_CS4_SIZE		0x02000000	/* 32 MB */
#   define CONFIG_SYS_CS4_CFG		0x0004DF00	/* 4WS, MX, AL, CE, AS_25, DS_32 */
#   define CONFIG_SYS_CS5_START	CONFIG_SYS_FLASH_BASE
#   define CONFIG_SYS_CS5_SIZE		0x02000000	/* 32 MB */
#   define CONFIG_SYS_CS5_CFG		0x0004DF00	/* 4WS, MX, AL, CE, AS_25, DS_32 */
#endif

#define CONFIG_SYS_CS1_START		CONFIG_SYS_FPGA_BASE
#define CONFIG_SYS_CS1_SIZE		0x00010000	/* 64 kB */
#define CONFIG_SYS_CS1_CFG		0x0019FF00	/* 25WS, MX, AL, AA, CE, AS_25, DS_32 */

#define CONFIG_SYS_CS2_START		CONFIG_SYS_LCD_BASE
#define CONFIG_SYS_CS2_SIZE		0x00400000	/* 4096 kB */
#define CONFIG_SYS_CS2_CFG		0x0032FD0C	/* 50WS, MX, AL, AA, CE, AS_25, DS_16, endian swapping */

#if CONFIG_TOTAL5200_REV==1
#   define CONFIG_SYS_CS3_START	CONFIG_SYS_CPLD_BASE
#   define CONFIG_SYS_CS3_SIZE		0x00010000	/* 64 kB */
#   define CONFIG_SYS_CS3_CFG		0x000ADF00	/* 10WS, MX, AL, CE, AS_25, DS_32 */
#else
#   define CONFIG_SYS_CS3_START	CONFIG_SYS_CPLD_BASE
#   define CONFIG_SYS_CS3_SIZE		0x00010000	/* 64 kB */
#   define CONFIG_SYS_CS3_CFG		0x000AD800	/* 10WS, MX, AL, CE, AS_24, DS_8 */
#endif

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x33333333

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

#define CONFIG_SYS_ATA_CS_ON_I2C2
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

/* Interval between registers                                                */
#define CONFIG_SYS_ATA_STRIDE          4

#endif /* __CONFIG_H */

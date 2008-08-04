/*
 * (C) Copyright 2008
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com
 *
 * based on the Sequoia board configuration by
 * Stefan Roese, Jacqueline Pira-Ferriol and Alain Saurel
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

/*
 **********************************************************************
 * DU440.h - configuration for esd's DU440 board (Power PC440EPx)
 **********************************************************************
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_DU440		1		/* Board is esd DU440	*/
#define CONFIG_440EPX		1		/* Specific PPC440EPx	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_SYS_CLK_FREQ	33333400	/* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/
#define CONFIG_LAST_STAGE_INIT  1               /* last_stage_init      */

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor */
#define CFG_MALLOC_LEN		(8 << 20)	/* Reserve 8 MB for malloc()  */

#define CFG_BOOT_BASE_ADDR	0xf0000000
#define CFG_SDRAM_BASE		0x00000000	/* _must_ be 0		*/
#define CFG_FLASH_BASE		0xfc000000	/* start of FLASH	*/
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_NAND0_ADDR		0xd0000000      /* NAND Flash		*/
#define CFG_NAND1_ADDR		0xd0100000      /* NAND Flash		*/
#define CFG_OCM_BASE		0xe0010000      /* ocm			*/
#define CFG_PCI_BASE		0xe0000000      /* Internal PCI regs	*/
#define CFG_PCI_MEMBASE		0x80000000	/* mapped pci memory	*/
#define CFG_PCI_MEMBASE1	CFG_PCI_MEMBASE  + 0x10000000
#define CFG_PCI_MEMBASE2	CFG_PCI_MEMBASE1 + 0x10000000
#define CFG_PCI_MEMBASE3	CFG_PCI_MEMBASE2 + 0x10000000
#define CFG_PCI_IOBASE          0xe8000000


/* Don't change either of these */
#define CFG_PERIPHERAL_BASE	0xef600000	/* internal peripherals	*/

#define CFG_USB2D0_BASE		0xe0000100
#define CFG_USB_DEVICE		0xe0000000
#define CFG_USB_HOST		0xe0000400

/*
 * Initial RAM & stack pointer
 */
/* 440EPx/440GRx have 16KB of internal SRAM, so no need for D-Cache	*/
#define CFG_INIT_RAM_OCM	1		/* OCM as init ram	*/
#define CFG_INIT_RAM_ADDR	CFG_OCM_BASE	/* OCM			*/

#define CFG_INIT_RAM_END	(4 << 10)
#define CFG_GBL_DATA_SIZE	256		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*
 * Serial Port
 */
/* TODO: external clock oscillator will be removed */
#define CFG_EXT_SERIAL_CLOCK	11059200	/* ext. 11.059MHz clk	*/
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI     1
#undef CONFIG_UART1_CONSOLE

#define CFG_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*
 * Video Port
 */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_SMI_LYNXEM
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_VIDEO_BMP_GZIP              /* gzip compressed bmp images */
#define CFG_VIDEO_LOGO_MAX_SIZE (4 << 20)  /* for decompressed img */
#define CFG_DEFAULT_VIDEO_MODE 0x31a       /* 1280x1024,16bpp */
#define CFG_CONSOLE_IS_IN_ENV
#define CFG_ISA_IO CFG_PCI_IOBASE

/*
 * Environment
 */
#define CFG_ENV_IS_IN_EEPROM    1	/* use FLASH for environment vars */

/*
 * FLASH related
 */
#define CFG_FLASH_CFI			/* The flash is CFI compatible */
#define CFG_FLASH_CFI_DRIVER		/* Use common CFI driver       */

#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks	      */
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip  */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)    */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)    */

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)   */
/* CFI_FLASH_PROTECTION make flash_protect hang sometimes -> disabled */
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection      */

#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash      */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000 /* size of one complete sector        */
#define CFG_ENV_ADDR		((-CFG_MONITOR_LEN)-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector   */

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif

#ifdef CFG_ENV_IS_IN_EEPROM
#define CFG_ENV_OFFSET		0	/* environment starts at */
					/* the beginning of the EEPROM */
#define CFG_ENV_SIZE		0x1000 /* 4096 bytes may be used for env vars */
#endif

/*
 * DDR SDRAM
 */
#define CFG_MBYTES_SDRAM        (1024)	/* 512 MiB      TODO: remove    */
#define CONFIG_DDR_DATA_EYE		/* use DDR2 optimization        */
#define CFG_MEM_TOP_HIDE        (4 << 10) /* don't use last 4kbytes     */
					/* 440EPx errata CHIP 11        */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for setup     */
#define CONFIG_DDR_ECC			/* Use ECC when available       */
#define SPD_EEPROM_ADDRESS	{0x50}
#define CONFIG_PROG_SDRAM_TLB

/*
 * I2C
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support    */
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged	        */
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address  */
#define CFG_I2C_SLAVE		0x7F
#define CONFIG_I2C_CMD_TREE     1
#define CONFIG_I2C_MULTI_BUS    1

#define CFG_SPD_BUS_NUM         0
#define IIC1_MCP3021_ADDR	0x4d
#define IIC1_USB2507_ADDR	0x2c
#ifdef CONFIG_I2C_MULTI_BUS
#define CFG_I2C_NOPROBES        {{1, IIC1_USB2507_ADDR}}
#endif
#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	0x54
#define CFG_I2C_EEPROM_ADDR_LEN 2
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 5
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10
#define CFG_I2C_EEPROM_ADDR_OVERFLOW 0x01

#define CFG_EEPROM_WREN         1
#define CFG_I2C_BOOT_EEPROM_ADDR 0x52

/*
 * standard dtt sensor configuration - bottom bit will determine local or
 * remote sensor of the TMP401
 */
#define CONFIG_DTT_SENSORS		{ 0, 1 }

/*
 * The PMC440 uses a TI TMP401 temperature sensor. This part
 * is basically compatible to the ADM1021 that is supported
 * by U-Boot.
 *
 * - i2c addr 0x4c
 * - conversion rate 0x02 = 0.25 conversions/second
 * - ALERT ouput disabled
 * - local temp sensor enabled, min set to 0 deg, max set to 70 deg
 * - remote temp sensor enabled, min set to 0 deg, max set to 70 deg
 */
#define CONFIG_DTT_ADM1021
#define CFG_DTT_ADM1021		{ { 0x4c, 0x02, 0, 1, 70, 0, 1, 70, 0} }

/*
 * RTC stuff
 */
#define CONFIG_RTC_DS1338
#define CFG_I2C_RTC_ADDR	0x68

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"ethrotate=no\0"						\
	"hostname=du440\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"flash_self=run ramargs addip addtty optargs;"			\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${img};run nfsargs addip addtty optargs;"	\
	        "bootm\0"						\
	"rootpath=/tftpboot/du440/target_root_du440\0"			\
	"img=/tftpboot/du440/uImage\0"					\
	"kernel_addr=FFC00000\0"					\
	"ramdisk_addr=FFE00000\0"					\
	"initrd_high=30000000\0"					\
	"load=tftp 100000 /tftpboot/du440/u-boot.bin\0"			\
	"update=protect off FFFA0000 FFFFFFFF;era FFFA0000 FFFFFFFF;"	\
		"cp.b 100000 FFFA0000 60000\0"				\
	""

#define CONFIG_PREBOOT                  /* enable preboot variable      */

#define CONFIG_BOOTDELAY	3	/* autoboot after 5 seconds	*/

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#ifndef __ASSEMBLY__
int du440_phy_addr(int devnum);
#endif

#define	CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		du440_phy_addr(0) /* PHY address	*/

#define CONFIG_PHY_RESET        1	/* reset phy upon startup	*/
#undef CONFIG_PHY_GIGE			/* no GbE detection		*/

#define CONFIG_HAS_ETH0
#define CFG_RX_ETH_BUFFER	128

#define CONFIG_NET_MULTI	1
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#define CONFIG_PHY1_ADDR	du440_phy_addr(1)

/*
 * USB
 */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CFG_OHCI_BE_CONTROLLER

#define CFG_USB_OHCI_CPU_INIT	1
#define CFG_USB_OHCI_REGS_BASE	CFG_USB_HOST
#define CFG_USB_OHCI_SLOT_NAME	"du440"
#define CFG_USB_OHCI_MAX_ROOT_PORTS	15

/* Comment this out to enable USB 1.1 device */
#define USB_2_0_DEVICE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#include <config_cmd_default.h>

#define CONFIG_CMD_AUTOSCRIPT
#define CONFIG_CMD_BSP
#define CONFIG_CMD_BMP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DTT
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM

#define CONFIG_SUPPORT_VFAT

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
/* Print Buffer Size */
#define CFG_PBSIZE              (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS	        16	/* max number of command args	*/
#define CFG_BARGSIZE	        CFG_CBSIZE /* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00400000 /* memtest works on		*/
#define CFG_MEMTEST_END		0x3f000000 /* 4 ... < 1GB DRAM	*/

#define CFG_LOAD_ADDR		0x100000  /* default load address	*/
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ		        1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

#define CONFIG_AUTOBOOT_KEYED	1
#define CONFIG_AUTOBOOT_PROMPT	\
	"Press SPACE to abort autoboot in %d seconds\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR "d"
#define CONFIG_AUTOBOOT_STOP_STR " "

/*
 * PCI stuff
 */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CFG_PCI_TARGBASE       0x80000000 /* PCIaddr mapped to CFG_PCI_MEMBASE*/

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT
#define CFG_PCI_MASTER_INIT

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)     /* Initial Memory map for Linux */

/*
 * External Bus Controller (EBC) Setup
 */
#define CFG_FLASH		CFG_FLASH_BASE

#define CFG_CPLD_BASE		0xC0000000
#define CFG_CPLD_RANGE	        0x00000010
#define CFG_DUMEM_BASE		0xC0100000
#define CFG_DUMEM_RANGE		0x00100000
#define CFG_DUIO_BASE		0xC0200000
#define CFG_DUIO_RANGE	        0x00010000

#define CFG_NAND0_CS		2		/* NAND chip connected to CSx */
#define CFG_NAND1_CS		3		/* NAND chip connected to CSx */
/* Memory Bank 0 (NOR-FLASH) initialization */
#define CFG_EBC_PB0AP		0x04017200
#define CFG_EBC_PB0CR		(CFG_FLASH_BASE | 0xda000)

/* Memory Bank 1 (CPLD, 16 bytes needed, but 1MB is minimum) */
#define CFG_EBC_PB1AP		0x018003c0
#define CFG_EBC_PB1CR		(CFG_CPLD_BASE | 0x18000)

/* Memory Bank 2 (NAND-FLASH) initialization */
#define CFG_EBC_PB2AP		0x018003c0
#define CFG_EBC_PB2CR		(CFG_NAND0_ADDR | 0x1c000)

/* Memory Bank 3 (NAND-FLASH) initialization */
#define CFG_EBC_PB3AP		0x018003c0
#define CFG_EBC_PB3CR		(CFG_NAND1_ADDR | 0x1c000)

/* Memory Bank 4 (DUMEM, 1MB) initialization */
#define CFG_EBC_PB4AP		0x018053c0
#define CFG_EBC_PB4CR		(CFG_DUMEM_BASE | 0x18000)

/* Memory Bank 5 (DUIO, 64KB needed, but 1MB is minimum) */
#define CFG_EBC_PB5AP		0x018053c0
#define CFG_EBC_PB5CR		(CFG_DUIO_BASE | 0x18000)

/*
 * NAND FLASH
 */
#define CFG_MAX_NAND_DEVICE	2
#define NAND_MAX_CHIPS		CFG_MAX_NAND_DEVICE
#define CFG_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips */
#define CFG_NAND_BASE_LIST	{CFG_NAND0_ADDR + CFG_NAND0_CS, \
				 CFG_NAND1_ADDR + CFG_NAND1_CS}

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

#define CONFIG_AUTOSCRIPT	1

#endif	/* __CONFIG_H */

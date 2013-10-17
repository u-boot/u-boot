/*
 * (C) Copyright 2008
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com
 *
 * based on the Sequoia board configuration by
 * Stefan Roese, Jacqueline Pira-Ferriol and Alain Saurel
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFFA0000
#endif

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/
#define CONFIG_LAST_STAGE_INIT  1               /* last_stage_init      */

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN		(8 << 20)	/* Reserve 8 MB for malloc()  */

#define CONFIG_SYS_BOOT_BASE_ADDR	0xf0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000	/* _must_ be 0		*/
#define CONFIG_SYS_FLASH_BASE		0xfc000000	/* start of FLASH	*/
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND0_ADDR		0xd0000000      /* NAND Flash		*/
#define CONFIG_SYS_NAND1_ADDR		0xd0100000      /* NAND Flash		*/
#define CONFIG_SYS_OCM_BASE		0xe0010000      /* ocm			*/
#define CONFIG_SYS_PCI_BASE		0xe0000000      /* Internal PCI regs	*/
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped pci memory	*/
#define CONFIG_SYS_PCI_MEMBASE1	CONFIG_SYS_PCI_MEMBASE  + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE2	CONFIG_SYS_PCI_MEMBASE1 + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE3	CONFIG_SYS_PCI_MEMBASE2 + 0x10000000
#define CONFIG_SYS_PCI_IOBASE		0xe8000000
#define CONFIG_SYS_PCI_SUBSYS_VENDORID	PCI_VENDOR_ID_ESDGMBH
#define CONFIG_SYS_PCI_SUBSYS_ID	0x0444		/* device ID for DU440 */

#define CONFIG_SYS_USB2D0_BASE		0xe0000100
#define CONFIG_SYS_USB_DEVICE		0xe0000000
#define CONFIG_SYS_USB_HOST		0xe0000400

/*
 * Initial RAM & stack pointer
 */
/* 440EPx/440GRx have 16KB of internal SRAM, so no need for D-Cache	*/
#define CONFIG_SYS_INIT_RAM_OCM	1		/* OCM as init ram	*/
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_BASE	/* OCM			*/

#define CONFIG_SYS_INIT_RAM_SIZE	(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#undef CONFIG_SYS_EXT_SERIAL_CLOCK
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE						\
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
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (4 << 20)  /* for decompressed img */
#define CONFIG_SYS_DEFAULT_VIDEO_MODE 0x31a       /* 1280x1024,16bpp */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_ISA_IO CONFIG_SYS_PCI_IOBASE

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_EEPROM    1	/* use FLASH for environment vars */

/*
 * FLASH related
 */
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible */
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver       */

#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks	      */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip  */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)    */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)    */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)   */
/* CFI_FLASH_PROTECTION make flash_protect hang sometimes -> disabled */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* use hardware flash protection      */

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash      */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000 /* size of one complete sector        */
#define CONFIG_ENV_ADDR		((-CONFIG_SYS_MONITOR_LEN)-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector   */

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif

#ifdef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET		0	/* environment starts at */
					/* the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x1000 /* 4096 bytes may be used for env vars */
#endif

/*
 * DDR SDRAM
 */
#define CONFIG_SYS_MBYTES_SDRAM        (1024)	/* 512 MiB      TODO: remove    */
#define CONFIG_DDR_DATA_EYE		/* use DDR2 optimization        */
#define CONFIG_SYS_MEM_TOP_HIDE        (4 << 10) /* don't use last 4kbytes     */
					/* 440EPx errata CHIP 11        */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for setup     */
#define CONFIG_DDR_ECC			/* Use ECC when available       */
#define SPD_EEPROM_ADDRESS	{0x50}
#define CONFIG_PROG_SDRAM_TLB

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		100000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F
#define CONFIG_SYS_I2C_PPC4XX_CH1
#define CONFIG_SYS_I2C_PPC4XX_SPEED_1		100000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_1		0x7F

#define CONFIG_SYS_SPD_BUS_NUM         0
#define IIC1_MCP3021_ADDR	0x4d
#define IIC1_USB2507_ADDR	0x2c
#define CONFIG_SYS_I2C_NOPROBES		{ {1, IIC1_USB2507_ADDR} }

#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 5
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW 0x01

#define CONFIG_SYS_EEPROM_WREN         1
#define CONFIG_SYS_I2C_BOOT_EEPROM_ADDR 0x52

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
#define CONFIG_SYS_DTT_ADM1021		{ { 0x4c, 0x02, 0, 1, 70, 0, 1, 70, 0} }

/*
 * RTC stuff
 */
#define CONFIG_RTC_DS1338
#define CONFIG_SYS_I2C_RTC_ADDR	0x68

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
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#ifndef __ASSEMBLY__
int du440_phy_addr(int devnum);
#endif

#define CONFIG_PPC4xx_EMAC
#define	CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		du440_phy_addr(0) /* PHY address	*/

#define CONFIG_PHY_RESET        1	/* reset phy upon startup	*/
#undef CONFIG_PHY_GIGE			/* no GbE detection		*/

#define CONFIG_HAS_ETH0
#define CONFIG_SYS_RX_ETH_BUFFER	128

#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#define CONFIG_PHY1_ADDR	du440_phy_addr(1)

/*
 * USB
 */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CONFIG_SYS_OHCI_BE_CONTROLLER

#define CONFIG_SYS_USB_OHCI_CPU_INIT	1
#define CONFIG_SYS_USB_OHCI_REGS_BASE	CONFIG_SYS_USB_HOST
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"du440"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15

/* Comment this out to enable USB 1.1 device */
#define USB_2_0_DEVICE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BMP
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_DTT
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
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_USB

#define CONFIG_SUPPORT_VFAT

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE              (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	        16	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	        CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00400000 /* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x3f000000 /* 4 ... < 1GB DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000  /* default load address	*/
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ		        1000	/* decrementer freq: 1 ms ticks	*/

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
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CONFIG_SYS_PCI_TARGBASE       0x80000000 /* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE*/

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT
#define CONFIG_SYS_PCI_MASTER_INIT

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)     /* Initial Memory map for Linux */

/*
 * External Bus Controller (EBC) Setup
 */
#define CONFIG_SYS_FLASH		CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_CPLD_BASE		0xC0000000
#define CONFIG_SYS_CPLD_RANGE	        0x00000010
#define CONFIG_SYS_DUMEM_BASE		0xC0100000
#define CONFIG_SYS_DUMEM_RANGE		0x00100000
#define CONFIG_SYS_DUIO_BASE		0xC0200000
#define CONFIG_SYS_DUIO_RANGE	        0x00010000

#define CONFIG_SYS_NAND0_CS		2		/* NAND chip connected to CSx */
#define CONFIG_SYS_NAND1_CS		3		/* NAND chip connected to CSx */
/* Memory Bank 0 (NOR-FLASH) initialization */
#define CONFIG_SYS_EBC_PB0AP		0x04017200
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH_BASE | 0xda000)

/* Memory Bank 1 (CPLD, 16 bytes needed, but 1MB is minimum) */
#define CONFIG_SYS_EBC_PB1AP		0x018003c0
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_CPLD_BASE | 0x18000)

/* Memory Bank 2 (NAND-FLASH) initialization */
#define CONFIG_SYS_EBC_PB2AP		0x018003c0
#define CONFIG_SYS_EBC_PB2CR		(CONFIG_SYS_NAND0_ADDR | 0x1c000)

/* Memory Bank 3 (NAND-FLASH) initialization */
#define CONFIG_SYS_EBC_PB3AP		0x018003c0
#define CONFIG_SYS_EBC_PB3CR		(CONFIG_SYS_NAND1_ADDR | 0x1c000)

/* Memory Bank 4 (DUMEM, 1MB) initialization */
#define CONFIG_SYS_EBC_PB4AP		0x018053c0
#define CONFIG_SYS_EBC_PB4CR		(CONFIG_SYS_DUMEM_BASE | 0x18000)

/* Memory Bank 5 (DUIO, 64KB needed, but 1MB is minimum) */
#define CONFIG_SYS_EBC_PB5AP		0x018053c0
#define CONFIG_SYS_EBC_PB5CR		(CONFIG_SYS_DUIO_BASE | 0x18000)

/*
 * NAND FLASH
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	2
#define CONFIG_SYS_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips */
#define CONFIG_SYS_NAND_BASE_LIST	{CONFIG_SYS_NAND0_ADDR + CONFIG_SYS_NAND0_CS, \
				 CONFIG_SYS_NAND1_ADDR + CONFIG_SYS_NAND1_CS}

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

#define CONFIG_SOURCE		1

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP

#endif	/* __CONFIG_H */

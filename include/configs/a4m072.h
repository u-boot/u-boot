/*
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2010
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC5200		1	/* This is a MPC5200 CPU */
#define CONFIG_A4M072		1	/* ... on A4M072 board */
#define CONFIG_MPC5200_DDR	1	/* ... use DDR RAM */

#define CONFIG_SYS_TEXT_BASE	0xFE000000

#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }
/* define to enable silent console */
#define CONFIG_SYS_DEVICE_NULLDEV	1	/* include nulldev device */

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */

#if defined(CONFIG_PCI)
#define CONFIG_PCI_SCAN_SHOW	1
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000
#endif

#define CONFIG_SYS_XLB_PIPELINING	1

#undef CONFIG_EEPRO100

/* USB */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_OHCI_BE_CONTROLLER
#undef CONFIG_SYS_USB_OHCI_BOARD_INIT
#define CONFIG_SYS_USB_OHCI_CPU_INIT	1
#define CONFIG_SYS_USB_OHCI_REGS_BASE	MPC5XXX_USB
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"mpc5200"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */

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
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_IDE

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

#if (CONFIG_SYS_TEXT_BASE == 0xFE000000)		/* Boot low with 32 MB Flash */
#define CONFIG_SYS_LOWBOOT		1
#define CONFIG_SYS_LOWBOOT32		1
#endif

/*
 * Autobooting
 */

#define CONFIG_SYS_AUTOLOAD	"n"

#undef	CONFIG_BOOTARGS
#define CONFIG_PREBOOT				"run try_update"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"bk=run add_mtd ; run add_consolespec ; bootm 200000\0"		\
	"cf1=diskboot 200000 0:1\0"					\
	"bootcmd_cf1=run bcf1\0"					\
	"bcf=setenv bootargs root=/dev/hda3\0"				\
	"bootcmd_nfs=run bnfs\0"					\
	"norargs=setenv bootargs root=/dev/mtdblock3 rootfstype=cramfs "\
		"panic=1\0"						\
	"bootcmd_nor=cp.b ${kernel_addr} 200000 100000;"		\
			"run norargs addip; run bk\0"			\
	"bnfs=nfs 200000 ${rootpath}/boot/uImage;"			\
			"run nfsargs addip ; run bk\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
				"nfsroot=${serverip}:${rootpath}\0"	\
	"try_update=usb start;sleep 2;usb start;sleep 1;"		\
			"fatload usb 0 2F0000 PCPUUPDT 2FF;usb stop;"	\
			"source 2F0000\0"				\
	"env_addr=FE060000\0"						\
	"kernel_addr=FE100000\0"					\
	"rootfs_addr=FE200000\0"					\
	"add_mtd=setenv bootargs ${bootargs} mtdparts="			\
		"phys_mapped_flash:384k(u),640k(e),1m(k),30m(r)\0"	\
	"bcf1=run cf1; run bcf; run addip; run bk\0"			\
	"add_consolespec=setenv bootargs ${bootargs} "			\
				"console=/dev/null quiet\0"		\
	"addip=if test -n ${ethaddr};"					\
		"then if test -n ${ipaddr};"				\
			"then setenv bootargs ${bootargs} "		\
				"ip=${ipaddr}:${serverip}:${gatewayip}:"\
				"${netmask}:${hostname}:${netdev}:off;"	\
			"fi;"						\
		"else;"							\
			"setenv bootargs ${bootargs} no_ethaddr;"	\
		"fi\0"							\
	"hostname=CPUP0\0"						\
	"netdev=eth0\0"							\
	"bootcmd=run bootcmd_nor\0" 					\
	""
/*
 * IPB Bus clocking configuration.
 */
#undef CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C			1	/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE		2	/* Select I2C module #1 or #2 */

#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x52	/* 1010010x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10
#define CONFIG_SYS_EEPROM_WREN			1
#define CONFIG_SYS_EEPROM_WP			GPIO_PSC2_4

/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_BASE		0xFE000000
#define CONFIG_SYS_FLASH_SIZE		0x02000000
#if !defined(CONFIG_SYS_LOWBOOT)
#error "CONFIG_SYS_LOWBOOT not defined?"
#else	/* CONFIG_SYS_LOWBOOT */
#if defined(CONFIG_SYS_LOWBOOT32)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00060000)
#endif
#endif	/* CONFIG_SYS_LOWBOOT */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks      */
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max num of sects on one chip */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_CS0_START}
#define CONFIG_SYS_FLASH_BANKS_SIZES	{CONFIG_SYS_CS0_SIZE}

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE	0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_SIZE		MPC5XXX_SRAM_SIZE	/* Size of used area in DPRAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(384 << 10)	/* Reserve 384 kB for Monitor	*/
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
#define CONFIG_PHY_ADDR		0x1f
#define CONFIG_PHY_TYPE		0x79c874		/* AMD Phy Controller */

/*
 * GPIO configuration
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x18000004

/*
 * Miscellaneous configurable options
 */
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
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

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE
/* Flash at CSBoot, CS0 */
#define CONFIG_SYS_BOOTCS_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x0002DD00
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE
/* External SRAM at CS1 */
#define CONFIG_SYS_CS1_START		0x62000000
#define CONFIG_SYS_CS1_SIZE		0x00400000
#define CONFIG_SYS_CS1_CFG		0x00009930
#define CONFIG_SYS_SRAM_BASE		CONFIG_SYS_CS1_START
#define CONFIG_SYS_SRAM_SIZE		CONFIG_SYS_CS1_SIZE
/* LED display at CS7 */
#define CONFIG_SYS_CS7_START		0x6a000000
#define CONFIG_SYS_CS7_SIZE		(64*1024)
#define CONFIG_SYS_CS7_CFG		0x0000bf30

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE		0x33333003

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00001000 /* 0x4000 for SE mode */

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 2 drives per IDE bus	*/

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

#define CONFIG_ATAPI                   1

/*-----------------------------------------------------------------------
 * Open firmware flat tree support
 *-----------------------------------------------------------------------
 */
#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc5200@f0000000/serial@2000"

/* Support for the 7-segment display */
#define CONFIG_SYS_DISP_CHR_RAM	     CONFIG_SYS_CS7_START
#define CONFIG_SHOW_ACTIVITY		/* used for display realization */

#define CONFIG_SHOW_BOOT_PROGRESS

#endif /* __CONFIG_H */

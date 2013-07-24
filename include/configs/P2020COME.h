/*
 * Copyright 2009-2010,2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* The P2020COME board is only booted via the Freescale On-Chip ROM */
#define CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_EXTRA_ENV_RELOC

#define CONFIG_SYS_TEXT_BASE		0xf8f80000
#define CONFIG_RESET_VECTOR_ADDRESS	0xf8fffffc

#ifdef CONFIG_SDCARD
#define CONFIG_RAMBOOT_SDCARD		1
#endif

#ifdef CONFIG_SPIFLASH
#define CONFIG_RAMBOOT_SPIFLASH		1
#endif

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48/P1020/P2020,etc*/
#define CONFIG_P2020		1
#define CONFIG_P2020COME	1
#define CONFIG_FSL_ELBC		1	/* Enable eLBC Support */
#define CONFIG_MP

#define CONFIG_PCI		1	/* Enable PCI/PCIE */
#if defined(CONFIG_PCI)
#define CONFIG_PCIE1		1	/* PCIE controller 1 (slot 1) */
#define CONFIG_PCIE2		1	/* PCIE controller 2 (slot 2) */
#define CONFIG_PCIE3		1	/* PCIE controller 3 (slot 3) */

#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */
#define CONFIG_FSL_PCIE_RESET	1	/* need PCIe reset errata */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */
#endif /* #if defined(CONFIG_PCI) */
#define CONFIG_FSL_LAW		1	/* Use common FSL init code */
#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_PCI)
#define CONFIG_E1000		1	/* E1000 pci Ethernet card */
#endif

#ifndef __ASSEMBLY__
extern unsigned long get_board_ddr_clk(unsigned long dummy);
extern unsigned long get_board_sys_clk(unsigned long dummy);
#endif

/*
 * For P2020COME DDRCLK and SYSCLK are from the same oscillator
 * For DA phase the SYSCLK is 66MHz
 * For EA phase the SYSCLK is 100MHz
 */
#define CONFIG_DDR_CLK_FREQ	get_board_ddr_clk(0)
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0)

#define CONFIG_HWCONFIG

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch prediction */

#define CONFIG_ADDR_STREAMING		/* toggle addr streaming */

#define CONFIG_ENABLE_36BIT_PHYS	1

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP			1
#define CONFIG_SYS_NUM_ADDR_MAP		16	/* number of TLB1 entries */
#endif

#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x1fffffff
#define CONFIG_PANIC_HANG	/* do not reset board on panic */

 /*
  * Config the L2 Cache as L2 SRAM
  */
#define CONFIG_SYS_INIT_L2_ADDR		0xf8f80000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	0xff8f80000ull
#else
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	CONFIG_SYS_INIT_L2_ADDR
#endif
#define CONFIG_SYS_L2_SIZE		(512 << 10)
#define CONFIG_SYS_INIT_L2_END		(CONFIG_SYS_INIT_L2_ADDR \
					+ CONFIG_SYS_L2_SIZE)

#define CONFIG_SYS_CCSRBAR		0xffe00000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_FSL_DDR3
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#define CONFIG_DDR_SPD

#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef

#define CONFIG_SYS_SDRAM_SIZE		2048ULL	/* DDR size on P2020COME */
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

#define CONFIG_SYS_DDR_ERR_INT_EN	0x0000000d
#define CONFIG_SYS_DDR_ERR_DIS		0x00000000
#define CONFIG_SYS_DDR_SBE		0x00ff0000

#define CONFIG_SYS_SPD_BUS_NUM		1
#define SPD_EEPROM_ADDRESS		0x53

/*
 * Memory map
 *
 * 0x0000_0000	0x7fff_ffff	DDR3			2G Cacheable
 * 0x8000_0000	0x9fff_ffff	PCI Express 3 Mem	1G non-cacheable
 * 0xa000_0000	0xbfff_ffff	PCI Express 2 Mem	1G non-cacheable
 * 0xc000_0000	0xdfff_ffff	PCI Express 1 Mem	1G non-cacheable
 * 0xffc1_0000	0xffc1_ffff	PCI Express 3 IO	64K non-cacheable
 * 0xffc2_0000	0xffc2_ffff	PCI Express 2 IO	64K non-cacheable
 * 0xffc3_0000	0xffc3_ffff	PCI Express 1 IO	64K non-cacheable
 *
 * 0xffd0_0000	0xffd0_3fff	L1 for stack		16K Cacheable TLB0
 * 0xffe0_0000	0xffef_ffff	CCSR			1M non-cacheable
 */

/*
 * Local Bus Definitions
 */

/* There is no NOR Flash on P2020COME */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_BOARD_EARLY_INIT_R	/* call board_early_init_r function */
#define CONFIG_HWCONFIG

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000	/* stack in RAM */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH	0xf
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW	CONFIG_SYS_INIT_RAM_ADDR
/* the assembler doesn't like typecast */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS \
	((CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#else
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS	CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR_PHYS
#endif
#define CONFIG_SYS_INIT_RAM_SIZE	0x00004000

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE \
						- GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_CONSOLE_IS_IN_ENV	/* determine from environment */

#define CONFIG_SYS_BAUDRATE_TABLE   \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/*
 * Pass open firmware flat tree
 */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/* new uImage format support */
#define CONFIG_FIT			1
#define CONFIG_FIT_VERBOSE		1

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x29} }

/*
 * I2C2 EEPROM
 */
#define CONFIG_ID_EEPROM
#ifdef CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#endif
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR2	0x18
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_EEPROM_BUS_NUM	0
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10 /* and takes up to 10 msec */

/*
 * eSPI - Enhanced SPI
 */
#define CONFIG_FSL_ESPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED		10000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#if defined(CONFIG_PCI)

/* controller 3, Slot 3, tgtid 3, Base address 8000 */
#define CONFIG_SYS_PCIE3_MEM_VIRT	0x80000000
#define CONFIG_SYS_PCIE3_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0x80000000
#define CONFIG_SYS_PCIE3_MEM_SIZE	0x20000000  /* 512M */
#define CONFIG_SYS_PCIE3_IO_VIRT	0xffc10000
#define CONFIG_SYS_PCIE3_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE3_IO_PHYS	0xffc10000
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00010000  /* 64k */

/* controller 2, Slot 2, tgtid 2, Base address 9000 */
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000  /* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc20000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xffc20000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000  /* 64k */

/* controller 1, Slot 1, tgtid 1, Base address a000 */
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000  /* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc30000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xffc30000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000  /* 64k */

#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP
#undef CONFIG_RTL8139

#ifdef CONFIG_RTL8139
/* This macro is used by RTL8139 but not defined in PPC architecture */
#define KSEG1ADDR(x)		(x)
#define _IO_BASE		0x00000000
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_DOS_PARTITION

#endif	/* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_MII		1	/* MII PHY management */
#define CONFIG_MII_DEFAULT_TSEC	1	/* Allow unregistered phys */
#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"eTSEC2"
#define CONFIG_TSEC3		1
#define CONFIG_TSEC3_NAME	"eTSEC3"

#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		2
#define TSEC3_PHY_ADDR		1

#undef CONFIG_VSC7385_ENET

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0

#define CONFIG_ETHPRIME		"eTSEC1"

#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#if defined(CONFIG_RAMBOOT_SDCARD)
	#define CONFIG_ENV_IS_IN_MMC	1
	#define CONFIG_FSL_FIXED_MMC_LOCATION
	#define CONFIG_ENV_SIZE		0x2000
	#define CONFIG_SYS_MMC_ENV_DEV	0
#elif defined(CONFIG_RAMBOOT_SPIFLASH)
	#define CONFIG_ENV_IS_IN_SPI_FLASH
	#define CONFIG_ENV_SPI_BUS	0
	#define CONFIG_ENV_SPI_CS	0
	#define CONFIG_ENV_SPI_MAX_HZ	10000000
	#define CONFIG_ENV_SPI_MODE	0
	#define CONFIG_ENV_OFFSET	0x100000	/* 1MB */
	#define CONFIG_ENV_SECT_SIZE	0x10000
	#define CONFIG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO		1
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_PCI)
#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI
#endif

#undef CONFIG_WATCHDOG			/* watchdog disabled */

#define CONFIG_MMC	1

#ifdef CONFIG_MMC
#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */
#define CONFIG_CMD_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_FSL_ESDHC
#define CONFIG_GENERIC_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
#define CONFIG_SYS_FSL_ESDHC_BROKEN_TIMEOUT
#endif /* CONFIG_MMC */

#define CONFIG_HAS_FSL_DR_USB
#ifdef CONFIG_HAS_FSL_DR_USB
#define CONFIG_USB_EHCI

#ifdef CONFIG_USB_EHCI
#define CONFIG_CMD_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_FSL
#define CONFIG_USB_STORAGE
#endif
#endif

#if defined(CONFIG_MMC) || defined(CONFIG_USB_EHCI)
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif

/* Misc Extra Settings */
#define CONFIG_CMD_DHCP			1

#define CONFIG_CMD_DATE			1
#define CONFIG_RTC_M41T62		1
#define CONFIG_SYS_RTC_BUS_NUM		1
#define CONFIG_SYS_I2C_RTC_ADDR		0x68

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_CMDLINE_EDITING			/* Command-line editing */
#define CONFIG_AUTO_COMPLETE	1		/* add autocompletion support */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
						/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms tick */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

/* The mac addresses for all ethernet interface */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#endif

#define CONFIG_HOSTNAME		unknown
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY	10	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS			/* the boot command will set bootargs */

#define CONFIG_BAUDRATE		115200

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"hwconfig=fsl_ddr:ecc=on\0"					\
	"bootcmd=run sdboot\0"						\
	"sdboot=setenv bootargs root=/dev/mmcblk0p2 rw "		\
		"rootdelay=$rootdelaysecond console=$consoledev,$baudrate "\
		"$othbootargs; mmcinfo; "				\
		"ext2load mmc 0:2 $loadaddr /boot/$bootfile; "		\
		"ext2load mmc 0:2 $fdtaddr /boot/$fdtfile; "		\
		"bootm $loadaddr - $fdtaddr\0"				\
	"sdfatboot=setenv bootargs root=/dev/ram rw "			\
		"rootdelay=$rootdelaysecond console=$consoledev,$baudrate "\
		"$othbootargs; mmcinfo; "				\
		"fatload mmc 0:1 $loadaddr $bootfile; "			\
		"fatload mmc 0:1 $fdtaddr $fdtfile; "			\
		"fatload mmc 0:1 $ramdiskaddr $ramdiskfile; "		\
		"bootm $loadaddr $ramdiskaddr $fdtaddr\0"		\
	"usbboot=setenv bootargs root=/dev/sda1 rw "			\
		"rootdelay=$rootdelaysecond console=$consoledev,$baudrate "\
		"$othbootargs; "					\
		"usb start; "						\
		"ext2load usb 0:1 $loadaddr /boot/$bootfile; "		\
		"ext2load usb 0:1 $fdtaddr /boot/$fdtfile; "		\
		"bootm $loadaddr - $fdtaddr\0"				\
	"usbfatboot=setenv bootargs root=/dev/ram rw "			\
		"console=$consoledev,$baudrate $othbootargs; "		\
		"usb start; "						\
		"fatload usb 0:2 $loadaddr $bootfile; "			\
		"fatload usb 0:2 $fdtaddr $fdtfile; "			\
		"fatload usb 0:2 $ramdiskaddr $ramdiskfile; "		\
		"bootm $loadaddr $ramdiskaddr $fdtaddr\0"		\
	"usbext2boot=setenv bootargs root=/dev/ram rw "			\
		"console=$consoledev,$baudrate $othbootargs; "		\
		"usb start; "						\
		"ext2load usb 0:4 $loadaddr $bootfile; "		\
		"ext2load usb 0:4 $fdtaddr $fdtfile; "			\
		"ext2load usb 0:4 $ramdiskaddr $ramdiskfile; "		\
		"bootm $loadaddr $ramdiskaddr $fdtaddr\0"		\
	"upgradespi=sf probe 0; "					\
		"setenv startaddr 0; "					\
		"setenv erasesize a0000; "				\
		"tftp 1000000 $tftppath/$uboot_spi; "			\
		"sf erase $startaddr $erasesize; "			\
		"sf write 1000000 $startaddr $filesize; "		\
		"sf erase 100000 120000\0"				\
	"clearspienv=sf probe 0;sf erase 100000 20000\0"		\
	"othbootargs=ramdisk_size=700000 cache-sram-size=0x10000\0"	\
	"netdev=eth0\0"							\
	"rootdelaysecond=15\0"						\
	"uboot_nor=u-boot-nor.bin\0"					\
	"uboot_spi=u-boot-p2020.spi\0"					\
	"uboot_sd=u-boot-p2020.bin\0"					\
	"consoledev=ttyS0\0"						\
	"ramdiskaddr=2000000\0"						\
	"ramdiskfile=rootfs-dev.ext2.img\0"				\
	"fdtaddr=c00000\0"						\
	"fdtfile=uImage-2.6.32-p2020.dtb\0"				\
	"tftppath=p2020\0"

#define CONFIG_HDBOOT							\
	"setenv bootargs root=/dev/$bdev rw rootdelay=30 "		\
	"console=$consoledev,$baudrate $othbootargs;"			\
	"usb start;"							\
	"ext2load usb 0:1 $loadaddr /boot/$bootfile;"			\
	"ext2load usb 0:1 $fdtaddr /boot/$fdtfile;"			\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw "				\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "\
	"console=$consoledev,$baudrate $othbootargs;"			\
	"tftp $loadaddr $tftppath/$bootfile;"				\
	"tftp $fdtaddr $tftppath/$fdtfile;"				\
	"bootm $loadaddr - $fdtaddr"


#define CONFIG_RAMBOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw "				\
	"console=$consoledev,$baudrate $othbootargs;"			\
	"tftp $ramdiskaddr $tftppath/$ramdiskfile;"			\
	"tftp $loadaddr $tftppath/$bootfile;"				\
	"tftp $fdtaddr $tftppath/$fdtfile;"				\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_HDBOOT

#endif  /* __CONFIG_H */

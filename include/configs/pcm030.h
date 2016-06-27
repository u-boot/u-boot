/*
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2006
 * Eric Schumann, Phytec Messatechnik GmbH
 *
 * (C) Copyright 2009
 * Jon Smirl <jonsmirl@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_BOARDINFO	 "phyCORE-MPC5200B-tiny"

/*-----------------------------------------------------------------------------
High Level Configuration Options
(easy to change)
-----------------------------------------------------------------------------*/
#define CONFIG_MPC5200		1	/* This is an MPC5200 CPU */
#define CONFIG_MPC5200_DDR	1	/* (with DDR-SDRAM) */
#define CONFIG_PHYCORE_MPC5200B_TINY 1	/* phyCORE-MPC5200B -> */
					/* FEC configuration and IDE */

/*
 * Valid values for CONFIG_SYS_TEXT_BASE are:
 * 0xFFF00000	boot high (standard configuration)
 * 0xFF000000	boot low
 * 0x00100000	boot from RAM (for testing only)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFF00000
#endif

#define CONFIG_SYS_MPC5XXX_CLKIN 33333333 /* ... running at 33.333333MHz */

/*-----------------------------------------------------------------------------
Serial console configuration
-----------------------------------------------------------------------------*/
#define CONFIG_PSC_CONSOLE	3	/* console is on PSC3 -> */
					/*define gps port conf. */
					/* register later on to */
					/*enable UART function! */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Command line configuration.
 */
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_PCI

#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */

#if (CONFIG_SYS_TEXT_BASE == 0xFF000000)	/* Boot low */
#define CONFIG_SYS_LOWBOOT 1
#endif
/* RAMBOOT will be defined automatically in memory section */

#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT 		"nor0=physmap-flash.0"
#define MTDPARTS_DEFAULT   	"mtdparts=physmap-flash.0:256k(ubootl)," \
	"1792k(kernel),13312k(jffs2),256k(uboot)ro,256k(oftree),-(space)"

#undef	CONFIG_BOOTARGS

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run bootcmd_net\" to load Kernel over TFTP and to "\
		"mount root filesystem over NFS;" \
	"echo"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"uimage=uImage-pcm030\0"					\
	"oftree=oftree-pcm030.dtb\0"					\
	"jffs2=root-pcm030.jffs2\0" 					\
	"uboot=u-boot-pcm030.bin\0"					\
	"bargs_base=setenv bootargs console=ttyPSC0,$(baudrate)"	\
		" $(mtdparts) rw\0" 					\
	"bargs_flash=setenv bootargs $(bootargs) root=/dev/mtdblock2"	\
		" rootfstype=jffs2\0" 					\
	"bargs_nfs=setenv bootargs $(bootargs) root=/dev/nfs"		\
		" ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)::"	\
		"$(netdev):off nfsroot=$(serverip):$(nfsrootfs),v3,tcp\0" \
	"bcmd_net=run bargs_base bargs_nfs; tftpboot 0x500000 $(uimage);" \
		" tftp 0x400000 $(oftree); bootm 0x500000 - 0x400000\0"	\
	"bcmd_flash=run bargs_base bargs_flash; bootm 0xff040000 - "	\
		"0xfff40000\0" 						\
		" cp.b 0x400000 0xff040000 $(filesize)\0" 		\
	"prg_jffs2=tftp 0x400000 $(jffs2); erase 0xff200000 0xffefffff; " \
		"cp.b 0x400000 0xff200000 $(filesize)\0" 		\
	"prg_oftree=tftp 0x400000 $(oftree); erase 0xfff40000 0xfff5ffff;" \
		" cp.b 0x400000 0xfff40000 $(filesize)\0" 		\
	"update=tftpboot 0x400000 $(uboot);erase 0xFFF00000 0xfff3ffff;" \
		" cp.b 0x400000 0xFFF00000 $(filesize)\0"		\
	"unlock=yes\0"							\
	""

#define CONFIG_BOOTCOMMAND		"run bcmd_flash"

/*--------------------------------------------------------------------------
IPB Bus clocking configuration.
 ---------------------------------------------------------------------------*/
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK	/* define for 133MHz speed */

/*-------------------------------------------------------------------------
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 * -----------------------------------------------------------------------*/
#define CONFIG_PCI			1
#define CONFIG_PCI_PNP			1
#define CONFIG_PCI_SCAN_SHOW		1
#define CONFIG_PCI_MEM_BUS		0x40000000
#define CONFIG_PCI_MEM_PHYS		CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE		0x10000000
#define CONFIG_PCI_IO_BUS		0x50000000
#define CONFIG_PCI_IO_PHYS		CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE		0x01000000
#define CONFIG_SYS_XLB_PIPELINING	1

/*---------------------------------------------------------------------------
 I2C configuration
---------------------------------------------------------------------------*/
#define CONFIG_HARD_I2C 1 /* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE 2 /* Select I2C module #1 or #2 */
#define CONFIG_SYS_I2C_SPEED 100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE 0x7F

/*---------------------------------------------------------------------------
 EEPROM CAT24WC32 configuration
---------------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x52	/* 1010100x */
#define CONFIG_SYS_I2C_FACT_ADDR	0x52	/* EEPROM CAT24WC32 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2	/* Bytes of address */
#define CONFIG_SYS_EEPROM_SIZE		2048
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 15

/*---------------------------------------------------------------------------
RTC configuration
---------------------------------------------------------------------------*/
#define RTC
#define CONFIG_RTC_PCF8563		1
#define CONFIG_SYS_I2C_RTC_ADDR		0x51

/*---------------------------------------------------------------------------
 Flash configuration
---------------------------------------------------------------------------*/

#define CONFIG_SYS_FLASH_BASE		0xff000000
#define CONFIG_SYS_FLASH_SIZE		0x01000000
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_MAX_FLASH_SECT 260 /* max num of sects on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS 1 /* max num of flash banks */
						/* (= chip selects) */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

/*
 * Use also hardware protection. This seems required, as the BDI uses
 * hardware protection. Without this, U-Boot can't work with this sectors,
 * as its protection is software only by default
 */
#define CONFIG_SYS_FLASH_PROTECTION	1

/*---------------------------------------------------------------------------
 Environment settings
---------------------------------------------------------------------------*/

/* pcm030 ships with environment is EEPROM by default */
#define CONFIG_ENV_IS_IN_EEPROM	1
#define CONFIG_ENV_OFFSET	0x00	/* environment starts at the */
					/*beginning of the EEPROM */
#define CONFIG_ENV_SIZE		CONFIG_SYS_EEPROM_SIZE

#define CONFIG_ENV_OVERWRITE	1

/*-----------------------------------------------------------------------------
  Memory map
-----------------------------------------------------------------------------*/
#define CONFIG_SYS_MBAR	0xF0000000	/* MBAR has to be switched by other */
					/* bootloader or debugger config */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR		0x80000000
/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_SIZE	MPC5XXX_SRAM_SIZE	/* Size of used */
								/* area in DPRAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#	define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN (192 << 10) /* Reserve 192 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN (128 << 10) /* Reserve 128 kB for malloc() */
#define CONFIG_SYS_BOOTMAPSZ (8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------------
 Ethernet configuration
-----------------------------------------------------------------------------*/
#define CONFIG_MPC5xxx_FEC		1
#define CONFIG_MPC5xxx_FEC_MII100
#define CONFIG_PHY_ADDR			0x01

/*---------------------------------------------------------------------------
 GPIO configuration
 ---------------------------------------------------------------------------*/

/* GPIO port configuration
 *
 * Pin mapping:
 *
 * [29:31] = 01x
 * PSC1_0 -> AC97 SDATA out
 * PSC1_1 -> AC97 SDTA in
 * PSC1_2 -> AC97 SYNC out
 * PSC1_3 -> AC97 bitclock out
 * PSC1_4 -> AC97 reset out
 *
 * [25:27] = 001
 * PSC2_0 -> CAN 1 Tx out
 * PSC2_1 -> CAN 1 Rx in
 * PSC2_2 -> CAN 2 Tx out
 * PSC2_3 -> CAN 2 Rx in
 * PSC2_4 -> GPIO (claimed for ATA reset, active low)
 *
 *
 * [20:23] = 1100
 * PSC3_0 -> UART Tx out
 * PSC3_1 -> UART Rx in
 * PSC3_2 -> UART RTS (in/out FIXME)
 * PSC3_3 -> UART CTS (in/out FIXME)
 * PSC3_4 -> LocalPlus Bus CS6 \
 * PSC3_5 -> LocalPlus Bus CS7 / --> see [4] and [5]
 * PSC3_6 -> dedicated SPI MOSI out (master case)
 * PSC3_7 -> dedicated SPI MISO in (master case)
 * PSC3_8 -> dedicated SPI SS out (master case)
 * PSC3_9 -> dedicated SPI CLK out (master case)
 *
 * [18:19] = 01
 * USB_0 -> USB OE out
 * USB_1 -> USB Tx- out
 * USB_2 -> USB Tx+ out
 * USB_3 -> USB RxD (in/out FIXME)
 * USB_4 -> USB Rx+ in
 * USB_5 -> USB Rx- in
 * USB_6 -> USB PortPower out
 * USB_7 -> USB speed out
 * USB_8 -> USB suspend (in/out FIXME)
 * USB_9 -> USB overcurrent in
 *
 * [17] = 0
 * USB differential mode
 *
 * [16] = 0
 * PCI enabled
 *
 * [12:15] = 0101
 * ETH_0 -> ETH Txen
 * ETH_1 -> ETH TxD0
 * ETH_2 -> ETH TxD1
 * ETH_3 -> ETH TxD2
 * ETH_4 -> ETH TxD3
 * ETH_5 -> ETH Txerr
 * ETH_6 -> ETH MDC
 * ETH_7 -> ETH MDIO
 * ETH_8 -> ETH RxDv
 * ETH_9 -> ETH RxCLK
 * ETH_10 -> ETH Collision
 * ETH_11 -> ETH TxD
 * ETH_12 -> ETH RxD0
 * ETH_13 -> ETH RxD1
 * ETH_14 -> ETH RxD2
 * ETH_15 -> ETH RxD3
 * ETH_16 -> ETH Rxerr
 * ETH_17 -> ETH CRS
 *
 * [9:11] = 101
 * PSC6_0 -> UART RxD in
 * PSC6_1 -> UART CTS (in/out FIXME)
 * PSC6_2 -> UART TxD out
 * PSC6_3 -> UART RTS (in/out FIXME)
 *
 * [2:3/6:7] = 00/11
 * TMR_0 -> ATA_CS0 out
 * TMR_1 -> ATA_CS1 out
 * TMR_2 -> GPIO
 * TMR_3 -> GPIO
 * TMR_4 -> GPIO
 * TMR_5 -> GPIO
 * TMR_6 -> GPIO
 * TMR_7 -> GPIO
 * I2C_0 -> I2C 1 Clock out
 * I2C_1 -> I2C 1 IO in/out
 * I2C_2 -> I2C 2 Clock out
 * I2C_3 -> I2C 2 IO in/out
 *
 * [4] = 1
 * PSC3_5 is used as CS7
 *
 * [5] = 1
 * PSC3_4 is used as CS6
 *
 * [1] = 0
 * gpio_wkup_7 is GPIO
 *
 * [0] = 0
 * gpio_wkup_6 is GPIO
 *
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x0f551c12

/*-----------------------------------------------------------------------------
 Miscellaneous configurable options
-------------------------------------------------------------------------------*/
#define CONFIG_SYS_LONGHELP	/* undef to save memory */

#define CONFIG_CMDLINE_EDITING 1 /* add command line history */

#define CONFIG_SYS_CACHELINE_SIZE 32 /* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT 5 /* log base 2 of the above value */
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE 256 /* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
							/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS 16 /* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START 0x00100000 /* memtest works on */
#define CONFIG_SYS_MEMTEST_END 0x00f00000 /* 1 ... 15 MB in DRAM */

#define CONFIG_SYS_LOAD_ADDR 0x400000 /* default load address */

#define CONFIG_DISPLAY_BOARDINFO 1

/*-----------------------------------------------------------------------------
 Various low-level settings
-----------------------------------------------------------------------------*/
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

/* no burst access on the LPB */
#define CONFIG_SYS_CS_BURST		0x00000000
/* one deadcycle for the 33MHz statemachine */
#define CONFIG_SYS_CS_DEADCYCLE		0x33333331
/* one additional waitstate for the 33MHz statemachine */
#define CONFIG_SYS_BOOTCS_CFG		0x0001dd00
#define CONFIG_SYS_BOOTCS_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE

#define CONFIG_SYS_RESET_ADDRESS 	0xff000000

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK		0x0001BBBB
#define CONFIG_USB_CONFIG		0x00001000

/*---------------------------------------------------------------------------
 IDE/ATA stuff Supports IDE harddisk
----------------------------------------------------------------------------*/

#undef  CONFIG_IDE_8xx_PCCARD	/* Use IDE with PC Card Adapter */
#undef	CONFIG_IDE_8xx_DIRECT	/* Direct IDE not supported */
#undef	CONFIG_IDE_LED		/* LED for ide not supported */
#define CONFIG_SYS_ATA_CS_ON_TIMER01
#define	CONFIG_IDE_RESET 1	/* reset for ide supported */
#define CONFIG_IDE_PREINIT
#define CONFIG_SYS_IDE_MAXBUS 1 /* max. 1 IDE bus */
#define CONFIG_SYS_IDE_MAXDEVICE 2 /* max. 2 drives per IDE bus */
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA
/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)
/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)
/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)
/* Interval between registers */
#define CONFIG_SYS_ATA_STRIDE		4
#define CONFIG_ATAPI			1

/* we enable IDE and FAT support, so we also need partition support */
#define CONFIG_DOS_PARTITION 1

/* USB */
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/* pass open firmware flat tree */
#define OF_CPU				"PowerPC,5200@0"
#define OF_TBCLK			CONFIG_SYS_MPC5XXX_CLKIN
#define OF_SOC				"soc5200@f0000000"
#define OF_STDOUT_PATH			"/soc5200@f0000000/serial@2400"

#endif /* __CONFIG_H */

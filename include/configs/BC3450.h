/*
 * -- Version 1.1 --
 *
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2005
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
 *
 * (C) Copyright 2005
 * Stefan Strobl, GERSYS GmbH, stefan.strobl@gersys.de.
 *
 * History:
 *	1.1 - add define CONFIG_ZERO_BOOTDELAY_CHECK
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU	    */
#define CONFIG_MPC5200		1	/* (more precisely a MPC5200 CPU)   */
#define CONFIG_TQM5200		1	/* ... on a TQM5200 module	    */

#define CONFIG_BC3450		1	/* ... on a BC3450 mainboard	    */
#define CONFIG_BC3450_PS2	1	/*  + a PS/2 converter onboard	    */
#define CONFIG_BC3450_IDE	1	/*  + IDE drives (Compact Flash)    */
#define CONFIG_BC3450_USB	1	/*  + USB support		    */
# define CONFIG_FAT		1	/*    + FAT support		    */
# define CONFIG_EXT2		1	/*    + EXT2 support		    */
#undef CONFIG_BC3450_BUZZER		/*  + Buzzer onboard		    */
#undef CONFIG_BC3450_CAN		/*  + CAN transceiver		    */
#undef CONFIG_BC3450_DS1340		/*  + a RTC DS1340 onboard	    */
#undef CONFIG_BC3450_DS3231		/*  + a RTC DS3231 onboard	tbd */
#undef CONFIG_BC3450_AC97		/*  + AC97 on PSC2,		tbd */
#define CONFIG_BC3450_FP	1	/*  + enable FP O/P		    */
#undef CONFIG_BC3450_CRT		/*  + enable CRT O/P (Debug only!)  */

/*
 * Valid values for CONFIG_SYS_TEXT_BASE are:
 * 0xFC000000	boot low (standard configuration with room for
 *		max 64 MByte Flash ROM)
 * 0x00100000	boot from RAM (for testing only)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFC000000
#endif

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz	    */

#define CONFIG_HIGH_BATS	1	/* High BATs supported		    */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1		*/
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps		*/
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * AT-PS/2 Multiplexer
 */
#ifdef CONFIG_BC3450_PS2
# define CONFIG_PS2KBD			/* AT-PS/2 Keyboard		*/
# define CONFIG_PS2MULT			/* .. on PS/2 Multiplexer	*/
# define CONFIG_PS2SERIAL	6		/* .. on PSC6		*/
# define CONFIG_PS2MULT_DELAY	(CONFIG_SYS_HZ/2)	/* Initial delay	*/
# define CONFIG_BOARD_EARLY_INIT_R
#endif /* CONFIG_BC3450_PS2 */

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
# define CONFIG_PCI		1
# define CONFIG_PCI_PNP		1
/* #define CONFIG_PCI_SCAN_SHOW 1 */
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

/*#define CONFIG_EEPRO100	XXX - FIXME: conflicts when CONFIG_MII is enabled */
#define CONFIG_SYS_RX_ETH_BUFFER	8	/* use 8 rx buffer on eepro100	*/
#define CONFIG_NS8382X		1

/*
 * Video console
 */
# define CONFIG_VIDEO
# define CONFIG_VIDEO_SM501
# define CONFIG_VIDEO_SM501_32BPP
# define CONFIG_CFB_CONSOLE
# define CONFIG_VIDEO_LOGO
# define CONFIG_VGA_AS_SINGLE_DEVICE
# define CONFIG_CONSOLE_EXTRA_INFO	/* display Board/Device-Infos */
# define CONFIG_VIDEO_SW_CURSOR
# define CONFIG_SPLASH_SCREEN
# define CONFIG_SYS_CONSOLE_IS_IN_ENV

/*
 * Partitions
 */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

/*
 * USB
 */
#ifdef CONFIG_BC3450_USB
# define CONFIG_USB_OHCI
# define CONFIG_USB_STORAGE
#endif /* CONFIG_BC3450_USB */

/*
 * POST support
 */
#define CONFIG_POST		(CONFIG_SYS_POST_MEMORY   | \
				 CONFIG_SYS_POST_CPU	   | \
				 CONFIG_SYS_POST_I2C)

#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
# define MPC5XXX_SRAM_POST_SIZE MPC5XXX_SRAM_SIZE-4
#endif /* CONFIG_POST */


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

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_BSP

#ifdef CONFIG_VIDEO
    #define CONFIG_CMD_BMP
#endif

#ifdef CONFIG_BC3450_IDE
    #define CONFIG_CMD_IDE
#endif

#if defined(CONFIG_BC3450_IDE) || defined(CONFIG_BC3450_USB)
    #ifdef CONFIG_FAT
	#define CONFIG_CMD_FAT
    #endif

    #ifdef CONFIG_EXT2
	#define CONFIG_CMD_EXT2
    #endif
#endif

#ifdef CONFIG_BC3450_USB
    #define CONFIG_CMD_USB
#endif

#ifdef CONFIG_PCI
    #define CONFIG_CMD_PCI
#endif

#ifdef CONFIG_POST
    #define CONFIG_CMD_DIAG
#endif


#define CONFIG_TIMESTAMP		/* display image timestamps */

#if (CONFIG_SYS_TEXT_BASE == 0xFC000000)		/* Boot low */
#   define CONFIG_SYS_LOWBOOT		1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo;"

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"ipaddr=192.168.1.10\0"						\
	"serverip=192.168.1.3\0"					\
	"netmask=255.255.255.0\0"					\
	"hostname=bc3450\0"						\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"kernel_addr=fc0a0000\0"					\
	"ramdisk_addr=fc1c0000\0"					\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=$(serverip):$(rootpath)\0"			\
	"ideargs=setenv bootargs root=/dev/hda2 ro\0"			\
	"addip=setenv bootargs $(bootargs) "				\
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"	\
		":$(hostname):$(netdev):off panic=1\0"			\
	"addcons=setenv bootargs $(bootargs) "				\
		"console=ttyS0,$(baudrate) console=tty0\0"		\
	"flash_self=run ramargs addip addcons;"				\
		"bootm $(kernel_addr) $(ramdisk_addr)\0"		\
	"flash_nfs=run nfsargs addip addcons; bootm $(kernel_addr)\0"	\
	"net_nfs=tftp 200000 $(bootfile); "				\
		"run nfsargs addip addcons; bootm\0"			\
	"ide_nfs=run nfsargs addip addcons; "				\
		"disk 200000 0:1; bootm\0"				\
	"ide_ide=run ideargs addip addcons; "				\
		"disk 200000 0:1; bootm\0"				\
	"usb_self=run usbload; run ramargs addip addcons; "		\
		"bootm 200000 400000\0"					\
	"usbload=usb reset; usb scan; usbboot 200000 0:1; "		\
		"usbboot 400000 0:2\0"					\
	"bootfile=uImage\0"						\
	"load=tftp 200000 $(u-boot)\0"					\
	"u-boot=u-boot.bin\0"						\
	"update=protect off FC000000 FC05FFFF;"				\
		"erase FC000000 FC05FFFF;"				\
		"cp.b 200000 FC000000 $(filesize);"			\
		"protect on FC000000 FC05FFFF\0"			\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

/*
 * IPB Bus clocking configuration.
 */
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * PCI Bus clocking configuration
 *
 * Actually a PCI Clock of 66 MHz is only set (in cpu_init.c) if
 * CONFIG_SYS_IPBCLK_EQUALS_XLBCLK is defined. This is because a PCI Clock
 *  of 66 MHz yet hasn't been tested with a IPB Bus Clock of 66 MHz.
 */
#if defined(CONFIG_SYS_IPBCLK_EQUALS_XLBCLK)
# define CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2	/* define for 66MHz speed */
#endif

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE		2	/* Select I2C module #2 */

/*
 * I2C clock frequency
 *
 * Please notice, that the resulting clock frequency could differ from the
 * configured value. This is because the I2C clock is derived from system
 * clock over a frequency divider with only a few divider values. U-boot
 * calculates the best approximation for CONFIG_SYS_I2C_SPEED. However the calculated
 * approximation allways lies below the configured value, never above.
 */
#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration for I²C EEPROM M24C32
 * M24C64 should work also. For other EEPROMs config should be verified.
 *
 * The TQM5200 module may hold an EEPROM at address 0x50.
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x (TQM) */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5	/* =32 Bytes per write */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * RTC configuration
 */
#if defined (CONFIG_BC3450_DS1340) && !defined (CONFIG_BC3450_DS3231)
# define CONFIG_RTC_M41T11	1
# define CONFIG_SYS_I2C_RTC_ADDR	0x68
#else
# define CONFIG_RTC_MPC5200	1	/* use MPC5200 internal RTC */
# define CONFIG_BOARD_EARLY_INIT_R
#endif

/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_TEXT_BASE /* 0xFC000000 */

/* use CFI flash driver if no module variant is spezified */
#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_BOOTCS_START }
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_SIZE		0x04000000 /* 64 MByte */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max num of sects on one chip */
#undef CONFIG_SYS_FLASH_USE_BUFFER_WRITE	/* not supported yet for AMD */

#if !defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00760000 + 0x00800000)
#else	/* CONFIG_SYS_LOWBOOT */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00060000)
#endif	/* CONFIG_SYS_LOWBOOT */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of flash banks
					   (= chip selects) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

/* Dynamic MTD partition support */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=TQM5200-0"
#define MTDPARTS_DEFAULT	"mtdparts=TQM5200-0:640k(firmware),"	\
						"1408k(kernel),"	\
						"2m(initrd),"		\
						"4m(small-fs),"		\
						"16m(big-fs),"		\
						"8m(misc)"

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

/* Use ON-Chip SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
# define CONFIG_SYS_INIT_RAM_SIZE	MPC5XXX_SRAM_POST_SIZE
#else
# define CONFIG_SYS_INIT_RAM_SIZE	MPC5XXX_SRAM_SIZE
#endif /*CONFIG_POST*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(384 << 10) /* Reserve 384 kB for Monitor   */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10) /* Reserve 128 kB for malloc()  */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)   /* Initial Memory map for Linux */

/*
 * Ethernet configuration
 *
 * Define CONFIG_MPC5xxx_MII10 to force FEC at 10MBIT
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
#undef CONFIG_MPC5xxx_MII10
#define CONFIG_PHY_ADDR		0x00

/*
 * GPIO configuration on BC3450
 *
 *  PSC1:   UART1 (Service-UART)	 [0x xxxxxxx4]
 *  PSC2:   UART2			 [0x xxxxxx4x]
 *    or:   AC/97 if CONFIG_BC3450_AC97	 [0x xxxxxx2x]
 *  PSC3:   USB2			 [0x xxxxx1xx]
 *  USB:    UART4(ext.)/UART5(int.)	 [0x xxxx2xxx]
 *	      (this has to match
 *	      CONFIG_USB_CONFIG which is
 *	      used by usb_ohci.c to set
 *	      the USB ports)
 *  Eth:    10/100Mbit Ethernet		 [0x xxx0xxxx]
 *	      (this is reset to '5'
 *	      in FEC driver: fec.c)
 *  PSC6:   UART6 (int. to PS/2 contr.)	 [0x xx5xxxxx]
 *  ATA/CS: ???				 [0x x1xxxxxx]
 *	    FIXME! UM Fig 2-10 suggests	 [0x x0xxxxxx]
 *  CS1:    Use Pin gpio_wkup_6 as second
 *	    SDRAM chip select (mem_cs1)
 *  Timer:  CAN2 / SPI
 *  I2C:    CAN1 / I²C2		  [0x bxxxxxxx]
 */
#ifdef CONFIG_BC3450_AC97
# define CONFIG_SYS_GPS_PORT_CONFIG	0xb1502124
#else /* PSC2=UART2 */
# define CONFIG_SYS_GPS_PORT_CONFIG	0xb1502144
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"=> "		/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size  */
#define CONFIG_SYS_MAXARGS		16		/* max no of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Arg. Buffer Size    */

#define CONFIG_SYS_ALT_MEMTEST				/* Enable an alternative,   */
						/*  more extensive mem test */

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on	    */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	    */

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	    */

#define CONFIG_SYS_HZ			1000		/* dec freq: 1ms ticks	    */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs		    */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value    */
#endif

/*
 * Enable loopw command.
 */
#define CONFIG_LOOPW

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#ifdef CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2
# define CONFIG_SYS_BOOTCS_CFG		0x0008DF30	/* for pci_clk	= 66 MHz */
#else
# define CONFIG_SYS_BOOTCS_CFG		0x0004DF30	/* for pci_clk = 33 MHz	 */
#endif
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

/* automatic configuration of chip selects */
#ifdef CONFIG_TQM5200
# define CONFIG_LAST_STAGE_INIT
#endif /* CONFIG_TQM5200 */

/*
 * SRAM - Do not map below 2 GB in address space, because this area is used
 * for SDRAM autosizing.
 */
#ifdef CONFIG_TQM5200
# define CONFIG_SYS_CS2_START		0xE5000000
# define CONFIG_SYS_CS2_SIZE		0x100000	/* 1 MByte */
# define CONFIG_SYS_CS2_CFG		0x0004D930
#endif /* CONFIG_TQM5200 */

/*
 * Grafic controller - Do not map below 2 GB in address space, because this
 * area is used for SDRAM autosizing.
 */
#ifdef CONFIG_TQM5200
# define SM501_FB_BASE		0xE0000000
# define CONFIG_SYS_CS1_START		(SM501_FB_BASE)
# define CONFIG_SYS_CS1_SIZE		0x4000000	/* 64 MByte */
# define CONFIG_SYS_CS1_CFG		0x8F48FF70
# define SM501_MMIO_BASE	CONFIG_SYS_CS1_START + 0x03E00000
#endif /* CONFIG_TQM5200 */

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x33333311	/* 1 dead cycle for	*/
						/*  flash and SM501	*/

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/*
 * USB stuff
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00002000	/* we're using Port 2	*/

/*
 * IDE/ATA stuff Supports IDE harddisk
 */
#undef	CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card Adapter */

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	  not supported */
#undef	CONFIG_IDE_LED			/* LED for ide	  not supported */

#define CONFIG_IDE_RESET		/* reset for ide      supported */
#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	2	/* max. 2 drives per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O */
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses */
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)

/* Offset for alternate registers */
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)

/* Interval between registers */
#define CONFIG_SYS_ATA_STRIDE		4

#endif /* __CONFIG_H */

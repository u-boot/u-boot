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

#define CFG_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz	    */

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot		    */

#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs		    */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value    */
#endif

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1		*/
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps		*/
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * AT-PS/2 Multiplexer
 */
#ifdef CONFIG_BC3450_PS2
# define CONFIG_PS2KBD			/* AT-PS/2 Keyboard		*/
# define CONFIG_PS2MULT			/* .. on PS/2 Multiplexer	*/
# define CONFIG_PS2SERIAL	6		/* .. on PSC6		*/
# define CONFIG_PS2MULT_DELAY	(CFG_HZ/2)	/* Initial delay	*/
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

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CONFIG_NET_MULTI	1
/*#define CONFIG_EEPRO100	XXX - FIXME: conflicts when CONFIG_MII is enabled */
#define CFG_RX_ETH_BUFFER	8	/* use 8 rx buffer on eepro100	*/
#define CONFIG_NS8382X		1

#ifdef CONFIG_PCI
# define ADD_PCI_CMD		CFG_CMD_PCI
#else
# define ADD_PCI_CMD		0
#endif

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
# define CFG_CONSOLE_IS_IN_ENV

#ifdef CONFIG_VIDEO
# define ADD_BMP_CMD		CFG_CMD_BMP
#else
# define ADD_BMP_CMD		0
#endif

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
# define ADD_USB_CMD		CFG_CMD_USB
# define CONFIG_USB_STORAGE
#else /* !CONFIG_BC3450_USB */
# define ADD_USB_CMD		0
#endif /* CONFIG_BC3450_USB */

/*
 * POST support
 */
#define CONFIG_POST		(CFG_POST_MEMORY   | \
				 CFG_POST_CPU	   | \
				 CFG_POST_I2C)

#ifdef CONFIG_POST
# define CFG_CMD_POST_DIAG CFG_CMD_DIAG
/* preserve space for the post_word at end of on-chip SRAM */
# define MPC5XXX_SRAM_POST_SIZE MPC5XXX_SRAM_SIZE-4
#else
# define CFG_CMD_POST_DIAG 0
#endif /* CONFIG_POST */

/*
 * IDE
 */
#ifdef CONFIG_BC3450_IDE
# define ADD_IDE_CMD		CFG_CMD_IDE
#else
# define ADD_IDE_CMD		0
#endif /* CONFIG_BC3450_IDE */

/*
 * Filesystem support
 */
#if defined (CONFIG_BC3450_IDE) || defined (CONFIG_BC3450_USB)
#ifdef CONFIG_FAT
# define ADD_FAT_CMD		CFG_CMD_FAT
#else
# define ADD_FAT_CMD		0
#endif /* CONFIG_FAT */

#ifdef CONFIG_EXT2
# define ADD_EXT2_CMD		CFG_CMD_EXT2
#else
# define ADD_EXT2_CMD		0
#endif /* CONFIG_EXT2 */
#endif /* CONFIG_BC3450_IDE / _USB */

/*
 * Supported commands
 */
#define CONFIG_COMMANDS	       (CONFIG_CMD_DFL	| \
				ADD_BMP_CMD	| \
				ADD_IDE_CMD	| \
				ADD_FAT_CMD	| \
				ADD_EXT2_CMD	| \
				ADD_PCI_CMD	| \
				ADD_USB_CMD	| \
				CFG_CMD_ASKENV	| \
				CFG_CMD_DATE	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_ECHO	| \
				CFG_CMD_EEPROM	| \
				CFG_CMD_I2C	| \
				CFG_CMD_JFFS2	| \
				CFG_CMD_MII	| \
				CFG_CMD_NFS	| \
				CFG_CMD_PING	| \
				CFG_CMD_POST_DIAG | \
				CFG_CMD_REGINFO | \
				CFG_CMD_SNTP	| \
				CFG_CMD_BSP)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_TIMESTAMP		/* display image timestamps */

#if (TEXT_BASE == 0xFC000000)		/* Boot low */
#   define CFG_LOWBOOT		1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
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
#define CFG_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * PCI Bus clocking configuration
 *
 * Actually a PCI Clock of 66 MHz is only set (in cpu_init.c) if
 * CFG_IPBCLK_EQUALS_XLBCLK is defined. This is because a PCI Clock
 *  of 66 MHz yet hasn't been tested with a IPB Bus Clock of 66 MHz.
 */
#if defined(CFG_IPBCLK_EQUALS_XLBCLK)
# define CFG_PCICLK_EQUALS_IPBCLK_DIV2	/* define for 66MHz speed */
#endif

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_MODULE		2	/* Select I2C module #2 */

/*
 * I2C clock frequency
 *
 * Please notice, that the resulting clock frequency could differ from the
 * configured value. This is because the I2C clock is derived from system
 * clock over a frequency divider with only a few divider values. U-boot
 * calculates the best approximation for CFG_I2C_SPEED. However the calculated
 * approximation allways lies below the configured value, never above.
 */
#define CFG_I2C_SPEED		100000 /* 100 kHz */
#define CFG_I2C_SLAVE		0x7F

/*
 * EEPROM configuration for I²C EEPROM M24C32
 * M24C64 should work also. For other EEPROMs config should be verified.
 *
 * The TQM5200 module may hold an EEPROM at address 0x50.
 */
#define CFG_I2C_EEPROM_ADDR		0x50	/* 1010000x (TQM) */
#define CFG_I2C_EEPROM_ADDR_LEN		2
#define CFG_EEPROM_PAGE_WRITE_BITS	5	/* =32 Bytes per write */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * RTC configuration
 */
#if defined (CONFIG_BC3450_DS1340) && !defined (CONFIG_BC3450_DS3231)
# define CONFIG_RTC_M41T11	1
# define CFG_I2C_RTC_ADDR	0x68
#else
# define CONFIG_RTC_MPC5200	1	/* use MPC5200 internal RTC */
# define CONFIG_BOARD_EARLY_INIT_R
#endif

/*
 * Flash configuration
 */
#define CFG_FLASH_BASE		TEXT_BASE /* 0xFC000000 */

/* use CFI flash driver if no module variant is spezified */
#define CFG_FLASH_CFI		1	/* Flash is CFI conformant */
#define CFG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#define CFG_FLASH_BANKS_LIST	{ CFG_BOOTCS_START }
#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_SIZE		0x04000000 /* 64 MByte */
#define CFG_MAX_FLASH_SECT	512	/* max num of sects on one chip */
#undef CFG_FLASH_USE_BUFFER_WRITE	/* not supported yet for AMD */

#if !defined(CFG_LOWBOOT)
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x00760000 + 0x00800000)
#else	/* CFG_LOWBOOT */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x00060000)
#endif	/* CFG_LOWBOOT */
#define CFG_MAX_FLASH_BANKS	1	/* max num of flash banks
					   (= chip selects) */
#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

/* Dynamic MTD partition support */
#define CONFIG_JFFS2_CMDLINE
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
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x10000
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000

/* Use ON-Chip SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
# define CFG_INIT_RAM_END	MPC5XXX_SRAM_POST_SIZE
#else
# define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE
#endif /*CONFIG_POST*/

#define CFG_GBL_DATA_SIZE	128	/* Bytes reserved for initial data  */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE	TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(384 << 10) /* Reserve 384 kB for Monitor   */
#define CFG_MALLOC_LEN		(128 << 10) /* Reserve 128 kB for malloc()  */
#define CFG_BOOTMAPSZ		(8 << 20)   /* Initial Memory map for Linux */

/*
 * Ethernet configuration
 *
 * Define CONFIG_FEC10MBIT to force FEC at 10MBIT
 */
#define CONFIG_MPC5xxx_FEC	1
#undef CONFIG_FEC_10MBIT
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
# define CFG_GPS_PORT_CONFIG	0xb1502124
#else /* PSC2=UART2 */
# define CFG_GPS_PORT_CONFIG	0xb1502144
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory	    */
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt   */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256		/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size  */
#define CFG_MAXARGS		16		/* max no of command args   */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Arg. Buffer Size    */

#define CFG_ALT_MEMTEST				/* Enable an alternative,   */
						/*  more extensive mem test */

#define CFG_MEMTEST_START	0x00100000	/* memtest works on	    */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	    */

#define CFG_LOAD_ADDR		0x100000	/* default load address	    */

#define CFG_HZ			1000		/* dec freq: 1ms ticks	    */

/*
 * Enable loopw commando. This has only affect, if CFG_CMD_MEM is defined,
 * which is normally part of the default commands (CFV_CMD_DFL)
 */
#define CONFIG_LOOPW

/*
 * Various low-level settings
 */
#if defined(CONFIG_MPC5200)
# define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
# define CFG_HID0_FINAL		HID0_ICE
#else
# define CFG_HID0_INIT		0
# define CFG_HID0_FINAL		0
#endif

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#ifdef CFG_PCICLK_EQUALS_IPBCLK_DIV2
# define CFG_BOOTCS_CFG		0x0008DF30	/* for pci_clk	= 66 MHz */
#else
# define CFG_BOOTCS_CFG		0x0004DF30	/* for pci_clk = 33 MHz	 */
#endif
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

/* automatic configuration of chip selects */
#ifdef CONFIG_TQM5200
# define CONFIG_LAST_STAGE_INIT
#endif /* CONFIG_TQM5200 */

/*
 * SRAM - Do not map below 2 GB in address space, because this area is used
 * for SDRAM autosizing.
 */
#ifdef CONFIG_TQM5200
# define CFG_CS2_START		0xE5000000
# define CFG_CS2_SIZE		0x100000	/* 1 MByte */
# define CFG_CS2_CFG		0x0004D930
#endif /* CONFIG_TQM5200 */

/*
 * Grafic controller - Do not map below 2 GB in address space, because this
 * area is used for SDRAM autosizing.
 */
#ifdef CONFIG_TQM5200
# define SM501_FB_BASE		0xE0000000
# define CFG_CS1_START		(SM501_FB_BASE)
# define CFG_CS1_SIZE		0x4000000	/* 64 MByte */
# define CFG_CS1_CFG		0x8F48FF70
# define SM501_MMIO_BASE	CFG_CS1_START + 0x03E00000
#endif /* CONFIG_TQM5200 */

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333311	/* 1 dead cycle for	*/
						/*  flash and SM501	*/

#define CFG_RESET_ADDRESS	0xff000000

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

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	2	/* max. 2 drives per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O */
#define CFG_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses */
#define CFG_ATA_REG_OFFSET	(CFG_ATA_DATA_OFFSET)

/* Offset for alternate registers */
#define CFG_ATA_ALT_OFFSET	(0x005C)

/* Interval between registers */
#define CFG_ATA_STRIDE		4

#endif /* __CONFIG_H */

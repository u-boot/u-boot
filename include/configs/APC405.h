/*
 * (C) Copyright 2005-2008
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
 *
 * (C) Copyright 2001-2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family   */
#define CONFIG_APCG405		1	/* ...on a APC405 board		*/

#define	CONFIG_SYS_TEXT_BASE	0xFFF80000

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_BOARD_EARLY_INIT_R 1
#define CONFIG_MISC_INIT_R      1       /* call misc_init_r()           */

#define CONFIG_SYS_CLK_FREQ     33333400 /* external frequency to pll   */

#define CONFIG_BOARD_TYPES	1	/* support board types		*/

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	1	/* autoboot after 3 seconds	*/
#define CONFIG_BOOTCOUNT_LIMIT	1

#undef	CONFIG_BOOTARGS

#define CONFIG_SYS_USB_LOAD_COMMAND	"fatload usb 0 200000 pImage;"		\
				"fatload usb 0 300000 pImage.initrd"
#define CONFIG_SYS_USB_SELF_COMMAND	"usb start;run usb_load;usb stop;"	\
				"run ramargs addip addcon usbargs;"	\
				"bootm 200000 300000"
#define CONFIG_SYS_USB_ARGS		"setenv bootargs $(bootargs) usbboot=1"
#define CONFIG_SYS_BOOTLIMIT		"3"
#define CONFIG_SYS_ALT_BOOTCOMMAND	"run usb_self;reset"

#define CONFIG_EXTRA_ENV_SETTINGS                                       \
	"hostname=abg405\0"                                             \
	"bd_type=abg405\0"                                              \
	"serial#=AA0000\0"                                              \
	"kernel_addr=fe000000\0"                                        \
	"ramdisk_addr=fe100000\0"                                       \
	"ramargs=setenv bootargs root=/dev/ram rw\0"                    \
	"nfsargs=setenv bootargs root=/dev/nfs rw "                     \
	"nfsroot=$(serverip):$(rootpath)\0"				\
	"addip=setenv bootargs $(bootargs) "                            \
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"	\
		":$(hostname)::off panic=1\0"				\
	"addcon=setenv bootargs $(bootargs) console=ttyS0,$(baudrate)"  \
		" $(optargs)\0"                                         \
	"flash_self=run ramargs addip addcon;"                          \
		"bootm $(kernel_addr) $(ramdisk_addr)\0"                \
	"net_nfs=tftp 200000 $(img);run nfsargs addip addcon;"          \
		"bootm\0"                                               \
	"rootpath=/tftpboot/abg405/target_root\0"                       \
	"img=/tftpboot/abg405/pImage\0"                                 \
	"load=tftp 100000 /tftpboot/abg405/u-boot.bin\0"		\
	"update=protect off fff80000 ffffffff;era fff80000 ffffffff;"   \
		"cp.b 100000 fff80000 80000\0"                          \
	"ipaddr=10.0.111.111\0"                                         \
	"netmask=255.255.0.0\0"                                         \
	"serverip=10.0.0.190\0"						\
	"splashimage=ffe80000\0"                                        \
	"usb_load="CONFIG_SYS_USB_LOAD_COMMAND"\0"				\
	"usb_self="CONFIG_SYS_USB_SELF_COMMAND"\0"				\
	"usbargs="CONFIG_SYS_USB_ARGS"\0"					\
	"bootlimit="CONFIG_SYS_BOOTLIMIT"\0"					\
	"altbootcmd="CONFIG_SYS_ALT_BOOTCOMMAND"\0"				\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self;reset"

#define CONFIG_ETHADDR		00:02:27:8e:00:00

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#undef  CONFIG_HAS_ETH1

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_LXT971_NO_SLEEP	1
#define CONFIG_RESET_PHY_R	1	/* use reset_phy() */

#define CONFIG_PHY_CLK_FREQ	EMAC_STACR_CLK_66MHZ /* 66 MHz OPB clock*/

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

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_USB

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define CONFIG_SUPPORT_VFAT

#define CONFIG_AUTO_UPDATE	1	/* autoupdate via CF or USB */

#undef  CONFIG_WATCHDOG			/* watchdog disabled */

#define CONFIG_RTC_MC146818		/* DS1685 is MC146818 compatible*/
#define CONFIG_SYS_RTC_REG_BASE_ADDR 0xF0000500 /* RTC Base Address */

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0 */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_CMDLINE_EDITING	1	/* add command line history */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_DEVICE_NULLDEV	1	/* include nulldev device */

#define CONFIG_SYS_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CONFIG_SYS_MEMTEST_START	0x0400000 /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM */

#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#define CONFIG_SYS_EXT_SERIAL_CLOCK    14745600 /* use external serial clock   */

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE      \
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, \
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

/* Only interrupt boot if space is pressed */
/* If a long serial cable is connected but */
/* other end is dead, garbage will be read */
#define CONFIG_AUTOBOOT_KEYED	1
#define CONFIG_AUTOBOOT_PROMPT	\
	"Press SPACE to abort autoboot in %d seconds\n", bootdelay
#undef CONFIG_AUTOBOOT_DELAY_STR
#define CONFIG_AUTOBOOT_STOP_STR " "

#define CONFIG_VERSION_VARIABLE	1	/* include version env variable */

#define CONFIG_SYS_RX_ETH_BUFFER	16	/* use 16 rx buffer on 405 emac */

/*
 * PCI stuff
 */
#define PCI_HOST_ADAPTER	0	/* configure as pci adapter     */
#define PCI_HOST_FORCE		1	/* configure as pci host        */
#define PCI_HOST_AUTO		2	/* detected via arbiter enable  */

#define CONFIG_PCI			/* include pci support          */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function     */
#define CONFIG_PCI_PNP			/* do pci plug-and-play         */
					/* resource configuration       */

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup  */
#define CONFIG_PCI_SKIP_HOST_BRIDGE 1
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x12FE  /* PCI Vendor ID: esd gmbh      */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0405  /* PCI Device ID: CPCI-405      */
#define CONFIG_SYS_PCI_CLASSCODE       0x0b20  /* PCI Class Code: Processor/PPC*/
#define CONFIG_SYS_PCI_PTM1LA  0x00000000      /* point to sdram               */
#define CONFIG_SYS_PCI_PTM1MS  0xfc000001      /* 64MB, enable hard-wired to 1 */
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CONFIG_SYS_PCI_PTM2LA  0xffc00000      /* point to flash               */
#define CONFIG_SYS_PCI_PTM2MS  0xffc00001      /* 4MB, enable                  */
#define CONFIG_SYS_PCI_PTM2PCI 0x04000000      /* Host: use this pci address   */

/*
 * IDE/ATA stuff
 */
#undef  CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef  CONFIG_IDE_LED			/* no led for ide supported */
#define CONFIG_IDE_RESET	1	/* reset for ide supported */

#define CONFIG_SYS_IDE_MAXBUS		1		/* max. 1 IDE busses */
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS) /* max. 1 drives per IDE bus */

#define CONFIG_SYS_ATA_BASE_ADDR	0xF0100000
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O */
#define CONFIG_SYS_ATA_REG_OFFSET	0x0000	/* Offset for normal register access */
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0000	/* Offset for alternate registers */

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_MONITOR_BASE	0xFFF80000
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)	/* Reserve 512 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN		(2*1024*1024)	/* Reserve 2MB for malloc() */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Init. Memory map for Linux */

/*
 * FLASH organization
 */
#define CONFIG_SYS_FLASH_BASE		0xFE000000
#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max num of sects on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT 2
#define CONFIG_SYS_FLASH_QUIET_TEST	1
#define CONFIG_SYS_FLASH_INCREMENT	0x01000000
#define CONFIG_SYS_FLASH_PROTECTION	1	/* use hardware protection */
#define CONFIG_SYS_FLASH_AUTOPROTECT_LIST { \
				{0xfe000000, 0x500000}, \
				{0xffe80000, 0x180000} \
				}
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster) */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ \
				CONFIG_SYS_FLASH_BASE, \
				CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_INCREMENT \
				}
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */

/*
 * Environment Variable setup
 */
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET		0x000	/* environment starts at the */
					/* beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x800	/* 2048 bytes may be used for env vars*/
#define CONFIG_ENV_OVERWRITE	1	/* allow overwriting vendor vars */

#define CONFIG_SYS_NVRAM_BASE_ADDR	0xF0000500	/* NVRAM base address */
#define CONFIG_SYS_NVRAM_SIZE		242		/* NVRAM size */

/*
 * I2C EEPROM (CAT24WC16) for environment
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		100000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC08 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address */
/* mask of address bits that overflow into the "EEPROM chip address" */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has */
					/* 16 byte page write mode using*/
					/* last	4 bits of the address */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10 /* and takes up to 10 msec */

/*
 * External Bus Controller (EBC) Setup
 */
#define FLASH0_BA       (CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_INCREMENT) /* FLASH 0 BA */
#define FLASH1_BA       CONFIG_SYS_FLASH_BASE      /* FLASH 1 Base Address          */
#define CAN_BA          0xF0000000          /* CAN Base Address              */
#define DUART0_BA       0xF0000400          /* DUART Base Address            */
#define DUART1_BA       0xF0000408          /* DUART Base Address            */
#define RTC_BA          0xF0000500          /* RTC Base Address              */
#define PS2_BA          0xF0000600          /* PS/2 Base Address             */
#define CF_BA           0xF0100000          /* CompactFlash Base Address     */
#define FPGA_BA         0xF0100100          /* FPGA internal Base Address    */
#define FUJI_BA         0xF0100200          /* Fuji internal Base Address    */
#define PCMCIA1_BA      0x20000000          /* PCMCIA Slot 1 Base Address    */
#define PCMCIA2_BA      0x28000000          /* PCMCIA Slot 2 Base Address    */
#define VGA_BA          0xF1000000          /* Epson VGA Base Address        */

#define CONFIG_SYS_FPGA_BASE_ADDR      FPGA_BA     /* FPGA internal Base Address    */

/* Memory Bank 0 (Flash Bank 0) initialization                               */
#define CONFIG_SYS_EBC_PB0AP   0x92015480
#define CONFIG_SYS_EBC_PB0CR   FLASH0_BA | 0x9A000 /* BAS=0xFF0,BS=16MB,BU=R/W,BW=16bit*/
#define CONFIG_SYS_EBC_PB0AP_HWREV8 CONFIG_SYS_EBC_PB0AP
#define CONFIG_SYS_EBC_PB0CR_HWREV8 FLASH1_BA | 0xBA000 /* BS=32MB */

/* Memory Bank 1 (Flash Bank 1) initialization                               */
#define CONFIG_SYS_EBC_PB1AP   0x92015480
#define CONFIG_SYS_EBC_PB1CR   FLASH1_BA | 0x9A000 /* BAS=0xFE0,BS=16MB,BU=R/W,BW=16bit*/

/* Memory Bank 2 (CAN0, 1, RTC, Duart) initialization                           */
#define CONFIG_SYS_EBC_PB2AP   0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB2CR   CAN_BA | 0x18000    /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit  */

/* Memory Bank 3 (CompactFlash IDE, FPGA internal) initialization               */
#define CONFIG_SYS_EBC_PB3AP   0x010059C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB3CR   CF_BA | 0x1A000     /* BAS=0xF01,BS=1MB,BU=R/W,BW=16bit */

/* Memory Bank 4 (PCMCIA Slot 1) initialization                                 */
#define CONFIG_SYS_EBC_PB4AP   0x050007C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB4CR   PCMCIA1_BA | 0xFA000 /*BAS=0x200,BS=128MB,BU=R/W,BW=16bit*/

/* Memory Bank 5 (Epson VGA) initialization                                     */
#define CONFIG_SYS_EBC_PB5AP   0x03805380   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=0 */
#define CONFIG_SYS_EBC_PB5CR   VGA_BA | 0x5A000    /* BAS=0xF10,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 6 (PCMCIA Slot 2) initialization                                 */
#define CONFIG_SYS_EBC_PB6AP   0x050007C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB6CR   PCMCIA2_BA | 0xFA000 /*BAS=0x280,BS=128MB,BU=R/W,BW=16bit*/

/*
 * FPGA stuff
 */

/* FPGA internal regs */
#define CONFIG_SYS_FPGA_CTRL           0x008
#define CONFIG_SYS_FPGA_CTRL2          0x00a

/* FPGA Control Reg */
#define CONFIG_SYS_FPGA_CTRL_CF_RESET  0x0001
#define CONFIG_SYS_FPGA_CTRL_WDI       0x0002
#define CONFIG_SYS_FPGA_CTRL_PS2_RESET 0x0020

#define CONFIG_SYS_FPGA_SPARTAN2       1           /* using Xilinx Spartan 2 now    */
#define CONFIG_SYS_FPGA_MAX_SIZE       80*1024     /* 80kByte is enough for XC2S50  */

/* FPGA program pin configuration */
#define CONFIG_SYS_FPGA_PRG            0x04000000  /* FPGA program pin (ppc output) */
#define CONFIG_SYS_FPGA_CLK            0x02000000  /* FPGA clk pin (ppc output)     */
#define CONFIG_SYS_FPGA_DATA           0x01000000  /* FPGA data pin (ppc output)    */
#define CONFIG_SYS_FPGA_INIT           0x00010000  /* FPGA init pin (ppc input)     */
#define CONFIG_SYS_FPGA_DONE           0x00008000  /* FPGA done pin (ppc input)     */

/*
 * LCD Setup
 */
#define CONFIG_SYS_LCD_BIG_MEM		(VGA_BA + 0x200000) /* S1D13806 Mem Base */
#define CONFIG_SYS_LCD_BIG_REG		VGA_BA /* S1D13806 Reg Base */

#define CONFIG_LCD_BIG		2 /* Epson S1D13806 used */

/* Image information... */
#define CONFIG_LCD_USED		CONFIG_LCD_BIG

#define CONFIG_SYS_LCD_MEM		CONFIG_SYS_LCD_BIG_MEM
#define CONFIG_SYS_LCD_REG		CONFIG_SYS_LCD_BIG_REG

#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (1 << 20)

/*
 * Definitions for initial stack pointer and data area (in data cache)
 */

/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM	1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of SDRAM */
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
/* reserve some memory for BOOT limit info */
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - 16)

#ifdef CONFIG_BOOTCOUNT_LIMIT /* reserve 2 word for bootcount limit */
#define CONFIG_SYS_BOOTCOUNT_ADDR (CONFIG_SYS_GBL_DATA_OFFSET - 8)
#endif

/*
 * PCI OHCI controller
 */
#define CONFIG_USB_OHCI_NEW	1
#define CONFIG_PCI_OHCI		1
#define CONFIG_SYS_OHCI_SWAP_REG_ACCESS 1
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 15
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"ohci_pci"
#define CONFIG_USB_STORAGE	1
#define CONFIG_SYS_USB_OHCI_BOARD_INIT 1

#endif /* __CONFIG_H */

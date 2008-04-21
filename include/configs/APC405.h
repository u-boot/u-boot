/*
 * (C) Copyright 2005-2008
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
 *
 * (C) Copyright 2001-2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
#define CONFIG_APCG405		1	/* ...on a APC405 board 	*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_BOARD_EARLY_INIT_R 1
#define CONFIG_MISC_INIT_R      1       /* call misc_init_r()           */

#define CONFIG_SYS_CLK_FREQ     33333400 /* external frequency to pll   */

#define CONFIG_BOARD_TYPES	1	/* support board types		*/

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	1	/* autoboot after 3 seconds	*/

#undef	CONFIG_BOOTARGS

#define CFG_USB_LOAD_COMMAND	"fatload usb 0 200000 pImage;"		\
				"fatload usb 0 300000 pImage.initrd"
#define CFG_USB_SELF_COMMAND	"usb start;run usb_load;usb stop;"	\
				"run ramargs addip addcon usbargs;"	\
				"bootm 200000 300000"
#define CFG_USB_ARGS		"setenv bootargs $(bootargs) usbboot=1"

#define CONFIG_EXTRA_ENV_SETTINGS                                       \
        "hostname=abg405\0"                                             \
        "bd_type=abg405\0"                                              \
	"serial#=AA0000\0"                                              \
        "kernel_addr=fe000000\0"                                        \
        "ramdisk_addr=fe100000\0"                                       \
        "ramargs=setenv bootargs root=/dev/ram rw\0"                    \
        "nfsargs=setenv bootargs root=/dev/nfs rw "                     \
                "nfsroot=$(serverip):$(rootpath)\0"                     \
        "addip=setenv bootargs $(bootargs) "                            \
                "ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"      \
                ":$(hostname)::off panic=1\0"                           \
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
	"usb_load="CFG_USB_LOAD_COMMAND"\0"				\
	"usb_self="CFG_USB_SELF_COMMAND"\0"				\
	"usbargs="CFG_USB_ARGS"\0"					\
        ""
#define CONFIG_BOOTCOMMAND	"run flash_self;run usb_self"

#define CONFIG_ETHADDR		00:02:27:8e:00:00

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_NET_MULTI	1
#undef  CONFIG_HAS_ETH1

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

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PCI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_IDE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_ELF
#define CONFIG_CMD_DATE
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_USB
#define CONFIG_CMD_AUTOSCRIPT

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define CONFIG_SUPPORT_VFAT

#define CONFIG_AUTO_UPDATE	1	/* autoupdate via CF or USB */

#undef  CONFIG_WATCHDOG			/* watchdog disabled */

#define CONFIG_RTC_MC146818		/* DS1685 is MC146818 compatible*/
#define CFG_RTC_REG_BASE_ADDR 0xF0000500 /* RTC Base Address */

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0 */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt */
#define CONFIG_CMDLINE_EDITING	1	/* add command line history */

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CFG_DEVICE_NULLDEV	1	/* include nulldev device */

#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CFG_MEMTEST_START	0x0400000 /* memtest works on */
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM */

#define CFG_EXT_SERIAL_CLOCK    14745600 /* use external serial clock   */

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE      \
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, \
	 57600, 115200, 230400, 460800, 921600 }

#define CFG_LOAD_ADDR	0x100000	/* default load address */
#define CFG_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

/* Only interrupt boot if space is pressed */
/* If a long serial cable is connected but */
/* other end is dead, garbage will be read */
#define CONFIG_AUTOBOOT_KEYED	1
#define CONFIG_AUTOBOOT_PROMPT	"Press SPACE to abort autoboot in %d seconds\n"
#undef CONFIG_AUTOBOOT_DELAY_STR
#define CONFIG_AUTOBOOT_STOP_STR " "

#define CONFIG_VERSION_VARIABLE	1	/* include version env variable */

#define CFG_RX_ETH_BUFFER	16	/* use 16 rx buffer on 405 emac */

/*
 * PCI stuff
 */
#define PCI_HOST_ADAPTER	0	/* configure as pci adapter     */
#define PCI_HOST_FORCE		1	/* configure as pci host        */
#define PCI_HOST_AUTO		2	/* detected via arbiter enable  */

#define CONFIG_PCI			/* include pci support          */
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function     */
#define CONFIG_PCI_PNP			/* do pci plug-and-play         */
					/* resource configuration       */

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup  */
#define CONFIG_PCI_SKIP_HOST_BRIDGE 1
#define CFG_PCI_SUBSYS_VENDORID 0x12FE  /* PCI Vendor ID: esd gmbh      */
#define CFG_PCI_SUBSYS_DEVICEID 0x0405  /* PCI Device ID: CPCI-405      */
#define CFG_PCI_CLASSCODE       0x0b20  /* PCI Class Code: Processor/PPC*/
#define CFG_PCI_PTM1LA  0x00000000      /* point to sdram               */
#define CFG_PCI_PTM1MS  0xfc000001      /* 64MB, enable hard-wired to 1 */
#define CFG_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CFG_PCI_PTM2LA  0xffc00000      /* point to flash               */
#define CFG_PCI_PTM2MS  0xffc00001      /* 4MB, enable                  */
#define CFG_PCI_PTM2PCI 0x04000000      /* Host: use this pci address   */

/*
 * IDE/ATA stuff
 */
#undef  CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef  CONFIG_IDE_LED			/* no led for ide supported */
#define CONFIG_IDE_RESET	1	/* reset for ide supported */

#define CFG_IDE_MAXBUS		1		/* max. 1 IDE busses */
#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS) /* max. 1 drives per IDE bus */

#define CFG_ATA_BASE_ADDR	0xF0100000
#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O */
#define CFG_ATA_REG_OFFSET	0x0000	/* Offset for normal register access */
#define CFG_ATA_ALT_OFFSET	0x0000	/* Offset for alternate registers */

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MONITOR_BASE	0xFFF80000
#define CFG_MONITOR_LEN		(512 * 1024)	/* Reserve 512 kB for Monitor */
#define CFG_MALLOC_LEN		(2*1024*1024)	/* Reserve 2MB for malloc() */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Init. Memory map for Linux */

/*
 * FLASH organization
 */
#ifndef __ASSEMBLY__
extern int flash_banks;
#endif

#define CFG_FLASH_BASE		0xFE000000
#define CFG_FLASH_CFI		1	/* Flash is CFI conformant */
#define CFG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#define CFG_MAX_FLASH_SECT	256	/* max num of sects on one chip */
#define CFG_MAX_FLASH_BANKS	flash_banks /* max num of flash banks */
					    /* updated in board_early_init_r */
#define CFG_MAX_FLASH_BANKS_DETECT 2
#define CFG_FLASH_QUIET_TEST	1
#define CFG_FLASH_INCREMENT	0x01000000
#define CFG_FLASH_PROTECTION	1	/* use hardware protection */
#define CFG_FLASH_AUTOPROTECT_LIST { \
				{0xfe000000, 0x500000}, \
				{0xffe80000, 0x180000} \
				}
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster) */
#define CFG_FLASH_BANKS_LIST	{ \
				CFG_FLASH_BASE, \
				CFG_FLASH_BASE + CFG_FLASH_INCREMENT \
				}
#define CFG_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */

/*
 * Environment Variable setup
 */
#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CFG_ENV_OFFSET		0x000	/* environment starts at the */
					/* beginning of the EEPROM */
#define CFG_ENV_SIZE		0x800	/* 2048 bytes may be used for env vars*/
#define CONFIG_ENV_OVERWRITE	1	/* allow overwriting vendor vars */

#define CFG_NVRAM_BASE_ADDR	0xF0000500	/* NVRAM base address */
#define CFG_NVRAM_SIZE		242		/* NVRAM size */

/*
 * I2C EEPROM (CAT24WC16) for environment
 */
#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC08 */
#define CFG_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address */
/* mask of address bits that overflow into the "EEPROM chip address" */
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CFG_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has */
					/* 16 byte page write mode using*/
					/* last	4 bits of the address */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10 /* and takes up to 10 msec */
#define CFG_EEPROM_PAGE_WRITE_ENABLE

/*
 * External Bus Controller (EBC) Setup
 */
#define FLASH0_BA       (CFG_FLASH_BASE + CFG_FLASH_INCREMENT) /* FLASH 0 BA */
#define FLASH1_BA       CFG_FLASH_BASE      /* FLASH 1 Base Address          */
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

#define CFG_FPGA_BASE_ADDR      FPGA_BA     /* FPGA internal Base Address    */

/* Memory Bank 0 (Flash Bank 0) initialization                               */
#define CFG_EBC_PB0AP   0x92015480
#define CFG_EBC_PB0CR   FLASH0_BA | 0x9A000 /* BAS=0xFF0,BS=16MB,BU=R/W,BW=16bit*/
#define CFG_EBC_PB0AP_HWREV8 CFG_EBC_PB0AP
#define CFG_EBC_PB0CR_HWREV8 FLASH1_BA | 0xBA000 /* BS=32MB */

/* Memory Bank 1 (Flash Bank 1) initialization                               */
#define CFG_EBC_PB1AP   0x92015480
#define CFG_EBC_PB1CR   FLASH1_BA | 0x9A000 /* BAS=0xFE0,BS=16MB,BU=R/W,BW=16bit*/

/* Memory Bank 2 (CAN0, 1, RTC, Duart) initialization                           */
#define CFG_EBC_PB2AP   0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB2CR   CAN_BA | 0x18000    /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit  */

/* Memory Bank 3 (CompactFlash IDE, FPGA internal) initialization               */
#define CFG_EBC_PB3AP   0x010059C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB3CR   CF_BA | 0x1A000     /* BAS=0xF01,BS=1MB,BU=R/W,BW=16bit */

/* Memory Bank 4 (PCMCIA Slot 1) initialization                                 */
#define CFG_EBC_PB4AP   0x050007C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB4CR   PCMCIA1_BA | 0xFA000 /*BAS=0x200,BS=128MB,BU=R/W,BW=16bit*/

/* Memory Bank 5 (Epson VGA) initialization                                     */
#define CFG_EBC_PB5AP   0x03805380   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=0 */
#define CFG_EBC_PB5CR   VGA_BA | 0x5A000    /* BAS=0xF10,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 6 (PCMCIA Slot 2) initialization                                 */
#define CFG_EBC_PB6AP   0x050007C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB6CR   PCMCIA2_BA | 0xFA000 /*BAS=0x280,BS=128MB,BU=R/W,BW=16bit*/

/*
 * FPGA stuff
 */

/* FPGA internal regs */
#define CFG_FPGA_CTRL           0x008
#define CFG_FPGA_CTRL2          0x00a

/* FPGA Control Reg */
#define CFG_FPGA_CTRL_CF_RESET  0x0001
#define CFG_FPGA_CTRL_WDI       0x0002
#define CFG_FPGA_CTRL_PS2_RESET 0x0020

#define CFG_FPGA_SPARTAN2       1           /* using Xilinx Spartan 2 now    */
#define CFG_FPGA_MAX_SIZE       80*1024     /* 80kByte is enough for XC2S50  */

/* FPGA program pin configuration */
#define CFG_FPGA_PRG            0x04000000  /* FPGA program pin (ppc output) */
#define CFG_FPGA_CLK            0x02000000  /* FPGA clk pin (ppc output)     */
#define CFG_FPGA_DATA           0x01000000  /* FPGA data pin (ppc output)    */
#define CFG_FPGA_INIT           0x00010000  /* FPGA init pin (ppc input)     */
#define CFG_FPGA_DONE           0x00008000  /* FPGA done pin (ppc input)     */

/*
 * LCD Setup
 */
#define CFG_LCD_BIG_MEM		(VGA_BA + 0x200000) /* S1D13806 Mem Base */
#define CFG_LCD_BIG_REG		VGA_BA /* S1D13806 Reg Base */

#define CONFIG_LCD_BIG		2 /* Epson S1D13806 used */

/* Image information... */
#define CONFIG_LCD_USED		CONFIG_LCD_BIG

#define CFG_LCD_MEM		CFG_LCD_BIG_MEM
#define CFG_LCD_REG		CFG_LCD_BIG_REG

#define CFG_VIDEO_LOGO_MAX_SIZE (1 << 20)

/*
 * Definitions for initial stack pointer and data area (in data cache)
 */

/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CFG_TEMP_STACK_OCM	1

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000

#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of SDRAM */
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM */
#define CFG_GBL_DATA_SIZE	128 /* reserved bytes for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

/*
 * PCI OHCI controller
 */
#define CONFIG_USB_OHCI_NEW	1
#define CONFIG_PCI_OHCI		1
#define CFG_OHCI_SWAP_REG_ACCESS 1
#define CFG_USB_OHCI_MAX_ROOT_PORTS 15
#define CFG_USB_OHCI_SLOT_NAME	"ohci_pci"
#define CONFIG_USB_STORAGE	1
#define CFG_USB_OHCI_BOARD_INIT 1

#endif /* __CONFIG_H */

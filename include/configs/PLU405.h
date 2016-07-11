/*
 * (C) Copyright 2001-2003
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

#define CONFIG_405EP		1	/* This is a PPC405 CPU		*/
#define CONFIG_PLU405		1	/* ...on a PLU405 board		*/

#define	CONFIG_SYS_TEXT_BASE	0xFFF80000
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

#define CONFIG_SYS_CLK_FREQ     33333400 /* external frequency to pll   */

#define CONFIG_BAUDRATE		9600

#undef	CONFIG_BOOTARGS
#undef	CONFIG_BOOTCOMMAND

#define CONFIG_PREBOOT                  /* enable preboot variable      */

#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#undef  CONFIG_HAS_ETH1

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_LXT971_NO_SLEEP  1       /* disable sleep mode in LXT971 */
#define CONFIG_RESET_PHY_R      1       /* use reset_phy()              */

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
#define CONFIG_CMD_PCI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NAND
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EEPROM

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define CONFIG_SUPPORT_VFAT

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_RTC_MC146818		/* DS1685 is MC146818 compatible*/
#define CONFIG_SYS_RTC_REG_BASE_ADDR	 0xF0000500 /* RTC Base Address		*/

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_DEVICE_NULLDEV	1	/* include nulldev device	*/

#define CONFIG_SYS_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef	CONFIG_SYS_EXT_SERIAL_CLOCK	       /* no external serial clock used */
#define CONFIG_SYS_BASE_BAUD	    691200

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

#define CONFIG_SYS_RX_ETH_BUFFER	16	/* use 16 rx buffer on 405 emac */

/*
 * NAND-FLASH stuff
 */
#define CONFIG_SYS_NAND_BASE_LIST	{CONFIG_SYS_NAND_BASE}
#define CONFIG_SYS_MAX_NAND_DEVICE	1         /* Max number of NAND devices */
#define NAND_BIG_DELAY_US	25

#define CONFIG_SYS_NAND_CE             (0x80000000 >> 1)   /* our CE is GPIO1  */
#define CONFIG_SYS_NAND_RDY            (0x80000000 >> 4)   /* our RDY is GPIO4 */
#define CONFIG_SYS_NAND_CLE            (0x80000000 >> 2)   /* our CLE is GPIO2 */
#define CONFIG_SYS_NAND_ALE            (0x80000000 >> 3)   /* our ALE is GPIO3 */

#define CONFIG_SYS_NAND_SKIP_BAD_DOT_I 1       /* ".i" read skips bad blocks   */
#define CONFIG_SYS_NAND_QUIET          1

/*
 * PCI stuff
 */
#define PCI_HOST_ADAPTER 0              /* configure as pci adapter     */
#define PCI_HOST_FORCE  1               /* configure as pci host        */
#define PCI_HOST_AUTO   2               /* detected via arbiter enable  */

#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_HOST	PCI_HOST_FORCE  /* select pci host function     */
#define CONFIG_PCI_PNP			/* do pci plug-and-play         */
					/* resource configuration       */

#define CONFIG_PCI_SCAN_SHOW            /* print pci devices @ startup  */

#define CONFIG_PCI_CONFIG_HOST_BRIDGE 1 /* don't skip host bridge config*/

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x12FE  /* PCI Vendor ID: esd gmbh      */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0405  /* PCI Device ID: CPCI-405      */
#define CONFIG_SYS_PCI_CLASSCODE       0x0b20  /* PCI Class Code: Processor/PPC*/
#define CONFIG_SYS_PCI_PTM1LA  0x00000000      /* point to sdram               */
#define CONFIG_SYS_PCI_PTM1MS  0xf8000001      /* 128MB, enable hard-wired to 1 */
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CONFIG_SYS_PCI_PTM2LA  0xffc00000      /* point to flash               */
#define CONFIG_SYS_PCI_PTM2MS  0xffc00001      /* 4MB, enable                  */
#define CONFIG_SYS_PCI_PTM2PCI 0x08000000      /* Host: use this pci address   */

/*
 * IDE/ATA stuff
 */
#undef	CONFIG_IDE_8xx_DIRECT		    /* no pcmcia interface required */
#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#define CONFIG_IDE_RESET	1	/* reset for ide supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1		/* max. 1 IDE busses	*/
/* max. 1 drives per IDE bus */
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS*1)

#define CONFIG_SYS_ATA_BASE_ADDR	0xF0100000
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O */
#define CONFIG_SYS_ATA_REG_OFFSET	0x0000	/* Offset for normal register access */
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0000	/* Offset for alternate registers */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*
 * FLASH organization
 */
#define FLASH_BASE0_PRELIM	0xFFC00000 /* FLASH bank #0 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max number of sectors on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	1000	/* Timeout for Flash Write (in ms) */

#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned short	/* flash word size (width) */
#define CONFIG_SYS_FLASH_ADDR0		0x5555	/* 1st addr for flash config cycles */
#define CONFIG_SYS_FLASH_ADDR1		0x2AAA	/* 2nd addr for flash config cycles */
/*
 * The following defines are added for buggy IOP480 byte interface.
 * All other boards should use the standard values (CPCI405 etc.)
 */
#define CONFIG_SYS_FLASH_READ0		0x0000	/* 0 is standard */
#define CONFIG_SYS_FLASH_READ1		0x0001	/* 1 is standard */
#define CONFIG_SYS_FLASH_READ2		0x0002	/* 2 is standard */

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector */

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(~(CONFIG_SYS_TEXT_BASE) + 1)
#define CONFIG_SYS_MALLOC_LEN		(1024 << 10)

/*
 * Environment Variable setup
 */
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET		0x100	/* reseve 0x100 bytes for strapping */
#define CONFIG_ENV_SIZE		0x700

/*
 * I2C EEPROM (24WC16) for environment
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM 24WC16 */
#define CONFIG_SYS_EEPROM_WREN         1

/* 24WC16 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 4	/* The 24WC16 has   */
					/* 16 byte page write mode using */
					/* last 4 bits of the address   */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */

/*
 * External Bus Controller (EBC) Setup
 */
#define CAN0_BA		0xF0000000	    /* CAN0 Base Address	*/
#define CAN1_BA		0xF0000100	    /* CAN1 Base Address	*/
#define DUART0_BA	0xF0000400	    /* DUART Base Address       */
#define DUART1_BA	0xF0000408	    /* DUART Base Address       */
#define RTC_BA		0xF0000500	    /* RTC Base Address         */
#define VGA_BA		0xF1000000	    /* Epson VGA Base Address   */
#define CONFIG_SYS_NAND_BASE	0xF4000000	    /* NAND FLASH Base Address  */

/* Memory Bank 0 (Flash Bank 0, NOR-FLASH) initialization */
/* TWT=16,CSN=1,OEN=1,WBN=1,WBF=1,TH=4,SOR=1 */
#define CONFIG_SYS_EBC_PB0AP		0x92015480
/* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */
#define CONFIG_SYS_EBC_PB0CR		0xFFC5A000

/* Memory Bank 1 (Flash Bank 1, NAND-FLASH) initialization */
#define CONFIG_SYS_EBC_PB1AP		0x92015480
/* BAS=0xF40,BS=1MB,BU=R/W,BW=8bit */
#define CONFIG_SYS_EBC_PB1CR		0xF4018000

/* Memory Bank 2 (8 Bit Peripheral: CAN, UART, RTC) initialization */
/* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB2AP		0x010053C0
/* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit */
#define CONFIG_SYS_EBC_PB2CR		0xF0018000

/* Memory Bank 3 (16 Bit Peripheral: FPGA internal, dig. IO) initialization */
/* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB3AP		0x010053C0
/* BAS=0xF01,BS=1MB,BU=R/W,BW=16bit */
#define CONFIG_SYS_EBC_PB3CR		0xF011A000

/*
 * FPGA stuff
 */
#define CONFIG_SYS_FPGA_BASE_ADDR 0xF0100100	    /* FPGA internal Base Address */

/* FPGA internal regs */
#define CONFIG_SYS_FPGA_CTRL		0x000

/* FPGA Control Reg */
#define CONFIG_SYS_FPGA_CTRL_CF_RESET	0x0001
#define CONFIG_SYS_FPGA_CTRL_WDI	0x0002
#define CONFIG_SYS_FPGA_CTRL_PS2_RESET 0x0020

#define CONFIG_SYS_FPGA_SPARTAN2	1	    /* using Xilinx Spartan 2 now */
#define CONFIG_SYS_FPGA_MAX_SIZE	128*1024    /* 128kByte is enough for XC2S50E*/

/* FPGA program pin configuration */
#define CONFIG_SYS_FPGA_PRG		0x04000000  /* FPGA program pin (ppc output) */
#define CONFIG_SYS_FPGA_CLK		0x02000000  /* FPGA clk pin (ppc output) */
#define CONFIG_SYS_FPGA_DATA		0x01000000  /* FPGA data pin (ppc output) */
#define CONFIG_SYS_FPGA_INIT		0x00010000  /* FPGA init pin (ppc input) */
#define CONFIG_SYS_FPGA_DONE		0x00008000  /* FPGA done pin (ppc input) */

/*
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM	  1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of SDRAM */
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM  */

#define CONFIG_SYS_GBL_DATA_OFFSET    (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Definitions for GPIO setup (PPC405EP specific)
 *
 * GPIO0[0]	- External Bus Controller BLAST output
 * GPIO0[1-9]	- Instruction trace outputs -> GPIO
 * GPIO0[10-13] - External Bus Controller CS_1 - CS_4 outputs
 * GPIO0[14-16] - External Bus Controller ABUS3-ABUS5 outputs -> GPIO
 * GPIO0[17-23] - External Interrupts IRQ0 - IRQ6 inputs
 * GPIO0[24-27] - UART0 control signal inputs/outputs
 * GPIO0[28-29] - UART1 data signal input/output
 * GPIO0[30-31] - EMAC0 and EMAC1 reject packet inputs
 */
#define CONFIG_SYS_GPIO0_OSRL		0x00000550
#define CONFIG_SYS_GPIO0_OSRH		0x00000110
#define CONFIG_SYS_GPIO0_ISR1L		0x00000000
#define CONFIG_SYS_GPIO0_ISR1H		0x15555445
#define CONFIG_SYS_GPIO0_TSRL		0x00000000
#define CONFIG_SYS_GPIO0_TSRH		0x00000000
#define CONFIG_SYS_GPIO0_TCR		0x77FE0014

#define CONFIG_SYS_DUART_RST		(0x80000000 >> 14)
#define CONFIG_SYS_EEPROM_WP		(0x80000000 >> 0)

/*
 * Default speed selection (cpu_plb_opb_ebc) in MHz.
 * This value will be set if iic boot eprom is disabled.
 */
#if 1
#define PLLMR0_DEFAULT	 PLLMR0_266_133_66_33
#define PLLMR1_DEFAULT	 PLLMR1_266_133_66_33
#endif
#if 0
#define PLLMR0_DEFAULT	 PLLMR0_200_100_50_33
#define PLLMR1_DEFAULT	 PLLMR1_200_100_50_33
#endif
#if 0
#define PLLMR0_DEFAULT	 PLLMR0_133_66_66_33
#define PLLMR1_DEFAULT	 PLLMR1_133_66_66_33
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

/*
 * UBI
 */
#define CONFIG_CMD_UBI
#define CONFIG_RBTREE
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_LZO

#endif	/* __CONFIG_H */

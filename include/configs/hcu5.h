/*
 * (C) Copyright 2007 Netstal Maschinen AG
 * Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/************************************************************************
 * hcu5.h - configuration for HCU5 board (derived from sequoia.h)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_HCU5		1		/* Board is HCU5	*/
#define CONFIG_440EPX		1		/* Specific PPC440EPx	*/
#define CONFIG_440		1		/* ... PPC440 family	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_SYS_CLK_FREQ	33333333	/* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor	*/
#define CFG_MALLOC_LEN		(256 * 1024)	/* Reserve 256 kB for malloc()	*/

#define CFG_BOOT_BASE_ADDR	0xfff00000
#define CFG_SDRAM_BASE		0x00000000	/* _must_ be 0		*/
#define CFG_FLASH_BASE		0xfff80000	/* start of FLASH	*/
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_OCM_BASE		0xe0010000      /* ocm			*/
#define CFG_PCI_BASE		0xe0000000      /* Internal PCI regs	*/
#define CFG_PCI_MEMBASE		0x80000000	/* mapped pci memory	*/
#define CFG_PCI_MEMBASE1	CFG_PCI_MEMBASE  + 0x10000000
#define CFG_PCI_MEMBASE2	CFG_PCI_MEMBASE1 + 0x10000000
#define CFG_PCI_MEMBASE3	CFG_PCI_MEMBASE2 + 0x10000000

/* Don't change either of these */
#define CFG_PERIPHERAL_BASE	0xef600000	/* internal peripherals	*/

#define CFG_USB2D0_BASE		0xe0000100
#define CFG_USB_DEVICE		0xe0000000
#define CFG_USB_HOST		0xe0000400

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
/* 440EPx/440GRx have 16KB of internal SRAM, so no need for D-Cache	*/
#define CFG_INIT_RAM_ADDR	CFG_OCM_BASE	/* OCM			*/

#define CFG_INIT_RAM_END	(4 << 10)
#define CFG_GBL_DATA_SIZE	256		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef CFG_EXT_SERIAL_CLOCK	       /* external serial clock */
#define CONFIG_BAUDRATE		9600
#undef CONFIG_SERIAL_MULTI            /* needed to be able to define
	CONFIG_SERIAL_SOFTWARE_FIFO, but
	CONFIG_SERIAL_SOFTWARE_FIFO (16) does not work */
/* Size (bytes) of interrupt driven serial port buffer.
 * Set to 0 to use polling instead of interrupts.
 * Setting to 0 will also disable RTS/CTS handshaking.
 */
#undef CONFIG_SERIAL_SOFTWARE_FIFO
#undef CONFIG_UART1_CONSOLE

#define CFG_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/

#undef	CFG_ENV_IS_IN_NVRAM
#undef  CFG_ENV_IS_IN_FLASH
#define	CFG_ENV_IS_IN_EEPROM
#undef  CFG_ENV_IS_NOWHERE

#ifdef  CFG_ENV_IS_IN_EEPROM
/* Put the environment after the SDRAM and bootstrap configuration */
#define PROM_SIZE 	2048
#define CFG_BOOSTRAP_OPTION_OFFSET	 512
#define CFG_ENV_OFFSET	 (CFG_BOOSTRAP_OPTION_OFFSET + 0x10)
#define CFG_ENV_SIZE	(PROM_SIZE-CFG_ENV_OFFSET)
#endif

#ifdef CFG_ENV_IS_IN_FLASH
/* Put the environment in Flash */
#define CFG_ENV_SECT_SIZE	0x10000 	/* size of one complete sector	*/
#define CFG_ENV_ADDR		((-CFG_MONITOR_LEN)-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CFG_MBYTES_SDRAM        (128)		/* 128 MB or 256 MB  		*/
#define CFG_DDR_CACHED_ADDR	0x40000000	/* setup 2nd TLB cached here	*/
#undef  CONFIG_DDR_DATA_EYE			/* Do not use DDR2 optimization	*/
#define CONFIG_DDR_ECC		1		/* enable ECC			*/

/*-----------------------------------------------------------------------
 * I2C stuff for a ATMEL AT24C16 (2kB holding ENV, we are using the
 * the second internal I2C controller of the PPC440EPx
 *----------------------------------------------------------------------*/
#define CFG_SPD_BUS_NUM		1

#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

/* This is the 7bit address of the device, not including P. */
#define CFG_I2C_EEPROM_ADDR 0x50
#define CFG_I2C_EEPROM_ADDR_LEN 1

/* The EEPROM can do 16byte ( 1 << 4 ) page writes. */
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CFG_EEPROM_PAGE_WRITE_BITS 4
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#undef CFG_I2C_MULTI_EEPROMS


#define CONFIG_PREBOOT	"echo;"						\
	"echo Type \"run nfs\" to mount Linux root filesystem over NFS;"\
	"echo"

#undef	CONFIG_BOOTARGS

/* Setup some board specific values for the default environment variables */
#define CONFIG_HOSTNAME		hcu5
#define CONFIG_IPADDR		172.25.1.42
#define CONFIG_ETHADDR      	00:60:13:00:00:00   /* Netstal Machines AG MAC */
#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define CONFIG_SERVERIP		172.25.1.3

#define CFG_TFTP_LOADADDR 0x01000000 /* @16 MB */

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"loadaddr=0x01000000\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;" 	\
		"bootm\0"						\
		"bootfile=hcu5/uImage\0" 				\
		"rootpath=/home/hcu/eldk/ppc_4xxFP\0"		 	\
		"load=tftp 100000 hcu5/u-boot.bin\0"		 	\
	"update=protect off FFFa0000 FFFFFFFF;era FFFa0000 FFFFFFFF;"	\
		"cp.b 100000 FFFa0000 60000\0"			        \
	"upd=run load;run update\0"					\
	"vx=tftp ${loadaddr} hcu5/hcu5_vx_rom;" 			\
	"setenv bootargs emac(0,0)hcu5_vx_rom e=${ipaddr} " 	 	\
		" h=${serverip} u=dpu pw=netstal8752 tn=hcu5 f=0x3008;" \
	"bootvx ${loadaddr}\0" \
	""
#define CONFIG_BOOTCOMMAND	"run vx"

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_M88E1111_PHY	1
#define	CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address, See schematics	*/

#define CONFIG_PHY_RESET        1	/* reset phy upon startup         */

#define CONFIG_HAS_ETH0
#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NET_MULTI	1
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#define CONFIG_PHY1_ADDR	1

/* USB */
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/* Comment this out to enable USB 1.1 device */
#define USB_2_0_DEVICE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

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
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_USB

#define CONFIG_SUPPORT_VFAT

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE              (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	        16	/* max number of command args	*/
#define CFG_BARGSIZE	        CFG_CBSIZE /* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000  /* default load address	*/
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ		        1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#undef CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CFG_PCI_TARGBASE        0x80000000 /* PCIaddr mapped to CFG_PCI_MEMBASE*/

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT
#define CFG_PCI_MASTER_INIT

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC				*/
#define CFG_PCI_SUBSYS_ID       0xcafe	/* Whatever			*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define CFG_FLASH		CFG_FLASH_BASE
#define CFG_CS_1		0xC8000000 /* CAN */
#define CFG_CS_2		0xCC000000 /* CPLD and IMC-Bus Standard */
#define CFG_CPLD		CFG_CS_2
#define CFG_CS_3		0xCD000000 /* CPLD and IMC-Bus Fast  */

/*-----------------------------------------------------------------------
 * FLASH organization
 * Memory Bank 0 (BOOT-FLASH) initialization
 */
#define CFG_BOOTFLASH_CS		0	/* Boot Flash chip connected to CSx	*/
#define CFG_EBC_PB0AP		0x02005400
#define CFG_EBC_PB0CR		0xFFF18000 /* (CFG_FLASH | 0xda000)  */
#define FLASH_BASE0_PRELIM	CFG_FLASH_BASE	/* FLASH bank #0	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	32	/* max number of sectors on one chip	*/


#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/* Memory Bank 1 CAN-Chips initialization						*/
#define CFG_EBC_PB1AP		0x02054500
#define CFG_EBC_PB1CR		0xC8018000

/* Memory Bank 2 CPLD/IMC-Bus standard initialization						*/
#define CFG_EBC_PB2AP		0x01840300
#define CFG_EBC_PB2CR		0xCC0BA000

/* Memory Bank 3 IMC-Bus fast mode initialization						*/
#define CFG_EBC_PB3AP		0x01800300
#define CFG_EBC_PB3CR		0xCE0BA000

/* Memory Bank 4 (not used) initialization						*/
#undef CFG_EBC_PB4AP
#undef CFG_EBC_PB4CR

/* Memory Bank 5 (not used) initialization						*/
#undef CFG_EBC_PB5AP
#undef CFG_EBC_PB5CR

#define HCU_CPLD_VERSION_REGISTER ( CFG_CPLD + 0x0F00000 )
#define HCU_HW_VERSION_REGISTER   ( CFG_CPLD + 0x1400000 )

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#define CFG_HUSH_PARSER                 /* use "hush" command parser    */
#ifdef  CFG_HUSH_PARSER
	#define CFG_PROMPT_HUSH_PS2     "> "
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	    /* which serial port to use */
#endif
#endif	/* __CONFIG_H */

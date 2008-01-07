/*
 * (C) Copyright 2007
 * Larry Johnson, lrj@acm.org
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/************************************************************************
 * korat.h - configuration for Korat board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_440EPX		1	/* Specific PPC440EPx           */
#define CONFIG_4xx		1	/* ... PPC4xx family            */
#define CONFIG_SYS_CLK_FREQ	33333333

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f      */
#define CONFIG_MISC_INIT_R	1	/* Call misc_init_r             */

/*-----------------------------------------------------------------------
 * Manufacturer's information serial EEPROM parameters
 *----------------------------------------------------------------------*/
#define MAN_DATA_EEPROM_ADDR	0x53	/* EEPROM I2C address           */
#define MAN_SERIAL_NO_FIELD	2
#define MAN_SERIAL_NO_LENGTH	13
#define MAN_MAC_ADDR_FIELD	3
#define MAN_MAC_ADDR_LENGTH	17

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor   */
#define CFG_MALLOC_LEN		(256 * 1024)	/* Reserve 256 kB for malloc()  */

#define CFG_BOOT_BASE_ADDR	0xf0000000
#define CFG_SDRAM_BASE		0x00000000	/* _must_ be 0          */
#define CFG_FLASH_BASE		0xfc000000	/* start of FLASH       */
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_OCM_BASE		0xe0010000	/* ocm                  */
#define CFG_OCM_DATA_ADDR	CFG_OCM_BASE
#define CFG_PCI_BASE		0xe0000000	/* Internal PCI regs    */
#define CFG_PCI_MEMBASE		0x80000000	/* mapped pci memory    */
#define CFG_PCI_MEMBASE1	CFG_PCI_MEMBASE  + 0x10000000
#define CFG_PCI_MEMBASE2	CFG_PCI_MEMBASE1 + 0x10000000
#define CFG_PCI_MEMBASE3	CFG_PCI_MEMBASE2 + 0x10000000

/* Don't change either of these */
#define CFG_PERIPHERAL_BASE	0xef600000	/* internal peripherals */

#define CFG_USB2D0_BASE		0xe0000100
#define CFG_USB_DEVICE		0xe0000000
#define CFG_USB_HOST		0xe0000400
#define CFG_CPLD_BASE		0xc0000000

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
/* 440EPx has 16KB of internal SRAM, so no need for D-Cache		*/
#undef CFG_INIT_RAM_DCACHE
#define CFG_INIT_RAM_ADDR	CFG_OCM_BASE	/* OCM                  */
#define CFG_INIT_RAM_END	(4 << 10)
#define CFG_GBL_DATA_SIZE	256	/* num bytes initial data       */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_POST_WORD_ADDR

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CFG_EXT_SERIAL_CLOCK	11059200	/* ext. 11.059MHz clk   */
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI	1
/* define this if you want console on UART1 */
#undef CONFIG_UART1_CONSOLE

#define CFG_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environ vars   */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI			/* The flash is CFI compatible  */
#define CFG_FLASH_CFI_DRIVER		/* Use common CFI driver        */

#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks           */
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip    */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)      */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)      */

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)     */
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection        */

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash        */

#define CFG_ENV_SECT_SIZE	0x20000	/* size of one complete sector          */
#define CFG_ENV_ADDR		((-CFG_MONITOR_LEN)-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector     */

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CFG_MBYTES_SDRAM        (512)	/* 512 MiB      TODO: remove    */
#define CONFIG_DDR_DATA_EYE		/* use DDR2 optimization        */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for setup     */
#define CONFIG_ZERO_SDRAM		/* Zero SDRAM after setup       */
#define CONFIG_DDR_ECC			/* Use ECC when available       */
#define SPD_EEPROM_ADDRESS	{0x50}
#define CONFIG_PROG_SDRAM_TLB
#define CFG_DRAM_TEST

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	/* I2C with hardware support    */
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged               */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address  */
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	(0xa8>>1)
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

/* I2C RTC */
#define CONFIG_RTC_M41T60	1
#define CFG_I2C_RTC_ADDR	0x68

/* I2C SYSMON (LM73)							*/
#define CONFIG_DTT_LM73		1	/* National Semi's LM73 */
#define CONFIG_DTT_SENSORS	{2}	/* Sensor addresses     */
#define CFG_DTT_MAX_TEMP	70
#define CFG_DTT_MIN_TEMP	-30

#define CONFIG_PREBOOT	"echo;"						\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

/* Setup some board specific values for the default environment variables */
#define CONFIG_HOSTNAME		korat
#define CFG_BOOTFILE		"bootfile=/tftpboot/korat/uImage\0"
#define CFG_ROOTPATH		"rootpath=/opt/eldk/ppc_4xxFP\0"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CFG_BOOTFILE							\
	CFG_ROOTPATH							\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
	        "bootm\0"						\
	"kernel_addr=FC000000\0"					\
	"ramdisk_addr=FC180000\0"					\
	"load=tftp 200000 /tftpboot/${hostname}/u-boot.bin\0"		\
	"update=protect off FFFA0000 FFFFFFFF;era FFFA0000 FFFFFFFF;"	\
		"cp.b 200000 FFFA0000 60000\0"			        \
	"upd=run load;run update\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds     */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download  */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change        */

#define	CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management           */
#define CONFIG_PHY_ADDR		2	/* PHY address, See schematics  */
#define CONFIG_PHY_DYNAMIC_ANEG	1

#define CONFIG_PHY_RESET        1	/* reset phy upon startup         */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0
#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NET_MULTI	1
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"   */
#define CONFIG_PHY1_ADDR	3

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
#define CONFIG_BOOTP_SUBNETMASK

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DTT
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_I2C_CMD_TREE
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_USB

/* POST support */
#define CONFIG_POST		(CFG_POST_CACHE    | \
				 CFG_POST_CPU	   | \
				 CFG_POST_ECC      | \
				 CFG_POST_ETHER	   | \
				 CFG_POST_FPU	   | \
				 CFG_POST_I2C	   | \
				 CFG_POST_MEMORY   | \
				 CFG_POST_RTC      | \
				 CFG_POST_SPR      | \
				 CFG_POST_UART)

#define CFG_POST_WORD_ADDR	(CFG_GBL_DATA_OFFSET - 0x4)
#define CONFIG_LOGBUFFER
#define CFG_POST_CACHE_ADDR	0xC8000000	/* free virtual address      */

#define CFG_CONSOLE_IS_IN_ENV	/* Otherwise it catches logbuffer as output */

#define CONFIG_SUPPORT_VFAT

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP			/* undef to save memory         */
#define CFG_PROMPT	        "=> "	/* Monitor Command Prompt       */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	        1024	/* Console I/O Buffer Size      */
#else
#define CFG_CBSIZE	        256	/* Console I/O Buffer Size      */
#endif
#define CFG_PBSIZE              (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS	        16	/* max number of command args   */
#define CFG_BARGSIZE	        CFG_CBSIZE /* Boot Argument Buffer Size */

#define CFG_MEMTEST_START	0x0400000 /* memtest works on           */
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM        */

#define CFG_LOAD_ADDR		0x100000  /* default load address       */
#define CFG_EXTBDINFO		1  /* To use extended board_into (bd_t) */

#define CFG_HZ		        1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history     */
#define CONFIG_LOOPW            1	/* enable loopw command         */
#define CONFIG_MX_CYCLIC        1	/* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK /* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI			/* include pci support          */
#define CONFIG_PCI_PNP			/* do pci plug-and-play         */
#define CFG_PCI_CACHE_LINE_SIZE	0	/* to avoid problems with PNP   */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CFG_PCI_TARGBASE        0x80000000 /* PCIaddr mapped to CFG_PCI_MEMBASE */

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT
#define CFG_PCI_MASTER_INIT

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC                         */
#define CFG_PCI_SUBSYS_ID       0xcafe	/* Whatever                     */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/

/* Memory Bank 0 (NOR-FLASH) initialization				*/
#define CFG_EBC_PB0AP		0x04017300
#define CFG_EBC_PB0CR		(CFG_FLASH_BASE | 0x000DA000)

/* Memory Bank 1 (NOR-FLASH) initialization				*/
#define CFG_EBC_PB1AP		0x04017300
#define CFG_EBC_PB1CR		(0xF8000000 | 0x000DA000)

/* Memory Bank 2 (CPLD) initialization					*/
#define CFG_EBC_PB2AP		0x04017300
#define CFG_EBC_PB2CR		(CFG_CPLD_BASE | 0x00038000)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH     */
#define BOOTFLAG_WARM	0x02	/* Software reboot                      */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400 /* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use     */
#endif
#endif /* __CONFIG_H */

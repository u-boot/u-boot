/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * TQM8349 board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */
#define CONFIG_MPC83XX		1	/* MPC83XX family */
#define CONFIG_MPC834X		1	/* MPC834X specific */
#define CONFIG_MPC8349		1	/* MPC8349 specific */
#define CONFIG_TQM834X		1	/* TQM834X board specific */

/* IMMR Base Addres Register, use Freescale default: 0xff400000 */
#define CFG_IMMR		0xff400000

/* System clock. Primary input clock when in PCI host mode */
#define CONFIG_83XX_CLKIN	66666000	/* 66,666 MHz */

/*
 * Local Bus LCRR
 *    LCRR:  DLL bypass, Clock divider is 8
 *
 *    for CSB = 266 MHz it gives LCB clock frequency = 33 MHz
 *
 * External Local Bus rate is
 *    CLKIN * HRCWL_CSB_TO_CLKIN / HRCWL_LCL_BUS_TO_SCB_CLK / LCRR_CLKDIV
 */
#define CFG_LCRR		(LCRR_DBYP | LCRR_CLKDIV_8)

/* board pre init: do not call, nothing to do */
#undef CONFIG_BOARD_EARLY_INIT_F

/* detect the number of flash banks */
#define CONFIG_BOARD_EARLY_INIT_R

/*
 * DDR Setup
 */
#define CFG_DDR_BASE		0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_BASE
#define CFG_DDR_SDRAM_BASE	CFG_DDR_BASE
#define DDR_CASLAT_25				/* CASLAT set to 2.5 */
#undef CONFIG_DDR_ECC				/* only for ECC DDR module */
#undef CONFIG_SPD_EEPROM			/* do not use SPD EEPROM for DDR setup */

#undef CFG_DRAM_TEST				/* memory test, takes time */
#define CFG_MEMTEST_START	0x00000000	/* memtest region */
#define CFG_MEMTEST_END		0x00100000

/*
 * FLASH on the Local Bus
 */
#define CFG_FLASH_CFI				/* use the Common Flash Interface */
#define CFG_FLASH_CFI_DRIVER			/* use the CFI driver */
#undef CFG_FLASH_CHECKSUM
#define CFG_FLASH_BASE		0x80000000	/* start of FLASH   */
#define CFG_FLASH_SIZE		8		/* FLASH size in MB */

/* buffered writes in the AMD chip set is not supported yet */
#undef CFG_FLASH_USE_BUFFER_WRITE

/*
 * FLASH bank number detection
 */

/*
 * When CFG_MAX_FLASH_BANKS_DETECT is defined, the actual number of Flash
 * banks has to be determined at runtime and stored in a gloabl variable
 * tqm834x_num_flash_banks. The value of CFG_MAX_FLASH_BANKS_DETECT is only
 * used instead of CFG_MAX_FLASH_BANKS to allocate the array flash_info, and
 * should be made sufficiently large to accomodate the number of banks that
 * might actually be detected.  Since most (all?) Flash related functions use
 * CFG_MAX_FLASH_BANKS as the number of actual banks on the board, it is
 * defined as tqm834x_num_flash_banks.
 */
#define CFG_MAX_FLASH_BANKS_DETECT	2
#ifndef __ASSEMBLY__
extern int tqm834x_num_flash_banks;
#endif
#define CFG_MAX_FLASH_BANKS (tqm834x_num_flash_banks)

#define CFG_MAX_FLASH_SECT		512	/* max sectors per device */

/* 32 bit device at 0x80000000 via GPCM (0x8000_1801) */
#define CFG_BR0_PRELIM		((CFG_FLASH_BASE & BR_BA) | \
					BR_MS_GPCM | BR_PS_32 | BR_V)

/* FLASH timing (0x0000_0c54) */
#define CFG_OR_TIMING_FLASH	(OR_GPCM_CSNT | OR_GPCM_ACS_0b10 | \
					OR_GPCM_SCY_5 | OR_GPCM_TRLX)

#define CFG_PRELIM_OR_AM	0xc0000000	/* OR addr mask: 1 GiB */

#define CFG_OR0_PRELIM		(CFG_PRELIM_OR_AM  | CFG_OR_TIMING_FLASH)

#define CFG_LBLAWAR0_PRELIM	0x8000001D	/* 1 GiB window size (2^(size + 1)) */

#define CFG_LBLAWBAR0_PRELIM	CFG_FLASH_BASE	/* Window base at flash base */

/* disable remaining mappings */
#define CFG_BR1_PRELIM		0x00000000
#define CFG_OR1_PRELIM		0x00000000
#define CFG_LBLAWBAR1_PRELIM	0x00000000
#define CFG_LBLAWAR1_PRELIM	0x00000000

#define CFG_BR2_PRELIM		0x00000000
#define CFG_OR2_PRELIM		0x00000000
#define CFG_LBLAWBAR2_PRELIM	0x00000000
#define CFG_LBLAWAR2_PRELIM	0x00000000

#define CFG_BR3_PRELIM		0x00000000
#define CFG_OR3_PRELIM		0x00000000
#define CFG_LBLAWBAR3_PRELIM	0x00000000
#define CFG_LBLAWAR3_PRELIM	0x00000000

#define CFG_BR4_PRELIM		0x00000000
#define CFG_OR4_PRELIM		0x00000000
#define CFG_LBLAWBAR4_PRELIM	0x00000000
#define CFG_LBLAWAR4_PRELIM	0x00000000

#define CFG_BR5_PRELIM		0x00000000
#define CFG_OR5_PRELIM		0x00000000
#define CFG_LBLAWBAR5_PRELIM	0x00000000
#define CFG_LBLAWAR5_PRELIM	0x00000000

#define CFG_BR6_PRELIM		0x00000000
#define CFG_OR6_PRELIM		0x00000000
#define CFG_LBLAWBAR6_PRELIM	0x00000000
#define CFG_LBLAWAR6_PRELIM	0x00000000

#define CFG_BR7_PRELIM		0x00000000
#define CFG_OR7_PRELIM		0x00000000
#define CFG_LBLAWBAR7_PRELIM	0x00000000
#define CFG_LBLAWAR7_PRELIM	0x00000000

/*
 * Monitor config
 */
#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef  CFG_RAMBOOT
#endif

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK	1
#define CFG_INIT_RAM_ADDR	0x20000000	/* Initial RAM address */
#define CFG_INIT_RAM_END	0x1000		/* End of used area in RAM*/

#define CFG_GBL_DATA_SIZE  	0x100		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(256 * 1024) /* Reserve 256 kB for malloc */

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#undef CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1	(CFG_IMMR + 0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR + 0x4600)

/*
 * I2C
 */
#define CONFIG_HARD_I2C				/* I2C with hardware support	*/
#undef CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CONFIG_FSL_I2C
#define CFG_I2C_SPEED			400000	/* I2C speed: 400KHz		*/
#define CFG_I2C_SLAVE			0x7F	/* slave address		*/
#define CFG_I2C_OFFSET			0x3000

/* I2C EEPROM, configuration for onboard EEPROMs 24C256 and 24C32 */
#define CFG_I2C_EEPROM_ADDR		0x50	/* 1010000x			*/
#define CFG_I2C_EEPROM_ADDR_LEN		2	/* 16 bit			*/
#define CFG_EEPROM_PAGE_WRITE_BITS	5	/* 32 bytes per write		*/
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	12	/* 10ms +/- 20%			*/
#define CFG_I2C_MULTI_EEPROMS		1       /* more than one eeprom		*/

/* I2C RTC */
#define CONFIG_RTC_DS1337			/* use ds1337 rtc via i2c	*/
#define CFG_I2C_RTC_ADDR		0x68	/* at address 0x68		*/

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75			1	/* ON Semi's LM75		*/
#define CONFIG_DTT_SENSORS		{0}	/* Sensor addresses		*/
#define CFG_DTT_MAX_TEMP		70
#define CFG_DTT_LOW_TEMP		-30
#define CFG_DTT_HYSTERESIS		3

/*
 * TSEC
 */
#define CONFIG_TSEC_ENET 		/* tsec ethernet support */
#define CONFIG_MII

#define CFG_TSEC1_OFFSET	0x24000
#define CFG_TSEC1		(CFG_IMMR + CFG_TSEC1_OFFSET)
#define CFG_TSEC2_OFFSET	0x25000
#define CFG_TSEC2		(CFG_IMMR + CFG_TSEC2_OFFSET)

#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI
#endif

#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define TSEC1_PHY_ADDR			2
#define TSEC2_PHY_ADDR			1
#define TSEC1_PHYIDX			0
#define TSEC2_PHYIDX			0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME			"TSEC0"

#endif	/* CONFIG_TSEC_ENET */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_PCI

#if defined(CONFIG_PCI)

#define CONFIG_PCI_PNP                  /* do pci plug-and-play */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup */

/* PCI1 host bridge */
#define CFG_PCI1_MEM_BASE       0xc0000000
#define CFG_PCI1_MEM_PHYS       CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE       0x20000000      /* 512M */
#define CFG_PCI1_IO_BASE        0xe2000000
#define CFG_PCI1_IO_PHYS        CFG_PCI1_IO_BASE
#define CFG_PCI1_IO_SIZE        0x1000000       /* 16M */

#undef CONFIG_EEPRO100
#define CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
	#define PCI_ENET0_IOADDR	CFG_PCI1_IO_BASE
	#define PCI_ENET0_MEMADDR	CFG_PCI1_MEM_BASE
	#define PCI_IDSEL_NUMBER	0x1c    /* slot0 (IDSEL) = 28 */
#endif

#define CFG_PCI_SUBSYS_VENDORID		0x1957  /* Freescale */

#endif	/* CONFIG_PCI */

/*
 * Environment
 */
#define CONFIG_ENV_OVERWRITE

#ifndef CFG_RAMBOOT
	#define CFG_ENV_IS_IN_FLASH	1
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x40000)
	#define CFG_ENV_SECT_SIZE	0x40000	/* 256K(one sector) for env */
	#define CFG_ENV_SIZE		0x2000
#else
	#define CFG_NO_FLASH		1	/* Flash is not usable now */
	#define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
	#define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO		1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE		1	/* allow baudrate change */

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
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CFG_RAMBOOT)
    #undef CONFIG_CMD_ENV
    #undef CONFIG_CMD_LOADS
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory	*/
#define CFG_LOAD_ADDR		0x2000000	/* default load address */
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CFG_HUSH_PARSER		1	/* Use the HUSH parser		*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif

#if defined(CONFIG_CMD_KGDB)
	#define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
	#define CFG_CBSIZE	256		/* Console I/O Buffer Size */
#endif

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ			1000		/* decrementer freq: 1ms ticks */

#undef CONFIG_WATCHDOG				/* watchdog disabled */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

#define CFG_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_4X1 |\
	HRCWL_VCO_1X2 |\
	HRCWL_CORE_TO_CSB_2X1)

#if defined(PCI_64BIT)
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_64_BIT_PCI |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCI2_ARBITER_DISABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_TSEC1M_IN_GMII |\
	HRCWH_TSEC2M_IN_GMII )
#else
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_32_BIT_PCI |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCI2_ARBITER_DISABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_TSEC1M_IN_GMII |\
	HRCWH_TSEC2M_IN_GMII )
#endif

/* System IO Config */
#define CFG_SICRH	SICRH_TSOBI1
#define CFG_SICRL	SICRL_LDP_A

/* i-cache and d-cache disabled */
#define CFG_HID0_INIT	0x000000000
#define CFG_HID0_FINAL	CFG_HID0_INIT
#define CFG_HID2	HID2_HBE

/* DDR 0 - 512M */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT1L	(CFG_SDRAM_BASE + 0x10000000 | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_SDRAM_BASE + 0x10000000 | BATU_BL_256M | BATU_VS | BATU_VP)

/* stack in DCACHE @ 512M (no backing mem) */
#define CFG_IBAT2L	(CFG_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT2U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

/* PCI */
#ifdef CONFIG_PCI
#define CFG_IBAT3L	(CFG_PCI1_MEM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT3U	(CFG_PCI1_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT4L	(CFG_PCI1_MEM_BASE + 0x10000000 | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT4U	(CFG_PCI1_MEM_BASE + 0x10000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT5L	(CFG_PCI1_IO_BASE | BATL_PP_10 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT5U	(CFG_PCI1_IO_BASE + 0x10000000 | BATU_BL_16M | BATU_VS | BATU_VP)
#else
#define CFG_IBAT3L	(0)
#define CFG_IBAT3U	(0)
#define CFG_IBAT4L	(0)
#define CFG_IBAT4U	(0)
#define CFG_IBAT5L	(0)
#define CFG_IBAT5U	(0)
#endif

/* IMMRBAR */
#define CFG_IBAT6L	(CFG_IMMR | BATL_PP_10 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT6U	(CFG_IMMR | BATU_BL_1M | BATU_VS | BATU_VP)

/* FLASH */
#define CFG_IBAT7L	(CFG_FLASH_BASE | BATL_PP_10 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT7U	(CFG_FLASH_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U
#define CFG_DBAT3L	CFG_IBAT3L
#define CFG_DBAT3U	CFG_IBAT3U
#define CFG_DBAT4L	CFG_IBAT4L
#define CFG_DBAT4U	CFG_IBAT4U
#define CFG_DBAT5L	CFG_IBAT5L
#define CFG_DBAT5U	CFG_IBAT5U
#define CFG_DBAT6L	CFG_IBAT6L
#define CFG_DBAT6U	CFG_IBAT6U
#define CFG_DBAT7L	CFG_IBAT7L
#define CFG_DBAT7U	CFG_IBAT7U

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

#define CONFIG_LOADADDR		400000	/* default location for tftp and bootm */

#define CONFIG_BOOTDELAY	6	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS			/* the boot command will set bootargs */

#define CONFIG_BAUDRATE		115200

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=tqm834x\0"						\
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
	"net_nfs=tftp 400000 ${bootfile};run nfsargs addip addtty;"     \
		"bootm\0"						\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"bootfile=/tftpboot/tqm834x/uImage\0"				\
	"kernel_addr=80060000\0"					\
	"ramdisk_addr=80160000\0"					\
	"load=tftp 100000 /tftpboot/tqm834x/u-boot.bin\0"		\
	"update=protect off 80000000 8003ffff; "			\
		"era 80000000 8003ffff; cp.b 100000 80000000 40000\0"	\
	"upd=run load update\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

/*
 * JFFS2 partitions
 */
/* mtdparts command line support */
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=TQM834x-0"

/* default mtd partition table */
#define MTDPARTS_DEFAULT	"mtdparts=TQM834x-0:256k(u-boot),256k(env),"\
						"1m(kernel),2m(initrd),"\
						"-(user);"\

#endif	/* __CONFIG_H */

/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#define CONFIG_MPC83xx		1	/* MPC83xx family */
#define CONFIG_MPC834x		1	/* MPC834x specific */
#define CONFIG_MPC8349		1	/* MPC8349 specific */
#define CONFIG_TQM834X		1	/* TQM834X board specific */

#define	CONFIG_SYS_TEXT_BASE	0x80000000

/* IMMR Base Address Register, use Freescale default: 0xff400000 */
#define CONFIG_SYS_IMMR		0xff400000

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
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_8

/* board pre init: do not call, nothing to do */
#undef CONFIG_BOARD_EARLY_INIT_F

/* detect the number of flash banks */
#define CONFIG_BOARD_EARLY_INIT_R

/*
 * DDR Setup
 */
				/* DDR is system memory*/
#define CONFIG_SYS_DDR_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define DDR_CASLAT_25		/* CASLAT set to 2.5 */
#undef CONFIG_DDR_ECC		/* only for ECC DDR module */
#undef CONFIG_SPD_EEPROM	/* do not use SPD EEPROM for DDR setup */

#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00100000

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER		/* use the CFI driver */
#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_BASE		0x80000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		8		/* FLASH size in MB */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sectors */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

/*
 * FLASH bank number detection
 */

/*
 * When CONFIG_SYS_MAX_FLASH_BANKS_DETECT is defined, the actual number of
 * Flash banks has to be determined at runtime and stored in a gloabl variable
 * tqm834x_num_flash_banks. The value of CONFIG_SYS_MAX_FLASH_BANKS_DETECT is
 * only used instead of CONFIG_SYS_MAX_FLASH_BANKS to allocate the array
 * flash_info, and should be made sufficiently large to accomodate the number
 * of banks that might actually be detected.  Since most (all?) Flash related
 * functions use CONFIG_SYS_MAX_FLASH_BANKS as the number of actual banks on
 * the board, it is defined as tqm834x_num_flash_banks.
 */
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	2

#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max sectors per device */

/* 32 bit device at 0x80000000 via GPCM (0x8000_1801) */
#define CONFIG_SYS_BR0_PRELIM	((CONFIG_SYS_FLASH_BASE & BR_BA) \
				| BR_MS_GPCM \
				| BR_PS_32 \
				| BR_V)

/* FLASH timing (0x0000_0c54) */
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_GPCM_CSNT \
					| OR_GPCM_ACS_DIV4 \
					| OR_GPCM_SCY_5 \
					| OR_GPCM_TRLX)

#define CONFIG_SYS_PRELIM_OR_AM		OR_AM_1GB /* OR addr mask: 1 GiB */

#define CONFIG_SYS_OR0_PRELIM		(CONFIG_SYS_PRELIM_OR_AM  \
					| CONFIG_SYS_OR_TIMING_FLASH)

#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_1GB)

					/* Window base at flash base */
#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE

/* disable remaining mappings */
#define CONFIG_SYS_BR1_PRELIM		0x00000000
#define CONFIG_SYS_OR1_PRELIM		0x00000000
#define CONFIG_SYS_LBLAWBAR1_PRELIM	0x00000000
#define CONFIG_SYS_LBLAWAR1_PRELIM	0x00000000

#define CONFIG_SYS_BR2_PRELIM		0x00000000
#define CONFIG_SYS_OR2_PRELIM		0x00000000
#define CONFIG_SYS_LBLAWBAR2_PRELIM	0x00000000
#define CONFIG_SYS_LBLAWAR2_PRELIM	0x00000000

#define CONFIG_SYS_BR3_PRELIM		0x00000000
#define CONFIG_SYS_OR3_PRELIM		0x00000000
#define CONFIG_SYS_LBLAWBAR3_PRELIM	0x00000000
#define CONFIG_SYS_LBLAWAR3_PRELIM	0x00000000

/*
 * Monitor config
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
# define CONFIG_SYS_RAMBOOT
#else
# undef  CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

				/* Reserve 384 kB = 3 sect. for Mon */
#define CONFIG_SYS_MONITOR_LEN	(384 * 1024)
				/* Reserve 512 kB for malloc */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024)

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000

/* I2C EEPROM, configuration for onboard EEPROMs 24C256 and 24C32 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2	/* 16 bit */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5	/* 32 bytes/write */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	12	/* 10ms +/- 20% */
#define CONFIG_SYS_I2C_MULTI_EEPROMS		/* more than one eeprom */

/* I2C RTC */
#define CONFIG_RTC_DS1337			/* use ds1337 rtc via i2c */
#define CONFIG_SYS_I2C_RTC_ADDR		0x68	/* at address 0x68 */

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75			1	/* ON Semi's LM75 */
#define CONFIG_DTT_SENSORS		{0}	/* Sensor addresses */
#define CONFIG_SYS_DTT_MAX_TEMP		70
#define CONFIG_SYS_DTT_LOW_TEMP		-30
#define CONFIG_SYS_DTT_HYSTERESIS	3

/*
 * TSEC
 */
#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_MII

#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC2_OFFSET)

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define TSEC1_PHY_ADDR		2
#define TSEC2_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"TSEC0"

#endif	/* CONFIG_TSEC_ENET */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_PCI

#if defined(CONFIG_PCI)

#define CONFIG_PCI_PNP			/* do pci plug-and-play */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

/* PCI1 host bridge */
#define CONFIG_SYS_PCI1_MEM_BASE	0x90000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	\
			(CONFIG_SYS_PCI1_MEM_BASE + CONFIG_SYS_PCI1_MEM_SIZE)
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BASE		0xe2000000
#define CONFIG_SYS_PCI1_IO_PHYS		CONFIG_SYS_PCI1_IO_BASE
#define CONFIG_SYS_PCI1_IO_SIZE		0x1000000	/* 16M */

#undef CONFIG_EEPRO100
#define CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
	#define PCI_ENET0_IOADDR	CONFIG_SYS_PCI1_IO_BASE
	#define PCI_ENET0_MEMADDR	CONFIG_SYS_PCI1_MEM_BASE
	#define PCI_IDSEL_NUMBER	0x1c    /* slot0 (IDSEL) = 28 */
#endif

#define CONFIG_SYS_PCI_SUBSYS_VENDORID		0x1957  /* Freescale */

#endif	/* CONFIG_PCI */

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K (one sector) for env */
#define CONFIG_ENV_SIZE		0x8000	/*  32K max size */
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define CONFIG_LOADS_ECHO		1 /* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1 /* allow baudrate change */

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
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_NFS
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SNTP

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#if defined(CONFIG_SYS_RAMBOOT)
    #undef CONFIG_CMD_SAVEENV
    #undef CONFIG_CMD_LOADS
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support */

#define CONFIG_SYS_HUSH_PARSER		1	/* Use the HUSH parser */

#if defined(CONFIG_CMD_KGDB)
	#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#else
	#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
#endif

				/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
				/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

#undef CONFIG_WATCHDOG		/* watchdog disabled */

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_4X1 |\
	HRCWL_VCO_1X2 |\
	HRCWL_CORE_TO_CSB_2X1)

#if defined(PCI_64BIT)
#define CONFIG_SYS_HRCW_HIGH (\
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
	HRCWH_TSEC2M_IN_GMII)
#else
#define CONFIG_SYS_HRCW_HIGH (\
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
	HRCWH_TSEC2M_IN_GMII)
#endif

/* System IO Config */
#define CONFIG_SYS_SICRH	0
#define CONFIG_SYS_SICRL	SICRL_LDP_A

/* i-cache and d-cache disabled */
#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(CONFIG_SYS_HID0_INIT | \
				 HID0_ENABLE_INSTRUCTION_CACHE)
#define CONFIG_SYS_HID2	HID2_HBE

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR 0 - 512M */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_SDRAM_BASE + 0x10000000 \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_SDRAM_BASE + 0x10000000 \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* stack in DCACHE @ 512M (no backing mem) */
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_INIT_RAM_ADDR \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_INIT_RAM_ADDR \
				| BATU_BL_128K \
				| BATU_VS \
				| BATU_VP)

/* PCI */
#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_PCI1_MEM_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_PCI1_MEM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATL_PP_RW \
				| BATL_MEMCOHERENCE \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT4U	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_PCI1_IO_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_PCI1_IO_BASE \
				| BATU_BL_16M \
				| BATU_VS \
				| BATU_VP)
#else
#define CONFIG_SYS_IBAT3L	(0)
#define CONFIG_SYS_IBAT3U	(0)
#define CONFIG_SYS_IBAT4L	(0)
#define CONFIG_SYS_IBAT4U	(0)
#define CONFIG_SYS_IBAT5L	(0)
#define CONFIG_SYS_IBAT5U	(0)
#endif

/* IMMRBAR */
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_IMMR \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_IMMR \
				| BATU_BL_1M \
				| BATU_VS \
				| BATU_VP)

/* FLASH */
#define CONFIG_SYS_IBAT7L	(CONFIG_SYS_FLASH_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT7U	(CONFIG_SYS_FLASH_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U
#define CONFIG_SYS_DBAT2L	CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U
#define CONFIG_SYS_DBAT3L	CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U
#define CONFIG_SYS_DBAT4L	CONFIG_SYS_IBAT4L
#define CONFIG_SYS_DBAT4U	CONFIG_SYS_IBAT4U
#define CONFIG_SYS_DBAT5L	CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */

				/* default location for tftp and bootm */
#define CONFIG_LOADADDR		400000

#define CONFIG_BOOTDELAY	6	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs */

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
	"addcons=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
	"flash_nfs_old=run nfsargs addip addcons;"			\
		"bootm ${kernel_addr}\0"				\
	"flash_nfs=run nfsargs addip addcons;"				\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"flash_self_old=run ramargs addip addcons;"			\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_self=run ramargs addip addcons;"				\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"net_nfs_old=tftp 400000 ${bootfile};"				\
		"run nfsargs addip addcons;bootm\0"			\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"tftp ${fdt_addr_r} ${fdt_file}; "			\
		"run nfsargs addip addcons; "				\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"bootfile=tqm834x/uImage\0"					\
	"fdtfile=tqm834x/tqm834x.dtb\0"					\
	"kernel_addr_r=400000\0"					\
	"fdt_addr_r=600000\0"						\
	"ramdisk_addr_r=800000\0"					\
	"kernel_addr=800C0000\0"					\
	"fdt_addr=800A0000\0"						\
	"ramdisk_addr=80300000\0"					\
	"u-boot=tqm834x/u-boot.bin\0"					\
	"load=tftp 200000 ${u-boot}\0"					\
	"update=protect off 80000000 +${filesize};"			\
		"era 80000000 +${filesize};"				\
		"cp.b 200000 80000000 ${filesize}\0"			\
	"upd=run load update\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

/*
 * JFFS2 partitions
 */
/* mtdparts command line support */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=TQM834x-0"

/* default mtd partition table */
#define MTDPARTS_DEFAULT	"mtdparts=TQM834x-0:256k(u-boot),256k(env)," \
						"1m(kernel),2m(initrd)," \
						"-(user);" \

#endif	/* __CONFIG_H */

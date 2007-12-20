/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006.
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
 * mpc8313epb board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1
#define CONFIG_MPC83XX		1
#define CONFIG_MPC831X		1
#define CONFIG_MPC8313		1
#define CONFIG_MPC8313ERDB	1

#define CONFIG_PCI
#define CONFIG_83XX_GENERIC_PCI

#ifdef CFG_66MHZ
#define CONFIG_83XX_CLKIN	66666667	/* in Hz */
#elif defined(CFG_33MHZ)
#define CONFIG_83XX_CLKIN	33333333	/* in Hz */
#else
#error Unknown oscillator frequency.
#endif

#define CONFIG_SYS_CLK_FREQ	CONFIG_83XX_CLKIN

#define CONFIG_BOARD_EARLY_INIT_F		/* call board_pre_init */

#define CFG_IMMR		0xE0000000

#define CFG_MEMTEST_START	0x00001000
#define CFG_MEMTEST_END		0x07f00000

/* Early revs of this board will lock up hard when attempting
 * to access the PMC registers, unless a JTAG debugger is
 * connected, or some resistor modifications are made.
 */
#define CFG_8313ERDB_BROKEN_PMC 1

#define CFG_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CFG_ACR_RPTCNT		3	/* Arbiter repeat count (0-7) */

/*
 * DDR Setup
 */
#define CFG_DDR_BASE		0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_BASE
#define CFG_DDR_SDRAM_BASE	CFG_DDR_BASE

/*
 * Manually set up DDR parameters, as this board does not
 * seem to have the SPD connected to I2C.
 */
#define CFG_DDR_SIZE		128		/* MB */
#define CFG_DDR_CONFIG		( CSCONFIG_EN | CSCONFIG_AP \
				| 0x00040000 /* TODO */ \
				| CSCONFIG_ROW_BIT_13 | CSCONFIG_COL_BIT_10 )
				/* 0x80840102 */

#define CFG_DDR_TIMING_3	0x00000000
#define CFG_DDR_TIMING_0	( ( 0 << TIMING_CFG0_RWT_SHIFT ) \
				| ( 0 << TIMING_CFG0_WRT_SHIFT ) \
				| ( 0 << TIMING_CFG0_RRT_SHIFT ) \
				| ( 0 << TIMING_CFG0_WWT_SHIFT ) \
				| ( 2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT ) \
				| ( 2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT ) \
				| ( 8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT ) \
				| ( 2 << TIMING_CFG0_MRS_CYC_SHIFT ) )
				/* 0x00220802 */
#define CFG_DDR_TIMING_1	( ( 3 << TIMING_CFG1_PRETOACT_SHIFT ) \
				| ( 9 << TIMING_CFG1_ACTTOPRE_SHIFT ) \
				| ( 3 << TIMING_CFG1_ACTTORW_SHIFT ) \
				| ( 5 << TIMING_CFG1_CASLAT_SHIFT ) \
				| (13 << TIMING_CFG1_REFREC_SHIFT ) \
				| ( 3 << TIMING_CFG1_WRREC_SHIFT ) \
				| ( 2 << TIMING_CFG1_ACTTOACT_SHIFT ) \
				| ( 2 << TIMING_CFG1_WRTORD_SHIFT ) )
				/* 0x3935d322 */
#define CFG_DDR_TIMING_2	( ( 0 << TIMING_CFG2_ADD_LAT_SHIFT ) \
				| (31 << TIMING_CFG2_CPO_SHIFT ) \
				| ( 2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT ) \
				| ( 2 << TIMING_CFG2_RD_TO_PRE_SHIFT ) \
				| ( 2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT ) \
				| ( 3 << TIMING_CFG2_CKE_PLS_SHIFT ) \
				| (10 << TIMING_CFG2_FOUR_ACT_SHIFT) )
				/* 0x0f9048ca */ /* P9-45,may need tuning */
#define CFG_DDR_INTERVAL	( ( 800 << SDRAM_INTERVAL_REFINT_SHIFT ) \
				| ( 100 << SDRAM_INTERVAL_BSTOPRE_SHIFT ) )
				/* 0x03200064 */
#if defined(CONFIG_DDR_2T_TIMING)
#define CFG_SDRAM_CFG		( SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_2T_EN \
				| SDRAM_CFG_DBW_32 )
#else
#define CFG_SDRAM_CFG		( SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_32_BE )
				/* 0x43080000 */
#endif
#define CFG_SDRAM_CFG2		0x00401000;
/* set burst length to 8 for 32-bit data path */
#define CFG_DDR_MODE		( ( 0x4440 << SDRAM_MODE_ESD_SHIFT ) \
				| ( 0x0232 << SDRAM_MODE_SD_SHIFT ) )
				/* 0x44400232 */
#define CFG_DDR_MODE_2		0x8000C000;

#define CFG_DDR_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
				/*0x02000000*/
#define CFG_DDRCDR_VALUE	( DDRCDR_EN \
				| DDRCDR_PZ_NOMZ \
				| DDRCDR_NZ_NOMZ \
				| DDRCDR_M_ODR )

/*
 * FLASH on the Local Bus
 */
#define CFG_FLASH_CFI				/* use the Common Flash Interface */
#define CFG_FLASH_CFI_DRIVER			/* use the CFI driver */
#define CFG_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CFG_FLASH_SIZE		8		/* flash size in MB */
#define CFG_FLASH_EMPTY_INFO			/* display empty sectors */
#define CFG_FLASH_USE_BUFFER_WRITE		/* buffer up multiple bytes */

#define CFG_BR0_PRELIM		(CFG_FLASH_BASE |	/* flash Base address */ \
				(2 << BR_PS_SHIFT) |	/* 16 bit port size */ \
				BR_V)			/* valid */
#define CFG_OR0_PRELIM		( 0xFF000000		/* 16 MByte */ \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_9 \
				| OR_GPCM_EHTR \
				| OR_GPCM_EAD )
				/* 0xFF006FF7	TODO SLOW 16 MB flash size */
#define CFG_LBLAWBAR0_PRELIM	CFG_FLASH_BASE	/* window base at flash base */
#define CFG_LBLAWAR0_PRELIM	0x80000017	/* 16 MB window size */

#define CFG_MAX_FLASH_BANKS	1		/* number of banks */
#define CFG_MAX_FLASH_SECT	135		/* sectors per device */

#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#endif

#define CFG_INIT_RAM_LOCK	1
#define CFG_INIT_RAM_ADDR	0xFD000000	/* Initial RAM address */
#define CFG_INIT_RAM_END	0x1000		/* End of used area in RAM*/

#define CFG_GBL_DATA_SIZE	0x100		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/* CFG_MONITOR_LEN must be a multiple of CFG_ENV_SECT_SIZE */
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(512 * 1024)	/* Reserved for malloc */

/*
 * Local Bus LCRR and LBCR regs
 */
#define CFG_LCRR	LCRR_EADC_1 | LCRR_CLKDIV_2	/* 0x00010002 */
#define CFG_LBC_LBCR	( 0x00040000 /* TODO */ \
			| (0xFF << LBCR_BMT_SHIFT) \
			| 0xF )	/* 0x0004ff0f */

#define CFG_LBC_MRTPR	0x20000000  /*TODO */ 	/* LB refresh timer prescal, 266MHz/32 */

/* drivers/nand/nand.c */
#define CFG_NAND_BASE		0xE2800000	/* 0xF0000000 */
#define CFG_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS		1
#define CONFIG_MTD_NAND_VERIFY_WRITE

#define CFG_BR1_PRELIM		( CFG_NAND_BASE \
				| (2<<BR_DECC_SHIFT)	/* Use HW ECC */ \
				| BR_PS_8		/* Port Size = 8 bit */ \
				| BR_MS_FCM		/* MSEL = FCM */ \
				| BR_V )		/* valid */
#define CFG_OR1_PRELIM		( 0xFFFF8000		/* length 32K */ \
				| OR_FCM_CSCT \
				| OR_FCM_CST \
				| OR_FCM_CHT \
				| OR_FCM_SCY_1 \
				| OR_FCM_TRLX \
				| OR_FCM_EHTR )
				/* 0xFFFF8396 */
#define CFG_LBLAWBAR1_PRELIM	CFG_NAND_BASE
#define CFG_LBLAWAR1_PRELIM	0x8000000E	/* 32KB  */

#define CFG_VSC7385_BASE	0xF0000000

#define CONFIG_VSC7385_ENET			/* VSC7385 ethernet support */
#define CFG_BR2_PRELIM		0xf0000801	/* VSC7385 Base address */
#define CFG_OR2_PRELIM		0xfffe09ff	/* VSC7385, 128K bytes*/
#define CFG_LBLAWBAR2_PRELIM	CFG_VSC7385_BASE/* Access window base at VSC7385 base */
#define CFG_LBLAWAR2_PRELIM	0x80000010	/* Access window size 128K */

/* local bus read write buffer mapping */
#define CFG_BR3_PRELIM		0xFA000801	/* map at 0xFA000000 */
#define CFG_OR3_PRELIM		0xFFFF8FF7	/* 32kB */
#define CFG_LBLAWBAR3_PRELIM	0xFA000000
#define CFG_LBLAWAR3_PRELIM	0x8000000E	/* 32KB  */

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CFG_NS16550_COM1	(CFG_IMMR+0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "

/* I2C */
#define CONFIG_HARD_I2C			/* I2C with hardware support*/
#define CONFIG_FSL_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{{0,0x69}} /* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000
#define CFG_I2C2_OFFSET		0x3100

/* TSEC */
#define CFG_TSEC1_OFFSET	0x24000
#define CFG_TSEC1		(CFG_IMMR+CFG_TSEC1_OFFSET)
#define CFG_TSEC2_OFFSET	0x25000
#define CFG_TSEC2		(CFG_IMMR+CFG_TSEC2_OFFSET)
#define CONFIG_NET_MULTI

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PCI1_MMIO_BASE	0x90000000
#define CFG_PCI1_MMIO_PHYS	CFG_PCI1_MMIO_BASE
#define CFG_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xE2000000
#define CFG_PCI1_IO_SIZE	0x00100000	/* 1M */

#define CONFIG_PCI_PNP		/* do pci plug-and-play */
#define CFG_PCI_SUBSYS_VENDORID 0x1057	/* Motorola */

/*
 * TSEC configuration
 */
#define CONFIG_TSEC_ENET		/* TSEC ethernet support */

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI		1
#endif

#define CONFIG_GMII			1	/* MII PHY management */
#define CONFIG_TSEC1		1

#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define TSEC1_PHY_ADDR			0x1c
#define TSEC2_PHY_ADDR			4
#define TSEC1_FLAGS			TSEC_GIGABIT
#define TSEC2_FLAGS			TSEC_GIGABIT
#define TSEC1_PHYIDX			0
#define TSEC2_PHYIDX			0

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME			"TSEC1"

/*
 * Configure on-board RTC
 */
#define CONFIG_RTC_DS1337
#define CFG_I2C_RTC_ADDR		0x68

/*
 * Environment
 */
#ifndef CFG_RAMBOOT
	#define CFG_ENV_IS_IN_FLASH	1
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
	#define CFG_ENV_SECT_SIZE	0x10000	/* 64K(one sector) for env */
	#define CFG_ENV_SIZE		0x2000

/* Address and size of Redundant Environment Sector */
#else
	#define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
	#define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

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

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE
#define CONFIG_CMD_PCI

#if defined(CFG_RAMBOOT)
    #undef CONFIG_CMD_ENV
    #undef CONFIG_CMD_LOADS
#endif

#define CONFIG_CMDLINE_EDITING 1


/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory */
#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt */
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size */

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

/* Cache Configuration */
#define CFG_DCACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32
#define CFG_CACHELINE_SHIFT	5	/*log base 2 of the above value*/

#define CFG_RCWH_PCIHOST 0x80000000	/* PCIHOST  */

#ifdef CFG_66MHZ

/* 66MHz IN, 133MHz CSB, 266 DDR, 266 CORE */
/* 0x62040000 */
#define CFG_HRCW_LOW (\
	0x20000000 /* reserved, must be set */ |\
	HRCWL_DDRCM |\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_2X1 |\
	HRCWL_CSB_TO_CLKIN_2X1 |\
	HRCWL_CORE_TO_CSB_2X1)

#elif defined(CFG_33MHZ)

/* 33MHz IN, 165MHz CSB, 330 DDR, 330 CORE */
/* 0x65040000 */
#define CFG_HRCW_LOW (\
	0x20000000 /* reserved, must be set */ |\
	HRCWL_DDRCM |\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_2X1 |\
	HRCWL_CSB_TO_CLKIN_5X1 |\
	HRCWL_CORE_TO_CSB_2X1)

#endif

/* 0xa0606c00 */
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_RL_EXT_LEGACY |\
	HRCWH_TSEC1M_IN_RGMII |\
	HRCWH_TSEC2M_IN_RGMII |\
	HRCWH_BIG_ENDIAN |\
	HRCWH_LALE_NORMAL)

/* System IO Config */
#define CFG_SICRH	(SICRH_TSOBI1 | SICRH_TSOBI2)	/* RGMII */
#define CFG_SICRL	SICRL_USBDR			/* Enable Internal USB Phy  */

#define CFG_HID0_INIT	0x000000000
#define CFG_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK | \
			 HID0_ENABLE_DYNAMIC_POWER_MANAGMENT)

#define CFG_HID2 HID2_HBE

/* DDR @ 0x00000000 */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* PCI @ 0x80000000 */
#define CFG_IBAT1L	(CFG_PCI1_MEM_BASE | BATL_PP_10)
#define CFG_IBAT1U	(CFG_PCI1_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_IBAT2L	(CFG_PCI1_MMIO_BASE | BATL_PP_10 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT2U	(CFG_PCI1_MMIO_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* PCI2 not supported on 8313 */
#define CFG_IBAT3L	(0)
#define CFG_IBAT3U	(0)
#define CFG_IBAT4L	(0)
#define CFG_IBAT4U	(0)

/* IMMRBAR @ 0xE0000000, PCI IO @ 0xE2000000 & BCSR @ 0xE2400000 */
#define CFG_IBAT5L	(CFG_IMMR | BATL_PP_10 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT5U	(CFG_IMMR | BATU_BL_256M | BATU_VS | BATU_VP)

/* SDRAM @ 0xF0000000, stack in DCACHE 0xFDF00000 & FLASH @ 0xFE000000 */
#define CFG_IBAT6L	(0xF0000000 | BATL_PP_10)
#define CFG_IBAT6U	(0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT7L	(0)
#define CFG_IBAT7U	(0)

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
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02	/* Software reboot */

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_ETHADDR		00:E0:0C:00:95:01
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH0
#define CONFIG_ETH1ADDR		00:E0:0C:00:95:02

#define CONFIG_IPADDR		10.0.0.2
#define CONFIG_SERVERIP		10.0.0.1
#define CONFIG_GATEWAYIP	10.0.0.1
#define CONFIG_NETMASK		255.0.0.0
#define CONFIG_NETDEV		eth1

#define CONFIG_HOSTNAME		mpc8313erdb
#define CONFIG_ROOTPATH		/nfs/root/path
#define CONFIG_BOOTFILE		uImage
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */
#define CONFIG_FDTFILE		mpc8313erdb.dtb

#define CONFIG_LOADADDR		200000	/* default location for tftp and bootm */
#define CONFIG_BOOTDELAY	-1	/* -1 disables auto-boot */
#define CONFIG_BAUDRATE		115200

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" MK_STR(CONFIG_NETDEV) "\0" 				\
	"ethprime=TSEC1\0"						\
	"uboot=" MK_STR(CONFIG_UBOOTPATH) "\0" 				\
	"tftpflash=tftpboot $loadaddr $uboot; " 			\
		"protect off " MK_STR(TEXT_BASE) " +$filesize; " 	\
		"erase " MK_STR(TEXT_BASE) " +$filesize; " 		\
		"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; " 	\
		"protect on " MK_STR(TEXT_BASE) " +$filesize; " 	\
		"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0" 	\
	"fdtaddr=400000\0"						\
	"fdtfile=" MK_STR(CONFIG_FDTFILE) "\0"				\
	"console=ttyS0\0"						\
	"setbootargs=setenv bootargs "					\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"setipargs=setenv bootargs nfsroot=$serverip:$rootpath " \
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv rootdev /dev/nfs;"					\
	"run setbootargs;"							\
	"run setipargs;"							\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv rootdev /dev/ram;"					\
	"run setbootargs;"						\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#undef MK_STR
#undef XMK_STR

#endif	/* __CONFIG_H */

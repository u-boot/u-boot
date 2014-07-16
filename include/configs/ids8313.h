/*
 * (C) Copyright 2013
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (c) 2011 IDS GmbH, Germany
 * Sergej Stepanov <ste@ids.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_MPC831x
#define CONFIG_MPC8313
#define CONFIG_IDS8313

#define CONFIG_SYS_GENERIC_BOARD

#define CONFIG_FSL_ELBC

#define CONFIG_MISC_INIT_R

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT	\
	"\nEnter password - autoboot in %d seconds...\n", CONFIG_BOOTDELAY
#define CONFIG_AUTOBOOT_DELAY_STR	"ids"
#define CONFIG_BOOT_RETRY_TIME		900
#define CONFIG_BOOT_RETRY_MIN		30
#define CONFIG_BOOTDELAY		1
#define CONFIG_RESET_TO_RETRY

#define CONFIG_83XX_CLKIN		66000000	/* in Hz */
#define CONFIG_SYS_CLK_FREQ		CONFIG_83XX_CLKIN

#define CONFIG_SYS_IMMR		0xF0000000

#define CONFIG_SYS_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CONFIG_SYS_ACR_RPTCNT		3	/* Arbiter repeat count (0-7) */

/*
 * Hardware Reset Configuration Word
 * if CLKIN is 66.000MHz, then
 * CSB = 132MHz, CORE = 264MHz, DDRC = 264MHz, LBC = 132MHz
 */
#define CONFIG_SYS_HRCW_LOW (0x20000000 /* reserved, must be set */ |\
			     HRCWL_DDR_TO_SCB_CLK_2X1 |\
			     HRCWL_CSB_TO_CLKIN_2X1 |\
			     HRCWL_CORE_TO_CSB_2X1)

#define CONFIG_SYS_HRCW_HIGH	(HRCWH_PCI_HOST |\
				 HRCWH_CORE_ENABLE |\
				 HRCWH_FROM_0XFFF00100 |\
				 HRCWH_BOOTSEQ_DISABLE |\
				 HRCWH_SW_WATCHDOG_DISABLE |\
				 HRCWH_ROM_LOC_LOCAL_8BIT |\
				 HRCWH_RL_EXT_LEGACY |\
				 HRCWH_TSEC1M_IN_MII |\
				 HRCWH_TSEC2M_IN_MII |\
				 HRCWH_BIG_ENDIAN)

#define CONFIG_SYS_SICRH	0x00000000
#define CONFIG_SYS_SICRL	(SICRL_LBC | SICRL_SPI_D)

#define CONFIG_HWCONFIG

#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK |\
				 HID0_ENABLE_INSTRUCTION_CACHE |\
				 HID0_DISABLE_DYNAMIC_POWER_MANAGMENT)

#define CONFIG_SYS_HID2	(HID2_HBE | 0x00020000)

/*
 * Definitions for initial stack pointer and data area (in DCACHE )
 */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000  /* End of used area in DPRAM */
#define CONFIG_SYS_GBL_DATA_SIZE	0x100
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE \
					 - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Local Bus LCRR and LBCR regs
 */
#define CONFIG_SYS_LCRR_EADC		LCRR_EADC_1
#define CONFIG_SYS_LCRR_CLKDIV		LCRR_CLKDIV_2
#define CONFIG_SYS_LBC_LBCR		(0x00040000 |\
					 (0xFF << LBCR_BMT_SHIFT) |\
					 0xF)

#define CONFIG_SYS_LBC_MRTPR		0x20000000

/*
 * Internal Definitions
 */
/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_BASE		0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE

/*
 * Manually set up DDR parameters,
 * as this board has not the SPD connected to I2C.
 */
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#define CONFIG_SYS_DDR_CONFIG		(CSCONFIG_EN |\
					 0x00010000 |\
					 CSCONFIG_ROW_BIT_13 |\
					 CSCONFIG_COL_BIT_10)

#define CONFIG_SYS_DDR_CONFIG_256	(CONFIG_SYS_DDR_CONFIG | \
					 CSCONFIG_BANK_BIT_3)

#define CONFIG_SYS_DDR_TIMING_3	(1 << 16)	/* ext refrec */
#define CONFIG_SYS_DDR_TIMING_0	((3 << TIMING_CFG0_RWT_SHIFT) |\
				(3 << TIMING_CFG0_WRT_SHIFT) |\
				(3 << TIMING_CFG0_RRT_SHIFT) |\
				(3 << TIMING_CFG0_WWT_SHIFT) |\
				(6 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) |\
				(2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) |\
				(8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) | \
				(2 << TIMING_CFG0_MRS_CYC_SHIFT))
#define CONFIG_SYS_DDR_TIMING_1	((4 << TIMING_CFG1_PRETOACT_SHIFT) |\
				(12 << TIMING_CFG1_ACTTOPRE_SHIFT) |\
				(4 << TIMING_CFG1_ACTTORW_SHIFT) |\
				(7 << TIMING_CFG1_CASLAT_SHIFT) |\
				(4 << TIMING_CFG1_REFREC_SHIFT) |\
				(4 << TIMING_CFG1_WRREC_SHIFT) |\
				(2 << TIMING_CFG1_ACTTOACT_SHIFT) |\
				(2 << TIMING_CFG1_WRTORD_SHIFT))
#define CONFIG_SYS_DDR_TIMING_2	((1 << TIMING_CFG2_ADD_LAT_SHIFT) |\
				(5 << TIMING_CFG2_CPO_SHIFT) |\
				(4 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) |\
				(2 << TIMING_CFG2_RD_TO_PRE_SHIFT) |\
				(0 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) |\
				(1 << TIMING_CFG2_CKE_PLS_SHIFT) |\
				(6 << TIMING_CFG2_FOUR_ACT_SHIFT))

#define CONFIG_SYS_DDR_INTERVAL	((0x800 << SDRAM_INTERVAL_REFINT_SHIFT) |\
				(0x800 << SDRAM_INTERVAL_BSTOPRE_SHIFT))

#define CONFIG_SYS_SDRAM_CFG		(SDRAM_CFG_SREN |\
					 SDRAM_CFG_2T_EN | SDRAM_CFG_HSE |\
					 SDRAM_CFG_DBW_32 |\
					 SDRAM_CFG_SDRAM_TYPE_DDR2)

#define CONFIG_SYS_SDRAM_CFG2		0x00401000
#define CONFIG_SYS_DDR_MODE		((0x0448 << SDRAM_MODE_ESD_SHIFT) |\
					 (0x0242 << SDRAM_MODE_SD_SHIFT))
#define CONFIG_SYS_DDR_MODE_2		0x00000000
#define CONFIG_SYS_DDR_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075
#define CONFIG_SYS_DDRCDR_VALUE	(DDRCDR_EN |\
					 DDRCDR_PZ_NOMZ |\
					 DDRCDR_NZ_NOMZ |\
					 DDRCDR_ODT |\
					 DDRCDR_M_ODR |\
					 DDRCDR_Q_DRN)

/*
 * on-board devices
 */
#define CONFIG_TSEC1
#define CONFIG_TSEC2
#define CONFIG_TSEC_ENET
#define CONFIG_NET_MULTI
#define CONFIG_HARD_SPI
#define CONFIG_HARD_I2C

/*
 * NOR FLASH setup
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CONFIG_FLASH_SHOW_PROGRESS	50
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

#define CONFIG_SYS_FLASH_BASE		0xFF800000
#define CONFIG_SYS_FLASH_SIZE		8
#define CONFIG_SYS_FLASH_PROTECTION

#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_LBLAWAR0_PRELIM	0x80000016

#define CONFIG_SYS_BR0_PRELIM		(CONFIG_SYS_FLASH_BASE |\
					 BR_PS_8 |\
					 BR_MS_GPCM |\
					 BR_V)

#define CONFIG_SYS_OR0_PRELIM		(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) |\
					 OR_GPCM_SCY_10 |\
					 OR_GPCM_EHTR |\
					 OR_GPCM_TRLX |\
					 OR_GPCM_CSNT |\
					 OR_GPCM_EAD)
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	128

#define CONFIG_SYS_FLASH_ERASE_TOUT	60000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500

/*
 * NAND FLASH setup
 */
#define CONFIG_SYS_NAND_BASE		0xE1000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_NAND_FSL_ELBC
#define CONFIG_SYS_NAND_PAGE_SIZE	(2048)
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 << 10)
#define NAND_CACHE_PAGES		64

#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_LBLAWAR1_PRELIM	0x8000000E
#define CONFIG_SYS_NAND_LBLAWBAR_PRELIM CONFIG_SYS_LBLAWBAR1_PRELIM
#define CONFIG_SYS_NAND_LBLAWAR_PRELIM	CONFIG_SYS_LBLAWAR1_PRELIM

#define CONFIG_SYS_BR1_PRELIM	((CONFIG_SYS_NAND_BASE) |\
				 (2<<BR_DECC_SHIFT) |\
				 BR_PS_8 |\
				 BR_MS_FCM |\
				 BR_V)

#define CONFIG_SYS_OR1_PRELIM	(0xFFFF8000 |\
				 OR_FCM_PGS |\
				 OR_FCM_CSCT |\
				 OR_FCM_CST |\
				 OR_FCM_CHT |\
				 OR_FCM_SCY_4 |\
				 OR_FCM_TRLX |\
				 OR_FCM_EHTR |\
				 OR_FCM_RST)

/*
 * MRAM setup
 */
#define CONFIG_SYS_MRAM_BASE		0xE2000000
#define CONFIG_SYS_MRAM_SIZE		0x20000	/* 128 Kb */
#define CONFIG_SYS_LBLAWBAR2_PRELIM	CONFIG_SYS_MRAM_BASE
#define CONFIG_SYS_LBLAWAR2_PRELIM	0x80000010	/* 128 Kb  */

#define CONFIG_SYS_OR_TIMING_MRAM

#define CONFIG_SYS_BR2_PRELIM		(CONFIG_SYS_MRAM_BASE |\
					 BR_PS_8 |\
					 BR_MS_GPCM |\
					 BR_V)

#define CONFIG_SYS_OR2_PRELIM		0xFFFE0C74

/*
 * CPLD setup
 */
#define CONFIG_SYS_CPLD_BASE		0xE3000000
#define CONFIG_SYS_CPLD_SIZE		0x8000
#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_CPLD_BASE
#define CONFIG_SYS_LBLAWAR3_PRELIM	0x8000000E

#define CONFIG_SYS_OR_TIMING_MRAM

#define CONFIG_SYS_BR3_PRELIM		(CONFIG_SYS_CPLD_BASE |\
					 BR_PS_8 |\
					 BR_MS_GPCM |\
					 BR_V)

#define CONFIG_SYS_OR3_PRELIM		0xFFFF8814

/*
 * HW-Watchdog
 */
#define CONFIG_WATCHDOG		1
#define CONFIG_SYS_WATCHDOG_VALUE	0xFFFF

/*
 * I2C setup
 */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3100
#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR	0x51

/*
 * SPI setup
 */
#ifdef CONFIG_HARD_SPI
#define CONFIG_MPC8XXX_SPI
#define CONFIG_CMD_SPI
#define CONFIG_SYS_GPIO1_PRELIM
#define CONFIG_SYS_GPIO1_DIR		0x00000001
#define CONFIG_SYS_GPIO1_DAT		0x00000001
#endif

/*
 * Ethernet setup
 */
#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME		"TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR			0x1
#define TSEC1_FLAGS			TSEC_GIGABIT
#define TSEC1_PHYIDX			0
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME		"TSEC1"
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define TSEC2_PHY_ADDR			0x3
#define TSEC2_FLAGS			TSEC_GIGABIT
#define TSEC2_PHYIDX			0
#endif
#define CONFIG_ETHPRIME		"TSEC1"

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}
#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)
#define CONFIG_SYS_NS16550_CLK		(CONFIG_83XX_CLKIN * 2)

#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_SYS_SCCR_USBDRCM	3

/*
 * BAT's
 */
#define CONFIG_HIGH_BATS

/* DDR @ 0x00000000 */
#define CONFIG_SYS_IBAT0L		(CONFIG_SYS_SDRAM_BASE |\
					 BATL_PP_10)
#define CONFIG_SYS_IBAT0U		(CONFIG_SYS_SDRAM_BASE |\
					 BATU_BL_256M |\
					 BATU_VS |\
					 BATU_VP)
#define CONFIG_SYS_DBAT0L		CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U		CONFIG_SYS_IBAT0U

/* Initial RAM @ 0xFD000000 */
#define CONFIG_SYS_IBAT1L		(CONFIG_SYS_INIT_RAM_ADDR |\
					 BATL_PP_10 |\
					 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT1U		(CONFIG_SYS_INIT_RAM_ADDR |\
					 BATU_BL_256K |\
					 BATU_VS |\
					 BATU_VP)
#define CONFIG_SYS_DBAT1L		CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U		CONFIG_SYS_IBAT1U

/* FLASH @ 0xFF800000 */
#define CONFIG_SYS_IBAT2L		(CONFIG_SYS_FLASH_BASE |\
					 BATL_PP_10 |\
					 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT2U		(CONFIG_SYS_FLASH_BASE |\
					 BATU_BL_8M |\
					 BATU_VS |\
					 BATU_VP)
#define CONFIG_SYS_DBAT2L		(CONFIG_SYS_FLASH_BASE |\
					 BATL_PP_10 |\
					 BATL_CACHEINHIBIT |\
					 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U		CONFIG_SYS_IBAT2U

#define CONFIG_SYS_IBAT3L		(0)
#define CONFIG_SYS_IBAT3U		(0)
#define CONFIG_SYS_DBAT3L		CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U		CONFIG_SYS_IBAT3U

#define CONFIG_SYS_IBAT4L		(0)
#define CONFIG_SYS_IBAT4U		(0)
#define CONFIG_SYS_DBAT4L		CONFIG_SYS_IBAT4L
#define CONFIG_SYS_DBAT4U		CONFIG_SYS_IBAT4U

/* IMMRBAR @ 0xF0000000 */
#define CONFIG_SYS_IBAT5L		(CONFIG_SYS_IMMR |\
					 BATL_PP_10 |\
					 BATL_CACHEINHIBIT |\
					 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT5U		(CONFIG_SYS_IMMR |\
					 BATU_BL_128M |\
					 BATU_VS |\
					 BATU_VP)
#define CONFIG_SYS_DBAT5L		CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U		CONFIG_SYS_IBAT5U

/* NAND-Flash @ 0xE1000000, MRAM @ 0xE2000000, CPLD @ 0xE3000000 */
#define CONFIG_SYS_IBAT6L		(0xE0000000 |\
					 BATL_PP_10 |\
					 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT6U		(0xE0000000 |\
					 BATU_BL_256M |\
					 BATU_VS |\
					 BATU_VP)
#define CONFIG_SYS_DBAT6L		CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U		CONFIG_SYS_IBAT6U

#define CONFIG_SYS_IBAT7L		(0)
#define CONFIG_SYS_IBAT7U		(0)
#define CONFIG_SYS_DBAT7L		CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U		CONFIG_SYS_IBAT7U

/*
 * U-Boot environment setup
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_NFS
#define CONFIG_CMD_NAND
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_CMD_EDITENV
#define CONFIG_CMD_JFFS2
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_OF_STDOUT_VIA_ALIAS

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(8 * 1024 * 1024)

/*
 * Environment Configuration
 */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE \
				+ CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SIZE		0x20000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)


#define CONFIG_NETDEV			eth1
#define CONFIG_HOSTNAME		ids8313
#define CONFIG_ROOTPATH		"/opt/eldk-4.2/ppc_6xx"
#define CONFIG_BOOTFILE		"ids8313/uImage"
#define CONFIG_UBOOTPATH		"ids8313/u-boot.bin"
#define CONFIG_FDTFILE			"ids8313/ids8313.dtb"
#define CONFIG_LOADADDR		0x400000
#define CONFIG_CMD_ENV_FLAGS
#define CONFIG_ENV_FLAGS_LIST_STATIC "ethaddr:mo,eth1addr:mo"

#define CONFIG_BAUDRATE		115200

/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ		(256 << 20)

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					 + sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser	*/
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define CONFIG_SYS_MEMTEST_START	0x00001000
#define CONFIG_SYS_MEMTEST_END		0x00C00000

#define CONFIG_SYS_LOAD_ADDR		0x100000
#define CONFIG_MII
#define CONFIG_LOADS_ECHO
#define CONFIG_TIMESTAMP
#define CONFIG_PREBOOT			"echo;" \
					"echo Type \\\"run nfsboot\\\" " \
					"to mount root filesystem over NFS;echo"
#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND		"run boot_cramfs"
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_JFFS2_NAND
#define CONFIG_JFFS2_DEV		"0"

/* mtdparts command line support */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_FLASH_CFI_MTD
#define CONFIG_MTD_DEVICE
#define MTDIDS_DEFAULT		"nor0=ff800000.flash,nand0=e1000000.flash"
#define MTDPARTS_DEFAULT	"mtdparts=ff800000.flash:7m(dum)," \
					"768k(BOOT-BIN)," \
					"128k(BOOT-ENV),128k(BOOT-REDENV);" \
					"e1000000.flash:-(ubi)"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" __stringify(CONFIG_NETDEV) "\0"			\
	"ethprime=TSEC1\0"						\
	"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"			\
	"tftpflash=tftpboot ${loadaddr} ${uboot}; "			\
		"protect off " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" +${filesize}; "					\
		"erase " __stringify(CONFIG_SYS_TEXT_BASE)		\
		" +${filesize}; "					\
		"cp.b ${loadaddr} " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" ${filesize}; "					\
		"protect on " __stringify(CONFIG_SYS_TEXT_BASE)		\
		" +${filesize}; "					\
		"cmp.b ${loadaddr} " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" ${filesize}\0"					\
	"console=ttyS0\0"						\
	"fdtaddr=0x780000\0"						\
	"kernel_addr=ff800000\0"					\
	"fdtfile=" __stringify(CONFIG_FDTFILE) "\0"			\
	"setbootargs=setenv bootargs "					\
		"root=${rootdev} rw console=${console},"		\
			"${baudrate} ${othbootargs}\0"			\
	"setipargs=setenv bootargs root=${rootdev} rw "			\
			"nfsroot=${serverip}:${rootpath} "		\
			"ip=${ipaddr}:${serverip}:${gatewayip}:"	\
			"${netmask}:${hostname}:${netdev}:off "		\
			"console=${console},${baudrate} ${othbootargs}\0" \
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"mtdids=" MTDIDS_DEFAULT "\0"					\
	"mtdparts=" MTDPARTS_DEFAULT "\0"				\
	"\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv rootdev /dev/nfs;"					\
	"run setipargs;run addmtd;"					\
	"tftp ${loadaddr} ${bootfile};"				\
	"tftp ${fdtaddr} ${fdtfile};"					\
	"fdt addr ${fdtaddr};"						\
	"bootm ${loadaddr} - ${fdtaddr}"

/* UBI Support */
#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_RBTREE
#define CONFIG_LZO
#define CONFIG_MTD_PARTITIONS

/* bootcount support */
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_I2C
#define CONFIG_BOOTCOUNT_ALEN	1
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x9

#define CONFIG_VERSION_VARIABLE

#define CONFIG_FIT
#define CONFIG_FIT_SIGNATURE
#define CONFIG_IMAGE_FORMAT_LEGACY
#define CONFIG_CMD_FDT
#define CONFIG_CMD_HASH
#define CONFIG_RSA
#define CONFIG_SHA1
#define CONFIG_SHA256

#endif	/* __CONFIG_H */

/*
 * Copyright (C) Sheldon Instruments, Inc. 2008
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*
 * simpc8313 board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_NAND_U_BOOT

#define CONFIG_E300			1
#define CONFIG_MPC831x			1
#define CONFIG_MPC8313			1

#define CONFIG_SYS_NAND_U_BOOT_SIZE	(512 << 10)
#define CONFIG_SYS_NAND_U_BOOT_DST	0x00100000
#define CONFIG_SYS_NAND_U_BOOT_START	0x00100100
#define CONFIG_SYS_NAND_U_BOOT_RELOC	0x00010000
#define CONFIG_SYS_NAND_U_BOOT_RELOC_SP (CONFIG_SYS_NAND_U_BOOT_RELOC + 0x10000)

#define CONFIG_SYS_TEXT_BASE	0x00100000 /* CONFIG_SYS_NAND_U_BOOT_DST */
#define CONFIG_SYS_TEXT_BASE_SPL 0xfff00000

#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE_SPL /* start of monitor */
#else
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

#define CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_FSL_ELBC			1

#define CONFIG_MISC_INIT_R

/*
 * On-board devices
 *
 * TSEC1 is Marvell PHY 88E1118
 */

#define CONFIG_SYS_33MHZ

#define CONFIG_83XX_CLKIN		33333333	/* in Hz */

#define CONFIG_SYS_CLK_FREQ		CONFIG_83XX_CLKIN

#define CONFIG_SYS_IMMR			0xE0000000

#if defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CONFIG_DEFAULT_IMMR		CONFIG_SYS_IMMR
#endif

#define CONFIG_SYS_MEMTEST_START	0x00001000
#define CONFIG_SYS_MEMTEST_END		0x07f00000

#define CONFIG_SYS_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CONFIG_SYS_ACR_RPTCNT	3	/* Arbiter repeat count (0-7) */

/*
 * Device configurations
 */
#define CONFIG_TSEC1

/*
 * DDR Setup
 */
					/* DDR is system memory*/
#define CONFIG_SYS_DDR_BASE		0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE

#define CONFIG_VERY_BIG_RAM
#define CONFIG_MAX_MEM_MAPPED		(512 << 20)

#define CONFIG_SYS_DDRCDR		(DDRCDR_EN \
					| DDRCDR_PZ_NOMZ \
					| DDRCDR_NZ_NOMZ \
					| DDRCDR_M_ODR)
					/* 0x73000002 TODO ODR & DRN ? */

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_NO_FLASH

#if !defined(CONFIG_NAND_SPL)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)	/* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024)	/* Reserved for malloc */

/*
 * Local Bus LCRR and LBCR regs
 */
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_EADC	LCRR_EADC_1
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_2
#define CONFIG_SYS_LBC_LBCR	(0x00040000 /* TODO */ \
				| (0xFF << LBCR_BMT_SHIFT) \
				| 0xF)	/* 0x0004ff0f */

				/* LB refresh timer prescal, 266MHz/32 */
#define CONFIG_SYS_LBC_MRTPR	0x20000000

/* drivers/mtd/nand/nand.c */
#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_NAND_BASE		0xFFF00000
#else
#define CONFIG_SYS_NAND_BASE		0xE2800000
#endif
#define CONFIG_SYS_FPGA_BASE		0xFF000000

#define CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_NAND_FSL_ELBC		1

#define CONFIG_SYS_NAND_BR_PRELIM	(CONFIG_SYS_NAND_BASE \
				| BR_DECC_CHK_GEN	/* Use HW ECC */ \
				| BR_PS_8		/* 8 bit Port */ \
				| BR_MS_FCM		/* MSEL = FCM */ \
				| BR_V)			/* valid */

#ifdef CONFIG_NAND_SP
#define CONFIG_SYS_NAND_OR_PRELIM	(OR_AM_32KB \
					| OR_FCM_CSCT \
					| OR_FCM_CST \
					| OR_FCM_CHT \
					| OR_FCM_SCY_1 \
					| OR_FCM_TRLX \
					| OR_FCM_EHTR)
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_32KB)
#define CONFIG_SYS_NAND_PAGE_SIZE	512	/* NAND chip page size */
					/* NAND chip block size */
#define CONFIG_SYS_NAND_BLOCK_SIZE	(16 << 10)
#define NAND_CACHE_PAGES		32
#elif defined(CONFIG_NAND_LP)
#define CONFIG_SYS_NAND_OR_PRELIM	(OR_AM_256KB \
					| OR_FCM_PGS \
					| OR_FCM_CSCT \
					| OR_FCM_CST \
					| OR_FCM_CHT \
					| OR_FCM_SCY_1 \
					| OR_FCM_TRLX \
					| OR_FCM_EHTR)
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_256KB)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048	/* NAND chip page size */
					/* NAND chip block size */
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 << 10)
#define NAND_CACHE_PAGES		64
#else
#error Page size of NAND not defined.
#endif /* CONFIG_NAND_SP */

#define CONFIG_SYS_NAND_U_BOOT_OFFS	CONFIG_SYS_NAND_BLOCK_SIZE

#define CONFIG_SYS_BR0_PRELIM		CONFIG_SYS_NAND_BR_PRELIM
#define CONFIG_SYS_OR0_PRELIM		CONFIG_SYS_NAND_OR_PRELIM

#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_NAND_BASE

#define CONFIG_SYS_NAND_LBLAWBAR_PRELIM	CONFIG_SYS_LBLAWBAR0_PRELIM
#define CONFIG_SYS_NAND_LBLAWAR_PRELIM	CONFIG_SYS_LBLAWAR0_PRELIM

#define CONFIG_SYS_BR1_PRELIM		(CONFIG_SYS_FPGA_BASE \
					| BR_PS_16 \
					| BR_MS_UPMA \
					| BR_V)
#define CONFIG_SYS_OR1_PRELIM		(OR_AM_2MB \
					| OR_UPM_BCTLD)

#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_FPGA_BASE
#define CONFIG_SYS_LBLAWAR1_PRELIM	(LBLAWAR_EN | LBLAWAR_2MB)

/*
 * JFFS2 configuration
 */
#define CONFIG_JFFS2_NAND
#define CONFIG_JFFS2_DEV	"nand0"

/* mtdparts command line support */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define MTDIDS_DEFAULT		"nand0=nand0"
#define MTDPARTS_DEFAULT	"mtdparts=nand0:2M(u-boot),6M(kernel),-(jffs2)"

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#ifdef CONFIG_NAND_SPL
#define CONFIG_NS16550_MIN_FUNCTIONS
#endif

#define CONFIG_SYS_BAUDRATE_TABLE	\
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1		(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2		(CONFIG_SYS_IMMR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	0x90000000
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BASE		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x00100000	/* 1M */

#define CONFIG_PCI_PNP			/* do pci plug-and-play */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID	0x1057	/* Motorola */

/*
 * TSEC
 */
#define CONFIG_TSEC_ENET		/* TSEC ethernet support */

#define CONFIG_GMII			/* MII PHY management */

#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME		"TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET		0x24000
#define TSEC1_PHY_ADDR			0x0
#define TSEC1_FLAGS			TSEC_GIGABIT
#define TSEC1_PHYIDX			0
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME		"TSEC1"
#define CONFIG_SYS_TSEC2_OFFSET		0x25000
#define TSEC2_PHY_ADDR			4
#define TSEC2_FLAGS			TSEC_GIGABIT
#define TSEC2_PHYIDX			0
#endif


/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME			"TSEC1"

/*
 * Configure on-board RTC
 */
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C_RTC_ADDR		0x68

/*
 * Environment
 */
#if defined(CONFIG_NAND_U_BOOT)
	#define CONFIG_ENV_IS_IN_NAND	1
	#define CONFIG_ENV_OFFSET	(768 * 1024)
	#define CONFIG_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE
	#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
	#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE
	#define CONFIG_ENV_RANGE	(CONFIG_ENV_SECT_SIZE * 4)
	#define CONFIG_ENV_OFFSET_REDUND	\
					(CONFIG_ENV_OFFSET + CONFIG_ENV_RANGE)
#elif !defined(CONFIG_SYS_RAMBOOT)
	#define CONFIG_ENV_IS_IN_FLASH	1
	#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
	#define CONFIG_ENV_SECT_SIZE	0x10000	/* 64K(one sector) for env */
	#define CONFIG_ENV_SIZE		0x2000

/* Address and size of Redundant Environment Sector */
#else
	#define CONFIG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
	#define CONFIG_ENV_SIZE		0x2000
#endif

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
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_FLASH

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_JFFS2

#if defined(CONFIG_SYS_RAMBOOT) && !defined(CONFIG_NAND_U_BOOT)
	#undef CONFIG_CMD_SAVEENV
	#undef CONFIG_CMD_LOADS
#endif

#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_AUTO_COMPLETE	/* add autocompletion support   */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */

#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE		\
				+ sizeof(CONFIG_SYS_PROMPT)	\
				+ 16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
				/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

#define CONFIG_SYS_RCWH_PCIHOST	0x80000000	/* PCIHOST */

#define CONFIG_SYS_HRCW_LOW	(HRCWL_LCL_BUS_TO_SCB_CLK_1X1	\
				| 0x20000000 /* reserved */	\
				| HRCWL_DDR_TO_SCB_CLK_2X1	\
				| HRCWL_CSB_TO_CLKIN_4X1	\
				| HRCWL_CORE_TO_CSB_2_5X1)

#define CONFIG_SYS_NS16550_CLK	(CONFIG_83XX_CLKIN * 4)

#define CONFIG_SYS_HRCW_HIGH_BASE	(HRCWH_PCI_HOST	\
				| HRCWH_PCI1_ARBITER_ENABLE	\
				| HRCWH_CORE_ENABLE		\
				| HRCWH_BOOTSEQ_DISABLE		\
				| HRCWH_SW_WATCHDOG_DISABLE	\
				| HRCWH_TSEC1M_IN_RGMII		\
				| HRCWH_TSEC2M_IN_RGMII		\
				| HRCWH_BIG_ENDIAN		\
				| HRCWH_LALE_NORMAL)

#ifdef CONFIG_NAND_LP
#define CONFIG_SYS_HRCW_HIGH	(CONFIG_SYS_HRCW_HIGH_BASE	\
				| HRCWH_FROM_0XFFF00100		\
				| HRCWH_ROM_LOC_NAND_LP_8BIT	\
				| HRCWH_RL_EXT_NAND)
#else
#define CONFIG_SYS_HRCW_HIGH	(CONFIG_SYS_HRCW_HIGH_BASE	\
				| HRCWH_FROM_0XFFF00100		\
				| HRCWH_ROM_LOC_NAND_SP_8BIT	\
				| HRCWH_RL_EXT_NAND)
#endif

/* System IO Config */
#define CONFIG_SYS_SICRH	(SICRH_ETSEC2_B	\
				| SICRH_ETSEC2_C	\
				| SICRH_ETSEC2_D	\
				| SICRH_ETSEC2_E	\
				| SICRH_ETSEC2_F	\
				| SICRH_ETSEC2_G	\
				| SICRH_TSOBI1		\
				| SICRH_TSOBI2)
#define CONFIG_SYS_SICRL	(SICRL_LBC		\
				| SICRL_USBDR_10	\
				| SICRL_ETSEC2_A)

#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK \
				| HID0_ENABLE_INSTRUCTION_CACHE \
				| HID0_ENABLE_DYNAMIC_POWER_MANAGMENT)

#define CONFIG_SYS_HID2		HID2_HBE

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR @ 0x00000000 */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE | BATL_PP_RW)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT1L	((CONFIG_SYS_SDRAM_BASE + 0x10000000) \
				| BATL_PP_RW)
#define CONFIG_SYS_IBAT1U	((CONFIG_SYS_SDRAM_BASE + 0x10000000) \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* PCI @ 0x80000000 */
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_PCI1_MEM_BASE | BATL_PP_RW)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_PCI1_MEM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* PCI2 not supported on 8313 */
#define CONFIG_SYS_IBAT4L	(0)
#define CONFIG_SYS_IBAT4U	(0)

/* IMMRBAR @ 0xE0000000, PCI IO @ 0xE2000000 */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_IMMR \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_IMMR \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* SDRAM @ 0xF0000000, stack in DCACHE 0xFDF00000 & FLASH @ 0xFE000000 */
#define CONFIG_SYS_IBAT6L	(0xF0000000 \
				| BATL_PP_RW \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT6U	(0xF0000000 \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)

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

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_NETDEV		"eth1"

#define CONFIG_HOSTNAME		simpc8313
#define CONFIG_ROOTPATH		"/tftpboot/"
#define CONFIG_BOOTFILE		"/tftpboot/uImage"
				/* U-Boot image on TFTP server */
#define CONFIG_UBOOTPATH	"u-boot-nand.bin"
#define CONFIG_FDTFILE		"simpc8313.dtb"

				/* default location for tftp and bootm */
#define CONFIG_LOADADDR		500000
#define CONFIG_BOOTDELAY	5	/* 5 second delay */
#define CONFIG_BAUDRATE		115200

#define CONFIG_BOOTCOMMAND	"nand read $loadaddr kernel 600000;" \
					"bootm $loadaddr - $fdtaddr"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" CONFIG_NETDEV "\0"					\
	"ethprime=TSEC1\0"						\
	"uboot=" CONFIG_UBOOTPATH "\0"					\
	"tftpflash=tftpboot $loadaddr $uboot; "				\
		"protect off " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" +$filesize; "	\
		"erase " __stringify(CONFIG_SYS_TEXT_BASE)		\
			" +$filesize; "	\
		"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" $filesize; "	\
		"protect on " __stringify(CONFIG_SYS_TEXT_BASE)		\
			" +$filesize; "	\
		"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" $filesize\0"	\
	"fdtaddr=ae0000\0"						\
	"fdtfile=" CONFIG_FDTFILE "\0"					\
	"console=ttyS0\0"						\
	"setbootargs=setenv bootargs "					\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"setipargs=setenv bootargs nfsroot=$serverip:$rootpath "	\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:"	\
							"$netdev:off "	\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"load_uboot=tftp 100000 u-boot-nand.bin\0"			\
	"burn_uboot=nand erase u-boot 80000; "				\
		"nand write 100000 u-boot $filesize\0"			\
	"update_uboot=run load_uboot;run burn_uboot\0"			\
	"mtdids=nand0=nand0\0"						\
	"mtdparts=mtdparts=nand0:2M(u-boot),6M(kernel),-(jffs2)\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
	"bootargs=root=/dev/mtdblock2 rootfstype=jffs2 rw "		\
		"console=ttyS0,115200\0"				\
	""

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv rootdev /dev/nfs;"					\
	"run setbootargs;"						\
	"run setipargs;"						\
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

#endif	/* __CONFIG_H */

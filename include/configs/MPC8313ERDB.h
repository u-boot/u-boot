/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006, 2010.
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
#define CONFIG_MPC83xx		1
#define CONFIG_MPC831x		1
#define CONFIG_MPC8313		1
#define CONFIG_MPC8313ERDB	1

#define CONFIG_SYS_NAND_U_BOOT_SIZE  (512 << 10)
#define CONFIG_SYS_NAND_U_BOOT_DST   0x00100000
#define CONFIG_SYS_NAND_U_BOOT_START 0x00100100
#define CONFIG_SYS_NAND_U_BOOT_OFFS  16384
#define CONFIG_SYS_NAND_U_BOOT_RELOC 0x00010000
#define CONFIG_SYS_NAND_U_BOOT_RELOC_SP (CONFIG_SYS_NAND_U_BOOT_RELOC + 0x10000)

#ifdef CONFIG_NAND_U_BOOT
#define CONFIG_SYS_TEXT_BASE	0x00100000 /* CONFIG_SYS_NAND_U_BOOT_DST */
#define CONFIG_SYS_TEXT_BASE_SPL 0xfff00000
#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE_SPL /* start of monitor */
#endif /* CONFIG_NAND_SPL */
#endif /* CONFIG_NAND_U_BOOT */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFE000000
#endif

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

#define CONFIG_PCI
#define CONFIG_FSL_ELBC 1

#define CONFIG_MISC_INIT_R

/*
 * On-board devices
 *
 * TSEC1 is VSC switch
 * TSEC2 is SoC TSEC
 */
#define CONFIG_VSC7385_ENET
#define CONFIG_TSEC2

#ifdef CONFIG_SYS_66MHZ
#define CONFIG_83XX_CLKIN	66666667	/* in Hz */
#elif defined(CONFIG_SYS_33MHZ)
#define CONFIG_83XX_CLKIN	33333333	/* in Hz */
#else
#error Unknown oscillator frequency.
#endif

#define CONFIG_SYS_CLK_FREQ	CONFIG_83XX_CLKIN

#define CONFIG_BOARD_EARLY_INIT_F		/* call board_early_init_f */
#define CONFIG_BOARD_EARLY_INIT_R		/* call board_early_init_r */

#define CONFIG_SYS_IMMR		0xE0000000

#if defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CONFIG_DEFAULT_IMMR	CONFIG_SYS_IMMR
#endif

#define CONFIG_SYS_MEMTEST_START	0x00001000
#define CONFIG_SYS_MEMTEST_END		0x07f00000

/* Early revs of this board will lock up hard when attempting
 * to access the PMC registers, unless a JTAG debugger is
 * connected, or some resistor modifications are made.
 */
#define CONFIG_SYS_8313ERDB_BROKEN_PMC 1

#define CONFIG_SYS_ACR_PIPE_DEP	3	/* Arbiter pipeline depth (0-3) */
#define CONFIG_SYS_ACR_RPTCNT		3	/* Arbiter repeat count (0-7) */

/*
 * Device configurations
 */

/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET

#define CONFIG_TSEC1

/* The flash address and size of the VSC7385 firmware image */
#define CONFIG_VSC7385_IMAGE		0xFE7FE000
#define CONFIG_VSC7385_IMAGE_SIZE	8192

#endif

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_BASE		0x00000000 /* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE

/*
 * Manually set up DDR parameters, as this board does not
 * seem to have the SPD connected to I2C.
 */
#define CONFIG_SYS_DDR_SIZE	128		/* MB */
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
				| CSCONFIG_ODT_RD_NEVER \
				| CSCONFIG_ODT_WR_ONLY_CURRENT \
				| CSCONFIG_ROW_BIT_13 \
				| CSCONFIG_COL_BIT_10)
				/* 0x80010102 */

#define CONFIG_SYS_DDR_TIMING_3	0x00000000
#define CONFIG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (0 << TIMING_CFG0_RRT_SHIFT) \
				| (0 << TIMING_CFG0_WWT_SHIFT) \
				| (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x00220802 */
#define CONFIG_SYS_DDR_TIMING_1	((3 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (8 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (3 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (5 << TIMING_CFG1_CASLAT_SHIFT) \
				| (10 << TIMING_CFG1_REFREC_SHIFT) \
				| (3 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x3835a322 */
#define CONFIG_SYS_DDR_TIMING_2	((1 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (5 << TIMING_CFG2_CPO_SHIFT) \
				| (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (6 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x129048c6 */ /* P9-45,may need tuning */
#define CONFIG_SYS_DDR_INTERVAL	((1296 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (1280 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x05100500 */
#if defined(CONFIG_DDR_2T_TIMING)
#define CONFIG_SYS_SDRAM_CFG	(SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_DBW_32 \
				| SDRAM_CFG_2T_EN)
				/* 0x43088000 */
#else
#define CONFIG_SYS_SDRAM_CFG	(SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_DBW_32)
				/* 0x43080000 */
#endif
#define CONFIG_SYS_SDRAM_CFG2		0x00401000
/* set burst length to 8 for 32-bit data path */
#define CONFIG_SYS_DDR_MODE	((0x4448 << SDRAM_MODE_ESD_SHIFT) \
				| (0x0632 << SDRAM_MODE_SD_SHIFT))
				/* 0x44480632 */
#define CONFIG_SYS_DDR_MODE_2	0x8000C000

#define CONFIG_SYS_DDR_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
				/*0x02000000*/
#define CONFIG_SYS_DDRCDR_VALUE	(DDRCDR_EN \
				| DDRCDR_PZ_NOMZ \
				| DDRCDR_NZ_NOMZ \
				| DDRCDR_M_ODR)

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER		/* use the CFI driver */
#define CONFIG_SYS_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		8	/* flash size in MB */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* Use h/w Flash protection. */
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* display empty sectors */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	/* buffer up multiple bytes */

#define CONFIG_SYS_NOR_BR_PRELIM	(CONFIG_SYS_FLASH_BASE \
					| BR_PS_16	/* 16 bit port */ \
					| BR_MS_GPCM	/* MSEL = GPCM */ \
					| BR_V)		/* valid */
#define CONFIG_SYS_NOR_OR_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) \
				| OR_GPCM_XACS \
				| OR_GPCM_SCY_9 \
				| OR_GPCM_EHTR \
				| OR_GPCM_EAD)
				/* 0xFF006FF7	TODO SLOW 16 MB flash size */
					/* window base at flash base */
#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
					/* 16 MB window size */
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_16MB)

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	135	/* sectors per device */

#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE) && \
	!defined(CONFIG_NAND_SPL)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000	/* Initial RAM addr */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN	(384 * 1024)	/* Reserve 384 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024)	/* Reserved for malloc */

/*
 * Local Bus LCRR and LBCR regs
 */
#define CONFIG_SYS_LCRR_EADC	LCRR_EADC_1
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_4
#define CONFIG_SYS_LBC_LBCR	(0x00040000 /* TODO */ \
				| (0xFF << LBCR_BMT_SHIFT) \
				| 0xF)	/* 0x0004ff0f */

				/* LB refresh timer prescal, 266MHz/32 */
#define CONFIG_SYS_LBC_MRTPR	0x20000000  /*TODO */

/* drivers/mtd/nand/nand.c */
#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_NAND_BASE		0xFFF00000
#else
#define CONFIG_SYS_NAND_BASE		0xE2800000
#endif

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITION
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT			"nand0=e2800000.flash"
#define MTDPARTS_DEFAULT		\
	"mtdparts=e2800000.flash:512k(uboot),128k(env),3m@1m(kernel),-(fs)"

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_CMD_NAND 1
#define CONFIG_NAND_FSL_ELBC 1
#define CONFIG_SYS_NAND_BLOCK_SIZE 16384
#define CONFIG_SYS_NAND_WINDOW_SIZE (32 * 1024)


#define CONFIG_SYS_NAND_BR_PRELIM	(CONFIG_SYS_NAND_BASE \
				| BR_DECC_CHK_GEN	/* Use HW ECC */ \
				| BR_PS_8		/* 8 bit port */ \
				| BR_MS_FCM		/* MSEL = FCM */ \
				| BR_V)			/* valid */
#define CONFIG_SYS_NAND_OR_PRELIM	\
				(P2SZ_TO_AM(CONFIG_SYS_NAND_WINDOW_SIZE) \
				| OR_FCM_CSCT \
				| OR_FCM_CST \
				| OR_FCM_CHT \
				| OR_FCM_SCY_1 \
				| OR_FCM_TRLX \
				| OR_FCM_EHTR)
				/* 0xFFFF8396 */

#ifdef CONFIG_NAND_U_BOOT
#define CONFIG_SYS_BR0_PRELIM CONFIG_SYS_NAND_BR_PRELIM
#define CONFIG_SYS_OR0_PRELIM CONFIG_SYS_NAND_OR_PRELIM
#define CONFIG_SYS_BR1_PRELIM CONFIG_SYS_NOR_BR_PRELIM
#define CONFIG_SYS_OR1_PRELIM CONFIG_SYS_NOR_OR_PRELIM
#else
#define CONFIG_SYS_BR0_PRELIM CONFIG_SYS_NOR_BR_PRELIM
#define CONFIG_SYS_OR0_PRELIM CONFIG_SYS_NOR_OR_PRELIM
#define CONFIG_SYS_BR1_PRELIM CONFIG_SYS_NAND_BR_PRELIM
#define CONFIG_SYS_OR1_PRELIM CONFIG_SYS_NAND_OR_PRELIM
#endif

#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_LBLAWAR1_PRELIM	(LBLAWAR_EN | LBLAWAR_32KB)

#define CONFIG_SYS_NAND_LBLAWBAR_PRELIM CONFIG_SYS_LBLAWBAR1_PRELIM
#define CONFIG_SYS_NAND_LBLAWAR_PRELIM CONFIG_SYS_LBLAWAR1_PRELIM

/* local bus write LED / read status buffer (BCSR) mapping */
#define CONFIG_SYS_BCSR_ADDR		0xFA000000
#define CONFIG_SYS_BCSR_SIZE		(32 * 1024)	/* 0x00008000 */
					/* map at 0xFA000000 on LCS3 */
#define CONFIG_SYS_BR3_PRELIM		(CONFIG_SYS_BCSR_ADDR \
					| BR_PS_8	/* 8 bit port */ \
					| BR_MS_GPCM	/* MSEL = GPCM */ \
					| BR_V)		/* valid */
					/* 0xFA000801 */
#define CONFIG_SYS_OR3_PRELIM		(P2SZ_TO_AM(CONFIG_SYS_BCSR_SIZE) \
					| OR_GPCM_CSNT \
					| OR_GPCM_ACS_DIV2 \
					| OR_GPCM_XACS \
					| OR_GPCM_SCY_15 \
					| OR_GPCM_TRLX_SET \
					| OR_GPCM_EHTR_SET \
					| OR_GPCM_EAD)
					/* 0xFFFF8FF7 */
#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_BCSR_ADDR
#define CONFIG_SYS_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_32KB)

/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET

					/* VSC7385 Base address on LCS2 */
#define CONFIG_SYS_VSC7385_BASE		0xF0000000
#define CONFIG_SYS_VSC7385_SIZE		(128 * 1024)	/* 0x00020000 */

#define CONFIG_SYS_BR2_PRELIM		(CONFIG_SYS_VSC7385_BASE \
					| BR_PS_8	/* 8 bit port */ \
					| BR_MS_GPCM	/* MSEL = GPCM */ \
					| BR_V)		/* valid */
#define CONFIG_SYS_OR2_PRELIM		(P2SZ_TO_AM(CONFIG_SYS_VSC7385_SIZE) \
					| OR_GPCM_CSNT \
					| OR_GPCM_XACS \
					| OR_GPCM_SCY_15 \
					| OR_GPCM_SETA \
					| OR_GPCM_TRLX_SET \
					| OR_GPCM_EHTR_SET \
					| OR_GPCM_EAD)
					/* 0xFFFE09FF */

					/* Access window base at VSC7385 base */
#define CONFIG_SYS_LBLAWBAR2_PRELIM	CONFIG_SYS_VSC7385_BASE
#define CONFIG_SYS_LBLAWAR2_PRELIM	(LBLAWAR_EN | LBLAWAR_128KB)

#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

#define CONFIG_MPC83XX_GPIO 1
#define CONFIG_CMD_GPIO 1

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* I2C */
#define CONFIG_HARD_I2C			/* I2C with hardware support*/
#define CONFIG_FSL_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED	400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE	0x7F
#define CONFIG_SYS_I2C_NOPROBES	{ {0, 0x69} } /* Don't probe these addrs */
#define CONFIG_SYS_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C2_OFFSET	0x3100

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
#define CONFIG_SYS_PCI1_IO_BASE	0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS	0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE	0x00100000	/* 1M */

#define CONFIG_PCI_PNP		/* do pci plug-and-play */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1057	/* Motorola */

/*
 * TSEC
 */
#define CONFIG_TSEC_ENET		/* TSEC ethernet support */

#define CONFIG_GMII			/* MII PHY management */

#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR		0x1c
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC1_PHYIDX		0
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define TSEC2_PHY_ADDR		4
#define TSEC2_FLAGS		TSEC_GIGABIT
#define TSEC2_PHYIDX		0
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
	#define CONFIG_ENV_OFFSET		(512 * 1024)
	#define CONFIG_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE
	#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
	#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE
	#define CONFIG_ENV_RANGE		(CONFIG_ENV_SECT_SIZE * 4)
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

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

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

#if defined(CONFIG_SYS_RAMBOOT) && !defined(CONFIG_NAND_U_BOOT)
    #undef CONFIG_CMD_SAVEENV
    #undef CONFIG_CMD_LOADS
#endif

#define CONFIG_CMDLINE_EDITING 1
#define CONFIG_AUTO_COMPLETE	/* add autocompletion support   */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */

						/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	\
			(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
				/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_SYS_HZ		1000	/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

#define CONFIG_SYS_RCWH_PCIHOST 0x80000000	/* PCIHOST  */

#ifdef CONFIG_SYS_66MHZ

/* 66MHz IN, 133MHz CSB, 266 DDR, 266 CORE */
/* 0x62040000 */
#define CONFIG_SYS_HRCW_LOW (\
	0x20000000 /* reserved, must be set */ |\
	HRCWL_DDRCM |\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_2X1 |\
	HRCWL_CSB_TO_CLKIN_2X1 |\
	HRCWL_CORE_TO_CSB_2X1)

#define CONFIG_SYS_NS16550_CLK (CONFIG_83XX_CLKIN * 2)

#elif defined(CONFIG_SYS_33MHZ)

/* 33MHz IN, 165MHz CSB, 330 DDR, 330 CORE */
/* 0x65040000 */
#define CONFIG_SYS_HRCW_LOW (\
	0x20000000 /* reserved, must be set */ |\
	HRCWL_DDRCM |\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_2X1 |\
	HRCWL_CSB_TO_CLKIN_5X1 |\
	HRCWL_CORE_TO_CSB_2X1)

#define CONFIG_SYS_NS16550_CLK (CONFIG_83XX_CLKIN * 5)

#endif

#define CONFIG_SYS_HRCW_HIGH_BASE (\
	HRCWH_PCI_HOST |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_TSEC1M_IN_RGMII |\
	HRCWH_TSEC2M_IN_RGMII |\
	HRCWH_BIG_ENDIAN)

#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_HRCW_HIGH (CONFIG_SYS_HRCW_HIGH_BASE |\
		       HRCWH_FROM_0XFFF00100 |\
		       HRCWH_ROM_LOC_NAND_SP_8BIT |\
		       HRCWH_RL_EXT_NAND)
#else
#define CONFIG_SYS_HRCW_HIGH (CONFIG_SYS_HRCW_HIGH_BASE |\
		       HRCWH_FROM_0X00000100 |\
		       HRCWH_ROM_LOC_LOCAL_16BIT |\
		       HRCWH_RL_EXT_LEGACY)
#endif

/* System IO Config */
#define CONFIG_SYS_SICRH	(SICRH_TSOBI1 | SICRH_TSOBI2)	/* RGMII */
			/* Enable Internal USB Phy and GPIO on LCD Connector */
#define CONFIG_SYS_SICRL	(SICRL_USBDR_10 | SICRL_LBC)

#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK | \
				 HID0_ENABLE_INSTRUCTION_CACHE | \
				 HID0_ENABLE_DYNAMIC_POWER_MANAGMENT)

#define CONFIG_SYS_HID2 HID2_HBE

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR @ 0x00000000 */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE | BATL_PP_RW)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* PCI @ 0x80000000 */
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_PCI1_MEM_BASE | BATL_PP_RW)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_PCI1_MEM_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_PCI1_MMIO_BASE \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* PCI2 not supported on 8313 */
#define CONFIG_SYS_IBAT3L	(0)
#define CONFIG_SYS_IBAT3U	(0)
#define CONFIG_SYS_IBAT4L	(0)
#define CONFIG_SYS_IBAT4U	(0)

/* IMMRBAR @ 0xE0000000, PCI IO @ 0xE2000000 & BCSR @ 0xE2400000 */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_IMMR \
				| BATL_PP_RW \
				| BATL_CACHEINHIBIT \
				| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_IMMR \
				| BATU_BL_256M \
				| BATU_VS \
				| BATU_VP)

/* SDRAM @ 0xF0000000, stack in DCACHE 0xFDF00000 & FLASH @ 0xFE000000 */
#define CONFIG_SYS_IBAT6L	(0xF0000000 | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT6U	(0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

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

#define CONFIG_HOSTNAME		mpc8313erdb
#define CONFIG_ROOTPATH		"/nfs/root/path"
#define CONFIG_BOOTFILE		"uImage"
				/* U-Boot image on TFTP server */
#define CONFIG_UBOOTPATH	"u-boot.bin"
#define CONFIG_FDTFILE		"mpc8313erdb.dtb"

				/* default location for tftp and bootm */
#define CONFIG_LOADADDR		800000
#define CONFIG_BOOTDELAY	6	/* -1 disables auto-boot */
#define CONFIG_BAUDRATE		115200

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
	"fdtaddr=780000\0"						\
	"fdtfile=" CONFIG_FDTFILE "\0"					\
	"console=ttyS0\0"						\
	"setbootargs=setenv bootargs "					\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"setipargs=setenv bootargs nfsroot=$serverip:$rootpath "	 \
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:"\
							"$netdev:off " \
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0"

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

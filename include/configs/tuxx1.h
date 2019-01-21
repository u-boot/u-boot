/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *                    Dave Liu <daveliu@freescale.com>
 *
 * Copyright (C) 2007 Logic Product Development, Inc.
 *                    Peter Barada <peterb@logicpd.com>
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * (C) Copyright 2010-2013
 * Lukas Roggli, KEYMILE Ltd, lukas.roggli@keymile.com
 * Holger Brunck,  Keymile GmbH, holger.bruncl@keymile.com
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_KM_BOARD_NAME	"tuxx1"
#define CONFIG_HOSTNAME		"tuxx1"

/*
 * High Level Configuration Options
 */
#define CONFIG_QE	/* Has QE */
#define CONFIG_KM8321	/* Keymile PBEC8321 board specific */

#define CONFIG_KM_DEF_ARCH	"arch=ppc_8xx\0"

/* include common defines/options for all Keymile boards */
#include "km/keymile-common.h"
#include "km/km-powerpc.h"

/*
 * System Clock Setup
 */
#define CONFIG_83XX_CLKIN		66000000
#define CONFIG_SYS_CLK_FREQ		66000000
#define CONFIG_83XX_PCICLK		66000000

/*
 * IMMR new address
 */
#define CONFIG_SYS_IMMR		0xE0000000

/*
 * Bus Arbitration Configuration Register (ACR)
 */
#define CONFIG_SYS_ACR_PIPE_DEP 3       /* pipeline depth 4 transactions */
#define CONFIG_SYS_ACR_RPTCNT   3       /* 4 consecutive transactions */
#define CONFIG_SYS_ACR_APARK    0       /* park bus to master (below) */
#define CONFIG_SYS_ACR_PARKM    3       /* parking master = QuiccEngine */

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_BASE		0x00000000 /* DDR is system memory */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_SDRAM_BASE2	(CONFIG_SYS_SDRAM_BASE + 0x10000000) /* +256M */

#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
					DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)

#define CFG_83XX_DDR_USES_CS0

/*
 * Manually set up DDR parameters
 */
#define CONFIG_DDR_II
#define CONFIG_SYS_DDR_SIZE		2048 /* MB */

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE /* start of monitor */
#define CONFIG_SYS_FLASH_BASE		0xF0000000

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#endif

/* Reserve 768 kB for Mon */
#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* End of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)

/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  0   Local   GPCM    16 bit  256MB FLASH
 *  1   Local   GPCM     8 bit  128MB GPIO/PIGGY
 *
 */
/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_SIZE		256 /* max FLASH size is 256M */

/* FLASH */
#define CONFIG_SYS_BR0_PRELIM	(0xF0000000 | BR_PS_16 | BR_MS_GPCM | BR_V)
#define CONFIG_SYS_OR0_PRELIM	(OR_AM_256MB | OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | OR_GPCM_SCY_5 | OR_GPCM_TRLX_SET | OR_GPCM_EAD)

#define CONFIG_SYS_MAX_FLASH_BANKS	1   /* max num of flash banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	512 /* max num of sects on one chip */
#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE }

/*
 * PRIO1/PIGGY on the local bus CS1
 */

/* KMBEC_FPGA */
#define CONFIG_SYS_BR1_PRELIM	(0xE8000000 | BR_PS_8 | BR_MS_GPCM | BR_V)
#define CONFIG_SYS_OR1_PRELIM	(OR_AM_128MB | OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | OR_GPCM_SCY_2 | OR_GPCM_TRLX_SET | OR_GPCM_EAD)

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
#define CONFIG_ETHPRIME		"UEC0"

#define CONFIG_UEC_ETH1		/* GETH1 */
#define UEC_VERBOSE_DEBUG	1

#ifdef CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM	3	/* UCC4 */
#define CONFIG_SYS_UEC1_RX_CLK		QE_CLK_NONE /* not used in RMII Mode */
#define CONFIG_SYS_UEC1_TX_CLK		QE_CLK17
#define CONFIG_SYS_UEC1_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR	0
#define CONFIG_SYS_UEC1_INTERFACE_TYPE	PHY_INTERFACE_MODE_RMII
#define CONFIG_SYS_UEC1_INTERFACE_SPEED	100
#endif

/*
 * Environment
 */

#ifndef CONFIG_SYS_RAMBOOT
#ifndef CONFIG_ENV_ADDR
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + \
					CONFIG_SYS_MONITOR_LEN)
#endif
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
#ifndef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET	(CONFIG_SYS_MONITOR_LEN)
#endif

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
						CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#else /* CFG_SYS_RAMBOOT */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE		0x2000
#endif /* CFG_SYS_RAMBOOT */

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_NUM_I2C_BUSES	4
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	200000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C_OFFSET		0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	200000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_BUSES	{{0, {I2C_NULL_HOP} }, \
		{0, {{I2C_MUX_PCA9547, 0x70, 2} } }, \
		{0, {{I2C_MUX_PCA9547, 0x70, 1} } }, \
		{1, {I2C_NULL_HOP} } }

#define CONFIG_KM_IVM_BUS		2	/* I2C2 (Mux-Port 1)*/

#if defined(CONFIG_CMD_NAND)
#define CONFIG_NAND_KMETER1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		CONFIG_SYS_KMBEC_FPGA_BASE
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

/*
 * Core HID Setup
 */
#define CONFIG_SYS_HID0_INIT		0x000000000
#define CONFIG_SYS_HID0_FINAL		(HID0_ENABLE_MACHINE_CHECK | \
					 HID0_ENABLE_INSTRUCTION_CACHE)
#define CONFIG_SYS_HID2			HID2_HBE

/*
 * Internal Definitions
 */
#define BOOTFLASH_START	0xF0000000

#define CONFIG_KM_CONSOLE_TTY	"ttyS0"

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE
#ifndef CONFIG_KM_DEF_ENV		/* if not set by keymile-common.h */
#define CONFIG_KM_DEF_ENV "km-common=empty\0"
#endif

#ifndef CONFIG_KM_DEF_ARCH
#define CONFIG_KM_DEF_ARCH	"arch=ppc_82xx\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_KM_DEF_ENV						\
	CONFIG_KM_DEF_ARCH						\
	"newenv="							\
		"prot off "__stringify(CONFIG_ENV_ADDR)" +0x40000 && "	\
		"era "__stringify(CONFIG_ENV_ADDR)" +0x40000\0"		\
	"unlock=yes\0"							\
	""

#if defined(CONFIG_UEC_ETH)
#define CONFIG_HAS_ETH0
#endif

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRL	SICRL_IRQ_CKS

#define CONFIG_SYS_DDRCDR (\
	DDRCDR_EN | \
	DDRCDR_PZ_MAXZ | \
	DDRCDR_NZ_MAXZ | \
	DDRCDR_M_ODR)

#define CONFIG_SYS_DDR_CS0_BNDS		0x0000007f
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SDRAM_TYPE_DDR2 | \
					 SDRAM_CFG_32_BE | \
					 SDRAM_CFG_SREN | \
					 SDRAM_CFG_HSE)

#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000
#define CONFIG_SYS_DDR_CLK_CNTL		(DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)
#define CONFIG_SYS_DDR_INTERVAL	((0x064 << SDRAM_INTERVAL_BSTOPRE_SHIFT) | \
				 (0x200 << SDRAM_INTERVAL_REFINT_SHIFT))

#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN | CSCONFIG_AP | \
					 CSCONFIG_ODT_WR_CFG | \
					 CSCONFIG_ROW_BIT_13 | \
					 CSCONFIG_COL_BIT_10)

#define CONFIG_SYS_DDR_MODE	0x47860242
#define CONFIG_SYS_DDR_MODE2	0x8080c000

#define CONFIG_SYS_DDR_TIMING_0	((2 << TIMING_CFG0_MRS_CYC_SHIFT) | \
				 (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) | \
				 (2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) | \
				 (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) | \
				 (0 << TIMING_CFG0_WWT_SHIFT) | \
				 (0 << TIMING_CFG0_RRT_SHIFT) | \
				 (0 << TIMING_CFG0_WRT_SHIFT) | \
				 (0 << TIMING_CFG0_RWT_SHIFT))

#define CONFIG_SYS_DDR_TIMING_1	((TIMING_CFG1_CASLAT_40) | \
				 (2 << TIMING_CFG1_WRTORD_SHIFT) | \
				 (2 << TIMING_CFG1_ACTTOACT_SHIFT) | \
				 (3 << TIMING_CFG1_WRREC_SHIFT) | \
				 (7 << TIMING_CFG1_REFREC_SHIFT) | \
				 (3 << TIMING_CFG1_ACTTORW_SHIFT) | \
				 (7 << TIMING_CFG1_ACTTOPRE_SHIFT) | \
				 (3 << TIMING_CFG1_PRETOACT_SHIFT))

#define CONFIG_SYS_DDR_TIMING_2	((8 << TIMING_CFG2_FOUR_ACT_SHIFT) | \
				 (3 << TIMING_CFG2_CKE_PLS_SHIFT) | \
				 (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) | \
				 (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) | \
				 (3 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) | \
				 (0 << TIMING_CFG2_ADD_LAT_SHIFT) | \
				 (5 << TIMING_CFG2_CPO_SHIFT))

#define CONFIG_SYS_DDR_TIMING_3	0x00000000

#define CONFIG_SYS_KMBEC_FPGA_BASE	0xE8000000
#define CONFIG_SYS_KMBEC_FPGA_SIZE	128

/* EEprom support */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1

/*
 * Local Bus Configuration & Clock Setup
 */
#define CONFIG_SYS_LCRR_DBYP	0x80000000
#define CONFIG_SYS_LCRR_EADC	0x00010000
#define CONFIG_SYS_LCRR_CLKDIV	0x00000002

#define CONFIG_SYS_LBC_LBCR	0x00000000

#define CONFIG_SYS_APP1_BASE	0xA0000000    /* PAXG */
#define	CONFIG_SYS_APP1_SIZE	256 /* Megabytes */
#define CONFIG_SYS_APP2_BASE	0xB0000000    /* PINC3 */
#define	CONFIG_SYS_APP2_SIZE	256 /* Megabytes */

/*
 * Init Local Bus Memory Controller:
 *				      Device on board
 * Bank Bus     Machine PortSz Size   TUDA1  TUXA1  TUGE1   KMSUPX4 KMOPTI2
 * -----------------------------------------------------------------------------
 *  2   Local   GPCM    8 bit  256MB  PAXG   LPXF   PAXI    LPXF    PAXE
 *  3   Local   GPCM    8 bit  256MB  PINC3  PINC2  unused  unused  OPI2(16 bit)
 *
 *				      Device on board (continued)
 * Bank Bus     Machine PortSz Size   KMTEPR2
 * -----------------------------------------------------------------------------
 *  2   Local   GPCM    8 bit  256MB  NVRAM
 *  3   Local   GPCM    8 bit  256MB  TEP2 (16 bit)
 */

/* APP1 */
#define CONFIG_SYS_BR2_PRELIM	(0xA0000000 | BR_PS_8 | BR_MS_GPCM | BR_V)
#define CONFIG_SYS_OR2_PRELIM	(OR_AM_256MB | OR_GPCM_CSNT | OR_GPCM_ACS_DIV4 | OR_GPCM_SCY_2 | OR_GPCM_TRLX_SET | OR_GPCM_EHTR_CLEAR | OR_GPCM_EAD)

/* APP2 */
#define CONFIG_SYS_BR3_PRELIM	(0xB0000000 | BR_PS_8 | BR_MS_GPCM | BR_V)
#define CONFIG_SYS_OR3_PRELIM	(OR_AM_256MB | OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | OR_GPCM_SCY_2 | OR_GPCM_TRLX_SET | OR_GPCM_EHTR_CLEAR)

#define CONFIG_SYS_MAMR		(MxMR_GPL_x4DIS | \
				 0x0000c000 | \
				 MxMR_WLFx_2X)

#endif /* __CONFIG_H */

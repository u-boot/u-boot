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
#define CONFIG_KM_BOARD_NAME	"kmopti2"
#define CONFIG_HOSTNAME		"kmopti2"

/*
 * High Level Configuration Options
 */
#define CONFIG_QE	/* Has QE */
#define CONFIG_KM8321	/* Keymile PBEC8321 board specific */

#define CONFIG_KM_DEF_ARCH	"arch=ppc_8xx\0"

/* include common defines/options for all 83xx Keymile boards */
#include "km83xx-common.h"

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRL	SICRL_IRQ_CKS

/*
 * Hardware Reset Configuration Word
 */
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 | \
	HRCWL_DDR_TO_SCB_CLK_2X1 | \
	HRCWL_CSB_TO_CLKIN_2X1 | \
	HRCWL_CORE_TO_CSB_2_5X1 | \
	HRCWL_CE_PLL_VCO_DIV_2 | \
	HRCWL_CE_TO_PLL_1X3)

#define CONFIG_SYS_HRCW_HIGH (\
	HRCWH_PCI_AGENT | \
	HRCWH_PCI_ARBITER_DISABLE | \
	HRCWH_CORE_ENABLE | \
	HRCWH_FROM_0X00000100 | \
	HRCWH_BOOTSEQ_DISABLE | \
	HRCWH_SW_WATCHDOG_DISABLE | \
	HRCWH_ROM_LOC_LOCAL_16BIT | \
	HRCWH_BIG_ENDIAN | \
	HRCWH_LALE_NORMAL)

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

/*
 * MMU Setup
 */
#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U

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

/*
 * Configuration for C2 on the local bus
 */
/* Window base at flash base */
#define CONFIG_SYS_LBLAWBAR2_PRELIM	CONFIG_SYS_APP1_BASE
/* Window size: 256 MB */
#define CONFIG_SYS_LBLAWAR2_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)

#define CONFIG_SYS_BR2_PRELIM	(CONFIG_SYS_APP1_BASE | \
				 BR_PS_8 | \
				 BR_MS_GPCM | \
				 BR_V)

#define CONFIG_SYS_OR2_PRELIM	(MEG_TO_AM(CONFIG_SYS_APP1_SIZE) | \
				 OR_GPCM_CSNT | \
				 OR_GPCM_ACS_DIV4 | \
				 OR_GPCM_SCY_2 | \
				 OR_GPCM_TRLX_SET | \
				 OR_GPCM_EHTR_CLEAR | \
				 OR_GPCM_EAD)

/*
 * Configuration for C3 on the local bus
 */
#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_APP2_BASE
#define CONFIG_SYS_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)
#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_APP2_BASE | \
				 BR_PS_16 |		\
				 BR_MS_GPCM |		\
				 BR_V)
#define CONFIG_SYS_OR3_PRELIM	(MEG_TO_AM(CONFIG_SYS_APP2_SIZE) | \
				 OR_GPCM_SCY_4 | \
				 OR_GPCM_TRLX_CLEAR | \
				 OR_GPCM_EHTR_CLEAR)
/*
 * MMU Setup
 */
/* APP1: icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_APP1_BASE | \
				 BATL_PP_RW | \
				 BATL_MEMCOHERENCE)
/* 512M should also include APP2... */
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_APP1_BASE | \
				 BATU_BL_256M | \
				 BATU_VS | \
				 BATU_VP)
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_APP1_BASE | \
				 BATL_PP_RW | \
				 BATL_CACHEINHIBIT | \
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

/* APP2:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_APP2_BASE | \
				 BATL_PP_RW | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_APP2_BASE | \
				 BATU_BL_256M | \
				 BATU_VS | \
				 BATU_VP)
#define CONFIG_SYS_DBAT6L	(CONFIG_SYS_APP2_BASE | \
				 BATL_PP_RW | \
				 BATL_CACHEINHIBIT | \
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U

#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U

#endif /* __CONFIG_H */

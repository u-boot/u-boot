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
 * (C) Copyright 2008-2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_QE		/* Has QE */
#define CONFIG_MPC8360		/* MPC8360 CPU specific */
#define CONFIG_KMETER1		/* KMETER1 board specific */
#define CONFIG_HOSTNAME		kmeter1
#define CONFIG_KM_BOARD_NAME   "kmeter1"

#define	CONFIG_SYS_TEXT_BASE	0xF0000000
#define CONFIG_KM_DEF_NETDEV	\
	"netdev=eth2\0"		\

/* include common defines/options for all 83xx Keymile boards */
#include "km/km83xx-common.h"

#define CONFIG_MISC_INIT_R
/*
 * System IO Setup
 */
#define CONFIG_SYS_SICRH		(SICRH_UC1EOBI | SICRH_UC2E1OBI)

/*
 * Hardware Reset Configuration Word
 */
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_CSB_TO_CLKIN_4X1 | \
	HRCWL_CORE_TO_CSB_2X1 | \
	HRCWL_CE_PLL_VCO_DIV_2 | \
	HRCWL_CE_TO_PLL_1X6 )

#define CONFIG_SYS_HRCW_HIGH (\
	HRCWH_CORE_ENABLE | \
	HRCWH_FROM_0X00000100 | \
	HRCWH_BOOTSEQ_DISABLE | \
	HRCWH_SW_WATCHDOG_DISABLE | \
	HRCWH_ROM_LOC_LOCAL_16BIT | \
	HRCWH_BIG_ENDIAN | \
	HRCWH_LALE_EARLY | \
	HRCWH_LDP_CLEAR )

#define CONFIG_SYS_DDR_CS0_BNDS		0x0000007f
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SDRAM_TYPE_DDR2 | \
					 SDRAM_CFG_SREN)
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000
#define CONFIG_SYS_DDR_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)
#define CONFIG_SYS_DDR_INTERVAL	((0x080 << SDRAM_INTERVAL_BSTOPRE_SHIFT) | \
				 (0x3cf << SDRAM_INTERVAL_REFINT_SHIFT))

#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN | CSCONFIG_AP | \
					 CSCONFIG_ROW_BIT_13 | \
					 CSCONFIG_COL_BIT_10 | \
					 CSCONFIG_ODT_WR_ACS)

#define	CONFIG_SYS_DDRCDR		0x40000001
#define CONFIG_SYS_DDR_MODE		0x47860452
#define CONFIG_SYS_DDR_MODE2		0x8080c000

#define CONFIG_SYS_DDR_TIMING_0	((2 << TIMING_CFG0_MRS_CYC_SHIFT) | \
				 (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) | \
				 (6 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) | \
				 (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) | \
				 (0 << TIMING_CFG0_WWT_SHIFT) | \
				 (0 << TIMING_CFG0_RRT_SHIFT) | \
				 (0 << TIMING_CFG0_WRT_SHIFT) | \
				 (0 << TIMING_CFG0_RWT_SHIFT))

#define CONFIG_SYS_DDR_TIMING_1	((TIMING_CFG1_CASLAT_50) | \
				 (2 << TIMING_CFG1_WRTORD_SHIFT) | \
				 (2 << TIMING_CFG1_ACTTOACT_SHIFT) | \
				 (3 << TIMING_CFG1_WRREC_SHIFT) | \
				 (7 << TIMING_CFG1_REFREC_SHIFT) | \
				 (3 << TIMING_CFG1_ACTTORW_SHIFT) | \
				 (8 << TIMING_CFG1_ACTTOPRE_SHIFT) | \
				 (3 << TIMING_CFG1_PRETOACT_SHIFT))

#define CONFIG_SYS_DDR_TIMING_2	((8 << TIMING_CFG2_FOUR_ACT_SHIFT) | \
				 (3 << TIMING_CFG2_CKE_PLS_SHIFT) | \
				 (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) | \
				 (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) | \
				 (4 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) | \
				 (0 << TIMING_CFG2_ADD_LAT_SHIFT) | \
				 (5 << TIMING_CFG2_CPO_SHIFT))

#define CONFIG_SYS_DDR_TIMING_3	0x00000000

/* PRIO FPGA */
#define	CONFIG_SYS_KMBEC_FPGA_BASE	0xE8000000
#define	CONFIG_SYS_KMBEC_FPGA_SIZE	128
/* PAXE FPGA */
#define	CONFIG_SYS_PAXE_BASE		0xA0000000
#define	CONFIG_SYS_PAXE_SIZE		512

/* EEprom support */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

/*
 * Local Bus Configuration & Clock Setup
 */
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_EADC	LCRR_EADC_2
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_4

/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  3   Local   GPCM     8 bit  512MB PAXE
 *
 */

/*
 * PAXE on the local bus CS3
 */
#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_PAXE_BASE
#define CONFIG_SYS_LBLAWAR3_PRELIM	0x8000001C /* 512MB window size */

#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_PAXE_BASE | \
				(1 << BR_PS_SHIFT) | /* 8 bit port size */ \
				BR_V)
#define CONFIG_SYS_OR3_PRELIM	(MEG_TO_AM(CONFIG_SYS_PAXE_SIZE) | \
				OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | \
				OR_GPCM_SCY_2 | \
				OR_GPCM_TRLX | OR_GPCM_EAD)

/*
 * MMU Setup
 */

/* PAXE:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_PAXE_BASE | BATL_PP_10 | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_PAXE_BASE | BATU_BL_256M | \
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_PAXE_BASE | BATL_PP_10 | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

#ifdef CONFIG_PCI
/* PCI MEM space: cacheable */
#define CFG_IBAT6L	(CFG_PCI1_MEM_PHYS | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT6U	(CFG_PCI1_MEM_PHYS | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT6L	CFG_IBAT6L
#define CFG_DBAT6U	CFG_IBAT6U
/* PCI MMIO space: cache-inhibit and guarded */
#define CFG_IBAT7L	(CFG_PCI1_MMIO_PHYS | BATL_PP_10 | \
			 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT7U	(CFG_PCI1_MMIO_PHYS | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT7L	CFG_IBAT7L
#define CFG_DBAT7U	CFG_IBAT7U
#else /* CONFIG_PCI */
#define CONFIG_SYS_IBAT6L	(0)
#define CONFIG_SYS_IBAT6U	(0)
#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U
#endif /* CONFIG_PCI */

#endif /* __CONFIG_H */

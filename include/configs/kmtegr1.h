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
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

/* This needs to be set prior to including km83xx-common.h */

#define CONFIG_HOSTNAME   "kmtegr1"
#define CONFIG_KM_BOARD_NAME   "kmtegr1"
#define CONFIG_KM_UBI_PARTITION_NAME_BOOT	"ubi0"
#define CONFIG_KM_UBI_PARTITION_NAME_APP	"ubi1"

#define CONFIG_ENV_ADDR		0xF0100000
#define CONFIG_ENV_OFFSET	0x100000

#define CONFIG_NAND_ECC_BCH
#define CONFIG_NAND_KMETER1
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define NAND_MAX_CHIPS				1

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 family */
#define CONFIG_QE		1	/* Has QE */

#define CONFIG_KM_DEF_ARCH	"arch=ppc_82xx\0"

/* include common defines/options for all 83xx Keymile boards */
#include "km83xx-common.h"

/* QE microcode/firmware address */
#define CONFIG_SYS_QE_FMAN_FW_IN_NOR
/* between the u-boot partition and env */
#ifndef CONFIG_SYS_QE_FW_ADDR
#define CONFIG_SYS_QE_FW_ADDR   0xF00C0000
#endif

/*
 * System IO Config
 */
/* 0x14000180 SICR_1 */
#define CONFIG_SYS_SICRL (0			\
		| SICR_1_UART1_UART1RTS		\
		| SICR_1_I2C_CKSTOP		\
		| SICR_1_IRQ_A_IRQ		\
		| SICR_1_IRQ_B_IRQ		\
		| SICR_1_GPIO_A_GPIO		\
		| SICR_1_GPIO_B_GPIO		\
		| SICR_1_GPIO_C_GPIO		\
		| SICR_1_GPIO_D_GPIO		\
		| SICR_1_GPIO_E_GPIO		\
		| SICR_1_GPIO_F_GPIO		\
		| SICR_1_USB_A_UART2S		\
		| SICR_1_USB_B_UART2RTS		\
		| SICR_1_FEC1_FEC1		\
		| SICR_1_FEC2_FEC2		\
		)

/* 0x00080400 SICR_2 */
#define CONFIG_SYS_SICRH (0			\
		| SICR_2_FEC3_FEC3		\
		| SICR_2_HDLC1_A_HDLC1		\
		| SICR_2_ELBC_A_LA		\
		| SICR_2_ELBC_B_LCLK		\
		| SICR_2_HDLC2_A_HDLC2		\
		| SICR_2_USB_D_GPIO		\
		| SICR_2_PCI_PCI		\
		| SICR_2_HDLC1_B_HDLC1		\
		| SICR_2_HDLC1_C_HDLC1		\
		| SICR_2_HDLC2_B_GPIO		\
		| SICR_2_HDLC2_C_HDLC2		\
		| SICR_2_QUIESCE_B		\
		)

/* GPR_1 */
#define CONFIG_SYS_GPR1  0x50008060

#define CONFIG_SYS_GP1DIR 0x00000000
#define CONFIG_SYS_GP1ODR 0x00000000
#define CONFIG_SYS_GP2DIR 0xFF000000
#define CONFIG_SYS_GP2ODR 0x00000000

/*
 * Hardware Reset Configuration Word
 */
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 | \
	HRCWL_DDR_TO_SCB_CLK_2X1 | \
	HRCWL_CSB_TO_CLKIN_2X1 | \
	HRCWL_CORE_TO_CSB_2X1 | \
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
					 CSCONFIG_ODT_RD_NEVER | \
					 CSCONFIG_ODT_WR_ONLY_CURRENT | \
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

/* must be after the include because KMBEC_FPGA is otherwise undefined */
#define CONFIG_SYS_NAND_BASE CONFIG_SYS_KMBEC_FPGA_BASE /* PRIO_BASE_ADDRESS */

#define CONFIG_SYS_APP1_BASE		0xA0000000
#define CONFIG_SYS_APP1_SIZE		256 /* Megabytes */
#define CONFIG_SYS_APP2_BASE		0xB0000000
#define CONFIG_SYS_APP2_SIZE		256 /* Megabytes */

/* EEprom support */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1

/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  2   Local   UPMA    16 bit  256MB APP1
 *  3   Local   GPCM    16 bit  256MB APP2
 *
 */

#define CONFIG_SYS_BR3_PRELIM (CONFIG_SYS_APP2_BASE | \
				 BR_PS_16 | \
				 BR_MS_GPCM | \
				 BR_V)

#define CONFIG_SYS_OR3_PRELIM (MEG_TO_AM(CONFIG_SYS_APP2_SIZE) | \
				 OR_GPCM_SCY_5 | \
				 OR_GPCM_TRLX_CLEAR | \
				 OR_GPCM_EHTR_CLEAR)

#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_APP2_BASE
#define CONFIG_SYS_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)

/*
 * MMU Setup
 */
#define CONFIG_SYS_IBAT5L (0)
#define CONFIG_SYS_IBAT5U (0)
#define CONFIG_SYS_DBAT5L CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U CONFIG_SYS_IBAT5U
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_APP2_BASE | BATL_PP_RW | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_APP2_BASE | BATU_BL_256M | \
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT6L	(CONFIG_SYS_APP2_BASE | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U

/* ethernet port connected to piggy (UEC2) */
#define CONFIG_HAS_ETH1
#define CONFIG_UEC_ETH2
#define CONFIG_SYS_UEC2_UCC_NUM		2       /* UCC3 */
#define CONFIG_SYS_UEC2_RX_CLK		QE_CLK_NONE /* not used in RMII Mode */
#define CONFIG_SYS_UEC2_TX_CLK		QE_CLK12
#define CONFIG_SYS_UEC2_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC2_PHY_ADDR	0
#define CONFIG_SYS_UEC2_INTERFACE_TYPE	PHY_INTERFACE_MODE_RMII
#define CONFIG_SYS_UEC2_INTERFACE_SPEED	100

#endif /* __CONFIG_H */

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

/* include common defines/options for all 8309 Keymile boards */
#include "km8309-common.h"
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

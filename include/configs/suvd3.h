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
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

/* This needs to be set prior to including km/km83xx-common.h */
#define	CONFIG_SYS_TEXT_BASE	0xF0000000

#if defined(CONFIG_SUVD3)	/* SUVD3 board specific */
#define CONFIG_HOSTNAME		suvd3
#define CONFIG_KM_BOARD_NAME   "suvd3"
/* include common defines/options for all 8321 Keymile boards */
#include "km/km8321-common.h"

#elif defined(CONFIG_KMVECT1)   /* VECT1 board specific */
#define CONFIG_HOSTNAME		kmvect1
#define CONFIG_KM_BOARD_NAME   "kmvect1"
/* at end of uboot partition, before env */
#define CONFIG_SYS_QE_FW_ADDR   0xF00B0000
/* include common defines/options for all 8309 Keymile boards */
#include "km/km8309-common.h"

#elif defined(CONFIG_KMTEGR1)   /* TEGR1 board specific */
#define CONFIG_HOSTNAME   kmtegr1
#define CONFIG_KM_BOARD_NAME   "kmtegr1"
#define CONFIG_KM_UBI_PARTITION_NAME_BOOT	"ubi0"
#define CONFIG_KM_UBI_PARTITION_NAME_APP	"ubi1"
#define MTDIDS_DEFAULT			"nor0=boot,nand0=app"
#define MTDPARTS_DEFAULT		"mtdparts="			\
	"boot:"								\
		"768k(u-boot),"						\
		"256k(qe-fw),"						\
		"128k(env),"						\
		"128k(envred),"						\
		"-(" CONFIG_KM_UBI_PARTITION_NAME_BOOT ");"		\
	"app:"								\
		"-(" CONFIG_KM_UBI_PARTITION_NAME_APP ");"

#define CONFIG_ENV_ADDR		0xF0100000
#define CONFIG_ENV_OFFSET	0x100000

#define CONFIG_CMD_NAND
#define CONFIG_NAND_ECC_BCH
#define CONFIG_BCH
#define CONFIG_NAND_KMETER1
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define NAND_MAX_CHIPS				1

/* include common defines/options for all 8309 Keymile boards */
#include "km/km8309-common.h"
/* must be after the include because KMBEC_FPGA is otherwise undefined */
#define CONFIG_SYS_NAND_BASE CONFIG_SYS_KMBEC_FPGA_BASE /* PRIO_BASE_ADDRESS */

#else
#error Supported boards are: SUVD3, KMVECT1, KMTEGR1
#endif

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

#if defined(CONFIG_SUVD3) || defined(CONFIG_KMVECT1)
/*
 * APP1 on the local bus CS2
 */
#define CONFIG_SYS_LBLAWBAR2_PRELIM	CONFIG_SYS_APP1_BASE
#define CONFIG_SYS_LBLAWAR2_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)

#define CONFIG_SYS_BR2_PRELIM	(CONFIG_SYS_APP1_BASE | \
				 BR_PS_16 | \
				 BR_MS_UPMA | \
				 BR_V)
#define CONFIG_SYS_OR2_PRELIM	(MEG_TO_AM(CONFIG_SYS_APP1_SIZE))

#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_APP2_BASE | \
				 BR_PS_16 | \
				 BR_V)

#define CONFIG_SYS_OR3_PRELIM	(MEG_TO_AM(CONFIG_SYS_APP2_SIZE) | \
				 OR_GPCM_CSNT | \
				 OR_GPCM_ACS_DIV4 | \
				 OR_GPCM_SCY_3 | \
				 OR_GPCM_TRLX_SET)

#define CONFIG_SYS_MAMR	(MxMR_GPL_x4DIS | \
			 0x0000c000 | \
			 MxMR_WLFx_2X)

#elif defined(CONFIG_KMTEGR1)
#define CONFIG_SYS_BR3_PRELIM (CONFIG_SYS_APP2_BASE | \
				 BR_PS_16 | \
				 BR_MS_GPCM | \
				 BR_V)

#define CONFIG_SYS_OR3_PRELIM (MEG_TO_AM(CONFIG_SYS_APP2_SIZE) | \
				 OR_GPCM_SCY_5 | \
				 OR_GPCM_TRLX_CLEAR | \
				 OR_GPCM_EHTR_CLEAR)

#endif /* CONFIG_KMTEGR1 */

#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_APP2_BASE
#define CONFIG_SYS_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)

/*
 * MMU Setup
 */
#if defined(CONFIG_SUVD3) || defined(CONFIG_KMVECT1)
/* APP1:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_APP1_BASE | BATL_PP_RW | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_APP1_BASE | BATU_BL_256M | \
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_APP1_BASE | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

#elif defined(CONFIG_KMTEGR1)
#define CONFIG_SYS_IBAT5L (0)
#define CONFIG_SYS_IBAT5U (0)
#define CONFIG_SYS_DBAT5L CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U CONFIG_SYS_IBAT5U
#endif /* CONFIG_KMTEGR1 */

#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_APP2_BASE | BATL_PP_RW | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_APP2_BASE | BATU_BL_256M | \
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT6L	(CONFIG_SYS_APP2_BASE | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U

/*
 * QE UEC ethernet configuration
 */
#if defined(CONFIG_KMVECT1)
#define CONFIG_MV88E6352_SWITCH
#define CONFIG_KM_MVEXTSW_ADDR		0x10

/* ethernet port connected to simple switch 88e6122 (UEC0) */
#define CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM		0	/* UCC1 */
#define CONFIG_SYS_UEC1_RX_CLK		QE_CLK9
#define CONFIG_SYS_UEC1_TX_CLK		QE_CLK10

#define CONFIG_FIXED_PHY		0xFFFFFFFF
#define CONFIG_SYS_FIXED_PHY_ADDR	0x1E	/* unused address */
#define CONFIG_SYS_FIXED_PHY_PORT(devnum, speed, duplex) \
		{devnum, speed, duplex}
#define CONFIG_SYS_FIXED_PHY_PORTS \
		CONFIG_SYS_FIXED_PHY_PORT("UEC0", SPEED_100, DUPLEX_FULL)

#define CONFIG_SYS_UEC1_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR	CONFIG_SYS_FIXED_PHY_ADDR
#define CONFIG_SYS_UEC1_INTERFACE_TYPE	PHY_INTERFACE_MODE_MII
#define CONFIG_SYS_UEC1_INTERFACE_SPEED	100
#endif /* CONFIG_KMVECT1 */

#if defined(CONFIG_KMVECT1) || defined(CONFIG_KMTEGR1)
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
#endif /* CONFIG_KMVECT1 || CONFIG_KMTEGR1 */

#endif /* __CONFIG_H */

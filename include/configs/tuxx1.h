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
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#if defined(CONFIG_KMSUPX5)
#define CONFIG_KM_BOARD_NAME	"kmsupx5"
#define CONFIG_HOSTNAME		kmsupx5
#elif defined(CONFIG_TUGE1)
#define CONFIG_KM_BOARD_NAME	"tuge1"
#define CONFIG_HOSTNAME		tuge1
#elif defined(CONFIG_TUXX1)	/* TUXX1 board (tuxa1/tuda1) specific */
#define CONFIG_KM_BOARD_NAME	"tuxx1"
#define CONFIG_HOSTNAME		tuxx1
#elif defined(CONFIG_KMOPTI2)
#define CONFIG_KM_BOARD_NAME	"kmopti2"
#define CONFIG_HOSTNAME		kmopti2
#elif defined(CONFIG_KMTEPR2)
#define CONFIG_KM_BOARD_NAME    "kmtepr2"
#define CONFIG_HOSTNAME         kmtepr2
#else
#error ("Board not supported")
#endif

#define	CONFIG_SYS_TEXT_BASE	0xF0000000

/* include common defines/options for all 8321 Keymile boards */
#include "km/km8321-common.h"

#define CONFIG_SYS_APP1_BASE	0xA0000000    /* PAXG */
#define	CONFIG_SYS_APP1_SIZE	256 /* Megabytes */
#if defined(CONFIG_TUXX1) || defined(CONFIG_KMOPTI2) || defined(CONFIG_KMTEPR2)
#define CONFIG_SYS_APP2_BASE	0xB0000000    /* PINC3 */
#define	CONFIG_SYS_APP2_SIZE	256 /* Megabytes */
#endif

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

#if defined(CONFIG_KMTEPRO2)
/*
 * Configuration for C2 (NVRAM) on the local bus
 */
#define CONFIG_SYS_LBLAWBAR2_PRELIM    CONFIG_SYS_APP1_BASE
#define CONFIG_SYS_LBLAWAR2_PRELIM     (LBLAWAR_EN | LBLAWAR_256MB)
#define CONFIG_SYS_BR2_PRELIM  (CONFIG_SYS_APP1_BASE | \
				BR_PS_8 | \
				BR_MS_GPCM | \
				BR_V)
#define CONFIG_SYS_OR2_PRELIM  (MEG_TO_AM(CONFIG_SYS_APP1_SIZE) | \
				OR_GPCM_CSNT | \
				OR_GPCM_ACS_DIV2 | \
				OR_GPCM_XACS | \
				OR_GPCM_SCY_2 | \
				OR_GPCM_TRLX_SET | \
				OR_GPCM_EHTR_SET | \
				OR_GPCM_EAD)
#else
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
#endif

#if defined(CONFIG_TUXX1)
/*
 * Configuration for C3 on the local bus
 */
/* Access window base at PINC3 base */
#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_APP2_BASE
/* Window size: 256 MB */
#define CONFIG_SYS_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)

#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_APP2_BASE | \
				 BR_PS_8 |		\
				 BR_MS_GPCM |		\
				 BR_V)

#define CONFIG_SYS_OR3_PRELIM	(MEG_TO_AM(CONFIG_SYS_APP2_SIZE) | \
				 OR_GPCM_CSNT |	\
				 OR_GPCM_ACS_DIV2 | \
				 OR_GPCM_SCY_2 | \
				 OR_GPCM_TRLX_SET | \
				 OR_GPCM_EHTR_CLEAR)

#define CONFIG_SYS_MAMR		(MxMR_GPL_x4DIS | \
				 0x0000c000 | \
				 MxMR_WLFx_2X)
#endif

#if defined(CONFIG_KMOPTI2) || defined(CONFIG_KMTEPR2)
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
#endif

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

#if defined(CONFIG_TUGE1) || defined(CONFIG_KMSUPX5)
#define CONFIG_SYS_IBAT6L	(0)
#define CONFIG_SYS_IBAT6U	(0)
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#else
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
#endif
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U

#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U

#endif /* __CONFIG_H */

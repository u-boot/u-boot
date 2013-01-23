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
/* include common defines/options for all 8309 Keymile boards */
#include "km/km8309-common.h"
#else
#error Supported boards are: SUVD3, KMVECT1
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

#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_APP2_BASE
#define CONFIG_SYS_LBLAWAR3_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)

/*
 * MMU Setup
 */


/* APP1:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_APP1_BASE | BATL_PP_RW | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_APP1_BASE | BATU_BL_256M | \
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_APP1_BASE | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_APP2_BASE | BATL_PP_RW | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_APP2_BASE | BATU_BL_256M | \
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT6L	(CONFIG_SYS_APP2_BASE | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U

#endif /* __CONFIG_H */

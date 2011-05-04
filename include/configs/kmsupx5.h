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
 * (C) Copyright 2010
 * Lukas Roggli, KEYMILE Ltd, lukas.roggli@keymile.com
 *
 * (C) Copyright 2010-2011
 * Thomas Reufer, KEYMILE Ltd, thomas.reufer@keymile.com
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
#define CONFIG_KMSUPX5		1 /* Keymile PBEC8321 board specific */
#define CONFIG_HOSTNAME		supx5
#define CONFIG_KM_BOARD_NAME	"supx5"

#define	CONFIG_SYS_TEXT_BASE	0xF0000000

/* include common defines/options for all 8321 Keymile boards */
#include "km/km8321-common.h"

/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  2   Local   GPCM    8 bit  256MB	LPXF
 *  3   Local   not used
 *
 */

/*
 * LPXF on the local bus CS2
 * Window base at flash base
 * Window size: 256 MB
 */

#define	CONFIG_SYS_LPXF_BASE		0xA0000000    /* LPXF */
#define	CONFIG_SYS_LPXF_SIZE		256 /* Megabytes */

#define CONFIG_SYS_LBLAWBAR2_PRELIM	CONFIG_SYS_LPXF_BASE
#define CONFIG_SYS_LBLAWAR2_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)

#define CONFIG_SYS_BR2_PRELIM	(CONFIG_SYS_LPXF_BASE | \
				 BR_PS_8 | \
				 BR_MS_GPCM | \
				 BR_V)

#define CONFIG_SYS_OR2_PRELIM	(MEG_TO_AM(CONFIG_SYS_LPXF_SIZE) | \
				 OR_GPCM_CSNT | \
				 OR_GPCM_ACS_DIV4 | \
				 OR_GPCM_SCY_2 | \
				 (OR_GPCM_TRLX & \
				 (~OR_GPCM_EHTR)) |  /* EHTR = 0 */ \
				 OR_GPCM_EAD)

/* LPXF:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_LPXF_BASE | BATL_PP_10 | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_LPXF_BASE | BATU_BL_256M | \
				 BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_LPXF_BASE | BATL_PP_10 | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

/* Bank 3 not used */
#define CONFIG_SYS_IBAT6L       (0)
#define CONFIG_SYS_IBAT6U       (0)
#define CONFIG_SYS_DBAT6L       CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U       CONFIG_SYS_IBAT6U

#endif /* __CONFIG_H */

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
 * (C) Copyright 2010-2011
 * Lukas Roggli, KEYMILE Ltd, lukas.roggli@keymile.com
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
#define CONFIG_TUDA1		/* TUDA1 board specific */
#define CONFIG_HOSTNAME		tuda1
#define CONFIG_KM_BOARD_NAME   "tuda1"

#define	CONFIG_SYS_TEXT_BASE	0xF0000000

/* include common defines/options for all 8321 Keymile boards */
#include "km/km8321-common.h"

#define CONFIG_SYS_APP1_BASE	0xA0000000    /* PAXG */
#define	CONFIG_SYS_APP1_SIZE	256 /* Megabytes */
#define CONFIG_SYS_APP2_BASE	0xB0000000    /* PINC3 */
#define	CONFIG_SYS_APP2_SIZE	256 /* Megabytes */

/*
 * Local Bus Configuration & Clock Setup
 */
#define CONFIG_SYS_LCRR		(LCRR_DBYP | LCRR_EADC_1 | LCRR_CLKDIV_2)
#define CONFIG_SYS_LBC_LBCR	0x00000000

/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  2   Local   GPCM    8 bit  256MB	PAXG
 *  3   Local   GPCM    8 bit  256MB	PINC3
 *
 */

/*
 * PAXG on the local bus CS2
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
				 (OR_GPCM_TRLX & \
				 (~OR_GPCM_EHTR)) |  /* EHTR = 0 */ \
				 OR_GPCM_EAD)
/*
 * PINC3 on the local bus CS3
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
				 (OR_GPCM_ACS_DIV2 & /* ACS = 11 */\
				 (~OR_GPCM_XACS)) |  /* XACS = 0 */\
				 (OR_GPCM_SCY_2 & \
				 (~OR_GPCM_EHTR)) |  /* EHTR = 0 */ \
				 OR_GPCM_TRLX)

#define CONFIG_SYS_MAMR		(MxMR_GPL_x4DIS | \
				 0x0000c000 | \
				 MxMR_WLFx_2X)

/*
 * MMU Setup
 */
/* PAXG:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_APP1_BASE | \
				 BATL_PP_10 | \
				 BATL_MEMCOHERENCE)
/* 512M should also include APP2... */
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_APP1_BASE | \
				 BATU_BL_256M | \
				 BATU_VS | \
				 BATU_VP)
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_APP1_BASE | \
				 BATL_PP_10 | \
				 BATL_CACHEINHIBIT | \
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

/* PINC3:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_APP2_BASE | \
				 BATL_PP_10 | \
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_APP2_BASE | \
				 BATU_BL_256M | \
				 BATU_VS | \
				 BATU_VP)
#define CONFIG_SYS_DBAT6L	(CONFIG_SYS_APP2_BASE | \
				 BATL_PP_10 | \
				 BATL_CACHEINHIBIT | \
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U

#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U

#endif /* __CONFIG_H */

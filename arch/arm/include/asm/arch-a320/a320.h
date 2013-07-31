/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __A320_H
#define __A320_H

/*
 * Hardware register bases
 */
#define CONFIG_FTSMC020_BASE	0x90200000	/* Static Memory Controller */
#define CONFIG_DEBUG_LED	0x902ffffc	/* Debug LED */
#define CONFIG_FTSDMC020_BASE	0x90300000	/* SDRAM Controller */
#define CONFIG_FTMAC100_BASE	0x90900000	/* Ethernet */
#define CONFIG_FTPMU010_BASE	0x98100000	/* Power Management Unit */
#define CONFIG_FTTMR010_BASE	0x98400000	/* Timer */
#define CONFIG_FTRTC010_BASE	0x98600000	/* Real Time Clock*/

#endif	/* __A320_H */

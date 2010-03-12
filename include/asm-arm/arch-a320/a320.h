/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

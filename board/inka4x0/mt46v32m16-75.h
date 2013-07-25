/*
 * Copyright (C) 2007 Semihalf
 * Written by Marian Balakowicz <m8@semihalf.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define SDRAM_DDR	1		/* is DDR */

/* Settings for XLB = 132 MHz */
#define SDRAM_MODE	0x018D0000
#define SDRAM_EMODE	0x40090000
#define SDRAM_CONTROL	0x714F0F00
#define SDRAM_CONFIG1	0x73711930
#define SDRAM_CONFIG2	0x46770000
#define SDRAM_TAPDELAY	0x10000000

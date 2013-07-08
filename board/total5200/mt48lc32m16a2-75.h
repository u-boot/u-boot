/*
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@freescale.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Micron MT48LC32M16A2-75 is compatible to:
 *  - Infineon HYB39S512160AT-75
 */

#define SDRAM_DDR	0		/* is SDR */

/* Settings for XLB = 132 MHz */
#define SDRAM_MODE	0x00CD0000
#define SDRAM_CONTROL	0x514F0000
#define SDRAM_CONFIG1	0xD2322800
#define SDRAM_CONFIG2	0x8AD70000

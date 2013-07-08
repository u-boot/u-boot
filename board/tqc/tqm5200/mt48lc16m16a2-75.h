/*
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define SDRAM_DDR	0		/* is SDR */

/* Settings for XLB = 132 MHz */
#define SDRAM_MODE	0x00CD0000
/* #define SDRAM_MODE	0x008D0000 */ /* CAS latency 2 */
#define SDRAM_CONTROL	0x504F0000
#define SDRAM_CONFIG1	0xD2322800
/* #define SDRAM_CONFIG1	0xD2222800 */ /* CAS latency 2 */
/*#define SDRAM_CONFIG1	0xD7322800 */ /* SDRAM controller bug workaround */
#define SDRAM_CONFIG2	0x8AD70000
/*#define SDRAM_CONFIG2	0xDDD70000 */ /* SDRAM controller bug workaround */

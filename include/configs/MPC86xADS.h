/*
  * A collection of structures, addresses, and values associated with
  * the Motorola MPC8xxADS board.  Copied from the FADS config.
  *
  * Copyright (c) 1998 Dan Malek (dmalek@jlc.net)
  *
  * Modified by, Yuli Barcohen, Arabella Software Ltd., yuli@arabellasw.com
  *
  * Values common to all FADS family boards are in board/fads/fads.h
  */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

/* board type */
#define CONFIG_MPC86xADS        1       /* new ADS */
#define CONFIG_FADS		1       /* We are FADS compatible (more or less) */

/* New MPC86xADS - pick one of these */
#define CONFIG_MPC866T 		1
#undef CONFIG_MPC866P
#undef CONFIG_MPC859T
#undef CONFIG_MPC859DSL
#undef CONFIG_MPC852T

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		38400

# define CFG_8XX_FACT		5		/* Multiply by 5	*/
# define CFG_8XX_XIN		10000000	/* 10 MHz in	*/

#define CONFIG_DRAM_50MHZ       1
#define CONFIG_SDRAM_50MHZ      1

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		14-22
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control
 */
#define CFG_PLPRCR ((CFG_8XX_FACT << PLPRCR_MFI_SHIFT) | PLPRCR_TEXPS)

#include "fads.h"

#endif	/* __CONFIG_H */

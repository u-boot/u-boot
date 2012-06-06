/*
  * A collection of structures, addresses, and values associated with
  * the Motorola 860T FADS board.  Copied from the MBX stuff.
  * Magnus Damm added defines for 8xxrom and extended bd_info.
  * Helmut Buchsbaum added bitvalues for BCSRx
  *
  * Copyright (c) 1998 Dan Malek (dmalek@jlc.net)
  *
  * Modified by, Yuli Barcohen, Arabella Software Ltd., yuli@arabellasw.com
  *
  * Values common to all FADS family boards are in board/fads/fads.h
  */

#ifndef __CONFIG_H
#define __CONFIG_H

/* board type */
#define CONFIG_FADS		1       /* old/new FADS + new ADS */

/* processor type */
#define CONFIG_MPC860T		1       /* 860T */

#define	CONFIG_SYS_TEXT_BASE	0xFE000000

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		38400
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/

#if 0 /* old FADS */
# define CONFIG_SYS_8XX_FACT		12	/* 4 MHz oscillator on EXTCLK */
#else /* new FADS */
# define CONFIG_SYS_8XX_FACT		10	/* 5 MHz oscillator on EXTCLK */
#endif

#define CONFIG_SYS_PLPRCR  (((CONFIG_SYS_8XX_FACT-1) << PLPRCR_MF_SHIFT) |	\
		PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

#define CONFIG_DRAM_50MHZ		1
#define CONFIG_SDRAM_50MHZ              1

#include "../../board/fads/fads.h"

#ifdef USE_REAL_FLASH_VALUES
/*
 * These values fit our FADS860T ...
 * The "default" behaviour with 1Mbyte initial doesn't work for us!
 */
#undef CONFIG_SYS_OR0_PRELIM
#undef CONFIG_SYS_BR0_PRELIM
#define CONFIG_SYS_OR0_PRELIM	0x0FFC00D34 /* Real values for the board */
#define CONFIG_SYS_BR0_PRELIM	0x02800001  /* Real values for the board */
#endif

#define CONFIG_SYS_DAUGHTERBOARD /* FADS has processor-specific daughterboard */

#endif	/* __CONFIG_H */

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

/* CPU type - pick one of these */
#define CONFIG_MPC866T		1
#undef CONFIG_MPC866P
#undef CONFIG_MPC859T
#undef CONFIG_MPC859DSL
#undef CONFIG_MPC852T

#define	CONFIG_SYS_TEXT_BASE	0xFE000000

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		38400

#define CONFIG_8xx_OSCLK		10000000 /* 10MHz oscillator on EXTCLK  */
#define CONFIG_8xx_CPUCLK_DEFAULT	50000000
#define CONFIG_SYS_8xx_CPUCLK_MIN		40000000
#define CONFIG_SYS_8xx_CPUCLK_MAX		80000000

#define CONFIG_DRAM_50MHZ       1
#define CONFIG_SDRAM_50MHZ      1

#include "../../board/fads/fads.h"

#define CONFIG_SYS_OR5_PRELIM		0xFFFF8110	/* 64Kbyte address space */
#define CONFIG_SYS_BR5_PRELIM		(CONFIG_SYS_PHYDEV_ADDR | BR_PS_8 | BR_V)

#endif	/* __CONFIG_H */

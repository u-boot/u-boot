/*
  * A collection of structures, addresses, and values associated with
  * the Motorola 860 ADS board.	 Copied from the MBX stuff.
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

/* Board type */
#define CONFIG_ADS		1	/* Old Motorola MPC821/860ADS */

/* Processor type */
#define CONFIG_MPC860		1

#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1 */
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE

#define CONFIG_BAUDRATE		38400	/* Console baudrate */

#if 0
#define CONFIG_SYS_8XX_FACT		1526	/* 32.768 kHz crystal on XTAL/EXTAL */
#else
#define CONFIG_SYS_8XX_FACT		12	/* 4 MHz oscillator on EXTCLK */
#endif

#define CONFIG_SYS_PLPRCR  (((CONFIG_SYS_8XX_FACT-1) << PLPRCR_MF_SHIFT) |	\
		PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

#define CONFIG_DRAM_50MHZ		1


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_PCMCIA
#define CONFIG_CMD_PING

/* This is picked up again in fads.h */
#define FADS_COMMANDS_ALREADY_DEFINED

#include "../../board/fads/fads.h"

#define CONFIG_SYS_PC_IDE_RESET	((ushort)0x0008)    /* PC 12	*/

#endif	/* __CONFIG_H */

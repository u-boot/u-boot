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

/* CFG_8XX_FACT * CFG_8XX_XIN = 50 MHz */
#if 0
#define CFG_8XX_XIN		32768	/* 32.768 kHz input frequency	*/
#define CFG_8XX_FACT		0x5F6	/* Multiply by 1526 */
#else
#define CFG_8XX_XIN		4000000	/* 4 MHz input frequency	*/
#define CFG_8XX_FACT		12	/* Multiply by 12 */
#endif

#define CONFIG_COMMANDS (CONFIG_CMD_DFL   \
			 | CFG_CMD_DHCP   \
			 | CFG_CMD_IMMAP  \
			 | CFG_CMD_PCMCIA \
			 | CFG_CMD_PING   \
			)

#define CONFIG_DRAM_50MHZ		1

#include "fads.h"

#define CFG_PLPRCR  (((CFG_8XX_FACT-1) << PLPRCR_MF_SHIFT) |	\
		PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

#define CFG_PC_IDE_RESET	((ushort)0x0008)    /* PC 12	*/

#endif	/* __CONFIG_H */

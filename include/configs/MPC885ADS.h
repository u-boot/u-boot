/*
 * A collection of structures, addresses, and values associated with
 * the Motorola DUET ADS board. Values common to all FADS family boards
 * are in board/fads/fads.h
 *
 * Copyright (C) 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Board type */
#define CONFIG_MPC885ADS	        1	/* Duet (MPC87x/88x) ADS */
#define CONFIG_FADS		1	/* We are FADS compatible (more or less) */

#define CONFIG_MPC885 		1	/* MPC885 CPU (Duet family) */

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		38400

#define CONFIG_8xx_OSCLK	10000000 /* 10 MHz oscillator on EXTCLK  */

#define CFG_PLPRCR		((1 << PLPRCR_MFD_SHIFT) | (12 << PLPRCR_MFI_SHIFT) | PLPRCR_TEXPS)

#define CONFIG_SDRAM_50MHZ      1

#define CONFIG_COMMANDS	(CONFIG_CMD_DFL   \
			 | CFG_CMD_DHCP   \
			 | CFG_CMD_IMMAP  \
			 | CFG_CMD_MII    \
			 | CFG_CMD_PING   \
			)

#include "fads.h"

#undef CFG_SCCR
#define CFG_SCCR	(SCCR_TBS|SCCR_EBDF11)

#define CFG_OR5_PRELIM		0xFFFF8110	/* 64Kbyte address space */
#define CFG_BR5_PRELIM		(CFG_PHYDEV_ADDR | BR_PS_8 | BR_V)

#define CONFIG_HAS_ETH1

#endif	/* __CONFIG_H */

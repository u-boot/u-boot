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
#define CONFIG_DUET_ADS	        1	/* Duet (MPC87x/88x) ADS */
#define CONFIG_FADS		1	/* We are FADS compatible (more or less) */

#define CONFIG_MPC885 		1	/* MPC885 CPU (Duet family) */

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		38400

#define CFG_8XX_FACT		5		/* Multiply by 5	*/
#define CFG_8XX_XIN		10000000	/* 10 MHz in	*/

#define CONFIG_SDRAM_50MHZ      1

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		14-22
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control
 */
#define CFG_PLPRCR ((CFG_8XX_FACT << PLPRCR_MFI_SHIFT) | PLPRCR_TEXPS)

#include "fads.h"

#define CFG_PHYDEV_ADDR		(BCSR_ADDR + 0x20000)

#define CFG_OR5_PRELIM		0xFFFF8110	/* 64Kbyte address space */
#define CFG_BR5_PRELIM		(CFG_PHYDEV_ADDR | BR_PS_8 | BR_V)

#define BCSR5			(CFG_PHYDEV_ADDR + 0x300)

#define BCSR5_MII2_EN		0x40
#define BCSR5_MII2_RST		0x20
#define BCSR5_T1_RST		0x10
#define BCSR5_ATM155_RST	0x08
#define BCSR5_ATM25_RST		0x04
#define BCSR5_MII1_EN		0x02
#define BCSR5_MII1_RST		0x01

#endif	/* __CONFIG_H */

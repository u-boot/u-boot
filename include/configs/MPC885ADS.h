/*
 * A collection of structures, addresses, and values associated with
 * the Motorola MPC885ADS board. Values common to all FADS family boards
 * are in board/fads/fads.h
 *
 * Copyright (C) 2003-2004 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MPC885ADS	1	/* MPC885ADS board */
#define CONFIG_FADS		1	/* We are FADS compatible (more or less) */

#define CONFIG_MPC885		1	/* MPC885 CPU (Duet family) */

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1 */
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		38400

#define CONFIG_8xx_OSCLK		10000000 /* 10 MHz oscillator on EXTCLK  */
#define CONFIG_8xx_CPUCLK_DEFAULT	50000000
#define CFG_8xx_CPUCLK_MIN		40000000
#define CFG_8xx_CPUCLK_MAX		133000000

#define CONFIG_SDRAM_50MHZ      1

#include "fads.h"

#define CFG_OR5_PRELIM		0xFFFF8110	/* 64Kbyte address space */
#define CFG_BR5_PRELIM		(CFG_PHYDEV_ADDR | BR_PS_8 | BR_V)

#define CONFIG_HAS_ETH1

#endif	/* __CONFIG_H */

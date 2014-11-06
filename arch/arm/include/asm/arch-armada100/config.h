/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This file should be included in board config header file.
 *
 * It supports common definitions for Armada100 platform
 */

#ifndef _ARMD1_CONFIG_H
#define _ARMD1_CONFIG_H

#include <asm/arch/armada100.h>
/* default Dcache Line length for armada100 */
#define CONFIG_SYS_CACHELINE_SIZE       32

#define CONFIG_SYS_TCLK		(14745600)	/* NS16550 clk config */
#define CONFIG_SYS_HZ_CLOCK	(3250000)	/* Timer Freq. 3.25MHZ */
#define CONFIG_MARVELL_MFP			/* Enable mvmfp driver */
#define MV_MFPR_BASE		ARMD1_MFPR_BASE
#define MV_UART_CONSOLE_BASE	ARMD1_UART1_BASE
#define CONFIG_SYS_NS16550_IER	(1 << 6)	/* Bit 6 in UART_IER register
						represents UART Unit Enable */
/*
 * I2C definition
 */
#ifdef CONFIG_CMD_I2C
#define CONFIG_I2C_MV		1
#define CONFIG_MV_I2C_NUM	2
#define CONFIG_I2C_MULTI_BUS	1
#define CONFIG_MV_I2C_REG	{0xd4011000, 0xd4025000}
#define CONFIG_HARD_I2C		1
#define CONFIG_SYS_I2C_SPEED	0
#define CONFIG_SYS_I2C_SLAVE	0xfe
#endif

#endif /* _ARMD1_CONFIG_H */

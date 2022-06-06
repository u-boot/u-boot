/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 */

/*
 * This file should be included in board config header file.
 *
 * It supports common definitions for MVEBU platforms
 */

#ifndef _MVEBU_CONFIG_H
#define _MVEBU_CONFIG_H

#include <asm/arch/soc.h>

#if defined(CONFIG_ARMADA_XP) || defined(CONFIG_ARMADA_375) \
	|| defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_MSYS)
/*
 * Set this for the common xor register definitions needed in dram.c
 * for A38x as well here.
 */
#define MV88F78X60 /* for the DDR training bin_hdr code */
#endif

#define CONFIG_SYS_L2_PL310

#define MV_UART_CONSOLE_BASE		MVEBU_UART0_BASE

/* Needed for SPI NOR booting in SPL */
#define CONFIG_DM_SEQ_ALIAS		1

/*
 * I2C related stuff
 */
#ifdef CONFIG_CMD_I2C
#ifndef CONFIG_SYS_I2C_SOFT
#define CONFIG_I2C_MVTWSI
#endif
#endif

/* Use common timer */
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(MVEBU_TIMER_BASE + 0x14)
#define CONFIG_SYS_TIMER_RATE		25000000

#endif /* __MVEBU_CONFIG_H */

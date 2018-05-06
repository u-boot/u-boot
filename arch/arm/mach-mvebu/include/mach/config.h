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
	|| defined(CONFIG_ARMADA_38X)
/*
 * Set this for the common xor register definitions needed in dram.c
 * for A38x as well here.
 */
#define MV88F78X60 /* for the DDR training bin_hdr code */
#endif

#define CONFIG_SYS_L2_PL310

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#endif

/*
 * By default kwbimage.cfg from board specific folder is used
 * If for some board, different configuration file need to be used,
 * CONFIG_SYS_KWD_CONFIG should be defined in board specific header file
 */
#ifndef CONFIG_SYS_KWD_CONFIG
#define	CONFIG_SYS_KWD_CONFIG	$(CONFIG_BOARDDIR)/kwbimage.cfg
#endif /* CONFIG_SYS_KWD_CONFIG */

/* Add target to build it automatically upon "make" */
#ifdef CONFIG_SPL
#define CONFIG_BUILD_TARGET	"u-boot-spl.kwb"
#endif

/* end of 16M scrubbed by training in bootrom */
#define CONFIG_SYS_INIT_SP_ADDR		0x00FF0000
#define CONFIG_NR_DRAM_BANKS_MAX	2

#define MV_UART_CONSOLE_BASE		MVEBU_UART0_BASE

/*
 * SPI Flash configuration
 */
#ifdef CONFIG_CMD_SF
#ifndef CONFIG_ENV_SPI_BUS
# define CONFIG_ENV_SPI_BUS		0
#endif
#ifndef CONFIG_ENV_SPI_CS
# define CONFIG_ENV_SPI_CS		0
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
# define CONFIG_ENV_SPI_MAX_HZ		50000000
#endif
#endif

/* Needed for SPI NOR booting in SPL */
#define CONFIG_DM_SEQ_ALIAS		1

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MII		/* expose smi ove miiphy interface */
#define CONFIG_ENV_OVERWRITE	/* ethaddr can be reprogrammed */
#define CONFIG_ARP_TIMEOUT	200
#define CONFIG_NET_RETRY_COUNT	50
#endif /* CONFIG_CMD_NET */

/*
 * I2C related stuff
 */
#ifdef CONFIG_CMD_I2C
#ifndef CONFIG_SYS_I2C_SOFT
#define CONFIG_I2C_MVTWSI
#endif
#define CONFIG_SYS_I2C_SLAVE		0x0
#define CONFIG_SYS_I2C_SPEED		100000
#endif

/* Use common timer */
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(MVEBU_TIMER_BASE + 0x14)
#define CONFIG_SYS_TIMER_RATE		25000000

#endif /* __MVEBU_CONFIG_H */

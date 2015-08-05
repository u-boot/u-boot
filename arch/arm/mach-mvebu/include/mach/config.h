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
 * It supports common definitions for MVEBU platforms
 */

#ifndef _MVEBU_CONFIG_H
#define _MVEBU_CONFIG_H

#include <asm/arch/soc.h>

#if defined(CONFIG_ARMADA_XP)
#define MV88F78X60 /* for the DDR training bin_hdr code */
#endif

#define CONFIG_SYS_CACHELINE_SIZE	32

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
#define CONFIG_HARD_SPI			1
#define CONFIG_KIRKWOOD_SPI		1
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

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_MII		/* expose smi ove miiphy interface */
#define CONFIG_MVNETA		/* Enable Marvell Gbe Controller Driver */
#define CONFIG_PHYLIB
#define CONFIG_ENV_OVERWRITE	/* ethaddr can be reprogrammed */
#define CONFIG_PHY_GIGE		/* GbE speed/duplex detect */
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

/* Common SPL configuration */
#ifndef CONFIG_SPL_LDSCRIPT
#define CONFIG_SPL_LDSCRIPT		"arch/arm/mach-mvebu/u-boot-spl.lds"
#endif

#endif /* __MVEBU_CONFIG_H */

/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PANTHEON_CONFIG_H
#define _PANTHEON_CONFIG_H

#include <asm/arch/pantheon.h>

/* default Dcache Line length for pantheon */
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_TCLK		(14745600)	/* NS16550 clk config */
#define CONFIG_SYS_HZ_CLOCK	(3250000)	/* Timer Freq. 3.25MHZ */
#define CONFIG_MARVELL_MFP			/* Enable mvmfp driver */
#define MV_MFPR_BASE		PANTHEON_MFPR_BASE
#define MV_UART_CONSOLE_BASE	PANTHEON_UART1_BASE
#define CONFIG_SYS_NS16550_IER	(1 << 6)	/* Bit 6 in UART_IER register
						represents UART Unit Enable */
/*
 * I2C definition
 */
#ifdef CONFIG_CMD_I2C
#define CONFIG_I2C_MV			1
#define CONFIG_MV_I2C_REG		0xd4011000
#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		0
#define CONFIG_SYS_I2C_SLAVE		0xfe
#endif

/*
 * MMC definition
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_CMD_FAT			1
#define CONFIG_MMC			1
#define CONFIG_GENERIC_MMC		1
#define CONFIG_SDHCI			1
#define CONFIG_MMC_SDHCI_IO_ACCESSORS	1
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	0x1000
#define CONFIG_MMC_SDMA			1
#define CONFIG_MV_SDHCI			1
#define CONFIG_DOS_PARTITION		1
#define CONFIG_EFI_PARTITION		1
#define CONFIG_SYS_MMC_NUM		2
#define CONFIG_SYS_MMC_BASE		{0xD4280000, 0xd4281000}
#endif

#endif /* _PANTHEON_CONFIG_H */

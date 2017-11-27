/*
 * include/configs/ulcb.h
 *     This file is ULCB board configuration.
 *
 * Copyright (C) 2017 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __ULCB_H
#define __ULCB_H

#undef DEBUG

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Board Clock */
/* XTAL_CLK : 33.33MHz */
#define CONFIG_SYS_CLK_FREQ	33333333u

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

/* CPLD SPI */
#define CONFIG_CMD_SPI
#define CONFIG_SOFT_SPI
#define SPI_DELAY	udelay(0)
#define SPI_SDA(val)	ulcb_softspi_sda(val)
#define SPI_SCL(val)	ulcb_softspi_scl(val)
#define SPI_READ	ulcb_softspi_read()
#ifndef	__ASSEMBLY__
void ulcb_softspi_sda(int);
void ulcb_softspi_scl(int);
unsigned char ulcb_softspi_read(void);
#endif

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2

#endif /* __ULCB_H */

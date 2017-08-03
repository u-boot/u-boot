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

#define CONFIG_RCAR_BOARD_STRING "ULCB"

#include "rcar-gen3-common.h"

/* M3 ULCB has 2 banks, each with 1 GiB of RAM */
#if defined(CONFIG_R8A7796)
#undef PHYS_SDRAM_1_SIZE
#undef PHYS_SDRAM_2_SIZE
#define PHYS_SDRAM_1_SIZE		(0x40000000u - DRAM_RSV_SIZE)
#define PHYS_SDRAM_2_SIZE		0x40000000u
#endif

/* SCIF */
#define CONFIG_CONS_SCIF2
#define CONFIG_CONS_INDEX	2
#define CONFIG_SH_SCIF_CLK_FREQ        CONFIG_S3D4_CLK_FREQ

/* [A] Hyper Flash */
/* use to RPC(SPI Multi I/O Bus Controller) */

/* Ethernet RAVB */
#define CONFIG_NET_MULTI
#define CONFIG_PHY_MICREL
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Board Clock */
/* XTAL_CLK : 33.33MHz */
#define RCAR_XTAL_CLK		33333333u
#define CONFIG_SYS_CLK_FREQ	RCAR_XTAL_CLK
/* ch0to2 CPclk, ch3to11 S3D2_PEREclk, ch12to14 S3D2_RTclk */
/* CPclk 16.66MHz, S3D2 133.33MHz , S3D4 66.66MHz          */
#define CONFIG_CP_CLK_FREQ	(CONFIG_SYS_CLK_FREQ / 2)
#define CONFIG_PLL1_CLK_FREQ	(CONFIG_SYS_CLK_FREQ * 192 / 2)
#define CONFIG_S3D2_CLK_FREQ	(266666666u/2)
#define CONFIG_S3D4_CLK_FREQ	(266666666u/4)

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

/* Generic Interrupt Controller Definitions */
#define CONFIG_GICV2
#define GICD_BASE	0xF1010000
#define GICC_BASE	0xF1020000

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

/* i2c */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SH
#define CONFIG_SYS_I2C_SLAVE		0x60
#define CONFIG_SYS_I2C_SH_NUM_CONTROLLERS	1
#define CONFIG_SYS_I2C_SH_SPEED0	400000
#define CONFIG_SH_I2C_DATA_HIGH		4
#define CONFIG_SH_I2C_DATA_LOW		5
#define CONFIG_SH_I2C_CLOCK		10000000

#define CONFIG_SYS_I2C_POWERIC_ADDR	0x30

/* USB */
#ifdef CONFIG_R8A7795
#define CONFIG_USB_MAX_CONTROLLER_COUNT	3
#else
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#endif

/* SDHI */
#define CONFIG_SH_SDHI_FREQ		200000000

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2

/* Module stop status bits */
/* MFIS, SCIF1 */
#define CONFIG_SMSTP2_ENA	0x00002040
/* SCIF2 */
#define CONFIG_SMSTP3_ENA	0x00000400
/* INTC-AP, IRQC */
#define CONFIG_SMSTP4_ENA	0x00000180

#endif /* __ULCB_H */

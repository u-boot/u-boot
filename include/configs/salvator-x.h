/*
 * include/configs/salvator-x.h
 *     This file is Salvator-X board configuration.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __SALVATOR_X_H
#define __SALVATOR_X_H

#undef DEBUG

#define CONFIG_RCAR_BOARD_STRING "Salvator-X"

#include "rcar-gen3-common.h"

/* SCIF */
#define CONFIG_SCIF_CONSOLE
#define CONFIG_CONS_SCIF2
#define CONFIG_CONS_INDEX	2
#define CONFIG_SH_SCIF_CLK_FREQ	CONFIG_SYS_CLK_FREQ

/* [A] Hyper Flash */
/* use to RPC(SPI Multi I/O Bus Controller) */
#define CONFIG_ENV_IS_NOWHERE

/* Board Clock */
/* XTAL_CLK : 33.33MHz */
#define RCAR_XTAL_CLK		33333333u
#define CONFIG_SYS_CLK_FREQ	RCAR_XTAL_CLK
/* ch0to2 CPclk, ch3to11 S3D2_PEREclk, ch12to14 S3D2_RTclk */
/* CPclk 16.66MHz, S3D2 133.33MHz                          */
#define CONFIG_CP_CLK_FREQ	(CONFIG_SYS_CLK_FREQ / 2)
#define CONFIG_PLL1_CLK_FREQ	(CONFIG_SYS_CLK_FREQ * 192 / 2)
#define CONFIG_S3D2_CLK_FREQ	(266666666u/2)

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

/* Generic Interrupt Controller Definitions */
#define CONFIG_GICV2
#define GICD_BASE	0xF1010000
#define GICC_BASE	0xF1020000

/* Module stop status bits */
/* MFIS, SCIF1 */
#define CONFIG_SMSTP2_ENA	0x00002040
/* INTC-AP, IRQC */
#define CONFIG_SMSTP4_ENA	0x00000180

#endif /* __SALVATOR_X_H */

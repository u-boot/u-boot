/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PANTHEON_H
#define _PANTHEON_H

/* Common APB clock register bit definitions */
#define APBC_APBCLK     (1<<0)  /* APB Bus Clock Enable */
#define APBC_FNCLK      (1<<1)  /* Functional Clock Enable */
#define APBC_RST        (1<<2)  /* Reset Generation */
/* Functional Clock Selection Mask */
#define APBC_FNCLKSEL(x)        (((x) & 0xf) << 4)

/* Common APMU register bit definitions */
#define APMU_PERI_CLK	(1<<4)	/* Peripheral Clock Enable */
#define APMU_AXI_CLK	(1<<3)	/* AXI Clock Enable*/
#define APMU_PERI_RST	(1<<1)	/* Peripheral Reset */
#define APMU_AXI_RST	(1<<0)	/* AXI Reset */

/* Register Base Addresses */
#define PANTHEON_DRAM_BASE	0xB0000000
#define PANTHEON_TIMER_BASE	0xD4014000
#define PANTHEON_WD_TIMER_BASE	0xD4080000
#define PANTHEON_APBC_BASE	0xD4015000
#define PANTHEON_UART1_BASE	0xD4017000
#define PANTHEON_UART2_BASE	0xD4018000
#define PANTHEON_GPIO_BASE	0xD4019000
#define PANTHEON_MFPR_BASE	0xD401E000
#define PANTHEON_MPMU_BASE	0xD4050000
#define PANTHEON_APMU_BASE	0xD4282800
#define PANTHEON_CPU_BASE	0xD4282C00

#endif /* _PANTHEON_H */

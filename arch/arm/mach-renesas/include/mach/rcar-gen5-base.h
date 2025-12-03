/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2025 Renesas Electronics Corp.
 */

#ifndef __ASM_ARCH_RCAR_GEN5_BASE_H
#define __ASM_ARCH_RCAR_GEN5_BASE_H

/*
 * R-Car (R8A78000) I/O Addresses
 */
#define TMU_BASE		0x1C030000

/* Arm Generic Timer */
#define CNTCR_BASE		0x1C000FFF /* Region 0 */
#define CNTFID0			(CNTCR_BASE + 0x020)
#define CNTCR_EN		BIT(0)

/* Reset */
#define RST_BASE		0xC1320000 /* Domain0 */
#define RST_SWSRES1A		(RST_BASE + 0x410)
#define RST_WDTRSTCR		(RST_BASE + 0x420)
#define RST_RWDT_RSTMSK		BIT(0)
#define RST_WWDT_RSTMSK		BIT(2)
#define RST_RESKCPROT0		(RST_BASE + 0x4F0)
#define RST_KCPROT_DIS		0xA5A5A501

/* GICv4 */
/* Distributor Registers */
#define GICD_BASE		0x38000000
#define GICR_BASE		(GICR_LPI_BASE)

/* ReDistributor Registers for Control and Physical LPIs */
#define GICR_LPI_BASE		0x38080000
#define GICR_WAKER		0x0014
#define GICR_PWRR		0x0024
#define GICR_LPI_WAKER		(GICR_LPI_BASE + GICR_WAKER)
#define GICR_LPI_PWRR		(GICR_LPI_BASE + GICR_PWRR)

/* ReDistributor Registers for SGIs and PPIs */
#define GICR_SGI_BASE		0x38090000
#define GICR_IGROUPR0		0x0080

#endif /* __ASM_ARCH_RCAR_GEN5_BASE_H */

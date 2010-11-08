/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91_rstc.h]
 *
 * Copyright (C) 2007 Andrew Victor
 * Copyright (C) 2007 Atmel Corporation.
 *
 * Reset Controller (RSTC) - System peripherals regsters.
 * Based on AT91SAM9261 datasheet revision D.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef AT91_RSTC_H
#define AT91_RSTC_H

#define AT91_ASM_RSTC_MR	(AT91_RSTC_BASE + 0x08)

#ifndef __ASSEMBLY__

typedef struct at91_rstc {
	u32	cr;	/* Reset Controller Control Register */
	u32	sr;	/* Reset Controller Status Register */
	u32	mr;	/* Reset Controller Mode Register */
} at91_rstc_t;

#endif /* __ASSEMBLY__ */

#define AT91_RSTC_KEY		0xA5000000

#define AT91_RSTC_CR_PROCRST	0x00000001
#define AT91_RSTC_CR_PERRST	0x00000004
#define AT91_RSTC_CR_EXTRST	0x00000008

#define AT91_RSTC_MR_URSTEN	0x00000001
#define AT91_RSTC_MR_URSTIEN	0x00000010
#define AT91_RSTC_MR_ERSTL(x)	((x & 0xf) << 8)
#define AT91_RSTC_MR_ERSTL_MASK	0x0000FF00

#define AT91_RSTC_SR_NRSTL	0x00010000

#ifdef CONFIG_AT91_LEGACY

#define AT91_RSTC_CR		(AT91_RSTC + 0x00)	/* Reset Controller Control Register */
#define		AT91_RSTC_PROCRST	(1 << 0)		/* Processor Reset */
#define		AT91_RSTC_PERRST	(1 << 2)		/* Peripheral Reset */
#define		AT91_RSTC_EXTRST	(1 << 3)		/* External Reset */

#define AT91_RSTC_SR		(AT91_RSTC + 0x04)	/* Reset Controller Status Register */
#define		AT91_RSTC_URSTS		(1 << 0)		/* User Reset Status */
#define		AT91_RSTC_RSTTYP	(7 << 8)		/* Reset Type */
#define			AT91_RSTC_RSTTYP_GENERAL	(0 << 8)
#define			AT91_RSTC_RSTTYP_WAKEUP		(1 << 8)
#define			AT91_RSTC_RSTTYP_WATCHDOG	(2 << 8)
#define			AT91_RSTC_RSTTYP_SOFTWARE	(3 << 8)
#define			AT91_RSTC_RSTTYP_USER	(4 << 8)
#define		AT91_RSTC_NRSTL		(1 << 16)		/* NRST Pin Level */
#define		AT91_RSTC_SRCMP		(1 << 17)		/* Software Reset Command in Progress */

#define AT91_RSTC_MR		(AT91_RSTC + 0x08)	/* Reset Controller Mode Register */
#define		AT91_RSTC_URSTEN	(1 << 0)		/* User Reset Enable */
#define		AT91_RSTC_URSTIEN	(1 << 4)		/* User Reset Interrupt Enable */
#define		AT91_RSTC_ERSTL		(0xf << 8)		/* External Reset Length */

#endif /* CONFIG_AT91_LEGACY */

#endif

/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91_pit.h]
 *
 * Copyright (C) 2007 Andrew Victor
 * Copyright (C) 2007 Atmel Corporation.
 *
 * Periodic Interval Timer (PIT) - System peripherals regsters.
 * Based on AT91SAM9261 datasheet revision D.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef AT91_PIT_H
#define AT91_PIT_H

typedef struct at91_pit {
	u32	mr;	/* 0x00 Mode Register */
	u32	sr;	/* 0x04 Status Register */
	u32	pivr;	/* 0x08 Periodic Interval Value Register */
	u32	piir;	/* 0x0C Periodic Interval Image Register */
} at91_pit_t;

#define		AT91_PIT_MR_IEN		0x02000000
#define		AT91_PIT_MR_EN		0x01000000
#define		AT91_PIT_MR_PIV_MASK	(x & 0x000fffff)
#define		AT91_PIT_MR_PIV(x)	(x & AT91_PIT_MR_PIV_MASK)

#ifdef CONFIG_AT91_LEGACY

#define AT91_PIT_MR		(AT91_PIT + 0x00)	/* Mode Register */
#define		AT91_PIT_PITIEN		(1 << 25)		/* Timer Interrupt Enable */
#define		AT91_PIT_PITEN		(1 << 24)		/* Timer Enabled */
#define		AT91_PIT_PIV		(0xfffff)		/* Periodic Interval Value */

#define AT91_PIT_SR		(AT91_PIT + 0x04)	/* Status Register */
#define		AT91_PIT_PITS		(1 << 0)		/* Timer Status */

#define AT91_PIT_PIVR		(AT91_PIT + 0x08)	/* Periodic Interval Value Register */
#define AT91_PIT_PIIR		(AT91_PIT + 0x0c)	/* Periodic Interval Image Register */
#define		AT91_PIT_PICNT		(0xfff << 20)		/* Interval Counter */
#define		AT91_PIT_CPIV		(0xfffff)		/* Inverval Value */

#endif /* CONFIG_AT91_LEGACY */
#endif

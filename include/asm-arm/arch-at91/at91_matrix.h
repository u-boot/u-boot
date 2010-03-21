/*
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef AT91_MATRIX_H
#define AT91_MATRIX_H

#ifdef __ASSEMBLY__

#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9G20)
#define AT91_ASM_MATRIX_CSA0	(AT91_MATRIX_BASE + 0x11C)
#elif defined(CONFIG_AT91SAM9261)
#define AT91_ASM_MATRIX_CSA0	(AT91_MATRIX_BASE + 0x30)
#elif defined(CONFIG_AT91SAM9263)
#define AT91_ASM_MATRIX_CSA0	(AT91_MATRIX_BASE + 0x120)
#elif defined(CONFIG_AT91SAM9G45)
#define AT91_ASM_MATRIX_CSA0	(AT91_MATRIX_BASE + 0x128)
#else
#error AT91_ASM_MATRIX_CSA0 is not definied for current CPU
#endif

#define AT91_ASM_MATRIX_MCFG	AT91_MATRIX_BASE

#else
#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9G20)
#define AT91_MATRIX_MASTERS	6
#define AT91_MATRIX_SLAVES	5
#elif defined(CONFIG_AT91SAM9261)
#define AT91_MATRIX_MASTERS	1
#define AT91_MATRIX_SLAVES	5
#elif defined(CONFIG_AT91SAM9263)
#define AT91_MATRIX_MASTERS	9
#define AT91_MATRIX_SLAVES	7
#elif defined(CONFIG_AT91SAM9G45)
#define AT91_MATRIX_MASTERS	11
#define AT91_MATRIX_SLAVES	8
#else
#error CPU not supported. Please update at91_matrix.h
#endif

typedef struct at91_priority {
	u32	a;
	u32	b;
} at91_priority_t;

typedef struct at91_matrix {
	u32		mcfg[AT91_MATRIX_MASTERS];
#if defined(CONFIG_AT91SAM9261)
	u32		scfg[AT91_MATRIX_SLAVES];
	u32		res61_1[3];
	u32		tcr;
	u32		res61_2[2];
	u32		csa;
	u32		pucr;
	u32		res61_3[114];
#else
	u32		reserve1[16 - AT91_MATRIX_MASTERS];
	u32		scfg[AT91_MATRIX_SLAVES];
	u32		reserve2[16 - AT91_MATRIX_SLAVES];
	at91_priority_t	pr[AT91_MATRIX_SLAVES];
	u32		reserve3[32 - (2 * AT91_MATRIX_SLAVES)];
	u32		mrcr;		/* 0x100 Master Remap Control */
	u32		reserve4[3];
#if	defined(CONFIG_AT91SAM9G45)
	u32		ccr[52];	/* 0x110 - 0x1E0 Chip Configuration */
	u32		womr;		/* 0x1E4 Write Protect Mode  */
	u32		wpsr;		/* 0x1E8 Write Protect Status */
	u32		resg45_1[10];
#elif defined(CONFIG_AT91SAM9260)  || defined(CONFIG_AT91SAM9G20)
	u32		res60_1[3];
	u32		csa;
	u32		res60_2[56];
#elif defined(CONFIG_AT91SAM9263)
	u32		res63_1;
	u32		tcmr;
	u32		res63_2[2];
	u32		csa[2];
	u32		res63_3[54];
#else
	u32		reserve5[60];
#endif
#endif
} at91_matrix_t;

#endif /* __ASSEMBLY__ */

#define AT91_MATRIX_CSA_DBPUC		0x00000100
#define AT91_MATRIX_CSA_VDDIOMSEL_1_8V	0x00000000
#define AT91_MATRIX_CSA_VDDIOMSEL_3_3V	0x00010000

#define AT91_MATRIX_CSA_EBI_CS1A	0x00000002
#define AT91_MATRIX_CSA_EBI_CS3A	0x00000008
#define AT91_MATRIX_CSA_EBI_CS4A	0x00000010
#define AT91_MATRIX_CSA_EBI_CS5A	0x00000020

#define AT91_MATRIX_CSA_EBI1_CS2A	0x00000008

#endif

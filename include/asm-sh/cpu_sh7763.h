/*
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2007,2008 Nobuhiro Iwamatsu
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _ASM_CPU_SH7763_H_
#define _ASM_CPU_SH7763_H_

/* CACHE */
#define CACHE_OC_NUM_WAYS	1
#define CCR				0xFF00001C
#define CCR_CACHE_INIT	0x0000090b

/* SCIF */
/* SCIF0 */
#define SCIF0_BASE	SCSMR0
#define SCSMR0		0xFFE00000

/* SCIF1 */
#define SCIF1_BASE	SCSMR1
#define SCSMR1		0xFFE08000

/* SCIF2 */
#define SCIF2_BASE	SCSMR2
#define SCSMR2		0xFFE10000

/* Watchdog Timer */
#define WTCNT		WDTST
#define WDTST		0xFFCC0000

/* TMU */
#define TSTR		0xFFD80004
#define TCOR0		0xFFD80008
#define TCNT0		0xFFD8000C
#define TCR0		0xFFD80010

#endif /* _ASM_CPU_SH7763_H_ */

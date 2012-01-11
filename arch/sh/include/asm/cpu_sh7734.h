/*
 * (C) Copyright 2008, 2011 Renesas Solutions Corp.
 *
 * SH7734 Internal I/O register
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

#ifndef _ASM_CPU_SH7734_H_
#define _ASM_CPU_SH7734_H_

#define CCR 0xFF00001C

#define CACHE_OC_NUM_WAYS	4
#define CCR_CACHE_INIT	0x0000090d

/* SCIF */
#define SCIF0_BASE  0xFFE40000
#define SCIF1_BASE  0xFFE41000
#define SCIF2_BASE  0xFFE42000
#define SCIF3_BASE  0xFFE43000
#define SCIF4_BASE  0xFFE44000
#define SCIF5_BASE  0xFFE45000

/* Timer */
#define TSTR	0xFFD80004
#define TCNT0	0xFFD8000C
#define TCR0	0xFFD80010

#endif /* _ASM_CPU_SH7734_H_ */

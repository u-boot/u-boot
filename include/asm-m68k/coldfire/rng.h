/*
 * RNG Memory Map
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __RNG_H__
#define __RNG_H__

/* Random Number Generator */
typedef struct rng_ctrl {
	u32 cr;			/* 0x00 Control */
	u32 sr;			/* 0x04 Status */
	u32 er;			/* 0x08 Entropy */
	u32 out;		/* 0x0C Output FIFO */
} rng_t;

#define RNG_CR_SLM		(0x00000010)	/* Sleep mode - 5445x */
#define RNG_CR_CI		(0x00000008)	/* Clear interrupt */
#define RNG_CR_IM		(0x00000004)	/* Interrupt mask */
#define RNG_CR_HA		(0x00000002)	/* High assurance */
#define RNG_CR_GO		(0x00000001)	/* Go bit */

#define RNG_SR_OFS(x)		(((x) & 0x000000FF) << 16)
#define RNG_SR_OFS_MASK		(0xFF00FFFF)
#define RNG_SR_OFL(x)		(((x) & 0x000000FF) << 8)
#define RNG_SR_OFL_MASK		(0xFFFF00FF)
#define RNG_SR_EI		(0x00000008)
#define RNG_SR_FUF		(0x00000004)
#define RNG_SR_LRS		(0x00000002)
#define RNG_SR_SV		(0x00000001)

#endif				/* __RNG_H__ */

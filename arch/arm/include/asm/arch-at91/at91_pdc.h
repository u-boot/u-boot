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

#ifndef AT91_PDC_H
#define AT91_PDC_H

typedef struct at91_pdc {
	u32	rpr;		/* 0x100 Receive Pointer Register */
	u32	rcr;		/* 0x104 Receive Counter Register */
	u32	tpr;		/* 0x108 Transmit Pointer Register */
	u32	tcr;		/* 0x10C Transmit Counter Register */
	u32	pnpr;		/* 0x110 Receive Next Pointer Register */
	u32	pncr;		/* 0x114 Receive Next Counter Register */
	u32	tnpr;		/* 0x118 Transmit Next Pointer Register */
	u32	tncr;		/* 0x11C Transmit Next Counter Register */
	u32	ptcr;		/* 0x120 Transfer Control Register */
	u32	ptsr;		/* 0x124 Transfer Status Register */
} at91_pdc_t;

#endif

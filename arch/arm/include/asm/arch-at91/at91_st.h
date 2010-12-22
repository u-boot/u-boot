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

#ifndef AT91_ST_H
#define AT91_ST_H

typedef struct at91_st {

	u32	cr;
	u32	pimr;
	u32	wdmr;
	u32	rtmr;
	u32	sr;
	u32	ier;
	u32	idr;
	u32	imr;
	u32	rtar;
	u32	crtr;
} at91_st_t ;

#define AT91_ST_CR_WDRST	1

#define AT91_ST_WDMR_WDV(x)	(x & 0xFFFF)
#define AT91_ST_WDMR_RSTEN	0x00010000
#define AT91_ST_WDMR_EXTEN 	0x00020000

#endif

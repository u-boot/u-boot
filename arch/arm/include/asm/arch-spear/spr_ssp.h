/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
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

#ifndef _SPR_SSP_H
#define _SPR_SSP_H

struct ssp_regs {
	u32 sspcr0;
	u32 sspcr1;
	u32 sspdr;
	u32 sspsr;
	u32 sspcpsr;
	u32 sspimsc;
	u32 sspicr;
	u32 sspdmacr;
};

#define SSPCR0_FRF_MOT_SPI	0x0000
#define SSPCR0_DSS_16BITS	0x000f

#define SSPCR1_SSE		0x0002

#define SSPSR_TNF		0x2
#define SSPSR_TFE		0x1

#endif

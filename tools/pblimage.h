/*
 * Copyright 2012 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef PBLIMAGE_H
#define PBLIMAGE_H

#define RCW_BYTES	64
#define RCW_PREAMBLE	0xaa55aa55
#define RCW_HEADER	0x010e0100

struct pbl_header {
	uint32_t preamble;
	uint32_t rcwheader;
	uint8_t rcw_data[RCW_BYTES];
};

#endif /* PBLIMAGE_H */

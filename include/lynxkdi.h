/*
 * (C) Copyright 2003
 * Orbacom Systems, Inc.
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

#ifndef __LYNXKDI_H__
#define __LYNXKDI_H__


/* Boot parameter struct passed to kernel
 */
typedef struct lynxos_bootparms_t {
	uint8_t		rsvd1[2];	/* Reserved			*/
	uint8_t		ethaddr[6];	/* Ethernet address		*/
	uint16_t	flags;		/* Boot flags			*/
	uint32_t	rate;		/* System frequency		*/
	uint32_t	clock_ref;	/* Time reference		*/
	uint32_t	dramsz;		/* DRAM size			*/
	uint32_t	rsvd2;		/* Reserved			*/
} lynxos_bootparms_t;


#endif	/* __LYNXKDI_H__ */

/*
 * MCF5249 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
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

#ifndef __IMMAP_5249__
#define __IMMAP_5249__

#define MMAP_INTC		(CFG_MBAR + 0x00000040)
#define MMAP_DTMR0		(CFG_MBAR + 0x00000140)
#define MMAP_DTMR1		(CFG_MBAR + 0x00000180)
#define MMAP_UART0		(CFG_MBAR + 0x000001C0)
#define MMAP_UART1		(CFG_MBAR + 0x00000200)
#define MMAP_QSPI		(CFG_MBAR + 0x00000400)

#endif				/* __IMMAP_5249__ */

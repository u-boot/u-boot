/* asi.h:  Address Space Identifier values for the LEON2 sparc.
 *
 * Copyright (C) 2008 Daniel Hellstrom (daniel@gaisler.com)
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
 *
 */

#ifndef _LEON2_ASI_H
#define _LEON2_ASI_H

#define ASI_CACHEMISS		0x01	/* Force D-Cache miss on load (lda) */
#define ASI_M_FLUSH_PROBE	0x03	/* MMU Flush/Probe */
#define ASI_IFLUSH		0x05	/* Flush I-Cache */
#define ASI_DFLUSH		0x06	/* Flush D-Cache */
#define ASI_BYPASS		0x1c	/* Bypass MMU (Physical address) */
#define ASI_MMUFLUSH		0x18	/* FLUSH TLB */
#define ASI_M_MMUREGS		0x19	/* READ/Write MMU Registers */

#endif	/* _LEON2_ASI_H */

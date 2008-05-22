/*
 *    Copyright (c) 2008 Nuovation System Designs, LLC
 *	Grant Erickson <gerickson@nuovations.com>
 *
 *    Copyright (c) 2007 DENX Software Engineering, GmbH
 *	Stefan Roese <sr@denx.de>
 *
 *    See file CREDITS for list of people who contributed to this
 *    project.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will abe useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *	This file implements ECC initialization for PowerPC processors
 *	using the SDRAM DDR2 controller, including the 405EX(r),
 *	440SP(E), 460EX and 460GT.
 *
 */

#ifndef _ECC_H_
#define _ECC_H_

#if !defined(CFG_ECC_PATTERN)
#define	CFG_ECC_PATTERN	0x00000000
#endif /* !defined(CFG_ECC_PATTERN) */

extern void ecc_init(unsigned long * const start, unsigned long size);

#endif /* _ECC_H_ */

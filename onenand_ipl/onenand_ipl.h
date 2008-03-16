/*
 * (C) Copyright 2005-2008 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
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

#ifndef _ONENAND_IPL_H
#define _ONENAND_IPL_H

#include <linux/mtd/onenand_regs.h>

#define ONENAND_BLOCK_SIZE              2048

#ifndef CFG_PRINTF
#define printf(format, args...)
#endif

#define onenand_readw(a)        readw(a)
#define onenand_writew(v, a)    writew(v, a)

#define THIS_ONENAND(a)         (CFG_ONENAND_BASE + (a))

#define READ_INTERRUPT()                                                \
	onenand_readw(THIS_ONENAND(ONENAND_REG_INTERRUPT))

#define ONENAND_PAGE_SIZE                       2048

extern int onenand_read_block0(unsigned char *buf);
#endif

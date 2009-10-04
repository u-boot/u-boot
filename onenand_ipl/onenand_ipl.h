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

#define onenand_readw(a)        readw(THIS_ONENAND(a))
#define onenand_writew(v, a)    writew(v, THIS_ONENAND(a))

#define THIS_ONENAND(a)         (CONFIG_SYS_ONENAND_BASE + (a))

#define READ_INTERRUPT()	onenand_readw(ONENAND_REG_INTERRUPT)

extern int (*onenand_read_page)(ulong block, ulong page,
				u_char *buf, int pagesize);
extern int onenand_read_block(unsigned char *buf);
#endif

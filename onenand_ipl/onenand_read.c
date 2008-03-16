/*
 * (C) Copyright 2005-2008 Samsung Electronis
 * Kyungmin Park <kyungmin.park@samsung.com>
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

#include <common.h>

#include <asm/io.h>
#include <asm/string.h>

#include "onenand_ipl.h"

#define onenand_block_address(block)		(block)
#define onenand_sector_address(page)		(page << 2)
#define onenand_buffer_address()		((1 << 3) << 8)
#define onenand_bufferram_address(block)	(0)

#ifdef __HAVE_ARCH_MEMCPY32
extern void *memcpy32(void *dest, void *src, int size);
#endif

/* read a page with ECC */
static inline int onenand_read_page(ulong block, ulong page,
				u_char * buf, int pagesize)
{
	unsigned long *base;

#ifndef __HAVE_ARCH_MEMCPY32
	unsigned int offset, value;
	unsigned long *p;
#endif

	onenand_writew(onenand_block_address(block),
		THIS_ONENAND(ONENAND_REG_START_ADDRESS1));

	onenand_writew(onenand_bufferram_address(block),
		THIS_ONENAND(ONENAND_REG_START_ADDRESS2));

	onenand_writew(onenand_sector_address(page),
		THIS_ONENAND(ONENAND_REG_START_ADDRESS8));

	onenand_writew(onenand_buffer_address(),
		THIS_ONENAND(ONENAND_REG_START_BUFFER));

	onenand_writew(ONENAND_INT_CLEAR, THIS_ONENAND(ONENAND_REG_INTERRUPT));

	onenand_writew(ONENAND_CMD_READ, THIS_ONENAND(ONENAND_REG_COMMAND));

#ifndef __HAVE_ARCH_MEMCPY32
	p = (unsigned long *) buf;
#endif
	base = (unsigned long *) (CFG_ONENAND_BASE + ONENAND_DATARAM);

	while (!(READ_INTERRUPT() & ONENAND_INT_READ))
		continue;

#ifdef __HAVE_ARCH_MEMCPY32
	/* 32 bytes boundary memory copy */
	memcpy32(buf, base, pagesize);
#else
	for (offset = 0; offset < (pagesize >> 2); offset++) {
		value = *(base + offset);
		*p++ = value;
	}
#endif

	return 0;
}

#define ONENAND_START_PAGE		1
#define ONENAND_PAGES_PER_BLOCK		64

/**
 * onenand_read_block - Read a block data to buf
 * @return 0 on success
 */
int onenand_read_block0(unsigned char *buf)
{
	int page, offset = 0;
	int pagesize = ONENAND_PAGE_SIZE;

	/* MLC OneNAND has 4KiB page size */
	if (onenand_readw(THIS_ONENAND(ONENAND_REG_TECHNOLOGY)))
		pagesize <<= 1;

	/* NOTE: you must read page from page 1 of block 0 */
	/* read the block page by page*/
	for (page = ONENAND_START_PAGE;
	    page < ONENAND_PAGES_PER_BLOCK; page++) {

		onenand_read_page(0, page, buf + offset, pagesize);
		offset += pagesize;
	}

	return 0;
}

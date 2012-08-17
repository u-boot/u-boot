/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on code:
 *	Copyright (C) 2005-2009 Samsung Electronics
 *	Kyungmin Park <kyungmin.park@samsung.com>
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
#include <linux/mtd/onenand_regs.h>
#include <onenand_uboot.h>

/*
 * Device geometry:
 * - 2048b page, 128k erase block.
 * - 4096b page, 256k erase block.
 */
enum onenand_spl_pagesize {
	PAGE_2K = 2048,
	PAGE_4K = 4096,
};

#define ONENAND_PAGES_PER_BLOCK			64
#define onenand_block_address(block)		(block)
#define onenand_sector_address(page)		(page << 2)
#define onenand_buffer_address()		((1 << 3) << 8)
#define onenand_bufferram_address(block)	(0)

static inline uint16_t onenand_readw(uint32_t addr)
{
	return readw(CONFIG_SYS_ONENAND_BASE + addr);
}

static inline void onenand_writew(uint16_t value, uint32_t addr)
{
	writew(value, CONFIG_SYS_ONENAND_BASE + addr);
}

static enum onenand_spl_pagesize onenand_spl_get_geometry(void)
{
	uint32_t dev_id, density;

	if (!onenand_readw(ONENAND_REG_TECHNOLOGY)) {
		dev_id = onenand_readw(ONENAND_REG_DEVICE_ID);
		density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;
		density &= ONENAND_DEVICE_DENSITY_MASK;

		if (density < ONENAND_DEVICE_DENSITY_4Gb)
			return PAGE_2K;

		if (dev_id & ONENAND_DEVICE_IS_DDP)
			return PAGE_2K;
	}

	return PAGE_4K;
}

static int onenand_spl_read_page(uint32_t block, uint32_t page, uint32_t *buf,
					enum onenand_spl_pagesize pagesize)
{
	const uint32_t addr = CONFIG_SYS_ONENAND_BASE + ONENAND_DATARAM;
	uint32_t offset;

	onenand_writew(onenand_block_address(block),
			ONENAND_REG_START_ADDRESS1);

	onenand_writew(onenand_bufferram_address(block),
			ONENAND_REG_START_ADDRESS2);

	onenand_writew(onenand_sector_address(page),
			ONENAND_REG_START_ADDRESS8);

	onenand_writew(onenand_buffer_address(),
			ONENAND_REG_START_BUFFER);

	onenand_writew(ONENAND_INT_CLEAR, ONENAND_REG_INTERRUPT);

	onenand_writew(ONENAND_CMD_READ, ONENAND_REG_COMMAND);

	while (!(onenand_readw(ONENAND_REG_INTERRUPT) & ONENAND_INT_READ))
		continue;

	/* Check for invalid block mark */
	if (page < 2 && (onenand_readw(ONENAND_SPARERAM) != 0xffff))
		return 1;

	for (offset = 0; offset < pagesize; offset += 4)
		buf[offset / 4] = readl(addr + offset);

	return 0;
}

void onenand_spl_load_image(uint32_t offs, uint32_t size, void *dst)
{
	uint32_t *addr = (uint32_t *)dst;
	uint32_t total_pages;
	uint32_t block;
	uint32_t page, rpage;
	enum onenand_spl_pagesize pagesize;
	int ret;

	pagesize = onenand_spl_get_geometry();

	/*
	 * The page can be either 2k or 4k, avoid using DIV_ROUND_UP to avoid
	 * pulling further unwanted functions into the SPL.
	 */
	if (pagesize == 2048) {
		total_pages = DIV_ROUND_UP(size, 2048);
		page = offs / 2048;
	} else {
		total_pages = DIV_ROUND_UP(size, 4096);
		page = offs / 4096;
	}

	for (; page <= total_pages; page++) {
		block = page / ONENAND_PAGES_PER_BLOCK;
		rpage = page & (ONENAND_PAGES_PER_BLOCK - 1);
		ret = onenand_spl_read_page(block, rpage, addr, pagesize);
		if (ret) {
			total_pages += ONENAND_PAGES_PER_BLOCK;
			page += ONENAND_PAGES_PER_BLOCK - 1;
		} else {
			addr += pagesize / 4;
		}
	}
}

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Altera Corporation (C) 2026. All rights reserved.
 */

#include <dm.h>
#include <hang.h>
#include <malloc.h>
#include <memalign.h>
#include <nand.h>

struct mtd_info *nand_get_mtd(void)
{
	struct mtd_info *mtd;

	mtd = get_nand_dev_by_index(nand_curr_device);
	if (!mtd)
		hang();

	return mtd;
}

int nand_spl_load_image(u32 offset, u32 len, void *dst)
{
	size_t count = len, actual = 0, page_align_overhead = 0;
	u32 page_align_offset = 0;
	u8 *page_buffer = NULL;
	int err = 0;
	struct mtd_info *mtd;

	if (!len || !dst)
		return -EINVAL;

	mtd = nand_get_mtd();

	if (offset & (mtd->writesize - 1)) {
		page_buffer = malloc_cache_aligned(mtd->writesize);
		if (!page_buffer) {
			debug("Error: allocating buffer\n");
			return -ENOMEM;
		}

		page_align_overhead = offset % mtd->writesize;
		page_align_offset = offset - page_align_overhead;
		count = mtd->writesize;

		err = nand_read_skip_bad(mtd, page_align_offset, &count,
					 &actual, mtd->size, page_buffer);
		if (err)
			goto out;

		count -= page_align_overhead;
		count = min((size_t)len, count);

		memcpy(dst, page_buffer + page_align_overhead, count);

		len -= count;
		if (!len)
			goto out;

		offset += count;
		dst += count;
		count = len;
	}

	err = nand_read_skip_bad(mtd, offset, &count,
				 &actual, mtd->size, dst);

out:
	if (page_buffer)
		free(page_buffer);

	return err;
}

void nand_deselect(void) {}

// SPDX-License-Identifier:     GPL-2.0
/*
 * Copyright (C) 2024 Intel Corporation <www.intel.com>
 */

#include <cadence-nand.h>
#include <dm.h>
#include <hang.h>
#include <nand.h>
#include <system-constants.h>

/* Unselect after operation */
void nand_deselect(void)
{
	struct mtd_info *mtd;
	struct nand_chip *chip;

	mtd = get_nand_dev_by_index(nand_curr_device);
	if (!mtd)
		hang();
	chip = mtd_to_nand(mtd);

	if (chip->select_chip)
		chip->select_chip(mtd, -1);
}

static int nand_is_bad_block(int block)
{
	struct mtd_info *mtd;
	struct nand_chip *chip;
	loff_t ofs = block * CONFIG_SYS_NAND_BLOCK_SIZE;

	mtd = get_nand_dev_by_index(nand_curr_device);
	if (!mtd)
		hang();
	chip = mtd_to_nand(mtd);

	return chip->block_bad(mtd, ofs);
}

static int nand_read_page(int block, int page, uchar *dst)
{
	struct mtd_info *mtd;
	int page_addr = block * SYS_NAND_BLOCK_PAGES + page;
	loff_t ofs = page_addr * CONFIG_SYS_NAND_PAGE_SIZE;
	int ret;
	size_t len = CONFIG_SYS_NAND_PAGE_SIZE;

	mtd = get_nand_dev_by_index(nand_curr_device);
	if (!mtd)
		hang();

	ret = nand_read(mtd, ofs, &len, dst);
	if (ret)
		printf("nand_read failed %d\n", ret);

	return ret;
}
#include "nand_spl_loaders.c"

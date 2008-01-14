/*
 *  drivers/mtd/onenand/onenand_uboot.c
 *
 *  Copyright (C) 2005-2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * OneNAND initialization at U-Boot
 */

#include <common.h>

#ifdef CONFIG_CMD_ONENAND

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

struct mtd_info onenand_mtd;
struct onenand_chip onenand_chip;

void onenand_init(void)
{
	memset(&onenand_mtd, 0, sizeof(struct mtd_info));
	memset(&onenand_chip, 0, sizeof(struct onenand_chip));

	onenand_chip.base = (void *) CFG_ONENAND_BASE;
	onenand_mtd.priv = &onenand_chip;

	onenand_scan(&onenand_mtd, 1);

	puts("OneNAND: ");
	print_size(onenand_mtd.size, "\n");
}

#endif	/* CONFIG_CMD_ONENAND */

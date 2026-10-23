// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2026 BayLibre SAS */

#include <mmc.h>
#include <spl.h>
#include <stdio.h>
#include <linux/errno.h>

#include "mtk_ab.h"

int spl_mmc_boot_partition(struct mmc *mmc, const u32 boot_device)
{
	struct blk_desc *desc = mmc_get_blk_desc(mmc);
	unsigned int slot;
	int ret;

	if (IS_SD(mmc)) {
		ret = -ENODEV;
		goto error;
	}

	ret = mtk_ab_select(desc, &slot);
	if (ret < 0)
		goto error;

	printf("MediaTek A/B: slot %c, partition %d\n", 'A' + slot, ret);
	return ret;

error:
	printf("MediaTek A/B: selection failed (%d)\n", ret);
	return ret;
}

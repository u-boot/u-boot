// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <fwu.h>
#include <fwu_mdata.h>

__weak int fwu_plat_get_update_index(u32 *update_idx)
{
	int ret;
	u32 active_idx;

	ret = fwu_get_active_index(&active_idx);

	if (ret < 0)
		return -1;

	*update_idx = active_idx ^= 0x1;

	return ret;
}

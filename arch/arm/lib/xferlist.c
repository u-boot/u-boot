// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include <linux/types.h>
#include <errno.h>
#include <bloblist.h>
#include "xferlist.h"

int xferlist_from_boot_arg(ulong *addr)
{
	int ret;

	ret = bloblist_check_reg_conv(saved_args[0], saved_args[2],
				      saved_args[1], saved_args[3]);
	if (ret)
		return ret;

	*addr = bloblist_get_base();

	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Helpers shared by the bootmeths that boot via the pxelinux parser
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <bootflow.h>
#include <bootmeth.h>
#include <extlinux.h>
#include <pxe_utils.h>
#include <vsprintf.h>

int extlinux_getfile(struct pxe_context *ctx, const char *file_path,
		     char *file_addr, enum bootflow_img_t type, ulong *sizep)
{
	struct extlinux_info *info = ctx->userdata;
	ulong addr;
	int ret;

	addr = simple_strtoul(file_addr, NULL, 16);

	/* Allow up to 1GB */
	*sizep = 1 << 30;
	ret = bootmeth_read_file(info->dev, info->bflow, file_path, addr,
				 type, sizep);
	if (ret)
		return log_msg_ret("read", ret);

	return 0;
}

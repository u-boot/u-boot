// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <mapmem.h>

int board_fit_config_name_match(const char *name)
{
	return 0;
}

struct legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return map_sysmem(0x100000, 0);
}

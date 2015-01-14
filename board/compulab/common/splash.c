/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <nand.h>
#include <errno.h>
#include <bmp_layout.h>
#include "common.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_NAND
static int splash_nand_read(u32 bmp_load_addr, int offset, size_t read_size)
{
	return nand_read_skip_bad(&nand_info[nand_curr_device], offset,
				  &read_size, NULL,
				  nand_info[nand_curr_device].size,
				  (u_char *)bmp_load_addr);
}
#else
static int splash_nand_read(u32 bmp_load_addr, int offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	return -ENOSYS;
}
#endif

static int splash_storage_read(u32 bmp_load_addr, int offset, size_t read_size)
{
	return splash_nand_read(bmp_load_addr, offset, read_size);
}

static int splash_load_raw(u32 bmp_load_addr, int offset)
{
	struct bmp_header *bmp_hdr;
	int res;
	size_t bmp_size, bmp_header_size = sizeof(struct bmp_header);

	if (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	res = splash_storage_read(bmp_load_addr, offset, bmp_header_size);
	if (res < 0)
		return res;

	bmp_hdr = (struct bmp_header *)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	if (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	return splash_storage_read(bmp_load_addr, offset, bmp_size);

splash_address_too_high:
	printf("Error: splashimage address too high. Data overwrites U-Boot "
		"and/or placed beyond DRAM boundaries.\n");

	return -EFAULT;
}

int cl_splash_screen_prepare(int offset)
{
	char *env_splashimage_value;
	u32 bmp_load_addr;

	env_splashimage_value = getenv("splashimage");
	if (env_splashimage_value == NULL)
		return -ENOENT;

	bmp_load_addr = simple_strtoul(env_splashimage_value, 0, 16);
	if (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		return -EFAULT;
	}

	return splash_load_raw(bmp_load_addr, offset);
}

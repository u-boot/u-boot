/*
 * Copyright (c) 2016 Google, Inc
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <os.h>
#include <spl.h>
#include <asm/spl.h>
#include <asm/state.h>

DECLARE_GLOBAL_DATA_PTR;

void board_init_f(ulong flag)
{
	struct sandbox_state *state = state_get_current();

	gd->arch.ram_buf = state->ram_buf;
	gd->ram_size = state->ram_size;
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOARD;
}

static int spl_board_load_image(struct spl_image_info *spl_image,
				struct spl_boot_device *bootdev)
{
	char fname[256];
	int ret;

	ret = os_find_u_boot(fname, sizeof(fname));
	if (ret) {
		printf("(%s not found, error %d)\n", fname, ret);
		return ret;
	}

	/* Hopefully this will not return */
	return os_spl_to_uboot(fname);
}
SPL_LOAD_IMAGE_METHOD("sandbox", 0, BOOT_DEVICE_BOARD, spl_board_load_image);

void spl_board_init(void)
{
	preloader_console_init();
}

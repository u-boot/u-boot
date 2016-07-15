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

void spl_board_announce_boot_device(void)
{
	char fname[256];
	int ret;

	ret = os_find_u_boot(fname, sizeof(fname));
	if (ret) {
		printf("(%s not found, error %d)\n", fname, ret);
		return;
	}
	printf("%s\n", fname);
}

int spl_board_load_image(void)
{
	char fname[256];
	int ret;

	ret = os_find_u_boot(fname, sizeof(fname));
	if (ret)
		return ret;

	/* Hopefully this will not return */
	return os_spl_to_uboot(fname);
}

void spl_board_init(void)
{
	struct udevice *dev;

	preloader_console_init();

	/*
	* Scan all the devices so that we can output their platform data. See
	* sandbox_spl_probe().
	*/
	for (uclass_first_device(UCLASS_MISC, &dev);
	dev;
	uclass_next_device(&dev))
		;
}

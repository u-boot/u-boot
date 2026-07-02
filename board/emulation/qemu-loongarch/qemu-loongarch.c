// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <dm.h>
#include <dm/ofnode.h>
#include <env.h>
#include <fdtdec.h>
#include <image.h>
#include <linux/sizes.h>
#include <lmb.h>
#include <log.h>
#include <spl.h>
#include <init.h>
#include <usb.h>
#include <virtio_types.h>
#include <virtio.h>

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_MTD_NOR_FLASH)
int is_flash_available(void)
{
	if (!ofnode_equal(ofnode_by_compatible(ofnode_null(), "cfi-flash"),
			  ofnode_null()))
		return 1;

	return 0;
}
#endif

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x10000000)
		return 0x10000000;

	return gd->ram_top;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	/* start usb so that usb keyboard can be used as input device */
	if (CONFIG_IS_ENABLED(USB_KEYBOARD))
		usb_init();

	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers
	 */
	virtio_init();

	return 0;
}

int board_fdt_blob_setup(void **fdtp)
{
	/* Stored the DTB address there during our init */
	*fdtp = (void *)(ulong)0x100000;
	return 0;
}

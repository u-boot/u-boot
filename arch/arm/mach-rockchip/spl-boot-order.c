/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <spl.h>

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int spl_node_to_boot_device(int node)
{
	struct udevice *parent;

	/*
	 * This should eventually move into the SPL code, once SPL becomes
	 * aware of the block-device layer.  Until then (and to avoid unneeded
	 * delays in getting this feature out, it lives at the board-level).
	 */
	if (!uclass_get_device_by_of_offset(UCLASS_MMC, node, &parent)) {
		struct udevice *dev;
		struct blk_desc *desc = NULL;

		for (device_find_first_child(parent, &dev);
		     dev;
		     device_find_next_child(&dev)) {
			if (device_get_uclass_id(dev) == UCLASS_BLK) {
				desc = dev_get_uclass_platdata(dev);
				break;
			}
		}

		if (!desc)
			return -ENOENT;

		switch (desc->devnum) {
		case 0:
			return BOOT_DEVICE_MMC1;
		case 1:
			return BOOT_DEVICE_MMC2;
		default:
			return -ENOSYS;
		}
	}

	/*
	 * SPL doesn't differentiate SPI flashes, so we keep the detection
	 * brief and inaccurate... hopefully, the common SPL layer can be
	 * extended with awareness of the BLK layer (and matching OF_CONTROL)
	 * soon.
	 */
	if (!uclass_get_device_by_of_offset(UCLASS_SPI_FLASH, node, &parent))
		return BOOT_DEVICE_SPI;

	return -1;
}

void board_boot_order(u32 *spl_boot_list)
{
	const void *blob = gd->fdt_blob;
	int chosen_node = fdt_path_offset(blob, "/chosen");
	int idx = 0;
	int elem;
	int boot_device;
	int node;
	const char *conf;

	if (chosen_node < 0) {
		debug("%s: /chosen not found, using spl_boot_device()\n",
		      __func__);
		spl_boot_list[0] = spl_boot_device();
		return;
	}

	for (elem = 0;
	     (conf = fdt_stringlist_get(blob, chosen_node,
					"u-boot,spl-boot-order", elem, NULL));
	     elem++) {
		/* First check if the list element is an alias */
		const char *alias = fdt_get_alias(blob, conf);
		if (alias)
			conf = alias;

		/* Try to resolve the config item (or alias) as a path */
		node = fdt_path_offset(blob, conf);
		if (node < 0) {
			debug("%s: could not find %s in FDT", __func__, conf);
			continue;
		}

		/* Try to map this back onto SPL boot devices */
		boot_device = spl_node_to_boot_device(node);
		if (boot_device < 0) {
			debug("%s: could not map node @%x to a boot-device\n",
			      __func__, node);
			continue;
		}

		spl_boot_list[idx++] = boot_device;
	}

	/* If we had no matches, fall back to spl_boot_device */
	if (idx == 0)
		spl_boot_list[0] = spl_boot_device();
}
#endif

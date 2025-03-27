// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2020 Intel Corporation. All rights reserved
 *  Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <hang.h>
#include <spl.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

/* This function is to map specified node onto SPL boot devices */
static int spl_node_to_boot_device(int node)
{
	const void *blob = gd->fdt_blob;
	struct udevice *parent;
	const char *prop;

	if (!uclass_get_device_by_of_offset(UCLASS_MMC, node, &parent))
		return BOOT_DEVICE_MMC1;
	else if (!uclass_get_device_by_of_offset(UCLASS_SPI_FLASH, node, &parent))
		return BOOT_DEVICE_SPI;
	else if (!uclass_get_device_by_of_offset(UCLASS_MTD, node, &parent))
		return BOOT_DEVICE_NAND;

	prop = fdt_getprop(blob, node, "device_type", NULL);
	if (prop) {
		if (!strcmp(prop, "memory"))
			return BOOT_DEVICE_RAM;

		printf("%s: unknown device_type %s\n", __func__, prop);
	}

	return -ENODEV;
}

static void default_spl_boot_list(u32 *spl_boot_list, int length)
{
	spl_boot_list[0] = spl_boot_device();

	if (length > 1)
		spl_boot_list[1] = BOOT_DEVICE_SPI;

	if (length > 2)
		spl_boot_list[2] = BOOT_DEVICE_NAND;
}

void board_boot_order(u32 *spl_boot_list)
{
	int idx = 0;
	const void *blob = gd->fdt_blob;
	int chosen_node = fdt_path_offset(blob, "/chosen");
	const char *conf;
	int elem;
	int boot_device;
	int node;
	int length;

	/* expect valid initialized spl_boot_list */
	if (!spl_boot_list)
		return;

	length = 1;
	while (spl_boot_list[length] == spl_boot_list[length - 1])
		length++;

	debug("%s: chosen_node is %d\n", __func__, chosen_node);
	if (chosen_node < 0) {
		printf("%s: /chosen not found, using default\n", __func__);
		default_spl_boot_list(spl_boot_list, length);
		return;
	}

	for (elem = 0;
	    (conf = fdt_stringlist_get(blob, chosen_node,
			"u-boot,spl-boot-order", elem, NULL));
	    elem++) {
		if (idx >= length) {
			printf("%s: limit %d to spl_boot_list exceeded\n", __func__,
			       length);
			break;
		}

		/* Resolve conf item as a path in device tree */
		node = fdt_path_offset(blob, conf);
		if (node < 0) {
			debug("%s: could not find %s in FDT\n", __func__, conf);
			continue;
		}

		/* Try to map spl node back onto SPL boot devices */
		boot_device = spl_node_to_boot_device(node);
		if (boot_device < 0) {
			debug("%s: could not map node @%x to a boot-device\n",
			      __func__, node);
			continue;
		}

		spl_boot_list[idx] = boot_device;
		debug("%s: spl_boot_list[%d] = %u\n", __func__, idx,
		      spl_boot_list[idx]);
		idx++;
	}

	if (idx == 0) {
		if (!conf && !elem) {
			printf("%s: spl-boot-order invalid, using default\n", __func__);
			default_spl_boot_list(spl_boot_list, length);
		} else {
			printf("%s: no valid element spl-boot-order list\n", __func__);
		}
	}
}

#if IS_ENABLED(CONFIG_SPL_MMC)
u32 spl_boot_mode(const u32 boot_device)
{
	if (IS_ENABLED(CONFIG_SPL_FS_FAT) || IS_ENABLED(CONFIG_SPL_FS_EXT4))
		return MMCSD_MODE_FS;
	else
		return MMCSD_MODE_RAW;
}
#endif

/* board specific function prior loading SSBL / U-Boot */
void spl_perform_fixups(struct spl_image_info *spl_image)
{
	int ret;
	struct udevice *dev;

	ret = uclass_get_device_by_name(UCLASS_NOP, "socfpga-smmu-secure-config", &dev);
	if (ret) {
		printf("HPS SMMU secure settings init failed: %d\n", ret);
		hang();
	}
}

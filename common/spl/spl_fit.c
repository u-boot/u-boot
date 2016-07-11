/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <image.h>
#include <libfdt.h>
#include <spl.h>

static ulong fdt_getprop_u32(const void *fdt, int node, const char *prop)
{
	const u32 *cell;
	int len;

	cell = fdt_getprop(fdt, node, prop, &len);
	if (len != sizeof(*cell))
		return -1U;
	return fdt32_to_cpu(*cell);
}

static int spl_fit_select_fdt(const void *fdt, int images, int *fdt_offsetp)
{
	const char *name, *fdt_name;
	int conf, node, fdt_node;
	int len;

	*fdt_offsetp = 0;
	conf = fdt_path_offset(fdt, FIT_CONFS_PATH);
	if (conf < 0) {
		debug("%s: Cannot find /configurations node: %d\n", __func__,
		      conf);
		return -EINVAL;
	}
	for (node = fdt_first_subnode(fdt, conf);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		name = fdt_getprop(fdt, node, "description", &len);
		if (!name)
			return -EINVAL;
		if (board_fit_config_name_match(name))
			continue;

		debug("Selecting config '%s'", name);
		fdt_name = fdt_getprop(fdt, node, FIT_FDT_PROP, &len);
		if (!fdt_name) {
			debug("%s: Cannot find fdt name property: %d\n",
			      __func__, len);
			return -EINVAL;
		}

		debug(", fdt '%s'\n", fdt_name);
		fdt_node = fdt_subnode_offset(fdt, images, fdt_name);
		if (fdt_node < 0) {
			debug("%s: Cannot find fdt node '%s': %d\n",
			      __func__, fdt_name, fdt_node);
			return -EINVAL;
		}

		*fdt_offsetp = fdt_getprop_u32(fdt, fdt_node, "data-offset");
		len = fdt_getprop_u32(fdt, fdt_node, "data-size");
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("FIT: Selected '%s'\n", name);
#endif

		return len;
	}

#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	printf("No matching DT out of these options:\n");
	for (node = fdt_first_subnode(fdt, conf);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		name = fdt_getprop(fdt, node, "name", &len);
		printf("   %s\n", name);
	}
#endif

	return -ENOENT;
}

int spl_load_simple_fit(struct spl_load_info *info, ulong sector, void *fit)
{
	int sectors;
	ulong size, load;
	unsigned long count;
	int node, images;
	void *load_ptr;
	int fdt_offset, fdt_len;
	int data_offset, data_size;
	int base_offset;
	int src_sector;
	void *dst;

	/*
	 * Figure out where the external images start. This is the base for the
	 * data-offset properties in each image.
	 */
	size = fdt_totalsize(fit);
	size = (size + 3) & ~3;
	base_offset = (size + 3) & ~3;

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block, placing it so it finishes before
	 * where we will load the image.
	 *
	 * Note that we will load the image such that its first byte will be
	 * at the load address. Since that byte may be part-way through a
	 * block, we may load the image up to one block before the load
	 * address. So take account of that here by subtracting an addition
	 * block length from the FIT start position.
	 *
	 * In fact the FIT has its own load address, but we assume it cannot
	 * be before CONFIG_SYS_TEXT_BASE.
	 */
	fit = (void *)(CONFIG_SYS_TEXT_BASE - size - info->bl_len);
	sectors = (size + info->bl_len - 1) / info->bl_len;
	count = info->read(info, sector, sectors, fit);
	debug("fit read sector %lx, sectors=%d, dst=%p, count=%lu\n",
	      sector, sectors, fit, count);
	if (count == 0)
		return -EIO;

	/* find the firmware image to load */
	images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__, images);
		return -1;
	}
	node = fdt_first_subnode(fit, images);
	if (node < 0) {
		debug("%s: Cannot find first image node: %d\n", __func__, node);
		return -1;
	}

	/* Get its information and set up the spl_image structure */
	data_offset = fdt_getprop_u32(fit, node, "data-offset");
	data_size = fdt_getprop_u32(fit, node, "data-size");
	load = fdt_getprop_u32(fit, node, "load");
	debug("data_offset=%x, data_size=%x\n", data_offset, data_size);
	spl_image.load_addr = load;
	spl_image.entry_point = load;
	spl_image.os = IH_OS_U_BOOT;

	/*
	 * Work out where to place the image. We read it so that the first
	 * byte will be at 'load'. This may mean we need to load it starting
	 * before then, since we can only read whole blocks.
	 */
	sectors = (data_size + info->bl_len - 1) / info->bl_len;
	data_offset += base_offset;
	load_ptr = (void *)load;
	debug("U-Boot size %x, data %p\n", data_size, load_ptr);
	dst = load_ptr - (data_offset % info->bl_len);

	/* Read the image */
	src_sector = sector + data_offset / info->bl_len;
	debug("image: data_offset=%x, dst=%p, src_sector=%x, sectors=%x\n",
	      data_offset, dst, src_sector, sectors);
	count = info->read(info, src_sector, sectors, dst);
	if (count != sectors)
		return -EIO;

	/* Figure out which device tree the board wants to use */
	fdt_len = spl_fit_select_fdt(fit, images, &fdt_offset);
	if (fdt_len < 0)
		return fdt_len;

	/*
	 * Read the device tree and place it after the image. There may be
	 * some extra data before it since we can only read entire blocks.
	 */
	dst = load_ptr + data_size;
	fdt_offset += base_offset;
	count = info->read(info, sector + fdt_offset / info->bl_len, sectors,
			   dst);
	debug("fit read %x sectors to %x, dst %p, data_offset %x\n",
	      sectors, spl_image.load_addr, dst, fdt_offset);
	if (count != sectors)
		return -EIO;

	/*
	 * Copy the device tree so that it starts immediately after the image.
	 * After this we will have the U-Boot image and its device tree ready
	 * for us to start.
	 */
	memcpy(dst, dst + fdt_offset % info->bl_len, fdt_len);

	return 0;
}

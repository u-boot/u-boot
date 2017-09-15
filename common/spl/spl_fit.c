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

#ifndef CONFIG_SYS_BOOTM_LEN
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)
#endif

/**
 * spl_fit_get_image_node(): By using the matching configuration subnode,
 * retrieve the name of an image, specified by a property name and an index
 * into that.
 * @fit:	Pointer to the FDT blob.
 * @images:	Offset of the /images subnode.
 * @type:	Name of the property within the configuration subnode.
 * @index:	Index into the list of strings in this property.
 *
 * Return:	the node offset of the respective image node or a negative
 * 		error number.
 */
static int spl_fit_get_image_node(const void *fit, int images,
				  const char *type, int index)
{
	const char *name, *str;
	int node, conf_node;
	int len, i;

	conf_node = fit_find_config_node(fit);
	if (conf_node < 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("No matching DT out of these options:\n");
		for (node = fdt_first_subnode(fit, conf_node);
		     node >= 0;
		     node = fdt_next_subnode(fit, node)) {
			name = fdt_getprop(fit, node, "description", &len);
			printf("   %s\n", name);
		}
#endif
		return conf_node;
	}

	name = fdt_getprop(fit, conf_node, type, &len);
	if (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		return -EINVAL;
	}

	str = name;
	for (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		if (!str || (str - name >= len)) {
			debug("no string for index %d\n", index);
			return -E2BIG;
		}
	}

	debug("%s: '%s'\n", type, str);
	node = fdt_subnode_offset(fit, images, str);
	if (node < 0) {
		debug("cannot find image node '%s': %d\n", str, node);
		return -EINVAL;
	}

	return node;
}

static int get_aligned_image_offset(struct spl_load_info *info, int offset)
{
	/*
	 * If it is a FS read, get the first address before offset which is
	 * aligned to ARCH_DMA_MINALIGN. If it is raw read return the
	 * block number to which offset belongs.
	 */
	if (info->filename)
		return offset & ~(ARCH_DMA_MINALIGN - 1);

	return offset / info->bl_len;
}

static int get_aligned_image_overhead(struct spl_load_info *info, int offset)
{
	/*
	 * If it is a FS read, get the difference between the offset and
	 * the first address before offset which is aligned to
	 * ARCH_DMA_MINALIGN. If it is raw read return the offset within the
	 * block.
	 */
	if (info->filename)
		return offset & (ARCH_DMA_MINALIGN - 1);

	return offset % info->bl_len;
}

static int get_aligned_image_size(struct spl_load_info *info, int data_size,
				  int offset)
{
	data_size = data_size + get_aligned_image_overhead(info, offset);

	if (info->filename)
		return data_size;

	return (data_size + info->bl_len - 1) / info->bl_len;
}

/**
 * spl_load_fit_image(): load the image described in a certain FIT node
 * @info:	points to information about the device to load data from
 * @sector:	the start sector of the FIT image on the device
 * @fit:	points to the flattened device tree blob describing the FIT
 * 		image
 * @base_offset: the beginning of the data area containing the actual
 *		image data, relative to the beginning of the FIT
 * @node:	offset of the DT node describing the image to load (relative
 * 		to @fit)
 * @image_info:	will be filled with information about the loaded image
 * 		If the FIT node does not contain a "load" (address) property,
 * 		the image gets loaded to the address pointed to by the
 * 		load_addr member in this struct.
 *
 * Return:	0 on success or a negative error number.
 */
static int spl_load_fit_image(struct spl_load_info *info, ulong sector,
			      void *fit, ulong base_offset, int node,
			      struct spl_image_info *image_info)
{
	int offset;
	size_t length;
	int len;
	ulong size;
	ulong load_addr, load_ptr;
	void *src;
	ulong overhead;
	int nr_sectors;
	int align_len = ARCH_DMA_MINALIGN - 1;
	uint8_t image_comp = -1, type = -1;
	const void *data;

	if (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_GZIP)) {
		if (fit_image_get_comp(fit, node, &image_comp))
			puts("Cannot get image compression format.\n");
		else
			debug("%s ", genimg_get_comp_name(image_comp));

		if (fit_image_get_type(fit, node, &type))
			puts("Cannot get image type.\n");
		else
			debug("%s ", genimg_get_type_name(type));
	}

	if (fit_image_get_load(fit, node, &load_addr))
		load_addr = image_info->load_addr;

	if (!fit_image_get_data_offset(fit, node, &offset)) {
		/* External data */
		offset += base_offset;
		if (fit_image_get_data_size(fit, node, &len))
			return -ENOENT;

		load_ptr = (load_addr + align_len) & ~align_len;
		length = len;

		overhead = get_aligned_image_overhead(info, offset);
		nr_sectors = get_aligned_image_size(info, length, offset);

		if (info->read(info,
			       sector + get_aligned_image_offset(info, offset),
			       nr_sectors, (void *)load_ptr) != nr_sectors)
			return -EIO;

		debug("External data: dst=%lx, offset=%x, size=%lx\n",
		      load_ptr, offset, (unsigned long)length);
		src = (void *)load_ptr + overhead;
	} else {
		/* Embedded data */
		if (fit_image_get_data(fit, node, &data, &length)) {
			puts("Cannot get image data/size\n");
			return -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (void *)data;
	}

#ifdef CONFIG_SPL_FIT_IMAGE_POST_PROCESS
	board_fit_image_post_process(&src, &length);
#endif

	if (IS_ENABLED(CONFIG_SPL_OS_BOOT)	&&
	    IS_ENABLED(CONFIG_SPL_GZIP)		&&
	    image_comp == IH_COMP_GZIP		&&
	    type == IH_TYPE_KERNEL) {
		size = length;
		if (gunzip((void *)load_addr, CONFIG_SYS_BOOTM_LEN,
			   src, &size)) {
			puts("Uncompressing error\n");
			return -EIO;
		}
		length = size;
	} else {
		memcpy((void *)load_addr, src, length);
	}

	if (image_info) {
		image_info->load_addr = load_addr;
		image_info->size = length;
		image_info->entry_point = fdt_getprop_u32(fit, node, "entry");
	}

	return 0;
}

int spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong sector, void *fit)
{
	int sectors;
	ulong size;
	unsigned long count;
	struct spl_image_info image_info;
	bool boot_os = false;
	int node = -1;
	int images, ret;
	int base_offset, align_len = ARCH_DMA_MINALIGN - 1;
	int index = 0;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
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
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	fit = (void *)((CONFIG_SYS_TEXT_BASE - size - info->bl_len -
			align_len) & ~align_len);
	sectors = get_aligned_image_size(info, size, 0);
	count = info->read(info, sector, sectors, fit);
	debug("fit read sector %lx, sectors=%d, dst=%p, count=%lu\n",
	      sector, sectors, fit, count);
	if (count == 0)
		return -EIO;

	/* find the node holding the images information */
	images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__, images);
		return -1;
	}

#ifdef CONFIG_SPL_OS_BOOT
	/* Find OS image first */
	node = spl_fit_get_image_node(fit, images, FIT_KERNEL_PROP, 0);
	if (node < 0)
		debug("No kernel image.\n");
	else
		boot_os = true;
#endif
	/* find the U-Boot image */
	if (node < 0)
		node = spl_fit_get_image_node(fit, images, "firmware", 0);
	if (node < 0) {
		debug("could not find firmware image, trying loadables...\n");
		node = spl_fit_get_image_node(fit, images, "loadables", 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	if (node < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, node);
		return -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = spl_load_fit_image(info, sector, fit, base_offset, node,
				 spl_image);
	if (ret)
		return ret;

#ifdef CONFIG_SPL_OS_BOOT
	if (!fit_image_get_os(fit, node, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
#else
	spl_image->os = IH_OS_U_BOOT;
#endif

	if (!boot_os) {
		/* Figure out which device tree the board wants to use */
		node = spl_fit_get_image_node(fit, images, FIT_FDT_PROP, 0);
		if (node < 0) {
			debug("%s: cannot find FDT node\n", __func__);
			return node;
		}

		/*
		 * Read the device tree and place it after the image.
		 * Align the destination address to ARCH_DMA_MINALIGN.
		 */
		image_info.load_addr = spl_image->load_addr + spl_image->size;
		ret = spl_load_fit_image(info, sector, fit, base_offset, node,
					 &image_info);
		if (ret < 0)
			return ret;
	}

	/* Now check if there are more images for us to load */
	for (; ; index++) {
		node = spl_fit_get_image_node(fit, images, "loadables", index);
		if (node < 0)
			break;

		ret = spl_load_fit_image(info, sector, fit, base_offset, node,
					 &image_info);
		if (ret < 0)
			continue;

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		if (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	if (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	return 0;
}

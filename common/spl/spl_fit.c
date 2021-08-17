// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <errno.h>
#include <fpga.h>
#include <gzip.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <spl.h>
#include <sysinfo.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ
#define CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ (64 * 1024)
#endif

#ifndef CONFIG_SYS_BOOTM_LEN
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)
#endif

struct spl_fit_info {
	const void *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	int images_node;	/* FDT offset to "/images" node */
	int conf_node;		/* FDT offset to selected configuration node */
};

__weak void board_spl_fit_post_load(const void *fit)
{
}

__weak ulong board_spl_fit_size_align(ulong size)
{
	return size;
}

static int find_node_from_desc(const void *fit, int node, const char *str)
{
	int child;

	if (node < 0)
		return -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	for (child = fdt_first_subnode(fit, node); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		int len;
		const char *desc = fdt_getprop(fit, child, "description", &len);

		if (!desc)
			continue;

		if (!strcmp(desc, str))
			return child;
	}

	return -ENOENT;
}

/**
 * spl_fit_get_image_name(): By using the matching configuration subnode,
 * retrieve the name of an image, specified by a property name and an index
 * into that.
 * @fit:	Pointer to the FDT blob.
 * @images:	Offset of the /images subnode.
 * @type:	Name of the property within the configuration subnode.
 * @index:	Index into the list of strings in this property.
 * @outname:	Name of the image
 *
 * Return:	0 on success, or a negative error number
 */
static int spl_fit_get_image_name(const struct spl_fit_info *ctx,
				  const char *type, int index,
				  const char **outname)
{
	struct udevice *sysinfo;
	const char *name, *str;
	__maybe_unused int node;
	int len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_node, type, &len);
	if (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		return -EINVAL;
	}

	str = name;
	for (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		if (!str || (str - name >= len)) {
			found = false;
			break;
		}
	}

	if (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		int rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		if (rc)
			return rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		if (rc && rc != -ENOENT)
			return rc;

		if (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			int node;
			int images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			node = find_node_from_desc(ctx->fit, images, str);
			if (node > 0)
				str = fdt_get_name(ctx->fit, node, NULL);

			found = true;
		}
	}

	if (!found) {
		debug("no string for index %d\n", index);
		return -E2BIG;
	}

	*outname = str;
	return 0;
}

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
 *		error number.
 */
static int spl_fit_get_image_node(const struct spl_fit_info *ctx,
				  const char *type, int index)
{
	const char *str;
	int err;
	int node;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	if (err)
		return err;

	debug("%s: '%s'\n", type, str);

	node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
	if (node < 0) {
		pr_err("cannot find image node '%s': %d\n", str, node);
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
 * @ctx:	points to the FIT context structure
 * @node:	offset of the DT node describing the image to load (relative
 *		to @fit)
 * @image_info:	will be filled with information about the loaded image
 *		If the FIT node does not contain a "load" (address) property,
 *		the image gets loaded to the address pointed to by the
 *		load_addr member in this struct, if load_addr is not 0
 *
 * Return:	0 on success or a negative error number.
 */
static int spl_load_fit_image(struct spl_load_info *info, ulong sector,
			      const struct spl_fit_info *ctx, int node,
			      struct spl_image_info *image_info)
{
	int offset;
	size_t length;
	int len;
	ulong size;
	ulong load_addr;
	void *load_ptr;
	void *src;
	ulong overhead;
	int nr_sectors;
	uint8_t image_comp = -1, type = -1;
	const void *data;
	const void *fit = ctx->fit;
	bool external_data = false;

	if (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_GZIP))) {
		if (fit_image_get_type(fit, node, &type))
			puts("Cannot get image type.\n");
		else
			debug("%s ", genimg_get_type_name(type));
	}

	if (IS_ENABLED(CONFIG_SPL_GZIP)) {
		fit_image_get_comp(fit, node, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	if (fit_image_get_load(fit, node, &load_addr)) {
		if (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, node, NULL));
			return -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	if (!fit_image_get_data_position(fit, node, &offset)) {
		external_data = true;
	} else if (!fit_image_get_data_offset(fit, node, &offset)) {
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	if (external_data) {
		void *src_ptr;

		/* External data */
		if (fit_image_get_data_size(fit, node, &len))
			return -ENOENT;

		src_ptr = map_sysmem(ALIGN(load_addr, ARCH_DMA_MINALIGN), len);
		length = len;

		overhead = get_aligned_image_overhead(info, offset);
		nr_sectors = get_aligned_image_size(info, length, offset);

		if (info->read(info,
			       sector + get_aligned_image_offset(info, offset),
			       nr_sectors, src_ptr) != nr_sectors)
			return -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + overhead;
	} else {
		/* Embedded data */
		if (fit_image_get_data(fit, node, &data, &length)) {
			puts("Cannot get image data/size\n");
			return -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (void *)data;	/* cast away const */
	}

	if (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, node, NULL));
		if (!fit_image_verify_with_data(fit, node, src, length))
			return -EPERM;
		puts("OK\n");
	}

	if (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, node, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	if (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		if (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			return -EIO;
		}
		length = size;
	} else {
		memcpy(load_ptr, src, length);
	}

	if (image_info) {
		ulong entry_point;

		image_info->load_addr = load_addr;
		image_info->size = length;

		if (!fit_image_get_entry(fit, node, &entry_point))
			image_info->entry_point = entry_point;
		else
			image_info->entry_point = FDT_ERROR;
	}

	return 0;
}

static bool os_takes_devicetree(uint8_t os)
{
	switch (os) {
	case IH_OS_U_BOOT:
		return true;
	case IH_OS_LINUX:
		return IS_ENABLED(CONFIG_SPL_OS_BOOT);
	default:
		return false;
	}
}

static int spl_fit_append_fdt(struct spl_image_info *spl_image,
			      struct spl_load_info *info, ulong sector,
			      const struct spl_fit_info *ctx)
{
	struct spl_image_info image_info;
	int node, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = spl_image->load_addr + spl_image->size;

	/* Figure out which device tree the board wants to use */
	node = spl_fit_get_image_node(ctx, FIT_FDT_PROP, index++);
	if (node < 0) {
		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		if (gd->fdt_blob)
			memcpy((void *)image_info.load_addr, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
		else
			return node;
	} else {
		ret = spl_load_fit_image(info, sector, ctx, node,
					 &image_info);
		if (ret < 0)
			return ret;
	}

	/* Make the load-address of the FDT available for the SPL framework */
	spl_image->fdt_addr = map_sysmem(image_info.load_addr, 0);
	if (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		return 0;

	if (CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)) {
		void *tmpbuffer = NULL;

		for (; ; index++) {
			node = spl_fit_get_image_node(ctx, FIT_FDT_PROP, index);
			if (node == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				break;
			} else if (node < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				continue;
			}

			if (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				tmpbuffer = malloc(CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
				if (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = spl_load_fit_image(info, sector, ctx,
						 node, &image_info);
			if (ret < 0)
				break;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			if (ret < 0)
				break;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(void *)image_info.load_addr);
			if (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, node, NULL));
				break;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, node, NULL));
		}
		free(tmpbuffer);
		if (ret)
			return ret;
	}
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	if (ret < 0)
		return ret;

	return ret;
}

static int spl_fit_record_loadable(const struct spl_fit_info *ctx, int index,
				   void *blob, struct spl_image_info *image)
{
	int ret = 0;
	const char *name;
	int node;

	if (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		return 0;

	ret = spl_fit_get_image_name(ctx, "loadables", index, &name);
	if (ret < 0)
		return ret;

	node = spl_fit_get_image_node(ctx, "loadables", index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
				  image->size, image->entry_point,
				  fdt_getprop(ctx->fit, node, "type", NULL),
				  fdt_getprop(ctx->fit, node, "os", NULL),
				  fdt_getprop(ctx->fit, node, "arch", NULL));
	return ret;
}

static int spl_fit_image_is_fpga(const void *fit, int node)
{
	const char *type;

	if (!IS_ENABLED(CONFIG_SPL_FPGA))
		return 0;

	type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);
	if (!type)
		return 0;

	return !strcmp(type, "fpga");
}

static int spl_fit_image_get_os(const void *fit, int noffset, uint8_t *os)
{
	if (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		return fit_image_get_os(fit, noffset, os);

	const char *name = fdt_getprop(fit, noffset, FIT_OS_PROP, NULL);
	if (!name)
		return -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	if (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	else
		*os = IH_OS_INVALID;

	return 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
static void *spl_get_fit_load_buffer(size_t size)
{
	void *buf;

	buf = malloc(size);
	if (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);
		pr_err("\tcheck CONFIG_SYS_SPL_MALLOC_SIZE\n");
		buf = spl_get_load_buffer(0, size);
	}
	return buf;
}

__weak void *board_spl_fit_buffer_addr(ulong fit_size, int sectors, int bl_len)
{
	return spl_get_fit_load_buffer(sectors * bl_len);
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(void)
{
	return false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak void *spl_load_simple_fit_fix_load(const void *fit)
{
	return (void *)fit;
}

static void warn_deprecated(const char *msg)
{
	printf("DEPRECATED: %s\n", msg);
	printf("\tSee doc/uImage.FIT/source_file_format.txt\n");
}

static int spl_fit_upload_fpga(struct spl_fit_info *ctx, int node,
			       struct spl_image_info *fpga_image)
{
	const char *compatible;
	int ret;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, node, "compatible", NULL);
	if (!compatible)
		warn_deprecated("'fpga' image without 'compatible' property");
	else if (strcmp(compatible, "u-boot,fpga-legacy"))
		printf("Ignoring compatible = %s property\n", compatible);

	ret = fpga_load(0, (void *)fpga_image->load_addr, fpga_image->size,
			BIT_FULL);
	if (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		return ret;
	}

	puts("FPGA image loaded from FIT\n");

	return 0;
}

static int spl_fit_load_fpga(struct spl_fit_info *ctx,
			     struct spl_load_info *info, ulong sector)
{
	int node, ret;

	struct spl_image_info fpga_image = {
		.load_addr = 0,
	};

	node = spl_fit_get_image_node(ctx, "fpga", 0);
	if (node < 0)
		return node;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = spl_load_fit_image(info, sector, ctx, node, &fpga_image);
	if (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		return ret;
	}

	return spl_fit_upload_fpga(ctx, node, &fpga_image);
}

static int spl_simple_fit_read(struct spl_fit_info *ctx,
			       struct spl_load_info *info, ulong sector,
			       const void *fit_header)
{
	unsigned long count, size;
	int sectors;
	void *buf;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	size = ALIGN(fdt_totalsize(fit_header), 4);
	size = board_spl_fit_size_align(size);
	ctx->ext_data_offset = ALIGN(size, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	sectors = get_aligned_image_size(info, size, 0);
	buf = board_spl_fit_buffer_addr(size, sectors, info->bl_len);

	count = info->read(info, sector, sectors, buf);
	ctx->fit = buf;
	debug("fit read sector %lx, sectors=%d, dst=%p, count=%lu, size=0x%lx\n",
	      sector, sectors, buf, count, size);

	return (count == 0) ? -EIO : 0;
}

static int spl_simple_fit_parse(struct spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_node = fit_find_config_node(ctx->fit);
	if (ctx->conf_node < 0)
		return -EINVAL;

	if (IS_ENABLED(CONFIG_SPL_FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_node, NULL));
		if (fit_config_verify(ctx->fit, ctx->conf_node))
			return -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_node = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	if (ctx->images_node < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_node);
		return -EINVAL;
	}

	return 0;
}

int spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong sector, void *fit)
{
	struct spl_image_info image_info;
	struct spl_fit_info ctx;
	int node = -1;
	int ret;
	int index = 0;
	int firmware_node;

	ret = spl_simple_fit_read(&ctx, info, sector, fit);
	if (ret < 0)
		return ret;

	/* skip further processing if requested to enable load-only use cases */
	if (spl_load_simple_fit_skip_processing())
		return 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	if (ret < 0)
		return ret;

	if (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, sector);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	if (node < 0)
		node = spl_fit_get_image_node(&ctx, FIT_FIRMWARE_PROP, 0);

	if (node < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		node = spl_fit_get_image_node(&ctx, FIT_KERNEL_PROP, 0);

	if (node < 0) {
		debug("could not find firmware image, trying loadables...\n");
		node = spl_fit_get_image_node(&ctx, "loadables", 0);
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
	ret = spl_load_fit_image(info, sector, &ctx, node, spl_image);
	if (ret)
		return ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	if (!spl_fit_image_get_os(ctx.fit, node, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	else if (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	if (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, sector, &ctx);
		if (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			return ret;
	}

	firmware_node = node;
	/* Now check if there are more images for us to load */
	for (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		node = spl_fit_get_image_node(&ctx, "loadables", index);
		if (node < 0)
			break;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		if (firmware_node == node)
			continue;

		image_info.load_addr = 0;
		ret = spl_load_fit_image(info, sector, &ctx, node, &image_info);
		if (ret < 0) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			return ret;
		}

		if (spl_fit_image_is_fpga(ctx.fit, node))
			spl_fit_upload_fpga(&ctx, node, &image_info);

		if (!spl_fit_image_get_os(ctx.fit, node, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		if (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, sector, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		if (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		/* Record our loadables into the FDT */
		if (spl_image->fdt_addr)
			spl_fit_record_loadable(&ctx, index,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	if (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;

	if (IS_ENABLED(CONFIG_IMX_HAB))
		board_spl_fit_post_load(ctx.fit);

	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <image.h>
#include <imx_container.h>
#include <mapmem.h>
#include <memalign.h>
#include <rand.h>
#include <spl.h>
#include <test/spl.h>
#include <test/ut.h>
#include <u-boot/crc.h>

int board_fit_config_name_match(const char *name)
{
	return 0;
}

struct legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return map_sysmem(0x100000, 0);
}

/* Try to reuse the load buffer to conserve memory */
void *board_spl_fit_buffer_addr(ulong fit_size, int sectors, int bl_len)
{
	static void *buf;
	static size_t size;

	if (size < sectors * bl_len) {
		free(buf);
		size = sectors * bl_len;
		buf = malloc_cache_aligned(size);
	}
	return buf;
}

/* Local flags for spl_image; start from the "top" to avoid conflicts */
#define SPL_IMX_CONTAINER	0x80000000

void generate_data(char *data, size_t size, const char *test_name)
{
	int i;
	unsigned int seed = 1;

	while (*test_name) {
		seed += *test_name++;
		rand_r(&seed);
	}

	for (i = 0; i < size; i++)
		data[i] = (i & 0xf) << 4 | (rand_r(&seed) & 0xf);
}

static size_t create_legacy(void *dst, struct spl_image_info *spl_image,
			    size_t *data_offset)
{
	struct legacy_img_hdr *hdr = dst;
	void *data = dst + sizeof(*hdr);

	if (data_offset)
		*data_offset = data - dst;

	if (!dst)
		goto out;

	image_set_magic(hdr, IH_MAGIC);
	image_set_time(hdr, 0);
	image_set_size(hdr, spl_image->size);
	image_set_load(hdr, spl_image->load_addr);
	image_set_ep(hdr, spl_image->entry_point);
	image_set_dcrc(hdr, crc32(0, data, spl_image->size));
	image_set_os(hdr, spl_image->os);
	image_set_arch(hdr, IH_ARCH_DEFAULT);
	image_set_type(hdr, IH_TYPE_FIRMWARE);
	image_set_comp(hdr, IH_COMP_NONE);
	image_set_name(hdr, spl_image->name);
	image_set_hcrc(hdr, crc32(0, (void *)hdr, sizeof(*hdr)));

out:
	return sizeof(*hdr) + spl_image->size;
}

static size_t create_imx8(void *dst, struct spl_image_info *spl_image,
			  size_t *data_offset)
{
	struct container_hdr *hdr = dst;
	struct boot_img_t *img = dst + sizeof(*hdr);
	size_t length = sizeof(*hdr) + sizeof(*img);
	/* Align to MMC block size for now */
	void *data = dst + 512;

	if (data_offset)
		*data_offset = data - dst;

	if (!dst)
		goto out;

	hdr->version = CONTAINER_HDR_VERSION;
	hdr->length_lsb = length & 0xff;
	hdr->length_msb = length >> 8;
	hdr->tag = CONTAINER_HDR_TAG;
	hdr->num_images = 1;

	/* spl_load_imx_container doesn't handle endianness; whoops! */
	img->offset = data - dst;
	img->size = spl_image->size;
	img->dst = spl_image->load_addr;
	img->entry = spl_image->entry_point;

out:
	return data - dst + spl_image->size;
}

#define ADDRESS_CELLS (sizeof(uintptr_t) / sizeof(u32))

static inline int fdt_property_addr(void *fdt, const char *name, uintptr_t val)
{
	if (sizeof(uintptr_t) == sizeof(u32))
		return fdt_property_u32(fdt, name, val);
	return fdt_property_u64(fdt, name, val);
}

static size_t start_fit(void *dst, size_t fit_size, size_t data_size,
			bool external)
{
	void *data;

	if (fdt_create(dst, fit_size))
		return 0;
	if (fdt_finish_reservemap(dst))
		return 0;
	if (fdt_begin_node(dst, ""))
		return 0;
	if (fdt_property_u32(dst, FIT_TIMESTAMP_PROP, 0))
		return 0;
	if (fdt_property_u32(dst, "#address-cells", ADDRESS_CELLS))
		return 0;
	if (fdt_property_string(dst, FIT_DESC_PROP, ""))
		return 0;

	if (fdt_begin_node(dst, "images"))
		return 0;
	if (fdt_begin_node(dst, "u-boot"))
		return 0;

	if (external) {
		if (fdt_property_u32(dst, FIT_DATA_OFFSET_PROP, 0))
			return 0;
		return fit_size;
	}

	if (fdt_property_placeholder(dst, FIT_DATA_PROP, data_size, &data))
		return 0;
	return data - dst;
}

static size_t create_fit(void *dst, struct spl_image_info *spl_image,
			 size_t *data_offset, bool external)
{
	size_t prop_size = 596, total_size = prop_size + spl_image->size;
	size_t off, size;

	if (external) {
		size = prop_size;
		off = size;
	} else {
		char tmp[256];

		size = total_size;
		off = start_fit(tmp, sizeof(tmp), 0, false);
		if (!off)
			return 0;
	}

	if (data_offset)
		*data_offset = off;

	if (!dst)
		goto out;

	if (start_fit(dst, size, spl_image->size, external) != off)
		return 0;

	if (fdt_property_string(dst, FIT_DESC_PROP, spl_image->name))
		return 0;
	if (fdt_property_string(dst, FIT_TYPE_PROP, "firmware"))
		return 0;
	if (fdt_property_string(dst, FIT_COMP_PROP, "none"))
		return 0;
	if (fdt_property_u32(dst, FIT_DATA_SIZE_PROP, spl_image->size))
		return 0;
	if (fdt_property_string(dst, FIT_OS_PROP,
				genimg_get_os_short_name(spl_image->os)))
		return 0;
	if (fdt_property_string(dst, FIT_ARCH_PROP,
				genimg_get_arch_short_name(IH_ARCH_DEFAULT)))
		return 0;
	if (fdt_property_addr(dst, FIT_ENTRY_PROP, spl_image->entry_point))
		return 0;
	if (fdt_property_addr(dst, FIT_LOAD_PROP, spl_image->load_addr))
		return 0;
	if (fdt_end_node(dst)) /* u-boot */
		return 0;
	if (fdt_end_node(dst)) /* images */
		return 0;

	if (fdt_begin_node(dst, "configurations"))
		return 0;
	if (fdt_property_string(dst, FIT_DEFAULT_PROP, "config-1"))
		return 0;
	if (fdt_begin_node(dst, "config-1"))
		return 0;
	if (fdt_property_string(dst, FIT_DESC_PROP, spl_image->name))
		return 0;
	if (fdt_property_string(dst, FIT_FIRMWARE_PROP, "u-boot"))
		return 0;
	if (fdt_end_node(dst)) /* configurations */
		return 0;
	if (fdt_end_node(dst)) /* config-1 */
		return 0;

	if (fdt_end_node(dst)) /* root */
		return 0;
	if (fdt_finish(dst))
		return 0;

	if (external) {
		if (fdt_totalsize(dst) > size)
			return 0;
		fdt_set_totalsize(dst, size);
	}

out:
	return total_size;
}

size_t create_image(void *dst, enum spl_test_image type,
		    struct spl_image_info *info, size_t *data_offset)
{
	bool external = false;

	info->os = IH_OS_U_BOOT;
	info->load_addr = CONFIG_TEXT_BASE;
	info->entry_point = CONFIG_TEXT_BASE + 0x100;
	info->flags = 0;

	switch (type) {
	case LEGACY:
		return create_legacy(dst, info, data_offset);
	case IMX8:
		info->flags = SPL_IMX_CONTAINER;
		return create_imx8(dst, info, data_offset);
	case FIT_EXTERNAL:
		/*
		 * spl_fit_append_fdt will clobber external images with U-Boot's
		 * FDT if the image doesn't have one. Just set the OS to
		 * something which doesn't take a devicetree.
		 */
		if (!IS_ENABLED(CONFIG_LOAD_FIT_FULL))
			info->os = IH_OS_TEE;
		external = true;
	case FIT_INTERNAL:
		info->flags = SPL_FIT_FOUND;
		return create_fit(dst, info, data_offset, external);
	}

	return 0;
}

int check_image_info(struct unit_test_state *uts, struct spl_image_info *info1,
		     struct spl_image_info *info2)
{
	if (info2->name) {
		if (info1->flags & SPL_FIT_FOUND)
			ut_asserteq_str(genimg_get_os_name(info1->os),
					info2->name);
		else
			ut_asserteq_str(info1->name, info2->name);
	}

	if (info1->flags & SPL_IMX_CONTAINER)
		ut_asserteq(IH_OS_INVALID, info2->os);
	else
		ut_asserteq(info1->os, info2->os);

	ut_asserteq(info1->entry_point, info2->entry_point);
	if (info1->flags & (SPL_FIT_FOUND | SPL_IMX_CONTAINER) ||
	    info2->flags & SPL_COPY_PAYLOAD_ONLY) {
		ut_asserteq(info1->load_addr, info2->load_addr);
		if (info1->flags & SPL_IMX_CONTAINER)
			ut_asserteq(0, info2->size);
		else
			ut_asserteq(info1->size, info2->size);
	} else {
		ut_asserteq(info1->load_addr - sizeof(struct legacy_img_hdr),
			    info2->load_addr);
		ut_asserteq(info1->size + sizeof(struct legacy_img_hdr),
			    info2->size);
	}

	return 0;
}

static ulong spl_test_read(struct spl_load_info *load, ulong sector,
			   ulong count, void *buf)
{
	memcpy(buf, load->priv + sector, count);
	return count;
}

static int spl_test_image(struct unit_test_state *uts, const char *test_name,
			  enum spl_test_image type)
{
	size_t img_size, img_data, data_size = SPL_TEST_DATA_SIZE;
	struct spl_image_info info_write = {
		.name = test_name,
		.size = data_size,
	}, info_read = { };
	char *data;
	void *img;

	img_size = create_image(NULL, type, &info_write, &img_data);
	ut_assert(img_size);
	img = calloc(img_size, 1);
	ut_assertnonnull(img);

	data = img + img_data;
	generate_data(data, data_size, test_name);
	ut_asserteq(img_size, create_image(img, type, &info_write, NULL));

	if (type == LEGACY) {
		ut_assertok(spl_parse_image_header(&info_read, NULL, img));
		if (check_image_info(uts, &info_write, &info_read))
			return CMD_RET_FAILURE;
	} else {
		struct spl_load_info load = {
			.bl_len = 1,
			.priv = img,
			.read = spl_test_read,
		};

		if (type == IMX8)
			ut_assertok(spl_load_imx_container(&info_read, &load,
							   0));
		else if (IS_ENABLED(CONFIG_SPL_FIT_FULL))
			ut_assertok(spl_parse_image_header(&info_read, NULL,
							   img));
		else
			ut_assertok(spl_load_simple_fit(&info_read, &load, 0,
							img));
		if (check_image_info(uts, &info_write, &info_read))
			return CMD_RET_FAILURE;
		ut_asserteq_mem(data, phys_to_virt(info_write.load_addr),
				data_size);
	}

	free(img);
	return 0;
}
SPL_IMG_TEST(spl_test_image, LEGACY, 0);
SPL_IMG_TEST(spl_test_image, IMX8, 0);
SPL_IMG_TEST(spl_test_image, FIT_INTERNAL, 0);
SPL_IMG_TEST(spl_test_image, FIT_EXTERNAL, 0);

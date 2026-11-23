// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2026 Carlo Caione <ccaione@baylibre.com> */

#include <image.h>
#include <mapmem.h>
#include <spl.h>
#include <test/spl.h>
#include <test/ut.h>

static int check_ram_image(struct unit_test_state *uts, void *image, size_t size,
			   struct spl_image_info *expected, const void *data)
{
	struct spl_image_info loaded = {};

	ut_assertok(spl_ram_load_fit(&loaded, image, size));
	ut_assertok(check_image_info(uts, expected, &loaded));
	ut_asserteq_mem(data, map_sysmem(loaded.load_addr, loaded.size), loaded.size);
	return 0;
}

static int spl_test_ram(struct unit_test_state *uts, const char *name,
			enum spl_test_image type)
{
	struct spl_image_info info = { .name = name, .size = SPL_TEST_DATA_SIZE };
	size_t size, offset;
	void *image;
	int ret;

	size = create_image(NULL, type, &info, &offset);
	ut_assert(size);
	image = calloc(1, size);
	ut_assertnonnull(image);
	generate_data(image + offset, info.size, name);
	if (create_image(image, type, &info, NULL) != size) {
		free(image);
		return -EINVAL;
	}
	if (type == FIT_INTERNAL)
		size = fdt_totalsize(image);
	ret = check_ram_image(uts, image, size, &info, image + offset);
	if (!ret)
		ret = check_ram_image(uts, image, SIZE_MAX, &info, image + offset);
	free(image);
	return ret;
}

SPL_IMG_TEST(spl_test_ram, FIT_INTERNAL, 0);
SPL_IMG_TEST(spl_test_ram, FIT_EXTERNAL, 0);

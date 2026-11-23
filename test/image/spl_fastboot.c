// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2026 Carlo Caione <ccaione@baylibre.com> */

#include <android_image.h>
#include <fastboot.h>
#include <image.h>
#include <mapmem.h>
#include <spl.h>
#include <test/spl.h>
#include <test/ut.h>

static int run_fastboot_command(struct unit_test_state *uts, const char *command,
				int expected_cmd, const char *expected_response)
{
	char response[FASTBOOT_RESPONSE_LEN];
	char cmd[FASTBOOT_COMMAND_LEN];

	strlcpy(cmd, command, sizeof(cmd));
	ut_asserteq(expected_cmd, fastboot_handle_command(cmd, response));
	ut_asserteq_str(expected_response, response);
	return 0;
}

static int download_image(struct unit_test_state *uts, const void *data,
			  size_t size)
{
	char command[FASTBOOT_COMMAND_LEN];
	char expected[FASTBOOT_RESPONSE_LEN];
	char response[FASTBOOT_RESPONSE_LEN];

	snprintf(command, sizeof(command), "download:%08x", (uint)size);
	snprintf(expected, sizeof(expected), "DATA%08x", (uint)size);
	ut_assertok(run_fastboot_command(uts, command, FASTBOOT_COMMAND_DOWNLOAD, expected));
	fastboot_data_download(data, size, response);
	ut_asserteq_str("", response);
	fastboot_data_complete(response);
	ut_asserteq_str("OKAY", response);
	return 0;
}

static int spl_test_fastboot_state(struct unit_test_state *uts)
{
	char buffer[64], data[32] = {}, response[FASTBOOT_RESPONSE_LEN];
	struct spl_image_info info = {};
	void *image;

	fastboot_init(buffer, sizeof(buffer));
	ut_assertok(run_fastboot_command(uts, "boot", FASTBOOT_COMMAND_BOOT,
					 "FAILNo complete image downloaded"));
	ut_asserteq(-EAGAIN, spl_fastboot_load(&info));
	ut_assertok(download_image(uts, data, sizeof(data)));
	ut_assertok(run_fastboot_command(uts, "continue", FASTBOOT_COMMAND_CONTINUE, "OKAY"));
	ut_asserteq(0, fastboot_get_boot_image(&image));
	ut_assertok(run_fastboot_command(uts, "boot", FASTBOOT_COMMAND_BOOT, "OKAY"));
	/* The request is only made when the USB OKAY has completed. */
	ut_asserteq(0, fastboot_get_boot_image(&image));
	fastboot_boot();
	ut_asserteq(sizeof(data), fastboot_get_boot_image(&image));
	ut_asserteq_ptr(buffer, image);

	ut_assertok(run_fastboot_command(uts, "download:00000020", FASTBOOT_COMMAND_DOWNLOAD,
					 "DATA00000020"));
	fastboot_data_download(data, 16, response);
	fastboot_data_complete(response);
	ut_asserteq_str("FAILIncomplete download", response);
	ut_assertok(run_fastboot_command(uts, "boot", FASTBOOT_COMMAND_BOOT,
					 "FAILNo complete image downloaded"));
	ut_asserteq(0, fastboot_get_boot_image(&image));

	ut_assertok(download_image(uts, data, sizeof(data)));
	fastboot_boot();
	fastboot_init(buffer, sizeof(buffer));
	ut_asserteq(0, fastboot_get_boot_image(&image));
	ut_assertok(run_fastboot_command(uts, "boot", FASTBOOT_COMMAND_BOOT,
					 "FAILNo complete image downloaded"));
	return 0;
}

SPL_TEST(spl_test_fastboot_state, 0);

static int check_download(struct unit_test_state *uts, void *image, size_t size,
			  struct spl_image_info *expected, const void *data)
{
	struct spl_image_info loaded = {};
	int ret;

	ut_assertok(download_image(uts, image, size));
	ut_assertok(run_fastboot_command(uts, "boot", FASTBOOT_COMMAND_BOOT, "OKAY"));
	fastboot_boot();
	ret = spl_fastboot_load(&loaded);
	if (!expected) {
		ut_assert(ret < 0);
		return 0;
	}
	if (CONFIG_IS_ENABLED(FIT_SIGNATURE) &&
	    !(expected->flags & SPL_FIT_FOUND)) {
		ut_asserteq(-EPERM, ret);
		ut_asserteq(0, loaded.size);
		return 0;
	}
	ut_assertok(ret);
	ut_assertok(check_image_info(uts, expected, &loaded));
	ut_asserteq_mem(data, map_sysmem(expected->load_addr, expected->size),
			expected->size);
	return 0;
}

static int check_images(struct unit_test_state *uts, void *image, size_t size,
			struct spl_image_info *info, const void *data,
			void *wrapped, enum spl_test_image type)
{
	struct andr_boot_img_hdr_v0 *hdr = wrapped;
	size_t wrapped_size = 2048 + size;
	u32 magic;
	bool fit = image_get_magic(image) == FDT_MAGIC;

	ut_assertok(check_download(uts, image, size, info, data));
	/* Both metadata and payload must fit in the completed download. */
	ut_assertok(check_download(uts, image, (fit ? sizeof(struct fdt_header) :
				   sizeof(struct legacy_img_hdr)) - 1,
				   NULL, NULL));
	if (type == FIT_EXTERNAL)
		ut_assertok(check_download(uts, image, fdt_totalsize(image) - 1,
					   NULL, NULL));
	ut_assertok(check_download(uts, image, size - 1, NULL, NULL));

	if (!IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT) ||
	    CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		magic = fdt_magic(image);
		fdt_set_magic(image, 0);
		ut_assertok(check_download(uts, image, size, NULL, NULL));
		fdt_set_magic(image, magic);
	}

	/* The Android envelope is independent of the payload format. */
	if (type != FIT_INTERNAL)
		return 0;

	memcpy(hdr->magic, ANDR_BOOT_MAGIC, ANDR_BOOT_MAGIC_SIZE);
	hdr->page_size = cpu_to_le32(2048);
	hdr->kernel_size = cpu_to_le32(size);
	ut_assertok(check_download(uts, wrapped, wrapped_size, info, data));
	ut_assertok(check_download(uts, wrapped, sizeof(*hdr) - 100, NULL, NULL));
	ut_assertok(check_download(uts, wrapped, wrapped_size - 1, NULL, NULL));
	/* Android padding must not make a truncated payload look complete. */
	hdr->kernel_size = cpu_to_le32(size - 1);
	ut_assertok(check_download(uts, wrapped, wrapped_size, NULL, NULL));
	hdr->kernel_size = cpu_to_le32(size);
	hdr->header_version = cpu_to_le32(3);
	ut_assertok(check_download(uts, wrapped, wrapped_size, NULL, NULL));
	hdr->header_version = 0;
	hdr->page_size = cpu_to_le32(2047);
	ut_assertok(check_download(uts, wrapped, wrapped_size, NULL, NULL));
	hdr->page_size = cpu_to_le32(2048);
	hdr->ramdisk_size = cpu_to_le32(1);
	ut_assertok(check_download(uts, wrapped, wrapped_size, NULL, NULL));
	return 0;
}

static int spl_test_fastboot(struct unit_test_state *uts, const char *name,
			     enum spl_test_image type)
{
	struct spl_image_info info = { .name = name, .size = SPL_TEST_DATA_SIZE };
	size_t size, offset;
	void *image, *buffer, *wrapped;
	int ret = -ENOMEM;

	size = create_image(NULL, type, &info, &offset);
	ut_assert(size);
	buffer = malloc(size + 2048);
	wrapped = calloc(1, size + 2048);
	if (!buffer || !wrapped)
		goto out;

	image = wrapped + 2048;
	generate_data(image + offset, info.size, name);
	if (create_image(image, type, &info, NULL) != size) {
		ret = -EINVAL;
		goto out;
	}
	if (type == FIT_INTERNAL)
		size = fdt_totalsize(image);
	fastboot_init(buffer, size + 2048);
	ret = check_images(uts, image, size, &info, image + offset, wrapped, type);
out:
	fastboot_init(NULL, 0);
	free(wrapped);
	free(buffer);
	ut_assertok(ret);
	return 0;
}

SPL_IMG_TEST(spl_test_fastboot, FIT_INTERNAL, 0);
SPL_IMG_TEST(spl_test_fastboot, FIT_EXTERNAL, 0);
SPL_IMG_TEST(spl_test_fastboot, LEGACY, 0);

static int spl_test_fastboot_raw(struct unit_test_state *uts)
{
	u8 image[31], buffer[sizeof(image)], *dest;
	struct spl_image_info expected = {}, loaded = {};
	int ret;

	if (!IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT))
		return -EAGAIN;
	generate_data(image, sizeof(image), __func__);
	spl_set_header_raw_uboot(&expected);
	dest = map_sysmem(expected.load_addr, sizeof(image) + 1);
	dest[sizeof(image)] = 0xa5;
	fastboot_init(buffer, sizeof(buffer));
	ut_assertok(download_image(uts, image, sizeof(image)));
	ut_assertok(run_fastboot_command(uts, "boot", FASTBOOT_COMMAND_BOOT, "OKAY"));
	fastboot_boot();
	ret = spl_fastboot_load(&loaded);
	fastboot_init(NULL, 0);
	ut_asserteq(0xa5, dest[sizeof(image)]);
	if (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		ut_asserteq(-EPERM, ret);
		ut_asserteq(0, loaded.size);
		return 0;
	}
	ut_assertok(ret);
	ut_asserteq(expected.load_addr, loaded.load_addr);
	ut_asserteq(expected.entry_point, loaded.entry_point);
	ut_asserteq(expected.os, loaded.os);
	ut_asserteq(sizeof(image), loaded.size);
	ut_asserteq_mem(image, dest, sizeof(image));
	return 0;
}

SPL_TEST(spl_test_fastboot_raw, 0);

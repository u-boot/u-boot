/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#ifndef TEST_SPL_H
#define TEST_SPL_H

struct unit_test_state;
struct spl_image_info;

/* Declare a new SPL test */
#define SPL_TEST(_name, _flags)		UNIT_TEST(_name, _flags, spl_test)

/**
 * generate_data() - Generate some test payload data
 * @data: The location to fill
 * @size: The size of @data
 * @test_name: The seed for the data
 *
 * Fill @data with data. The upper nibbles will be an incrementing counter
 * (0x00, 0x10, 0x20...) to make the data identifiable in a hex dump. The lower
 * nibbles are random bits seeded with @test_name.
 */
void generate_data(char *data, size_t size, const char *test_name);

/**
 * enum spl_test_image - Image types for testing
 * @LEGACY: "Legacy" uImages
 * @IMX8: i.MX8 Container images
 * @FIT_INTERNAL: FITs with internal data
 * @FIT_EXTERNAL: FITs with external data
 */
enum spl_test_image {
	LEGACY,
	IMX8,
	FIT_INTERNAL,
	FIT_EXTERNAL,
};

/**
 * create_image() - Create an image for testing
 * @dst: The location to create the image at
 * @type: The type of image to create
 * @info: Image parameters
 * @data_offset: Offset of payload data within the image
 *
 * Create a new image at @dst. @dst must be initialized to all zeros. @info
 * should already have name and size filled in. All other parameters will be
 * filled in by this function. @info can later be passed to check_image_info().
 *
 * If @dst is %NULL, then no data is written. Otherwise, @dst must be
 * initialized to zeros, except payload data which must already be present at
 * @data_offset. @data_offset may be %NULL if unnecessary.
 *
 * Typically, this function will be called as follows:
 *
 *     size = create_image(NULL, type, &info, &off);
 *     img = calloc(size, 1);
 *     generate_data(img + off, ...);
 *     create_image(img, type, &info, NULL);
 *
 * Return: The size of the image, or 0 on error
 */
size_t create_image(void *dst, enum spl_test_image type,
		    struct spl_image_info *info, size_t *data_offset);

/**
 * check_image_info() - Check image info after loading
 * @uts: Current unit test state
 * @info1: The base, known good info
 * @info2: The info to check
 *
 * Check @info2 against @info1. This function is typically called after calling
 * a function to load/parse an image. Image data is not checked.
 *
 * Return: 0 on success, or 1 on failure
 */
int check_image_info(struct unit_test_state *uts, struct spl_image_info *info1,
		     struct spl_image_info *info2);

/**
 * image_supported() - Determine whether an image type is supported
 * @type: The image type to check
 *
 * Return: %true if supported and %false otherwise
 */
static inline bool image_supported(enum spl_test_image type)
{
	switch (type) {
	case LEGACY:
		return IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_FORMAT);
	case IMX8:
		return IS_ENABLED(CONFIG_SPL_LOAD_IMX_CONTAINER);
	case FIT_INTERNAL:
	case FIT_EXTERNAL:
		return IS_ENABLED(CONFIG_SPL_LOAD_FIT) ||
		       IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL);
	}

	return false;
}

/* Declare an image test (skipped if the image type is unsupported) */
#define SPL_IMG_TEST(func, type, flags) \
static int func##_##type(struct unit_test_state *uts) \
{ \
	if (!image_supported(type)) \
		return -EAGAIN; \
	return func(uts, __func__, type); \
} \
SPL_TEST(func##_##type, flags)

/* More than a couple blocks, and will not be aligned to anything */
#define SPL_TEST_DATA_SIZE	4099

/* Flags necessary for accessing DM devices */
#define DM_FLAGS (UT_TESTF_DM | UT_TESTF_SCAN_FDT)

#endif /* TEST_SPL_H */

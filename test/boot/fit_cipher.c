// SPDX-License-Identifier: GPL-2.0+
/*
 * Regression tests for the FIT cipher bounds checks in
 * image_aes_decrypt() and fit_image_get_data_size_unciphered().
 *
 * Copyright 2026 Pranav Rajendran <pranavkasthuri@gmail.com>
 */

#include <errno.h>
#include <image.h>
#include <uboot_aes.h>
#include <u-boot/aes.h>
#include <linux/libfdt.h>
#include <test/test.h>
#include <test/ut.h>

#define FIT_CIPHER_TEST(_name, _flags)	UNIT_TEST(_name, _flags, fit_cipher)

/* A cipher_len that is not a whole number of AES blocks must be rejected */
static int fit_cipher_test_unaligned_len(struct unit_test_state *uts)
{
	struct image_cipher_info info;
	unsigned char key[AES128_KEY_LENGTH] = { 0 };
	unsigned char iv[AES_BLOCK_LENGTH] = { 0 };
	unsigned char cipher[AES_BLOCK_LENGTH + 1] = { 0 };
	void *data = NULL;
	size_t size = 0;
	int ret;

	memset(&info, 0, sizeof(info));
	info.cipher = image_get_cipher_algo("aes128");
	ut_assertnonnull(info.cipher);
	info.key = key;
	info.iv = iv;

	ret = image_aes_decrypt(&info, cipher, sizeof(cipher), &data, &size);
	ut_asserteq(-EINVAL, ret);
	ut_assertnull(data);

	return 0;
}
FIT_CIPHER_TEST(fit_cipher_test_unaligned_len, 0);

/*
 * An unciphered size larger than the ciphertext must be rejected; a size
 * exactly equal to the ciphertext length is the valid boundary case.
 */
static int fit_cipher_test_oversized_unciphered_size(struct unit_test_state *uts)
{
	struct image_cipher_info info;
	unsigned char key[AES128_KEY_LENGTH] = { 0 };
	unsigned char iv[AES_BLOCK_LENGTH] = { 0 };
	unsigned char cipher[AES_BLOCK_LENGTH * 2] = { 0 };
	void *data = NULL;
	size_t size = 0;
	int ret;

	memset(&info, 0, sizeof(info));
	info.cipher = image_get_cipher_algo("aes128");
	ut_assertnonnull(info.cipher);
	info.key = key;
	info.iv = iv;

	info.size_unciphered = sizeof(cipher) + 1;
	ret = image_aes_decrypt(&info, cipher, sizeof(cipher), &data, &size);
	ut_asserteq(-EINVAL, ret);
	ut_assertnull(data);

	/* the boundary itself, size_unciphered == cipher_len, is valid */
	info.size_unciphered = sizeof(cipher);
	ut_assertok(image_aes_decrypt(&info, cipher, sizeof(cipher), &data,
				      &size));
	ut_assertnonnull(data);
	ut_asserteq(sizeof(cipher), size);
	free(data);

	return 0;
}
FIT_CIPHER_TEST(fit_cipher_test_oversized_unciphered_size, 0);

/*
 * A 'data-size-unciphered' property that is not exactly one fdt32_t long
 * must be rejected rather than read out of bounds.
 */
static int fit_cipher_test_data_size_unciphered_len(struct unit_test_state *uts)
{
	char fit[512];
	int images, img, ret;
	u16 truncated = 0x1234;
	fdt32_t valid = cpu_to_fdt32(0x100);
	size_t data_size = 0;

	ut_assertok(fdt_create_empty_tree(fit, sizeof(fit)));
	images = fdt_add_subnode(fit, 0, "images");
	ut_assert(images >= 0);

	img = fdt_add_subnode(fit, images, "kernel");
	ut_assert(img >= 0);
	ut_assertok(fdt_setprop(fit, img, "data-size-unciphered",
				&truncated, sizeof(truncated)));

	ret = fit_image_get_data_size_unciphered(fit, img, &data_size);
	ut_asserteq(-EINVAL, ret);

	/* a correctly sized property still works */
	ut_assertok(fdt_setprop(fit, img, "data-size-unciphered",
				&valid, sizeof(valid)));
	ut_assertok(fit_image_get_data_size_unciphered(fit, img, &data_size));
	ut_asserteq(0x100, data_size);

	return 0;
}
FIT_CIPHER_TEST(fit_cipher_test_data_size_unciphered_len, 0);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for target-side FIT AES decryption
 *
 * Copyright (C) 2026 James Hilliard
 */

#include <image.h>
#include <u-boot/aes.h>
#include <uboot_aes.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

static int lib_test_image_aes_decrypt(struct unit_test_state *uts)
{
	u8 key[AES256_KEY_LENGTH] = { };
	u8 key_exp[AES256_EXPAND_KEY_LENGTH];
	u8 iv[AES_BLOCK_LENGTH] = { };
	u8 plain[2 * AES_BLOCK_LENGTH];
	u8 cipher[sizeof(plain)];
	u8 output[sizeof(plain)];
	struct cipher_algo algo = {
		.name = "aes256",
		.key_len = sizeof(key),
		.iv_len = sizeof(iv),
	};
	struct image_cipher_info info = {
		.cipher = &algo,
		.key = key,
		.iv = iv,
		.size_unciphered = sizeof(plain),
	};
	size_t size;
	int i, ret;

	for (i = 0; i < sizeof(key); i++)
		key[i] = i;
	for (i = 0; i < sizeof(iv); i++)
		iv[i] = 0x80 + i;
	for (i = 0; i < sizeof(plain); i++)
		plain[i] = 0x40 + i;

	aes_expand_key(key, sizeof(key), key_exp);
	aes_cbc_encrypt_blocks(sizeof(key), key_exp, iv, plain, cipher,
			       ARRAY_SIZE(cipher) / AES_BLOCK_LENGTH);

	size = 0;
	ut_assertok(image_aes_decrypt_to(&info, cipher, sizeof(cipher), output,
					 &size));
	ut_asserteq(sizeof(plain), size);
	ut_asserteq_mem(plain, output, sizeof(plain));

	memcpy(output, cipher, sizeof(cipher));
	size = 0;
	ut_assertok(image_aes_decrypt_to(&info, output, sizeof(output), output,
					 &size));
	ut_asserteq(sizeof(plain), size);
	ut_asserteq_mem(plain, output, sizeof(plain));

	size = 0x55;
	ret = image_aes_decrypt_to(&info, cipher, sizeof(cipher) - 1, output,
				   &size);
	ut_asserteq(-EINVAL, ret);
	ut_asserteq(0x55, size);

	info.size_unciphered = sizeof(cipher) + 1;
	ret = image_aes_decrypt_to(&info, cipher, sizeof(cipher), output, &size);
	ut_asserteq(-EINVAL, ret);
	info.size_unciphered = sizeof(plain);

	info.key = NULL;
	ret = image_aes_decrypt_to(&info, cipher, sizeof(cipher), output, &size);
	ut_asserteq(-EINVAL, ret);
	info.key = key;
	info.iv = NULL;
	ret = image_aes_decrypt_to(&info, cipher, sizeof(cipher), output, &size);
	ut_asserteq(-EINVAL, ret);
	info.iv = iv;
	info.cipher = NULL;
	ret = image_aes_decrypt_to(&info, cipher, sizeof(cipher), output, &size);
	ut_asserteq(-EINVAL, ret);
	ret = image_aes_decrypt_to(NULL, cipher, sizeof(cipher), output, &size);
	ut_asserteq(-EINVAL, ret);

	return 0;
}

LIB_TEST(lib_test_image_aes_decrypt, 0);

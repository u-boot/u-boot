// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019, Softathome
 */

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <time.h>
#else
#include <common.h>
#include <malloc.h>
DECLARE_GLOBAL_DATA_PTR;
#endif /* !USE_HOSTCC*/
#include <image.h>
#include <uboot_aes.h>
#include <u-boot/aes.h>

struct cipher_algo cipher_algos[] = {
	{
		.name = "aes128",
		.key_len = AES128_KEY_LENGTH,
		.iv_len  = AES_BLOCK_LENGTH,
#if IMAGE_ENABLE_ENCRYPT
		.calculate_type = EVP_aes_128_cbc,
#endif
		.encrypt = image_aes_encrypt,
		.add_cipher_data = image_aes_add_cipher_data
	},
	{
		.name = "aes192",
		.key_len = AES192_KEY_LENGTH,
		.iv_len  = AES_BLOCK_LENGTH,
#if IMAGE_ENABLE_ENCRYPT
		.calculate_type = EVP_aes_192_cbc,
#endif
		.encrypt = image_aes_encrypt,
		.add_cipher_data = image_aes_add_cipher_data
	},
	{
		.name = "aes256",
		.key_len = AES256_KEY_LENGTH,
		.iv_len  = AES_BLOCK_LENGTH,
#if IMAGE_ENABLE_ENCRYPT
		.calculate_type = EVP_aes_256_cbc,
#endif
		.encrypt = image_aes_encrypt,
		.add_cipher_data = image_aes_add_cipher_data
	}
};

struct cipher_algo *image_get_cipher_algo(const char *full_name)
{
	int i;
	const char *name;

	for (i = 0; i < ARRAY_SIZE(cipher_algos); i++) {
		name = cipher_algos[i].name;
		if (!strncmp(name, full_name, strlen(name)))
			return &cipher_algos[i];
	}

	return NULL;
}

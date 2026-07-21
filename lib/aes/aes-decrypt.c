// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019, softathome
 */

#ifndef USE_HOSTCC
#include <dm.h>
#include <malloc.h>
#endif
#include <image.h>
#include <uboot_aes.h>

#ifndef USE_HOSTCC
static int image_aes_validate(struct image_cipher_info *info,
			      const void *cipher, size_t cipher_len,
			      void *data, size_t *size)
{
	if (!info || !info->cipher || !info->key || !info->iv || !cipher ||
	    !data || !size)
		return -EINVAL;
	if (info->cipher->iv_len != AES_BLOCK_LENGTH ||
	    (info->cipher->key_len != AES128_KEY_LENGTH &&
	     info->cipher->key_len != AES192_KEY_LENGTH &&
	     info->cipher->key_len != AES256_KEY_LENGTH))
		return -EINVAL;
	if (!cipher_len || cipher_len % AES_BLOCK_LENGTH ||
	    info->size_unciphered > cipher_len)
		return -EINVAL;

	return 0;
}
#endif

int image_aes_decrypt_to(struct image_cipher_info *info,
			 const void *cipher, size_t cipher_len,
			 void *data, size_t *size)
{
#ifdef USE_HOSTCC
	return -ENOSYS;
#else
	unsigned int aes_blocks, key_len;
	int ret;

	ret = image_aes_validate(info, cipher, cipher_len, data, size);
	if (ret)
		return ret;
	key_len = info->cipher->key_len;
	aes_blocks = cipher_len / AES_BLOCK_LENGTH;

	if (CONFIG_IS_ENABLED(DM_AES)) {
		ret = dm_aes_cbc_decrypt_with_key(key_len * 8, (u8 *)info->key,
						  (u8 *)info->iv, (u8 *)cipher,
						  data, aes_blocks);
		if (!ret) {
			*size = info->size_unciphered;
			return 0;
		}
		if (ret != -ENODEV && ret != -EOPNOTSUPP)
			return ret;
	}

	if (!IS_ENABLED(CONFIG_XPL_BUILD)) {
		unsigned char key_exp[AES256_EXPAND_KEY_LENGTH];

		/* First we expand the key. */
		aes_expand_key((u8 *)info->key, key_len, key_exp);

		aes_cbc_decrypt_blocks(key_len, key_exp, (u8 *)info->iv,
				       (u8 *)cipher, data, aes_blocks);
		*size = info->size_unciphered;
		return 0;
	}

	return -ENOSYS;
#endif
}

int image_aes_decrypt(struct image_cipher_info *info,
		      const void *cipher, size_t cipher_len,
		      void **data, size_t *size)
{
#ifdef USE_HOSTCC
	return 0;
#else
	int ret;

	*data = malloc(cipher_len);
	if (!*data) {
		printf("Can't allocate memory to decrypt\n");
		return -ENOMEM;
	}

	ret = image_aes_decrypt_to(info, cipher, cipher_len, *data, size);
	if (ret) {
		free(*data);
		*data = NULL;
	}

	return ret;
#endif
}

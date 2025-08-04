// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010 - 2011 NVIDIA Corporation <www.nvidia.com>
 */

#include <dm.h>
#include <log.h>
#include <linux/errno.h>
#include <asm/arch-tegra/crypto.h>
#include "uboot_aes.h"

int sign_data_block(u8 *source, unsigned int length, u8 *signature)
{
	struct udevice *dev;
	int ret;

	/* Only one AES engine should be present */
	ret = uclass_get_device(UCLASS_AES, 0, &dev);
	if (ret) {
		log_err("%s: failed to get tegra_aes: %d\n", __func__, ret);
		return ret;
	}

	ret = dm_aes_select_key_slot(dev, 128, TEGRA_AES_SLOT_SBK);
	if (ret)
		return ret;

	return dm_aes_cmac(dev, source, signature,
			   DIV_ROUND_UP(length, AES_BLOCK_LENGTH));
}

int encrypt_data_block(u8 *source, u8 *dest, unsigned int length)
{
	struct udevice *dev;
	int ret;

	/* Only one AES engine should be present */
	ret = uclass_get_device(UCLASS_AES, 0, &dev);
	if (ret) {
		log_err("%s: failed to get tegra_aes: %d\n", __func__, ret);
		return ret;
	}

	ret = dm_aes_select_key_slot(dev, 128, TEGRA_AES_SLOT_SBK);
	if (ret)
		return ret;

	return dm_aes_cbc_encrypt(dev, (u8 *)AES_ZERO_BLOCK, source, dest,
				  DIV_ROUND_UP(length, AES_BLOCK_LENGTH));
}

int decrypt_data_block(u8 *source, u8 *dest, unsigned int length)
{
	struct udevice *dev;
	int ret;

	/* Only one AES engine should be present */
	ret = uclass_get_device(UCLASS_AES, 0, &dev);
	if (ret) {
		log_err("%s: failed to get tegra_aes: %d\n", __func__, ret);
		return ret;
	}

	ret = dm_aes_select_key_slot(dev, 128, TEGRA_AES_SLOT_SBK);
	if (ret)
		return ret;

	return dm_aes_cbc_decrypt(dev, (u8 *)AES_ZERO_BLOCK, source, dest,
				  DIV_ROUND_UP(length, AES_BLOCK_LENGTH));
}

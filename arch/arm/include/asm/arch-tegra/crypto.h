/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010 - 2011 NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#define TEGRA_AES_SLOT_SBK		0

/**
 * sign_data_block - Sign a block of data
 *
 * @source		Source data
 * @length		Size of source data in bytes
 * @signature		Destination address for signature, AES_KEY_LENGTH bytes
 * Return:		0 on success, negative value on failure
 */
int sign_data_block(u8 *source, unsigned int length, u8 *signature);

/**
 * encrypt_data_block - Encrypt a block of data
 *
 * @source		Source data
 * @dest		Destination data
 * @length		Size of source data in bytes
 * Return:		0 on success, negative value on failure
 */
int encrypt_data_block(u8 *source, u8 *dest, unsigned int length);

/**
 * decrypt_data_block - Decrypt a block of data
 *
 * @source		Source data
 * @dest		Destination data
 * @length		Size of source data in bytes
 * Return:		0 on success, negative value on failure
 */
int decrypt_data_block(u8 *source, u8 *dest, unsigned int length);

#endif /* #ifndef _CRYPTO_H_ */

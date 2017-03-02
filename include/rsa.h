/*
 * Copyright (C) 2017 Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _RSA_REF_H_
#define _RSA_REF_H_

/**
 * rsa_hw - verify the authenticated images
 * @key_ptr: Key to use
 * @src_ptr: Source data to authenticate
 * @len: Length of authenticated image
 *
 * Verifies the authenticated images using signature and publickey
 * The key comprises of signature followed by publickey
 *
 * Return: return 0 on success any nonzero on failure
 */
int rsa_hw(u8 *key_ptr, u8 *src_ptr, u32 len);

#endif /* _RSA_REF_H_ */

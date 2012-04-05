/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010 - 2011 NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _AES_REF_H_
#define _AES_REF_H_

/*
 * AES encryption library, with small code size, supporting only 128-bit AES
 *
 * AES is a stream cipher which works a block at a time, with each block
 * in this case being AES_KEY_LENGTH bytes.
 */

enum {
	AES_STATECOLS	= 4,	/* columns in the state & expanded key */
	AES_KEYCOLS	= 4,	/* columns in a key */
	AES_ROUNDS	= 10,	/* rounds in encryption */

	AES_KEY_LENGTH	= 128 / 8,
	AES_EXPAND_KEY_LENGTH	= 4 * AES_STATECOLS * (AES_ROUNDS + 1),
};

/**
 * Expand a key into a key schedule, which is then used for the other
 * operations.
 *
 * \param key		Key, of length AES_KEY_LENGTH bytes
 * \param expkey	Buffer to place expanded key, AES_EXPAND_KEY_LENGTH
 */
void aes_expand_key(u8 *key, u8 *expkey);

/**
 * Encrypt a single block of data
 *
 * in		Input data
 * expkey	Expanded key to use for encryption (from aes_expand_key())
 * out		Output data
 */
void aes_encrypt(u8 *in, u8 *expkey, u8 *out);

/**
 * Decrypt a single block of data
 *
 * in		Input data
 * expkey	Expanded key to use for decryption (from aes_expand_key())
 * out		Output data
 */
void aes_decrypt(u8 *in, u8 *expkey, u8 *out);

#endif /* _AES_REF_H_ */

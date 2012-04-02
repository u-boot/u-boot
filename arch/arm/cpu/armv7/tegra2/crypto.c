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

#include <common.h>
#include <asm/errno.h>
#include "crypto.h"
#include "aes.h"

static u8 zero_key[16];

#define AES_CMAC_CONST_RB 0x87  /* from RFC 4493, Figure 2.2 */

enum security_op {
	SECURITY_SIGN		= 1 << 0,	/* Sign the data */
	SECURITY_ENCRYPT	= 1 << 1,	/* Encrypt the data */
};

static void debug_print_vector(char *name, u32 num_bytes, u8 *data)
{
	u32 i;

	debug("%s [%d] @0x%08x", name, num_bytes, (u32)data);
	for (i = 0; i < num_bytes; i++) {
		if (i % 16 == 0)
			debug(" = ");
		debug("%02x", data[i]);
		if ((i+1) % 16 != 0)
			debug(" ");
	}
	debug("\n");
}

/**
 * Apply chain data to the destination using EOR
 *
 * Each array is of length AES_AES_KEY_LENGTH.
 *
 * \param cbc_chain_data	Chain data
 * \param src			Source data
 * \param dst			Destination data, which is modified here
 */
static void apply_cbc_chain_data(u8 *cbc_chain_data, u8 *src, u8 *dst)
{
	int i;

	for (i = 0; i < 16; i++)
		*dst++ = *src++ ^ *cbc_chain_data++;
}

/**
 * Encrypt some data with AES.
 *
 * \param key_schedule		Expanded key to use
 * \param src			Source data to encrypt
 * \param dst			Destination buffer
 * \param num_aes_blocks	Number of AES blocks to encrypt
 */
static void encrypt_object(u8 *key_schedule, u8 *src, u8 *dst,
			   u32 num_aes_blocks)
{
	u8 tmp_data[AES_KEY_LENGTH];
	u8 *cbc_chain_data;
	u32 i;

	cbc_chain_data = zero_key;	/* Convenient array of 0's for IV */

	for (i = 0; i < num_aes_blocks; i++) {
		debug("encrypt_object: block %d of %d\n", i, num_aes_blocks);
		debug_print_vector("AES Src", AES_KEY_LENGTH, src);

		/* Apply the chain data */
		apply_cbc_chain_data(cbc_chain_data, src, tmp_data);
		debug_print_vector("AES Xor", AES_KEY_LENGTH, tmp_data);

		/* encrypt the AES block */
		aes_encrypt(tmp_data, key_schedule, dst);
		debug_print_vector("AES Dst", AES_KEY_LENGTH, dst);

		/* Update pointers for next loop. */
		cbc_chain_data = dst;
		src += AES_KEY_LENGTH;
		dst += AES_KEY_LENGTH;
	}
}

/**
 * Shift a vector left by one bit
 *
 * \param in	Input vector
 * \param out	Output vector
 * \param size	Length of vector in bytes
 */
static void left_shift_vector(u8 *in, u8 *out, int size)
{
	int carry = 0;
	int i;

	for (i = size - 1; i >= 0; i--) {
		out[i] = (in[i] << 1) | carry;
		carry = in[i] >> 7;	/* get most significant bit */
	}
}

/**
 * Sign a block of data, putting the result into dst.
 *
 * \param key			Input AES key, length AES_KEY_LENGTH
 * \param key_schedule		Expanded key to use
 * \param src			Source data of length 'num_aes_blocks' blocks
 * \param dst			Destination buffer, length AES_KEY_LENGTH
 * \param num_aes_blocks	Number of AES blocks to encrypt
 */
static void sign_object(u8 *key, u8 *key_schedule, u8 *src, u8 *dst,
			u32 num_aes_blocks)
{
	u8 tmp_data[AES_KEY_LENGTH];
	u8 left[AES_KEY_LENGTH];
	u8 k1[AES_KEY_LENGTH];
	u8 *cbc_chain_data;
	unsigned i;

	cbc_chain_data = zero_key;	/* Convenient array of 0's for IV */

	/* compute K1 constant needed by AES-CMAC calculation */
	for (i = 0; i < AES_KEY_LENGTH; i++)
		tmp_data[i] = 0;

	encrypt_object(key_schedule, tmp_data, left, 1);
	debug_print_vector("AES(key, nonce)", AES_KEY_LENGTH, left);

	left_shift_vector(left, k1, sizeof(left));
	debug_print_vector("L", AES_KEY_LENGTH, left);

	if ((left[0] >> 7) != 0) /* get MSB of L */
		k1[AES_KEY_LENGTH-1] ^= AES_CMAC_CONST_RB;
	debug_print_vector("K1", AES_KEY_LENGTH, k1);

	/* compute the AES-CMAC value */
	for (i = 0; i < num_aes_blocks; i++) {
		/* Apply the chain data */
		apply_cbc_chain_data(cbc_chain_data, src, tmp_data);

		/* for the final block, XOR K1 into the IV */
		if (i == num_aes_blocks - 1)
			apply_cbc_chain_data(tmp_data, k1, tmp_data);

		/* encrypt the AES block */
		aes_encrypt(tmp_data, key_schedule, dst);

		debug("sign_obj: block %d of %d\n", i, num_aes_blocks);
		debug_print_vector("AES-CMAC Src", AES_KEY_LENGTH, src);
		debug_print_vector("AES-CMAC Xor", AES_KEY_LENGTH, tmp_data);
		debug_print_vector("AES-CMAC Dst", AES_KEY_LENGTH, dst);

		/* Update pointers for next loop. */
		cbc_chain_data = dst;
		src += AES_KEY_LENGTH;
	}

	debug_print_vector("AES-CMAC Hash", AES_KEY_LENGTH, dst);
}

/**
 * Encrypt and sign a block of data (depending on security mode).
 *
 * \param key		Input AES key, length AES_KEY_LENGTH
 * \param oper		Security operations mask to perform (enum security_op)
 * \param src		Source data
 * \param length	Size of source data
 * \param sig_dst	Destination address for signature, AES_KEY_LENGTH bytes
 */
static int encrypt_and_sign(u8 *key, enum security_op oper, u8 *src,
			    u32 length, u8 *sig_dst)
{
	u32 num_aes_blocks;
	u8 key_schedule[AES_EXPAND_KEY_LENGTH];

	debug("encrypt_and_sign: length = %d\n", length);
	debug_print_vector("AES key", AES_KEY_LENGTH, key);

	/*
	 * The only need for a key is for signing/checksum purposes, so
	 * if not encrypting, expand a key of 0s.
	 */
	aes_expand_key(oper & SECURITY_ENCRYPT ? key : zero_key, key_schedule);

	num_aes_blocks = (length + AES_KEY_LENGTH - 1) / AES_KEY_LENGTH;

	if (oper & SECURITY_ENCRYPT) {
		/* Perform this in place, resulting in src being encrypted. */
		debug("encrypt_and_sign: begin encryption\n");
		encrypt_object(key_schedule, src, src, num_aes_blocks);
		debug("encrypt_and_sign: end encryption\n");
	}

	if (oper & SECURITY_SIGN) {
		/* encrypt the data, overwriting the result in signature. */
		debug("encrypt_and_sign: begin signing\n");
		sign_object(key, key_schedule, src, sig_dst, num_aes_blocks);
		debug("encrypt_and_sign: end signing\n");
	}

	return 0;
}

int sign_data_block(u8 *source, unsigned length, u8 *signature)
{
	return encrypt_and_sign(zero_key, SECURITY_SIGN, source,
				length, signature);
}

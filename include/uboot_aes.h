/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010 - 2011 NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _AES_REF_H_
#define _AES_REF_H_

#include <errno.h>

#ifdef USE_HOSTCC
/* Define compat stuff for use in fw_* tools. */
typedef unsigned char u8;
typedef unsigned int u32;
#define debug(...) do {} while (0)
#endif

/*
 * AES encryption library, with small code size, supporting only 128-bit AES
 *
 * AES is a stream cipher which works a block at a time, with each block
 * in this case being AES_BLOCK_LENGTH bytes.
 */

enum {
	AES_STATECOLS	= 4,	/* columns in the state & expanded key */
	AES128_KEYCOLS	= 4,	/* columns in a key for aes128 */
	AES192_KEYCOLS	= 6,	/* columns in a key for aes128 */
	AES256_KEYCOLS	= 8,	/* columns in a key for aes128 */
	AES128_ROUNDS	= 10,	/* rounds in encryption for aes128 */
	AES192_ROUNDS	= 12,	/* rounds in encryption for aes192 */
	AES256_ROUNDS	= 14,	/* rounds in encryption for aes256 */
	AES128_KEY_LENGTH	= 128 / 8,
	AES192_KEY_LENGTH	= 192 / 8,
	AES256_KEY_LENGTH	= 256 / 8,
	AES128_EXPAND_KEY_LENGTH = 4 * AES_STATECOLS * (AES128_ROUNDS + 1),
	AES192_EXPAND_KEY_LENGTH = 4 * AES_STATECOLS * (AES192_ROUNDS + 1),
	AES256_EXPAND_KEY_LENGTH = 4 * AES_STATECOLS * (AES256_ROUNDS + 1),
	AES_BLOCK_LENGTH	= 128 / 8,
};

/**
 * aes_expand_key() - Expand the AES key
 *
 * Expand a key into a key schedule, which is then used for the other
 * operations.
 *
 * @key		Key
 * @key_size	Size of the key (in bits)
 * @expkey	Buffer to place expanded key, AES_EXPAND_KEY_LENGTH
 */
void aes_expand_key(u8 *key, u32 key_size, u8 *expkey);

/**
 * aes_encrypt() - Encrypt single block of data with AES 128
 *
 * @key_size	Size of the aes key (in bits)
 * @in		Input data
 * @expkey	Expanded key to use for encryption (from aes_expand_key())
 * @out		Output data
 */
void aes_encrypt(u32 key_size, u8 *in, u8 *expkey, u8 *out);

/**
 * aes_decrypt() - Decrypt single block of data with AES 128
 *
 * @key_size	Size of the aes key (in bits)
 * @in		Input data
 * @expkey	Expanded key to use for decryption (from aes_expand_key())
 * @out		Output data
 */
void aes_decrypt(u32 key_size, u8 *in, u8 *expkey, u8 *out);

/**
 * Apply chain data to the destination using EOR
 *
 * Each array is of length AES_BLOCK_LENGTH.
 *
 * @cbc_chain_data	Chain data
 * @src			Source data
 * @dst			Destination data, which is modified here
 */
void aes_apply_cbc_chain_data(u8 *cbc_chain_data, u8 *src, u8 *dst);

/**
 * aes_cbc_encrypt_blocks() - Encrypt multiple blocks of data with AES CBC.
 *
 * @key_size		Size of the aes key (in bits)
 * @key_exp		Expanded key to use
 * @iv			Initialization vector
 * @src			Source data to encrypt
 * @dst			Destination buffer
 * @num_aes_blocks	Number of AES blocks to encrypt
 */
void aes_cbc_encrypt_blocks(u32 key_size, u8 *key_exp, u8 *iv, u8 *src, u8 *dst,
			    u32 num_aes_blocks);

/**
 * Decrypt multiple blocks of data with AES CBC.
 *
 * @key_size		Size of the aes key (in bits)
 * @key_exp		Expanded key to use
 * @iv			Initialization vector
 * @src			Source data to decrypt
 * @dst			Destination buffer
 * @num_aes_blocks	Number of AES blocks to decrypt
 */
void aes_cbc_decrypt_blocks(u32 key_size, u8 *key_exp, u8 *iv, u8 *src, u8 *dst,
			    u32 num_aes_blocks);

/* An AES block filled with zeros */
static const u8 AES_ZERO_BLOCK[AES_BLOCK_LENGTH] = { 0 };
struct udevice;

/**
 * struct struct aes_ops - Driver model for AES related operations
 *
 * The uclass interface is implemented by AES crypto devices which use driver model.
 *
 * Some AES crypto devices use key slots to store the key for the encrypt/decrypt
 * operations, while others may simply pass the key on each operation.
 *
 * In case the device does not implement hardware slots, driver can emulate or simply
 * store one active key slot at 0 in the driver state and pass it on each underlying
 * hw calls for AES operations.
 *
 * Note that some devices like Tegra AES engine may contain preloaded keys by bootrom,
 * thus in those cases the set_key_for_key_slot() may be skipped.
 *
 * Sequence for a series of AES CBC encryption, one decryption and a CMAC hash example
 * with 128bits key at slot 0 would be as follow:
 *
 * set_key_for_key_slot(DEV, 128, KEY, 0);
 * select_key_slot(DEV, 128, 0);
 * aes_cbc_encrypt(DEV, IV1, SRC1, DST1, LEN1);
 * aes_cbc_encrypt(DEV, IV2, SRC2, DST2, LEN2);
 * aes_cbc_decrypt(DEV, IV3, SRC3, DST3, LEN3);
 */
struct aes_ops {
	/**
	 * available_key_slots() - How many key slots this AES device has
	 *
	 * @dev			The AES udevice
	 * @return		Available slots to use, 0 for none
	 */
	int (*available_key_slots)(struct udevice *dev);

	/**
	 * select_key_slot() - Selects the AES key slot to use for following operations
	 *
	 * @dev			The AES udevice
	 * @key_size		Size of the aes key (in bits)
	 * @slot		The key slot to set as selected
	 * @return		0 on success, negative value on failure
	 */
	int (*select_key_slot)(struct udevice *dev, u32 key_size, u8 slot);

	/**
	 * set_key_for_key_slot() - Sets the AES key to use for specified key slot
	 *
	 * @dev			The AES udevice
	 * @key_size		Size of the aes key (in bits)
	 * @key			An AES key to set
	 * @slot		The slot to load the key at
	 * @return		0 on success, negative value on failure
	 */
	int (*set_key_for_key_slot)(struct udevice *dev, u32 key_size, u8 *key,
				    u8 slot);

	/**
	 * aes_ecb_encrypt() - Encrypt multiple blocks of data with AES ECB.
	 *
	 * @dev			The AES udevice
	 * @src			Source data of length 'num_aes_blocks' blocks
	 * @dst			Destination data of length 'num_aes_blocks' blocks
	 * @num_aes_blocks	Number of AES blocks to encrypt/decrypt
	 * @return		0 on success, negative value on failure
	 */
	int (*aes_ecb_encrypt)(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks);

	/**
	 * aes_ecb_decrypt() - Decrypt multiple blocks of data with AES ECB.
	 *
	 * @dev			The AES udevice
	 * @src			Source data of length 'num_aes_blocks' blocks
	 * @dst			Destination data of length 'num_aes_blocks' blocks
	 * @num_aes_blocks	Number of AES blocks to encrypt/decrypt
	 * @return		0 on success, negative value on failure
	 */
	int (*aes_ecb_decrypt)(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks);

	/**
	 * aes_cbc_encrypt() - Encrypt multiple blocks of data with AES CBC.
	 *
	 * @dev			The AES udevice
	 * @iv			Initialization vector
	 * @src			Source data of length 'num_aes_blocks' blocks
	 * @dst			Destination data of length 'num_aes_blocks' blocks
	 * @num_aes_blocks	Number of AES blocks to encrypt/decrypt
	 * @return		0 on success, negative value on failure
	 */
	int (*aes_cbc_encrypt)(struct udevice *dev, u8 *iv,
			       u8 *src, u8 *dst, u32 num_aes_blocks);

	/**
	 * aes_cbc_decrypt() - Decrypt multiple blocks of data with AES CBC.
	 *
	 * @dev			The AES udevice
	 * @iv			Initialization vector
	 * @src			Source data of length 'num_aes_blocks' blocks
	 * @dst			Destination data of length 'num_aes_blocks' blocks
	 * @num_aes_blocks	Number of AES blocks to encrypt/decrypt
	 * @return		0 on success, negative value on failure
	 */
	int (*aes_cbc_decrypt)(struct udevice *dev, u8 *iv,
			       u8 *src, u8 *dst, u32 num_aes_blocks);
};

#define aes_get_ops(dev)	((struct aes_ops *)(dev)->driver->ops)

#if CONFIG_IS_ENABLED(DM_AES)

/**
 * dm_aes_get_available_key_slots - How many key slots this AES device has
 *
 * @dev			The AES udevice
 * Return:		Available slots to use, 0 for none, -ve on failure
 */
int dm_aes_get_available_key_slots(struct udevice *dev);

/**
 * dm_aes_select_key_slot - Selects the AES key slot to use for following operations
 *
 * @dev			The AES udevice
 * @key_size		Size of the aes key (in bits)
 * @slot		The key slot to set as selected
 * Return:		0 on success, -ve on failure
 */
int dm_aes_select_key_slot(struct udevice *dev, u32 key_size, u8 slot);

/**
 * dm_aes_set_key_for_key_slot - Sets the AES key to use for specified key slot
 *
 * @dev			The AES udevice
 * @key_size		Size of the aes key (in bits)
 * @key			An AES key to set
 * @slot		The slot to load the key at
 * Return:		0 on success, negative value on failure
 */
int dm_aes_set_key_for_key_slot(struct udevice *dev, u32 key_size, u8 *key, u8 slot);

/**
 * dm_aes_ecb_encrypt - Encrypt multiple blocks of data with AES ECB.
 *
 * @dev			The AES udevice
 * @src			Source data of length 'num_aes_blocks' blocks
 * @dst			Destination data of length 'num_aes_blocks' blocks
 * @num_aes_blocks	Number of AES blocks to encrypt/decrypt
 * Return:		0 on success, negative value on failure
 */
int dm_aes_ecb_encrypt(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks);

/**
 * dm_aes_ecb_decrypt - Decrypt multiple blocks of data with AES ECB.
 *
 * @dev			The AES udevice
 * @src			Source data of length 'num_aes_blocks' blocks
 * @dst			Destination data of length 'num_aes_blocks' blocks
 * @num_aes_blocks	Number of AES blocks to encrypt/decrypt
 * Return:		0 on success, negative value on failure
 */
int dm_aes_ecb_decrypt(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks);

/**
 * dm_aes_cbc_encrypt - Encrypt multiple blocks of data with AES CBC.
 *
 * @dev			The AES udevice
 * @iv			Initialization vector
 * @src			Source data of length 'num_aes_blocks' blocks
 * @dst			Destination data of length 'num_aes_blocks' blocks
 * @num_aes_blocks	Number of AES blocks to encrypt/decrypt
 * Return:		0 on success, negative value on failure
 */
int dm_aes_cbc_encrypt(struct udevice *dev, u8 *iv, u8 *src, u8 *dst, u32 num_aes_blocks);

/**
 * dm_aes_cbc_decrypt - Decrypt multiple blocks of data with AES CBC.
 *
 * @dev			The AES udevice
 * @iv			Initialization vector
 * @src			Source data of length 'num_aes_blocks' blocks
 * @dst			Destination data of length 'num_aes_blocks' blocks
 * @num_aes_blocks	Number of AES blocks to encrypt/decrypt
 * Return:		0 on success, negative value on failure
 */
int dm_aes_cbc_decrypt(struct udevice *dev, u8 *iv, u8 *src, u8 *dst, u32 num_aes_blocks);

/**
 * dm_aes_cmac - Hashes the input data with AES-CMAC, putting the result into dst.
 * The key slot must be selected already.
 *
 * @dev			The AES udevice
 * @key_size		Size of the aes key (in bits)
 * @src			Source data of length 'num_aes_blocks' blocks
 * @dst			Destination for hash result
 * @num_aes_blocks	Number of AES blocks to encrypt
 * Return:		0 on success, negative value on failure.
 */
int dm_aes_cmac(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks);

#else

static inline int dm_aes_get_available_key_slots(struct udevice *dev)
{
	return -ENOSYS;
}

static inline int dm_aes_select_key_slot(struct udevice *dev, u32 key_size, u8 slot)
{
	return -ENOSYS;
}

static inline int dm_aes_set_key_for_key_slot(struct udevice *dev, u32 key_size, u8 *key,
					      u8 slot)
{
	return -ENOSYS;
}

static inline int dm_aes_ecb_encrypt(struct udevice *dev, u8 *src, u8 *dst,
				     u32 num_aes_blocks)
{
	return -ENOSYS;
}

static inline int dm_aes_ecb_decrypt(struct udevice *dev, u8 *src, u8 *dst,
				     u32 num_aes_blocks)
{
	return -ENOSYS;
}

static inline int dm_aes_cbc_encrypt(struct udevice *dev, u8 *iv, u8 *src,
				     u8 *dst, u32 num_aes_blocks)
{
	return -ENOSYS;
}

static inline int dm_aes_cbc_decrypt(struct udevice *dev, u8 *iv, u8 *src,
				     u8 *dst, u32 num_aes_blocks)
{
	return -ENOSYS;
}

static inline int dm_aes_cmac(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks)
{
	return -ENOSYS;
}

#endif /* CONFIG_DM_AES */

#endif /* _AES_REF_H_ */

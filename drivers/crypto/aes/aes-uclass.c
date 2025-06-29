// SPDX-License-Identifier: GPL-2.0+

#define LOG_CATEGORY UCLASS_AES

#include <dm.h>
#include <malloc.h>
#include <log.h>
#include <uboot_aes.h>
#include <linux/string.h>

int dm_aes_get_available_key_slots(struct udevice *dev)
{
	const struct aes_ops *ops;

	if (!dev)
		return -ENODEV;

	ops = aes_get_ops(dev);

	if (!ops->available_key_slots)
		return -ENOSYS;

	return ops->available_key_slots(dev);
}

int dm_aes_select_key_slot(struct udevice *dev, u32 key_size, u8 slot)
{
	const struct aes_ops *ops;

	if (!dev)
		return -ENODEV;

	ops = aes_get_ops(dev);

	if (!ops->select_key_slot)
		return -ENOSYS;

	return ops->select_key_slot(dev, key_size, slot);
}

int dm_aes_set_key_for_key_slot(struct udevice *dev, u32 key_size, u8 *key, u8 slot)
{
	const struct aes_ops *ops;

	if (!dev)
		return -ENODEV;

	ops = aes_get_ops(dev);

	if (!ops->set_key_for_key_slot)
		return -ENOSYS;

	return ops->set_key_for_key_slot(dev, key_size, key, slot);
}

int dm_aes_ecb_encrypt(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks)
{
	const struct aes_ops *ops;

	if (!dev)
		return -ENODEV;

	ops = aes_get_ops(dev);

	if (!ops->aes_ecb_encrypt)
		return -ENOSYS;

	return ops->aes_ecb_encrypt(dev, src, dst, num_aes_blocks);
}

int dm_aes_ecb_decrypt(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks)
{
	const struct aes_ops *ops;

	if (!dev)
		return -ENODEV;

	ops = aes_get_ops(dev);

	if (!ops->aes_ecb_decrypt)
		return -ENOSYS;

	return ops->aes_ecb_decrypt(dev, src, dst, num_aes_blocks);
}

int dm_aes_cbc_encrypt(struct udevice *dev, u8 *iv, u8 *src, u8 *dst, u32 num_aes_blocks)
{
	const struct aes_ops *ops;

	if (!dev)
		return -ENODEV;

	ops = aes_get_ops(dev);

	if (!ops->aes_cbc_encrypt)
		return -ENOSYS;

	return ops->aes_cbc_encrypt(dev, iv, src, dst, num_aes_blocks);
}

int dm_aes_cbc_decrypt(struct udevice *dev, u8 *iv, u8 *src, u8 *dst, u32 num_aes_blocks)
{
	const struct aes_ops *ops;

	if (!dev)
		return -ENODEV;

	ops = aes_get_ops(dev);

	if (!ops->aes_cbc_decrypt)
		return -ENOSYS;

	return ops->aes_cbc_decrypt(dev, iv, src, dst, num_aes_blocks);
}

static void left_shift_vector(u8 *in, u8 *out, int size)
{
	int carry = 0;
	int i;

	for (i = size - 1; i >= 0; i--) {
		out[i] = (in[i] << 1) | carry;
		carry = in[i] >> 7;	/* get most significant bit */
	}
}

int dm_aes_cmac(struct udevice *dev, u8 *src, u8 *dst, u32 num_aes_blocks)
{
	const u8 AES_CMAC_CONST_RB = 0x87; /* from RFC 4493, Figure 2.2 */
	const u32 TMP_BUFFER_LEN = 128;
	u8 tmp_block[AES128_KEY_LENGTH] = { };
	u8 k1[AES128_KEY_LENGTH];
	u8 *tmp_buffer;
	int ret;

	log_debug("%s: 0x%p -> %p blocks %d\n", __func__, src, dst, num_aes_blocks);

	if (!num_aes_blocks) {
		log_debug("%s: called with 0 blocks!\n", __func__);
		return -1;
	}

	/* Compute K1 constant needed by AES-CMAC calculation */
	ret = dm_aes_cbc_encrypt(dev, (u8 *)AES_ZERO_BLOCK, (u8 *)AES_ZERO_BLOCK, tmp_block, 1);
	if (ret)
		return -1;

	left_shift_vector(tmp_block, k1, AES_BLOCK_LENGTH);

	if ((tmp_block[0] >> 7) != 0) /* get MSB of L */
		k1[AES128_KEY_LENGTH - 1] ^= AES_CMAC_CONST_RB;

	/* Set what will be the initial IV as zero */
	memset(tmp_block, 0, AES_BLOCK_LENGTH);

	/* Process all blocks except last by calling engine several times per dma buffer size */
	if (num_aes_blocks > 1) {
		tmp_buffer = malloc(AES_BLOCK_LENGTH * min(num_aes_blocks - 1, TMP_BUFFER_LEN));
		while (num_aes_blocks > 1) {
			u32 blocks = min(num_aes_blocks - 1, TMP_BUFFER_LEN);

			/* Encrypt the current remaining set of blocks that fits in tmp buffer */
			ret = dm_aes_cbc_encrypt(dev, tmp_block, src, tmp_buffer, blocks);
			if (ret)
				return -1;

			num_aes_blocks -= blocks;
			src += blocks * AES_BLOCK_LENGTH;

			/* Copy the last encrypted block to tmp_block as IV */
			memcpy(tmp_block, tmp_buffer + ((blocks - 1) * AES_BLOCK_LENGTH),
			       AES_BLOCK_LENGTH);
		}
		free(tmp_buffer);
	}

	if (num_aes_blocks != 1) {
		log_debug("%s: left with %d blocks! must be 1\n", __func__, num_aes_blocks);
		return -1;
	}

	/* XOR last IV with K1 */
	aes_apply_cbc_chain_data(tmp_block, k1, tmp_block);

	/* Encrypt the last src block already with tmp_block as IV and output to dst */
	return dm_aes_cbc_encrypt(dev, tmp_block, src, dst, 1);
}

UCLASS_DRIVER(aes) = {
	.id	= UCLASS_AES,
	.name	= "aes",
};

// SPDX-License-Identifier: GPL-2.0+

#include <config.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <uboot_aes.h>

#define SW_KEY_SLOTS 2

struct sw_aes_priv {
	u8 key_slots[SW_KEY_SLOTS][AES256_KEY_LENGTH];
	u8 key_schedule[AES256_EXPAND_KEY_LENGTH];
	u8 selected_slot;
	u32 selected_key_size;
	bool key_expanded;
};

static int prepare_aes(struct sw_aes_priv *priv)
{
	if (!priv->selected_key_size) {
		log_debug("%s: AES key size not set, setup a slot first\n", __func__);
		return 1;
	}

	if (priv->key_expanded)
		return 0;

	priv->key_expanded = 1;

	aes_expand_key(priv->key_slots[priv->selected_slot], priv->selected_key_size,
		       priv->key_schedule);

	return 0;
}

static int sw_aes_ops_available_key_slots(struct udevice *dev)
{
	return SW_KEY_SLOTS;
}

static int sw_aes_ops_select_key_slot(struct udevice *dev, u32 key_size, u8 slot)
{
	struct sw_aes_priv *priv = dev_get_priv(dev);

	if (slot >= SW_KEY_SLOTS)
		return 1;

	priv->selected_slot = slot;
	priv->selected_key_size = key_size;
	priv->key_expanded = 0;

	return 0;
}

static int sw_aes_ops_set_key_for_key_slot(struct udevice *dev, u32 key_size,
					   u8 *key, u8 slot)
{
	struct sw_aes_priv *priv = dev_get_priv(dev);

	if (slot >= SW_KEY_SLOTS)
		return 1;

	memcpy(priv->key_slots[slot], key, key_size / 8);

	if (priv->selected_slot == slot)
		priv->selected_key_size = key_size;

	priv->key_expanded = 0;

	return 0;
}

static int sw_aes_ops_aes_ecb_encrypt(struct udevice *dev, u8 *src, u8 *dst,
				      u32 num_aes_blocks)
{
	struct sw_aes_priv *priv = dev_get_priv(dev);
	int ret;

	ret = prepare_aes(priv);
	if (ret)
		return ret;

	while (num_aes_blocks > 0) {
		aes_encrypt(priv->selected_key_size, src, priv->key_schedule, dst);
		num_aes_blocks -= 1;
		src += AES_BLOCK_LENGTH;
		dst += AES_BLOCK_LENGTH;
	}

	return 0;
}

static int sw_aes_ops_aes_ecb_decrypt(struct udevice *dev, u8 *src, u8 *dst,
				      u32 num_aes_blocks)
{
	struct sw_aes_priv *priv = dev_get_priv(dev);
	int ret;

	ret = prepare_aes(priv);
	if (ret)
		return ret;

	while (num_aes_blocks > 0) {
		aes_decrypt(priv->selected_key_size, src, priv->key_schedule, dst);
		num_aes_blocks -= 1;
		src += AES_BLOCK_LENGTH;
		dst += AES_BLOCK_LENGTH;
	}

	return 0;
}

static int sw_aes_ops_aes_cbc_encrypt(struct udevice *dev, u8 *iv, u8 *src,
				      u8 *dst, u32 num_aes_blocks)
{
	struct sw_aes_priv *priv = dev_get_priv(dev);
	int ret;

	ret = prepare_aes(priv);
	if (ret)
		return ret;

	aes_cbc_encrypt_blocks(priv->selected_key_size, priv->key_schedule, iv,
			       src, dst, num_aes_blocks);

	return 0;
}

static int sw_aes_ops_aes_cbc_decrypt(struct udevice *dev, u8 *iv, u8 *src,
				      u8 *dst, u32 num_aes_blocks)
{
	struct sw_aes_priv *priv = dev_get_priv(dev);
	int ret;

	ret = prepare_aes(priv);
	if (ret)
		return ret;

	aes_cbc_decrypt_blocks(priv->selected_key_size, priv->key_schedule,
			       iv, src, dst, num_aes_blocks);

	return 0;
}

static const struct aes_ops aes_ops_sw = {
	.available_key_slots = sw_aes_ops_available_key_slots,
	.select_key_slot = sw_aes_ops_select_key_slot,
	.set_key_for_key_slot = sw_aes_ops_set_key_for_key_slot,
	.aes_ecb_encrypt = sw_aes_ops_aes_ecb_encrypt,
	.aes_ecb_decrypt = sw_aes_ops_aes_ecb_decrypt,
	.aes_cbc_encrypt = sw_aes_ops_aes_cbc_encrypt,
	.aes_cbc_decrypt = sw_aes_ops_aes_cbc_decrypt,
};

static const struct udevice_id sw_aes_ids[] = {
	{ .compatible = "software-aes-engine" },
	{ }
};

U_BOOT_DRIVER(aes_sw) = {
	.name = "aes_sw",
	.id = UCLASS_AES,
	.of_match = sw_aes_ids,
	.ops = &aes_ops_sw,
	.priv_auto = sizeof(struct sw_aes_priv),
};

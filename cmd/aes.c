// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 * Copyright (C) 2025 Ion Agorria <ion@agorria.com>
 *
 * Command for AES-[128/192/256] operations.
 */

#include <command.h>
#include <uboot_aes.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <dm/uclass.h>
#include <dm/device.h>

u32 aes_get_key_len(char *command)
{
	u32 key_len = AES128_KEY_LENGTH;

	if (!strcmp(command, "aes.192"))
		key_len = AES192_KEY_LENGTH;
	else if (!strcmp(command, "aes.256"))
		key_len = AES256_KEY_LENGTH;

	return key_len;
}

int aes_get_driver(struct udevice **dev)
{
	int ret;

	ret = uclass_get_device(UCLASS_AES, 0, dev);
	if (ret) {
		printf("Failed to get AES driver: %d\n", ret);
		return ret;
	}

	return 0;
}

int cmd_aes_cbc_simple(int argc, char *const argv[], u32 key_len)
{
	uint32_t key_addr, iv_addr, src_addr, dst_addr, len;
	uint8_t *key_ptr, *iv_ptr, *src_ptr, *dst_ptr;
	u8 key_exp[AES256_EXPAND_KEY_LENGTH];
	u32 aes_blocks;
	int enc;

	if (argc != 7)
		return CMD_RET_USAGE;

	if (!strncmp(argv[1], "enc", 3))
		enc = 1;
	else if (!strncmp(argv[1], "dec", 3))
		enc = 0;
	else
		return CMD_RET_USAGE;

	key_addr = hextoul(argv[2], NULL);
	iv_addr = hextoul(argv[3], NULL);
	src_addr = hextoul(argv[4], NULL);
	dst_addr = hextoul(argv[5], NULL);
	len = hextoul(argv[6], NULL);

	key_ptr = (uint8_t *)map_sysmem(key_addr, key_len);
	iv_ptr = (uint8_t *)map_sysmem(iv_addr, 128 / 8);
	src_ptr = (uint8_t *)map_sysmem(src_addr, len);
	dst_ptr = (uint8_t *)map_sysmem(dst_addr, len);

	/* First we expand the key. */
	aes_expand_key(key_ptr, key_len, key_exp);

	/* Calculate the number of AES blocks to encrypt. */
	aes_blocks = DIV_ROUND_UP(len, AES_BLOCK_LENGTH);

	if (enc)
		aes_cbc_encrypt_blocks(key_len, key_exp, iv_ptr, src_ptr,
				       dst_ptr, aes_blocks);
	else
		aes_cbc_decrypt_blocks(key_len, key_exp, iv_ptr, src_ptr,
				       dst_ptr, aes_blocks);

	unmap_sysmem(key_ptr);
	unmap_sysmem(iv_ptr);
	unmap_sysmem(src_ptr);
	unmap_sysmem(dst_ptr);

	return CMD_RET_SUCCESS;
}

int cmd_aes_get_slots(void)
{
	struct udevice *dev;
	u8 slots;
	int ret;

	ret = aes_get_driver(&dev);
	if (ret)
		return ret;

	slots = dm_aes_get_available_key_slots(dev);
	printf("Available slots: %d\n", slots);

	return CMD_RET_SUCCESS;
}

int cmd_aes_set_key(int argc, char *const argv[], u32 key_len)
{
	struct udevice *dev;
	u32 key_addr, slot;
	u8 *key_ptr;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	ret = aes_get_driver(&dev);
	if (ret)
		return ret;

	key_addr = hextoul(argv[2], NULL);
	slot = hextoul(argv[3], NULL);

	key_ptr = (uint8_t *)map_sysmem(key_addr, key_len);

	ret = dm_aes_set_key_for_key_slot(dev, key_len * 8, key_ptr, slot);
	unmap_sysmem(key_ptr);
	if (ret) {
		printf("Unable to set key at slot: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

int cmd_aes_select_slot(int argc, char *const argv[], u32 key_len)
{
	struct udevice *dev;
	u32 slot;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	ret = aes_get_driver(&dev);
	if (ret)
		return ret;

	slot = hextoul(argv[2], NULL);

	ret = dm_aes_select_key_slot(dev, key_len * 8, slot);
	if (ret) {
		printf("Unable to select key slot: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

int cmd_aes_ecb(int argc, char *const argv[], u32 key_len)
{
	struct udevice *dev;
	u32 src_addr, dst_addr, len;
	u8 *src_ptr, *dst_ptr;
	u32 aes_blocks;
	int enc, ret;

	if (argc != 6)
		return CMD_RET_USAGE;

	ret = aes_get_driver(&dev);
	if (ret)
		return ret;

	if (!strncmp(argv[1], "enc", 3))
		enc = 1;
	else if (!strncmp(argv[1], "dec", 3))
		enc = 0;
	else
		return CMD_RET_USAGE;

	src_addr = hextoul(argv[3], NULL);
	dst_addr = hextoul(argv[4], NULL);
	len = hextoul(argv[5], NULL);

	src_ptr = (uint8_t *)map_sysmem(src_addr, len);
	dst_ptr = (uint8_t *)map_sysmem(dst_addr, len);

	/* Calculate the number of AES blocks to encrypt. */
	aes_blocks = DIV_ROUND_UP(len, AES_BLOCK_LENGTH);

	if (enc)
		ret = dm_aes_ecb_encrypt(dev, src_ptr, dst_ptr, aes_blocks);
	else
		ret = dm_aes_ecb_decrypt(dev, src_ptr, dst_ptr, aes_blocks);

	unmap_sysmem(src_ptr);
	unmap_sysmem(dst_ptr);

	if (ret) {
		printf("Unable to do ecb operation: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

int cmd_aes_cbc(int argc, char *const argv[], u32 key_len)
{
	struct udevice *dev;
	u32 iv_addr, src_addr, dst_addr, len;
	u8 *iv_ptr, *src_ptr, *dst_ptr;
	u32 aes_blocks;
	int enc, ret;

	if (argc != 7)
		return CMD_RET_USAGE;

	ret = aes_get_driver(&dev);
	if (ret)
		return ret;

	if (!strncmp(argv[1], "enc", 3))
		enc = 1;
	else if (!strncmp(argv[1], "dec", 3))
		enc = 0;
	else
		return CMD_RET_USAGE;

	iv_addr = hextoul(argv[3], NULL);
	src_addr = hextoul(argv[4], NULL);
	dst_addr = hextoul(argv[5], NULL);
	len = hextoul(argv[6], NULL);

	iv_ptr = (uint8_t *)map_sysmem(iv_addr, AES_BLOCK_LENGTH);
	src_ptr = (uint8_t *)map_sysmem(src_addr, len);
	dst_ptr = (uint8_t *)map_sysmem(dst_addr, len);

	/* Calculate the number of AES blocks to encrypt. */
	aes_blocks = DIV_ROUND_UP(len, AES_BLOCK_LENGTH);

	if (enc)
		ret = dm_aes_cbc_encrypt(dev, iv_ptr, src_ptr, dst_ptr, aes_blocks);
	else
		ret = dm_aes_cbc_decrypt(dev, iv_ptr, src_ptr, dst_ptr, aes_blocks);

	unmap_sysmem(iv_ptr);
	unmap_sysmem(src_ptr);
	unmap_sysmem(dst_ptr);

	if (ret) {
		printf("Unable to do cbc operation: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

/**
 * do_aes() - Handle the "aes" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_aes(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32 key_len;

	if (argc < 2)
		return CMD_RET_USAGE;

	key_len = aes_get_key_len(argv[0]);

	if (!strncmp(argv[1], "enc", 3) || !strncmp(argv[1], "dec", 3))
		return cmd_aes_cbc_simple(argc, argv, key_len);
	else if (CONFIG_IS_ENABLED(DM_AES) && !strncmp(argv[1], "get_slots", 9))
		return cmd_aes_get_slots();
	else if (CONFIG_IS_ENABLED(DM_AES) && !strncmp(argv[1], "set_key", 7))
		return cmd_aes_set_key(argc, argv, key_len);
	else if (CONFIG_IS_ENABLED(DM_AES) && !strncmp(argv[1], "select_slot", 11))
		return cmd_aes_select_slot(argc, argv, key_len);
	else if (CONFIG_IS_ENABLED(DM_AES) && !strncmp(argv[1], "ecb", 3))
		return cmd_aes_ecb(argc, argv, key_len);
	else if (CONFIG_IS_ENABLED(DM_AES) && !strncmp(argv[1], "cbc", 3))
		return cmd_aes_cbc(argc, argv, key_len);
	else
		return CMD_RET_USAGE;
}

/***************************************************/
U_BOOT_LONGHELP(aes,
	"[.128,.192,.256] enc key iv src dst len - CBC encrypt block of data $len bytes long\n"
	"                             at address $src using a key at address\n"
	"                             $key with initialization vector at address\n"
	"                             $iv. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"                             The $key and $iv must be 16 bytes long.\n"
	"aes [.128,.192,.256] dec key iv src dst len - CBC decrypt block of data $len bytes long\n"
	"                             at address $src using a key at address\n"
	"                             $key with initialization vector at address\n"
	"                             $iv. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"                             The $key and $iv must be 16 bytes long."

#if CONFIG_IS_ENABLED(DM_AES)
	"\n"
	"aes get_slots - Gives number of available key slots\n"
	"aes [.128,.192,.256] set_key key slot - Load key at address $key into the slot $slot\n"
	"aes [.128,.192,.256] select_slot slot - Select current active key slot\n"
	"aes [.128,.192,.256] ecb enc src dst len - ECB encrypt block of data $len bytes long\n"
	"                             at address $src using a key at current\n"
	"                             slot. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"aes [.128,.192,.256] ecb dec src dst len - ECB decrypt block of data $len bytes long\n"
	"                             at address $src using a key at current\n"
	"                             slot. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"aes [.128,.192,.256] cbc enc iv src dst len - CBC encrypt block of data $len bytes long\n"
	"                             at address $src using a key at current\n"
	"                             slot with initialization vector at address\n"
	"                             $iv. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"                             The $iv must be 16 bytes long.\n"
	"aes [.128,.192,.256] cbc dec iv src dst len - CBC decrypt block of data $len bytes long\n"
	"                             at address $src using a key at current\n"
	"                             slot with initialization vector at address\n"
	"                             $iv. Store the result at address $dst.\n"
	"                             The $len size must be multiple of 16 bytes.\n"
	"                             The $iv must be 16 bytes long."
#endif
);

U_BOOT_CMD(
	aes, 7, 1, do_aes,
	"AES 128/192/256 operations",
	aes_help_text
);

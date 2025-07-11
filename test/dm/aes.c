// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the driver model AES API
 *
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/test.h>
#include <uboot_aes.h>
#include <test/test.h>
#include <test/ut.h>

#define AES128_KEYSIZE		128

static int dm_test_aes(struct unit_test_state *uts)
{
	struct udevice *dev;
	u8 test_key[AES128_KEY_LENGTH] = { 0x63, 0x68, 0x69, 0x63, 0x6b, 0x65, 0x6e, 0x20,
					   0x74, 0x65, 0x72, 0x69, 0x79, 0x61, 0x6b, 0x69 };
	u8 test_iv[AES128_KEY_LENGTH] = { 0 };

	u8 test_input[AES_BLOCK_LENGTH] = { 0x49, 0x20, 0x77, 0x6f, 0x75, 0x6c, 0x64, 0x20,
					    0x6c, 0x69, 0x6b, 0x65, 0x20, 0x74, 0x68, 0x65 };
	u8 exp_output[AES_BLOCK_LENGTH] = { 0x97, 0x68, 0x72, 0x68, 0xd6, 0xec, 0xcc, 0xc0,
					    0xc0, 0x7b, 0x25, 0xe2, 0x5e, 0xcf, 0xe5, 0x84 };
	u8 exp_cmac[AES_BLOCK_LENGTH] = { 0xfc, 0x89, 0x20, 0xc8, 0x46, 0x97, 0xb1, 0x3d,
					  0x31, 0x2c, 0xc2, 0x49, 0x5c, 0x5a, 0x0b, 0x9f };
	u8 test_output[AES_BLOCK_LENGTH];

	ut_assertok(uclass_first_device_err(UCLASS_AES, &dev));

	/* software AES exposes 2 key slots */
	ut_asserteq(2, dm_aes_get_available_key_slots(dev));

	ut_assertok(dm_aes_select_key_slot(dev, AES128_KEYSIZE, 0));
	ut_assertok(dm_aes_set_key_for_key_slot(dev, AES128_KEYSIZE, test_key, 0));

	ut_assertok(dm_aes_ecb_encrypt(dev, test_input, test_output, 1));
	ut_assertok(memcmp(exp_output, test_output, 16));

	ut_assertok(dm_aes_ecb_decrypt(dev, test_output, test_output, 1));
	ut_assertok(memcmp(test_input, test_output, 16));

	ut_assertok(dm_aes_cbc_encrypt(dev, test_iv, test_input, test_output, 1));
	ut_assertok(memcmp(exp_output, test_output, 16));

	ut_assertok(dm_aes_cbc_decrypt(dev, test_iv, test_output, test_output, 1));
	ut_assertok(memcmp(test_input, test_output, 16));

	ut_assertok(dm_aes_cmac(dev, test_input, test_output, 1));
	ut_assertok(memcmp(exp_cmac, test_output, 16));

	return 0;
}

DM_TEST(dm_test_aes, UTF_SCAN_FDT);

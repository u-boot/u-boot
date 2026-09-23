// SPDX-License-Identifier: GPL-2.0+
/*
 * Functional tests for UCLASS_FFA  class
 *
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <u-boot/uuid.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

#include <efi.h>

/* test UUID */
#define TEST_SVC_UUID	"ed32d533-4209-99e6-2d72-cdd998a79cc0"
/* U-Boot default fw image namespace */
#define DEFAULT_FW_IMAGE_NAMESPACE "8c9f137e-91dc-427b-b2d6-b420faebaf2a"

#define UUID_SIZE 16

/* The UUID binary data (little-endian format) */
static const u8 ref_uuid_bin[UUID_SIZE] = {
	0x33, 0xd5, 0x32, 0xed,
	0x09, 0x42, 0xe6, 0x99,
	0x72, 0x2d, 0xc0, 0x9c,
	0xa7, 0x98, 0xd9, 0xcd
};

static int lib_test_uuid_to_le(struct unit_test_state *uts)
{
	const char *uuid_str = TEST_SVC_UUID;
	u8 ret_uuid_bin[UUID_SIZE] = {0};

	ut_assertok(uuid_str_to_le_bin(uuid_str, ret_uuid_bin));
	ut_asserteq_mem(ref_uuid_bin, ret_uuid_bin, UUID_SIZE);

	return 0;
}
LIB_TEST(lib_test_uuid_to_le, 0);

#if defined(CONFIG_RANDOM_UUID) || defined(CONFIG_CMD_UUID)
/* Test UUID attribute bits (version, variant) */
static int lib_test_uuid_bits(struct unit_test_state *uts)
{
	unsigned char uuid[16];
	efi_guid_t guid;
	int i;

	/*
	 * Reduce the chance of a randomly generated UUID disguising
	 * a regression by testing multiple times.
	 */
	for (i = 0; i < 5; i++) {
		/* Test UUID v4 */
		gen_rand_uuid((unsigned char *)&uuid);

		printf("v4 UUID: %pUb\n", (efi_guid_t *)uuid);

		/* version 4 */
		ut_assert((uuid[6] & 0xf0) == 0x40);
		/* variant 1 */
		ut_assert((uuid[8] & UUID_VARIANT_MASK) == (UUID_VARIANT << UUID_VARIANT_SHIFT));

		/* Test v5, use the v4 UUID as the namespace */
		gen_v5_guid((struct uuid *)uuid,
			    &guid, "test", 4, NULL);

		printf("v5 GUID: %pUl\n", (efi_guid_t *)uuid);

		/* This is a GUID so bits 6 and 7 are swapped (little endian). Version 5 */
		ut_assert((guid.b[7] & 0xf0) == 0x50);
		/* variant 1 */
		ut_assert((guid.b[8] & UUID_VARIANT_MASK) == (UUID_VARIANT << UUID_VARIANT_SHIFT));
	}

	return 0;
}

LIB_TEST(lib_test_uuid_bits, 0);
#endif

struct dynamic_uuid_test_data {
	const char *compatible;
	const char *images[4];
	const char *expected_uuids[4];
};

/**
 * bytes_to_null_pair() - offset of a pair of null bytes within a string
 *
 * Steps through the string two bytes at a time, looking for a null character,
 * which in UTF-16 is a pair of null bytes. The string is walked byte-wise
 * rather than as u16, since a string literal carries no alignment guarantee
 * and some architectures cannot load a halfword from an odd address.
 *
 * @str:	UTF-16LE string, terminated by a null character
 * Return:	number of bytes preceding the terminator
 */
static size_t bytes_to_null_pair(const char *str)
{
	size_t len = 0;

	while (str[len] || str[len + 1])
		len += 2;

	return len;
}

static int lib_test_dynamic_uuid_case(struct unit_test_state *uts,
				      const struct dynamic_uuid_test_data *data)
{
	struct uuid namespace;
	int j;

	ut_assertok(uuid_str_to_bin(DEFAULT_FW_IMAGE_NAMESPACE, (unsigned char *)&namespace,
				    UUID_STR_FORMAT_GUID));

	for (j = 0; data->images[j]; j++) {
		const char *expected_uuid = data->expected_uuids[j];
		const char *image = data->images[j];
		efi_guid_t uuid;
		char uuid_str[37];

		gen_v5_guid(&namespace, &uuid,
			    data->compatible, strlen(data->compatible),
			    image, bytes_to_null_pair(image),
			    NULL);
		uuid_bin_to_str((unsigned char *)&uuid, uuid_str, UUID_STR_FORMAT_GUID);

		ut_asserteq_str(expected_uuid, uuid_str);
	}

	return 0;
}

static int lib_test_dynamic_uuid(struct unit_test_state *uts)
{
	int ret, i;
	const struct dynamic_uuid_test_data test_data[] = {
		{
			.compatible = "sandbox",
			.images = {
				/* Forced UTF-16LE to keep these endianness agnostic */
				"S\0A\0N\0D\0B\0O\0X\0-\0U\0B\0O\0O\0T\0\0",
				"S\0A\0N\0D\0B\0O\0X\0-\0U\0B\0O\0O\0T\0-\0E\0N\0V\0\0",
				"S\0A\0N\0D\0B\0O\0X\0-\0F\0I\0T\0\0",
				NULL,
			},
			.expected_uuids = {
				"985f2937-7c2e-5e9a-8a5e-8e063312964b",
				"9e339473-c2eb-530a-a69b-0cd6bbbed40e",
				"46610520-469e-59dc-a8dd-c11832b877ea",
				NULL,
			}
		},
		{
			.compatible = "qcom,qrb4210-rb2",
			.images = {
				"Q\0U\0A\0L\0C\0O\0M\0M\0-\0U\0B\0O\0O\0T\0\0",
				NULL,
			},
			.expected_uuids = {
				"d5021fac-8dd0-5ed7-90c2-763c304aaf86",
				NULL,
			}
		},
	};

	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		ret = lib_test_dynamic_uuid_case(uts, &test_data[i]);
		if (ret)
			return ret;
	}

	return 0;
}

LIB_TEST(lib_test_dynamic_uuid, 0);

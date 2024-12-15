// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * Unit tests for sha256_hmac functions
 */

#include <command.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>
#include <u-boot/sha256.h>

struct test_sha256_hmac_s {
	unsigned char *key;
	int keylen;
	unsigned char *input;
	int ilen;
	unsigned char *expected;
};

/*
 * data comes from:
 * https://datatracker.ietf.org/doc/html/rfc4231
 */
static unsigned char key_test1[] = {
	0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b };

static unsigned char input_test1[] = {
	0x48, 0x69, 0x20, 0x54, 0x68, 0x65, 0x72, 0x65 };

static unsigned char expected_test1[] = {
	0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
	0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
	0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
	0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7 };

static unsigned char key_test2[] = { 0x4a, 0x65, 0x66, 0x65 };

static unsigned char input_test2[] = {
	0x77, 0x68, 0x61, 0x74, 0x20, 0x64, 0x6f, 0x20,
	0x79, 0x61, 0x20, 0x77, 0x61, 0x6e, 0x74, 0x20,
	0x66, 0x6f, 0x72, 0x20, 0x6e, 0x6f, 0x74, 0x68,
	0x69, 0x6e, 0x67, 0x3f };

static unsigned char expected_test2[] = {
	0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e,
	0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7,
	0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27, 0x39, 0x83,
	0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43 };

static struct test_sha256_hmac_s test_sha256_hmac[] = {
	{
		.key = key_test1,
		.keylen = sizeof(key_test1),
		.input = input_test1,
		.ilen = sizeof(input_test1),
		.expected = expected_test1,
	},
	{
		.key = key_test2,
		.keylen = sizeof(key_test2),
		.input = input_test2,
		.ilen = sizeof(input_test2),
		.expected = expected_test2,
	},
};

static int _lib_test_sha256_hmac_run(struct unit_test_state *uts,
				     unsigned char *key, int keylen,
				     unsigned char *input, int ilen,
				     unsigned char *expected)
{
	unsigned char output[32];

	sha256_hmac(key, keylen, input, ilen, output);
	ut_asserteq_mem(expected, output, 32);

	return 0;
}

static int lib_test_sha256_hmac_run(struct unit_test_state *uts,
				    struct test_sha256_hmac_s *test)
{
	unsigned char *key = test->key;
	int keylen = test->keylen;
	unsigned char *input = test->input;
	int ilen = test->ilen;
	unsigned char *expected = test->expected;

	return _lib_test_sha256_hmac_run(uts, key, keylen, input, ilen, expected);
}

static int lib_test_sha256_hmac(struct unit_test_state *uts)
{
	int i, ret = 0;

	for (i = 0; i < ARRAY_SIZE(test_sha256_hmac); i++) {
		ret = lib_test_sha256_hmac_run(uts, &test_sha256_hmac[i]);
		if (ret)
			break;
	}

	return ret;
}

LIB_TEST(lib_test_sha256_hmac, 0);

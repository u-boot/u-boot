// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * Unit tests for sha256_hkdf functions
 */

#include <command.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>
#include <u-boot/sha256.h>

struct test_sha256_hkdf_s {
	unsigned char *salt;
	int saltlen;
	unsigned char *ikm;
	int ikmlen;
	unsigned char *info;
	int infolen;
	unsigned char *expected;
	int expectedlen;
};

/*
 * data comes from:
 * https://www.rfc-editor.org/rfc/rfc5869
 */
static unsigned char salt_test1[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c };

static unsigned char ikm_test1[] = {
	0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b };

static unsigned char info_test1[] = {
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9 };

static unsigned char expected_test1[] = {
	0x3c, 0xb2, 0x5f, 0x25, 0xfa, 0xac, 0xd5, 0x7a,
	0x90, 0x43, 0x4f, 0x64, 0xd0, 0x36, 0x2f, 0x2a,
	0x2d, 0x2d, 0x0a, 0x90, 0xcf, 0x1a, 0x5a, 0x4c,
	0x5d, 0xb0, 0x2d, 0x56, 0xec, 0xc4, 0xc5, 0xbf,
	0x34, 0x00, 0x72, 0x08, 0xd5, 0xb8, 0x87, 0x18,
	0x58, 0x65 };

static struct test_sha256_hkdf_s test_sha256_hkdf[] = {
	{
		.salt = salt_test1,
		.saltlen = sizeof(salt_test1),
		.ikm = ikm_test1,
		.ikmlen = sizeof(ikm_test1),
		.info = info_test1,
		.infolen = sizeof(info_test1),
		.expected = expected_test1,
		.expectedlen = sizeof(expected_test1),
	},
};

static int _lib_test_sha256_hkdf_run(struct unit_test_state *uts,
				     unsigned char *salt, int saltlen,
				     unsigned char *ikm, int ikmlen,
				     unsigned char *info, int infolen,
				     unsigned char *expected, int expectedlen)
{
	unsigned char output[64];
	int ret;

	ut_assert(expectedlen <= sizeof(output));
	ret = sha256_hkdf(salt, saltlen, ikm, ikmlen, info, infolen, output, expectedlen);
	ut_assert(!ret);
	ut_asserteq_mem(expected, output, expectedlen);

	return 0;
}

static int lib_test_sha256_hkdf_run(struct unit_test_state *uts,
				    struct test_sha256_hkdf_s *test)
{
	unsigned char *salt = test->salt;
	int saltlen = test->saltlen;
	unsigned char *ikm = test->ikm;
	int ikmlen = test->ikmlen;
	unsigned char *info = test->info;
	int infolen = test->infolen;
	unsigned char *expected = test->expected;
	int expectedlen = test->expectedlen;

	return _lib_test_sha256_hkdf_run(uts, salt, saltlen, ikm, ikmlen,
					 info, infolen, expected, expectedlen);
}

static int lib_test_sha256_hkdf(struct unit_test_state *uts)
{
	int i, ret = 0;

	for (i = 0; i < ARRAY_SIZE(test_sha256_hkdf); i++) {
		ret = lib_test_sha256_hkdf_run(uts, &test_sha256_hkdf[i]);
		if (ret)
			break;
	}

	return ret;
}

LIB_TEST(lib_test_sha256_hkdf, 0);

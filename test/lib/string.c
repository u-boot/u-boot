// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Unit tests for memory functions
 *
 * The architecture dependent implementations run through different lines of
 * code depending on the alignment and length of memory regions copied or set.
 * This has to be considered in testing.
 */

#include <command.h>
#include <log.h>
#include <string.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

/* Xor mask used for marking memory regions */
#define MASK 0xA5
/* Number of different alignment values */
#define SWEEP 16
/* Allow for copying up to 32 bytes */
#define BUFLEN (SWEEP + 33)

#define TEST_STR	"hello"

/**
 * init_buffer() - initialize buffer
 *
 * The buffer is filled with incrementing values xor'ed with the mask.
 *
 * @buf:	buffer
 * @mask:	xor mask
 */
static void init_buffer(u8 buf[], u8 mask)
{
	int i;

	for (i = 0; i < BUFLEN; ++i)
		buf[i] = i ^ mask;
}

/**
 * test_memset() - test result of memset()
 *
 * @uts:	unit test state
 * @buf:	buffer
 * @mask:	value set by memset()
 * @offset:	relative start of region changed by memset() in buffer
 * @len:	length of region changed by memset()
 * Return:	0 = success, 1 = failure
 */
static int test_memset(struct unit_test_state *uts, u8 buf[], u8 mask,
		       int offset, int len)
{
	int i;

	for (i = 0; i < BUFLEN; ++i) {
		if (i < offset || i >= offset + len) {
			ut_asserteq(i, buf[i]);
		} else {
			ut_asserteq(mask, buf[i]);
		}
	}
	return 0;
}

/**
 * lib_memset() - unit test for memset()
 *
 * Test memset() with varied alignment and length of the changed buffer.
 *
 * @uts:	unit test state
 * Return:	0 = success, 1 = failure
 */
static int lib_memset(struct unit_test_state *uts)
{
	u8 buf[BUFLEN];
	int offset, len;
	void *ptr;

	for (offset = 0; offset <= SWEEP; ++offset) {
		for (len = 1; len < BUFLEN - SWEEP; ++len) {
			init_buffer(buf, 0);
			ptr = memset(buf + offset, MASK, len);
			ut_asserteq_ptr(buf + offset, (u8 *)ptr);
			if (test_memset(uts, buf, MASK, offset, len)) {
				debug("%s: failure %d, %d\n",
				      __func__, offset, len);
				return CMD_RET_FAILURE;
			}
		}
	}
	return 0;
}
LIB_TEST(lib_memset, 0);

/**
 * test_memmove() - test result of memcpy() or memmove()
 *
 * @uts:	unit test state
 * @buf:	buffer
 * @mask:	xor mask used to initialize source buffer
 * @offset1:	relative start of copied region in source buffer
 * @offset2:	relative start of copied region in destination buffer
 * @len:	length of region changed by memset()
 * Return:	0 = success, 1 = failure
 */
static int test_memmove(struct unit_test_state *uts, u8 buf[], u8 mask,
			int offset1, int offset2, int len)
{
	int i;

	for (i = 0; i < BUFLEN; ++i) {
		if (i < offset2 || i >= offset2 + len) {
			ut_asserteq(i, buf[i]);
		} else {
			ut_asserteq((i + offset1 - offset2) ^ mask, buf[i]);
		}
	}
	return 0;
}

/**
 * lib_memcpy() - unit test for memcpy()
 *
 * Test memcpy() with varied alignment and length of the copied buffer.
 *
 * @uts:	unit test state
 * Return:	0 = success, 1 = failure
 */
static int lib_memcpy(struct unit_test_state *uts)
{
	u8 buf1[BUFLEN];
	u8 buf2[BUFLEN];
	int offset1, offset2, len;
	void *ptr;

	init_buffer(buf1, MASK);

	for (offset1 = 0; offset1 <= SWEEP; ++offset1) {
		for (offset2 = 0; offset2 <= SWEEP; ++offset2) {
			for (len = 1; len < BUFLEN - SWEEP; ++len) {
				init_buffer(buf2, 0);
				ptr = memcpy(buf2 + offset2, buf1 + offset1,
					     len);
				ut_asserteq_ptr(buf2 + offset2, (u8 *)ptr);
				if (test_memmove(uts, buf2, MASK, offset1,
						 offset2, len)) {
					debug("%s: failure %d, %d, %d\n",
					      __func__, offset1, offset2, len);
					return CMD_RET_FAILURE;
				}
			}
		}
	}
	return 0;
}
LIB_TEST(lib_memcpy, 0);

/**
 * lib_memmove() - unit test for memmove()
 *
 * Test memmove() with varied alignment and length of the copied buffer.
 *
 * @uts:	unit test state
 * Return:	0 = success, 1 = failure
 */
static int lib_memmove(struct unit_test_state *uts)
{
	u8 buf[BUFLEN];
	int offset1, offset2, len;
	void *ptr;

	for (offset1 = 0; offset1 <= SWEEP; ++offset1) {
		for (offset2 = 0; offset2 <= SWEEP; ++offset2) {
			for (len = 1; len < BUFLEN - SWEEP; ++len) {
				init_buffer(buf, 0);
				ptr = memmove(buf + offset2, buf + offset1,
					      len);
				ut_asserteq_ptr(buf + offset2, (u8 *)ptr);
				if (test_memmove(uts, buf, 0, offset1, offset2,
						 len)) {
					debug("%s: failure %d, %d, %d\n",
					      __func__, offset1, offset2, len);
					return CMD_RET_FAILURE;
				}
			}
		}
	}
	return 0;
}
LIB_TEST(lib_memmove, 0);

/** lib_memdup() - unit test for memdup() */
static int lib_memdup(struct unit_test_state *uts)
{
	char buf[BUFLEN];
	size_t len;
	char *p, *q;

	/* Zero size should do nothing */
	p = memdup(NULL, 0);
	ut_assertnonnull(p);
	free(p);

	p = memdup(buf, 0);
	ut_assertnonnull(p);
	free(p);

	strcpy(buf, TEST_STR);
	len = sizeof(TEST_STR);
	p = memdup(buf, len);
	ut_asserteq_mem(p, buf, len);

	q = memdup(p, len);
	ut_asserteq_mem(q, buf, len);
	free(q);
	free(p);

	return 0;
}
LIB_TEST(lib_memdup, 0);

/** lib_strnstr() - unit test for strnstr() */
static int lib_strnstr(struct unit_test_state *uts)
{
	const char *s1 = "Itsy Bitsy Teenie Weenie";
	const char *s2 = "eenie";
	const char *s3 = "eery";

	ut_asserteq_ptr(&s1[12], strnstr(s1, s2, SIZE_MAX));
	ut_asserteq_ptr(&s1[12], strnstr(s1, s2, 17));
	ut_assertnull(strnstr(s1, s2, 16));
	ut_assertnull(strnstr(s1, s2, 0));
	ut_asserteq_ptr(&s1[13], strnstr(&s1[3], &s2[1], SIZE_MAX));
	ut_asserteq_ptr(&s1[13], strnstr(&s1[3], &s2[1], 14));
	ut_assertnull(strnstr(&s1[3], &s2[1], 13));
	ut_assertnull(strnstr(&s1[3], &s2[1], 0));
	ut_assertnull(strnstr(s1, s3, SIZE_MAX));
	ut_assertnull(strnstr(s1, s3, 0));

	return 0;
}
LIB_TEST(lib_strnstr, 0);

/** lib_strstr() - unit test for strstr() */
static int lib_strstr(struct unit_test_state *uts)
{
	const char *s1 = "Itsy Bitsy Teenie Weenie";
	const char *s2 = "eenie";
	const char *s3 = "easy";

	ut_asserteq_ptr(&s1[12], strstr(s1, s2));
	ut_asserteq_ptr(&s1[13], strstr(&s1[3], &s2[1]));
	ut_assertnull(strstr(s1, s3));
	ut_asserteq_ptr(&s1[2], strstr(s1, &s3[2]));
	ut_asserteq_ptr(&s1[8], strstr(&s1[5], &s3[2]));

	return 0;
}
LIB_TEST(lib_strstr, 0);

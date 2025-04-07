// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <membuf.h>
#include <os.h>
#include <rand.h>
#include <string.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

#define TEST_SIZE	16
#define TEST_COUNT	10000

static void membuf_zero(struct membuf *mb)
{
	memset(mb->start, '\0', mb->end - mb->start);
}

static int membuf_check(struct unit_test_state *uts, struct membuf *mb,
			int value)
{
	/* head is out of range */
	ut_assert(!(mb->head < mb->start || mb->head >= mb->end));

	/* tail is out of range */
	ut_assert(!(mb->tail < mb->start || mb->tail >= mb->end));

	return 0;
}

/* write from 1 to test_size bytes, and check they come back OK */
static int lib_test_membuf_one(struct unit_test_state *uts)
{
	char in[TEST_SIZE * 2], out[TEST_SIZE * 2];
	struct membuf mb;
	int size, ret, test_size, i;

	ut_assertok(membuf_new(&mb, TEST_SIZE));

	/* setup in test */
	for (i = 0; i < TEST_SIZE; i++) {
		in[i] = (i & 63) + '0';
		in[i + TEST_SIZE] = in[i];
	}

	test_size = TEST_SIZE;

	for (i = 1; i < TEST_COUNT; i++) {
		membuf_zero(&mb);
		size = rand() % test_size;

		// now write patterns and check they come back OK
		ret = membuf_put(&mb, in, 0);
		ret = membuf_put(&mb, in, size);
		ut_asserteq(size, ret);

		ret = membuf_put(&mb, in, 0);
		ut_assertok(membuf_check(uts, &mb, i));

		ret = membuf_get(&mb, out, 0);
		ret = membuf_get(&mb, out, size);
		ut_asserteq(size, ret);

		ret = membuf_get(&mb, out, 0);
		ut_assertok(membuf_check(uts, &mb, i));

		ut_asserteq_mem(in, out, size);
	}

	return 0;
}
LIB_TEST(lib_test_membuf_one, 0);

/* write random number of bytes, and check they come back OK */
static int lib_test_membuf_random(struct unit_test_state *uts)
{
	char in[TEST_SIZE * 2];
	char buf[TEST_SIZE * 2];
	struct membuf mb;
	int size, ret, test_size, i;
	char *inptr, *outptr;
	int max_avail, min_free;

	ut_assertok(membuf_new(&mb, TEST_SIZE));

	for (i = 0; i < TEST_SIZE; i++) {
		in[i] = (i & 63) + '0';
		in[i + TEST_SIZE] = in[i];
	}

	test_size = TEST_SIZE;

	inptr = in;
	outptr = in;
	min_free = TEST_COUNT;
	max_avail = 0;
	membuf_zero(&mb);
	for (i = 0; i < TEST_COUNT; i++) {
		size = rand() % test_size;

		if (membuf_free(&mb) < min_free)
			min_free = membuf_free(&mb);

		ret = membuf_put(&mb, inptr, size);
		ut_assertok(membuf_check(uts, &mb, i));
		inptr += ret;
		if (inptr >= in + TEST_SIZE)
			inptr -= TEST_SIZE;

		size = rand() % (test_size - 1);

		if (membuf_avail(&mb) > max_avail)
			max_avail = membuf_avail(&mb);

		ret = membuf_get(&mb, buf, size);
		ut_assertok(membuf_check(uts, &mb, i));
		ut_asserteq_mem(buf, outptr, ret);

		outptr += ret;
		if (outptr >= in + TEST_SIZE)
			outptr -= TEST_SIZE;
	}

	return 0;
}
LIB_TEST(lib_test_membuf_random, 0);

/* test membuf_extend() with split segments */
static int lib_test_membuf_extend(struct unit_test_state *uts)
{
	char in[TEST_SIZE * 2];
	char buf[TEST_SIZE * 2];
	struct membuf mb;
	int ret, test_size, i, cur;
	char *data;

	ut_assertok(membuf_new(&mb, TEST_SIZE));

	for (i = 0; i < TEST_SIZE; i++) {
		in[i] = (i & 63) + '0';
		in[i + TEST_SIZE] = in[i];
	}

	test_size = TEST_SIZE - 1;

	for (cur = 0; cur <= test_size; cur++) {
		ut_assertok(membuf_new(&mb, TEST_SIZE));

		membuf_zero(&mb);

		/*
		 * add some bytes, then remove them - this will force the membuf
		 * to have data split into two segments when we fill it
		 */
		ret = membuf_putraw(&mb, TEST_SIZE / 2, true, &data);
		membuf_getraw(&mb, ret, true, &data);
		ut_asserteq(TEST_SIZE / 2, ret);

		/* fill it */
		ret = membuf_put(&mb, in, cur);
		ut_assertok(membuf_check(uts, &mb, cur));
		ut_asserteq(cur, ret);

		/* extend the buffer */
		ut_assertok(membuf_extend_by(&mb, TEST_SIZE, -1));
		ut_assertok(membuf_check(uts, &mb, cur));

		/* check our data is still there */
		ret = membuf_get(&mb, buf, TEST_SIZE * 2);
		ut_assertok(membuf_check(uts, &mb, cur));
		ut_asserteq(cur, ret);
		ut_asserteq_mem(in, buf, cur);
		membuf_uninit(&mb);
	}

	return 0;
}
LIB_TEST(lib_test_membuf_extend, 0);

/* test membuf_readline() with generated data */
static int lib_test_membuf_readline(struct unit_test_state *uts)
{
	char *buf;
	int size, cur, i, ret, readptr, cmpptr;
	struct membuf mb;
	char *data;
	char str[256];
	char *s;

	ut_assertok(membuf_new(&mb, 1024));
	membuf_zero(&mb);

	/* Use the README as test data */
	ut_assertok(os_read_file("README", (void **)&buf, &size));

	cur = 0;
	readptr = 0;
	cmpptr = 0;
	for (i = 0; i < 100000; i++, cur += 1) {
		/* fill the buffer with up to 'cur' bytes */
		ret = membuf_putraw(&mb, cur, false, &data);

		if (ret > 0) {
			int can_read = min(ret, size - readptr);

			memcpy(data, &buf[readptr], can_read);
			readptr += can_read;

			membuf_putraw(&mb, can_read, true, &data);
			ut_assertok(membuf_check(uts, &mb, i));
		}

		/* read a line and compare */
		ret = membuf_readline(&mb, str, 256, 0, true);
		ut_assertok(membuf_check(uts, &mb, i));
		if (ret) {
			char *ptr;

			s = &buf[cmpptr];
			ptr = strchr(s, '\n');
			*ptr = '\0';

			ut_asserteq_str(s, str);
			cmpptr += strlen(s) + 1;
			*ptr = '\n';
		} else {
			ut_assert(membuf_free(&mb));
		}
	}
	membuf_dispose(&mb);
	os_free(buf);

	return 0;
}
LIB_TEST(lib_test_membuf_readline, 0);

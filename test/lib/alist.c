// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <alist.h>
#include <string.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

struct my_struct {
	uint val;
	uint other_val;
};

enum {
	obj_size	= sizeof(struct my_struct),
};

/* Test alist_init() */
static int lib_test_alist_init(struct unit_test_state *uts)
{
	struct alist lst;
	ulong start;

	start = ut_check_free();

	/* with a size of 0, the fields should be inited, with no memory used */
	memset(&lst, '\xff', sizeof(lst));
	ut_assert(alist_init_struct(&lst, struct my_struct));
	ut_asserteq_ptr(NULL, lst.data);
	ut_asserteq(0, lst.count);
	ut_asserteq(0, lst.alloc);
	ut_assertok(ut_check_delta(start));
	alist_uninit(&lst);
	ut_asserteq_ptr(NULL, lst.data);
	ut_asserteq(0, lst.count);
	ut_asserteq(0, lst.alloc);

	/* use an impossible size */
	ut_asserteq(false, alist_init(&lst, obj_size,
				      CONFIG_SYS_MALLOC_LEN));
	ut_assertnull(lst.data);
	ut_asserteq(0, lst.count);
	ut_asserteq(0, lst.alloc);

	/* use a small size */
	ut_assert(alist_init(&lst, obj_size, 4));
	ut_assertnonnull(lst.data);
	ut_asserteq(0, lst.count);
	ut_asserteq(4, lst.alloc);

	/* free it */
	alist_uninit(&lst);
	ut_asserteq_ptr(NULL, lst.data);
	ut_asserteq(0, lst.count);
	ut_asserteq(0, lst.alloc);
	ut_assertok(ut_check_delta(start));

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_init, 0);

/* Test alist_get() and alist_getd() */
static int lib_test_alist_get(struct unit_test_state *uts)
{
	struct alist lst;
	ulong start;
	void *ptr;

	start = ut_check_free();

	ut_assert(alist_init(&lst, obj_size, 3));
	ut_asserteq(0, lst.count);
	ut_asserteq(3, lst.alloc);

	ut_assertnull(alist_get_ptr(&lst, 2));
	ut_assertnull(alist_get_ptr(&lst, 3));

	ptr = alist_ensure_ptr(&lst, 1);
	ut_assertnonnull(ptr);
	ut_asserteq(2, lst.count);
	ptr = alist_ensure_ptr(&lst, 2);
	ut_asserteq(3, lst.count);
	ut_assertnonnull(ptr);

	ptr = alist_ensure_ptr(&lst, 3);
	ut_assertnonnull(ptr);
	ut_asserteq(4, lst.count);
	ut_asserteq(6, lst.alloc);

	ut_assertnull(alist_get_ptr(&lst, 4));

	alist_uninit(&lst);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_get, 0);

/* Test alist_has() */
static int lib_test_alist_has(struct unit_test_state *uts)
{
	struct alist lst;
	ulong start;
	void *ptr;

	start = ut_check_free();

	ut_assert(alist_init(&lst, obj_size, 3));

	ut_assert(!alist_has(&lst, 0));
	ut_assert(!alist_has(&lst, 1));
	ut_assert(!alist_has(&lst, 2));
	ut_assert(!alist_has(&lst, 3));

	/* create a new one to force expansion */
	ptr = alist_ensure_ptr(&lst, 4);
	ut_assertnonnull(ptr);

	ut_assert(alist_has(&lst, 0));
	ut_assert(alist_has(&lst, 1));
	ut_assert(alist_has(&lst, 2));
	ut_assert(alist_has(&lst, 3));
	ut_assert(alist_has(&lst, 4));
	ut_assert(!alist_has(&lst, 5));

	alist_uninit(&lst);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_has, 0);

/* Test alist_ensure() */
static int lib_test_alist_ensure(struct unit_test_state *uts)
{
	struct my_struct *ptr3, *ptr4;
	struct alist lst;
	ulong start;

	start = ut_check_free();

	ut_assert(alist_init_struct(&lst, struct my_struct));
	ut_asserteq(obj_size, lst.obj_size);
	ut_asserteq(0, lst.count);
	ut_asserteq(0, lst.alloc);
	ptr3 = alist_ensure_ptr(&lst, 3);
	ut_asserteq(4, lst.count);
	ut_asserteq(4, lst.alloc);
	ut_assertnonnull(ptr3);
	ptr3->val = 3;

	ptr4 = alist_ensure_ptr(&lst, 4);
	ut_asserteq(8, lst.alloc);
	ut_asserteq(5, lst.count);
	ut_assertnonnull(ptr4);
	ptr4->val = 4;
	ut_asserteq(4, alist_get(&lst, 4, struct my_struct)->val);

	ut_asserteq_ptr(ptr4, alist_ensure(&lst, 4, struct my_struct));

	alist_ensure(&lst, 4, struct my_struct)->val = 44;
	ut_asserteq(44, alist_get(&lst, 4, struct my_struct)->val);
	ut_asserteq(3, alist_get(&lst, 3, struct my_struct)->val);
	ut_assertnull(alist_get(&lst, 7, struct my_struct));
	ut_asserteq(8, lst.alloc);
	ut_asserteq(5, lst.count);

	/* add some more, checking handling of malloc() failure */
	malloc_enable_testing(0);
	ut_assertnonnull(alist_ensure(&lst, 7, struct my_struct));
	ut_assertnull(alist_ensure(&lst, 8, struct my_struct));
	malloc_disable_testing();

	lst.flags &= ~ALISTF_FAIL;
	ut_assertnonnull(alist_ensure(&lst, 8, struct my_struct));
	ut_asserteq(16, lst.alloc);
	ut_asserteq(9, lst.count);

	alist_uninit(&lst);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_ensure, 0);

/* Test alist_add() bits not tested by lib_test_alist_ensure() */
static int lib_test_alist_add(struct unit_test_state *uts)
{
	struct my_struct data, *ptr, *ptr2;
	const struct my_struct *chk;
	struct alist lst;
	ulong start;

	start = ut_check_free();

	ut_assert(alist_init_struct(&lst, struct my_struct));

	data.val = 123;
	data.other_val = 456;
	ptr = alist_add(&lst, data);
	ut_assertnonnull(ptr);
	ut_asserteq(4, lst.alloc);
	ut_asserteq(1, lst.count);

	ut_asserteq(123, ptr->val);
	ut_asserteq(456, ptr->other_val);

	ptr2 = alist_add_placeholder(&lst);
	ut_assertnonnull(ptr2);

	ptr2->val = 321;
	ptr2->other_val = 654;

	chk = alist_get(&lst, 1, struct my_struct);
	ut_asserteq(321, chk->val);
	ut_asserteq(654, chk->other_val);

	ptr2 = alist_getw(&lst, 1, struct my_struct);
	ut_asserteq(321, ptr2->val);
	ut_asserteq(654, ptr2->other_val);

	alist_uninit(&lst);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_add, 0);

/* Test alist_next()  */
static int lib_test_alist_next(struct unit_test_state *uts)
{
	const struct my_struct *ptr;
	struct my_struct data, *ptr2;
	struct alist lst;
	ulong start;

	start = ut_check_free();

	ut_assert(alist_init_struct(&lst, struct my_struct));
	data.val = 123;
	data.other_val = 0;
	alist_add(&lst, data);

	data.val = 321;
	alist_add(&lst, data);

	data.val = 789;
	alist_add(&lst, data);

	ptr = alist_get(&lst, 0, struct my_struct);
	ut_assertnonnull(ptr);
	ut_asserteq(123, ptr->val);

	ptr = alist_next(&lst, ptr);
	ut_assertnonnull(ptr);
	ut_asserteq(321, ptr->val);

	ptr2 = (struct my_struct *)ptr;
	ptr2 = alist_nextw(&lst, ptr2);
	ut_assertnonnull(ptr2);

	ptr = alist_next(&lst, ptr);
	ut_assertnonnull(ptr);
	ut_asserteq(789, ptr->val);
	ut_asserteq_ptr(ptr, ptr2);
	ptr2->val = 89;
	ut_asserteq(89, ptr->val);

	ptr = alist_next(&lst, ptr);
	ut_assertnull(ptr);

	alist_uninit(&lst);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_next, 0);

/* Test alist_for_each()  */
static int lib_test_alist_for_each(struct unit_test_state *uts)
{
	const struct my_struct *ptr;
	struct my_struct data, *ptr2;
	struct alist lst;
	ulong start;
	int sum;

	start = ut_check_free();

	ut_assert(alist_init_struct(&lst, struct my_struct));
	ut_asserteq_ptr(NULL, alist_end(&lst, struct my_struct));

	sum = 0;
	alist_for_each(ptr, &lst)
		sum++;
	ut_asserteq(0, sum);

	alist_for_each(ptr, &lst)
		sum++;
	ut_asserteq(0, sum);

	/* add three items */
	data.val = 1;
	data.other_val = 0;
	alist_add(&lst, data);

	ptr = lst.data;
	ut_asserteq_ptr(ptr + 1, alist_end(&lst, struct my_struct));

	data.val = 2;
	alist_add(&lst, data);
	ut_asserteq_ptr(ptr + 2, alist_end(&lst, struct my_struct));

	data.val = 3;
	alist_add(&lst, data);
	ut_asserteq_ptr(ptr + 3, alist_end(&lst, struct my_struct));

	/* check alist_chk_ptr() */
	ut_asserteq(true, alist_chk_ptr(&lst, ptr + 2));
	ut_asserteq(false, alist_chk_ptr(&lst, ptr + 3));
	ut_asserteq(false, alist_chk_ptr(&lst, ptr + 4));
	ut_asserteq(true, alist_chk_ptr(&lst, ptr));
	ut_asserteq(false, alist_chk_ptr(&lst, ptr - 1));

	/* sum all items */
	sum = 0;
	alist_for_each(ptr, &lst)
		sum += ptr->val;
	ut_asserteq(6, sum);

	/* increment all items */
	alist_for_each(ptr2, &lst)
		ptr2->val += 1;

	/* sum all items again */
	sum = 0;
	alist_for_each(ptr, &lst)
		sum += ptr->val;
	ut_asserteq(9, sum);

	ptr = lst.data;
	ut_asserteq_ptr(ptr + 3, alist_end(&lst, struct my_struct));

	/* empty the list and try again */
	alist_empty(&lst);
	ut_asserteq_ptr(ptr, alist_end(&lst, struct my_struct));
	ut_assertnull(alist_get(&lst, 0, struct my_struct));

	sum = 0;
	alist_for_each(ptr, &lst)
		sum += ptr->val;
	ut_asserteq(0, sum);

	alist_uninit(&lst);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_for_each, 0);

/* Test alist_empty()  */
static int lib_test_alist_empty(struct unit_test_state *uts)
{
	struct my_struct data;
	struct alist lst;
	ulong start;

	start = ut_check_free();

	ut_assert(alist_init_struct(&lst, struct my_struct));
	ut_asserteq(0, lst.count);
	data.val = 1;
	data.other_val = 0;
	alist_add(&lst, data);
	ut_asserteq(1, lst.count);
	ut_asserteq(4, lst.alloc);

	alist_empty(&lst);
	ut_asserteq(0, lst.count);
	ut_asserteq(4, lst.alloc);
	ut_assertnonnull(lst.data);
	ut_asserteq(sizeof(data), lst.obj_size);

	alist_uninit(&lst);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_empty, 0);

static int lib_test_alist_filter(struct unit_test_state *uts)
{
	struct my_struct *from, *to, *ptr;
	struct my_struct data;
	struct alist lst;
	ulong start;
	int count;

	start = ut_check_free();

	ut_assert(alist_init_struct(&lst, struct my_struct));
	data.val = 1;
	data.other_val = 0;
	alist_add(&lst, data);

	data.val = 2;
	alist_add(&lst, data);

	data.val = 3;
	alist_add(&lst, data);
	ptr = lst.data;

	/* filter out all values except 2 */
	alist_for_each_filter(from, to, &lst) {
		if (from->val != 2)
			*to++ = *from;
	}
	alist_update_end(&lst, to);

	ut_asserteq(2, lst.count);
	ut_assertnonnull(lst.data);

	ut_asserteq(1, alist_get(&lst, 0, struct my_struct)->val);
	ut_asserteq(3, alist_get(&lst, 1, struct my_struct)->val);
	ut_asserteq_ptr(ptr + 3, from);
	ut_asserteq_ptr(ptr + 2, to);

	/* filter out nothing */
	alist_for_each_filter(from, to, &lst) {
		if (from->val != 2)
			*to++ = *from;
	}
	alist_update_end(&lst, to);
	ut_asserteq_ptr(ptr + 2, from);
	ut_asserteq_ptr(ptr + 2, to);

	ut_asserteq(2, lst.count);
	ut_assertnonnull(lst.data);

	ut_asserteq(1, alist_get(&lst, 0, struct my_struct)->val);
	ut_asserteq(3, alist_get(&lst, 1, struct my_struct)->val);

	/* filter out everything */
	alist_for_each_filter(from, to, &lst) {
		if (from->val == 2)
			*to++ = *from;
	}
	alist_update_end(&lst, to);
	ut_asserteq_ptr(ptr + 2, from);
	ut_asserteq_ptr(ptr, to);

	/* filter out everything (nop) */
	count = 0;
	alist_for_each_filter(from, to, &lst) {
		if (from->val == 2)
			*to++ = *from;
		count++;
	}
	alist_update_end(&lst, to);
	ut_asserteq_ptr(ptr, from);
	ut_asserteq_ptr(ptr, to);
	ut_asserteq(0, count);

	ut_asserteq(0, lst.count);
	ut_assertnonnull(lst.data);

	alist_uninit(&lst);

	/* Check for memory leaks */
	ut_assertok(ut_check_delta(start));

	return 0;
}
LIB_TEST(lib_test_alist_filter, 0);

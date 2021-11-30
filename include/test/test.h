/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc.
 */

#ifndef __TEST_TEST_H
#define __TEST_TEST_H

#include <malloc.h>
#include <linux/bitops.h>

/*
 * struct unit_test_state - Entire state of test system
 *
 * @fail_count: Number of tests that failed
 * @start: Store the starting mallinfo when doing leak test
 * @of_live: true to use livetree if available, false to use flattree
 * @of_root: Record of the livetree root node (used for setting up tests)
 * @root: Root device
 * @testdev: Test device
 * @force_fail_alloc: Force all memory allocs to fail
 * @skip_post_probe: Skip uclass post-probe processing
 * @expect_str: Temporary string used to hold expected string value
 * @actual_str: Temporary string used to hold actual string value
 */
struct unit_test_state {
	int fail_count;
	struct mallinfo start;
	struct device_node *of_root;
	bool of_live;
	struct udevice *root;
	struct udevice *testdev;
	int force_fail_alloc;
	int skip_post_probe;
	char expect_str[512];
	char actual_str[512];
};

/* Test flags for each test */
enum {
	UT_TESTF_SCAN_PDATA	= BIT(0),	/* test needs platform data */
	UT_TESTF_PROBE_TEST	= BIT(1),	/* probe test uclass */
	UT_TESTF_SCAN_FDT	= BIT(2),	/* scan device tree */
	UT_TESTF_FLAT_TREE	= BIT(3),	/* test needs flat DT */
	UT_TESTF_LIVE_TREE	= BIT(4),	/* needs live device tree */
	UT_TESTF_CONSOLE_REC	= BIT(5),	/* needs console recording */
	/* do extra driver model init and uninit */
	UT_TESTF_DM		= BIT(6),
};

/**
 * struct unit_test - Information about a unit test
 *
 * @name: Name of test
 * @func: Function to call to perform test
 * @flags: Flags indicated pre-conditions for test
 */
struct unit_test {
	const char *file;
	const char *name;
	int (*func)(struct unit_test_state *state);
	int flags;
};

/**
 * UNIT_TEST() - create linker generated list entry for unit a unit test
 *
 * The macro UNIT_TEST() is used to create a linker generated list entry. These
 * list entries are enumerate tests that can be execute using the ut command.
 * The list entries are used both by the implementation of the ut command as
 * well as in a related Python test.
 *
 * For Python testing the subtests are collected in Python function
 * generate_ut_subtest() by applying a regular expression to the lines of file
 * u-boot.sym. The list entries have to follow strict naming conventions to be
 * matched by the expression.
 *
 * Use UNIT_TEST(foo_test_bar, _flags, foo_test) for a test bar in test suite
 * foo that can be executed via command 'ut foo bar' and is implemented in
 * function foo_test_bar().
 *
 * @_name:	concatenation of name of the test suite, "_test_", and the name
 *		of the test
 * @_flags:	an integer field that can be evaluated by the test suite
 *		implementation
 * @_suite:	name of the test suite concatenated with "_test"
 */
#define UNIT_TEST(_name, _flags, _suite)				\
	ll_entry_declare(struct unit_test, _name, ut_ ## _suite) = {	\
		.file = __FILE__,					\
		.name = #_name,						\
		.flags = _flags,					\
		.func = _name,						\
	}

/* Get the start of a list of unit tests for a particular suite */
#define UNIT_TEST_SUITE_START(_suite) \
	ll_entry_start(struct unit_test, ut_ ## _suite)
#define UNIT_TEST_SUITE_COUNT(_suite) \
	ll_entry_count(struct unit_test, ut_ ## _suite)

/* Use ! and ~ so that all tests will be sorted between these two values */
#define UNIT_TEST_ALL_START()	ll_entry_start(struct unit_test, ut_!)
#define UNIT_TEST_ALL_END()	ll_entry_start(struct unit_test, ut_~)
#define UNIT_TEST_ALL_COUNT()	(UNIT_TEST_ALL_END() - UNIT_TEST_ALL_START())

/* Sizes for devres tests */
enum {
	TEST_DEVRES_SIZE	= 100,
	TEST_DEVRES_COUNT	= 10,
	TEST_DEVRES_TOTAL	= TEST_DEVRES_SIZE * TEST_DEVRES_COUNT,

	/* A few different sizes */
	TEST_DEVRES_SIZE2	= 15,
	TEST_DEVRES_SIZE3	= 37,
};

/**
 * testbus_get_clear_removed() - Test function to obtain removed device
 *
 * This is used in testbus to find out which device was removed. Calling this
 * function returns a pointer to the device and then clears it back to NULL, so
 * that a future test can check it.
 */
struct udevice *testbus_get_clear_removed(void);

static inline void arch_reset_for_test(void)
{
#ifdef CONFIG_SANDBOX
#include <asm/state.h>

	state_reset_for_test(state_get_current());
#endif
}

#endif /* __TEST_TEST_H */

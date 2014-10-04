/*
 * Copyright (c) 2013 Google, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DM_TEST_H
#define __DM_TEST_H

#include <dm.h>
#include <malloc.h>

/**
 * struct dm_test_cdata - configuration data for test instance
 *
 * @ping_add: Amonut to add each time we get a ping
 * @base: Base address of this device
 */
struct dm_test_pdata {
	int ping_add;
	uint32_t base;
};

/**
 * struct test_ops - Operations supported by the test device
 *
 * @ping: Ping operation
 *	@dev: Device to operate on
 *	@pingval: Value to ping the device with
 *	@pingret: Returns resulting value from driver
 *	@return 0 if OK, -ve on error
 */
struct test_ops {
	int (*ping)(struct udevice *dev, int pingval, int *pingret);
};

/* Operations that our test driver supports */
enum {
	DM_TEST_OP_BIND = 0,
	DM_TEST_OP_UNBIND,
	DM_TEST_OP_PROBE,
	DM_TEST_OP_REMOVE,

	/* For uclass */
	DM_TEST_OP_POST_BIND,
	DM_TEST_OP_PRE_UNBIND,
	DM_TEST_OP_POST_PROBE,
	DM_TEST_OP_PRE_REMOVE,
	DM_TEST_OP_INIT,
	DM_TEST_OP_DESTROY,

	DM_TEST_OP_COUNT,
};

/* Test driver types */
enum {
	DM_TEST_TYPE_FIRST = 0,
	DM_TEST_TYPE_SECOND,
};

/* The number added to the ping total on each probe */
#define DM_TEST_START_TOTAL	5

/**
 * struct dm_test_priv - private data for the test devices
 */
struct dm_test_priv {
	int ping_total;
	int op_count[DM_TEST_OP_COUNT];
};

/**
 * struct dm_test_perdev_class_priv - private per-device data for test uclass
 */
struct dm_test_uclass_perdev_priv {
	int base_add;
};

/**
 * struct dm_test_uclass_priv - private data for test uclass
 */
struct dm_test_uclass_priv {
	int total_add;
};

/**
 * struct dm_test_parent_data - parent's information on each child
 *
 * @sum: Test value used to check parent data works correctly
 * @flag: Used to track calling of parent operations
 */
struct dm_test_parent_data {
	int sum;
	int flag;
};

/*
 * Operation counts for the test driver, used to check that each method is
 * called correctly
 */
extern int dm_testdrv_op_count[DM_TEST_OP_COUNT];

extern struct dm_test_state global_test_state;

/*
 * struct dm_test_state - Entire state of dm test system
 *
 * This is often abreviated to dms.
 *
 * @root: Root device
 * @testdev: Test device
 * @fail_count: Number of tests that failed
 * @force_fail_alloc: Force all memory allocs to fail
 * @skip_post_probe: Skip uclass post-probe processing
 * @removed: Used to keep track of a device that was removed
 */
struct dm_test_state {
	struct udevice *root;
	struct udevice *testdev;
	int fail_count;
	int force_fail_alloc;
	int skip_post_probe;
	struct udevice *removed;
	struct mallinfo start;
};

/* Test flags for each test */
enum {
	DM_TESTF_SCAN_PDATA	= 1 << 0,	/* test needs platform data */
	DM_TESTF_PROBE_TEST	= 1 << 1,	/* probe test uclass */
	DM_TESTF_SCAN_FDT	= 1 << 2,	/* scan device tree */
};

/**
 * struct dm_test - Information about a driver model test
 *
 * @name: Name of test
 * @func: Function to call to perform test
 * @flags: Flags indicated pre-conditions for test
 */
struct dm_test {
	const char *name;
	int (*func)(struct dm_test_state *dms);
	int flags;
};

/* Declare a new driver model test */
#define DM_TEST(_name, _flags)						\
	ll_entry_declare(struct dm_test, _name, dm_test) = {		\
		.name = #_name,						\
		.flags = _flags,					\
		.func = _name,						\
	}

/* Declare ping methods for the drivers */
int test_ping(struct udevice *dev, int pingval, int *pingret);
int testfdt_ping(struct udevice *dev, int pingval, int *pingret);

/**
 * dm_check_operations() - Check that we can perform ping operations
 *
 * This checks that the ping operations work as expected for a device
 *
 * @dms: Overall test state
 * @dev: Device to test
 * @base: Base address, used to check ping return value
 * @priv: Pointer to private test information
 * @return 0 if OK, -ve on error
 */
int dm_check_operations(struct dm_test_state *dms, struct udevice *dev,
			uint32_t base, struct dm_test_priv *priv);

/**
 * dm_check_devices() - check the devices respond to operations correctly
 *
 * @dms: Overall test state
 * @num_devices: Number of test devices to check
 * @return 0 if OK, -ve on error
 */
int dm_check_devices(struct dm_test_state *dms, int num_devices);

/**
 * dm_leak_check_start() - Prepare to check for a memory leak
 *
 * Call this before allocating memory to record the amount of memory being
 * used.
 *
 * @dms: Overall test state
 */
void dm_leak_check_start(struct dm_test_state *dms);

/**
 * dm_leak_check_end() - Check that no memory has leaked
 *
 * Call this after dm_leak_check_start() and after you have hopefuilly freed
 * all the memory that was allocated. This function will print an error if
 * it sees a different amount of total memory allocated than before.
 *
 * @dms: Overall test state
 */int dm_leak_check_end(struct dm_test_state *dms);


/**
 * dm_test_main() - Run all the tests
 *
 * This runs all available driver model tests
 *
 * @return 0 if OK, -ve on error
 */
int dm_test_main(void);

#endif

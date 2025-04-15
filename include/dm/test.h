/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc.
 */

#ifndef __DM_TEST_H
#define __DM_TEST_H

#include <linux/types.h>

struct udevice;

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
 *	Return: 0 if OK, -ve on error
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
	DM_TEST_OP_PRE_PROBE,
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

	DM_TEST_TYPE_COUNT,
};

/* The number added to the ping total on each probe */
#define DM_TEST_START_TOTAL	5

/**
 * struct dm_test_priv - private data for the test devices
 */
struct dm_test_priv {
	int ping_total;
	int op_count[DM_TEST_OP_COUNT];
	int uclass_flag;
	int uclass_total;
	int uclass_postp;
};

/* struct dm_test_uc_priv - private data for the testdrv uclass */
struct dm_test_uc_priv {
	int dummy;
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
 * struct dm_test_uclass_plat - private plat data for test uclass
 */
struct dm_test_uclass_plat {
	char dummy[32];
};

/**
 * struct dm_test_parent_data - parent's information on each child
 *
 * @sum: Test value used to check parent data works correctly
 * @flag: Used to track calling of parent operations
 * @uclass_flag: Used to track calling of parent operations by uclass
 */
struct dm_test_parent_data {
	int sum;
	int flag;
};

/* Test values for test device's uclass platform data */
enum {
	TEST_UC_PDATA_INTVAL1 = 2,
	TEST_UC_PDATA_INTVAL2 = 334,
	TEST_UC_PDATA_INTVAL3 = 789452,
};

/**
 * struct dm_test_uclass_platda - uclass's information on each device
 *
 * @intval1: set to TEST_UC_PDATA_INTVAL1 in .post_bind method of test uclass
 * @intval2: set to TEST_UC_PDATA_INTVAL2 in .post_bind method of test uclass
 * @intval3: set to TEST_UC_PDATA_INTVAL3 in .post_bind method of test uclass
 */
struct dm_test_perdev_uc_pdata {
	int intval1;
	int intval2;
	int intval3;
};

/*
 * Operation counts for the test driver, used to check that each method is
 * called correctly
 */
extern int dm_testdrv_op_count[DM_TEST_OP_COUNT];

extern struct unit_test_state global_dm_test_state;

/* Declare a new driver model test */
#define DM_TEST(_name, _flags) \
	UNIT_TEST(_name, UTF_DM | UTF_CONSOLE | (_flags), dm)

/*
 * struct sandbox_sdl_plat - Platform data for the SDL video driver
 *
 * This platform data is needed in tests, so declare it here
 *
 * @xres: Width of display in pixels
 * @yres: Height of display in pixels
 * @bpix: Log2 of bits per pixel (enum video_log2_bpp)
 * @rot: Console rotation (0=normal orientation, 1=90 degrees clockwise,
 *	2=upside down, 3=90 degree counterclockwise)
 * @vidconsole_drv_name: Name of video console driver (set by tests)
 * @font_size: Console font size to select (set by tests)
 */
struct sandbox_sdl_plat {
	int xres;
	int yres;
	int bpix;
	int rot;
	const char *vidconsole_drv_name;
	int font_size;
};

/**
 * struct dm_test_parent_plat - Used to track state in bus tests
 *
 * @count:
 * @bind_flag: Indicates that the child post-bind method was called
 * @uclass_bind_flag: Also indicates that the child post-bind method was called
 */
struct dm_test_parent_plat {
	int count;
	int bind_flag;
	int uclass_bind_flag;
};

enum {
	TEST_FLAG_CHILD_PROBED	= 10,
	TEST_FLAG_CHILD_REMOVED	= -7,
};

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
 * Return: 0 if OK, -ve on error
 */
int dm_check_operations(struct unit_test_state *uts, struct udevice *dev,
			uint32_t base, struct dm_test_priv *priv);

/**
 * dm_check_devices() - check the devices respond to operations correctly
 *
 * @dms: Overall test state
 * @num_devices: Number of test devices to check
 * Return: 0 if OK, -ve on error
 */
int dm_check_devices(struct unit_test_state *uts, int num_devices);

/**
 * dm_leak_check_start() - Prepare to check for a memory leak
 *
 * Call this before allocating memory to record the amount of memory being
 * used.
 *
 * @dms: Overall test state
 */
void dm_leak_check_start(struct unit_test_state *uts);

/**
 * dm_leak_check_end() - Check that no memory has leaked
 *
 * Call this after dm_leak_check_start() and after you have hopefuilly freed
 * all the memory that was allocated. This function will print an error if
 * it sees a different amount of total memory allocated than before.
 *
 * @dms: Overall test state
 */int dm_leak_check_end(struct unit_test_state *uts);

#endif

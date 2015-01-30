/*
 * Tests for the core driver model code
 *
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/ut.h>
#include <dm/util.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	TEST_INTVAL1		= 0,
	TEST_INTVAL2		= 3,
	TEST_INTVAL3		= 6,
	TEST_INTVAL_MANUAL	= 101112,
	TEST_INTVAL_PRE_RELOC	= 7,
};

static const struct dm_test_pdata test_pdata[] = {
	{ .ping_add		= TEST_INTVAL1, },
	{ .ping_add		= TEST_INTVAL2, },
	{ .ping_add		= TEST_INTVAL3, },
};

static const struct dm_test_pdata test_pdata_manual = {
	.ping_add		= TEST_INTVAL_MANUAL,
};

static const struct dm_test_pdata test_pdata_pre_reloc = {
	.ping_add		= TEST_INTVAL_PRE_RELOC,
};

U_BOOT_DEVICE(dm_test_info1) = {
	.name = "test_drv",
	.platdata = &test_pdata[0],
};

U_BOOT_DEVICE(dm_test_info2) = {
	.name = "test_drv",
	.platdata = &test_pdata[1],
};

U_BOOT_DEVICE(dm_test_info3) = {
	.name = "test_drv",
	.platdata = &test_pdata[2],
};

static struct driver_info driver_info_manual = {
	.name = "test_manual_drv",
	.platdata = &test_pdata_manual,
};

static struct driver_info driver_info_pre_reloc = {
	.name = "test_pre_reloc_drv",
	.platdata = &test_pdata_manual,
};

void dm_leak_check_start(struct dm_test_state *dms)
{
	dms->start = mallinfo();
	if (!dms->start.uordblks)
		puts("Warning: Please add '#define DEBUG' to the top of common/dlmalloc.c\n");
}

int dm_leak_check_end(struct dm_test_state *dms)
{
	struct mallinfo end;
	int id;

	/* Don't delete the root class, since we started with that */
	for (id = UCLASS_ROOT + 1; id < UCLASS_COUNT; id++) {
		struct uclass *uc;

		uc = uclass_find(id);
		if (!uc)
			continue;
		ut_assertok(uclass_destroy(uc));
	}

	end = mallinfo();
	ut_asserteq(dms->start.uordblks, end.uordblks);

	return 0;
}

/* Test that binding with platdata occurs correctly */
static int dm_test_autobind(struct dm_test_state *dms)
{
	struct udevice *dev;

	/*
	 * We should have a single class (UCLASS_ROOT) and a single root
	 * device with no children.
	 */
	ut_assert(dms->root);
	ut_asserteq(1, list_count_items(&gd->uclass_root));
	ut_asserteq(0, list_count_items(&gd->dm_root->child_head));
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_POST_BIND]);

	ut_assertok(dm_scan_platdata(false));

	/* We should have our test class now at least, plus more children */
	ut_assert(1 < list_count_items(&gd->uclass_root));
	ut_assert(0 < list_count_items(&gd->dm_root->child_head));

	/* Our 3 dm_test_infox children should be bound to the test uclass */
	ut_asserteq(3, dm_testdrv_op_count[DM_TEST_OP_POST_BIND]);

	/* No devices should be probed */
	list_for_each_entry(dev, &gd->dm_root->child_head, sibling_node)
		ut_assert(!(dev->flags & DM_FLAG_ACTIVATED));

	/* Our test driver should have been bound 3 times */
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_BIND] == 3);

	return 0;
}
DM_TEST(dm_test_autobind, 0);

/* Test that autoprobe finds all the expected devices */
static int dm_test_autoprobe(struct dm_test_state *dms)
{
	int expected_base_add;
	struct udevice *dev;
	struct uclass *uc;
	int i;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));
	ut_assert(uc);

	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_INIT]);
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_POST_PROBE]);

	/* The root device should not be activated until needed */
	ut_assert(dms->root->flags & DM_FLAG_ACTIVATED);

	/*
	 * We should be able to find the three test devices, and they should
	 * all be activated as they are used (lazy activation, required by
	 * U-Boot)
	 */
	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assertf(!(dev->flags & DM_FLAG_ACTIVATED),
			   "Driver %d/%s already activated", i, dev->name);

		/* This should activate it */
		ut_assertok(uclass_get_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assert(dev->flags & DM_FLAG_ACTIVATED);

		/* Activating a device should activate the root device */
		if (!i)
			ut_assert(dms->root->flags & DM_FLAG_ACTIVATED);
	}

	/* Our 3 dm_test_infox children should be passed to post_probe */
	ut_asserteq(3, dm_testdrv_op_count[DM_TEST_OP_POST_PROBE]);

	/* Also we can check the per-device data */
	expected_base_add = 0;
	for (i = 0; i < 3; i++) {
		struct dm_test_uclass_perdev_priv *priv;
		struct dm_test_pdata *pdata;

		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);

		priv = dev->uclass_priv;
		ut_assert(priv);
		ut_asserteq(expected_base_add, priv->base_add);

		pdata = dev->platdata;
		expected_base_add += pdata->ping_add;
	}

	return 0;
}
DM_TEST(dm_test_autoprobe, DM_TESTF_SCAN_PDATA);

/* Check that we see the correct platdata in each device */
static int dm_test_platdata(struct dm_test_state *dms)
{
	const struct dm_test_pdata *pdata;
	struct udevice *dev;
	int i;

	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		pdata = dev->platdata;
		ut_assert(pdata->ping_add == test_pdata[i].ping_add);
	}

	return 0;
}
DM_TEST(dm_test_platdata, DM_TESTF_SCAN_PDATA);

/* Test that we can bind, probe, remove, unbind a driver */
static int dm_test_lifecycle(struct dm_test_state *dms)
{
	int op_count[DM_TEST_OP_COUNT];
	struct udevice *dev, *test_dev;
	int pingret;
	int ret;

	memcpy(op_count, dm_testdrv_op_count, sizeof(op_count));

	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_BIND]
			== op_count[DM_TEST_OP_BIND] + 1);
	ut_assert(!dev->priv);

	/* Probe the device - it should fail allocating private data */
	dms->force_fail_alloc = 1;
	ret = device_probe(dev);
	ut_assert(ret == -ENOMEM);
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_PROBE]
			== op_count[DM_TEST_OP_PROBE] + 1);
	ut_assert(!dev->priv);

	/* Try again without the alloc failure */
	dms->force_fail_alloc = 0;
	ut_assertok(device_probe(dev));
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_PROBE]
			== op_count[DM_TEST_OP_PROBE] + 2);
	ut_assert(dev->priv);

	/* This should be device 3 in the uclass */
	ut_assertok(uclass_find_device(UCLASS_TEST, 3, &test_dev));
	ut_assert(dev == test_dev);

	/* Try ping */
	ut_assertok(test_ping(dev, 100, &pingret));
	ut_assert(pingret == 102);

	/* Now remove device 3 */
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_PRE_REMOVE]);
	ut_assertok(device_remove(dev));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_PRE_REMOVE]);

	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_UNBIND]);
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_PRE_UNBIND]);
	ut_assertok(device_unbind(dev));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_UNBIND]);
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_PRE_UNBIND]);

	return 0;
}
DM_TEST(dm_test_lifecycle, DM_TESTF_SCAN_PDATA | DM_TESTF_PROBE_TEST);

/* Test that we can bind/unbind and the lists update correctly */
static int dm_test_ordering(struct dm_test_state *dms)
{
	struct udevice *dev, *dev_penultimate, *dev_last, *test_dev;
	int pingret;

	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);

	/* Bind two new devices (numbers 4 and 5) */
	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
					&dev_penultimate));
	ut_assert(dev_penultimate);
	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
					&dev_last));
	ut_assert(dev_last);

	/* Now remove device 3 */
	ut_assertok(device_remove(dev));
	ut_assertok(device_unbind(dev));

	/* The device numbering should have shifted down one */
	ut_assertok(uclass_find_device(UCLASS_TEST, 3, &test_dev));
	ut_assert(dev_penultimate == test_dev);
	ut_assertok(uclass_find_device(UCLASS_TEST, 4, &test_dev));
	ut_assert(dev_last == test_dev);

	/* Add back the original device 3, now in position 5 */
	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);

	/* Try ping */
	ut_assertok(test_ping(dev, 100, &pingret));
	ut_assert(pingret == 102);

	/* Remove 3 and 4 */
	ut_assertok(device_remove(dev_penultimate));
	ut_assertok(device_unbind(dev_penultimate));
	ut_assertok(device_remove(dev_last));
	ut_assertok(device_unbind(dev_last));

	/* Our device should now be in position 3 */
	ut_assertok(uclass_find_device(UCLASS_TEST, 3, &test_dev));
	ut_assert(dev == test_dev);

	/* Now remove device 3 */
	ut_assertok(device_remove(dev));
	ut_assertok(device_unbind(dev));

	return 0;
}
DM_TEST(dm_test_ordering, DM_TESTF_SCAN_PDATA);

/* Check that we can perform operations on a device (do a ping) */
int dm_check_operations(struct dm_test_state *dms, struct udevice *dev,
			uint32_t base, struct dm_test_priv *priv)
{
	int expected;
	int pingret;

	/* Getting the child device should allocate platdata / priv */
	ut_assertok(testfdt_ping(dev, 10, &pingret));
	ut_assert(dev->priv);
	ut_assert(dev->platdata);

	expected = 10 + base;
	ut_asserteq(expected, pingret);

	/* Do another ping */
	ut_assertok(testfdt_ping(dev, 20, &pingret));
	expected = 20 + base;
	ut_asserteq(expected, pingret);

	/* Now check the ping_total */
	priv = dev->priv;
	ut_asserteq(DM_TEST_START_TOTAL + 10 + 20 + base * 2,
		    priv->ping_total);

	return 0;
}

/* Check that we can perform operations on devices */
static int dm_test_operations(struct dm_test_state *dms)
{
	struct udevice *dev;
	int i;

	/*
	 * Now check that the ping adds are what we expect. This is using the
	 * ping-add property in each node.
	 */
	for (i = 0; i < ARRAY_SIZE(test_pdata); i++) {
		uint32_t base;

		ut_assertok(uclass_get_device(UCLASS_TEST, i, &dev));

		/*
		 * Get the 'reg' property, which tells us what the ping add
		 * should be. We don't use the platdata because we want
		 * to test the code that sets that up (testfdt_drv_probe()).
		 */
		base = test_pdata[i].ping_add;
		debug("dev=%d, base=%d\n", i, base);

		ut_assert(!dm_check_operations(dms, dev, base, dev->priv));
	}

	return 0;
}
DM_TEST(dm_test_operations, DM_TESTF_SCAN_PDATA);

/* Remove all drivers and check that things work */
static int dm_test_remove(struct dm_test_state *dms)
{
	struct udevice *dev;
	int i;

	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assertf(dev->flags & DM_FLAG_ACTIVATED,
			   "Driver %d/%s not activated", i, dev->name);
		ut_assertok(device_remove(dev));
		ut_assertf(!(dev->flags & DM_FLAG_ACTIVATED),
			   "Driver %d/%s should have deactivated", i,
			   dev->name);
		ut_assert(!dev->priv);
	}

	return 0;
}
DM_TEST(dm_test_remove, DM_TESTF_SCAN_PDATA | DM_TESTF_PROBE_TEST);

/* Remove and recreate everything, check for memory leaks */
static int dm_test_leak(struct dm_test_state *dms)
{
	int i;

	for (i = 0; i < 2; i++) {
		struct udevice *dev;
		int ret;
		int id;

		dm_leak_check_start(dms);

		ut_assertok(dm_scan_platdata(false));
		ut_assertok(dm_scan_fdt(gd->fdt_blob, false));

		/* Scanning the uclass is enough to probe all the devices */
		for (id = UCLASS_ROOT; id < UCLASS_COUNT; id++) {
			for (ret = uclass_first_device(UCLASS_TEST, &dev);
			     dev;
			     ret = uclass_next_device(&dev))
				;
			ut_assertok(ret);
		}

		ut_assertok(dm_leak_check_end(dms));
	}

	return 0;
}
DM_TEST(dm_test_leak, 0);

/* Test uclass init/destroy methods */
static int dm_test_uclass(struct dm_test_state *dms)
{
	struct uclass *uc;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_INIT]);
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_DESTROY]);
	ut_assert(uc->priv);

	ut_assertok(uclass_destroy(uc));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_INIT]);
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_DESTROY]);

	return 0;
}
DM_TEST(dm_test_uclass, 0);

/**
 * create_children() - Create children of a parent node
 *
 * @dms:	Test system state
 * @parent:	Parent device
 * @count:	Number of children to create
 * @key:	Key value to put in first child. Subsequence children
 *		receive an incrementing value
 * @child:	If not NULL, then the child device pointers are written into
 *		this array.
 * @return 0 if OK, -ve on error
 */
static int create_children(struct dm_test_state *dms, struct udevice *parent,
			   int count, int key, struct udevice *child[])
{
	struct udevice *dev;
	int i;

	for (i = 0; i < count; i++) {
		struct dm_test_pdata *pdata;

		ut_assertok(device_bind_by_name(parent, false,
						&driver_info_manual, &dev));
		pdata = calloc(1, sizeof(*pdata));
		pdata->ping_add = key + i;
		dev->platdata = pdata;
		if (child)
			child[i] = dev;
	}

	return 0;
}

#define NODE_COUNT	10

static int dm_test_children(struct dm_test_state *dms)
{
	struct udevice *top[NODE_COUNT];
	struct udevice *child[NODE_COUNT];
	struct udevice *grandchild[NODE_COUNT];
	struct udevice *dev;
	int total;
	int ret;
	int i;

	/* We don't care about the numbering for this test */
	dms->skip_post_probe = 1;

	ut_assert(NODE_COUNT > 5);

	/* First create 10 top-level children */
	ut_assertok(create_children(dms, dms->root, NODE_COUNT, 0, top));

	/* Now a few have their own children */
	ut_assertok(create_children(dms, top[2], NODE_COUNT, 2, NULL));
	ut_assertok(create_children(dms, top[5], NODE_COUNT, 5, child));

	/* And grandchildren */
	for (i = 0; i < NODE_COUNT; i++)
		ut_assertok(create_children(dms, child[i], NODE_COUNT, 50 * i,
					    i == 2 ? grandchild : NULL));

	/* Check total number of devices */
	total = NODE_COUNT * (3 + NODE_COUNT);
	ut_asserteq(total, dm_testdrv_op_count[DM_TEST_OP_BIND]);

	/* Try probing one of the grandchildren */
	ut_assertok(uclass_get_device(UCLASS_TEST,
				      NODE_COUNT * 3 + 2 * NODE_COUNT, &dev));
	ut_asserteq_ptr(grandchild[0], dev);

	/*
	 * This should have probed the child and top node also, for a total
	 * of 3 nodes.
	 */
	ut_asserteq(3, dm_testdrv_op_count[DM_TEST_OP_PROBE]);

	/* Probe the other grandchildren */
	for (i = 1; i < NODE_COUNT; i++)
		ut_assertok(device_probe(grandchild[i]));

	ut_asserteq(2 + NODE_COUNT, dm_testdrv_op_count[DM_TEST_OP_PROBE]);

	/* Probe everything */
	for (ret = uclass_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_next_device(&dev))
		;
	ut_assertok(ret);

	ut_asserteq(total, dm_testdrv_op_count[DM_TEST_OP_PROBE]);

	/* Remove a top-level child and check that the children are removed */
	ut_assertok(device_remove(top[2]));
	ut_asserteq(NODE_COUNT + 1, dm_testdrv_op_count[DM_TEST_OP_REMOVE]);
	dm_testdrv_op_count[DM_TEST_OP_REMOVE] = 0;

	/* Try one with grandchildren */
	ut_assertok(uclass_get_device(UCLASS_TEST, 5, &dev));
	ut_asserteq_ptr(dev, top[5]);
	ut_assertok(device_remove(dev));
	ut_asserteq(1 + NODE_COUNT * (1 + NODE_COUNT),
		    dm_testdrv_op_count[DM_TEST_OP_REMOVE]);

	/* Try the same with unbind */
	ut_assertok(device_unbind(top[2]));
	ut_asserteq(NODE_COUNT + 1, dm_testdrv_op_count[DM_TEST_OP_UNBIND]);
	dm_testdrv_op_count[DM_TEST_OP_UNBIND] = 0;

	/* Try one with grandchildren */
	ut_assertok(uclass_get_device(UCLASS_TEST, 5, &dev));
	ut_asserteq_ptr(dev, top[6]);
	ut_assertok(device_unbind(top[5]));
	ut_asserteq(1 + NODE_COUNT * (1 + NODE_COUNT),
		    dm_testdrv_op_count[DM_TEST_OP_UNBIND]);

	return 0;
}
DM_TEST(dm_test_children, 0);

/* Test that pre-relocation devices work as expected */
static int dm_test_pre_reloc(struct dm_test_state *dms)
{
	struct udevice *dev;

	/* The normal driver should refuse to bind before relocation */
	ut_asserteq(-EPERM, device_bind_by_name(dms->root, true,
						&driver_info_manual, &dev));

	/* But this one is marked pre-reloc */
	ut_assertok(device_bind_by_name(dms->root, true,
					&driver_info_pre_reloc, &dev));

	return 0;
}
DM_TEST(dm_test_pre_reloc, 0);

static int dm_test_uclass_before_ready(struct dm_test_state *dms)
{
	struct uclass *uc;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));

	memset(gd, '\0', sizeof(*gd));
	ut_asserteq_ptr(NULL, uclass_find(UCLASS_TEST));

	return 0;
}

DM_TEST(dm_test_uclass_before_ready, 0);

static int dm_test_device_get_uclass_id(struct dm_test_state *dms)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_TEST, 0, &dev));
	ut_asserteq(UCLASS_TEST, device_get_uclass_id(dev));

	return 0;
}
DM_TEST(dm_test_device_get_uclass_id, DM_TESTF_SCAN_PDATA);

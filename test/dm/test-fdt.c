/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/test.h>
#include <dm/root.h>
#include <dm/ut.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

static int testfdt_drv_ping(struct udevice *dev, int pingval, int *pingret)
{
	const struct dm_test_pdata *pdata = dev->platdata;
	struct dm_test_priv *priv = dev_get_priv(dev);

	*pingret = pingval + pdata->ping_add;
	priv->ping_total += *pingret;

	return 0;
}

static const struct test_ops test_ops = {
	.ping = testfdt_drv_ping,
};

static int testfdt_ofdata_to_platdata(struct udevice *dev)
{
	struct dm_test_pdata *pdata = dev_get_platdata(dev);

	pdata->ping_add = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					"ping-add", -1);
	pdata->base = fdtdec_get_addr(gd->fdt_blob, dev->of_offset,
				      "ping-expect");

	return 0;
}

static int testfdt_drv_probe(struct udevice *dev)
{
	struct dm_test_priv *priv = dev_get_priv(dev);

	priv->ping_total += DM_TEST_START_TOTAL;

	return 0;
}

static const struct udevice_id testfdt_ids[] = {
	{
		.compatible = "denx,u-boot-fdt-test",
		.data = DM_TEST_TYPE_FIRST },
	{
		.compatible = "google,another-fdt-test",
		.data = DM_TEST_TYPE_SECOND },
	{ }
};

U_BOOT_DRIVER(testfdt_drv) = {
	.name	= "testfdt_drv",
	.of_match	= testfdt_ids,
	.id	= UCLASS_TEST_FDT,
	.ofdata_to_platdata = testfdt_ofdata_to_platdata,
	.probe	= testfdt_drv_probe,
	.ops	= &test_ops,
	.priv_auto_alloc_size = sizeof(struct dm_test_priv),
	.platdata_auto_alloc_size = sizeof(struct dm_test_pdata),
};

/* From here is the testfdt uclass code */
int testfdt_ping(struct udevice *dev, int pingval, int *pingret)
{
	const struct test_ops *ops = device_get_ops(dev);

	if (!ops->ping)
		return -ENOSYS;

	return ops->ping(dev, pingval, pingret);
}

UCLASS_DRIVER(testfdt) = {
	.name		= "testfdt",
	.id		= UCLASS_TEST_FDT,
};

int dm_check_devices(struct dm_test_state *dms, int num_devices)
{
	struct udevice *dev;
	int ret;
	int i;

	/*
	 * Now check that the ping adds are what we expect. This is using the
	 * ping-add property in each node.
	 */
	for (i = 0; i < num_devices; i++) {
		uint32_t base;

		ret = uclass_get_device(UCLASS_TEST_FDT, i, &dev);
		ut_assert(!ret);

		/*
		 * Get the 'ping-expect' property, which tells us what the
		 * ping add should be. We don't use the platdata because we
		 * want to test the code that sets that up
		 * (testfdt_drv_probe()).
		 */
		base = fdtdec_get_addr(gd->fdt_blob, dev->of_offset,
				       "ping-expect");
		debug("dev=%d, base=%d: %s\n", i, base,
		      fdt_get_name(gd->fdt_blob, dev->of_offset, NULL));

		ut_assert(!dm_check_operations(dms, dev, base,
					       dev_get_priv(dev)));
	}

	return 0;
}

/* Test that FDT-based binding works correctly */
static int dm_test_fdt(struct dm_test_state *dms)
{
	const int num_devices = 4;
	struct udevice *dev;
	struct uclass *uc;
	int ret;
	int i;

	ret = dm_scan_fdt(gd->fdt_blob, false);
	ut_assert(!ret);

	ret = uclass_get(UCLASS_TEST_FDT, &uc);
	ut_assert(!ret);

	/* These are num_devices compatible root-level device tree nodes */
	ut_asserteq(num_devices, list_count_items(&uc->dev_head));

	/* Each should have no platdata / priv */
	for (i = 0; i < num_devices; i++) {
		ret = uclass_find_device(UCLASS_TEST_FDT, i, &dev);
		ut_assert(!ret);
		ut_assert(!dev_get_priv(dev));
		ut_assert(!dev->platdata);
	}

	ut_assertok(dm_check_devices(dms, num_devices));

	return 0;
}
DM_TEST(dm_test_fdt, 0);

static int dm_test_fdt_pre_reloc(struct dm_test_state *dms)
{
	struct uclass *uc;
	int ret;

	ret = dm_scan_fdt(gd->fdt_blob, true);
	ut_assert(!ret);

	ret = uclass_get(UCLASS_TEST_FDT, &uc);
	ut_assert(!ret);

	/* These is only one pre-reloc device */
	ut_asserteq(1, list_count_items(&uc->dev_head));

	return 0;
}
DM_TEST(dm_test_fdt_pre_reloc, 0);

/* Test that sequence numbers are allocated properly */
static int dm_test_fdt_uclass_seq(struct dm_test_state *dms)
{
	struct udevice *dev;

	/* A few basic santiy tests */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_FDT, 3, true, &dev));
	ut_asserteq_str("b-test", dev->name);

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_FDT, 0, true, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 5,
						       true, &dev));
	ut_asserteq_ptr(NULL, dev);

	/* Test aliases */
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 6, &dev));
	ut_asserteq_str("e-test", dev->name);

	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 7,
						       true, &dev));

	/*
	 * Note that c-test nodes are not probed since it is not a top-level
	 * node
	 */
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 3, &dev));
	ut_asserteq_str("b-test", dev->name);

	/*
	 * d-test wants sequence number 3 also, but it can't have it because
	 * b-test gets it first.
	 */
	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 2, &dev));
	ut_asserteq_str("d-test", dev->name);

	/* d-test actually gets 0 */
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("d-test", dev->name);

	/* initially no one wants seq 1 */
	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_TEST_FDT, 1,
						      &dev));
	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 1, &dev));

	/* But now that it is probed, we can find it */
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 1, &dev));
	ut_asserteq_str("a-test", dev->name);

	return 0;
}
DM_TEST(dm_test_fdt_uclass_seq, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can find a device by device tree offset */
static int dm_test_fdt_offset(struct dm_test_state *dms)
{
	const void *blob = gd->fdt_blob;
	struct udevice *dev;
	int node;

	node = fdt_path_offset(blob, "/e-test");
	ut_assert(node > 0);
	ut_assertok(uclass_get_device_by_of_offset(UCLASS_TEST_FDT, node,
						   &dev));
	ut_asserteq_str("e-test", dev->name);

	/* This node should not be bound */
	node = fdt_path_offset(blob, "/junk");
	ut_assert(node > 0);
	ut_asserteq(-ENODEV, uclass_get_device_by_of_offset(UCLASS_TEST_FDT,
							    node, &dev));

	/* This is not a top level node so should not be probed */
	node = fdt_path_offset(blob, "/some-bus/c-test@5");
	ut_assert(node > 0);
	ut_asserteq(-ENODEV, uclass_get_device_by_of_offset(UCLASS_TEST_FDT,
							    node, &dev));

	return 0;
}
DM_TEST(dm_test_fdt_offset, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

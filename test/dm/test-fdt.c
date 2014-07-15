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

static int testfdt_drv_ping(struct device *dev, int pingval, int *pingret)
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

static int testfdt_ofdata_to_platdata(struct device *dev)
{
	struct dm_test_pdata *pdata = dev_get_platdata(dev);

	pdata->ping_add = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					"ping-add", -1);
	pdata->base = fdtdec_get_addr(gd->fdt_blob, dev->of_offset, "reg");

	return 0;
}

static int testfdt_drv_probe(struct device *dev)
{
	struct dm_test_priv *priv = dev_get_priv(dev);

	priv->ping_total += DM_TEST_START_TOTAL;

	return 0;
}

static const struct device_id testfdt_ids[] = {
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
int testfdt_ping(struct device *dev, int pingval, int *pingret)
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

/* Test that FDT-based binding works correctly */
static int dm_test_fdt(struct dm_test_state *dms)
{
	const int num_drivers = 3;
	struct device *dev;
	struct uclass *uc;
	int ret;
	int i;

	ret = dm_scan_fdt(gd->fdt_blob);
	ut_assert(!ret);

	ret = uclass_get(UCLASS_TEST_FDT, &uc);
	ut_assert(!ret);

	/* These are num_drivers compatible root-level device tree nodes */
	ut_asserteq(num_drivers, list_count_items(&uc->dev_head));

	/* Each should have no platdata / priv */
	for (i = 0; i < num_drivers; i++) {
		ret = uclass_find_device(UCLASS_TEST_FDT, i, &dev);
		ut_assert(!ret);
		ut_assert(!dev_get_priv(dev));
		ut_assert(!dev->platdata);
	}

	/*
	 * Now check that the ping adds are what we expect. This is using the
	 * ping-add property in each node.
	 */
	for (i = 0; i < num_drivers; i++) {
		uint32_t base;

		ret = uclass_get_device(UCLASS_TEST_FDT, i, &dev);
		ut_assert(!ret);

		/*
		 * Get the 'reg' property, which tells us what the ping add
		 * should be. We don't use the platdata because we want
		 * to test the code that sets that up (testfdt_drv_probe()).
		 */
		base = fdtdec_get_addr(gd->fdt_blob, dev->of_offset, "reg");
		debug("dev=%d, base=%d: %s\n", i, base,
		      fdt_get_name(gd->fdt_blob, dev->of_offset, NULL));

		ut_assert(!dm_check_operations(dms, dev, base,
					       dev_get_priv(dev)));
	}

	return 0;
}
DM_TEST(dm_test_fdt, 0);

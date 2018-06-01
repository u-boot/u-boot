// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/test.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <test/ut.h>

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

	pdata->ping_add = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"ping-add", -1);
	pdata->base = fdtdec_get_addr(gd->fdt_blob, dev_of_offset(dev),
				      "ping-expect");

	return 0;
}

static int testfdt_drv_probe(struct udevice *dev)
{
	struct dm_test_priv *priv = dev_get_priv(dev);

	priv->ping_total += DM_TEST_START_TOTAL;

	/*
	 * If this device is on a bus, the uclass_flag will be set before
	 * calling this function. This is used by
	 * dm_test_bus_child_pre_probe_uclass().
	 */
	priv->uclass_total += priv->uclass_flag;

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
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

struct dm_testprobe_pdata {
	int probe_err;
};

static int testprobe_drv_probe(struct udevice *dev)
{
	struct dm_testprobe_pdata *pdata = dev_get_platdata(dev);

	return pdata->probe_err;
}

static const struct udevice_id testprobe_ids[] = {
	{ .compatible = "denx,u-boot-probe-test" },
	{ }
};

U_BOOT_DRIVER(testprobe_drv) = {
	.name	= "testprobe_drv",
	.of_match	= testprobe_ids,
	.id	= UCLASS_TEST_PROBE,
	.probe	= testprobe_drv_probe,
	.platdata_auto_alloc_size	= sizeof(struct dm_testprobe_pdata),
};

UCLASS_DRIVER(testprobe) = {
	.name		= "testprobe",
	.id		= UCLASS_TEST_PROBE,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

int dm_check_devices(struct unit_test_state *uts, int num_devices)
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
		base = fdtdec_get_addr(gd->fdt_blob, dev_of_offset(dev),
				       "ping-expect");
		debug("dev=%d, base=%d: %s\n", i, base,
		      fdt_get_name(gd->fdt_blob, dev_of_offset(dev), NULL));

		ut_assert(!dm_check_operations(uts, dev, base,
					       dev_get_priv(dev)));
	}

	return 0;
}

/* Test that FDT-based binding works correctly */
static int dm_test_fdt(struct unit_test_state *uts)
{
	const int num_devices = 7;
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

	/* Each should have platform data but no private data */
	for (i = 0; i < num_devices; i++) {
		ret = uclass_find_device(UCLASS_TEST_FDT, i, &dev);
		ut_assert(!ret);
		ut_assert(!dev_get_priv(dev));
		ut_assert(dev->platdata);
	}

	ut_assertok(dm_check_devices(uts, num_devices));

	return 0;
}
DM_TEST(dm_test_fdt, 0);

static int dm_test_fdt_pre_reloc(struct unit_test_state *uts)
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
static int dm_test_fdt_uclass_seq(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* A few basic santiy tests */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_FDT, 3, true, &dev));
	ut_asserteq_str("b-test", dev->name);

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_FDT, 8, true, &dev));
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
	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 4, &dev));

	/* But now that it is probed, we can find it */
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 1, &dev));
	ut_asserteq_str("f-test", dev->name);

	return 0;
}
DM_TEST(dm_test_fdt_uclass_seq, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can find a device by device tree offset */
static int dm_test_fdt_offset(struct unit_test_state *uts)
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
DM_TEST(dm_test_fdt_offset,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT | DM_TESTF_FLAT_TREE);

/**
 * Test various error conditions with uclass_first_device() and
 * uclass_next_device()
 */
static int dm_test_first_next_device(struct unit_test_state *uts)
{
	struct dm_testprobe_pdata *pdata;
	struct udevice *dev, *parent = NULL;
	int count;
	int ret;

	/* There should be 4 devices */
	for (ret = uclass_first_device(UCLASS_TEST_PROBE, &dev), count = 0;
	     dev;
	     ret = uclass_next_device(&dev)) {
		count++;
		parent = dev_get_parent(dev);
		}
	ut_assertok(ret);
	ut_asserteq(4, count);

	/* Remove them and try again, with an error on the second one */
	ut_assertok(uclass_get_device(UCLASS_TEST_PROBE, 1, &dev));
	pdata = dev_get_platdata(dev);
	pdata->probe_err = -ENOMEM;
	device_remove(parent, DM_REMOVE_NORMAL);
	ut_assertok(uclass_first_device(UCLASS_TEST_PROBE, &dev));
	ut_asserteq(-ENOMEM, uclass_next_device(&dev));
	ut_asserteq_ptr(dev, NULL);

	/* Now an error on the first one */
	ut_assertok(uclass_get_device(UCLASS_TEST_PROBE, 0, &dev));
	pdata = dev_get_platdata(dev);
	pdata->probe_err = -ENOENT;
	device_remove(parent, DM_REMOVE_NORMAL);
	ut_asserteq(-ENOENT, uclass_first_device(UCLASS_TEST_PROBE, &dev));

	return 0;
}
DM_TEST(dm_test_first_next_device, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/**
 * check_devices() - Check return values and pointers
 *
 * This runs through a full sequence of uclass_first_device_check()...
 * uclass_next_device_check() checking that the return values and devices
 * are correct.
 *
 * @uts: Test state
 * @devlist: List of expected devices
 * @mask: Indicates which devices should return an error. Device n should
 *	  return error (-NOENT - n) if bit n is set, or no error (i.e. 0) if
 *	  bit n is clear.
 */
static int check_devices(struct unit_test_state *uts,
			 struct udevice *devlist[], int mask)
{
	int expected_ret;
	struct udevice *dev;
	int i;

	expected_ret = (mask & 1) ? -ENOENT : 0;
	mask >>= 1;
	ut_asserteq(expected_ret,
		    uclass_first_device_check(UCLASS_TEST_PROBE, &dev));
	for (i = 0; i < 4; i++) {
		ut_asserteq_ptr(devlist[i], dev);
		expected_ret = (mask & 1) ? -ENOENT - (i + 1) : 0;
		mask >>= 1;
		ut_asserteq(expected_ret, uclass_next_device_check(&dev));
	}
	ut_asserteq_ptr(NULL, dev);

	return 0;
}

/* Test uclass_first_device_check() and uclass_next_device_check() */
static int dm_test_first_next_ok_device(struct unit_test_state *uts)
{
	struct dm_testprobe_pdata *pdata;
	struct udevice *dev, *parent = NULL, *devlist[4];
	int count;
	int ret;

	/* There should be 4 devices */
	count = 0;
	for (ret = uclass_first_device_check(UCLASS_TEST_PROBE, &dev);
	     dev;
	     ret = uclass_next_device_check(&dev)) {
		ut_assertok(ret);
		devlist[count++] = dev;
		parent = dev_get_parent(dev);
		}
	ut_asserteq(4, count);
	ut_assertok(uclass_first_device_check(UCLASS_TEST_PROBE, &dev));
	ut_assertok(check_devices(uts, devlist, 0));

	/* Remove them and try again, with an error on the second one */
	pdata = dev_get_platdata(devlist[1]);
	pdata->probe_err = -ENOENT - 1;
	device_remove(parent, DM_REMOVE_NORMAL);
	ut_assertok(check_devices(uts, devlist, 1 << 1));

	/* Now an error on the first one */
	pdata = dev_get_platdata(devlist[0]);
	pdata->probe_err = -ENOENT - 0;
	device_remove(parent, DM_REMOVE_NORMAL);
	ut_assertok(check_devices(uts, devlist, 3 << 0));

	/* Now errors on all */
	pdata = dev_get_platdata(devlist[2]);
	pdata->probe_err = -ENOENT - 2;
	pdata = dev_get_platdata(devlist[3]);
	pdata->probe_err = -ENOENT - 3;
	device_remove(parent, DM_REMOVE_NORMAL);
	ut_assertok(check_devices(uts, devlist, 0xf << 0));

	return 0;
}
DM_TEST(dm_test_first_next_ok_device, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static const struct udevice_id fdt_dummy_ids[] = {
	{ .compatible = "denx,u-boot-fdt-dummy", },
	{ }
};

UCLASS_DRIVER(fdt_dummy) = {
	.name		= "fdt-dummy",
	.id		= UCLASS_TEST_DUMMY,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

U_BOOT_DRIVER(fdt_dummy_drv) = {
	.name	= "fdt_dummy_drv",
	.of_match	= fdt_dummy_ids,
	.id	= UCLASS_TEST_DUMMY,
};

static int dm_test_fdt_translation(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Some simple translations */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, true, &dev));
	ut_asserteq_str("dev@0,0", dev->name);
	ut_asserteq(0x8000, dev_read_addr(dev));

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 1, true, &dev));
	ut_asserteq_str("dev@1,100", dev->name);
	ut_asserteq(0x9000, dev_read_addr(dev));

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 2, true, &dev));
	ut_asserteq_str("dev@2,200", dev->name);
	ut_asserteq(0xA000, dev_read_addr(dev));

	/* No translation for busses with #size-cells == 0 */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 3, true, &dev));
	ut_asserteq_str("dev@42", dev->name);
	ut_asserteq(0x42, dev_read_addr(dev));

	return 0;
}
DM_TEST(dm_test_fdt_translation, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test devfdt_remap_addr_index() */
static int dm_test_fdt_remap_addr_flat(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt_addr_t addr;
	void *paddr;

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, true, &dev));

	addr = devfdt_get_addr(dev);
	ut_asserteq(0x8000, addr);

	paddr = map_physmem(addr, 0, MAP_NOCACHE);
	ut_assertnonnull(paddr);
	ut_asserteq_ptr(paddr, devfdt_remap_addr(dev));

	return 0;
}
DM_TEST(dm_test_fdt_remap_addr_flat,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT | DM_TESTF_FLAT_TREE);

/* Test dev_remap_addr_index() */
static int dm_test_fdt_remap_addr_live(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt_addr_t addr;
	void *paddr;

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, true, &dev));

	addr = dev_read_addr(dev);
	ut_asserteq(0x8000, addr);

	paddr = map_physmem(addr, 0, MAP_NOCACHE);
	ut_assertnonnull(paddr);
	ut_asserteq_ptr(paddr, dev_remap_addr(dev));

	return 0;
}
DM_TEST(dm_test_fdt_remap_addr_live,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

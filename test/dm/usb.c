/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Test that sandbox USB works correctly */
static int dm_test_usb_base(struct unit_test_state *uts)
{
	struct udevice *bus;

	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_USB, 0, &bus));
	ut_assertok(uclass_get_device(UCLASS_USB, 0, &bus));
	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_USB, 2, &bus));

	return 0;
}
DM_TEST(dm_test_usb_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/*
 * Test that we can use the flash stick. This is more of a functional test. It
 * covers scanning the bug, setting up a hub and a flash stick and reading
 * data from the flash stick.
 */
static int dm_test_usb_flash(struct unit_test_state *uts)
{
	struct udevice *dev;
	block_dev_desc_t *dev_desc;
	char cmp[1024];

	state_set_skip_delays(true);
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &dev));
	ut_assertok(get_device("usb", "0", &dev_desc));

	/* Read a few blocks and look for the string we expect */
	ut_asserteq(512, dev_desc->blksz);
	memset(cmp, '\0', sizeof(cmp));
	ut_asserteq(2, dev_desc->block_read(dev_desc, 0, 2, cmp));
	ut_assertok(strcmp(cmp, "this is a test"));

	return 0;
}
DM_TEST(dm_test_usb_flash, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* test that we can handle multiple storage devices */
static int dm_test_usb_multi(struct unit_test_state *uts)
{
	struct udevice *dev;

	state_set_skip_delays(true);
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 2, &dev));

	return 0;
}
DM_TEST(dm_test_usb_multi, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int count_usb_devices(void)
{
	struct udevice *hub;
	struct uclass *uc;
	int count = 0;
	int ret;

	ret = uclass_get(UCLASS_USB_HUB, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(hub, uc) {
		struct udevice *dev;

		count++;
		for (device_find_first_child(hub, &dev);
		     dev;
		     device_find_next_child(&dev)) {
			count++;
		}
	}

	return count;
}

/* test that we can remove an emulated device and it is then not found */
static int dm_test_usb_remove(struct unit_test_state *uts)
{
	struct udevice *dev, *emul;

	/* Scan and check that all devices are present */
	state_set_skip_delays(true);
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 2, &dev));
	ut_asserteq(6, count_usb_devices());
	ut_assertok(usb_stop());
	ut_asserteq(6, count_usb_devices());

	/* Remove the second emulation device */
	ut_assertok(uclass_find_device_by_name(UCLASS_USB_EMUL, "flash-stick@1",
					       &dev));
	ut_assertok(device_unbind(dev));

	/* Rescan - only the first and third should be present */
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &dev));
	ut_assertok(usb_emul_find_for_dev(dev, &emul));
	ut_asserteq_str("flash-stick@0", emul->name);
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 1, &dev));
	ut_assertok(usb_emul_find_for_dev(dev, &emul));
	ut_asserteq_str("flash-stick@2", emul->name);

	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_MASS_STORAGE, 2, &dev));

	ut_asserteq(5, count_usb_devices());
	ut_assertok(usb_stop());
	ut_asserteq(5, count_usb_devices());

	return 0;
}
DM_TEST(dm_test_usb_remove, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

const char usb_tree_base[] =
"  1  Hub (12 Mb/s, 100mA)\n"
"  |  sandbox hub 2345\n"
"  |\n"
"  |\b+-2  Mass Storage (12 Mb/s, 100mA)\n"
"  |    sandbox flash flash-stick@0\n"
"  |  \n"
"  |\b+-3  Mass Storage (12 Mb/s, 100mA)\n"
"  |    sandbox flash flash-stick@1\n"
"  |  \n"
"  |\b+-4  Mass Storage (12 Mb/s, 100mA)\n"
"  |    sandbox flash flash-stick@2\n"
"  |  \n"
"  |\b+-5  Human Interface (12 Mb/s, 100mA)\n"
"       sandbox keyboard keyb@3\n"
"     \n";

/* test that the 'usb tree' command output looks correct */
static int dm_test_usb_tree(struct unit_test_state *uts)
{
	char *data;
	int len;

	state_set_skip_delays(true);
	ut_assertok(usb_init());
	console_record_reset_enable();
	usb_show_tree();
	len = membuff_getraw(&gd->console_out, -1, true, &data);
	if (len)
		data[len] = '\0';
	ut_asserteq_str(usb_tree_base, data);
	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_usb_tree, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

const char usb_tree_remove[] =
"  1  Hub (12 Mb/s, 100mA)\n"
"  |  sandbox hub 2345\n"
"  |\n"
"  |\b+-2  Mass Storage (12 Mb/s, 100mA)\n"
"  |    sandbox flash flash-stick@0\n"
"  |  \n"
"  |\b+-3  Mass Storage (12 Mb/s, 100mA)\n"
"  |    sandbox flash flash-stick@2\n"
"  |  \n"
"  |\b+-4  Human Interface (12 Mb/s, 100mA)\n"
"       sandbox keyboard keyb@3\n"
"     \n";

/*
 * test that the 'usb tree' command output looks correct when we remove a
 * device
 */
static int dm_test_usb_tree_remove(struct unit_test_state *uts)
{
	struct udevice *dev;
	char *data;
	int len;

	/* Remove the second emulation device */
	ut_assertok(uclass_find_device_by_name(UCLASS_USB_EMUL, "flash-stick@1",
					       &dev));
	ut_assertok(device_unbind(dev));

	state_set_skip_delays(true);
	ut_assertok(usb_init());
	console_record_reset_enable();
	usb_show_tree();
	len = membuff_getraw(&gd->console_out, -1, true, &data);
	if (len)
		data[len] = '\0';
	ut_asserteq_str(usb_tree_remove, data);
	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_usb_tree_remove, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

const char usb_tree_reorder[] =
"  1  Hub (12 Mb/s, 100mA)\n"
"  |  sandbox hub 2345\n"
"  |\n"
"  |\b+-2  Mass Storage (12 Mb/s, 100mA)\n"
"  |    sandbox flash flash-stick@0\n"
"  |  \n"
"  |\b+-3  Mass Storage (12 Mb/s, 100mA)\n"
"  |    sandbox flash flash-stick@2\n"
"  |  \n"
"  |\b+-4  Human Interface (12 Mb/s, 100mA)\n"
"  |    sandbox keyboard keyb@3\n"
"  |  \n"
"  |\b+-5  Mass Storage (12 Mb/s, 100mA)\n"
"       sandbox flash flash-stick@1\n"
"     \n";

/*
 * test that the 'usb tree' command output looks correct when we reorder two
 * devices.
 */
static int dm_test_usb_tree_reorder(struct unit_test_state *uts)
{
	struct udevice *dev, *parent;
	char *data;
	int len;

	/* Remove the second emulation device */
	ut_assertok(uclass_find_device_by_name(UCLASS_USB_EMUL, "flash-stick@1",
					       &dev));
	parent = dev->parent;

	/* Reorder the devices in the parent list and uclass list */
	list_del(&dev->sibling_node);
	list_add_tail(&dev->sibling_node, &parent->child_head);

	list_del(&dev->uclass_node);
	list_add_tail(&dev->uclass_node, &dev->uclass->dev_head);

	state_set_skip_delays(true);
	ut_assertok(usb_init());
	console_record_reset_enable();
	usb_show_tree();
	len = membuff_getraw(&gd->console_out, -1, true, &data);
	if (len)
		data[len] = '\0';
	ut_asserteq_str(usb_tree_reorder, data);
	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_usb_tree_reorder, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_usb_keyb(struct unit_test_state *uts)
{
	struct udevice *dev;

	state_set_skip_delays(true);
	ut_assertok(usb_init());

	/* Initially there should be no characters */
	ut_asserteq(0, tstc());

	ut_assertok(uclass_get_device_by_name(UCLASS_USB_EMUL, "keyb",
					      &dev));

	/*
	 * Add a string to the USB keyboard buffer - it should appear in
	 * stdin
	 */
	ut_assertok(sandbox_usb_keyb_add_string(dev, "ab"));
	ut_asserteq(1, tstc());
	ut_asserteq('a', getc());
	ut_asserteq(1, tstc());
	ut_asserteq('b', getc());
	ut_asserteq(0, tstc());

	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_usb_keyb, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

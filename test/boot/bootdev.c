// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for bootdev functions. All start with 'bootdev'
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootstd.h>
#include <dm.h>
#include <bootdev.h>
#include <bootflow.h>
#include <mapmem.h>
#include <os.h>
#include <test/suites.h>
#include <test/ut.h>
#include "bootstd_common.h"

/* Allow reseting the USB-started flag */
extern char usb_started;

/* Check 'bootdev list' command */
static int bootdev_test_cmd_list(struct unit_test_state *uts)
{
	int probed;

	console_record_reset_enable();
	for (probed = 0; probed < 2; probed++) {
		int probe_ch = probed ? '+' : ' ';

		ut_assertok(run_command(probed ? "bootdev list -p" :
			"bootdev list", 0));
		ut_assert_nextline("Seq  Probed  Status  Uclass    Name");
		ut_assert_nextlinen("---");
		ut_assert_nextline("%3x   [ %c ]  %6s  %-8s  %s", 0, probe_ch, "OK",
				   "mmc", "mmc2.bootdev");
		ut_assert_nextline("%3x   [ %c ]  %6s  %-8s  %s", 1, probe_ch, "OK",
				   "mmc", "mmc1.bootdev");
		ut_assert_nextline("%3x   [ %c ]  %6s  %-8s  %s", 2, probe_ch, "OK",
				   "mmc", "mmc0.bootdev");
		ut_assert_nextlinen("---");
		ut_assert_nextline("(3 bootdevs)");
		ut_assert_console_end();
	}

	return 0;
}
BOOTSTD_TEST(bootdev_test_cmd_list, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check 'bootdev select' and 'info' commands */
static int bootdev_test_cmd_select(struct unit_test_state *uts)
{
	struct bootstd_priv *std;

	/* get access to the CLI's cur_bootdev */
	ut_assertok(bootstd_get_priv(&std));

	console_record_reset_enable();
	ut_asserteq(1, run_command("bootdev info", 0));
	ut_assert_nextlinen("Please use");
	ut_assert_console_end();

	/* select by sequence */
	ut_assertok(run_command("bootdev select 0", 0));
	ut_assert_console_end();

	ut_assertok(run_command("bootdev info", 0));
	ut_assert_nextline("Name:      mmc2.bootdev");
	ut_assert_nextline("Sequence:  0");
	ut_assert_nextline("Status:    Probed");
	ut_assert_nextline("Uclass:    mmc");
	ut_assert_nextline("Bootflows: 0 (0 valid)");
	ut_assert_console_end();

	/* select by bootdev name */
	ut_assertok(run_command("bootdev select mmc1.bootdev", 0));
	ut_assert_console_end();
	ut_assertnonnull(std->cur_bootdev);
	ut_asserteq_str("mmc1.bootdev", std->cur_bootdev->name);

	/* select by bootdev label*/
	ut_assertok(run_command("bootdev select mmc1", 0));
	ut_assert_console_end();
	ut_assertnonnull(std->cur_bootdev);
	ut_asserteq_str("mmc1.bootdev", std->cur_bootdev->name);

	/* deselect */
	ut_assertok(run_command("bootdev select", 0));
	ut_assert_console_end();
	ut_assertnull(std->cur_bootdev);

	ut_asserteq(1, run_command("bootdev info", 0));
	ut_assert_nextlinen("Please use");
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(bootdev_test_cmd_select, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check bootdev labels */
static int bootdev_test_labels(struct unit_test_state *uts)
{
	struct udevice *dev, *media;

	ut_assertok(bootdev_find_by_label("mmc2", &dev));
	ut_asserteq(UCLASS_BOOTDEV, device_get_uclass_id(dev));
	media = dev_get_parent(dev);
	ut_asserteq(UCLASS_MMC, device_get_uclass_id(media));
	ut_asserteq_str("mmc2", media->name);

	/* Check invalid uclass */
	ut_asserteq(-EINVAL, bootdev_find_by_label("fred0", &dev));

	/* Check unknown sequence number */
	ut_asserteq(-ENOENT, bootdev_find_by_label("mmc6", &dev));

	return 0;
}
BOOTSTD_TEST(bootdev_test_labels, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check bootdev ordering with the bootdev-order property */
static int bootdev_test_order(struct unit_test_state *uts)
{
	struct bootflow_iter iter;
	struct bootflow bflow;

	/*
	 * First try the order set by the bootdev-order property
	 * Like all sandbox unit tests this relies on the devicetree setting up
	 * the required devices:
	 *
	 * mmc0 - nothing connected
	 * mmc1 - connected to mmc1.img file
	 * mmc2 - nothing connected
	 */
	ut_assertok(env_set("boot_targets", NULL));
	ut_assertok(bootflow_scan_first(&iter, 0, &bflow));
	ut_asserteq(2, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_order[0]->name);
	ut_asserteq_str("mmc1.bootdev", iter.dev_order[1]->name);
	bootflow_iter_uninit(&iter);

	/* Use the environment variable to override it */
	ut_assertok(env_set("boot_targets", "mmc1 mmc2"));
	ut_assertok(bootflow_scan_first(&iter, 0, &bflow));
	ut_asserteq(2, iter.num_devs);
	ut_asserteq_str("mmc1.bootdev", iter.dev_order[0]->name);
	ut_asserteq_str("mmc2.bootdev", iter.dev_order[1]->name);
	bootflow_iter_uninit(&iter);

	/*
	 * Now drop both orderings, to check the default (prioriy/sequence)
	 * ordering
	 */
	ut_assertok(env_set("boot_targets", NULL));
	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	ut_assertok(bootflow_scan_first(&iter, 0, &bflow));
	ut_asserteq(3, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_order[0]->name);
	ut_asserteq_str("mmc1.bootdev", iter.dev_order[1]->name);
	ut_asserteq_str("mmc0.bootdev", iter.dev_order[2]->name);

	/*
	 * Check that adding aliases for the bootdevs works. We just fake it by
	 * setting the sequence numbers directly.
	 */
	iter.dev_order[0]->seq_ = 0;
	iter.dev_order[1]->seq_ = 3;
	iter.dev_order[2]->seq_ = 2;
	bootflow_iter_uninit(&iter);

	ut_assertok(bootflow_scan_first(&iter, 0, &bflow));
	ut_asserteq(3, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_order[0]->name);
	ut_asserteq_str("mmc0.bootdev", iter.dev_order[1]->name);
	ut_asserteq_str("mmc1.bootdev", iter.dev_order[2]->name);
	bootflow_iter_uninit(&iter);

	return 0;
}
BOOTSTD_TEST(bootdev_test_order, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

/* Check bootdev ordering with the uclass priority */
static int bootdev_test_prio(struct unit_test_state *uts)
{
	struct bootdev_uc_plat *ucp;
	struct bootflow_iter iter;
	struct bootflow bflow;
	struct udevice *blk;

	/* Start up USB which gives us three additional bootdevs */
	usb_started = false;
	ut_assertok(run_command("usb start", 0));

	ut_assertok(bootstd_test_drop_bootdev_order(uts));

	/* 3 MMC and 3 USB bootdevs: MMC should come before USB */
	console_record_reset_enable();
	ut_assertok(bootflow_scan_first(&iter, 0, &bflow));
	ut_asserteq(6, iter.num_devs);
	ut_asserteq_str("mmc2.bootdev", iter.dev_order[0]->name);
	ut_asserteq_str("usb_mass_storage.lun0.bootdev",
			iter.dev_order[3]->name);

	ut_assertok(bootdev_get_sibling_blk(iter.dev_order[3], &blk));
	ut_asserteq_str("usb_mass_storage.lun0", blk->name);

	/* adjust the priority of the first USB bootdev to the highest */
	ucp = dev_get_uclass_plat(iter.dev_order[3]);
	ucp->prio = 1;

	bootflow_iter_uninit(&iter);
	ut_assertok(bootflow_scan_first(&iter, 0, &bflow));
	ut_asserteq(6, iter.num_devs);
	ut_asserteq_str("usb_mass_storage.lun0.bootdev",
			iter.dev_order[0]->name);
	ut_asserteq_str("mmc2.bootdev", iter.dev_order[1]->name);

	return 0;
}
BOOTSTD_TEST(bootdev_test_prio, UT_TESTF_DM | UT_TESTF_SCAN_FDT);

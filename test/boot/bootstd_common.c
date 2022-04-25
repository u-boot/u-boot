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
#include <test/suites.h>
#include <test/ut.h>
#include "bootstd_common.h"

int bootstd_test_drop_bootdev_order(struct unit_test_state *uts)
{
	struct bootstd_priv *priv;
	struct udevice *bootstd;

	ut_assertok(uclass_first_device_err(UCLASS_BOOTSTD, &bootstd));
	priv = dev_get_priv(bootstd);
	priv->bootdev_order = NULL;

	return 0;
}

int do_ut_bootstd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(bootstd_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(bootstd_test);

	return cmd_ut_category("bootstd", "bootstd_test_",
			       tests, n_ents, argc, argv);
}

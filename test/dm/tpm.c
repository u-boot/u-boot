// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <tpm_api.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of the TPM uclass */
static int dm_test_tpm(struct unit_test_state *uts)
{
	struct udevice *dev;
	char buf[50];

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_TPM, &dev));
	ut_assert(tpm_is_v2(dev));

	ut_assert(tpm_report_state(dev, buf, sizeof(buf)));
	ut_asserteq_str("init_done=0", buf);

	ut_assertok(tpm_init(dev));

	ut_assert(tpm_report_state(dev, buf, sizeof(buf)));
	ut_asserteq_str("init_done=1", buf);

	return 0;
}
DM_TEST(dm_test_tpm, UT_TESTF_SCAN_FDT);

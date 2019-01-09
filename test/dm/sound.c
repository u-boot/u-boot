// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <sound.h>
#include <dm/test.h>
#include <test/ut.h>
#include <asm/test.h>

/* Basic test of the sound codec uclass */
static int dm_test_sound(struct unit_test_state *uts)
{
	struct sound_uc_priv *uc_priv;
	struct udevice *dev;

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_SOUND, &dev));
	uc_priv = dev_get_uclass_priv(dev);
	ut_asserteq_str("audio-codec", uc_priv->codec->name);
	ut_asserteq_str("i2s", uc_priv->i2s->name);
	ut_asserteq(0, sandbox_get_setup_called(dev));

	ut_assertok(sound_beep(dev, 1, 100));
	ut_asserteq(4560, sandbox_get_sound_sum(dev));
	ut_assertok(sound_beep(dev, 1, 100));
	ut_asserteq(9120, sandbox_get_sound_sum(dev));

	return 0;
}
DM_TEST(dm_test_sound, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

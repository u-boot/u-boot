// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the EDID timing selection
 *
 * Copyright 2026 Ana Bobillo <anabobillo3@gmail.com>
 */

#include <edid.h>
#include <fdtdec.h>
#include <linux/string.h>
#include <test/common.h>
#include <test/test.h>
#include <test/ut.h>

/* Detailed timings for 3840x2160 and 1920x1080 at 60Hz */
static const u8 dtd_3840x2160[18] = {
	0x08, 0xe8, 0x00, 0x30, 0xf2, 0x70, 0x5a, 0x80,
	0xb0, 0x58, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1e,
};

static const u8 dtd_1920x1080[18] = {
	0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40,
	0x58, 0x2c, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1e,
};

static void edid_test_fill(u8 *buf)
{
	struct edid1_info *edid = (struct edid1_info *)buf;

	memset(buf, '\0', EDID_SIZE);
	memcpy(edid->header, "\x0\xff\xff\xff\xff\xff\xff\x0", 8);
	edid->version = 1;
	edid->revision = 4;
	edid->video_input_definition = 0xa5;	/* digital, 8 bits/colour */
	edid->feature_support = 0x02;		/* preferred timing mode */
	memset(edid->standard_timings, 0x01, sizeof(edid->standard_timings));
	memcpy(&edid->monitor_details.timing[0], dtd_3840x2160,
	       sizeof(dtd_3840x2160));
	memcpy(&edid->monitor_details.timing[sizeof(dtd_3840x2160)],
	       dtd_1920x1080, sizeof(dtd_1920x1080));
}

static bool edid_test_max_1920(void *priv, const struct display_timing *timing)
{
	return timing->hactive.typ <= 1920;
}

static bool edid_test_none(void *priv, const struct display_timing *timing)
{
	return false;
}

/* Test that a timing rejected by mode_valid is skipped, and never returned */
static int edid_test_timing_validate(struct unit_test_state *uts)
{
	struct display_timing timing;
	u8 buf[EDID_SIZE];
	int bpc;

	edid_test_fill(buf);

	ut_assertok(edid_get_timing_validate(buf, sizeof(buf), &timing, &bpc,
					     NULL, NULL));
	ut_asserteq(3840, timing.hactive.typ);
	ut_asserteq(2160, timing.vactive.typ);
	ut_asserteq(8, bpc);

	ut_assertok(edid_get_timing_validate(buf, sizeof(buf), &timing, &bpc,
					     edid_test_max_1920, NULL));
	ut_asserteq(1920, timing.hactive.typ);
	ut_asserteq(1080, timing.vactive.typ);

	/* With no valid mode, the timing passed in is left as it was */
	memset(&timing, '\0', sizeof(timing));
	timing.hactive.typ = 640;
	timing.vactive.typ = 480;
	ut_asserteq(-EINVAL, edid_get_timing_validate(buf, sizeof(buf), &timing,
						      &bpc, edid_test_none,
						      NULL));
	ut_asserteq(640, timing.hactive.typ);
	ut_asserteq(480, timing.vactive.typ);

	return 0;
}
COMMON_TEST(edid_test_timing_validate, 0);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Unit tests for event handling
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <event.h>
#include <test/common.h>
#include <test/test.h>
#include <test/ut.h>

struct test_state {
	struct udevice *dev;
	int val;
};

static int h_adder(void *ctx, struct event *event)
{
	struct event_data_test *data = &event->data.test;
	struct test_state *test_state = ctx;

	test_state->val += data->signal;

	return 0;
}

static int test_event_base(struct unit_test_state *uts)
{
	struct test_state state;
	int signal;

	state.val = 12;
	ut_assertok(event_register("wibble", EVT_TEST, h_adder, &state));

	signal = 17;

	/* Check that the handler is called */
	ut_assertok(event_notify(EVT_TEST, &signal, sizeof(signal)));
	ut_asserteq(12 + 17, state.val);

	return 0;
}
COMMON_TEST(test_event_base, 0);

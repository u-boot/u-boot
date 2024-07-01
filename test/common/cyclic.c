// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#include <cyclic.h>
#include <dm.h>
#include <test/common.h>
#include <test/test.h>
#include <test/ut.h>
#include <watchdog.h>
#include <linux/delay.h>

/* Test that cyclic function is called */
static struct cyclic_test {
	struct cyclic_info cyclic;
	bool called;
} cyclic_test;

static void test_cb(struct cyclic_info *c)
{
	struct cyclic_test *t = container_of(c, struct cyclic_test, cyclic);
	t->called = true;
}

static int dm_test_cyclic_running(struct unit_test_state *uts)
{
	cyclic_test.called = false;
	cyclic_register(&cyclic_test.cyclic, test_cb, 10 * 1000, "cyclic_test");

	/* Execute all registered cyclic functions */
	schedule();
	ut_asserteq(true, cyclic_test.called);

	cyclic_unregister(&cyclic_test.cyclic);

	return 0;
}
COMMON_TEST(dm_test_cyclic_running, 0);

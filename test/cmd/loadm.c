// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for loadm command
 *
 * Copyright 2022 ARM Limited
 * Copyright 2022 Linaro
 *
 * Authors:
 *   Rui Miguel Silva <rui.silva@linaro.org>
 */

#include <console.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

#define BUF_SIZE 0x100

#define LOADM_TEST(_name, _flags)	UNIT_TEST(_name, _flags, loadm)

static int loadm_test_params(struct unit_test_state *uts)
{
	run_command("loadm", 0);
	ut_assert_nextline("loadm - load binary blob from source address to destination address");

	ut_assertok(console_record_reset_enable());
	run_command("loadm 0x12345678", 0);
	ut_assert_nextline("loadm - load binary blob from source address to destination address");

	ut_assertok(console_record_reset_enable());
	run_command("loadm 0x12345678 0x12345678", 0);
	ut_assert_nextline("loadm - load binary blob from source address to destination address");

	ut_assertok(console_record_reset_enable());
	run_command("loadm 0x12345678 0x12345678 0", 0);
	ut_assert_nextline("loadm: can not load zero bytes");

	return 0;
}
LOADM_TEST(loadm_test_params, UTF_CONSOLE);

static int loadm_test_load (struct unit_test_state *uts)
{
	char *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	memset(buf, 0xaa, BUF_SIZE / 2);

	run_command("loadm 0x0 0x80 0x80", 0);
	ut_assert_nextline("loaded bin to memory: size: 128");

	unmap_sysmem(buf);

	return 0;
}
LOADM_TEST(loadm_test_load, UTF_CONSOLE);

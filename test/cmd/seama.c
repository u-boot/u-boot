// SPDX-License-Identifier: GPL-2.0+
/*
 * Executes tests for SEAMA (SEAttle iMAge) command
 *
 * Copyright (C) 2021 Linus Walleij <linus.walleij@linaro.org>
 */

#include <command.h>
#include <dm.h>
#include <test/test.h>
#include <test/ut.h>

#define SEAMA_TEST(_name, _flags)	UNIT_TEST(_name, _flags, seama)

static int seama_test_noargs(struct unit_test_state *uts)
{
	/* Test that 'seama' with no arguments fails gracefully */
	run_command("seama", 0);
	ut_assert_nextlinen("seama - Load the SEAMA image and sets envs");
	ut_assert_skipline();
	ut_assert_skipline();
	ut_assert_skipline();
	ut_assert_skipline();
	ut_assert_console_end();
	return 0;
}
SEAMA_TEST(seama_test_noargs, UTF_CONSOLE);

static int seama_test_addr(struct unit_test_state *uts)
{
	/* Test that loads SEAMA image 0 to address 0x01000000 */
	run_command("seama 0x01000000", 0);
	ut_assert_nextlinen("Loading SEAMA image 0 from nand0");
	ut_assert_nextlinen("SEMA IMAGE:");
	ut_assert_nextlinen("  metadata size ");
	ut_assert_nextlinen("  image size ");
	ut_assert_nextlinen("  checksum ");
	ut_assert_nextlinen("Decoding SEAMA image 0x01000040..");
	ut_assert_console_end();
	return 0;
}
SEAMA_TEST(seama_test_addr, UTF_CONSOLE);

static int seama_test_index(struct unit_test_state *uts)
{
	/* Test that loads SEAMA image 0 exlicitly specified */
	run_command("seama 0x01000000 0", 0);
	ut_assert_nextlinen("Loading SEAMA image 0 from nand0");
	ut_assert_nextlinen("SEMA IMAGE:");
	ut_assert_nextlinen("  metadata size ");
	ut_assert_nextlinen("  image size ");
	ut_assert_nextlinen("  checksum ");
	ut_assert_nextlinen("Decoding SEAMA image 0x01000040..");
	ut_assert_console_end();
	return 0;
}
SEAMA_TEST(seama_test_index, UTF_CONSOLE);

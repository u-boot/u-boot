// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for Altera SoC FPGA rsu command (usage path).
 *
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 */

#include <command.h>
#include <test/cmd.h>
#include <test/ut.h>

static int cmd_ut_socfpga_rsu_usage(struct unit_test_state *uts)
{
	ut_asserteq(CMD_RET_USAGE, run_command("rsu", 0));

	return 0;
}

CMD_TEST(cmd_ut_socfpga_rsu_usage, 0);

/*
 * Malformed numeric arguments (non-digit characters, overflow, trailing junk)
 * must be rejected by the subcommand handlers with CMD_RET_USAGE *before* any
 * RSU state is touched. This exercises the rsu_parse_num() / rsu_parse_slot()
 * /rsu_parse_hex_* helpers without requiring a working RSU backend, because
 * parsing is performed up-front and the rsu_init() call is never reached on
 * the error path.
 */
static int cmd_ut_socfpga_rsu_bad_slot(struct unit_test_state *uts)
{
	ut_asserteq(CMD_RET_USAGE,
		    run_command("rsu slot_get_info foo", 0));
	ut_asserteq(CMD_RET_USAGE,
		    run_command("rsu slot_get_info 12xyz", 0));
	ut_asserteq(CMD_RET_USAGE,
		    run_command("rsu slot_get_info 99999999999", 0));

	return 0;
}

CMD_TEST(cmd_ut_socfpga_rsu_bad_slot, 0);

static int cmd_ut_socfpga_rsu_bad_size(struct unit_test_state *uts)
{
	/*
	 * slot_program_buf takes <slot> <buffer> <size>; an INT_MAX+1 size
	 * must be rejected with CMD_RET_USAGE (would otherwise be silently
	 * cast to a negative int, which is how buffer overflow bugs start).
	 */
	ut_asserteq(CMD_RET_USAGE,
		    run_command("rsu slot_program_buf 0 0x1000 80000000", 0));
	/* Garbage in the size field. */
	ut_asserteq(CMD_RET_USAGE,
		    run_command("rsu slot_program_buf 0 0x1000 qqq", 0));

	return 0;
}

CMD_TEST(cmd_ut_socfpga_rsu_bad_size, 0);

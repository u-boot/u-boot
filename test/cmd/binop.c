// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the binop command
 */

#include <command.h>
#include <env.h>
#include <mapmem.h>
#include <test/cmd.h>
#include <test/ut.h>

#define MEM_ADDR	((ulong)CONFIG_SYS_LOAD_ADDR + 0x1000)

/* Error messages for a bad hex string, given directly or in an env var */
#define HEX_ERR		"Error: '%s' is not an env var or hex string of up to %d bytes"
#define ENV_ERR		"Error: env var '%s' value '%s' is not a hex string of up to %d bytes"

static int cmd_test_binop_ops(struct unit_test_state *uts)
{
	ut_assertok(run_command("binop xor 4 01020304 ffffffff binop_out", 0));
	ut_asserteq_str("fefdfcfb", env_get("binop_out"));

	ut_assertok(run_command("binop or 4 01020304 00000000 binop_out", 0));
	ut_asserteq_str("01020304", env_get("binop_out"));

	ut_assertok(run_command("binop and 4 0f0f0f0f 00ff00ff binop_out", 0));
	ut_asserteq_str("000f000f", env_get("binop_out"));

	ut_assertok(run_command("binop xor 2 AABB CCDD binop_out", 0));
	ut_asserteq_str("6666", env_get("binop_out"));

	env_set("binop_out", NULL);

	return 0;
}

CMD_TEST(cmd_test_binop_ops, 0);

static int cmd_test_binop_env(struct unit_test_state *uts)
{
	ut_assertok(env_set("binop_a", "aabbccdd"));
	ut_assertok(env_set("binop_b", "11223344"));

	ut_assertok(run_command("binop xor 4 binop_a binop_b binop_out", 0));
	ut_asserteq_str("bb99ff99", env_get("binop_out"));

	/* Short env var is zero-padded */
	ut_assertok(env_set("binop_a", "01"));
	ut_assertok(run_command("binop xor 4 binop_a binop_b binop_out", 0));
	ut_asserteq_str("10223344", env_get("binop_out"));

	env_set("binop_a", NULL);
	env_set("binop_b", NULL);
	env_set("binop_out", NULL);

	return 0;
}

CMD_TEST(cmd_test_binop_env, 0);

static int cmd_test_binop_mem(struct unit_test_state *uts)
{
	const u8 expect[] = { 0x0e, 0x0d, 0x0c, 0x0b };
	u8 *buf;

	buf = map_sysmem(MEM_ADDR, 8);
	memset(buf, 0, 8);
	buf[0] = 0x01;
	buf[1] = 0x02;
	buf[2] = 0x03;
	buf[3] = 0x04;

	ut_assertok(run_commandf("binop xor 4 *0x%lx 01010101 binop_out",
				 MEM_ADDR));
	ut_asserteq_str("00030205", env_get("binop_out"));

	ut_assertok(run_commandf("binop xor 4 *0x%lx 0f0f0f0f *0x%lx",
				 MEM_ADDR, MEM_ADDR + 4));
	ut_asserteq_mem(expect, buf + 4, sizeof(expect));

	unmap_sysmem(buf);
	env_set("binop_out", NULL);

	return 0;
}

CMD_TEST(cmd_test_binop_mem, 0);

static int cmd_test_binop_short(struct unit_test_state *uts)
{
	ut_assertok(run_command("binop xor 4 01 02 binop_out", 0));
	ut_asserteq_str("03000000", env_get("binop_out"));

	ut_assertok(env_set("binop_a", "0102"));
	ut_assertok(run_command("binop or 4 binop_a 03 binop_out", 0));
	ut_asserteq_str("03020000", env_get("binop_out"));

	env_set("binop_a", NULL);
	env_set("binop_out", NULL);

	return 0;
}

CMD_TEST(cmd_test_binop_short, 0);

static int cmd_test_binop_print(struct unit_test_state *uts)
{
	ut_assertok(run_command("binop xor 2 0102 0304", 0));
	ut_assert_nextline("02 06 ");
	ut_assert_console_end();

	/* Output wraps after 16 bytes */
	ut_assertok(run_command("binop xor 17 000102030405060708090a0b0c0d0e0f10 00",
				0));
	ut_assert_nextline("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f ");
	ut_assert_nextline("10 ");
	ut_assert_console_end();

	/* Exactly 16 bytes fill one line, with no blank line after it */
	ut_assertok(run_command("binop xor 16 000102030405060708090a0b0c0d0e0f 00",
				0));
	ut_assert_nextline("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f ");
	ut_assert_console_end();

	return 0;
}

CMD_TEST(cmd_test_binop_print, UTF_CONSOLE);

/* Check that the command printed its usage text and nothing else */
static int check_usage(struct unit_test_state *uts)
{
	ut_assert_nextline("binop - compute binary operation");
	ut_assert_nextline_empty();
	if (IS_ENABLED(CONFIG_SYS_LONGHELP)) {
		ut_assert_nextline("Usage:");
		ut_assert_skip_to_line("      xor, or, and.");
	}
	ut_assert_console_end();

	return 0;
}

static int cmd_test_binop_invalid(struct unit_test_state *uts)
{
	/* Failed operations must not change binop_out */
	ut_assertok(env_set("binop_out", "deadbeef"));

	/* Literal hex string: too long, odd length, not hex */
	ut_asserteq(1, run_command("binop xor 2 010203 01 binop_out", 0));
	ut_assert_nextline(HEX_ERR, "010203", 2);
	ut_assert_console_end();
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	ut_asserteq(1, run_command("binop xor 2 010 01 binop_out", 0));
	ut_assert_nextline(HEX_ERR, "010", 2);
	ut_assert_console_end();
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	ut_asserteq(1, run_command("binop xor 2 zz 01 binop_out", 0));
	ut_assert_nextline(HEX_ERR, "zz", 2);
	ut_assert_console_end();
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	/* Env var value: not hex (but fits the count), too long, odd length */
	ut_assertok(env_set("binop_bad", "zzzz"));
	ut_asserteq(1, run_command("binop xor 2 binop_bad 01 binop_out", 0));
	ut_assert_nextline(ENV_ERR, "binop_bad", "zzzz", 2);
	ut_assert_console_end();
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	ut_assertok(env_set("binop_big", "0102030405"));
	ut_asserteq(1, run_command("binop xor 2 binop_big 01 binop_out", 0));
	ut_assert_nextline(ENV_ERR, "binop_big", "0102030405", 2);
	ut_assert_console_end();
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	ut_assertok(env_set("binop_odd", "010"));
	ut_asserteq(1, run_command("binop xor 2 binop_odd 01 binop_out", 0));
	ut_assert_nextline(ENV_ERR, "binop_odd", "010", 2);
	ut_assert_console_end();
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	/* Invalid count and operator */
	ut_asserteq(1, run_command("binop xor 0 01 02 binop_out", 0));
	ut_assertok(check_usage(uts));
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	ut_asserteq(1, run_command("binop xor x 01 02 binop_out", 0));
	ut_assertok(check_usage(uts));
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	ut_asserteq(1, run_command("binop nope 2 0102 0304 binop_out", 0));
	ut_assertok(check_usage(uts));
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	/* Invalid source and destination addresses */
	ut_asserteq(1, run_command("binop xor 2 *nope 01 binop_out", 0));
	ut_assert_nextline("Error: invalid address 'nope'");
	ut_assert_console_end();
	ut_asserteq_str("deadbeef", env_get("binop_out"));

	ut_asserteq(1, run_command("binop xor 2 0102 0304 *nope", 0));
	ut_assert_nextline("Error: invalid address 'nope'");
	ut_assert_console_end();

	env_set("binop_bad", NULL);
	env_set("binop_big", NULL);
	env_set("binop_odd", NULL);
	env_set("binop_out", NULL);

	return 0;
}

CMD_TEST(cmd_test_binop_invalid, UTF_CONSOLE);

static int cmd_test_binop_overrun(struct unit_test_state *uts)
{
	/*
	 * A 25-byte value with a 4-byte count used to overrun the 4-byte
	 * heap buffer. The command must now reject it.
	 */
	ut_assertok(env_set("binop_first", "01020304"));
	ut_assertok(env_set("binop_second",
			    "12334545434543534543534535456456456543534543534534"));

	ut_asserteq(1, run_command("binop xor 4 binop_first binop_second", 0));
	ut_assert_nextline(ENV_ERR, "binop_second", env_get("binop_second"), 4);
	ut_assert_console_end();

	env_set("binop_first", NULL);
	env_set("binop_second", NULL);

	return 0;
}

CMD_TEST(cmd_test_binop_overrun, UTF_CONSOLE);

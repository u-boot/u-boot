// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Tests for hash command
 *
 * Copyright 2024, Heinrich Schuchardt <heinrich.schuchardt@canoncal.com>
 */

#include <command.h>
#include <env.h>
#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_cmd_hash_md5(struct unit_test_state *uts)
{
	if (!CONFIG_IS_ENABLED(MD5)) {
		ut_assert(run_command("hash md5 $loadaddr 0", 0));

		return 0;
	}

	ut_assertok(run_command("hash md5 $loadaddr 0", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_ptr(uts->actual_str,
			strstr(uts->actual_str, "md5 for "));
	ut_assert(strstr(uts->actual_str,
			 "d41d8cd98f00b204e9800998ecf8427e"));
	ut_assert_console_end();

	ut_assertok(run_command("hash md5 $loadaddr 0 foo; echo $foo", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_ptr(uts->actual_str,
			strstr(uts->actual_str, "md5 for "));
	ut_assert(strstr(uts->actual_str,
			 "d41d8cd98f00b204e9800998ecf8427e"));
	ut_assertok(ut_check_console_line(uts,
					  "d41d8cd98f00b204e9800998ecf8427e"));

	if (!CONFIG_IS_ENABLED(HASH_VERIFY)) {
		ut_assert(run_command("hash -v sha256 $loadaddr 0 foo", 0));
		ut_assertok(ut_check_console_line(
				uts, "hash - compute hash message digest"));

		return 0;
	}

	ut_assertok(run_command("hash -v md5 $loadaddr 0 foo", 0));
	ut_assert_console_end();

	env_set("foo", "ffffffffffffffffffffffffffffffff");
	ut_assert(run_command("hash -v md5 $loadaddr 0 foo", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_assert(strstr(uts->actual_str, "!="));
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_cmd_hash_md5, UTF_CONSOLE);

static int dm_test_cmd_hash_sha256(struct unit_test_state *uts)
{
	if (!CONFIG_IS_ENABLED(SHA256)) {
		ut_assert(run_command("hash sha256 $loadaddr 0", 0));

		return 0;
	}

	ut_assertok(run_command("hash sha256 $loadaddr 0", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_ptr(uts->actual_str,
			strstr(uts->actual_str, "sha256 for "));
	ut_assert(strstr(uts->actual_str,
			 "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
	ut_assert_console_end();

	ut_assertok(run_command("hash sha256 $loadaddr 0 foo; echo $foo", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_ptr(uts->actual_str,
			strstr(uts->actual_str, "sha256 for "));
	ut_assert(strstr(uts->actual_str,
			 "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
	ut_assertok(ut_check_console_line(
			uts, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));

	if (!CONFIG_IS_ENABLED(HASH_VERIFY)) {
		ut_assert(run_command("hash -v sha256 $loadaddr 0 foo", 0));
		ut_assertok(ut_check_console_line(
				uts, "hash - compute hash message digest"));

		return 0;
	}

	ut_assertok(run_command("hash -v sha256 $loadaddr 0 foo", 0));
	ut_assert_console_end();

	env_set("foo", "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
	ut_assert(run_command("hash -v sha256 $loadaddr 0 foo", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_assert(strstr(uts->actual_str, "!="));
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_cmd_hash_sha256, UTF_CONSOLE);

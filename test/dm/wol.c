// SPDX-License-Identifier: GPL-2.0+

#include <command.h>
#include <env.h>
#include <net/wol.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_wol_magic(struct unit_test_state *uts)
{
	static const u8 mac[] = { 2, 3, 4, 5, 6, 7 };
	u8 packet[sizeof(struct wol_hdr) + WOL_PASSWORD_6B + 1];
	struct wol_hdr *wol = (void *)(packet + 1);
	int i;

	memset(wol->wol_sync, WOL_SYNC_BYTE, WOL_SYNC_COUNT);
	for (i = 0; i < WOL_MAC_REPETITIONS; i++)
		memcpy(wol->wol_dest + i * ARP_HLEN, mac, ARP_HLEN);

	ut_assert(wol_check_magic(wol, sizeof(*wol), mac));
	for (i = 0; i < sizeof(*wol); i++)
		ut_assert(!wol_check_magic(wol, i, mac));
	for (i = 0; i < sizeof(*wol); i++) {
		packet[i + 1] ^= 1;
		ut_assert(!wol_check_magic(wol, sizeof(*wol), mac));
		packet[i + 1] ^= 1;
	}

	return 0;
}

DM_TEST(dm_test_wol_magic, 0);

static int wol_password_check(struct unit_test_state *uts)
{
	u8 packet[sizeof(struct wol_hdr) + WOL_PASSWORD_6B + 1] = {};
	struct wol_hdr *wol = (void *)(packet + 1);
	int i;

	for (i = 0; i < WOL_PASSWORD_6B; i++)
		wol->wol_passwd[i] = i + 1;
	ut_assertok(env_set("wolpassword", "keep"));
	wol_save_password(wol, sizeof(*wol));
	ut_asserteq_str("keep", env_get("wolpassword"));
	wol_save_password(wol, sizeof(*wol) + 3);
	ut_asserteq_str("keep", env_get("wolpassword"));
	wol_save_password(wol, sizeof(*wol) + WOL_PASSWORD_4B);
	ut_asserteq_str("1.2.3.4", env_get("wolpassword"));
	wol_save_password(wol, sizeof(*wol) + WOL_PASSWORD_6B);
	ut_asserteq_str("01:02:03:04:05:06", env_get("wolpassword"));
	/* Passwords are not MAC-address variables with write-once semantics. */
	ut_assertok(env_set("wolpassword", "02:00:00:00:00:01"));
	wol_save_password(wol, sizeof(*wol) + WOL_PASSWORD_6B);
	ut_asserteq_str("01:02:03:04:05:06", env_get("wolpassword"));

	return 0;
}

static int dm_test_wol_password(struct unit_test_state *uts)
{
	const char *value = env_get("wolpassword");
	char *saved = value ? strdup(value) : NULL;
	int ret;

	if (value)
		ut_assertnonnull(saved);
	ret = wol_password_check(uts);
	env_set("wolpassword", saved);
	free(saved);

	return ret;
}

DM_TEST(dm_test_wol_password, 0);

static int dm_test_wol_args(struct unit_test_state *uts)
{
	const char * const commands[] = {
		"wol", "wol ''", "wol -1", "wol abc", "wol 1x", "wol 0x10",
		"wol 18446744073709551616", "wol 18446744073709552",
		"wol 999999999999999999999999999999999", "wol 1 extra",
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		ut_assert(run_command(commands[i], 0));

	return 0;
}

DM_TEST(dm_test_wol_args, UTF_CONSOLE);

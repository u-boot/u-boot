// SPDX-License-Identifier: GPL-2.0
/* Regression tests for lwIP NFS command argument ownership. */

#include <command.h>
#include <env.h>
#include <malloc.h>
#include <net.h>
#include <dm/test.h>
#include <test/ut.h>

static int nfs_filename_check(struct unit_test_state *uts)
{
	char *const argv[] = { "nfs", "0", "1.1.2.2:/dir:file" };
	char *const bad_argv[] = { "nfs", "not-an-address", "1.1.2.2:/dir:file" };
	unsigned int allocated;
	int i;

	/* A zero address rejects the transfer after parsing, before any I/O. */
	ut_asserteq(CMD_RET_FAILURE, do_nfs(NULL, 0, 3, argv));
	allocated = mallinfo().uordblks;
	for (i = 0; i < 32; i++) {
		ut_asserteq(CMD_RET_FAILURE, do_nfs(NULL, 0, 3, argv));
		ut_asserteq(allocated, mallinfo().uordblks);
	}
	ut_asserteq_str("1.1.2.2:/dir:file", argv[2]);
	ut_asserteq(CMD_RET_USAGE, do_nfs(NULL, 0, 3, bad_argv));
	strcpy(net_boot_file_name, "1.1.2.2:/global:file");
	ut_asserteq(CMD_RET_FAILURE, do_nfs(NULL, 0, 1, argv));
	ut_asserteq_str("1.1.2.2:/global:file", net_boot_file_name);
	net_boot_file_name[0] = '\0';
	ut_asserteq(CMD_RET_FAILURE, do_nfs(NULL, 0, 1, argv));
	ut_asserteq_str("1.1.2.2:/environment:file", env_get("bootfile"));

	return 0;
}

static int dm_test_nfs_filename(struct unit_test_state *uts)
{
	char bootfile[sizeof(net_boot_file_name)];
	const char *file = env_get("bootfile"), *addr = env_get("loadaddr");
	char *saved_file = file ? strdup(file) : NULL;
	char *saved_addr = addr ? strdup(addr) : NULL;
	int ret = -ENOMEM;

	if ((file && !saved_file) || (addr && !saved_addr))
		goto out;
	memcpy(bootfile, net_boot_file_name, sizeof(bootfile));
	if (!env_set("bootfile", "1.1.2.2:/environment:file") &&
	    !env_set("loadaddr", "0"))
		ret = nfs_filename_check(uts);
	memcpy(net_boot_file_name, bootfile, sizeof(bootfile));
	if (env_set("bootfile", saved_file) || env_set("loadaddr", saved_addr))
		ret = -EINVAL;
out:
	free(saved_file);
	free(saved_addr);

	return ret;
}

DM_TEST(dm_test_nfs_filename, 0);

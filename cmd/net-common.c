// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <command.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <net.h>
#include <linux/compat.h>
#include <linux/ethtool.h>

static int do_net_list(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct udevice *current = eth_get_dev();
	unsigned char env_enetaddr[ARP_HLEN];
	const struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_ETH, dev, uc) {
		eth_env_get_enetaddr_by_index("eth", dev_seq(dev), env_enetaddr);
		printf("eth%d : %s %pM %s\n", dev_seq(dev), dev->name, env_enetaddr,
		       current == dev ? "active" : "");
	}
	return CMD_RET_SUCCESS;
}

static int do_net_stats(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int nstats, err, i, off;
	struct udevice *dev;
	u64 *values;
	u8 *strings;

	if (argc < 2)
		return CMD_RET_USAGE;

	err = uclass_get_device_by_name(UCLASS_ETH, argv[1], &dev);
	if (err) {
		printf("Could not find device %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	if (!eth_get_ops(dev)->get_sset_count ||
	    !eth_get_ops(dev)->get_strings ||
	    !eth_get_ops(dev)->get_stats) {
		printf("Driver does not implement stats dump!\n");
		return CMD_RET_FAILURE;
	}

	nstats = eth_get_ops(dev)->get_sset_count(dev);
	strings = kcalloc(nstats, ETH_GSTRING_LEN, GFP_KERNEL);
	if (!strings)
		return CMD_RET_FAILURE;

	values = kcalloc(nstats, sizeof(u64), GFP_KERNEL);
	if (!values)
		goto err_free_strings;

	eth_get_ops(dev)->get_strings(dev, strings);
	eth_get_ops(dev)->get_stats(dev, values);

	off = 0;
	for (i = 0; i < nstats; i++) {
		printf("  %s: %llu\n", &strings[off], values[i]);
		off += ETH_GSTRING_LEN;
	};

	kfree(strings);
	kfree(values);

	return CMD_RET_SUCCESS;

err_free_strings:
	kfree(strings);

	return CMD_RET_FAILURE;
}

static struct cmd_tbl cmd_net[] = {
	U_BOOT_CMD_MKENT(list, 1, 0, do_net_list, "", ""),
	U_BOOT_CMD_MKENT(stats, 2, 0, do_net_stats, "", ""),
};

static int do_net(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_net, ARRAY_SIZE(cmd_net));

	/* Drop the net command */
	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(net, 3, 1, do_net, "NET sub-system",
	   "list - list available devices\n"
	   "stats <device> - dump statistics for specified device\n");

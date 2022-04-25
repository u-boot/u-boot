// SPDX-License-Identifier: GPL-2.0+
/*
 * 'bootflow' command
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootstd.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <mapmem.h>

/**
 * report_bootflow_err() - Report where a bootflow failed
 *
 * When a bootflow does not make it to the 'loaded' state, something went wrong.
 * Print a helpful message if there is an error
 *
 * @bflow: Bootflow to process
 * @err: Error code (0 if none)
 */
static void report_bootflow_err(struct bootflow *bflow, int err)
{
	if (!err)
		return;

	/* Indent out to 'Method' */
	printf("     ** ");

	switch (bflow->state) {
	case BOOTFLOWST_BASE:
		printf("No media/partition found");
		break;
	case BOOTFLOWST_MEDIA:
		printf("No partition found");
		break;
	case BOOTFLOWST_PART:
		printf("No filesystem found");
		break;
	case BOOTFLOWST_FS:
		printf("File not found");
		break;
	case BOOTFLOWST_FILE:
		printf("File cannot be loaded");
		break;
	case BOOTFLOWST_READY:
		printf("Ready");
		break;
	case BOOTFLOWST_COUNT:
		break;
	}

	printf(", err=%d\n", err);
}

/**
 * show_bootflow() - Show the status of a bootflow
 *
 * @seq: Bootflow index
 * @bflow: Bootflow to show
 * @errors: True to show the error received, if any
 */
static void show_bootflow(int index, struct bootflow *bflow, bool errors)
{
	printf("%3x  %-11s  %-6s  %-9.9s %4x  %-25.25s %s\n", index,
	       bflow->method->name, bootflow_state_get_name(bflow->state),
	       dev_get_uclass_name(dev_get_parent(bflow->dev)), bflow->part,
	       bflow->name, bflow->fname);
	if (errors)
		report_bootflow_err(bflow, bflow->err);
}

static void show_header(void)
{
	printf("Seq  Method       State   Uclass    Part  Name                      Filename\n");
	printf("---  -----------  ------  --------  ----  ------------------------  ----------------\n");
}

static void show_footer(int count, int num_valid)
{
	printf("---  -----------  ------  --------  ----  ------------------------  ----------------\n");
	printf("(%d bootflow%s, %d valid)\n", count, count != 1 ? "s" : "",
	       num_valid);
}

static int do_bootflow_scan(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow_iter iter;
	struct udevice *dev;
	struct bootflow bflow;
	bool all = false, boot = false, errors = false, list = false;
	int num_valid = 0;
	bool has_args;
	int ret, i;
	int flags;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;
	dev = std->cur_bootdev;

	has_args = argc > 1 && *argv[1] == '-';
	if (IS_ENABLED(CONFIG_CMD_BOOTFLOW_FULL)) {
		if (has_args) {
			all = strchr(argv[1], 'a');
			boot = strchr(argv[1], 'b');
			errors = strchr(argv[1], 'e');
			list = strchr(argv[1], 'l');
			argc--;
			argv++;
		}
		if (argc > 1) {
			const char *label = argv[1];

			if (bootdev_find_by_any(label, &dev))
				return CMD_RET_FAILURE;
		}
	} else {
		if (has_args) {
			printf("Flags not supported: enable CONFIG_BOOTFLOW_FULL\n");
			return CMD_RET_USAGE;
		}
		boot = true;
	}

	std->cur_bootflow = NULL;

	flags = 0;
	if (list)
		flags |= BOOTFLOWF_SHOW;
	if (all)
		flags |= BOOTFLOWF_ALL;

	/*
	 * If we have a device, just scan for bootflows attached to that device
	 */
	if (IS_ENABLED(CONFIG_CMD_BOOTFLOW_FULL) && dev) {
		if (list) {
			printf("Scanning for bootflows in bootdev '%s'\n",
			       dev->name);
			show_header();
		}
		bootdev_clear_bootflows(dev);
		for (i = 0,
		     ret = bootflow_scan_bootdev(dev, &iter, flags, &bflow);
		     i < 1000 && ret != -ENODEV;
		     i++, ret = bootflow_scan_next(&iter, &bflow)) {
			bflow.err = ret;
			if (!ret)
				num_valid++;
			ret = bootdev_add_bootflow(&bflow);
			if (ret) {
				printf("Out of memory\n");
				return CMD_RET_FAILURE;
			}
			if (list)
				show_bootflow(i, &bflow, errors);
			if (boot && !bflow.err)
				bootflow_run_boot(&iter, &bflow);
		}
	} else {
		if (list) {
			printf("Scanning for bootflows in all bootdevs\n");
			show_header();
		}
		bootstd_clear_glob();

		for (i = 0,
		     ret = bootflow_scan_first(&iter, flags, &bflow);
		     i < 1000 && ret != -ENODEV;
		     i++, ret = bootflow_scan_next(&iter, &bflow)) {
			bflow.err = ret;
			if (!ret)
				num_valid++;
			ret = bootdev_add_bootflow(&bflow);
			if (ret) {
				printf("Out of memory\n");
				return CMD_RET_FAILURE;
			}
			if (list)
				show_bootflow(i, &bflow, errors);
			if (boot && !bflow.err)
				bootflow_run_boot(&iter, &bflow);
		}
	}
	bootflow_iter_uninit(&iter);
	if (list)
		show_footer(i, num_valid);

	return 0;
}

#ifdef CONFIG_CMD_BOOTFLOW_FULL
static int do_bootflow_list(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct udevice *dev;
	struct bootflow *bflow;
	int num_valid = 0;
	bool errors = false;
	int ret, i;

	if (argc > 1 && *argv[1] == '-')
		errors = strchr(argv[1], 'e');

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;
	dev = std->cur_bootdev;

	/* If we have a device, just list bootflows attached to that device */
	if (dev) {
		printf("Showing bootflows for bootdev '%s'\n", dev->name);
		show_header();
		for (ret = bootdev_first_bootflow(dev, &bflow), i = 0;
		     !ret;
		     ret = bootdev_next_bootflow(&bflow), i++) {
			num_valid += bflow->state == BOOTFLOWST_READY;
			show_bootflow(i, bflow, errors);
		}
	} else {
		printf("Showing all bootflows\n");
		show_header();
		for (ret = bootflow_first_glob(&bflow), i = 0;
		     !ret;
		     ret = bootflow_next_glob(&bflow), i++) {
			num_valid += bflow->state == BOOTFLOWST_READY;
			show_bootflow(i, bflow, errors);
		}
	}
	show_footer(i, num_valid);

	return 0;
}

static int do_bootflow_select(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow, *found;
	struct udevice *dev;
	const char *name;
	char *endp;
	int seq, i;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;
;
	if (argc < 2) {
		std->cur_bootflow = NULL;
		return 0;
	}
	dev = std->cur_bootdev;

	name = argv[1];
	seq = simple_strtol(name, &endp, 16);
	found = NULL;

	/*
	 * If we have a bootdev device, only allow selection of bootflows
	 * attached to that device
	 */
	if (dev) {
		for (ret = bootdev_first_bootflow(dev, &bflow), i = 0;
		     !ret;
		     ret = bootdev_next_bootflow(&bflow), i++) {
			if (*endp ? !strcmp(bflow->name, name) : i == seq) {
				found = bflow;
				break;
			}
		}
	} else {
		for (ret = bootflow_first_glob(&bflow), i = 0;
		     !ret;
		     ret = bootflow_next_glob(&bflow), i++) {
			if (*endp ? !strcmp(bflow->name, name) : i == seq) {
				found = bflow;
				break;
			}
		}
	}

	if (!found) {
		printf("Cannot find bootflow '%s' ", name);
		if (dev)
			printf("in bootdev '%s' ", dev->name);
		printf("(err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}
	std->cur_bootflow = found;

	return 0;
}

static int do_bootflow_info(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow;
	bool dump = false;
	int ret;

	if (argc > 1 && *argv[1] == '-')
		dump = strchr(argv[1], 'd');

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;

	if (!std->cur_bootflow) {
		printf("No bootflow selected\n");
		return CMD_RET_FAILURE;
	}
	bflow = std->cur_bootflow;

	printf("Name:      %s\n", bflow->name);
	printf("Device:    %s\n", bflow->dev->name);
	printf("Block dev: %s\n", bflow->blk ? bflow->blk->name : "(none)");
	printf("Method:    %s\n", bflow->method->name);
	printf("State:     %s\n", bootflow_state_get_name(bflow->state));
	printf("Partition: %d\n", bflow->part);
	printf("Subdir:    %s\n", bflow->subdir ? bflow->subdir : "(none)");
	printf("Filename:  %s\n", bflow->fname);
	printf("Buffer:    %lx\n", (ulong)map_to_sysmem(bflow->buf));
	printf("Size:      %x (%d bytes)\n", bflow->size, bflow->size);
	printf("Error:     %d\n", bflow->err);
	if (dump && bflow->buf) {
		/* Set some sort of maximum on the size */
		int size = min(bflow->size, 10 << 10);
		int i;

		printf("Contents:\n\n");
		for (i = 0; i < size; i++) {
			putc(bflow->buf[i]);
			if (!(i % 128) && ctrlc()) {
				printf("...interrupted\n");
				break;
			}
		}
	}

	return 0;
}

static int do_bootflow_boot(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct bootstd_priv *std;
	struct bootflow *bflow;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;

	/*
	 * Require a current bootflow. Users can use 'bootflow scan -b' to
	 * automatically scan and boot, if needed.
	 */
	if (!std->cur_bootflow) {
		printf("No bootflow selected\n");
		return CMD_RET_FAILURE;
	}
	bflow = std->cur_bootflow;
	ret = bootflow_run_boot(NULL, bflow);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}
#endif /* CONFIG_CMD_BOOTFLOW_FULL */

#ifdef CONFIG_SYS_LONGHELP
static char bootflow_help_text[] =
#ifdef CONFIG_CMD_BOOTFLOW_FULL
	"scan [-abel] [bdev]   - scan for valid bootflows (-l list, -a all, -e errors, -b boot)\n"
	"bootflow list [-e]             - list scanned bootflows (-e errors)\n"
	"bootflow select [<num>|<name>] - select a bootflow\n"
	"bootflow info [-d]             - show info on current bootflow (-d dump bootflow)\n"
	"bootflow boot                  - boot current bootflow (or first available if none selected)";
#else
	"scan - boot first available bootflow\n";
#endif
#endif /* CONFIG_SYS_LONGHELP */

U_BOOT_CMD_WITH_SUBCMDS(bootflow, "Boot flows", bootflow_help_text,
	U_BOOT_SUBCMD_MKENT(scan, 3, 1, do_bootflow_scan),
#ifdef CONFIG_CMD_BOOTFLOW_FULL
	U_BOOT_SUBCMD_MKENT(list, 2, 1, do_bootflow_list),
	U_BOOT_SUBCMD_MKENT(select, 2, 1, do_bootflow_select),
	U_BOOT_SUBCMD_MKENT(info, 2, 1, do_bootflow_info),
	U_BOOT_SUBCMD_MKENT(boot, 1, 1, do_bootflow_boot)
#endif
);

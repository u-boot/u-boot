// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/uclass-internal.h>

static void show_devices(struct udevice *dev, int depth, int last_flag)
{
	int i, is_last;
	struct udevice *child;

	/* print the first 20 characters to not break the tree-format. */
	printf(" %-10.10s  %3d  [ %c ]   %-20.20s  ", dev->uclass->uc_drv->name,
	       dev_get_uclass_index(dev, NULL),
	       dev->flags & DM_FLAG_ACTIVATED ? '+' : ' ', dev->driver->name);

	for (i = depth; i >= 0; i--) {
		is_last = (last_flag >> i) & 1;
		if (i) {
			if (is_last)
				printf("    ");
			else
				printf("|   ");
		} else {
			if (is_last)
				printf("`-- ");
			else
				printf("|-- ");
		}
	}

	printf("%s\n", dev->name);

	list_for_each_entry(child, &dev->child_head, sibling_node) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_devices(child, depth + 1, (last_flag << 1) | is_last);
	}
}

void dm_dump_all(void)
{
	struct udevice *root;

	root = dm_root();
	if (root) {
		printf(" Class     Index  Probed  Driver                Name\n");
		printf("-----------------------------------------------------------\n");
		show_devices(root, -1, 0);
	}
}

/**
 * dm_display_line() - Display information about a single device
 *
 * Displays a single line of information with an option prefix
 *
 * @dev:	Device to display
 */
static void dm_display_line(struct udevice *dev, int index)
{
	printf("%-3i %c %s @ %08lx", index,
	       dev->flags & DM_FLAG_ACTIVATED ? '*' : ' ',
	       dev->name, (ulong)map_to_sysmem(dev));
	if (dev->seq != -1 || dev->req_seq != -1)
		printf(", seq %d, (req %d)", dev->seq, dev->req_seq);
	puts("\n");
}

void dm_dump_uclass(void)
{
	struct uclass *uc;
	int ret;
	int id;

	for (id = 0; id < UCLASS_COUNT; id++) {
		struct udevice *dev;
		int i = 0;

		ret = uclass_get(id, &uc);
		if (ret)
			continue;

		printf("uclass %d: %s\n", id, uc->uc_drv->name);
		if (list_empty(&uc->dev_head))
			continue;
		uclass_foreach_dev(dev, uc) {
			dm_display_line(dev, i);
			i++;
		}
		puts("\n");
	}
}

void dm_dump_driver_compat(void)
{
	struct driver *d = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;
	const struct udevice_id *match;

	puts("Driver                Compatible\n");
	puts("--------------------------------\n");
	for (entry = d; entry < d + n_ents; entry++) {
		match = entry->of_match;

		printf("%-20.20s", entry->name);
		if (match) {
			printf("  %s", match->compatible);
			match++;
		}
		printf("\n");

		for (; match && match->compatible; match++)
			printf("%-20.20s  %s\n", "", match->compatible);
	}
}

void dm_dump_drivers(void)
{
	struct driver *d = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;
	struct udevice *udev;
	struct uclass *uc;
	int i;

	puts("Driver                    uid uclass               Devices\n");
	puts("----------------------------------------------------------\n");

	for (entry = d; entry < d + n_ents; entry++) {
		uclass_get(entry->id, &uc);

		printf("%-25.25s %-3.3d %-20.20s ", entry->name, entry->id,
		       uc ? uc->uc_drv->name : "<no uclass>");

		if (!uc) {
			puts("\n");
			continue;
		}

		i = 0;
		uclass_foreach_dev(udev, uc) {
			if (udev->driver != entry)
				continue;
			if (i)
				printf("%-51.51s", "");

			printf("%-25.25s\n", udev->name);
			i++;
		}
		if (!i)
			puts("<none>\n");
	}
}

void dm_dump_static_driver_info(void)
{
	struct driver_info *drv = ll_entry_start(struct driver_info,
						 driver_info);
	const int n_ents = ll_entry_count(struct driver_info, driver_info);
	struct driver_info *entry;

	puts("Driver                    Address\n");
	puts("---------------------------------\n");
	for (entry = drv; entry != drv + n_ents; entry++) {
		printf("%-25.25s @%08lx\n", entry->name,
		       (ulong)map_to_sysmem(entry->platdata));
	}
}

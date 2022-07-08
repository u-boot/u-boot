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
	u32 flags = dev_get_flags(dev);

	/* print the first 20 characters to not break the tree-format. */
	printf(IS_ENABLED(CONFIG_SPL_BUILD) ? " %s  %d  [ %c ]   %s  " :
	       " %-10.10s  %3d  [ %c ]   %-20.20s  ", dev->uclass->uc_drv->name,
	       dev_get_uclass_index(dev, NULL),
	       flags & DM_FLAG_ACTIVATED ? '+' : ' ', dev->driver->name);

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

	device_foreach_child(child, dev) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_devices(child, depth + 1, (last_flag << 1) | is_last);
	}
}

void dm_dump_tree(void)
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
	       dev_get_flags(dev) & DM_FLAG_ACTIVATED ? '*' : ' ',
	       dev->name, (ulong)map_to_sysmem(dev));
	if (dev->seq_ != -1)
		printf(", seq %d", dev_seq(dev));
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
	int ret;
	int i;

	puts("Driver                    uid uclass               Devices\n");
	puts("----------------------------------------------------------\n");

	for (entry = d; entry < d + n_ents; entry++) {
		ret = uclass_get(entry->id, &uc);

		printf("%-25.25s %-3.3d %-20.20s ", entry->name, entry->id,
		       !ret ? uc->uc_drv->name : "<no uclass>");

		if (ret) {
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
	for (entry = drv; entry != drv + n_ents; entry++)
		printf("%-25.25s %p\n", entry->name, entry->plat);
}

void dm_dump_mem(struct dm_stats *stats)
{
	int total, total_delta;
	int i;

	/* Support SPL printf() */
	printf("Struct sizes: udevice %x, driver %x, uclass %x, uc_driver %x\n",
	       (int)sizeof(struct udevice), (int)sizeof(struct driver),
	       (int)sizeof(struct uclass), (int)sizeof(struct uclass_driver));
	printf("Memory: device %x:%x, device names %x, uclass %x:%x\n",
	       stats->dev_count, stats->dev_size, stats->dev_name_size,
	       stats->uc_count, stats->uc_size);
	printf("\n");
	printf("%-15s  %5s  %5s  %5s  %5s  %5s\n", "Attached type", "Count",
	       "Size", "Cur", "Tags", "Save");
	printf("%-15s  %5s  %5s  %5s  %5s  %5s\n", "---------------", "-----",
	       "-----", "-----", "-----", "-----");
	total_delta = 0;
	for (i = 0; i < DM_TAG_ATTACH_COUNT; i++) {
		int cur_size, new_size, delta;

		cur_size = stats->dev_count * sizeof(struct udevice);
		new_size = stats->dev_count * (sizeof(struct udevice) -
			sizeof(void *));
		/*
		 * Let's assume we can fit each dmtag_node into 32 bits. We can
		 * limit the 'tiny tags' feature to SPL with
		 * CONFIG_SPL_SYS_MALLOC_F_LEN <= 64KB, so needing 14 bits to
		 * point to anything in that region (with 4-byte alignment).
		 * So:
		 *    4 bits for tag
		 *    14 bits for offset of dev
		 *    14 bits for offset of data
		 */
		new_size += stats->attach_count[i] * sizeof(u32);
		delta = cur_size - new_size;
		total_delta += delta;
		printf("%-16s %5x %6x %6x %6x %6x (%d)\n", tag_get_name(i),
		       stats->attach_count[i], stats->attach_size[i],
		       cur_size, new_size, delta > 0 ? delta : 0, delta);
	}
	printf("%-16s %5x %6x\n", "uclass", stats->uc_attach_count,
	       stats->uc_attach_size);
	printf("%-16s %5x %6x  %5s  %5s  %6x (%d)\n", "Attached total",
	       stats->attach_count_total + stats->uc_attach_count,
	       stats->attach_size_total + stats->uc_attach_size, "", "",
	       total_delta > 0 ? total_delta : 0, total_delta);
	printf("%-16s %5x %6x\n", "tags", stats->tag_count, stats->tag_size);
	printf("\n");
	printf("Total size: %x (%d)\n", stats->total_size, stats->total_size);
	printf("\n");

	total = stats->total_size;
	total -= total_delta;
	printf("With tags:       %x (%d)\n", total, total);

	/* Use singly linked lists in struct udevice (3 nodes in each) */
	total -= sizeof(void *) * 3 * stats->dev_count;
	printf("- singly-linked: %x (%d)\n", total, total);

	/* Use an index into the struct_driver list instead of a pointer */
	total = total + stats->dev_count * (1 - sizeof(void *));
	printf("- driver index:  %x (%d)\n", total, total);

	/* Same with the uclass */
	total = total + stats->dev_count * (1 - sizeof(void *));
	printf("- uclass index:  %x (%d)\n", total, total);

	/* Drop the device name */
	printf("Drop device name (not SRAM): %x (%d)\n", stats->dev_name_size,
	       stats->dev_name_size);
}

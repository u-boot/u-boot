// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * made from cmd_ext2, which was:
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * made from cmd_reiserfs by
 *
 * (C) Copyright 2003 - 2004
 * Sysgo Real-Time Solutions, AG <www.elinos.com>
 * Pavel Bartusek <pba@sysgo.com>
 */

#include <alist.h>
#include <blk.h>
#include <command.h>
#include <config.h>
#include <dm.h>
#include <env.h>
#include <stddef.h>
#include <part.h>
#include <sort.h>
#include <stdio.h>
#include <vsprintf.h>

enum cmd_part_info {
	CMD_PART_INFO_START = 0,
	CMD_PART_INFO_SIZE,
	CMD_PART_INFO_NUMBER,
	CMD_PART_INFO_NAME,
};

static int do_part_uuid(int argc, char *const argv[])
{
	int part;
	struct blk_desc *dev_desc;
	struct disk_partition info;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 3)
		return CMD_RET_USAGE;

	part = blk_get_device_part_str(argv[0], argv[1], &dev_desc, &info, 0);
	if (part < 0)
		return 1;

	if (argc > 2)
		env_set(argv[2], info.uuid);
	else
		printf("%s\n", info.uuid);

	return 0;
}

static int do_part_list(int argc, char *const argv[])
{
	int ret;
	struct blk_desc *desc;
	char *var = NULL;
	bool bootable = false;
	int i;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc > 2) {
		for (i = 2; i < argc ; i++) {
			if (argv[i][0] == '-') {
				if (!strcmp(argv[i], "-bootable")) {
					bootable = true;
				} else {
					printf("Unknown option %s\n", argv[i]);
					return CMD_RET_USAGE;
				}
			} else {
				var = argv[i];
				break;
			}
		}

		/* Loops should have been exited at the last argument, which
		 * as it contained the variable */
		if (argc != i + 1)
			return CMD_RET_USAGE;
	}

	ret = blk_get_device_by_str(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	if (var != NULL) {
		int p;
		char str[3 * MAX_SEARCH_PARTITIONS] = { '\0', };
		struct disk_partition info;

		for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
			char t[5];
			int r = part_get_info(desc, p, &info);

			if (r != 0)
				continue;

			if (bootable && !info.bootable)
				continue;

			sprintf(t, "%s%x", str[0] ? " " : "", p);
			strcat(str, t);
		}
		env_set(var, str);
		return 0;
	}

	part_print(desc);

	return 0;
}

static int do_part_info(int argc, char *const argv[], enum cmd_part_info param)
{
	struct blk_desc *desc;
	struct disk_partition info;
	char buf[512] = { 0 };
	char *endp;
	int part;
	int err;
	int ret;

	if (argc < 3)
		return CMD_RET_USAGE;
	if (argc > 4)
		return CMD_RET_USAGE;

	ret = blk_get_device_by_str(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	part = simple_strtoul(argv[2], &endp, 0);
	if (*endp == '\0') {
		err = part_get_info(desc, part, &info);
		if (err)
			return 1;
	} else {
		part = part_get_info_by_name(desc, argv[2], &info);
		if (part < 0)
			return 1;
	}

	switch (param) {
	case CMD_PART_INFO_START:
		snprintf(buf, sizeof(buf), LBAF, info.start);
		break;
	case CMD_PART_INFO_SIZE:
		snprintf(buf, sizeof(buf), LBAF, info.size);
		break;
	case CMD_PART_INFO_NUMBER:
		snprintf(buf, sizeof(buf), "0x%x", part);
		break;
	case CMD_PART_INFO_NAME:
		snprintf(buf, sizeof(buf), "%s", info.name);
		break;
	default:
		printf("** Unknown cmd_part_info value: %d\n", param);
		return 1;
	}

	if (argc > 3)
		env_set(argv[3], buf);
	else
		printf("%s\n", buf);

	return 0;
}

static int do_part_start(int argc, char *const argv[])
{
	return do_part_info(argc, argv, CMD_PART_INFO_START);
}

static int do_part_size(int argc, char *const argv[])
{
	return do_part_info(argc, argv, CMD_PART_INFO_SIZE);
}

static int do_part_number(int argc, char *const argv[])
{
	return do_part_info(argc, argv, CMD_PART_INFO_NUMBER);
}

static int do_part_name(int argc, char *const argv[])
{
	return do_part_info(argc, argv, CMD_PART_INFO_NAME);
}

static int do_part_set(int argc, char *const argv[])
{
	const char *devname, *partstr, *typestr;
	struct blk_desc *desc;
	int dev;

	if (argc < 3)
		return CMD_RET_USAGE;

	/* Look up the device */
	devname = argv[0];
	partstr = argv[1];
	typestr = argv[2];
	dev = blk_get_device_by_str(devname, partstr, &desc);
	if (dev < 0) {
		printf("** Bad device specification %s %s **\n", devname,
		       partstr);
		return CMD_RET_FAILURE;
	}

	desc->part_type = part_get_type_by_name(typestr);
	if (!desc->part_type) {
		printf("Unknown partition type '%s'\n", typestr);
		return CMD_RET_FAILURE;
	}
	part_print(desc);

	return 0;
}

#ifdef CONFIG_PARTITION_TYPE_GUID
static int do_part_type(int argc, char *const argv[])
{
	int part;
	struct blk_desc *dev_desc;
	struct disk_partition info;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 3)
		return CMD_RET_USAGE;

	part = blk_get_device_part_str(argv[0], argv[1], &dev_desc, &info, 0);
	if (part < 0)
		return 1;

	if (argc > 2)
		env_set(argv[2], info.type_guid);
	else
		printf("%s\n", info.type_guid);

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(CMD_PART_DUPCHECK)
struct part_seen {
	char uuid[UUID_STR_LEN + 1];
	char name[PART_NAME_LEN + 1];
	struct udevice *dev;
	int part;
};

static int compare_uuid(const void *a, const void *b)
{
	const struct part_seen *pa = a;
	const struct part_seen *pb = b;

	/* Sort empty UUIDs to the end */
	if (!pa->uuid[0] && !pb->uuid[0])
		return 0;
	if (!pa->uuid[0])
		return 1;
	if (!pb->uuid[0])
		return -1;

	return strcmp(pa->uuid, pb->uuid);
}

static int compare_name(const void *a, const void *b)
{
	const struct part_seen *pa = a;
	const struct part_seen *pb = b;

	/* Sort empty names to the end */
	if (!pa->name[0] && !pb->name[0])
		return 0;
	if (!pa->name[0])
		return 1;
	if (!pb->name[0])
		return -1;

	return strcmp(pa->name, pb->name);
}

/**
 * detect_duplicates() - Sort and detect duplicate fields in the partition list
 *
 * Sorts the seen list using the given comparator, then performs a single
 * linear pass to find and report consecutive duplicate entries.
 *
 * @seen:       The list of collected partition entries
 * @cmp:        Comparator function for qsort (must sort empties to the end)
 * @field_label: Human-readable label for reporting (e.g. "PARTUUID", "PARTLABEL")
 * @field_off:  Offset of the string field within struct part_seen
 * @dup_groups: Output: number of distinct duplicate values found
 * @total:      Total number of partitions that have this field set
 */
static void detect_duplicates(struct alist *seen,
			      int (*cmp)(const void *, const void *),
			      const char *field_label, size_t field_off,
			      int *dup_groups, int total)
{
	const struct part_seen *pi;
	const struct part_seen *pj;
	const char *field_i;
	const char *field_j;
	int occurrences;
	int count = seen->count;
	int dup_parts = 0;

	*dup_groups = 0;

	qsort(seen->data, count, sizeof(struct part_seen), cmp);

	for (int i = 0; i < count; i++) {
		pi = alist_get(seen, i, struct part_seen);
		field_i = (const char *)pi + field_off;
		occurrences = 1;

		if (!field_i[0])
			break;  /* Reached empty fields at the end */

		/* Count consecutive duplicates */
		while (i + occurrences < count) {
			pj = alist_get(seen, i + occurrences, struct part_seen);
			field_j = (const char *)pj + field_off;

			if (strcmp(field_i, field_j) != 0)
				break;
			occurrences++;
		}

		if (occurrences > 1) {
			printf("Warning: duplicate %s %s (%d copies)\n",
			       field_label, field_i, occurrences);
			for (int j = 0; j < occurrences; j++) {
				pj = alist_get(seen, i + j, struct part_seen);
				printf("  found on %s:%d\n",
				       pj->dev->name,
				       pj->part);
			}

			(*dup_groups)++;
			dup_parts += occurrences;
			i += occurrences - 1;
		}
	}

	if (*dup_groups)
		printf("Found %d duplicate %s(s) (%d total copies) among %d partitions\n",
		       *dup_groups, field_label, dup_parts, total);
}

static int do_part_dupcheck(int argc, char *const argv[])
{
	struct alist seen;
	int uuid_count = 0;
	int label_count = 0;
	int duplicate_uuids = 0;
	int duplicate_labels = 0;
	struct udevice *dev;

	if (argc)
		return CMD_RET_USAGE;

	if (!blk_count_devices(BLKF_BOTH)) {
		printf("No block devices found\n");
		return CMD_RET_SUCCESS;
	}

	alist_init_struct(&seen, struct part_seen);

	/* First pass: collect all partitions with UUIDs or labels */
	blk_foreach_probe(BLKF_BOTH, dev) {
		struct blk_desc *desc = dev_get_uclass_plat(dev);

		for (int part = 1; part <= MAX_SEARCH_PARTITIONS; part++) {
			struct disk_partition info;
			struct part_seen entry;
			bool has_uuid;
			bool has_label;

			if (part_get_info(desc, part, &info))
				continue;
			has_uuid = disk_partition_uuid(&info)[0] != '\0';
			has_label = info.name[0] != '\0';
			if (!has_uuid && !has_label)
				continue;

			memset(&entry, 0, sizeof(entry));
			if (has_uuid)
				strlcpy(entry.uuid, disk_partition_uuid(&info),
					sizeof(entry.uuid));
			if (has_label)
				strlcpy(entry.name, (const char *)info.name,
					sizeof(entry.name));
			entry.dev = dev;
			entry.part = part;

			if (has_uuid)
				uuid_count++;
			if (has_label)
				label_count++;

			if (!alist_add(&seen, entry)) {
				printf("Unable to grow dupcheck list\n");
				alist_uninit(&seen);
				return CMD_RET_FAILURE;
			}
		}
	}

	if (!seen.count) {
		printf("No partitions with UUID or label found\n");
		alist_uninit(&seen);
		return CMD_RET_SUCCESS;
	}

	/* Detect duplicate UUIDs */
	detect_duplicates(&seen, compare_uuid, "PARTUUID",
			  offsetof(struct part_seen, uuid),
			  &duplicate_uuids, uuid_count);

	/* Detect duplicate partition labels */
	detect_duplicates(&seen, compare_name, "PARTLABEL",
			  offsetof(struct part_seen, name),
			  &duplicate_labels, label_count);

	if (!duplicate_uuids && !duplicate_labels)
		printf("No duplicate PARTUUIDs or PARTLABELs found (%d UUIDs, %d labels)\n",
		       uuid_count, label_count);
	else if (!duplicate_uuids)
		printf("No duplicate PARTUUIDs found (%d UUIDs)\n", uuid_count);
	else if (!duplicate_labels)
		printf("No duplicate PARTLABELs found (%d labels)\n", label_count);

	alist_uninit(&seen);

	return (duplicate_uuids || duplicate_labels) ?
		CMD_RET_FAILURE : CMD_RET_SUCCESS;
}
#endif

static int do_part_types(int argc, char * const argv[])
{
	struct part_driver *drv = ll_entry_start(struct part_driver,
						 part_driver);
	const int n_ents = ll_entry_count(struct part_driver, part_driver);
	struct part_driver *entry;
	int i = 0;

	puts("Supported partition tables");

	for (entry = drv; entry != drv + n_ents; entry++) {
		printf("%c %s", i ? ',' : ':', entry->name);
		i++;
	}
	if (!i)
		puts(": <none>");
	puts("\n");
	return CMD_RET_SUCCESS;
}

static int do_part(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "uuid"))
		return do_part_uuid(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "list"))
		return do_part_list(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "start"))
		return do_part_start(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "size"))
		return do_part_size(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "number"))
		return do_part_number(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "name"))
		return do_part_name(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "types"))
		return do_part_types(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "set"))
		return do_part_set(argc - 2, argv + 2);
#ifdef CONFIG_PARTITION_TYPE_GUID
	else if (!strcmp(argv[1], "type"))
		return do_part_type(argc - 2, argv + 2);
#endif
#if CONFIG_IS_ENABLED(CMD_PART_DUPCHECK)
	else if (!strcmp(argv[1], "dupcheck"))
		return do_part_dupcheck(argc - 2, argv + 2);
#endif
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	part,	CONFIG_SYS_MAXARGS,	1,	do_part,
	"disk partition related commands",
	"uuid <interface> <dev>:<part>\n"
	"    - print partition UUID\n"
	"part uuid <interface> <dev>:<part> <varname>\n"
	"    - set environment variable to partition UUID\n"
	"part list <interface> <dev>\n"
	"    - print a device's partition table\n"
	"part list <interface> <dev> [flags] <varname>\n"
	"    - set environment variable to the list of partitions\n"
	"      flags can be -bootable (list only bootable partitions)\n"
	"part start <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the start of the partition (in blocks)\n"
	"      part can be either partition number or partition name\n"
	"part size <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the size of the partition (in blocks)\n"
	"      part can be either partition number or partition name\n"
	"part number <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the partition number using the partition name\n"
	"      part must be specified as partition name\n"
	"part name <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the partition name using the partition number\n"
	"      part must be specified as partition number\n"
#ifdef CONFIG_PARTITION_TYPE_GUID
	"part type <interface> <dev>:<part>\n"
	"    - print partition type\n"
	"part type <interface> <dev>:<part> <varname>\n"
	"    - set environment variable to partition type\n"
#endif
	"part set <interface> <dev> type\n"
	"    - set partition type for a device\n"
	"part types\n"
	"    - list supported partition table types"
#if CONFIG_IS_ENABLED(CMD_PART_DUPCHECK)
	"\n"
	"part dupcheck\n"
	"    - scan all block devices for duplicate partition UUIDs and labels"
#endif
);

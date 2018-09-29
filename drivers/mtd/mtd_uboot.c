// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#include <common.h>
#include <linux/mtd/mtd.h>
#include <jffs2/jffs2.h> /* Legacy */

/**
 * mtd_search_alternate_name - Search an alternate name for @mtdname thanks to
 *                             the mtdids legacy environment variable.
 *
 * The mtdids string is a list of comma-separated 'dev_id=mtd_id' tupples.
 * Check if one of the mtd_id matches mtdname, in this case save dev_id in
 * altname.
 *
 * @mtdname: Current MTD device name
 * @altname: Alternate name to return
 * @max_len: Length of the alternate name buffer
 *
 * @return 0 on success, an error otherwise.
 */
int mtd_search_alternate_name(const char *mtdname, char *altname,
			      unsigned int max_len)
{
	const char *mtdids, *equal, *comma, *dev_id, *mtd_id;
	int dev_id_len, mtd_id_len;

	mtdids = env_get("mtdids");
	if (!mtdids)
		return -EINVAL;

	do {
		/* Find the '=' sign */
		dev_id = mtdids;
		equal = strchr(dev_id, '=');
		if (!equal)
			break;
		dev_id_len = equal - mtdids;
		mtd_id = equal + 1;

		/* Find the end of the tupple */
		comma = strchr(mtdids, ',');
		if (comma)
			mtd_id_len = comma - mtd_id;
		else
			mtd_id_len = &mtdids[strlen(mtdids)] - mtd_id + 1;

		if (!dev_id_len || !mtd_id_len)
			return -EINVAL;

		if (dev_id_len + 1 > max_len)
			continue;

		/* Compare the name we search with the current mtd_id */
		if (!strncmp(mtdname, mtd_id, mtd_id_len)) {
			strncpy(altname, dev_id, dev_id_len);
			altname[dev_id_len] = 0;

			return 0;
		}

		/* Go to the next tupple */
		mtdids = comma + 1;
	} while (comma);

	return -EINVAL;
}

/* Legacy */

static int get_part(const char *partname, int *idx, loff_t *off, loff_t *size,
		loff_t *maxsize, int devtype)
{
#ifdef CONFIG_CMD_MTDPARTS
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = mtdparts_init();
	if (ret)
		return ret;

	ret = find_dev_and_part(partname, &dev, &pnum, &part);
	if (ret)
		return ret;

	if (dev->id->type != devtype) {
		printf("not same typ %d != %d\n", dev->id->type, devtype);
		return -1;
	}

	*off = part->offset;
	*size = part->size;
	*maxsize = part->size;
	*idx = dev->id->num;

	return 0;
#else
	puts("mtdparts support missing.\n");
	return -1;
#endif
}

int mtd_arg_off(const char *arg, int *idx, loff_t *off, loff_t *size,
		loff_t *maxsize, int devtype, uint64_t chipsize)
{
	if (!str2off(arg, off))
		return get_part(arg, idx, off, size, maxsize, devtype);

	if (*off >= chipsize) {
		puts("Offset exceeds device limit\n");
		return -1;
	}

	*maxsize = chipsize - *off;
	*size = *maxsize;
	return 0;
}

int mtd_arg_off_size(int argc, char *const argv[], int *idx, loff_t *off,
		     loff_t *size, loff_t *maxsize, int devtype,
		     uint64_t chipsize)
{
	int ret;

	if (argc == 0) {
		*off = 0;
		*size = chipsize;
		*maxsize = *size;
		goto print;
	}

	ret = mtd_arg_off(argv[0], idx, off, size, maxsize, devtype,
			  chipsize);
	if (ret)
		return ret;

	if (argc == 1)
		goto print;

	if (!str2off(argv[1], size)) {
		printf("'%s' is not a number\n", argv[1]);
		return -1;
	}

	if (*size > *maxsize) {
		puts("Size exceeds partition or device limit\n");
		return -1;
	}

print:
	printf("device %d ", *idx);
	if (*size == chipsize)
		puts("whole chip\n");
	else
		printf("offset 0x%llx, size 0x%llx\n",
		       (unsigned long long)*off, (unsigned long long)*size);
	return 0;
}

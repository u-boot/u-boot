/*
 * cmd_gpt.c -- GPT (GUID Partition Table) handling command
 *
 * Copyright (C) 2015
 * Lukasz Majewski <l.majewski@majess.pl>
 *
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 * author: Piotr Wilczek <p.wilczek@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <command.h>
#include <part_efi.h>
#include <exports.h>
#include <linux/ctype.h>
#include <div64.h>
#include <memalign.h>

#ifndef CONFIG_PARTITION_UUIDS
#error CONFIG_PARTITION_UUIDS must be enabled for CONFIG_CMD_GPT to be enabled
#endif

/**
 * extract_env(): Expand env name from string format '&{env_name}'
 *                and return pointer to the env (if the env is set)
 *
 * @param str - pointer to string
 * @param env - pointer to pointer to extracted env
 *
 * @return - zero on successful expand and env is set
 */
static int extract_env(const char *str, char **env)
{
	int ret = -1;
	char *e, *s;
#ifdef CONFIG_RANDOM_UUID
	char uuid_str[UUID_STR_LEN + 1];
#endif

	if (!str || strlen(str) < 4)
		return -1;

	if (!((strncmp(str, "${", 2) == 0) && (str[strlen(str) - 1] == '}')))
		return -1;

	s = strdup(str);
	if (s == NULL)
		return -1;

	memset(s + strlen(s) - 1, '\0', 1);
	memmove(s, s + 2, strlen(s) - 1);

	e = getenv(s);
	if (e == NULL) {
#ifdef CONFIG_RANDOM_UUID
		debug("%s unset. ", str);
		gen_rand_uuid_str(uuid_str, UUID_STR_FORMAT_STD);
		setenv(s, uuid_str);

		e = getenv(s);
		if (e) {
			debug("Set to random.\n");
			ret = 0;
		} else {
			debug("Can't get random UUID.\n");
		}
#else
		debug("%s unset.\n", str);
#endif
	} else {
		debug("%s get from environment.\n", str);
		ret = 0;
	}

	*env = e;
	free(s);

	return ret;
}

/**
 * extract_val(): Extract value from a key=value pair list (comma separated).
 *                Only value for the given key is returend.
 *                Function allocates memory for the value, remember to free!
 *
 * @param str - pointer to string with key=values pairs
 * @param key - pointer to the key to search for
 *
 * @return - pointer to allocated string with the value
 */
static char *extract_val(const char *str, const char *key)
{
	char *v, *k;
	char *s, *strcopy;
	char *new = NULL;

	strcopy = strdup(str);
	if (strcopy == NULL)
		return NULL;

	s = strcopy;
	while (s) {
		v = strsep(&s, ",");
		if (!v)
			break;
		k = strsep(&v, "=");
		if (!k)
			break;
		if  (strcmp(k, key) == 0) {
			new = strdup(v);
			break;
		}
	}

	free(strcopy);

	return new;
}

/**
 * found_key(): Found key without value in parameter list (comma separated).
 *
 * @param str - pointer to string with key
 * @param key - pointer to the key to search for
 *
 * @return - true on found key
 */
static bool found_key(const char *str, const char *key)
{
	char *k;
	char *s, *strcopy;
	bool result = false;

	strcopy = strdup(str);
	if (!strcopy)
		return NULL;

	s = strcopy;
	while (s) {
		k = strsep(&s, ",");
		if (!k)
			break;
		if  (strcmp(k, key) == 0) {
			result = true;
			break;
		}
	}

	free(strcopy);

	return result;
}

/**
 * set_gpt_info(): Fill partition information from string
 *		function allocates memory, remember to free!
 *
 * @param dev_desc - pointer block device descriptor
 * @param str_part - pointer to string with partition information
 * @param str_disk_guid - pointer to pointer to allocated string with disk guid
 * @param partitions - pointer to pointer to allocated partitions array
 * @param parts_count - number of partitions
 *
 * @return - zero on success, otherwise error
 *
 */
static int set_gpt_info(struct blk_desc *dev_desc,
			const char *str_part,
			char **str_disk_guid,
			disk_partition_t **partitions,
			u8 *parts_count)
{
	char *tok, *str, *s;
	int i;
	char *val, *p;
	int p_count;
	disk_partition_t *parts;
	int errno = 0;
	uint64_t size_ll, start_ll;
	lbaint_t offset = 0;

	debug("%s:  lba num: 0x%x %d\n", __func__,
	      (unsigned int)dev_desc->lba, (unsigned int)dev_desc->lba);

	if (str_part == NULL)
		return -1;

	str = strdup(str_part);

	/* extract disk guid */
	s = str;
	val = extract_val(str, "uuid_disk");
	if (!val) {
#ifdef CONFIG_RANDOM_UUID
		*str_disk_guid = malloc(UUID_STR_LEN + 1);
		gen_rand_uuid_str(*str_disk_guid, UUID_STR_FORMAT_STD);
#else
		free(str);
		return -2;
#endif
	} else {
		val = strsep(&val, ";");
		if (extract_env(val, &p))
			p = val;
		*str_disk_guid = strdup(p);
		free(val);
		/* Move s to first partition */
		strsep(&s, ";");
	}
	if (strlen(s) == 0)
		return -3;

	i = strlen(s) - 1;
	if (s[i] == ';')
		s[i] = '\0';

	/* calculate expected number of partitions */
	p_count = 1;
	p = s;
	while (*p) {
		if (*p++ == ';')
			p_count++;
	}

	/* allocate memory for partitions */
	parts = calloc(sizeof(disk_partition_t), p_count);

	/* retrieve partitions data from string */
	for (i = 0; i < p_count; i++) {
		tok = strsep(&s, ";");

		if (tok == NULL)
			break;

		/* uuid */
		val = extract_val(tok, "uuid");
		if (!val) {
			/* 'uuid' is optional if random uuid's are enabled */
#ifdef CONFIG_RANDOM_UUID
			gen_rand_uuid_str(parts[i].uuid, UUID_STR_FORMAT_STD);
#else
			errno = -4;
			goto err;
#endif
		} else {
			if (extract_env(val, &p))
				p = val;
			if (strlen(p) >= sizeof(parts[i].uuid)) {
				printf("Wrong uuid format for partition %d\n", i);
				errno = -4;
				goto err;
			}
			strcpy((char *)parts[i].uuid, p);
			free(val);
		}
#ifdef CONFIG_PARTITION_TYPE_GUID
		/* guid */
		val = extract_val(tok, "type");
		if (val) {
			/* 'type' is optional */
			if (extract_env(val, &p))
				p = val;
			if (strlen(p) >= sizeof(parts[i].type_guid)) {
				printf("Wrong type guid format for partition %d\n",
				       i);
				errno = -4;
				goto err;
			}
			strcpy((char *)parts[i].type_guid, p);
			free(val);
		}
#endif
		/* name */
		val = extract_val(tok, "name");
		if (!val) { /* name is mandatory */
			errno = -4;
			goto err;
		}
		if (extract_env(val, &p))
			p = val;
		if (strlen(p) >= sizeof(parts[i].name)) {
			errno = -4;
			goto err;
		}
		strcpy((char *)parts[i].name, p);
		free(val);

		/* size */
		val = extract_val(tok, "size");
		if (!val) { /* 'size' is mandatory */
			errno = -4;
			goto err;
		}
		if (extract_env(val, &p))
			p = val;
		if ((strcmp(p, "-") == 0)) {
			/* remove first usable lba and last block */
			parts[i].size = dev_desc->lba - 34  - 1 - offset;
		} else {
			size_ll = ustrtoull(p, &p, 0);
			parts[i].size = lldiv(size_ll, dev_desc->blksz);
		}

		free(val);

		/* start address */
		val = extract_val(tok, "start");
		if (val) { /* start address is optional */
			if (extract_env(val, &p))
				p = val;
			start_ll = ustrtoull(p, &p, 0);
			parts[i].start = lldiv(start_ll, dev_desc->blksz);
			free(val);
		}

		offset += parts[i].size + parts[i].start;

		/* bootable */
		if (found_key(tok, "bootable"))
			parts[i].bootable = 1;
	}

	*parts_count = p_count;
	*partitions = parts;
	free(str);

	return 0;
err:
	free(str);
	free(*str_disk_guid);
	free(parts);

	return errno;
}

static int gpt_default(struct blk_desc *blk_dev_desc, const char *str_part)
{
	int ret;
	char *str_disk_guid;
	u8 part_count = 0;
	disk_partition_t *partitions = NULL;

	/* fill partitions */
	ret = set_gpt_info(blk_dev_desc, str_part,
			&str_disk_guid, &partitions, &part_count);
	if (ret) {
		if (ret == -1)
			printf("No partition list provided\n");
		if (ret == -2)
			printf("Missing disk guid\n");
		if ((ret == -3) || (ret == -4))
			printf("Partition list incomplete\n");
		return -1;
	}

	/* save partitions layout to disk */
	ret = gpt_restore(blk_dev_desc, str_disk_guid, partitions, part_count);
	free(str_disk_guid);
	free(partitions);

	return ret;
}

static int gpt_verify(struct blk_desc *blk_dev_desc, const char *str_part)
{
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_head, 1,
				     blk_dev_desc->blksz);
	disk_partition_t *partitions = NULL;
	gpt_entry *gpt_pte = NULL;
	char *str_disk_guid;
	u8 part_count = 0;
	int ret = 0;

	/* fill partitions */
	ret = set_gpt_info(blk_dev_desc, str_part,
			&str_disk_guid, &partitions, &part_count);
	if (ret) {
		if (ret == -1) {
			printf("No partition list provided - only basic check\n");
			ret = gpt_verify_headers(blk_dev_desc, gpt_head,
						 &gpt_pte);
			goto out;
		}
		if (ret == -2)
			printf("Missing disk guid\n");
		if ((ret == -3) || (ret == -4))
			printf("Partition list incomplete\n");
		return -1;
	}

	/* Check partition layout with provided pattern */
	ret = gpt_verify_partitions(blk_dev_desc, partitions, part_count,
				    gpt_head, &gpt_pte);
	free(str_disk_guid);
	free(partitions);
 out:
	free(gpt_pte);
	return ret;
}

/**
 * do_gpt(): Perform GPT operations
 *
 * @param cmdtp - command name
 * @param flag
 * @param argc
 * @param argv
 *
 * @return zero on success; otherwise error
 */
static int do_gpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = CMD_RET_SUCCESS;
	int dev = 0;
	char *ep;
	struct blk_desc *blk_dev_desc = NULL;

	if (argc < 4 || argc > 5)
		return CMD_RET_USAGE;

	dev = (int)simple_strtoul(argv[3], &ep, 10);
	if (!ep || ep[0] != '\0') {
		printf("'%s' is not a number\n", argv[3]);
		return CMD_RET_USAGE;
	}
	blk_dev_desc = blk_get_dev(argv[2], dev);
	if (!blk_dev_desc) {
		printf("%s: %s dev %d NOT available\n",
		       __func__, argv[2], dev);
		return CMD_RET_FAILURE;
	}

	if ((strcmp(argv[1], "write") == 0) && (argc == 5)) {
		printf("Writing GPT: ");
		ret = gpt_default(blk_dev_desc, argv[4]);
	} else if ((strcmp(argv[1], "verify") == 0)) {
		ret = gpt_verify(blk_dev_desc, argv[4]);
		printf("Verify GPT: ");
	} else {
		return CMD_RET_USAGE;
	}

	if (ret) {
		printf("error!\n");
		return CMD_RET_FAILURE;
	}

	printf("success!\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(gpt, CONFIG_SYS_MAXARGS, 1, do_gpt,
	"GUID Partition Table",
	"<command> <interface> <dev> <partitions_list>\n"
	" - GUID partition table restoration and validity check\n"
	" Restore or verify GPT information on a device connected\n"
	" to interface\n"
	" Example usage:\n"
	" gpt write mmc 0 $partitions\n"
	" gpt verify mmc 0 $partitions\n"
);

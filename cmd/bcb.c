// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Eugeniu Rosca <rosca.eugeniu@gmail.com>
 *
 * Command to read/modify/write Android BCB fields
 */

#include <android_bootloader_message.h>
#include <bcb.h>
#include <command.h>
#include <env.h>
#include <android_ab.h>
#include <display_options.h>
#include <log.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>
#include <vsprintf.h>
#include <linux/err.h>

static const char * const fields[] = {
	"command",
	"status",
	"recovery",
	"stage"
};

static struct bootloader_message bcb __aligned(ARCH_DMA_MINALIGN) = { { 0 } };
static struct disk_partition partition_data;

static struct blk_desc *block;
static struct disk_partition *partition = &partition_data;

static int bcb_not_loaded(void)
{
	printf("Error: Please, load BCB first!\n");
	return -1;
}

static int bcb_field_get(const char *name, char **fieldp, int *sizep)
{
	if (!strcmp(name, "command")) {
		*fieldp = bcb.command;
		*sizep = sizeof(bcb.command);
	} else if (!strcmp(name, "status")) {
		*fieldp = bcb.status;
		*sizep = sizeof(bcb.status);
	} else if (!strcmp(name, "recovery")) {
		*fieldp = bcb.recovery;
		*sizep = sizeof(bcb.recovery);
	} else if (!strcmp(name, "stage")) {
		*fieldp = bcb.stage;
		*sizep = sizeof(bcb.stage);
	} else if (!strcmp(name, "reserved")) {
		*fieldp = bcb.reserved;
		*sizep = sizeof(bcb.reserved);
	} else {
		printf("Error: Unknown bcb field '%s'\n", name);
		return -1;
	}

	return 0;
}

static void __bcb_reset(void)
{
	block = NULL;
	partition = &partition_data;
	memset(&partition_data, 0, sizeof(struct disk_partition));
	memset(&bcb, 0, sizeof(struct bootloader_message));
}

static int __bcb_initialize(const char *iface, int devnum, const char *partp)
{
	char *endp;
	int part, ret;

	block = blk_get_dev(iface, devnum);
	if (!block) {
		ret = -ENODEV;
		goto err_read_fail;
	}

	/*
	 * always select the first hwpart in case another
	 * blk operation selected a different hwpart
	 */
	ret = blk_dselect_hwpart(block, 0);
	if (IS_ERR_VALUE(ret)) {
		ret = -ENODEV;
		goto err_read_fail;
	}

	part = simple_strtoul(partp, &endp, 0);
	if (*endp == '\0') {
		ret = part_get_info(block, part, partition);
		if (ret)
			goto err_read_fail;
	} else {
		part = part_get_info_by_name(block, partp, partition);
		if (part < 0) {
			ret = part;
			goto err_read_fail;
		}
	}

	return CMD_RET_SUCCESS;

err_read_fail:
	printf("Error: %s %d:%s read failed (%d)\n", iface, devnum,
	       partition->name, ret);
	__bcb_reset();
	return CMD_RET_FAILURE;
}

static int __bcb_load(void)
{
	u64 cnt;
	int ret;

	cnt = DIV_ROUND_UP(sizeof(struct bootloader_message), partition->blksz);
	if (cnt > partition->size)
		goto err_too_small;

	if (blk_dread(block, partition->start, cnt, &bcb) != cnt) {
		ret = -EIO;
		goto err_read_fail;
	}

	debug("%s: Loaded from %d %d:%s\n", __func__, block->uclass_id,
	      block->devnum, partition->name);

	return CMD_RET_SUCCESS;
err_read_fail:
	printf("Error: %d %d:%s read failed (%d)\n", block->uclass_id,
	       block->devnum, partition->name, ret);
	goto err;
err_too_small:
	printf("Error: %d %d:%s too small!", block->uclass_id,
	       block->devnum, partition->name);
err:
	__bcb_reset();
	return CMD_RET_FAILURE;
}

static int do_bcb_load(struct cmd_tbl *cmdtp, int flag, int argc,
		       char * const argv[])
{
	int ret;
	int devnum;
	char *endp;
	char *iface = "mmc";

	if (argc < 3)
		return CMD_RET_USAGE;

	if (argc == 4) {
		iface = argv[1];
		argc--;
		argv++;
	}

	devnum = simple_strtoul(argv[1], &endp, 0);
	if (*endp != '\0') {
		printf("Error: Device id '%s' not a number\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	ret = __bcb_initialize(iface, devnum, argv[2]);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	return __bcb_load();
}

static int __bcb_set(const char *fieldp, const char *valp)
{
	int size, len;
	char *field, *str, *found, *tmp;

	if (bcb_field_get(fieldp, &field, &size))
		return CMD_RET_FAILURE;

	len = strlen(valp);
	if (len >= size) {
		printf("Error: sizeof('%s') = %d >= %d = sizeof(bcb.%s)\n",
		       valp, len, size, fieldp);
		return CMD_RET_FAILURE;
	}
	str = strdup(valp);
	if (!str) {
		printf("Error: Out of memory while strdup\n");
		return CMD_RET_FAILURE;
	}

	tmp = str;
	field[0] = '\0';
	while ((found = strsep(&tmp, ":"))) {
		if (field[0] != '\0')
			strcat(field, "\n");
		strcat(field, found);
	}
	free(str);

	return CMD_RET_SUCCESS;
}

static int do_bcb_set(struct cmd_tbl *cmdtp, int flag, int argc,
		      char * const argv[])
{
	if (argc < 3)
		return CMD_RET_USAGE;

	if (!block)
		return bcb_not_loaded();

	return __bcb_set(argv[1], argv[2]);
}

static int do_bcb_clear(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	int size;
	char *field;

	if (!block)
		return bcb_not_loaded();

	if (argc == 1) {
		memset(&bcb, 0, sizeof(bcb));
		return CMD_RET_SUCCESS;
	}

	if (bcb_field_get(argv[1], &field, &size))
		return CMD_RET_FAILURE;

	memset(field, 0, size);

	return CMD_RET_SUCCESS;
}

static int do_bcb_test(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int size;
	char *field;
	char *op;

	if (argc < 4)
		return CMD_RET_USAGE;

	if (!block)
		return bcb_not_loaded();

	op = argv[2];

	if (bcb_field_get(argv[1], &field, &size))
		return CMD_RET_FAILURE;

	if (*op == '=' && *(op + 1) == '\0') {
		if (!strncmp(argv[3], field, size))
			return CMD_RET_SUCCESS;
		else
			return CMD_RET_FAILURE;
	} else if (*op == '~' && *(op + 1) == '\0') {
		if (!strstr(field, argv[3]))
			return CMD_RET_FAILURE;
		else
			return CMD_RET_SUCCESS;
	} else {
		printf("Error: Unknown operator '%s'\n", op);
	}

	return CMD_RET_FAILURE;
}

static int do_bcb_dump(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int size;
	char *field;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!block)
		return bcb_not_loaded();

	if (bcb_field_get(argv[1], &field, &size))
		return CMD_RET_FAILURE;

	print_buffer((ulong)field - (ulong)&bcb, (void *)field, 1, size, 16);

	return CMD_RET_SUCCESS;
}

static int __bcb_store(void)
{
	u64 cnt;
	int ret;

	cnt = DIV_ROUND_UP(sizeof(struct bootloader_message), partition->blksz);

	if (blk_dwrite(block, partition->start, cnt, &bcb) != cnt) {
		ret = -EIO;
		goto err;
	}

	return CMD_RET_SUCCESS;
err:
	printf("Error: %d %d:%s write failed (%d)\n", block->uclass_id,
	       block->devnum, partition->name, ret);

	return CMD_RET_FAILURE;
}

static int do_bcb_store(struct cmd_tbl *cmdtp, int flag, int argc,
			char * const argv[])
{
	if (!block)
		return bcb_not_loaded();

	return __bcb_store();
}

int bcb_find_partition_and_load(const char *iface, int devnum, char *partp)
{
	int ret;

	__bcb_reset();

	ret = __bcb_initialize(iface, devnum, partp);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	return __bcb_load();
}

int bcb_load(struct blk_desc *block_description, struct disk_partition *disk_partition)
{
	__bcb_reset();

	block = block_description;
	partition = disk_partition;

	return __bcb_load();
}

int bcb_set(enum bcb_field field, const char *value)
{
	if (field > BCB_FIELD_STAGE)
		return CMD_RET_FAILURE;
	return __bcb_set(fields[field], value);
}

int bcb_get(enum bcb_field field, char *value_out, size_t value_size)
{
	int size;
	char *field_value;

	if (field > BCB_FIELD_STAGE)
		return CMD_RET_FAILURE;
	if (bcb_field_get(fields[field], &field_value, &size))
		return CMD_RET_FAILURE;

	strlcpy(value_out, field_value, value_size);

	return CMD_RET_SUCCESS;
}

int bcb_store(void)
{
	return __bcb_store();
}

void bcb_reset(void)
{
	__bcb_reset();
}

__maybe_unused static int do_bcb_ab_select(struct cmd_tbl *cmdtp,
					   int flag, int argc,
					   char * const argv[])
{
	int ret;
	struct blk_desc *dev_desc;
	struct disk_partition part_info;
	char slot[2];
	bool dec_tries = true;

	if (argc < 4)
		return CMD_RET_USAGE;

	for (int i = 4; i < argc; i++) {
		if (!strcmp(argv[i], "--no-dec"))
			dec_tries = false;
		else
			return CMD_RET_USAGE;
	}

	/* Lookup the "misc" partition from argv[2] and argv[3] */
	if (part_get_info_by_dev_and_name_or_num(argv[2], argv[3],
						 &dev_desc, &part_info,
						 false) < 0) {
		return CMD_RET_FAILURE;
	}

	ret = ab_select_slot(dev_desc, &part_info, dec_tries);
	if (ret < 0) {
		printf("Android boot failed, error %d.\n", ret);
		return CMD_RET_FAILURE;
	}

	/* Android standard slot names are 'a', 'b', ... */
	slot[0] = BOOT_SLOT_NAME(ret);
	slot[1] = '\0';
	env_set(argv[1], slot);
	printf("ANDROID: Booting slot: %s\n", slot);

	return CMD_RET_SUCCESS;
}

__maybe_unused static int do_bcb_ab_dump(struct cmd_tbl *cmdtp,
					 int flag, int argc,
					 char *const argv[])
{
	int ret;
	struct blk_desc *dev_desc;
	struct disk_partition part_info;

	if (argc < 3)
		return CMD_RET_USAGE;

	if (part_get_info_by_dev_and_name_or_num(argv[1], argv[2],
						 &dev_desc, &part_info,
						 false) < 0) {
		return CMD_RET_FAILURE;
	}

	ret = ab_dump_abc(dev_desc, &part_info);
	if (ret < 0) {
		printf("Cannot dump ABC data, error %d.\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(bcb,
	"load <interface> <dev> <part>  - load  BCB from <interface> <dev>:<part>\n"
	"load <dev> <part>              - load  BCB from mmc <dev>:<part>\n"
	"bcb set   <field> <val>        - set   BCB <field> to <val>\n"
	"bcb clear [<field>]            - clear BCB <field> or all fields\n"
	"bcb test  <field> <op> <val>   - test  BCB <field> against <val>\n"
	"bcb dump  <field>              - dump  BCB <field>\n"
	"bcb store                      - store BCB back to <interface>\n"
	"\n"
#if IS_ENABLED(CONFIG_ANDROID_AB)
	"bcb ab_select -\n"
	"    Select the slot used to boot from and register the boot attempt.\n"
	"    <slot_var_name> <interface> <dev[:part|#part_name]> [--no-dec]\n"
	"    - Load the slot metadata from the partition 'part' on\n"
	"      device type 'interface' instance 'dev' and store the active\n"
	"      slot in the 'slot_var_name' variable. This also updates the\n"
	"      Android slot metadata with a boot attempt, which can cause\n"
	"      successive calls to this function to return a different result\n"
	"      if the returned slot runs out of boot attempts.\n"
	"    - If 'part_name' is passed, preceded with a # instead of :, the\n"
	"      partition name whose label is 'part_name' will be looked up in\n"
	"      the partition table. This is commonly the \"misc\" partition.\n"
	"    - If '--no-dec' is set, the number of tries remaining will not\n"
	"      decremented for the selected boot slot\n"
	"\n"
	"bcb ab_dump -\n"
	"    Dump boot_control information from specific partition.\n"
	"    <interface> <dev[:part|#part_name]>\n"
	"\n"
#endif
	"Legend:\n"
	"<interface> - storage device interface (virtio, mmc, etc)\n"
	"<dev>       - storage device index containing the BCB partition\n"
	"<part>      - partition index or name containing the BCB\n"
	"<field>     - one of {command,status,recovery,stage,reserved}\n"
	"<op>        - the binary operator used in 'bcb test':\n"
	"              '=' returns true if <val> matches the string stored in <field>\n"
	"              '~' returns true if <val> matches a subset of <field>'s string\n"
	"<val>       - string/text provided as input to bcb {set,test}\n"
	"              NOTE: any ':' character in <val> will be replaced by line feed\n"
	"              during 'bcb set' and used as separator by upper layers\n"
);

U_BOOT_CMD_WITH_SUBCMDS(bcb,
	"Load/set/clear/test/dump/store Android BCB fields", bcb_help_text,
	U_BOOT_SUBCMD_MKENT(load, 4, 1, do_bcb_load),
	U_BOOT_SUBCMD_MKENT(set, 3, 1, do_bcb_set),
	U_BOOT_SUBCMD_MKENT(clear, 2, 1, do_bcb_clear),
	U_BOOT_SUBCMD_MKENT(test, 4, 1, do_bcb_test),
	U_BOOT_SUBCMD_MKENT(dump, 2, 1, do_bcb_dump),
	U_BOOT_SUBCMD_MKENT(store, 1, 1, do_bcb_store),
#if IS_ENABLED(CONFIG_ANDROID_AB)
	U_BOOT_SUBCMD_MKENT(ab_select, 5, 1, do_bcb_ab_select),
	U_BOOT_SUBCMD_MKENT(ab_dump, 3, 1, do_bcb_ab_dump),
#endif
);

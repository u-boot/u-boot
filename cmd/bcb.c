// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Eugeniu Rosca <rosca.eugeniu@gmail.com>
 *
 * Command to read/modify/write Android BCB fields
 */

#include <android_bootloader_message.h>
#include <bcb.h>
#include <command.h>
#include <common.h>
#include <display_options.h>
#include <log.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>

enum bcb_cmd {
	BCB_CMD_LOAD,
	BCB_CMD_FIELD_SET,
	BCB_CMD_FIELD_CLEAR,
	BCB_CMD_FIELD_TEST,
	BCB_CMD_FIELD_DUMP,
	BCB_CMD_STORE,
};

static int bcb_dev = -1;
static int bcb_part = -1;
static struct bootloader_message bcb __aligned(ARCH_DMA_MINALIGN) = { { 0 } };

static int bcb_cmd_get(char *cmd)
{
	if (!strcmp(cmd, "load"))
		return BCB_CMD_LOAD;
	if (!strcmp(cmd, "set"))
		return BCB_CMD_FIELD_SET;
	if (!strcmp(cmd, "clear"))
		return BCB_CMD_FIELD_CLEAR;
	if (!strcmp(cmd, "test"))
		return BCB_CMD_FIELD_TEST;
	if (!strcmp(cmd, "store"))
		return BCB_CMD_STORE;
	if (!strcmp(cmd, "dump"))
		return BCB_CMD_FIELD_DUMP;
	else
		return -1;
}

static int bcb_is_misused(int argc, char *const argv[])
{
	int cmd = bcb_cmd_get(argv[0]);

	switch (cmd) {
	case BCB_CMD_LOAD:
	case BCB_CMD_FIELD_SET:
		if (argc != 3)
			goto err;
		break;
	case BCB_CMD_FIELD_TEST:
		if (argc != 4)
			goto err;
		break;
	case BCB_CMD_FIELD_CLEAR:
		if (argc != 1 && argc != 2)
			goto err;
		break;
	case BCB_CMD_STORE:
		if (argc != 1)
			goto err;
		break;
	case BCB_CMD_FIELD_DUMP:
		if (argc != 2)
			goto err;
		break;
	default:
		printf("Error: 'bcb %s' not supported\n", argv[0]);
		return -1;
	}

	if (cmd != BCB_CMD_LOAD && (bcb_dev < 0 || bcb_part < 0)) {
		printf("Error: Please, load BCB first!\n");
		return -1;
	}

	return 0;
err:
	printf("Error: Bad usage of 'bcb %s'\n", argv[0]);

	return -1;
}

static int bcb_field_get(char *name, char **fieldp, int *sizep)
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

static int __bcb_load(int devnum, const char *partp)
{
	struct blk_desc *desc;
	struct disk_partition info;
	u64 cnt;
	char *endp;
	int part, ret;

	desc = blk_get_devnum_by_type(IF_TYPE_MMC, devnum);
	if (!desc) {
		ret = -ENODEV;
		goto err_read_fail;
	}

	part = simple_strtoul(partp, &endp, 0);
	if (*endp == '\0') {
		ret = part_get_info(desc, part, &info);
		if (ret)
			goto err_read_fail;
	} else {
		part = part_get_info_by_name(desc, partp, &info);
		if (part < 0) {
			ret = part;
			goto err_read_fail;
		}
	}

	cnt = DIV_ROUND_UP(sizeof(struct bootloader_message), info.blksz);
	if (cnt > info.size)
		goto err_too_small;

	if (blk_dread(desc, info.start, cnt, &bcb) != cnt) {
		ret = -EIO;
		goto err_read_fail;
	}

	bcb_dev = desc->devnum;
	bcb_part = part;
	debug("%s: Loaded from mmc %d:%d\n", __func__, bcb_dev, bcb_part);

	return CMD_RET_SUCCESS;
err_read_fail:
	printf("Error: mmc %d:%s read failed (%d)\n", devnum, partp, ret);
	goto err;
err_too_small:
	printf("Error: mmc %d:%s too small!", devnum, partp);
	goto err;
err:
	bcb_dev = -1;
	bcb_part = -1;

	return CMD_RET_FAILURE;
}

static int do_bcb_load(struct cmd_tbl *cmdtp, int flag, int argc,
		       char * const argv[])
{
	char *endp;
	int devnum = simple_strtoul(argv[1], &endp, 0);

	if (*endp != '\0') {
		printf("Error: Device id '%s' not a number\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	return __bcb_load(devnum, argv[2]);
}

static int __bcb_set(char *fieldp, const char *valp)
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
	return __bcb_set(argv[1], argv[2]);
}

static int do_bcb_clear(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	int size;
	char *field;

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
	char *op = argv[2];

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

	if (bcb_field_get(argv[1], &field, &size))
		return CMD_RET_FAILURE;

	print_buffer((ulong)field - (ulong)&bcb, (void *)field, 1, size, 16);

	return CMD_RET_SUCCESS;
}

static int __bcb_store(void)
{
	struct blk_desc *desc;
	struct disk_partition info;
	u64 cnt;
	int ret;

	desc = blk_get_devnum_by_type(IF_TYPE_MMC, bcb_dev);
	if (!desc) {
		ret = -ENODEV;
		goto err;
	}

	ret = part_get_info(desc, bcb_part, &info);
	if (ret)
		goto err;

	cnt = DIV_ROUND_UP(sizeof(struct bootloader_message), info.blksz);

	if (blk_dwrite(desc, info.start, cnt, &bcb) != cnt) {
		ret = -EIO;
		goto err;
	}

	return CMD_RET_SUCCESS;
err:
	printf("Error: mmc %d:%d write failed (%d)\n", bcb_dev, bcb_part, ret);

	return CMD_RET_FAILURE;
}

static int do_bcb_store(struct cmd_tbl *cmdtp, int flag, int argc,
			char * const argv[])
{
	return __bcb_store();
}

int bcb_write_reboot_reason(int devnum, char *partp, const char *reasonp)
{
	int ret;

	ret = __bcb_load(devnum, partp);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	ret = __bcb_set("command", reasonp);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	ret = __bcb_store();
	if (ret != CMD_RET_SUCCESS)
		return ret;

	return 0;
}

static struct cmd_tbl cmd_bcb_sub[] = {
	U_BOOT_CMD_MKENT(load, CONFIG_SYS_MAXARGS, 1, do_bcb_load, "", ""),
	U_BOOT_CMD_MKENT(set, CONFIG_SYS_MAXARGS, 1, do_bcb_set, "", ""),
	U_BOOT_CMD_MKENT(clear, CONFIG_SYS_MAXARGS, 1, do_bcb_clear, "", ""),
	U_BOOT_CMD_MKENT(test, CONFIG_SYS_MAXARGS, 1, do_bcb_test, "", ""),
	U_BOOT_CMD_MKENT(dump, CONFIG_SYS_MAXARGS, 1, do_bcb_dump, "", ""),
	U_BOOT_CMD_MKENT(store, CONFIG_SYS_MAXARGS, 1, do_bcb_store, "", ""),
};

static int do_bcb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_bcb_sub, ARRAY_SIZE(cmd_bcb_sub));
	if (!c)
		return CMD_RET_USAGE;

	if (bcb_is_misused(argc, argv)) {
		/*
		 * We try to improve the user experience by reporting the
		 * root-cause of misusage, so don't return CMD_RET_USAGE,
		 * since the latter prints out the full-blown help text
		 */
		return CMD_RET_FAILURE;
	}

	return c->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	bcb, CONFIG_SYS_MAXARGS, 1, do_bcb,
	"Load/set/clear/test/dump/store Android BCB fields",
	"load  <dev> <part>       - load  BCB from mmc <dev>:<part>\n"
	"bcb set   <field> <val>      - set   BCB <field> to <val>\n"
	"bcb clear [<field>]          - clear BCB <field> or all fields\n"
	"bcb test  <field> <op> <val> - test  BCB <field> against <val>\n"
	"bcb dump  <field>            - dump  BCB <field>\n"
	"bcb store                    - store BCB back to mmc\n"
	"\n"
	"Legend:\n"
	"<dev>   - MMC device index containing the BCB partition\n"
	"<part>  - MMC partition index or name containing the BCB\n"
	"<field> - one of {command,status,recovery,stage,reserved}\n"
	"<op>    - the binary operator used in 'bcb test':\n"
	"          '=' returns true if <val> matches the string stored in <field>\n"
	"          '~' returns true if <val> matches a subset of <field>'s string\n"
	"<val>   - string/text provided as input to bcb {set,test}\n"
	"          NOTE: any ':' character in <val> will be replaced by line feed\n"
	"          during 'bcb set' and used as separator by upper layers\n"
);

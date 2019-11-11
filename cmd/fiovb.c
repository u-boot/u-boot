/*
 * (C) Copyright 2019, Foundries.IO
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>
#include <env.h>
#include <image.h>
#include <malloc.h>
#include <mmc.h>
#include <fiovb.h>

static struct fiovb_ops *fiovb_ops;

int do_fiovb_init(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long mmc_dev;

	if (argc != 2)
		return CMD_RET_USAGE;

	mmc_dev = simple_strtoul(argv[1], NULL, 16);

	if (fiovb_ops)
		fiovb_ops_free(fiovb_ops);

	fiovb_ops = fiovb_ops_alloc(mmc_dev);
	if (fiovb_ops)
		return CMD_RET_SUCCESS;

	printf("Failed to initialize fiovb\n");

	return CMD_RET_FAILURE;
}

int do_fiovb_read_pvalue(struct cmd_tbl *cmdtp, int flag, int argc,
		         char * const argv[])
{
	const char *name;
	size_t bytes;
	size_t bytes_read;
	void *buffer;
	char *endp;
	char fiovb_name[30] = { 0 }; /* fiovb.name */
	char fiovb_val[32] = { 0 };

	if (!fiovb_ops) {
		printf("Foundries.IO Verified Boot is not initialized, run 'fiovb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	bytes = simple_strtoul(argv[2], &endp, 10);
	if (*endp && *endp != '\n')
		return CMD_RET_USAGE;

	buffer = malloc(bytes);
	if (!buffer)
		return CMD_RET_FAILURE;

	if (fiovb_ops->read_persistent_value(fiovb_ops, name, bytes, buffer,
					   &bytes_read) == FIOVB_IO_RESULT_OK) {
		printf("Read %zu bytes, value = %s\n", bytes_read,
		       (char *)buffer);
		/* Mirror fiovb variables into the environment */
		snprintf(fiovb_name, sizeof(fiovb_name), "fiovb.%s", name);
		snprintf(fiovb_val, sizeof(fiovb_val), "%s", (char *)buffer);
		env_set(fiovb_name, fiovb_val);
		free(buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read persistent value\n");

	free(buffer);

	return CMD_RET_FAILURE;
}

int do_fiovb_write_pvalue(struct cmd_tbl *cmdtp, int flag, int argc,
			  char * const argv[])
{
	const char *name;
	const char *value;
	char fiovb_name[30] = { 0 }; /* fiovb.name */

	if (!fiovb_ops) {
		printf("Foundries.IO Verified Boot is not initialized, run 'fiovb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	value = argv[2];

	if (fiovb_ops->write_persistent_value(fiovb_ops, name, strlen(value) + 1,
					    (const uint8_t *)value) ==
	    FIOVB_IO_RESULT_OK) {
		printf("Wrote %zu bytes\n", strlen(value) + 1);
		snprintf(fiovb_name, sizeof(fiovb_name), "fiovb.%s", name);
		env_set(fiovb_name, value);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to write persistent value\n");

	return CMD_RET_FAILURE;
}

int do_fiovb_delete_pvalue(struct cmd_tbl *cmdtp, int flag, int argc,
			   char * const argv[])
{
	const char *name;
	char fiovb_name[30] = { 0 }; /* fiovb.name */

	if (!fiovb_ops) {
		printf("Foundries.IO Verified Boot is not initialized, run 'fiovb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 2)
		return CMD_RET_USAGE;

	name = argv[1];

	if (fiovb_ops->delete_persistent_value(fiovb_ops, name) ==
	    FIOVB_IO_RESULT_OK) {
		printf("Deleted persistent value %s\n", name);
		snprintf(fiovb_name, sizeof(fiovb_name), "fiovb.%s", name);
		env_set(fiovb_name, NULL);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to delete persistent value\n");

	return CMD_RET_FAILURE;
}

static struct cmd_tbl cmd_fiovb[] = {
	U_BOOT_CMD_MKENT(init, 2, 0, do_fiovb_init, "", ""),
	U_BOOT_CMD_MKENT(read_pvalue, 3, 0, do_fiovb_read_pvalue, "", ""),
	U_BOOT_CMD_MKENT(write_pvalue, 3, 0, do_fiovb_write_pvalue, "", ""),
	U_BOOT_CMD_MKENT(delete_pvalue, 2, 0, do_fiovb_delete_pvalue, "", ""),
};

static int do_fiovb(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_fiovb, ARRAY_SIZE(cmd_fiovb));

	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;

	if (flag == CMD_FLAG_REPEAT)
		return CMD_RET_FAILURE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	fiovb, 29, 0, do_fiovb,
	"Provides commands for testing Foundries.IO verified boot functionality"
	" - supported value names: m4hash, bootcount, upgrade_available and rollback",
	"init <dev> - initialize fiovb for <dev>\n"
	"read_pvalue <name> <bytes> - read a persistent value <name>\n"
	"write_pvalue <name> <value> - write a persistent value <name>\n"
	"delete_pvalue <name> - delete a persistent value <name>\n"
	);

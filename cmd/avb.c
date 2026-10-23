// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#include <avb_verify.h>
#include <command.h>
#include <env.h>
#include <hexdump.h>
#include <image.h>
#include <malloc.h>
#include <mmc.h>

#define AVB_BOOTARGS	"avb_bootargs"

static struct AvbOps *avb_ops;

int do_avb_init(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	enum uclass_id uclass_id;
	int dev_num;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	/* "avb init <dev>" keeps its old meaning of mmc <dev> */
	if (argc == 2) {
		uclass_id = UCLASS_MMC;
		dev_num = hextoul(argv[1], NULL);
	} else {
		uclass_id = blk_get_uclass_id(argv[1]);
		if (uclass_id == UCLASS_INVALID) {
			printf("Unknown interface '%s'\n", argv[1]);
			return CMD_RET_FAILURE;
		}
		dev_num = hextoul(argv[2], NULL);
	}

	if (avb_ops)
		avb_ops_free(avb_ops);

	avb_ops = avb_ops_alloc(uclass_id, dev_num);
	if (avb_ops)
		return CMD_RET_SUCCESS;
	else
		printf("Can't allocate AvbOps");

	printf("Failed to initialize AVB\n");

	return CMD_RET_FAILURE;
}

int do_avb_read_part(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	const char *part;
	s64 offset;
	size_t bytes, bytes_read = 0;
	void *buffer;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 5)
		return CMD_RET_USAGE;

	part = argv[1];
	offset = hextoul(argv[2], NULL);
	bytes = hextoul(argv[3], NULL);
	buffer = (void *)hextoul(argv[4], NULL);

	ret = avb_ops->read_from_partition(avb_ops, part, offset,
					   bytes, buffer, &bytes_read);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Read %zu bytes\n", bytes_read);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read from partition '%s', err = %d\n",
	       part, ret);

	return CMD_RET_FAILURE;
}

int do_avb_read_part_hex(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	const char *part;
	s64 offset;
	size_t bytes, bytes_read = 0;
	char *buffer;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 4)
		return CMD_RET_USAGE;

	part = argv[1];
	offset = hextoul(argv[2], NULL);
	bytes = hextoul(argv[3], NULL);

	buffer = malloc(bytes);
	if (!buffer) {
		printf("Failed to tlb_allocate buffer for data\n");
		return CMD_RET_FAILURE;
	}
	memset(buffer, 0, bytes);

	ret = avb_ops->read_from_partition(avb_ops, part, offset,
					   bytes, buffer, &bytes_read);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Requested %zu, read %zu bytes\n", bytes, bytes_read);
		printf("Data: ");
		for (int i = 0; i < bytes_read; i++)
			printf("%02X", buffer[i]);

		printf("\n");

		free(buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read from partition '%s', err = %d\n",
	       part, ret);

	free(buffer);
	return CMD_RET_FAILURE;
}

int do_avb_write_part(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	const char *part;
	s64 offset;
	size_t bytes;
	void *buffer;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 5)
		return CMD_RET_USAGE;

	part = argv[1];
	offset = hextoul(argv[2], NULL);
	bytes = hextoul(argv[3], NULL);
	buffer = (void *)hextoul(argv[4], NULL);

	ret = avb_ops->write_to_partition(avb_ops, part, offset,
					  bytes, buffer);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Wrote %zu bytes\n", bytes);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to write in partition '%s', err = %d\n",
	       part, ret);

	return CMD_RET_FAILURE;
}

int do_avb_read_rb(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	size_t index;
	u64 rb_idx;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 2)
		return CMD_RET_USAGE;

	index = (size_t)hextoul(argv[1], NULL);

	ret = avb_ops->read_rollback_index(avb_ops, index, &rb_idx);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Rollback index: %llx\n", rb_idx);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read rollback index id = %zu, err = %d\n",
	       index, ret);

	return CMD_RET_FAILURE;
}

int do_avb_write_rb(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	size_t index;
	u64 rb_idx;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	index = (size_t)hextoul(argv[1], NULL);
	rb_idx = hextoul(argv[2], NULL);

	ret = avb_ops->write_rollback_index(avb_ops, index, rb_idx);
	if (ret == AVB_IO_RESULT_OK)
		return CMD_RET_SUCCESS;

	printf("Failed to write rollback index id = %zu, err = %d\n",
	       index, ret);

	return CMD_RET_FAILURE;
}

int do_avb_get_uuid(struct cmd_tbl *cmdtp, int flag,
		    int argc, char *const argv[])
{
	const char *part;
	char buffer[UUID_STR_LEN + 1];
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 2)
		return CMD_RET_USAGE;

	part = argv[1];

	ret = avb_ops->get_unique_guid_for_partition(avb_ops, part,
						     buffer,
						     UUID_STR_LEN + 1);
	if (ret == AVB_IO_RESULT_OK) {
		printf("'%s' UUID: %s\n", part, buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read partition '%s' UUID, err = %d\n",
	       part, ret);

	return CMD_RET_FAILURE;
}

int do_avb_verify_part(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	const char * const requested_partitions[] = {"boot", NULL};
	AvbSlotVerifyResult slot_result;
	AvbSlotVerifyData *out_data;
	enum avb_boot_state boot_state;
	char *cmdline;
	char *extra_args;
	char *slot_suffix = "";
	int ret;

	bool unlocked = false;
	int res = CMD_RET_FAILURE;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc < 1 || argc > 2)
		return CMD_RET_USAGE;

	if (argc == 2)
		slot_suffix = argv[1];

	printf("## Android Verified Boot 2.0 version %s\n",
	       avb_version_string());

	ret = avb_ops->read_is_device_unlocked(avb_ops, &unlocked);
	if (ret != AVB_IO_RESULT_OK) {
		printf("Can't determine device lock state, err = %d\n",
		       ret);
		return CMD_RET_FAILURE;
	}

	slot_result =
		avb_slot_verify(avb_ops,
				requested_partitions,
				slot_suffix,
				unlocked,
				AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE,
				&out_data);

	/*
	 * LOCKED devices with custom root of trust setup is not supported (YELLOW)
	 */
	if (slot_result == AVB_SLOT_VERIFY_RESULT_OK) {
		printf("Verification passed successfully\n");

		/*
		 * ORANGE state indicates that device may be freely modified.
		 * Device integrity is left to the user to verify out-of-band.
		 */
		if (unlocked)
			boot_state = AVB_ORANGE;
		else
			boot_state = AVB_GREEN;

		/* export boot state to AVB_BOOTARGS env var */
		extra_args = avb_set_state(avb_ops, boot_state);
		if (extra_args)
			cmdline = append_cmd_line(out_data->cmdline,
						  extra_args);
		else
			cmdline = out_data->cmdline;

		env_set(AVB_BOOTARGS, cmdline);

		res = CMD_RET_SUCCESS;
	} else {
		printf("Verification failed, reason: %s\n", str_avb_slot_error(slot_result));
	}

	if (out_data)
		avb_slot_verify_data_free(out_data);

	return res;
}

int do_avb_is_unlocked(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	bool unlock;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 1) {
		printf("--%s(-1)\n", __func__);
		return CMD_RET_USAGE;
	}

	ret = avb_ops->read_is_device_unlocked(avb_ops, &unlock);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Unlocked = %d\n", unlock);
		return CMD_RET_SUCCESS;
	}

	printf("Can't determine device lock state, err = %d\n",
	       ret);

	return CMD_RET_FAILURE;
}

int do_avb_read_pvalue(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	const char *name;
	size_t bytes;
	size_t bytes_read;
	void *buffer;
	char *endp;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	bytes = dectoul(argv[2], &endp);
	if (*endp && *endp != '\n')
		return CMD_RET_USAGE;

	buffer = malloc(bytes);
	if (!buffer)
		return CMD_RET_FAILURE;

	ret = avb_ops->read_persistent_value(avb_ops, name, bytes,
					     buffer, &bytes_read);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Read %zu bytes, value = %s\n", bytes_read,
		       (char *)buffer);
		free(buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read persistent value, err = %d\n", ret);

	free(buffer);

	return CMD_RET_FAILURE;
}

int do_avb_write_pvalue(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	const char *name;
	const char *value;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	value = argv[2];

	ret = avb_ops->write_persistent_value(avb_ops, name,
					      strlen(value) + 1,
					      (const uint8_t *)value);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Wrote %zu bytes\n", strlen(value) + 1);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to write persistent value `%s` = `%s`, err = %d\n",
	       name, value, ret);

	return CMD_RET_FAILURE;
}

int do_avb_read_pvalue_hex(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	const char *name;
	size_t bytes;
	size_t bytes_read;
	size_t i;
	u8 *buffer;
	char *endp;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	bytes = dectoul(argv[2], &endp);
	if (*endp && *endp != '\n')
		return CMD_RET_USAGE;

	buffer = malloc(bytes);
	if (!buffer)
		return CMD_RET_FAILURE;

	ret = avb_ops->read_persistent_value(avb_ops, name, bytes,
					     buffer, &bytes_read);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Read %zu bytes, value = ", bytes_read);
		for (i = 0; i < bytes_read; i++)
			printf("%02x", buffer[i]);
		printf("\n");
		free(buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read persistent value, err = %d\n", ret);

	free(buffer);

	return CMD_RET_FAILURE;
}

int do_avb_write_pvalue_hex(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	const char *name;
	const char *hex;
	size_t len, bytes;
	u8 *buffer;
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	hex = argv[2];
	len = strlen(hex);
	if (!len || len % 2) {
		printf("Value must be an even number of hex digits\n");
		return CMD_RET_USAGE;
	}
	bytes = len / 2;

	buffer = malloc(bytes);
	if (!buffer)
		return CMD_RET_FAILURE;

	if (hex2bin(buffer, hex, bytes)) {
		printf("Value is not a valid hex string\n");
		free(buffer);
		return CMD_RET_USAGE;
	}

	ret = avb_ops->write_persistent_value(avb_ops, name, bytes, buffer);
	free(buffer);
	if (ret == AVB_IO_RESULT_OK) {
		printf("Wrote %zu bytes\n", bytes);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to write persistent value `%s`, err = %d\n", name, ret);

	return CMD_RET_FAILURE;
}

#ifdef CONFIG_OPTEE_TA_AVB
static int avb_set_lock(bool locked)
{
	int ret;

	if (!avb_ops) {
		printf("AVB is not initialized, please run 'avb init <id>'\n");
		return CMD_RET_FAILURE;
	}

	ret = avb_write_lock_state(avb_ops, locked);
	if (ret != AVB_IO_RESULT_OK) {
		printf("Failed to %s the device, err = %d\n",
		       locked ? "lock" : "unlock", ret);
		return CMD_RET_FAILURE;
	}

	printf("Device %s, rollback indexes reset\n",
	       locked ? "locked" : "unlocked");

	return CMD_RET_SUCCESS;
}

int do_avb_lock(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return avb_set_lock(true);
}

int do_avb_unlock(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	return avb_set_lock(false);
}
#endif

static struct cmd_tbl cmd_avb[] = {
	U_BOOT_CMD_MKENT(init, 3, 0, do_avb_init, "", ""),
	U_BOOT_CMD_MKENT(read_rb, 2, 0, do_avb_read_rb, "", ""),
	U_BOOT_CMD_MKENT(write_rb, 3, 0, do_avb_write_rb, "", ""),
	U_BOOT_CMD_MKENT(is_unlocked, 1, 0, do_avb_is_unlocked, "", ""),
	U_BOOT_CMD_MKENT(get_uuid, 2, 0, do_avb_get_uuid, "", ""),
	U_BOOT_CMD_MKENT(read_part, 5, 0, do_avb_read_part, "", ""),
	U_BOOT_CMD_MKENT(read_part_hex, 4, 0, do_avb_read_part_hex, "", ""),
	U_BOOT_CMD_MKENT(write_part, 5, 0, do_avb_write_part, "", ""),
	U_BOOT_CMD_MKENT(verify, 2, 0, do_avb_verify_part, "", ""),
#ifdef CONFIG_OPTEE_TA_AVB
	U_BOOT_CMD_MKENT(read_pvalue, 3, 0, do_avb_read_pvalue, "", ""),
	U_BOOT_CMD_MKENT(write_pvalue, 3, 0, do_avb_write_pvalue, "", ""),
	U_BOOT_CMD_MKENT(read_pvalue_hex, 3, 0, do_avb_read_pvalue_hex, "", ""),
	U_BOOT_CMD_MKENT(write_pvalue_hex, 3, 0, do_avb_write_pvalue_hex, "", ""),
	U_BOOT_CMD_MKENT(lock, 1, 0, do_avb_lock, "", ""),
	U_BOOT_CMD_MKENT(unlock, 1, 0, do_avb_unlock, "", ""),
#endif
};

static int do_avb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_avb, ARRAY_SIZE(cmd_avb));

	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;

	if (flag == CMD_FLAG_REPEAT)
		return CMD_RET_FAILURE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	avb, 29, 0, do_avb,
	"Provides commands for testing Android Verified Boot 2.0 functionality",
	"init [<interface>] <dev> - initialize avb2 for hex <dev> on <interface>\n"
	"    (mmc, nvme, usb, ...; defaults to mmc)\n"
	"avb read_rb <num> - read rollback index at location <num>\n"
	"avb write_rb <num> <rb> - write rollback index <rb> to <num>\n"
	"avb is_unlocked - returns unlock status of the device\n"
	"avb get_uuid <partname> - read and print uuid of partition <part>\n"
	"avb read_part <partname> <offset> <num> <addr> - read <num> bytes from\n"
	"    partition <partname> to buffer <addr>\n"
	"avb read_part_hex <partname> <offset> <num> - read <num> bytes from\n"
	"    partition <partname> and print to stdout\n"
	"avb write_part <partname> <offset> <num> <addr> - write <num> bytes to\n"
	"    <partname> by <offset> using data from <addr>\n"
#ifdef CONFIG_OPTEE_TA_AVB
	"avb read_pvalue <name> <bytes> - read a persistent value <name>\n"
	"avb write_pvalue <name> <value> - write a persistent value <name>\n"
	"avb read_pvalue_hex <name> <bytes> - read a persistent value <name>\n"
	"    and print it as hex\n"
	"avb write_pvalue_hex <name> <hex> - write the bytes given as a hex\n"
	"    string to persistent value <name>\n"
	"avb lock - lock the device (verification errors stop the boot);\n"
	"    resets all rollback indexes\n"
	"avb unlock - unlock the device; resets all rollback indexes\n"
#endif
	"avb verify [slot_suffix] - run verification process using hash data\n"
	"    from vbmeta structure\n"
	"    [slot_suffix] - _a, _b, etc (if vbmeta partition is slotted)\n"
	);

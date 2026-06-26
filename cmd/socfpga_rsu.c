// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 * SoC FPGA RSU console command (Stratix 10 / Agilex family).
 */

#include <command.h>
#include <limits.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <rsu_console.h>
#include <vsprintf.h>
#include <asm/arch/rsu.h>

/*
 * Strictly parse a numeric argv[] value.
 *
 * U-Boot's dectoul()/simple_strtoul*() silently return partial values
 * for "12xyz" and wrap on u64 overflow without indication; either could
 * feed a malformed argv into slot tables or pointers. Parse digit-by-
 * digit so we can reject both, and allow a single trailing '\n'.
 */
static int rsu_parse_num(const char *s, unsigned int base, u64 *out)
{
	const char *start;
	u64 result = 0;

	if (!s || !*s || !out)
		return -EINVAL;
	if (base != 10 && base != 16)
		return -EINVAL;

	/* Accept an optional "0x" prefix for base 16 (matches U-Boot usage). */
	if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		s += 2;

	start = s;
	while (*s) {
		unsigned int digit;
		char c = *s;

		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'a' && c <= 'f')
			digit = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			digit = c - 'A' + 10;
		else
			break;

		if (digit >= base)
			break;

		/* Pre-division overflow guard: result*base + digit must fit a u64. */
		if (result > (ULLONG_MAX - digit) / base)
			return -ERANGE;

		result = result * base + digit;
		s++;
	}

	if (s == start)
		return -EINVAL;
	if (*s != '\0' && !(*s == '\n' && s[1] == '\0'))
		return -EINVAL;

	*out = result;
	return 0;
}

/* Parse a decimal slot index into an int, rejecting values > INT_MAX. */
static int rsu_parse_slot(const char *s, int *out)
{
	u64 v;

	if (rsu_parse_num(s, 10, &v) || v > INT_MAX)
		return -EINVAL;
	*out = (int)v;
	return 0;
}

/*
 * Parse a hex size into int (rejects > INT_MAX); a negative value would
 * be treated as a huge unsigned length downstream.
 */
static int rsu_parse_hex_int(const char *s, int *out)
{
	u64 v;

	if (rsu_parse_num(s, 16, &v) || v > INT_MAX)
		return -EINVAL;
	*out = (int)v;
	return 0;
}

/* Parse a hex size argument into unsigned int, rejecting values > UINT_MAX. */
static int rsu_parse_hex_uint(const char *s, unsigned int *out)
{
	u64 v;

	if (rsu_parse_num(s, 16, &v) || v > UINT_MAX)
		return -EINVAL;
	*out = (unsigned int)v;
	return 0;
}

/* Parse a hex u32, rejecting values > U32_MAX. */
static int rsu_parse_hex_u32(const char *s, u32 *out)
{
	u64 v;

	if (rsu_parse_num(s, 16, &v) || v > U32_MAX)
		return -EINVAL;
	*out = (u32)v;
	return 0;
}

static int slot_count(int argc, char * const argv[])
{
	int count;

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	count = rsu_slot_count();
	rsu_exit();

	if (count < 0)
		return CMD_RET_FAILURE;

	printf("Number of slots = %d.\n", count);

	return CMD_RET_SUCCESS;
}

static int slot_by_name(int argc, char * const argv[])
{
	char *name;
	int slot;

	if (argc != 2)
		return CMD_RET_USAGE;

	name = argv[1];

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	slot = rsu_slot_by_name(name);
	rsu_exit();

	if (slot < 0)
		return CMD_RET_FAILURE;

	printf("Slot name '%s' is %d.\n", name, slot);
	return CMD_RET_SUCCESS;
}

static int slot_get_info(int argc, char * const argv[])
{
	int slot;
	struct rsu_slot_info info;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_get_info(slot, &info);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	printf("NAME: %s\n", info.name);
	printf("OFFSET: 0x%016llX\n", info.offset);
	printf("SIZE: 0x%08X\n", info.size);
	if (info.priority)
		printf("PRIORITY: %i\n", info.priority);
	else
		printf("PRIORITY: [disabled]\n");

	return CMD_RET_SUCCESS;
}

static int slot_size(int argc, char * const argv[])
{
	int slot;
	int size;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	size = rsu_slot_size(slot);
	rsu_exit();

	if (size < 0)
		return CMD_RET_FAILURE;

	printf("Slot %d size = %d.\n", slot, size);
	return CMD_RET_SUCCESS;
}

static int slot_priority(int argc, char * const argv[])
{
	int slot;
	int priority;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	priority = rsu_slot_priority(slot);
	rsu_exit();

	if (priority < 0)
		return CMD_RET_FAILURE;

	printf("Slot %d priority = %d.\n", slot, priority);
	return CMD_RET_SUCCESS;
}

static int slot_erase(int argc, char * const argv[])
{
	int slot;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_erase(slot);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	printf("Slot %d erased.\n", slot);
	return CMD_RET_SUCCESS;
}

static int slot_program_buf(int argc, char * const argv[])
{
	int slot;
	u64 address;
	int size;
	int ret;
	u32 addr_lo;
	u32 addr_hi;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot) ||
	    rsu_parse_num(argv[2], 16, &address) ||
	    rsu_parse_hex_int(argv[3], &size))
		return CMD_RET_USAGE;

	/*
	 * Reject u64 addresses that don't round-trip through ulong so the
	 * cast to (void *) below cannot silently truncate.
	 */
	if (address > (u64)ULONG_MAX)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_program_buf(slot, (void *)(ulong)address, size);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	addr_hi = upper_32_bits(address);
	addr_lo = lower_32_bits(address);
	printf("Slot %d was programmed with buffer=0x%08x%08x size=%d.\n",
	       slot, addr_hi, addr_lo, size);

	return CMD_RET_SUCCESS;
}

static int slot_program_factory_update_buf(int argc, char * const argv[])
{
	int slot;
	u64 address;
	int size;
	int ret;
	u32 addr_lo;
	u32 addr_hi;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot) ||
	    rsu_parse_num(argv[2], 16, &address) ||
	    rsu_parse_hex_int(argv[3], &size))
		return CMD_RET_USAGE;

	/*
	 * Reject u64 addresses that don't round-trip through ulong so the
	 * cast to (void *) below cannot silently truncate.
	 */
	if (address > (u64)ULONG_MAX)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_program_factory_update_buf(slot, (void *)(ulong)address,
						  size);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	addr_hi = upper_32_bits(address);
	addr_lo = lower_32_bits(address);
	printf("Slot %d was programmed with buffer=0x%08x%08x size=%d.\n",
	       slot, addr_hi, addr_lo, size);

	return CMD_RET_SUCCESS;
}

static int slot_program_buf_raw(int argc, char * const argv[])
{
	int slot;
	u64 address;
	int size;
	int ret;
	u32 addr_lo;
	u32 addr_hi;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot) ||
	    rsu_parse_num(argv[2], 16, &address) ||
	    rsu_parse_hex_int(argv[3], &size))
		return CMD_RET_USAGE;

	/*
	 * Reject u64 addresses that don't round-trip through ulong so the
	 * cast to (void *) below cannot silently truncate.
	 */
	if (address > (u64)ULONG_MAX)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_program_buf_raw(slot, (void *)(ulong)address, size);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	addr_hi = upper_32_bits(address);
	addr_lo = lower_32_bits(address);
	printf("Slot %d was programmed with raw buffer=0x%08x%08x size=%d.\n",
	       slot, addr_hi, addr_lo, size);

	return CMD_RET_SUCCESS;
}

static int slot_verify_buf(int argc, char * const argv[])
{
	int slot;
	u64 address;
	int size;
	int ret;
	u32 addr_lo;
	u32 addr_hi;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot) ||
	    rsu_parse_num(argv[2], 16, &address) ||
	    rsu_parse_hex_int(argv[3], &size))
		return CMD_RET_USAGE;

	/*
	 * Reject u64 addresses that don't round-trip through ulong so the
	 * cast to (void *) below cannot silently truncate.
	 */
	if (address > (u64)ULONG_MAX)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_verify_buf(slot, (void *)(ulong)address, size);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	addr_hi = upper_32_bits(address);
	addr_lo = lower_32_bits(address);
	printf("Slot %d was verified with buffer=0x%08x%08x size=%d.\n",
	       slot, addr_hi, addr_lo, size);

	return CMD_RET_SUCCESS;
}

static int slot_verify_buf_raw(int argc, char * const argv[])
{
	int slot;
	u64 address;
	int size;
	int ret;
	u32 addr_lo;
	u32 addr_hi;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot) ||
	    rsu_parse_num(argv[2], 16, &address) ||
	    rsu_parse_hex_int(argv[3], &size))
		return CMD_RET_USAGE;

	/*
	 * Reject u64 addresses that don't round-trip through ulong so the
	 * cast to (void *) below cannot silently truncate.
	 */
	if (address > (u64)ULONG_MAX)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_verify_buf_raw(slot, (void *)(ulong)address, size);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	addr_hi = upper_32_bits(address);
	addr_lo = lower_32_bits(address);
	printf("Slot %d was verified with raw buffer=0x%08x%08x size=%d.\n",
	       slot, addr_hi, addr_lo, size);

	return CMD_RET_SUCCESS;
}

static int slot_enable(int argc, char * const argv[])
{
	int slot;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_enable(slot);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	printf("Slot %d enabled.\n", slot);
	return CMD_RET_SUCCESS;
}

static int slot_disable(int argc, char * const argv[])
{
	int slot;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_disable(slot);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	printf("Slot %d disabled.\n", slot);
	return CMD_RET_SUCCESS;
}

static int slot_load(int argc, char * const argv[])
{
	int slot;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_load(slot);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	printf("Slot %d loading.\n", slot);
	return CMD_RET_SUCCESS;
}

static int slot_load_factory(int argc, char * const argv[])
{
	int ret;

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_load_factory();
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	printf("Factory loading.\n");
	return CMD_RET_SUCCESS;
}

static int slot_rename(int argc, char * const argv[])
{
	int slot;
	char *name;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;
	name = argv[2];

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_rename(slot, name);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	printf("Slot %d renamed to %s.\n", slot, name);
	return CMD_RET_SUCCESS;
}

static int slot_delete(int argc, char * const argv[])
{
	int slot;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_slot(argv[1], &slot))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_delete(slot);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	printf("Slot %d deleted.\n", slot);
	return CMD_RET_SUCCESS;
}

static int slot_create(int argc, char * const argv[])
{
	char *name;
	u64 address;
	unsigned int size;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	name = argv[1];
	if (rsu_parse_num(argv[2], 16, &address) ||
	    rsu_parse_hex_uint(argv[3], &size))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_slot_create(name, address, size);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	printf("Slot %s created at 0x%016llx with size = 0x%08x bytes.\n", name,
	       address, size);
	return CMD_RET_SUCCESS;
}

static int status_log(int argc, char * const argv[])
{
	struct rsu_status_info info;
	int ret;

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_status_log(&info);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	printf("Current Image\t: 0x%016llx\n", info.current_image);
	printf("Last Fail Image\t: 0x%016llx\n", info.fail_image);
	printf("State\t\t: 0x%08x\n", info.state);
	printf("Version\t\t: 0x%08x\n", info.version);
	printf("Error location\t: 0x%08x\n", info.error_location);
	printf("Error details\t: 0x%08x\n", info.error_details);
	if (info.version)
		printf("Retry counter\t: 0x%08x\n", info.retry_counter);

	return CMD_RET_SUCCESS;
}

static int notify(int argc, char * const argv[])
{
	u32 stage;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_hex_u32(argv[1], &stage))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_notify(stage);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int clear_error_status(int argc, char * const argv[])
{
	int ret;

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_clear_error_status();
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int reset_retry_counter(int argc, char * const argv[])
{
	int ret;

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_reset_retry_counter();
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int display_dcmf_version(int argc, char * const argv[])
{
	int i, ret;
	u32 versions[4];

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_dcmf_version(versions);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	for (i = 0; i < 4; i++)
		printf("DCMF%d version = %d.%d.%d\n", i,
		       (int)DCMF_VERSION_MAJOR(versions[i]),
		       (int)DCMF_VERSION_MINOR(versions[i]),
		       (int)DCMF_VERSION_UPDATE(versions[i]));

	return CMD_RET_SUCCESS;
}

static int display_dcmf_status(int argc, char * const argv[])
{
	int i, ret;
	u16 status[4];

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_dcmf_status(status);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	for (i = 0; i < 4; i++)
		printf("DCMF%d: %s\n", i, status[i] ? "Corrupted" : "OK");

	return CMD_RET_SUCCESS;
}

static int display_max_retry(int argc, char * const argv[])
{
	int ret;
	u8 value;

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_max_retry(&value);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	printf("max_retry = %d\n", (int)value);

	return CMD_RET_SUCCESS;
}

static int create_empty_cpb(int argc, char * const argv[])
{
	int ret;

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_create_empty_cpb();
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int restore_cpb(int argc, char * const argv[])
{
	int ret;
	u64 addr;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_num(argv[1], 16, &addr))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_restore_cpb(addr);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int save_cpb(int argc, char * const argv[])
{
	int ret;
	u64 addr;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_num(argv[1], 16, &addr))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_save_cpb(addr);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int restore_spt(int argc, char * const argv[])
{
	int ret;
	u64 addr;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_num(argv[1], 16, &addr))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_restore_spt(addr);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int save_spt(int argc, char * const argv[])
{
	int ret;
	u64 addr;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (rsu_parse_num(argv[1], 16, &addr))
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_save_spt(addr);
	rsu_exit();

	if (ret < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int check_running_factory(int argc, char * const argv[])
{
	int ret;
	int factory;

	if (argc != 1)
		return CMD_RET_USAGE;

	if (rsu_init(NULL))
		return CMD_RET_FAILURE;

	ret = rsu_running_factory(&factory);
	rsu_exit();

	if (ret)
		return CMD_RET_FAILURE;

	printf("Running factory image: %s\n", factory ? "yes" : "no");
	return CMD_RET_SUCCESS;
}

struct func_t {
	const char *cmd_string;
	int (*func_ptr)(int cmd_argc, char * const cmd_argv[]);
};

static const struct func_t rsu_func_t[] = {
	{"dtb", rsu_dtb},
	{"list", rsu_spt_cpb_list},
	{"slot_by_name", slot_by_name},
	{"slot_count", slot_count},
	{"slot_disable", slot_disable},
	{"slot_enable", slot_enable},
	{"slot_erase", slot_erase},
	{"slot_get_info", slot_get_info},
	{"slot_load", slot_load},
	{"slot_load_factory", slot_load_factory},
	{"slot_priority", slot_priority},
	{"slot_program_buf", slot_program_buf},
	{"slot_program_buf_raw", slot_program_buf_raw},
	{"slot_program_factory_update_buf", slot_program_factory_update_buf},
	{"slot_rename", slot_rename},
	{"slot_delete", slot_delete},
	{"slot_create", slot_create},
	{"slot_size", slot_size},
	{"slot_verify_buf", slot_verify_buf},
	{"slot_verify_buf_raw", slot_verify_buf_raw},
	{"status_log", status_log},
	{"update", rsu_update},
	{"notify", notify},
	{"clear_error_status", clear_error_status},
	{"reset_retry_counter", reset_retry_counter},
	{"display_dcmf_version", display_dcmf_version},
	{"display_dcmf_status", display_dcmf_status},
	{"display_max_retry", display_max_retry},
	{"save_spt", save_spt},
	{"restore_spt", restore_spt},
	{"create_empty_cpb", create_empty_cpb},
	{"restore_cpb", restore_cpb},
	{"save_cpb", save_cpb},
	{"check_running_factory", check_running_factory}
};

int do_rsu(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;
	int i;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	cmd = argv[1];
	--argc;
	++argv;

	for (i = 0; i < ARRAY_SIZE(rsu_func_t); i++) {
		if (!strcmp(cmd, rsu_func_t[i].cmd_string)) {
			ret = rsu_func_t[i].func_ptr(argc, argv);
			/*
			 * Some helpers return negative errno rather than
			 * CMD_RET_*; normalise so a failure cannot kill an
			 * interactive shell session.
			 */
			if (ret == CMD_RET_SUCCESS ||
			    ret == CMD_RET_FAILURE ||
			    ret == CMD_RET_USAGE)
				return ret;
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(rsu, 5, 1, do_rsu,
	   "Altera SoC FPGA Remote System Update",
	   "dtb   - Update Linux DTB qspi-boot partition offset with spt0 value\n"
	   "list  - List down the available bitstreams in flash\n"
	   "slot_by_name <name> - find slot by name and display the slot number\n"
	   "slot_count - display the slot count\n"
	   "slot_disable <slot> - remove slot from CPB\n"
	   "slot_enable <slot> - make slot the highest priority\n"
	   "slot_erase <slot> - erase slot\n"
	   "slot_get_info <slot> - display slot information\n"
	   "slot_load <slot> - load slot immediately\n"
	   "slot_load_factory - load factory immediately\n"
	   "slot_priority <slot> - display slot priority\n"
	   "slot_program_buf <slot> <buffer> <size> - program buffer into slot, and make it highest priority\n"
	   "slot_program_buf_raw <slot> <buffer> <size> - program raw buffer into slot\n"
	   "slot_program_factory_update_buf <slot> <buffer> <size> - program factory update buffer into slot, and make it highest priority\n"
	   "slot_rename <slot> <name> - rename slot\n"
	   "slot_delete <slot> - delete slot\n"
	   "slot_create <name> <address> <size> - create slot\n"
	   "slot_size <slot> - display slot size\n"
	   "slot_verify_buf <slot> <buffer> <size> - verify slot contents against buffer\n"
	   "slot_verify_buf_raw <slot> <buffer> <size> - verify slot contents against raw buffer\n"
	   "status_log - display RSU status\n"
	   "update <flash_offset> - Initiate firmware to load bitstream as specified by flash_offset\n"
	   "notify <value> - Let SDM know the current state of HPS software\n"
	   "clear_error_status - clear the RSU error status\n"
	   "reset_retry_counter - reset the RSU retry counter\n"
	   "display_dcmf_version - display DCMF versions and store them for SMC handler usage\n"
	   "display_dcmf_status - display DCMF status and store it for SMC handler usage\n"
	   "display_max_retry - display max_retry parameter, and store it for SMC handler usage\n"
	   "restore_spt <address> - restore SPT from an address\n"
	   "save_spt <address> - save SPT to an address\n"
	   "create_empty_cpb - create an empty CPB\n"
	   "restore_cpb <address> - restore CPB from an address\n"
	   "save_cpb <address> - save CPB to an address\n"
	   "check_running_factory - check if currently running the factory image"
);

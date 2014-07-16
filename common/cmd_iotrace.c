/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <iotrace.h>

static void do_print_stats(void)
{
	ulong start, size, offset, count;

	printf("iotrace is %sabled\n", iotrace_get_enabled() ? "en" : "dis");
	iotrace_get_buffer(&start, &size, &offset, &count);
	printf("Start:  %08lx\n", start);
	printf("Size:   %08lx\n", size);
	printf("Offset: %08lx\n", offset);
	printf("Output: %08lx\n", start + offset);
	printf("Count:  %08lx\n", count);
	printf("CRC32:  %08lx\n", (ulong)iotrace_get_checksum());
}

static int do_set_buffer(int argc, char * const argv[])
{
	ulong addr = 0, size = 0;

	if (argc == 2) {
		addr = simple_strtoul(*argv++, NULL, 16);
		size = simple_strtoul(*argv++, NULL, 16);
	} else if (argc != 0) {
		return CMD_RET_USAGE;
	}

	iotrace_set_buffer(addr, size);

	return 0;
}

int do_iotrace(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd = argc < 2 ? NULL : argv[1];

	if (!cmd)
		return cmd_usage(cmdtp);
	switch (*cmd) {
	case 'b':
		return do_set_buffer(argc - 2, argv + 2);
	case 'p':
		iotrace_set_enabled(0);
		break;
	case 'r':
		iotrace_set_enabled(1);
		break;
	case 's':
		do_print_stats();
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	iotrace,	4,	1,	do_iotrace,
	"iotrace utility commands",
	"stats                        - display iotrace stats\n"
	"iotrace buffer <address> <size>      - set iotrace buffer\n"
	"iotrace pause                        - pause tracing\n"
	"iotrace resume                       - resume tracing"
);

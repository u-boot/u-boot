// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2013 ADVANSEE
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
 *
 * Based on the mpc512x iim code:
 * Copyright 2008 Silicon Turnkey Express, Inc.
 * Martha Marx <mmarx@silicontkx.com>
 */

#include <command.h>
#include <console.h>
#include <fuse.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <linux/errno.h>
#include <linux/string.h>

static int confirm_prog(void)
{
	puts("Warning: Programming fuses is an irreversible operation!\n"
			"         This may brick your system.\n"
			"         Use this command only if you are sure of "
					"what you are doing!\n"
			"\nReally perform this fuse programming? <y/N>\n");

	if (confirm_yesno())
		return 1;

	puts("Fuse programming aborted\n");
	return 0;
}

static int do_fuse(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	const char *op = cmd_arg1(argc, argv);
	int confirmed = argc >= 3 && !strcmp(argv[2], "-y");
	u32 bank, word, cnt, val, cmp;
	ulong addr;
	void *buf, *start;
	int ret, i;

	argc -= 2 + confirmed;
	argv += 2 + confirmed;

	if (IS_ENABLED(CONFIG_CMD_FUSE_WRITEBUFF) && !strcmp(op, "writebuff")) {
		if (argc == 1)
			addr = simple_strtoul(argv[0], NULL, 16);
		else
			return CMD_RET_USAGE;
	} else {
		if (argc < 2)
			return CMD_RET_USAGE;

		bank = simple_strtoul(argv[0], NULL, 0);
		word = simple_strtoul(argv[1], NULL, 0);
	}

	if (!strcmp(op, "read")) {
		if (argc == 2)
			cnt = 1;
		else if (argc == 3)
			cnt = simple_strtoul(argv[2], NULL, 0);
		else
			return CMD_RET_USAGE;

		printf("Reading bank %u:\n", bank);
		for (i = 0; i < cnt; i++, word++) {
			if (!(i % 4))
				printf("\nWord 0x%.8x:", word);

			ret = fuse_read(bank, word, &val);
			if (ret)
				goto err;

			printf(" %.8x", val);
		}
		putc('\n');
	} else if (!strcmp(op, "readm")) {
		if (argc == 3)
			cnt = 1;
		else if (argc == 4)
			cnt = simple_strtoul(argv[3], NULL, 0);
		else
			return CMD_RET_USAGE;

		addr = simple_strtoul(argv[2], NULL, 16);

		start = map_sysmem(addr, 4);
		buf = start;

		printf("Reading bank %u len %u to 0x%lx\n", bank, cnt, addr);
		for (i = 0; i < cnt; i++, word++) {
			ret = fuse_read(bank, word, &val);
			if (ret)
				goto err;

			*((u32 *)buf) = val;
			buf += 4;
		}

		unmap_sysmem(start);
	} else if (!strcmp(op, "cmp")) {
		if (argc == 3)
			cmp = simple_strtoul(argv[2], NULL, 0);
		else
			return CMD_RET_USAGE;

		printf("Comparing bank %u:\n", bank);
		printf("\nWord 0x%.8x:", word);
		printf("\nValue 0x%.8x:", cmp);

		ret = fuse_read(bank, word, &val);
		if (ret)
			goto err;

		printf("0x%.8x\n", val);
		if (val != cmp) {
			printf("failed\n");
			return CMD_RET_FAILURE;
		}
		printf("passed\n");
	} else if (!strcmp(op, "sense")) {
		if (argc == 2)
			cnt = 1;
		else if (argc == 3)
			cnt = simple_strtoul(argv[2], NULL, 0);
		else
			return CMD_RET_USAGE;

		printf("Sensing bank %u:\n", bank);
		for (i = 0; i < cnt; i++, word++) {
			if (!(i % 4))
				printf("\nWord 0x%.8x:", word);

			ret = fuse_sense(bank, word, &val);
			if (ret)
				goto err;

			printf(" %.8x", val);
		}
		putc('\n');
	} else if (!strcmp(op, "prog")) {
		if (argc < 3)
			return CMD_RET_USAGE;

		for (i = 2; i < argc; i++, word++) {
			val = simple_strtoul(argv[i], NULL, 16);

			printf("Programming bank %u word 0x%.8x to 0x%.8x...\n",
					bank, word, val);
			if (!confirmed && !confirm_prog())
				return CMD_RET_FAILURE;
			ret = fuse_prog(bank, word, val);
			if (ret)
				goto err;
		}
	} else if (!strcmp(op, "override")) {
		if (argc < 3)
			return CMD_RET_USAGE;

		for (i = 2; i < argc; i++, word++) {
			val = simple_strtoul(argv[i], NULL, 16);

			printf("Overriding bank %u word 0x%.8x with "
					"0x%.8x...\n", bank, word, val);
			ret = fuse_override(bank, word, val);
			if (ret)
				goto err;
		}
	} else if (IS_ENABLED(CONFIG_CMD_FUSE_WRITEBUFF) && !strcmp(op, "writebuff")) {
		printf("Programming fuses using a structured buffer in memory "
				"starting at addr 0x%lx\n", addr);
		if (!confirmed && !confirm_prog())
			return CMD_RET_FAILURE;

		ret = fuse_writebuff(addr);
		if (ret)
			goto err;
	} else {
		return CMD_RET_USAGE;
	}

	return 0;

err:
	puts("ERROR\n");
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	fuse, CONFIG_SYS_MAXARGS, 0, do_fuse,
	"Fuse sub-system",
	     "read <bank> <word> [<cnt>] - read 1 or 'cnt' fuse words,\n"
	"    starting at 'word'\n"
	"fuse cmp <bank> <word> <hexval> - compare 'hexval' to fuse\n"
	"    at 'word'\n"
	"fuse readm <bank> <word> <addr> [<cnt>] - read 1 or 'cnt' fuse words,\n"
	"    starting at 'word' into memory at 'addr'\n"
	"fuse sense <bank> <word> [<cnt>] - sense 1 or 'cnt' fuse words,\n"
	"    starting at 'word'\n"
	"fuse prog [-y] <bank> <word> <hexval> [<hexval>...] - program 1 or\n"
	"    several fuse words, starting at 'word' (PERMANENT)\n"
	"fuse override <bank> <word> <hexval> [<hexval>...] - override 1 or\n"
	"    several fuse words, starting at 'word'\n"
#ifdef CONFIG_CMD_FUSE_WRITEBUFF
	"fuse writebuff [-y] <addr> - program fuse data\n"
	"    using a structured buffer in memory starting at 'addr'\n"
#endif /* CONFIG_CMD_FUSE_WRITEBUFF */
);

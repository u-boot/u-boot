/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Ported 'dynenv' to 'nand env.oob' command
 * (C) 2010 Nanometrics, Inc.
 * 'dynenv' -- Dynamic environment offset in NAND OOB
 * (C) Copyright 2006-2007 OpenMoko, Inc.
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 */

#include <common.h>
#include <linux/mtd/mtd.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>

#if defined(CONFIG_CMD_MTDPARTS)

/* partition handling routines */
int mtdparts_init(void);
int id_parse(const char *id, const char **ret_id, u8 *dev_type, u8 *dev_num);
int find_dev_and_part(const char *id, struct mtd_device **dev,
		      u8 *part_num, struct part_info **part);
#endif

static int nand_dump(nand_info_t *nand, ulong off, int only_oob)
{
	int i;
	u_char *datbuf, *oobbuf, *p;

	datbuf = malloc(nand->writesize + nand->oobsize);
	oobbuf = malloc(nand->oobsize);
	if (!datbuf || !oobbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(nand->writesize - 1);
	loff_t addr = (loff_t) off;
	struct mtd_oob_ops ops;
	memset(&ops, 0, sizeof(ops));
	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf; /* must exist, but oob data will be appended to ops.datbuf */
	ops.len = nand->writesize;
	ops.ooblen = nand->oobsize;
	ops.mode = MTD_OOB_RAW;
	i = nand->read_oob(nand, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		free(datbuf);
		free(oobbuf);
		return 1;
	}
	printf("Page %08lx dump:\n", off);
	i = nand->writesize >> 4;
	p = datbuf;

	while (i--) {
		if (!only_oob)
			printf("\t%02x %02x %02x %02x %02x %02x %02x %02x"
			       "  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			       p[8], p[9], p[10], p[11], p[12], p[13], p[14],
			       p[15]);
		p += 16;
	}
	puts("OOB:\n");
	i = nand->oobsize >> 3;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
	free(datbuf);
	free(oobbuf);

	return 0;
}

/* ------------------------------------------------------------------------- */

static inline int str2long(char *p, ulong *num)
{
	char *endptr;

	*num = simple_strtoul(p, &endptr, 16);
	return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}

static int
arg_off_size(int argc, char * const argv[], nand_info_t *nand, ulong *off, size_t *size)
{
	int idx = nand_curr_device;
#if defined(CONFIG_CMD_MTDPARTS)
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;

	if (argc >= 1 && !(str2long(argv[0], off))) {
		if ((mtdparts_init() == 0) &&
		    (find_dev_and_part(argv[0], &dev, &pnum, &part) == 0)) {
			if (dev->id->type != MTD_DEV_TYPE_NAND) {
				puts("not a NAND device\n");
				return -1;
			}
			*off = part->offset;
			if (argc >= 2) {
				if (!(str2long(argv[1], (ulong *)size))) {
					printf("'%s' is not a number\n", argv[1]);
					return -1;
				}
				if (*size > part->size)
					*size = part->size;
			} else {
				*size = part->size;
			}
			idx = dev->id->num;
			*nand = nand_info[idx];
			goto out;
		}
	}
#endif

	if (argc >= 1) {
		if (!(str2long(argv[0], off))) {
			printf("'%s' is not a number\n", argv[0]);
			return -1;
		}
	} else {
		*off = 0;
	}

	if (argc >= 2) {
		if (!(str2long(argv[1], (ulong *)size))) {
			printf("'%s' is not a number\n", argv[1]);
			return -1;
		}
	} else {
		*size = nand->size - *off;
	}

#if defined(CONFIG_CMD_MTDPARTS)
out:
#endif
	printf("device %d ", idx);
	if (*size == nand->size)
		puts("whole chip\n");
	else
		printf("offset 0x%lx, size 0x%zx\n", *off, *size);
	return 0;
}

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
static void print_status(ulong start, ulong end, ulong erasesize, int status)
{
	printf("%08lx - %08lx: %08lx blocks %s%s%s\n",
		start,
		end - 1,
		(end - start) / erasesize,
		((status & NAND_LOCK_STATUS_TIGHT) ?  "TIGHT " : ""),
		((status & NAND_LOCK_STATUS_LOCK) ?  "LOCK " : ""),
		((status & NAND_LOCK_STATUS_UNLOCK) ?  "UNLOCK " : ""));
}

static void do_nand_status(nand_info_t *nand)
{
	ulong block_start = 0;
	ulong off;
	int last_status = -1;

	struct nand_chip *nand_chip = nand->priv;
	/* check the WP bit */
	nand_chip->cmdfunc(nand, NAND_CMD_STATUS, -1, -1);
	printf("device is %swrite protected\n",
		(nand_chip->read_byte(nand) & 0x80 ?
		"NOT " : ""));

	for (off = 0; off < nand->size; off += nand->erasesize) {
		int s = nand_get_lock_status(nand, off);

		/* print message only if status has changed */
		if (s != last_status && off != 0) {
			print_status(block_start, off, nand->erasesize,
					last_status);
			block_start = off;
		}
		last_status = s;
	}
	/* Print the last block info */
	print_status(block_start, off, nand->erasesize, last_status);
}
#endif

#ifdef CONFIG_ENV_OFFSET_OOB
unsigned long nand_env_oob_offset;

int do_nand_env_oob(cmd_tbl_t *cmdtp, nand_info_t *nand,
		    int argc, char * const argv[])
{
	int ret;
	uint32_t oob_buf[ENV_OFFSET_SIZE/sizeof(uint32_t)];

	char *cmd = argv[1];

	if (!strcmp(cmd, "get")) {
		ret = get_nand_env_oob(nand, &nand_env_oob_offset);
		if (ret)
			return 1;

		printf("0x%08lx\n", nand_env_oob_offset);
	} else if (!strcmp(cmd, "set")) {
		ulong addr;
		size_t dummy_size;
		struct mtd_oob_ops ops;

		if (argc < 3)
			goto usage;

		if (arg_off_size(argc - 2, argv + 2, nand, &addr,
				 &dummy_size) < 0) {
			printf("Offset or partition name expected\n");
			return 1;
		}

		if (nand->oobavail < ENV_OFFSET_SIZE) {
			printf("Insufficient available OOB bytes:\n"
			       "%d OOB bytes available but %d required for "
			       "env.oob support\n",
			       nand->oobavail, ENV_OFFSET_SIZE);
			return 1;
		}

		if ((addr & (nand->erasesize - 1)) != 0) {
			printf("Environment offset must be block-aligned\n");
			return 1;
		}

		ops.datbuf = NULL;
		ops.mode = MTD_OOB_AUTO;
		ops.ooboffs = 0;
		ops.ooblen = ENV_OFFSET_SIZE;
		ops.oobbuf = (void *) oob_buf;

		oob_buf[0] = ENV_OOB_MARKER;
		oob_buf[1] = addr / nand->erasesize;

		ret = nand->write_oob(nand, ENV_OFFSET_SIZE, &ops);
		if (ret) {
			printf("Error writing OOB block 0\n");
			return ret;
		}

		ret = get_nand_env_oob(nand, &nand_env_oob_offset);
		if (ret) {
			printf("Error reading env offset in OOB\n");
			return ret;
		}

		if (addr != nand_env_oob_offset) {
			printf("Verification of env offset in OOB failed: "
			       "0x%08lx expected but got 0x%08lx\n",
			       addr, nand_env_oob_offset);
			return 1;
		}
	} else {
		goto usage;
	}

	return ret;

usage:
	return cmd_usage(cmdtp);
}

#endif

static void nand_print_info(int idx)
{
	nand_info_t *nand = &nand_info[idx];
	struct nand_chip *chip = nand->priv;
	printf("Device %d: ", idx);
	if (chip->numchips > 1)
		printf("%dx ", chip->numchips);
	printf("%s, sector size %u KiB\n",
	       nand->name, nand->erasesize >> 10);
}

int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int i, dev, ret = 0;
	ulong addr, off;
	size_t size;
	char *cmd, *s;
	nand_info_t *nand;
#ifdef CONFIG_SYS_NAND_QUIET
	int quiet = CONFIG_SYS_NAND_QUIET;
#else
	int quiet = 0;
#endif
	const char *quiet_str = getenv("quiet");

	/* at least two arguments please */
	if (argc < 2)
		goto usage;

	if (quiet_str)
		quiet = simple_strtoul(quiet_str, NULL, 0) != 0;

	cmd = argv[1];

	if (strcmp(cmd, "info") == 0) {

		putc('\n');
		for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++) {
			if (nand_info[i].name)
				nand_print_info(i);
		}
		return 0;
	}

	if (strcmp(cmd, "device") == 0) {

		if (argc < 3) {
			putc('\n');
			if ((nand_curr_device < 0) ||
			    (nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE))
				puts("no devices available\n");
			else
				nand_print_info(nand_curr_device);
			return 0;
		}
		dev = (int)simple_strtoul(argv[2], NULL, 10);
		if (dev < 0 || dev >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[dev].name) {
			puts("No such device\n");
			return 1;
		}
		printf("Device %d: %s", dev, nand_info[dev].name);
		puts("... is now current device\n");
		nand_curr_device = dev;

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
		/*
		 * Select the chip in the board/cpu specific driver
		 */
		board_nand_select_device(nand_info[dev].priv, dev);
#endif

		return 0;
	}

	if (strcmp(cmd, "bad") != 0 && strcmp(cmd, "erase") != 0 &&
	    strncmp(cmd, "dump", 4) != 0 &&
	    strncmp(cmd, "read", 4) != 0 && strncmp(cmd, "write", 5) != 0 &&
	    strcmp(cmd, "scrub") != 0 && strcmp(cmd, "markbad") != 0 &&
	    strcmp(cmd, "biterr") != 0 &&
	    strcmp(cmd, "lock") != 0 && strcmp(cmd, "unlock") != 0
#ifdef CONFIG_ENV_OFFSET_OOB
	    && strcmp(cmd, "env.oob") != 0
#endif
	    )
		goto usage;

#ifdef CONFIG_ENV_OFFSET_OOB
	/* this command operates only on the first nand device */
	if (strcmp(cmd, "env.oob") == 0) {
		return do_nand_env_oob(cmdtp, &nand_info[0],
				       argc - 1, argv + 1);
	}
#endif

	/* the following commands operate on the current device */
	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE ||
	    !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];

	if (strcmp(cmd, "bad") == 0) {
		printf("\nDevice %d bad blocks:\n", nand_curr_device);
		for (off = 0; off < nand->size; off += nand->erasesize)
			if (nand_block_isbad(nand, off))
				printf("  %08lx\n", off);
		return 0;
	}

	/*
	 * Syntax is:
	 *   0    1     2       3    4
	 *   nand erase [clean] [off size]
	 */
	if (strcmp(cmd, "erase") == 0 || strcmp(cmd, "scrub") == 0) {
		nand_erase_options_t opts;
		/* "clean" at index 2 means request to write cleanmarker */
		int clean = argc > 2 && !strcmp("clean", argv[2]);
		int o = clean ? 3 : 2;
		int scrub = !strcmp(cmd, "scrub");

		printf("\nNAND %s: ", scrub ? "scrub" : "erase");
		/* skip first two or three arguments, look for offset and size */
		if (arg_off_size(argc - o, argv + o, nand, &off, &size) != 0)
			return 1;

		memset(&opts, 0, sizeof(opts));
		opts.offset = off;
		opts.length = size;
		opts.jffs2  = clean;
		opts.quiet  = quiet;

		if (scrub) {
			puts("Warning: "
			     "scrub option will erase all factory set "
			     "bad blocks!\n"
			     "         "
			     "There is no reliable way to recover them.\n"
			     "         "
			     "Use this command only for testing purposes "
			     "if you\n"
			     "         "
			     "are sure of what you are doing!\n"
			     "\nReally scrub this NAND flash? <y/N>\n");

			if (getc() == 'y') {
				puts("y");
				if (getc() == '\r')
					opts.scrub = 1;
				else {
					puts("scrub aborted\n");
					return -1;
				}
			} else {
				puts("scrub aborted\n");
				return -1;
			}
		}
		ret = nand_erase_opts(nand, &opts);
		printf("%s\n", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
	}

	if (strncmp(cmd, "dump", 4) == 0) {
		if (argc < 3)
			goto usage;

		s = strchr(cmd, '.');
		off = (int)simple_strtoul(argv[2], NULL, 16);

		if (s != NULL && strcmp(s, ".oob") == 0)
			ret = nand_dump(nand, off, 1);
		else
			ret = nand_dump(nand, off, 0);

		return ret == 0 ? 1 : 0;

	}

	if (strncmp(cmd, "read", 4) == 0 || strncmp(cmd, "write", 5) == 0) {
		int read;

		if (argc < 4)
			goto usage;

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);

		read = strncmp(cmd, "read", 4) == 0; /* 1 = read, 0 = write */
		printf("\nNAND %s: ", read ? "read" : "write");
		if (arg_off_size(argc - 3, argv + 3, nand, &off, &size) != 0)
			return 1;

		s = strchr(cmd, '.');
		if (!s || !strcmp(s, ".jffs2") ||
		    !strcmp(s, ".e") || !strcmp(s, ".i")) {
			if (read)
				ret = nand_read_skip_bad(nand, off, &size,
							 (u_char *)addr);
			else
				ret = nand_write_skip_bad(nand, off, &size,
							  (u_char *)addr);
		} else if (!strcmp(s, ".oob")) {
			/* out-of-band data */
			mtd_oob_ops_t ops = {
				.oobbuf = (u8 *)addr,
				.ooblen = size,
				.mode = MTD_OOB_RAW
			};

			if (read)
				ret = nand->read_oob(nand, off, &ops);
			else
				ret = nand->write_oob(nand, off, &ops);
		} else {
			printf("Unknown nand command suffix '%s'.\n", s);
			return 1;
		}

		printf(" %zu bytes %s: %s\n", size,
		       read ? "read" : "written", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
	}

	if (strcmp(cmd, "markbad") == 0) {
		argc -= 2;
		argv += 2;

		if (argc <= 0)
			goto usage;

		while (argc > 0) {
			addr = simple_strtoul(*argv, NULL, 16);

			if (nand->block_markbad(nand, addr)) {
				printf("block 0x%08lx NOT marked "
					"as bad! ERROR %d\n",
					addr, ret);
				ret = 1;
			} else {
				printf("block 0x%08lx successfully "
					"marked as bad\n",
					addr);
			}
			--argc;
			++argv;
		}
		return ret;
	}

	if (strcmp(cmd, "biterr") == 0) {
		/* todo */
		return 1;
	}

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
	if (strcmp(cmd, "lock") == 0) {
		int tight = 0;
		int status = 0;
		if (argc == 3) {
			if (!strcmp("tight", argv[2]))
				tight = 1;
			if (!strcmp("status", argv[2]))
				status = 1;
		}
		if (status) {
			do_nand_status(nand);
		} else {
			if (!nand_lock(nand, tight)) {
				puts("NAND flash successfully locked\n");
			} else {
				puts("Error locking NAND flash\n");
				return 1;
			}
		}
		return 0;
	}

	if (strcmp(cmd, "unlock") == 0) {
		if (arg_off_size(argc - 2, argv + 2, nand, &off, &size) < 0)
			return 1;

		if (!nand_unlock(nand, off, size)) {
			puts("NAND flash successfully unlocked\n");
		} else {
			puts("Error unlocking NAND flash, "
			     "write and erase will probably fail\n");
			return 1;
		}
		return 0;
	}
#endif

usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	nand, CONFIG_SYS_MAXARGS, 1, do_nand,
	"NAND sub-system",
	"info - show available NAND devices\n"
	"nand device [dev] - show or set current device\n"
	"nand read - addr off|partition size\n"
	"nand write - addr off|partition size\n"
	"    read/write 'size' bytes starting at offset 'off'\n"
	"    to/from memory address 'addr', skipping bad blocks.\n"
	"nand erase [clean] [off size] - erase 'size' bytes from\n"
	"    offset 'off' (entire device if not specified)\n"
	"nand bad - show bad blocks\n"
	"nand dump[.oob] off - dump page\n"
	"nand scrub - really clean NAND erasing bad blocks (UNSAFE)\n"
	"nand markbad off [...] - mark bad block(s) at offset (UNSAFE)\n"
	"nand biterr off - make a bit error at offset (UNSAFE)"
#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
	"\n"
	"nand lock [tight] [status]\n"
	"    bring nand to lock state or display locked pages\n"
	"nand unlock [offset] [size] - unlock section"
#endif
#ifdef CONFIG_ENV_OFFSET_OOB
	"\n"
	"nand env.oob - environment offset in OOB of block 0 of"
	"    first device.\n"
	"nand env.oob set off|partition - set enviromnent offset\n"
	"nand env.oob get - get environment offset"
#endif
);

static int nand_load_image(cmd_tbl_t *cmdtp, nand_info_t *nand,
			   ulong offset, ulong addr, char *cmd)
{
	int r;
	char *ep, *s;
	size_t cnt;
	image_header_t *hdr;
#if defined(CONFIG_FIT)
	const void *fit_hdr = NULL;
#endif

	s = strchr(cmd, '.');
	if (s != NULL &&
	    (strcmp(s, ".jffs2") && strcmp(s, ".e") && strcmp(s, ".i"))) {
		printf("Unknown nand load suffix '%s'\n", s);
		show_boot_progress(-53);
		return 1;
	}

	printf("\nLoading from %s, offset 0x%lx\n", nand->name, offset);

	cnt = nand->writesize;
	r = nand_read_skip_bad(nand, offset, &cnt, (u_char *) addr);
	if (r) {
		puts("** Read error\n");
		show_boot_progress (-56);
		return 1;
	}
	show_boot_progress (56);

	switch (genimg_get_format ((void *)addr)) {
	case IMAGE_FORMAT_LEGACY:
		hdr = (image_header_t *)addr;

		show_boot_progress (57);
		image_print_contents (hdr);

		cnt = image_get_image_size (hdr);
		break;
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = (const void *)addr;
		puts ("Fit image detected...\n");

		cnt = fit_get_size (fit_hdr);
		break;
#endif
	default:
		show_boot_progress (-57);
		puts ("** Unknown image type\n");
		return 1;
	}
	show_boot_progress (57);

	r = nand_read_skip_bad(nand, offset, &cnt, (u_char *) addr);
	if (r) {
		puts("** Read error\n");
		show_boot_progress (-58);
		return 1;
	}
	show_boot_progress (58);

#if defined(CONFIG_FIT)
	/* This cannot be done earlier, we need complete FIT image in RAM first */
	if (genimg_get_format ((void *)addr) == IMAGE_FORMAT_FIT) {
		if (!fit_check_format (fit_hdr)) {
			show_boot_progress (-150);
			puts ("** Bad FIT image format\n");
			return 1;
		}
		show_boot_progress (151);
		fit_print_contents (fit_hdr);
	}
#endif

	/* Loading ok, update default load address */

	load_addr = addr;

	/* Check if we should attempt an auto-start */
	if (((ep = getenv("autostart")) != NULL) && (strcmp(ep, "yes") == 0)) {
		char *local_args[2];
		extern int do_bootm(cmd_tbl_t *, int, int, char *[]);

		local_args[0] = cmd;
		local_args[1] = NULL;

		printf("Automatic boot of image at addr 0x%08lx ...\n", addr);

		do_bootm(cmdtp, 0, 1, local_args);
		return 1;
	}
	return 0;
}

int do_nandboot(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	char *boot_device = NULL;
	int idx;
	ulong addr, offset = 0;
#if defined(CONFIG_CMD_MTDPARTS)
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;

	if (argc >= 2) {
		char *p = (argc == 2) ? argv[1] : argv[2];
		if (!(str2long(p, &addr)) && (mtdparts_init() == 0) &&
		    (find_dev_and_part(p, &dev, &pnum, &part) == 0)) {
			if (dev->id->type != MTD_DEV_TYPE_NAND) {
				puts("Not a NAND device\n");
				return 1;
			}
			if (argc > 3)
				goto usage;
			if (argc == 3)
				addr = simple_strtoul(argv[1], NULL, 16);
			else
				addr = CONFIG_SYS_LOAD_ADDR;
			return nand_load_image(cmdtp, &nand_info[dev->id->num],
					       part->offset, addr, argv[0]);
		}
	}
#endif

	show_boot_progress(52);
	switch (argc) {
	case 1:
		addr = CONFIG_SYS_LOAD_ADDR;
		boot_device = getenv("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	case 4:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		offset = simple_strtoul(argv[3], NULL, 16);
		break;
	default:
#if defined(CONFIG_CMD_MTDPARTS)
usage:
#endif
		show_boot_progress(-53);
		return cmd_usage(cmdtp);
	}

	show_boot_progress(53);
	if (!boot_device) {
		puts("\n** No boot device **\n");
		show_boot_progress(-54);
		return 1;
	}
	show_boot_progress(54);

	idx = simple_strtoul(boot_device, NULL, 16);

	if (idx < 0 || idx >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[idx].name) {
		printf("\n** Device %d not available\n", idx);
		show_boot_progress(-55);
		return 1;
	}
	show_boot_progress(55);

	return nand_load_image(cmdtp, &nand_info[idx], offset, addr, argv[0]);
}

U_BOOT_CMD(nboot, 4, 1, do_nandboot,
	"boot from NAND device",
	"[partition] | [[[loadAddr] dev] offset]"
);

// SPDX-License-Identifier:  GPL-2.0+
/*
 * mtd.c
 *
 * Generic command to handle basic operations on any memory device.
 *
 * Copyright: Bootlin, 2018
 * Author: Miquèl Raynal <miquel.raynal@bootlin.com>
 */

#include <command.h>
#include <console.h>
#include <led.h>
#if CONFIG_IS_ENABLED(CMD_MTD_OTP)
#include <hexdump.h>
#endif
#include <malloc.h>
#include <mapmem.h>
#include <mtd.h>
#include <dm/devres.h>
#include <linux/err.h>

#include <linux/ctype.h>

static struct mtd_info *get_mtd_by_name(const char *name)
{
	struct mtd_info *mtd;

	mtd_probe_devices();

	mtd = get_mtd_device_nm(name);
	if (IS_ERR_OR_NULL(mtd))
		printf("MTD device %s not found, ret %ld\n", name,
		       PTR_ERR(mtd));

	return mtd;
}

static uint mtd_len_to_pages(struct mtd_info *mtd, u64 len)
{
	do_div(len, mtd->writesize);

	return len;
}

static bool mtd_is_aligned_with_min_io_size(struct mtd_info *mtd, u64 size)
{
	return !do_div(size, mtd->writesize);
}

static bool mtd_is_aligned_with_block_size(struct mtd_info *mtd, u64 size)
{
	return !do_div(size, mtd->erasesize);
}

static void mtd_dump_buf(const u8 *buf, uint len, uint offset)
{
	int i, j;

	for (i = 0; i < len; ) {
		printf("0x%08x:\t", offset + i);
		for (j = 0; j < 8; j++)
			printf("%02x ", buf[i + j]);
		printf(" ");
		i += 8;
		for (j = 0; j < 8; j++)
			printf("%02x ", buf[i + j]);
		printf("\n");
		i += 8;
	}
}

static void mtd_dump_device_buf(struct mtd_info *mtd, u64 start_off,
				const u8 *buf, u64 len, bool woob)
{
	bool has_pages = mtd->type == MTD_NANDFLASH ||
		mtd->type == MTD_MLCNANDFLASH;
	int npages = mtd_len_to_pages(mtd, len);
	uint page;

	if (has_pages) {
		for (page = 0; page < npages; page++) {
			u64 data_off = (u64)page * mtd->writesize;

			printf("\nDump %d data bytes from 0x%08llx:\n",
			       mtd->writesize, start_off + data_off);
			mtd_dump_buf(&buf[data_off],
				     mtd->writesize, start_off + data_off);

			if (woob) {
				u64 oob_off = (u64)page * mtd->oobsize;

				printf("Dump %d OOB bytes from page at 0x%08llx:\n",
				       mtd->oobsize, start_off + data_off);
				mtd_dump_buf(&buf[len + oob_off],
					     mtd->oobsize, 0);
			}
		}
	} else {
		printf("\nDump %lld data bytes from 0x%llx:\n",
		       len, start_off);
		mtd_dump_buf(buf, len, start_off);
	}
}

static void mtd_show_parts(struct mtd_info *mtd, int level)
{
	struct mtd_info *part;
	int i;

	list_for_each_entry(part, &mtd->partitions, node) {
		for (i = 0; i < level; i++)
			printf("\t");
		printf("  - 0x%012llx-0x%012llx : \"%s\"\n",
		       part->offset, part->offset + part->size, part->name);

		mtd_show_parts(part, level + 1);
	}
}

static void mtd_show_device(struct mtd_info *mtd)
{
	/* Device */
	printf("* %s\n", mtd->name);
	if (mtd->dev) {
		printf("  - device: %s\n", mtd->dev->name);
		printf("  - parent: %s\n", mtd->dev->parent->name);
		printf("  - driver: %s\n", mtd->dev->driver->name);
	}
	if (IS_ENABLED(CONFIG_OF_CONTROL) && mtd->dev) {
		char buf[256];
		int res;

		res = ofnode_get_path(mtd_get_ofnode(mtd), buf, 256);
		printf("  - path: %s\n", res == 0 ? buf : "unavailable");
	}

	/* MTD device information */
	printf("  - type: ");
	switch (mtd->type) {
	case MTD_RAM:
		printf("RAM\n");
		break;
	case MTD_ROM:
		printf("ROM\n");
		break;
	case MTD_NORFLASH:
		printf("NOR flash\n");
		break;
	case MTD_NANDFLASH:
		printf("NAND flash\n");
		break;
	case MTD_DATAFLASH:
		printf("Data flash\n");
		break;
	case MTD_UBIVOLUME:
		printf("UBI volume\n");
		break;
	case MTD_MLCNANDFLASH:
		printf("MLC NAND flash\n");
		break;
	case MTD_ABSENT:
	default:
		printf("Unknown\n");
		break;
	}

	printf("  - block size: 0x%x bytes\n", mtd->erasesize);
	printf("  - min I/O: 0x%x bytes\n", mtd->writesize);

	if (mtd->oobsize) {
		printf("  - OOB size: %u bytes\n", mtd->oobsize);
		printf("  - OOB available: %u bytes\n", mtd->oobavail);
	}

	if (mtd->ecc_strength) {
		printf("  - ECC strength: %u bits\n", mtd->ecc_strength);
		printf("  - ECC step size: %u bytes\n", mtd->ecc_step_size);
		printf("  - bitflip threshold: %u bits\n",
		       mtd->bitflip_threshold);
	}

	printf("  - 0x%012llx-0x%012llx : \"%s\"\n",
	       mtd->offset, mtd->offset + mtd->size, mtd->name);

	/* MTD partitions, if any */
	mtd_show_parts(mtd, 1);
}

/* Logic taken from fs/ubifs/recovery.c:is_empty() */
static bool mtd_oob_write_is_empty(struct mtd_oob_ops *op)
{
	int i;

	for (i = 0; i < op->len; i++)
		if (op->datbuf[i] != 0xff)
			return false;

	for (i = 0; i < op->ooblen; i++)
		if (op->oobbuf[i] != 0xff)
			return false;

	return true;
}

#if CONFIG_IS_ENABLED(CMD_MTD_OTP)
static int do_mtd_otp_read(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct mtd_info *mtd;
	size_t retlen;
	off_t from;
	size_t len;
	bool user;
	int ret;
	u8 *buf;

	if (argc != 5)
		return CMD_RET_USAGE;

	if (!strcmp(argv[2], "u"))
		user = true;
	else if (!strcmp(argv[2], "f"))
		user = false;
	else
		return CMD_RET_USAGE;

	mtd = get_mtd_by_name(argv[1]);
	if (IS_ERR_OR_NULL(mtd))
		return CMD_RET_FAILURE;

	from = simple_strtoul(argv[3], NULL, 0);
	len = simple_strtoul(argv[4], NULL, 0);

	ret = CMD_RET_FAILURE;

	buf = malloc(len);
	if (!buf)
		goto put_mtd;

	printf("Reading %s OTP from 0x%lx, %zu bytes\n",
	       user ? "user" : "factory", from, len);

	if (user)
		ret = mtd_read_user_prot_reg(mtd, from, len, &retlen, buf);
	else
		ret = mtd_read_fact_prot_reg(mtd, from, len, &retlen, buf);
	if (ret) {
		free(buf);
		pr_err("OTP read failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto put_mtd;
	}

	if (retlen != len)
		pr_err("OTP read returns %zu, but %zu expected\n",
		       retlen, len);

	print_hex_dump("", 0, 16, 1, buf, retlen, true);

	free(buf);

	ret = CMD_RET_SUCCESS;

put_mtd:
	put_mtd_device(mtd);

	return ret;
}

static int do_mtd_otp_lock(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct mtd_info *mtd;
	off_t from;
	size_t len;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	mtd = get_mtd_by_name(argv[1]);
	if (IS_ERR_OR_NULL(mtd))
		return CMD_RET_FAILURE;

	from = simple_strtoul(argv[2], NULL, 0);
	len = simple_strtoul(argv[3], NULL, 0);

	ret = mtd_lock_user_prot_reg(mtd, from, len);
	if (ret) {
		pr_err("OTP lock failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto put_mtd;
	}

	ret = CMD_RET_SUCCESS;

put_mtd:
	put_mtd_device(mtd);

	return ret;
}

static int do_mtd_otp_write(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct mtd_info *mtd;
	size_t retlen;
	size_t binlen;
	u8 *binbuf;
	off_t from;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	mtd = get_mtd_by_name(argv[1]);
	if (IS_ERR_OR_NULL(mtd))
		return CMD_RET_FAILURE;

	from = simple_strtoul(argv[2], NULL, 0);
	binlen = strlen(argv[3]) / 2;

	ret = CMD_RET_FAILURE;
	binbuf = malloc(binlen);
	if (!binbuf)
		goto put_mtd;

	hex2bin(binbuf, argv[3], binlen);

	printf("Will write:\n");

	print_hex_dump("", 0, 16, 1, binbuf, binlen, true);

	printf("to 0x%lx\n", from);

	printf("Continue (y/n)?\n");

	if (confirm_yesno() != 1) {
		pr_err("OTP write canceled\n");
		ret = CMD_RET_SUCCESS;
		goto put_mtd;
	}

	ret = mtd_write_user_prot_reg(mtd, from, binlen, &retlen, binbuf);
	if (ret) {
		pr_err("OTP write failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto put_mtd;
	}

	if (retlen != binlen)
		pr_err("OTP write returns %zu, but %zu expected\n",
		       retlen, binlen);

	ret = CMD_RET_SUCCESS;

put_mtd:
	free(binbuf);
	put_mtd_device(mtd);

	return ret;
}

static int do_mtd_otp_info(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct otp_info otp_info;
	struct mtd_info *mtd;
	size_t retlen;
	bool user;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	if (!strcmp(argv[2], "u"))
		user = true;
	else if (!strcmp(argv[2], "f"))
		user = false;
	else
		return CMD_RET_USAGE;

	mtd = get_mtd_by_name(argv[1]);
	if (IS_ERR_OR_NULL(mtd))
		return CMD_RET_FAILURE;

	if (user)
		ret = mtd_get_user_prot_info(mtd, sizeof(otp_info), &retlen,
					     &otp_info);
	else
		ret = mtd_get_fact_prot_info(mtd, sizeof(otp_info), &retlen,
					     &otp_info);
	if (ret) {
		pr_err("OTP info failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto put_mtd;
	}

	if (retlen != sizeof(otp_info)) {
		pr_err("OTP info returns %zu, but %zu expected\n",
		       retlen, sizeof(otp_info));
		ret = CMD_RET_FAILURE;
		goto put_mtd;
	}

	printf("%s OTP region info:\n", user ? "User" : "Factory");
	printf("\tstart: %u\n", otp_info.start);
	printf("\tlength: %u\n", otp_info.length);
	printf("\tlocked: %u\n", otp_info.locked);

	ret = CMD_RET_SUCCESS;

put_mtd:
	put_mtd_device(mtd);

	return ret;
}
#endif

static int do_mtd_list(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct mtd_info *mtd;
	int dev_nb = 0;

	/* Ensure all devices (and their partitions) are probed */
	mtd_probe_devices();

	printf("List of MTD devices:\n");
	mtd_for_each_device(mtd) {
		if (!mtd_is_partition(mtd))
			mtd_show_device(mtd);

		dev_nb++;
	}

	if (!dev_nb) {
		printf("No MTD device found\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int mtd_special_write_oob(struct mtd_info *mtd, u64 off,
				 struct mtd_oob_ops *io_op,
				 bool write_empty_pages, bool woob)
{
	int ret = 0;

	/*
	 * By default, do not write an empty page.
	 * Skip it by simulating a successful write.
	 */
	if (!write_empty_pages && mtd_oob_write_is_empty(io_op)) {
		io_op->retlen = mtd->writesize;
		io_op->oobretlen = woob ? mtd->oobsize : 0;
	} else {
		ret = mtd_write_oob(mtd, off, io_op);
	}

	return ret;
}

static int do_mtd_io(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	bool dump, read, raw, woob, write_empty_pages, has_pages = false;
	u64 start_off, off, len, remaining, default_len;
	struct mtd_oob_ops io_op = {};
	uint user_addr = 0, npages;
	const char *cmd = argv[0];
	struct mtd_info *mtd;
	u32 oob_len;
	u8 *buf;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	mtd = get_mtd_by_name(argv[1]);
	if (IS_ERR_OR_NULL(mtd))
		return CMD_RET_FAILURE;

	if (mtd->type == MTD_NANDFLASH || mtd->type == MTD_MLCNANDFLASH)
		has_pages = true;

	dump = !strncmp(cmd, "dump", 4);
	read = dump || !strncmp(cmd, "read", 4);
	raw = strstr(cmd, ".raw");
	woob = strstr(cmd, ".oob");
	write_empty_pages = !has_pages || strstr(cmd, ".dontskipff");

	argc -= 2;
	argv += 2;

	if (!dump) {
		if (!argc) {
			ret = CMD_RET_USAGE;
			goto out_put_mtd;
		}

		user_addr = hextoul(argv[0], NULL);
		argc--;
		argv++;
	}

	start_off = argc > 0 ? hextoul(argv[0], NULL) : 0;
	if (!mtd_is_aligned_with_min_io_size(mtd, start_off)) {
		printf("Offset not aligned with a page (0x%x)\n",
		       mtd->writesize);
		ret = CMD_RET_FAILURE;
		goto out_put_mtd;
	}

	default_len = dump ? mtd->writesize : mtd->size;
	len = argc > 1 ? hextoul(argv[1], NULL) : default_len;
	if (!mtd_is_aligned_with_min_io_size(mtd, len)) {
		len = round_up(len, mtd->writesize);
		printf("Size not on a page boundary (0x%x), rounding to 0x%llx\n",
		       mtd->writesize, len);
	}

	remaining = len;
	npages = mtd_len_to_pages(mtd, len);
	oob_len = woob ? npages * mtd->oobsize : 0;

	if (dump)
		buf = kmalloc(len + oob_len, GFP_KERNEL);
	else
		buf = map_sysmem(user_addr, 0);

	if (!buf) {
		printf("Could not map/allocate the user buffer\n");
		ret = CMD_RET_FAILURE;
		goto out_put_mtd;
	}

	if (has_pages)
		printf("%s %lld byte(s) (%d page(s)) at offset 0x%08llx%s%s%s\n",
		       read ? "Reading" : "Writing", len, npages, start_off,
		       raw ? " [raw]" : "", woob ? " [oob]" : "",
		       !read && write_empty_pages ? " [dontskipff]" : "");
	else
		printf("%s %lld byte(s) at offset 0x%08llx\n",
		       read ? "Reading" : "Writing", len, start_off);

	io_op.mode = raw ? MTD_OPS_RAW : MTD_OPS_AUTO_OOB;
	io_op.len = has_pages ? mtd->writesize : len;
	io_op.ooblen = woob ? mtd->oobsize : 0;
	io_op.datbuf = buf;
	io_op.oobbuf = woob ? &buf[len] : NULL;

	/* Search for the first good block after the given offset */
	off = start_off;
	while (mtd_block_isbad(mtd, off))
		off += mtd->erasesize;

	led_activity_blink();

	/* Loop over the pages to do the actual read/write */
	while (remaining) {
		/* Skip the block if it is bad */
		if (mtd_is_aligned_with_block_size(mtd, off) &&
		    mtd_block_isbad(mtd, off)) {
			off += mtd->erasesize;
			continue;
		}

		if (read)
			ret = mtd_read_oob(mtd, off, &io_op);
		else
			ret = mtd_special_write_oob(mtd, off, &io_op,
						    write_empty_pages, woob);

		if (ret) {
			printf("Failure while %s at offset 0x%llx\n",
			       read ? "reading" : "writing", off);
			break;
		}

		off += io_op.retlen;
		remaining -= io_op.retlen;
		io_op.datbuf += io_op.retlen;
		io_op.oobbuf += io_op.oobretlen;
	}

	led_activity_off();

	if (!ret && dump)
		mtd_dump_device_buf(mtd, start_off, buf, len, woob);

	if (dump)
		kfree(buf);
	else
		unmap_sysmem(buf);

	if (ret) {
		printf("%s on %s failed with error %d\n",
		       read ? "Read" : "Write", mtd->name, ret);
		ret = CMD_RET_FAILURE;
	} else {
		ret = CMD_RET_SUCCESS;
	}

out_put_mtd:
	put_mtd_device(mtd);

	return ret;
}

static int do_mtd_erase(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct erase_info erase_op = {};
	struct mtd_info *mtd;
	u64 off, len;
	bool scrub;
	int ret = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	mtd = get_mtd_by_name(argv[1]);
	if (IS_ERR_OR_NULL(mtd))
		return CMD_RET_FAILURE;

	scrub = strstr(argv[0], ".dontskipbad");

	argc -= 2;
	argv += 2;

	off = argc > 0 ? hextoul(argv[0], NULL) : 0;
	len = argc > 1 ? hextoul(argv[1], NULL) : mtd->size;

	if (!mtd_is_aligned_with_block_size(mtd, off)) {
		printf("Offset not aligned with a block (0x%x)\n",
		       mtd->erasesize);
		ret = CMD_RET_FAILURE;
		goto out_put_mtd;
	}

	if (!mtd_is_aligned_with_block_size(mtd, len)) {
		printf("Size not a multiple of a block (0x%x)\n",
		       mtd->erasesize);
		ret = CMD_RET_FAILURE;
		goto out_put_mtd;
	}

	printf("Erasing 0x%08llx ... 0x%08llx (%d eraseblock(s))\n",
	       off, off + len - 1, mtd_div_by_eb(len, mtd));

	erase_op.mtd = mtd;
	erase_op.addr = off;
	erase_op.len = mtd->erasesize;

	led_activity_blink();

	while (len) {
		if (!scrub) {
			ret = mtd_block_isbad(mtd, erase_op.addr);
			if (ret < 0) {
				printf("Failed to get bad block at 0x%08llx\n",
				       erase_op.addr);
				ret = CMD_RET_FAILURE;
				goto out_put_mtd;
			}

			if (ret > 0) {
				printf("Skipping bad block at 0x%08llx\n",
				       erase_op.addr);
				ret = 0;
				len -= mtd->erasesize;
				erase_op.addr += mtd->erasesize;
				continue;
			}
		}

		ret = mtd_erase(mtd, &erase_op);
		if (ret && ret != -EIO)
			break;

		len -= mtd->erasesize;
		erase_op.addr += mtd->erasesize;
	}

	led_activity_off();

	if (ret && ret != -EIO)
		ret = CMD_RET_FAILURE;
	else
		ret = CMD_RET_SUCCESS;

out_put_mtd:
	put_mtd_device(mtd);

	return ret;
}

static int do_mtd_bad(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct mtd_info *mtd;
	loff_t off;

	if (argc < 2)
		return CMD_RET_USAGE;

	mtd = get_mtd_by_name(argv[1]);
	if (IS_ERR_OR_NULL(mtd))
		return CMD_RET_FAILURE;

	if (!mtd_can_have_bb(mtd)) {
		printf("Only NAND-based devices can have bad blocks\n");
		goto out_put_mtd;
	}

	printf("MTD device %s bad blocks list:\n", mtd->name);
	for (off = 0; off < mtd->size; off += mtd->erasesize) {
		if (mtd_block_isbad(mtd, off))
			printf("\t0x%08llx\n", off);
	}

out_put_mtd:
	put_mtd_device(mtd);

	return CMD_RET_SUCCESS;
}

#ifdef CONFIG_AUTO_COMPLETE
static int mtd_name_complete(int argc, char *const argv[], char last_char,
			     int maxv, char *cmdv[])
{
	int len = 0, n_found = 0;
	struct mtd_info *mtd;

	argc--;
	argv++;

	if (argc > 1 ||
	    (argc == 1 && (last_char == '\0' || isblank(last_char))))
		return 0;

	if (argc)
		len = strlen(argv[0]);

	mtd_for_each_device(mtd) {
		if (argc &&
		    (len > strlen(mtd->name) ||
		     strncmp(argv[0], mtd->name, len)))
			continue;

		if (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			break;
		}

		cmdv[n_found++] = mtd->name;
	}

	cmdv[n_found] = NULL;

	return n_found;
}
#endif /* CONFIG_AUTO_COMPLETE */

U_BOOT_LONGHELP(mtd,
	"- generic operations on memory technology devices\n\n"
	"mtd list\n"
	"mtd read[.raw][.oob]                  <name> <addr> [<off> [<size>]]\n"
	"mtd dump[.raw][.oob]                  <name>        [<off> [<size>]]\n"
	"mtd write[.raw][.oob][.dontskipff]    <name> <addr> [<off> [<size>]]\n"
	"mtd erase[.dontskipbad]               <name>        [<off> [<size>]]\n"
	"\n"
	"Specific functions:\n"
	"mtd bad                               <name>\n"
#if CONFIG_IS_ENABLED(CMD_MTD_OTP)
	"mtd otpread                           <name> [u|f] <off> <size>\n"
	"mtd otpwrite                          <name> <off> <hex string>\n"
	"mtd otplock                           <name> <off> <size>\n"
	"mtd otpinfo                           <name> [u|f]\n"
#endif
	"\n"
	"With:\n"
	"\t<name>: NAND partition/chip name (or corresponding DM device name or OF path)\n"
	"\t<addr>: user address from/to which data will be retrieved/stored\n"
	"\t<off>: offset in <name> in bytes (default: start of the part)\n"
	"\t\t* must be block-aligned for erase\n"
	"\t\t* must be page-aligned otherwise\n"
	"\t<size>: length of the operation in bytes (default: the entire device)\n"
	"\t\t* must be a multiple of a block for erase\n"
	"\t\t* must be a multiple of a page otherwise (special case: default is a page with dump)\n"
#if CONFIG_IS_ENABLED(CMD_MTD_OTP)
	"\t<hex string>: hex string without '0x' and spaces. Example: ABCD1234\n"
	"\t[u|f]: user or factory OTP region\n"
#endif
	"\n"
	"The .dontskipff option forces writing empty pages, don't use it if unsure.\n");

U_BOOT_CMD_WITH_SUBCMDS(mtd, "MTD utils", mtd_help_text,
#if CONFIG_IS_ENABLED(CMD_MTD_OTP)
		U_BOOT_SUBCMD_MKENT(otpread, 5, 1, do_mtd_otp_read),
		U_BOOT_SUBCMD_MKENT(otpwrite, 4, 1, do_mtd_otp_write),
		U_BOOT_SUBCMD_MKENT(otplock, 4, 1, do_mtd_otp_lock),
		U_BOOT_SUBCMD_MKENT(otpinfo, 3, 1, do_mtd_otp_info),
#endif
		U_BOOT_SUBCMD_MKENT(list, 1, 1, do_mtd_list),
		U_BOOT_SUBCMD_MKENT_COMPLETE(read, 5, 0, do_mtd_io,
					     mtd_name_complete),
		U_BOOT_SUBCMD_MKENT_COMPLETE(write, 5, 0, do_mtd_io,
					     mtd_name_complete),
		U_BOOT_SUBCMD_MKENT_COMPLETE(dump, 4, 0, do_mtd_io,
					     mtd_name_complete),
		U_BOOT_SUBCMD_MKENT_COMPLETE(erase, 4, 0, do_mtd_erase,
					     mtd_name_complete),
		U_BOOT_SUBCMD_MKENT_COMPLETE(bad, 2, 1, do_mtd_bad,
					     mtd_name_complete));

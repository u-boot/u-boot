// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for ChromiumOS
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <blk.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootm.h>
#include <bootmeth.h>
#include <display_options.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <part.h>
#include <linux/sizes.h>
#include "bootmeth_cros.h"

enum {
	PROBE_SIZE	= SZ_4K,	/* initial bytes read from partition */

	X86_SETUP_OFFSET = -0x1000,	/* setup offset relative to base */
	CMDLINE_OFFSET	= -0x2000,	/* cmdline offset relative to base */
	X86_KERNEL_OFFSET = 0x4000,	/* kernel offset relative to base */
};

static int cros_check(struct udevice *dev, struct bootflow_iter *iter)
{
	/* This only works on block and network devices */
	if (bootflow_iter_check_blk(iter))
		return log_msg_ret("blk", -ENOTSUPP);

	return 0;
}

static int copy_cmdline(const char *from, const char *uuid, char **bufp)
{
	const int maxlen = 2048;
	char buf[maxlen];
	char *cmd, *to, *end;
	int len;

	/* Allow space for cmdline + UUID */
	len = strnlen(from, sizeof(buf));
	if (len >= maxlen)
		return -E2BIG;

	log_debug("uuid %d %s\n", uuid ? (int)strlen(uuid) : 0, uuid);
	for (to = buf, end = buf + maxlen - UUID_STR_LEN - 1; *from; from++) {
		if (to >= end)
			return -E2BIG;
		if (from[0] == '%' && from[1] == 'U' && uuid &&
		    strlen(uuid) == UUID_STR_LEN) {
			strcpy(to, uuid);
			to += UUID_STR_LEN;
			from++;
		} else {
			*to++ = *from;
		}
	}
	*to = '\0';
	len = to - buf;
	cmd = strdup(buf);
	if (!cmd)
		return -ENOMEM;
	free(*bufp);
	*bufp = cmd;

	return 0;
}

/**
 * scan_part() - Scan a kernel partition to see if has a ChromeOS header
 *
 * This reads the first PROBE_SIZE of a partition, loookng for
 * VB2_KEYBLOCK_MAGIC
 *
 * @blk: Block device to scan
 * @partnum: Partition number to scan
 * @info: Please to put partition info
 * @hdrp: Return allocated keyblock header on success
 */
static int scan_part(struct udevice *blk, int partnum,
		     struct disk_partition *info, struct vb2_keyblock **hdrp)
{
	struct blk_desc *desc = dev_get_uclass_plat(blk);
	struct vb2_keyblock *hdr;
	ulong num_blks;
	int ret;

	ret = part_get_info(desc, partnum, info);
	if (ret)
		return log_msg_ret("part", ret);

	/* Make a buffer for the header information */
	num_blks = PROBE_SIZE >> desc->log2blksz;
	log_debug("Reading header, blk=%s, start=%lx, blocks=%lx\n",
		  blk->name, (ulong)info->start, num_blks);
	hdr = memalign(SZ_1K, PROBE_SIZE);
	if (!hdr)
		return log_msg_ret("hdr", -ENOMEM);
	ret = blk_read(blk, info->start, num_blks, hdr);
	if (ret != num_blks) {
		free(hdr);
		return log_msg_ret("inf", -EIO);
	}

	if (memcmp(VB2_KEYBLOCK_MAGIC, hdr->magic, VB2_KEYBLOCK_MAGIC_SIZE)) {
		free(hdr);
		return -ENOENT;
	}

	*hdrp = hdr;

	return 0;
}

static int cros_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	ulong base, setup, cmdline, num_blks, kern_base;
	const struct vb2_kernel_preamble *preamble;
	ulong body_offset, body_size;
	struct disk_partition info;
	const char *uuid = NULL;
	struct vb2_keyblock *hdr;
	int part, ret;
	void *buf;

	log_debug("starting, part=%d\n", bflow->part);

	/* We consider the whole disk, not any one partition */
	if (bflow->part)
		return log_msg_ret("max", -ENOENT);

	/* Check partition 2 then 4 */
	part = 2;
	ret = scan_part(bflow->blk, part, &info, &hdr);
	if (ret) {
		part = 4;
		ret = scan_part(bflow->blk, part, &info, &hdr);
		if (ret)
			return log_msg_ret("scan", ret);
	}
	bflow->part = part;

	log_info("Selected parition %d, header at %lx\n", bflow->part,
		 (ulong)map_to_sysmem(hdr));
	preamble = (void *)hdr + hdr->keyblock_size;
	log_debug("Kernel preamble at %lx, version major %x, minor %x\n",
		  (ulong)map_to_sysmem(preamble),
		  preamble->header_version_major,
		  preamble->header_version_minor);

	log_debug("  - load_address %lx, bl_addr %lx, bl_size %lx\n",
		  (ulong)preamble->body_load_address,
		  (ulong)preamble->bootloader_address,
		  (ulong)preamble->bootloader_size);

	body_offset = hdr->keyblock_size + preamble->preamble_size;
	body_size = preamble->body_signature.data_size;
	log_debug("Kernel body at %lx size %lx\n", body_offset, body_size);
	bflow->size = body_size;

	buf = memalign(SZ_1K, body_size);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);

	/* Check that the header is not smaller than permitted */
	if (body_offset < PROBE_SIZE)
		return log_msg_ret("san", EFAULT);

	/* Read kernel body */
	num_blks = body_size >> desc->log2blksz;
	log_debug("Reading body to %lx, blk=%s, size=%lx, blocks=%lx\n",
		  (ulong)map_to_sysmem(buf), bflow->blk->name, body_size,
		  num_blks);
	ret = blk_read(bflow->blk,
		       info.start + (body_offset >> desc->log2blksz),
		       num_blks, buf);
	if (ret != num_blks)
		return log_msg_ret("inf", -EIO);
	base = map_to_sysmem(buf) + preamble->bootloader_address -
		preamble->body_load_address;

	setup = base + X86_SETUP_OFFSET;
	cmdline = base + CMDLINE_OFFSET;
	kern_base = base + X86_KERNEL_OFFSET;
	log_debug("base %lx setup %lx cmdline %lx kern_base %lx\n", base,
		  setup, cmdline, kern_base);

#ifdef CONFIG_X86
	const char *version;

	version = zimage_get_kernel_version(map_sysmem(setup, 0),
					    map_sysmem(kern_base, 0));
	log_debug("version %s\n", version);
	if (version)
		bflow->name = strdup(version);
#endif
	if (!bflow->name)
		bflow->name = strdup("ChromeOS");
	if (!bflow->name)
		return log_msg_ret("nam", -ENOMEM);
	bflow->os_name = strdup("ChromeOS");
	if (!bflow->os_name)
		return log_msg_ret("os", -ENOMEM);

#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
	uuid = info.uuid;
#endif
	ret = copy_cmdline(map_sysmem(cmdline, 0), uuid, &bflow->cmdline);
	if (ret)
		return log_msg_ret("cmd", ret);

	bflow->state = BOOTFLOWST_READY;
	bflow->buf = buf;
	bflow->x86_setup = map_sysmem(setup, 0);

	return 0;
}

static int cros_read_file(struct udevice *dev, struct bootflow *bflow,
			 const char *file_path, ulong addr, ulong *sizep)
{
	return -ENOSYS;
}

static int cros_boot(struct udevice *dev, struct bootflow *bflow)
{
#ifdef CONFIG_X86
	zboot_start(map_to_sysmem(bflow->buf), bflow->size, 0, 0,
		    map_to_sysmem(bflow->x86_setup),
		    bflow->cmdline);
#endif

	return log_msg_ret("go", -EFAULT);
}

static int cros_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = "ChromiumOS boot";

	return 0;
}

static struct bootmeth_ops cros_bootmeth_ops = {
	.check		= cros_check,
	.read_bootflow	= cros_read_bootflow,
	.read_file	= cros_read_file,
	.boot		= cros_boot,
};

static const struct udevice_id cros_bootmeth_ids[] = {
	{ .compatible = "u-boot,cros" },
	{ }
};

U_BOOT_DRIVER(bootmeth_cros) = {
	.name		= "bootmeth_cros",
	.id		= UCLASS_BOOTMETH,
	.of_match	= cros_bootmeth_ids,
	.ops		= &cros_bootmeth_ops,
	.bind		= cros_bootmeth_bind,
};

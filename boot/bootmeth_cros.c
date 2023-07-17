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
#include <bootmeth.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <part.h>
#ifdef CONFIG_X86
#include <asm/zimage.h>
#endif
#include <linux/sizes.h>

enum {
	/* Offsets in the kernel-partition header */
	KERN_START	= 0x4f0,
	KERN_SIZE	= 0x518,

	SETUP_OFFSET	= 0x1000,	/* bytes before base */
	CMDLINE_OFFSET	= 0x2000,	/* bytes before base */
	OFFSET_BASE	= 0x100000,	/* assumed kernel load-address */
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

static int cros_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	ulong base, start, size, setup, cmdline, num_blks, kern_base;
	struct disk_partition info;
	const char *uuid = NULL;
	void *buf, *hdr;
	int ret;

	log_debug("starting, part=%d\n", bflow->part);

	/* We consider the whole disk, not any one partition */
	if (bflow->part)
		return log_msg_ret("max", -ENOENT);

	/* Check partition 2 */
	ret = part_get_info(desc, 2, &info);
	if (ret)
		return log_msg_ret("part", ret);

	/* Make a buffer for the header information */
	num_blks = SZ_4K >> desc->log2blksz;
	log_debug("Reading header, blk=%s, start=%lx, blocks=%lx\n",
		  bflow->blk->name, (ulong)info.start, num_blks);
	hdr = memalign(SZ_1K, SZ_4K);
	if (!hdr)
		return log_msg_ret("hdr", -ENOMEM);
	ret = blk_read(bflow->blk, info.start, num_blks, hdr);
	if (ret != num_blks)
		return log_msg_ret("inf", ret);

	if (memcmp("CHROMEOS", hdr, 8))
		return -ENOENT;

	log_info("Header at %lx\n", (ulong)map_to_sysmem(hdr));
	start = *(u32 *)(hdr + KERN_START);
	size = ALIGN(*(u32 *)(hdr + KERN_SIZE), desc->blksz);
	log_debug("Reading start %lx size %lx\n", start, size);
	bflow->size = size;

	buf = memalign(SZ_1K, size);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);
	num_blks = size >> desc->log2blksz;
	log_debug("Reading data, blk=%s, start=%lx, blocks=%lx\n",
		  bflow->blk->name, (ulong)info.start, num_blks);
	ret = blk_read(bflow->blk, (ulong)info.start + 0x80, num_blks, buf);
	if (ret != num_blks)
		return log_msg_ret("inf", ret);
	base = map_to_sysmem(buf);

	setup = base + start - OFFSET_BASE - SETUP_OFFSET;
	cmdline = base + start - OFFSET_BASE - CMDLINE_OFFSET;
	kern_base = base + start - OFFSET_BASE + SZ_16K;
	log_debug("base %lx setup %lx, cmdline %lx, kern_base %lx\n", base,
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

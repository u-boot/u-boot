// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for distro boot using PXE (network boot)
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <command.h>
#include <distro.h>
#include <dm.h>
#include <fs.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <mmc.h>
#include <net.h>
#include <pxe_utils.h>

static int disto_pxe_getfile(struct pxe_context *ctx, const char *file_path,
			     char *file_addr, ulong *sizep)
{
	struct distro_info *info = ctx->userdata;
	ulong addr;
	int ret;

	addr = simple_strtoul(file_addr, NULL, 16);
	ret = bootmeth_read_file(info->dev, info->bflow, file_path, addr,
				 sizep);
	if (ret)
		return log_msg_ret("read", ret);

	return 0;
}

static int distro_pxe_check(struct udevice *dev, struct bootflow_iter *iter)
{
	int ret;

	/* This only works on network devices */
	ret = bootflow_iter_uses_network(iter);
	if (ret)
		return log_msg_ret("net", ret);

	return 0;
}

static int distro_pxe_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	const char *addr_str;
	char fname[200];
	char *bootdir;
	ulong addr;
	ulong size;
	char *buf;
	int ret;

	addr_str = env_get("pxefile_addr_r");
	if (!addr_str)
		return log_msg_ret("pxeb", -EPERM);
	addr = simple_strtoul(addr_str, NULL, 16);

	log_debug("calling pxe_get()\n");
	ret = pxe_get(addr, &bootdir, &size);
	log_debug("pxe_get() returned %d\n", ret);
	if (ret)
		return log_msg_ret("pxeb", ret);
	bflow->size = size;

	/* Use the directory of the dhcp bootdir as our subdir, if provided */
	if (bootdir) {
		const char *last_slash;
		int path_len;

		last_slash = strrchr(bootdir, '/');
		if (last_slash) {
			path_len = (last_slash - bootdir) + 1;
			bflow->subdir = malloc(path_len + 1);
			memcpy(bflow->subdir, bootdir, path_len);
			bflow->subdir[path_len] = '\0';
		}
	}
	snprintf(fname, sizeof(fname), "%s%s",
		 bflow->subdir ? bflow->subdir : "", DISTRO_FNAME);

	bflow->fname = strdup(fname);
	if (!bflow->fname)
		return log_msg_ret("name", -ENOMEM);

	bflow->state = BOOTFLOWST_READY;

	/* Allocate the buffer, including the \0 byte added by get_pxe_file() */
	buf = malloc(size + 1);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);
	memcpy(buf, map_sysmem(addr, 0), size + 1);
	bflow->buf = buf;

	return 0;
}

static int distro_pxe_read_file(struct udevice *dev, struct bootflow *bflow,
				const char *file_path, ulong addr, ulong *sizep)
{
	char *tftp_argv[] = {"tftp", NULL, NULL, NULL};
	struct pxe_context *ctx = dev_get_priv(dev);
	char file_addr[17];
	ulong size;
	int ret;

	sprintf(file_addr, "%lx", addr);
	tftp_argv[1] = file_addr;
	tftp_argv[2] = (void *)file_path;

	if (do_tftpb(ctx->cmdtp, 0, 3, tftp_argv))
		return -ENOENT;
	ret = pxe_get_file_size(&size);
	if (ret)
		return log_msg_ret("tftp", ret);
	if (size > *sizep)
		return log_msg_ret("spc", -ENOSPC);
	*sizep = size;

	return 0;
}

static int distro_pxe_boot(struct udevice *dev, struct bootflow *bflow)
{
	struct pxe_context *ctx = dev_get_priv(dev);
	struct cmd_tbl cmdtp = {};	/* dummy */
	struct distro_info info;
	ulong addr;
	int ret;

	addr = map_to_sysmem(bflow->buf);
	info.dev = dev;
	info.bflow = bflow;
	info.cmdtp = &cmdtp;
	ret = pxe_setup_ctx(ctx, &cmdtp, disto_pxe_getfile, &info, false,
			    bflow->subdir);
	if (ret)
		return log_msg_ret("ctx", -EINVAL);

	ret = pxe_process(ctx, addr, false);
	if (ret)
		return log_msg_ret("bread", -EINVAL);

	return 0;
}

static int distro_bootmeth_pxe_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"PXE boot from a network device" : "PXE";

	return 0;
}

static struct bootmeth_ops distro_bootmeth_pxe_ops = {
	.check		= distro_pxe_check,
	.read_bootflow	= distro_pxe_read_bootflow,
	.read_file	= distro_pxe_read_file,
	.boot		= distro_pxe_boot,
};

static const struct udevice_id distro_bootmeth_pxe_ids[] = {
	{ .compatible = "u-boot,distro-pxe" },
	{ }
};

U_BOOT_DRIVER(bootmeth_pxe) = {
	.name		= "bootmeth_pxe",
	.id		= UCLASS_BOOTMETH,
	.of_match	= distro_bootmeth_pxe_ids,
	.ops		= &distro_bootmeth_pxe_ops,
	.bind		= distro_bootmeth_pxe_bind,
	.priv_auto	= sizeof(struct pxe_context),
};

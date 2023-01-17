// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for distro boot via EFI
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
#include <dm.h>
#include <efi_loader.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <mmc.h>
#include <net.h>
#include <pxe_utils.h>

#define EFI_DIRNAME	"efi/boot/"

/**
 * get_efi_leafname() - Get the leaf name for the EFI file we expect
 *
 * @str: Place to put leaf name for this architecture, e.g. "bootaa64.efi".
 *	Must have at least 16 bytes of space
 * @max_len: Length of @str, must be >=16
 */
static int get_efi_leafname(char *str, int max_len)
{
	const char *base;

	if (max_len < 16)
		return log_msg_ret("spc", -ENOSPC);
	if (IS_ENABLED(CONFIG_ARM64))
		base = "bootaa64";
	else if (IS_ENABLED(CONFIG_ARM))
		base = "bootarm";
	else if (IS_ENABLED(CONFIG_X86_RUN_32BIT))
		base = "bootia32";
	else if (IS_ENABLED(CONFIG_X86_RUN_64BIT))
		base = "bootx64";
	else if (IS_ENABLED(CONFIG_ARCH_RV32I))
		base = "bootriscv32";
	else if (IS_ENABLED(CONFIG_ARCH_RV64I))
		base = "bootriscv64";
	else if (IS_ENABLED(CONFIG_SANDBOX))
		base = "bootsbox";
	else
		return -EINVAL;

	strcpy(str, base);
	strcat(str, ".efi");

	return 0;
}

static int get_efi_pxe_arch(void)
{
	/* http://www.iana.org/assignments/dhcpv6-parameters/dhcpv6-parameters.xml */
	if (IS_ENABLED(CONFIG_ARM64))
		return 0xb;
	else if (IS_ENABLED(CONFIG_ARM))
		return 0xa;
	else if (IS_ENABLED(CONFIG_X86_64))
		return 0x6;
	else if (IS_ENABLED(CONFIG_X86))
		return 0x7;
	else if (IS_ENABLED(CONFIG_ARCH_RV32I))
		return 0x19;
	else if (IS_ENABLED(CONFIG_ARCH_RV64I))
		return 0x1b;
	else if (IS_ENABLED(CONFIG_SANDBOX))
		return 0;	/* not used */

	return -EINVAL;
}

static int get_efi_pxe_vci(char *str, int max_len)
{
	int ret;

	ret = get_efi_pxe_arch();
	if (ret < 0)
		return ret;

	snprintf(str, max_len, "PXEClient:Arch:%05x:UNDI:003000", ret);

	return 0;
}

static int efiload_read_file(struct blk_desc *desc, struct bootflow *bflow)
{
	const struct udevice *media_dev;
	int size = bflow->size;
	const char *dev_name;
	char devnum_str[9];
	char dirname[200];
	char *last_slash;
	int ret;

	ret = bootmeth_alloc_file(bflow, 0x2000000, 0x10000);
	if (ret)
		return log_msg_ret("read", ret);

	/*
	 * This is a horrible hack to tell EFI about this boot device. Once we
	 * unify EFI with the rest of U-Boot we can clean this up. The same hack
	 * exists in multiple places, e.g. in the fs, tftp and load commands.
	 *
	 * Once we can clean up the EFI code to make proper use of driver model,
	 * this can go away.
	 */
	media_dev = dev_get_parent(bflow->dev);
	snprintf(devnum_str, sizeof(devnum_str), "%x", dev_seq(media_dev));

	strlcpy(dirname, bflow->fname, sizeof(dirname));
	last_slash = strrchr(dirname, '/');
	if (last_slash)
		*last_slash = '\0';

	log_debug("setting bootdev %s, %s, %s, %p, %x\n",
		  dev_get_uclass_name(media_dev), devnum_str, bflow->fname,
		  bflow->buf, size);
	dev_name = device_get_uclass_id(media_dev) == UCLASS_MASS_STORAGE ?
		 "usb" : dev_get_uclass_name(media_dev);
	efi_set_bootdev(dev_name, devnum_str, bflow->fname, bflow->buf, size);

	return 0;
}

static int distro_efi_check(struct udevice *dev, struct bootflow_iter *iter)
{
	/* This only works on block and network devices */
	if (bootflow_iter_check_blk(iter) && bootflow_iter_check_net(iter))
		return log_msg_ret("blk", -ENOTSUPP);

	return 0;
}

static int distro_efi_read_bootflow_file(struct udevice *dev,
					 struct bootflow *bflow)
{
	struct blk_desc *desc = NULL;
	char fname[256];
	int ret;

	/* We require a partition table */
	if (!bflow->part)
		return -ENOENT;

	strcpy(fname, EFI_DIRNAME);
	ret = get_efi_leafname(fname + strlen(fname),
			       sizeof(fname) - strlen(fname));
	if (ret)
		return log_msg_ret("leaf", ret);

	if (bflow->blk)
		 desc = dev_get_uclass_plat(bflow->blk);
	ret = bootmeth_try_file(bflow, desc, NULL, fname);
	if (ret)
		return log_msg_ret("try", ret);

	ret = efiload_read_file(desc, bflow);
	if (ret)
		return log_msg_ret("read", -EINVAL);

	return 0;
}

static int distro_efi_read_bootflow_net(struct bootflow *bflow)
{
	const char *addr_str;
	int ret, arch, size;
	char str[36];
	ulong addr;

	ret = get_efi_pxe_vci(str, sizeof(str));
	if (ret)
		return log_msg_ret("vci", ret);
	ret = get_efi_pxe_arch();
	if (ret < 0)
		return log_msg_ret("arc", ret);
	arch = ret;

	ret = env_set("bootp_vci", str);
	if (ret)
		return log_msg_ret("vcs", ret);
	ret = env_set_ulong("bootp_arch", arch);
	if (ret)
		return log_msg_ret("ars", ret);

	/* figure out the load address */
	addr_str = env_get("kernel_addr_r");
	addr = addr_str ? hextoul(addr_str, NULL) : image_load_addr;

	/* clear any previous bootfile */
	env_set("bootfile", NULL);

	/* read the kernel */
	ret = dhcp_run(addr, NULL, true);
	if (ret)
		return log_msg_ret("dhc", ret);

	size = env_get_hex("filesize", -1);
	if (size <= 0)
		return log_msg_ret("sz", -EINVAL);
	bflow->size = size;

	/* do the hideous EFI hack */
	efi_set_bootdev("Net", "", bflow->fname, map_sysmem(addr, 0),
			bflow->size);

	bflow->state = BOOTFLOWST_READY;

	return 0;
}

static int distro_efi_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	const struct udevice *media = dev_get_parent(bflow->dev);
	int ret;

	if (IS_ENABLED(CONFIG_CMD_DHCP) &&
	    device_get_uclass_id(media) == UCLASS_ETH) {
		/* we only support reading from one device, so ignore 'dev' */
		ret = distro_efi_read_bootflow_net(bflow);
		if (ret)
			return log_msg_ret("net", ret);
	} else {
		ret = distro_efi_read_bootflow_file(dev, bflow);
		if (ret)
			return log_msg_ret("blk", ret);
	}

	return 0;
}

int distro_efi_boot(struct udevice *dev, struct bootflow *bflow)
{
	char cmd[50];

	/*
	 * At some point we can add a real interface to bootefi so we can call
	 * this directly. For now, go through the CLI like distro boot.
	 */
	snprintf(cmd, sizeof(cmd), "bootefi %lx %lx",
		 (ulong)map_to_sysmem(bflow->buf),
		 (ulong)map_to_sysmem(gd->fdt_blob));
	if (run_command(cmd, 0))
		return log_msg_ret("run", -EINVAL);

	return 0;
}

static int distro_bootmeth_efi_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"EFI boot from an .efi file" : "EFI";

	return 0;
}

static struct bootmeth_ops distro_efi_bootmeth_ops = {
	.check		= distro_efi_check,
	.read_bootflow	= distro_efi_read_bootflow,
	.read_file	= bootmeth_common_read_file,
	.boot		= distro_efi_boot,
};

static const struct udevice_id distro_efi_bootmeth_ids[] = {
	{ .compatible = "u-boot,distro-efi" },
	{ }
};

U_BOOT_DRIVER(bootmeth_efi) = {
	.name		= "bootmeth_efi",
	.id		= UCLASS_BOOTMETH,
	.of_match	= distro_efi_bootmeth_ids,
	.ops		= &distro_efi_bootmeth_ops,
	.bind		= distro_bootmeth_efi_bind,
};

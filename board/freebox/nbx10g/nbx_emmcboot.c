// SPDX-License-Identifier: GPL-2.0+
/*
 * Nodebox 10G dual-bank eMMC boot command with automatic fallback
 *
 * Copyright (C) 2026 Free Mobile, Freebox
 *
 * This implements a dual-bank boot system with automatic fallback:
 * - Bank0: Stable/fallback boot image
 * - Bank1: Newer/test boot image
 *
 * The boot order depends on the reboot tracking counter (nrboot):
 * - If healthy: try Bank1 first, then Bank0
 * - If degraded (>= 4 failures): try Bank0 first, then Bank1
 */

#include <command.h>
#include <env.h>
#include <mmc.h>
#include <malloc.h>
#include <memalign.h>
#include <vsprintf.h>
#include <u-boot/crc.h>
#include <u-boot/schedule.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include "nbx_imagetag.h"
#include "nbx_nrboot.h"

/* Partition offsets defined in Kconfig (CONFIG_NBX_MMC_PART_*) */

/* Image Tag Functions */

static int mvebu_imagetag_check(struct mvebu_image_tag *tag,
				unsigned long maxsize, const char *name)
{
	if (be32_to_cpu(tag->magic) != MVEBU_IMAGE_TAG_MAGIC) {
		if (name)
			printf("%s: invalid TAG magic: %.8x\n", name,
			       be32_to_cpu(tag->magic));
		return -EINVAL;
	}

	if (be32_to_cpu(tag->version) != MVEBU_IMAGE_TAG_VERSION) {
		if (name)
			printf("%s: invalid TAG version: %.8x\n", name,
			       be32_to_cpu(tag->version));
		return -EINVAL;
	}

	if (be32_to_cpu(tag->total_size) < sizeof(*tag)) {
		if (name)
			printf("%s: tag size is too small!\n", name);
		return -EINVAL;
	}

	if (be32_to_cpu(tag->total_size) > maxsize) {
		if (name)
			printf("%s: tag size is too big!\n", name);
		return -EINVAL;
	}

	if (be32_to_cpu(tag->device_tree_offset) < sizeof(*tag) ||
	    be32_to_cpu(tag->device_tree_offset) +
	    be32_to_cpu(tag->device_tree_size) > maxsize) {
		if (name)
			printf("%s: bogus device tree offset/size!\n", name);
		return -EINVAL;
	}

	if (be32_to_cpu(tag->kernel_offset) < sizeof(*tag) ||
	    be32_to_cpu(tag->kernel_offset) +
	    be32_to_cpu(tag->kernel_size) > maxsize) {
		if (name)
			printf("%s: bogus kernel offset/size!\n", name);
		return -EINVAL;
	}

	if (be32_to_cpu(tag->rootfs_offset) < sizeof(*tag) ||
	    be32_to_cpu(tag->rootfs_offset) +
	    be32_to_cpu(tag->rootfs_size) > maxsize) {
		if (name)
			printf("%s: bogus rootfs offset/size!\n", name);
		return -EINVAL;
	}

	if (name) {
		/*
		 * Ensure null-termination within the 32-byte fields
		 * before printing to avoid displaying garbage.
		 */
		tag->image_name[sizeof(tag->image_name) - 1] = '\0';
		tag->build_date[sizeof(tag->build_date) - 1] = '\0';
		tag->build_user[sizeof(tag->build_user) - 1] = '\0';

		printf("%s: Found valid tag: %s / %s / %s\n", name,
		       tag->image_name, tag->build_date, tag->build_user);
	}

	return 0;
}

static int mvebu_imagetag_crc(struct mvebu_image_tag *tag, const char *name)
{
	u32 crc = ~0;

	crc = crc32(crc, ((unsigned char *)tag) + 4,
		    be32_to_cpu(tag->total_size) - 4);

	if (be32_to_cpu(tag->crc) != crc) {
		if (name)
			printf("%s: invalid tag CRC!\n", name);
		return -EINVAL;
	}

	return 0;
}

/* NRBoot (Reboot Tracking) Functions */

struct mvebu_nrboot {
	u16 nrboot;
	u16 nrsuccess;
};

#define MVEBU_MAX_FAILURE	4

static int mvebu_count_bits(u16 val)
{
	int i, found = 0;

	for (i = 0; i < 16; i++) {
		if (val & (1 << i))
			found++;
	}
	return found;
}

int mvebu_check_nrboot(struct mmc *mmc, unsigned long offset)
{
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	struct mvebu_nrboot *nr;
	uint blk_start = ALIGN(offset, bd->blksz) / bd->blksz;
	uint blk_cnt = ALIGN(sizeof(*nr), bd->blksz) / bd->blksz;
	uint n;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, blk_cnt * bd->blksz);
	nr = (void *)buf;

	n = blk_dread(bd, blk_start, blk_cnt, buf);
	if (n != blk_cnt)
		return 0;

	printf(" - nr.nrboot = %04x\n", nr->nrboot);
	printf(" - nr.nrsuccess = %04x\n", nr->nrsuccess);

	/* Sanity check on values */
	if (mvebu_count_bits(~nr->nrboot + 1) <= 1 &&
	    mvebu_count_bits(~nr->nrsuccess + 1) <= 1) {
		int boot, success;

		boot = 16 - mvebu_count_bits(nr->nrboot);
		success = 16 - mvebu_count_bits(nr->nrsuccess);

		printf(" - Nrboot: %d / Nrsuccess: %d\n", boot, success);

		if (boot == 16 || boot < success ||
		    boot - success >= MVEBU_MAX_FAILURE) {
			printf(" - Nrboot exceeded\n");
			return 0;
		}

		/* Increment boot attempt counter */
		boot++;
		nr->nrboot = ~((1 << boot) - 1);

		printf(" - Setting Nrboot to %d\n", boot);

		n = blk_dwrite(bd, blk_start, blk_cnt, buf);
		if (n != blk_cnt)
			return 0;

		return 1;
	}

	printf(" - Invalid NR values\n");

	return 0;
}

/* emmcboot Command */

static void mvebu_try_emmcboot(struct mmc *mmc, unsigned long offset,
			       unsigned long maxsize, const char *bank)
{
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	struct mvebu_image_tag *tag;
	ulong image_addr = 0;
	ulong fdt_addr = 0;
	ulong tag_addr;
	uint tag_blk_start = ALIGN(offset, bd->blksz) / bd->blksz;
	uint tag_blk_cnt = ALIGN(sizeof(*tag), bd->blksz) / bd->blksz;
	uint n;

	ALLOC_CACHE_ALIGN_BUFFER(char, tag_buf, tag_blk_cnt * bd->blksz);
	tag = (void *)tag_buf;

	schedule();

	printf("## Trying %s boot...\n", bank);

	/* Load tag header */
	n = blk_dread(bd, tag_blk_start, tag_blk_cnt, tag_buf);
	if (n != tag_blk_cnt) {
		printf("%s: failed to read tag header\n", bank);
		return;
	}

	if (mvebu_imagetag_check(tag, maxsize, bank) != 0)
		return;

	if (tag->rootfs_size != 0) {
		printf("%s: rootfs in tag not supported\n", bank);
		return;
	}

	/* Get image and device tree load addresses from environment */
	image_addr = env_get_ulong("image_addr", 16, 0);
	if (!image_addr) {
		puts("emmcboot needs image_addr\n");
		return;
	}

	fdt_addr = env_get_ulong("fdt_addr", 16, 0);
	if (!fdt_addr) {
		puts("emmcboot needs fdt_addr\n");
		return;
	}

	tag_addr = image_addr;

	/* Load full image, temporarily reuse image_addr for this */
	{
		uint data_blk_start = ALIGN(offset, bd->blksz) / bd->blksz;
		uint data_blk_cnt = ALIGN(mvebu_imagetag_total_size(tag),
					  bd->blksz) / bd->blksz;

		n = blk_dread(bd, data_blk_start, data_blk_cnt, (void *)tag_addr);
		if (n != data_blk_cnt) {
			printf("%s: failed to read full image\n", bank);
			return;
		}

		if (mvebu_imagetag_crc((void *)tag_addr, bank) != 0)
			return;
	}

	schedule();

	/* Copy image and device tree to the right addresses */
	/* We assume that image_addr + tag_size < fdt_addr */
	{
		tag = (void *)tag_addr;
		memcpy((void *)fdt_addr,
		       ((void *)tag_addr) + mvebu_imagetag_device_tree_offset(tag),
		       mvebu_imagetag_device_tree_size(tag));
		memmove((void *)image_addr,
			((void *)tag_addr) + mvebu_imagetag_kernel_offset(tag),
			mvebu_imagetag_kernel_size(tag));
	}

	schedule();

	/* Set bootargs and boot */
	{
		char bootargs[256];
		char *console_env;

		console_env = env_get("console");
		if (console_env)
			snprintf(bootargs, sizeof(bootargs), "%s bank=%s",
				 console_env, bank);
		else
			snprintf(bootargs, sizeof(bootargs), "bank=%s", bank);

		env_set("bootargs", bootargs);

		printf("## Booting kernel from %s...\n", bank);
		printf("   Image addr: 0x%lx\n", image_addr);
		printf("   FDT addr:   0x%lx\n", fdt_addr);

		/* Build and run booti command */
		{
			char cmd[128];

			snprintf(cmd, sizeof(cmd), "booti 0x%lx - 0x%lx",
				 image_addr, fdt_addr);
			run_command(cmd, 0);
		}
	}

	printf("## %s boot failed\n", bank);
}

static int do_emmcboot(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int dev;
	struct mmc *mmc;

	dev = 0;
	if (argc >= 2)
		dev = dectoul(argv[1], NULL);

	mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("No MMC device %d found\n", dev);
		return CMD_RET_FAILURE;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return CMD_RET_FAILURE;
	}

	/* Switch to partition 0 (user data area) */
	if (blk_select_hwpart_devnum(UCLASS_MMC, dev, 0)) {
		puts("MMC partition switch failed\n");
		return CMD_RET_FAILURE;
	}

	if (mvebu_check_nrboot(mmc, CONFIG_NBX_MMC_PART_NRBOOT_OFFSET)) {
		/* System is healthy: try newer bank first */
		mvebu_try_emmcboot(mmc, CONFIG_NBX_MMC_PART_BANK1_OFFSET,
				   CONFIG_NBX_MMC_PART_BANK1_SIZE, "bank1");
		mvebu_try_emmcboot(mmc, CONFIG_NBX_MMC_PART_BANK0_OFFSET,
				   CONFIG_NBX_MMC_PART_BANK0_SIZE, "bank0");
	} else {
		/* System is degraded: use stable bank first */
		mvebu_try_emmcboot(mmc, CONFIG_NBX_MMC_PART_BANK0_OFFSET,
				   CONFIG_NBX_MMC_PART_BANK0_SIZE, "bank0");
		mvebu_try_emmcboot(mmc, CONFIG_NBX_MMC_PART_BANK1_OFFSET,
				   CONFIG_NBX_MMC_PART_BANK1_SIZE, "bank1");
	}

	puts("emmcboot: all boot attempts failed\n");
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	emmcboot, 2, 0, do_emmcboot,
	"boot from MVEBU eMMC image banks",
	"[dev]\n"
	"    - Boot from eMMC device <dev> (default 0)\n"
	"    - Requires image_addr and fdt_addr environment variables\n"
	"    - Uses dual-bank boot with automatic fallback\n"
	"    - Bank selection based on reboot tracking (nrboot)"
);

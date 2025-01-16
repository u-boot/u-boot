// SPDX-License-Identifier: GPL-2.0
/*
 * Verified Boot for Embedded (VBE) common functions
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <bootstage.h>
#include <dm.h>
#include <blk.h>
#include <image.h>
#include <mapmem.h>
#include <memalign.h>
#include <spl.h>
#include <u-boot/crc.h>
#include "vbe_common.h"

int vbe_get_blk(const char *storage, struct udevice **blkp)
{
	struct blk_desc *desc;
	char devname[16];
	const char *end;
	int devnum;

	/* First figure out the block device */
	log_debug("storage=%s\n", storage);
	devnum = trailing_strtoln_end(storage, NULL, &end);
	if (devnum == -1)
		return log_msg_ret("num", -ENODEV);
	if (end - storage >= sizeof(devname))
		return log_msg_ret("end", -E2BIG);
	strlcpy(devname, storage, end - storage + 1);
	log_debug("dev=%s, %x\n", devname, devnum);

	desc = blk_get_dev(devname, devnum);
	if (!desc)
		return log_msg_ret("get", -ENXIO);
	*blkp = desc->bdev;

	return 0;
}

int vbe_read_version(struct udevice *blk, ulong offset, char *version,
		     int max_size)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, buf, MMC_MAX_BLOCK_LEN);

	/* we can use an assert() here since we already read only one block */
	assert(max_size <= MMC_MAX_BLOCK_LEN);

	/*
	 * we can use an assert() here since reading the wrong block will just
	 * cause an invalid version-string to be (safely) read
	 */
	assert(!(offset & (MMC_MAX_BLOCK_LEN - 1)));

	offset /= MMC_MAX_BLOCK_LEN;

	if (blk_read(blk, offset, 1, buf) != 1)
		return log_msg_ret("read", -EIO);
	strlcpy(version, buf, max_size);
	log_debug("version=%s\n", version);

	return 0;
}

int vbe_read_nvdata(struct udevice *blk, ulong offset, ulong size, u8 *buf)
{
	uint hdr_ver, hdr_size, data_size, crc;
	const struct vbe_nvdata *nvd;

	/* we can use an assert() here since we already read only one block */
	assert(size <= MMC_MAX_BLOCK_LEN);

	/*
	 * We can use an assert() here since reading the wrong block will just
	 * cause invalid state to be (safely) read. If the crc passes, then we
	 * obtain invalid state and it will likely cause booting to fail.
	 *
	 * VBE relies on valid values being in U-Boot's devicetree, so this
	 * should not every be wrong on a production device.
	 */
	assert(!(offset & (MMC_MAX_BLOCK_LEN - 1)));

	if (offset & (MMC_MAX_BLOCK_LEN - 1))
		return log_msg_ret("get", -EBADF);
	offset /= MMC_MAX_BLOCK_LEN;

	if (blk_read(blk, offset, 1, buf) != 1)
		return log_msg_ret("read", -EIO);
	nvd = (struct vbe_nvdata *)buf;
	hdr_ver = (nvd->hdr & NVD_HDR_VER_MASK) >> NVD_HDR_VER_SHIFT;
	hdr_size = (nvd->hdr & NVD_HDR_SIZE_MASK) >> NVD_HDR_SIZE_SHIFT;
	if (hdr_ver != NVD_HDR_VER_CUR)
		return log_msg_ret("hdr", -EPERM);
	data_size = 1 << hdr_size;
	if (!data_size || data_size > sizeof(*nvd))
		return log_msg_ret("sz", -EPERM);

	crc = crc8(0, buf + 1, data_size - 1);
	if (crc != nvd->crc8)
		return log_msg_ret("crc", -EPERM);

	return 0;
}

int vbe_read_fit(struct udevice *blk, ulong area_offset, ulong area_size,
		 ulong *load_addrp, ulong *lenp, char **namep)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, sbuf, MMC_MAX_BLOCK_LEN);
	ulong size, blknum, addr, len, load_addr, num_blks;
	const char *fit_uname, *fit_uname_config;
	struct bootm_headers images = {};
	enum image_phase_t phase;
	struct blk_desc *desc;
	int node, ret;
	void *buf;

	desc = dev_get_uclass_plat(blk);

	/* read in one block to find the FIT size */
	blknum =  area_offset / desc->blksz;
	log_debug("read at %lx, blknum %lx\n", area_offset, blknum);
	ret = blk_read(blk, blknum, 1, sbuf);
	if (ret < 0)
		return log_msg_ret("rd", ret);

	ret = fdt_check_header(sbuf);
	if (ret < 0)
		return log_msg_ret("fdt", -EINVAL);
	size = fdt_totalsize(sbuf);
	if (size > area_size)
		return log_msg_ret("fdt", -E2BIG);
	log_debug("FIT size %lx\n", size);

	/*
	 * Load the FIT into the SPL memory. This is typically a FIT with
	 * external data, so this is quite small, perhaps a few KB.
	 */
	addr = CONFIG_VAL(TEXT_BASE);
	buf = map_sysmem(addr, size);
	num_blks = DIV_ROUND_UP(size, desc->blksz);
	log_debug("read %lx, %lx blocks to %lx / %p\n", size, num_blks, addr,
		  buf);
	ret = blk_read(blk, blknum, num_blks, buf);
	if (ret < 0)
		return log_msg_ret("rd", ret);

	/* figure out the phase to load */
	phase = IS_ENABLED(CONFIG_VPL_BUILD) ? IH_PHASE_SPL : IH_PHASE_U_BOOT;

	/*
	 * Load the image from the FIT. We ignore any load-address information
	 * so in practice this simply locates the image in the external-data
	 * region and returns its address and size. Since we only loaded the FIT
	 * itself, only a part of the image will be present, at best.
	 */
	fit_uname = NULL;
	fit_uname_config = NULL;
	log_debug("loading FIT\n");
	ret = fit_image_load(&images, addr, &fit_uname, &fit_uname_config,
			     IH_ARCH_SANDBOX, image_ph(phase, IH_TYPE_FIRMWARE),
			     BOOTSTAGE_ID_FIT_SPL_START, FIT_LOAD_IGNORED,
			     &load_addr, &len);
	if (ret < 0)
		return log_msg_ret("ld", ret);
	node = ret;
	log_debug("loaded to %lx\n", load_addr);

	/* For FIT external data, read in the external data */
	if (load_addr + len > addr + size) {
		ulong base, full_size;
		void *base_buf;

		/* Find the start address to load from */
		base = ALIGN_DOWN(load_addr, desc->blksz);

		/*
		 * Get the total number of bytes to load, taking care of
		 * block alignment
		 */
		full_size = load_addr + len - base;

		/*
		 * Get the start block number, number of blocks and the address
		 * to load to, then load the blocks
		 */
		blknum = (area_offset + base - addr) / desc->blksz;
		num_blks = DIV_ROUND_UP(full_size, desc->blksz);
		base_buf = map_sysmem(base, full_size);
		ret = blk_read(blk, blknum, num_blks, base_buf);
		log_debug("read %lx %lx, %lx blocks to %lx / %p: ret=%d\n",
			  blknum, full_size, num_blks, base, base_buf, ret);
		if (ret < 0)
			return log_msg_ret("rd", ret);
	}
	if (load_addrp)
		*load_addrp = load_addr;
	if (lenp)
		*lenp = len;
	if (namep) {
		*namep = strdup(fdt_get_name(buf, node, NULL));
		if (!namep)
			return log_msg_ret("nam", -ENOMEM);
	}

	return 0;
}

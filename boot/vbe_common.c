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

binman_sym_declare(ulong, u_boot_vpl_nodtb, size);
binman_sym_declare(ulong, u_boot_vpl_bss_pad, size);
binman_sym_declare(ulong, u_boot_spl_nodtb, size);
binman_sym_declare(ulong, u_boot_spl_bss_pad, size);

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

/**
 * h_vbe_load_read() - Handler for reading an SPL image from a FIT
 *
 * See spl_load_reader for the definition
 */
ulong h_vbe_load_read(struct spl_load_info *load, ulong off, ulong size,
		      void *buf)
{
	struct blk_desc *desc = load->priv;
	lbaint_t sector = off >> desc->log2blksz;
	lbaint_t count = size >> desc->log2blksz;
	int ret;

	log_debug("vbe read log2blksz %x offset %lx sector %lx count %lx\n",
		  desc->log2blksz, (ulong)off, (long)sector, (ulong)count);

	ret = blk_dread(desc, sector, count, buf);
	log_debug("ret=%x\n", ret);
	if (ret < 0)
		return ret;

	return ret << desc->log2blksz;
}

int vbe_read_fit(struct udevice *blk, ulong area_offset, ulong area_size,
		 struct spl_image_info *image, ulong *load_addrp, ulong *lenp,
		 char **namep)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, sbuf, MMC_MAX_BLOCK_LEN);
	ulong size, blknum, addr, len, load_addr, num_blks, spl_load_addr;
	ulong aligned_size, fdt_load_addr, fdt_size;
	const char *fit_uname, *fit_uname_config;
	struct bootm_headers images = {};
	enum image_phase_t phase;
	struct blk_desc *desc;
	int node, ret;
	bool for_xpl;
	void *buf;

	desc = dev_get_uclass_plat(blk);

	/* read in one block to find the FIT size */
	blknum =  area_offset / desc->blksz;
	log_debug("read at %lx, blknum %lx\n", area_offset, blknum);
	ret = blk_read(blk, blknum, 1, sbuf);
	if (ret < 0)
		return log_msg_ret("rd", ret);
	else if (ret != 1)
		return log_msg_ret("rd2", -EIO);

	ret = fdt_check_header(sbuf);
	if (ret < 0)
		return log_msg_ret("fdt", -EINVAL);
	size = fdt_totalsize(sbuf);
	if (size > area_size)
		return log_msg_ret("fdt", -E2BIG);
	log_debug("FIT size %lx\n", size);
	aligned_size = ALIGN(size, desc->blksz);

	/*
	 * Load the FIT into the SPL memory. This is typically a FIT with
	 * external data, so this is quite small, perhaps a few KB.
	 */
	if (IS_ENABLED(CONFIG_SANDBOX)) {
		addr = CONFIG_VAL(TEXT_BASE);
		buf = map_sysmem(addr, size);
	} else {
		buf = malloc(aligned_size);
		if (!buf)
			return log_msg_ret("fit", -ENOMEM);
		addr = map_to_sysmem(buf);
	}
	num_blks = aligned_size / desc->blksz;
	log_debug("read %lx, %lx blocks to %lx / %p\n", aligned_size, num_blks,
		  addr, buf);
	ret = blk_read(blk, blknum, num_blks, buf);
	if (ret < 0)
		return log_msg_ret("rd3", ret);
	else if (ret != num_blks)
		return log_msg_ret("rd4", -EIO);
	log_debug("check total size %x off_dt_strings %x\n", fdt_totalsize(buf),
		  fdt_off_dt_strings(buf));

#if CONFIG_IS_ENABLED(SYS_MALLOC_F)
	log_debug("malloc base %lx ptr %x limit %x top %lx\n",
		  gd->malloc_base, gd->malloc_ptr, gd->malloc_limit,
		  gd->malloc_base + gd->malloc_limit);
#endif
	/* figure out the phase to load */
	phase = IS_ENABLED(CONFIG_TPL_BUILD) ? IH_PHASE_NONE :
		IS_ENABLED(CONFIG_VPL_BUILD) ? IH_PHASE_SPL : IH_PHASE_U_BOOT;

	log_debug("loading FIT\n");

	if (xpl_phase() == PHASE_SPL && !IS_ENABLED(CONFIG_SANDBOX)) {
		struct spl_load_info info;

		spl_load_init(&info, h_vbe_load_read, desc, desc->blksz);
		xpl_set_fdt_update(&info, false);
		xpl_set_phase(&info, IH_PHASE_U_BOOT);
		log_debug("doing SPL from %s blksz %lx log2blksz %x area_offset %lx + fdt_size %lx\n",
			  blk->name, desc->blksz, desc->log2blksz, area_offset, ALIGN(size, 4));
		ret = spl_load_simple_fit(image, &info, area_offset, buf);
		log_debug("spl_load_simple_fit() ret=%d\n", ret);

		return ret;
	}

	/*
	 * Load the image from the FIT. We ignore any load-address information
	 * so in practice this simply locates the image in the external-data
	 * region and returns its address and size. Since we only loaded the FIT
	 * itself, only a part of the image will be present, at best.
	 */
	fit_uname = NULL;
	fit_uname_config = NULL;
	ret = fit_image_load(&images, addr, &fit_uname, &fit_uname_config,
			     IH_ARCH_DEFAULT, image_ph(phase, IH_TYPE_FIRMWARE),
			     BOOTSTAGE_ID_FIT_SPL_START, FIT_LOAD_IGNORED,
			     &load_addr, &len);
	if (ret == -ENOENT) {
		ret = fit_image_load(&images, addr, &fit_uname,
				     &fit_uname_config, IH_ARCH_DEFAULT,
				     image_ph(phase, IH_TYPE_LOADABLE),
				     BOOTSTAGE_ID_FIT_SPL_START,
				     FIT_LOAD_IGNORED, &load_addr, &len);
	}
	if (ret < 0)
		return log_msg_ret("ld", ret);
	node = ret;
	log_debug("load %lx size %lx\n", load_addr, len);

	fdt_load_addr = 0;
	fdt_size = 0;
	if ((xpl_phase() == PHASE_TPL || xpl_phase() == PHASE_VPL) &&
	    !IS_ENABLED(CONFIG_SANDBOX)) {
		/* allow use of a different image from the configuration node */
		fit_uname = NULL;
		ret = fit_image_load(&images, addr, &fit_uname,
				     &fit_uname_config, IH_ARCH_DEFAULT,
				     image_ph(phase, IH_TYPE_FLATDT),
				     BOOTSTAGE_ID_FIT_SPL_START,
				     FIT_LOAD_IGNORED, &fdt_load_addr,
				     &fdt_size);
		fdt_size = ALIGN(fdt_size, desc->blksz);
		log_debug("FDT noload to %lx size %lx\n", fdt_load_addr,
			  fdt_size);
	}

	for_xpl = !USE_BOOTMETH && CONFIG_IS_ENABLED(RELOC_LOADER);
	if (for_xpl) {
		image->size = len;
		image->fdt_size = fdt_size;
		ret = spl_reloc_prepare(image, &spl_load_addr);
		if (ret)
			return log_msg_ret("spl", ret);
	}
	if (!IS_ENABLED(CONFIG_SANDBOX))
		image->os = IH_OS_U_BOOT;

	/* For FIT external data, read in the external data */
	log_debug("load_addr %lx len %lx addr %lx aligned_size %lx\n",
		  load_addr, len, addr, aligned_size);
	if (load_addr + len > addr + aligned_size) {
		ulong base, full_size, offset, extra, fdt_base, fdt_full_size;
		ulong fdt_offset;
		void *base_buf, *fdt_base_buf;

		/* Find the start address to load from */
		base = ALIGN_DOWN(load_addr, desc->blksz);

		offset = area_offset + load_addr - addr;
		blknum = offset / desc->blksz;
		extra = offset % desc->blksz;

		/*
		 * Get the total number of bytes to load, taking care of
		 * block alignment
		 */
		full_size = len + extra;

		/*
		 * Get the start block number, number of blocks and the address
		 * to load to, then load the blocks
		 */
		num_blks = DIV_ROUND_UP(full_size, desc->blksz);
		if (for_xpl)
			base = spl_load_addr;
		base_buf = map_sysmem(base, full_size);
		ret = blk_read(blk, blknum, num_blks, base_buf);
		log_debug("read foffset %lx blknum %lx full_size %lx num_blks %lx to %lx / %p: ret=%d\n",
			  offset - 0x8000, blknum, full_size, num_blks, base, base_buf,
			  ret);
		if (ret < 0)
			return log_msg_ret("rd", ret);
		if (ret != num_blks)
			return log_msg_ret("rd", -EIO);
		if (extra && !IS_ENABLED(CONFIG_SANDBOX)) {
			log_debug("move %p %p %lx\n", base_buf,
				  base_buf + extra, len);
			memmove(base_buf, base_buf + extra, len);
		}

		if ((xpl_phase() == PHASE_VPL || xpl_phase() == PHASE_TPL) &&
		    !IS_ENABLED(CONFIG_SANDBOX)) {
			image->load_addr = spl_get_image_text_base();
			image->entry_point = image->load_addr;
		}

		/* now the FDT */
		if (fdt_size) {
			fdt_offset = area_offset + fdt_load_addr - addr;
			blknum = fdt_offset / desc->blksz;
			extra = fdt_offset % desc->blksz;
			fdt_full_size = fdt_size + extra;
			num_blks = DIV_ROUND_UP(fdt_full_size, desc->blksz);
			fdt_base = ALIGN(base + len, 4);
			fdt_base_buf = map_sysmem(fdt_base, fdt_size);
			ret = blk_read(blk, blknum, num_blks, fdt_base_buf);
			log_debug("fdt read foffset %lx blknum %lx full_size %lx num_blks %lx to %lx / %p: ret=%d\n",
				  fdt_offset - 0x8000, blknum, fdt_full_size, num_blks,
				  fdt_base, fdt_base_buf, ret);
			if (ret != num_blks)
				return log_msg_ret("rdf", -EIO);
			if (extra) {
				log_debug("move %p %p %lx\n", fdt_base_buf,
					  fdt_base_buf + extra, fdt_size);
				memmove(fdt_base_buf, fdt_base_buf + extra,
					fdt_size);
			}
#if CONFIG_IS_ENABLED(RELOC_LOADER)
			image->fdt_buf = fdt_base_buf;

			ulong xpl_size;
			ulong xpl_pad;
			ulong fdt_start;

			if (xpl_phase() == PHASE_TPL) {
				xpl_size = binman_sym(ulong, u_boot_vpl_nodtb, size);
				xpl_pad = binman_sym(ulong, u_boot_vpl_bss_pad, size);
			} else {
				xpl_size = binman_sym(ulong, u_boot_spl_nodtb, size);
				xpl_pad = binman_sym(ulong, u_boot_spl_bss_pad, size);
			}
			fdt_start = image->load_addr + xpl_size + xpl_pad;
			log_debug("load_addr %lx xpl_size %lx copy-to %lx\n",
				  image->load_addr, xpl_size + xpl_pad,
				  fdt_start);
			image->fdt_start = map_sysmem(fdt_start, fdt_size);
#endif
		}
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

ofnode vbe_get_node(void)
{
	return ofnode_path("/bootstd/firmware0");
}

/*
 * drivers/mtd/nand/nand_util.c
 *
 * Copyright (C) 2006 by Weiss-Electronic GmbH.
 * All rights reserved.
 *
 * @author:	Guido Classen <clagix@gmail.com>
 * @descr:	NAND Flash support
 * @references: borrowed heavily from Linux mtd-utils code:
 *		flash_eraseall.c by Arcom Control System Ltd
 *		nandwrite.c by Steven J. Hill (sjhill@realitydiluted.com)
 *			       and Thomas Gleixner (tglx@linutronix.de)
 *
 * Copyright (C) 2008 Nokia Corporation: drop_ffs() function by
 * Artem Bityutskiy <dedekind1@gmail.com> from mtd-utils
 *
 * Copyright 2010 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <memalign.h>
#include <div64.h>

#include <asm/errno.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <jffs2/jffs2.h>

typedef struct erase_info	erase_info_t;
typedef struct mtd_info		mtd_info_t;

/* support only for native endian JFFS2 */
#define cpu_to_je16(x) (x)
#define cpu_to_je32(x) (x)

/**
 * nand_erase_opts: - erase NAND flash with support for various options
 *		      (jffs2 formatting)
 *
 * @param meminfo	NAND device to erase
 * @param opts		options,  @see struct nand_erase_options
 * @return		0 in case of success
 *
 * This code is ported from flash_eraseall.c from Linux mtd utils by
 * Arcom Control System Ltd.
 */
int nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts)
{
	struct jffs2_unknown_node cleanmarker;
	erase_info_t erase;
	unsigned long erase_length, erased_length; /* in blocks */
	int result;
	int percent_complete = -1;
	const char *mtd_device = meminfo->name;
	struct mtd_oob_ops oob_opts;
	struct nand_chip *chip = meminfo->priv;

	if ((opts->offset & (meminfo->erasesize - 1)) != 0) {
		printf("Attempt to erase non block-aligned data\n");
		return -1;
	}

	memset(&erase, 0, sizeof(erase));
	memset(&oob_opts, 0, sizeof(oob_opts));

	erase.mtd = meminfo;
	erase.len  = meminfo->erasesize;
	erase.addr = opts->offset;
	erase_length = lldiv(opts->length + meminfo->erasesize - 1,
			     meminfo->erasesize);

	cleanmarker.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	cleanmarker.nodetype = cpu_to_je16(JFFS2_NODETYPE_CLEANMARKER);
	cleanmarker.totlen = cpu_to_je32(8);

	/* scrub option allows to erase badblock. To prevent internal
	 * check from erase() method, set block check method to dummy
	 * and disable bad block table while erasing.
	 */
	if (opts->scrub) {
		erase.scrub = opts->scrub;
		/*
		 * We don't need the bad block table anymore...
		 * after scrub, there are no bad blocks left!
		 */
		if (chip->bbt) {
			kfree(chip->bbt);
		}
		chip->bbt = NULL;
		chip->options &= ~NAND_BBT_SCANNED;
	}

	for (erased_length = 0;
	     erased_length < erase_length;
	     erase.addr += meminfo->erasesize) {

		WATCHDOG_RESET();

		if (opts->lim && (erase.addr >= (opts->offset + opts->lim))) {
			puts("Size of erase exceeds limit\n");
			return -EFBIG;
		}
		if (!opts->scrub) {
			int ret = mtd_block_isbad(meminfo, erase.addr);
			if (ret > 0) {
				if (!opts->quiet)
					printf("\rSkipping bad block at  "
					       "0x%08llx                 "
					       "                         \n",
					       erase.addr);

				if (!opts->spread)
					erased_length++;

				continue;

			} else if (ret < 0) {
				printf("\n%s: MTD get bad block failed: %d\n",
				       mtd_device,
				       ret);
				return -1;
			}
		}

		erased_length++;

		result = mtd_erase(meminfo, &erase);
		if (result != 0) {
			printf("\n%s: MTD Erase failure: %d\n",
			       mtd_device, result);
			continue;
		}

		/* format for JFFS2 ? */
		if (opts->jffs2 && chip->ecc.layout->oobavail >= 8) {
			struct mtd_oob_ops ops;
			ops.ooblen = 8;
			ops.datbuf = NULL;
			ops.oobbuf = (uint8_t *)&cleanmarker;
			ops.ooboffs = 0;
			ops.mode = MTD_OPS_AUTO_OOB;

			result = mtd_write_oob(meminfo,
						    erase.addr,
						    &ops);
			if (result != 0) {
				printf("\n%s: MTD writeoob failure: %d\n",
				       mtd_device, result);
				continue;
			}
		}

		if (!opts->quiet) {
			unsigned long long n = erased_length * 100ULL;
			int percent;

			do_div(n, erase_length);
			percent = (int)n;

			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
				percent_complete = percent;

				printf("\rErasing at 0x%llx -- %3d%% complete.",
				       erase.addr, percent);

				if (opts->jffs2 && result == 0)
					printf(" Cleanmarker written at 0x%llx.",
					       erase.addr);
			}
		}
	}
	if (!opts->quiet)
		printf("\n");

	return 0;
}

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK

#define NAND_CMD_LOCK_TIGHT     0x2c
#define NAND_CMD_LOCK_STATUS    0x7a
 
/******************************************************************************
 * Support for locking / unlocking operations of some NAND devices
 *****************************************************************************/

/**
 * nand_lock: Set all pages of NAND flash chip to the LOCK or LOCK-TIGHT
 *	      state
 *
 * @param mtd		nand mtd instance
 * @param tight		bring device in lock tight mode
 *
 * @return		0 on success, -1 in case of error
 *
 * The lock / lock-tight command only applies to the whole chip. To get some
 * parts of the chip lock and others unlocked use the following sequence:
 *
 * - Lock all pages of the chip using nand_lock(mtd, 0) (or the lockpre pin)
 * - Call nand_unlock() once for each consecutive area to be unlocked
 * - If desired: Bring the chip to the lock-tight state using nand_lock(mtd, 1)
 *
 *   If the device is in lock-tight state software can't change the
 *   current active lock/unlock state of all pages. nand_lock() / nand_unlock()
 *   calls will fail. It is only posible to leave lock-tight state by
 *   an hardware signal (low pulse on _WP pin) or by power down.
 */
int nand_lock(struct mtd_info *mtd, int tight)
{
	int ret = 0;
	int status;
	struct nand_chip *chip = mtd->priv;

	/* select the NAND device */
	chip->select_chip(mtd, 0);

	/* check the Lock Tight Status */
	chip->cmdfunc(mtd, NAND_CMD_LOCK_STATUS, -1, 0);
	if (chip->read_byte(mtd) & NAND_LOCK_STATUS_TIGHT) {
		printf("nand_lock: Device is locked tight!\n");
		ret = -1;
		goto out;
	}

	chip->cmdfunc(mtd,
		      (tight ? NAND_CMD_LOCK_TIGHT : NAND_CMD_LOCK),
		      -1, -1);

	/* call wait ready function */
	status = chip->waitfunc(mtd, chip);

	/* see if device thinks it succeeded */
	if (status & 0x01) {
		ret = -1;
	}

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}

/**
 * nand_get_lock_status: - query current lock state from one page of NAND
 *			   flash
 *
 * @param mtd		nand mtd instance
 * @param offset	page address to query (must be page-aligned!)
 *
 * @return		-1 in case of error
 *			>0 lock status:
 *			  bitfield with the following combinations:
 *			  NAND_LOCK_STATUS_TIGHT: page in tight state
 *			  NAND_LOCK_STATUS_UNLOCK: page unlocked
 *
 */
int nand_get_lock_status(struct mtd_info *mtd, loff_t offset)
{
	int ret = 0;
	int chipnr;
	int page;
	struct nand_chip *chip = mtd->priv;

	/* select the NAND device */
	chipnr = (int)(offset >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);


	if ((offset & (mtd->writesize - 1)) != 0) {
		printf("nand_get_lock_status: "
			"Start address must be beginning of "
			"nand page!\n");
		ret = -1;
		goto out;
	}

	/* check the Lock Status */
	page = (int)(offset >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_LOCK_STATUS, -1, page & chip->pagemask);

	ret = chip->read_byte(mtd) & (NAND_LOCK_STATUS_TIGHT
					  | NAND_LOCK_STATUS_UNLOCK);

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}

/**
 * nand_unlock: - Unlock area of NAND pages
 *		  only one consecutive area can be unlocked at one time!
 *
 * @param mtd		nand mtd instance
 * @param start		start byte address
 * @param length	number of bytes to unlock (must be a multiple of
 *			page size nand->writesize)
 * @param allexcept	if set, unlock everything not selected
 *
 * @return		0 on success, -1 in case of error
 */
int nand_unlock(struct mtd_info *mtd, loff_t start, size_t length,
	int allexcept)
{
	int ret = 0;
	int chipnr;
	int status;
	int page;
	struct nand_chip *chip = mtd->priv;

	debug("nand_unlock%s: start: %08llx, length: %zd!\n",
		allexcept ? " (allexcept)" : "", start, length);

	/* select the NAND device */
	chipnr = (int)(start >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	/* check the WP bit */
	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	if (!(chip->read_byte(mtd) & NAND_STATUS_WP)) {
		printf("nand_unlock: Device is write protected!\n");
		ret = -1;
		goto out;
	}

	/* check the Lock Tight Status */
	page = (int)(start >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_LOCK_STATUS, -1, page & chip->pagemask);
	if (chip->read_byte(mtd) & NAND_LOCK_STATUS_TIGHT) {
		printf("nand_unlock: Device is locked tight!\n");
		ret = -1;
		goto out;
	}

	if ((start & (mtd->erasesize - 1)) != 0) {
		printf("nand_unlock: Start address must be beginning of "
			"nand block!\n");
		ret = -1;
		goto out;
	}

	if (length == 0 || (length & (mtd->erasesize - 1)) != 0) {
		printf("nand_unlock: Length must be a multiple of nand block "
			"size %08x!\n", mtd->erasesize);
		ret = -1;
		goto out;
	}

	/*
	 * Set length so that the last address is set to the
	 * starting address of the last block
	 */
	length -= mtd->erasesize;

	/* submit address of first page to unlock */
	chip->cmdfunc(mtd, NAND_CMD_UNLOCK1, -1, page & chip->pagemask);

	/* submit ADDRESS of LAST page to unlock */
	page += (int)(length >> chip->page_shift);

	/*
	 * Page addresses for unlocking are supposed to be block-aligned.
	 * At least some NAND chips use the low bit to indicate that the
	 * page range should be inverted.
	 */
	if (allexcept)
		page |= 1;

	chip->cmdfunc(mtd, NAND_CMD_UNLOCK2, -1, page & chip->pagemask);

	/* call wait ready function */
	status = chip->waitfunc(mtd, chip);
	/* see if device thinks it succeeded */
	if (status & 0x01) {
		/* there was an error */
		ret = -1;
		goto out;
	}

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}
#endif

/**
 * check_skip_len
 *
 * Check if there are any bad blocks, and whether length including bad
 * blocks fits into device
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length image length
 * @param used length of flash needed for the requested length
 * @return 0 if the image fits and there are no bad blocks
 *         1 if the image fits, but there are bad blocks
 *        -1 if the image does not fit
 */
static int check_skip_len(nand_info_t *nand, loff_t offset, size_t length,
		size_t *used)
{
	size_t len_excl_bad = 0;
	int ret = 0;

	while (len_excl_bad < length) {
		size_t block_len, block_off;
		loff_t block_start;

		if (offset >= nand->size)
			return -1;

		block_start = offset & ~(loff_t)(nand->erasesize - 1);
		block_off = offset & (nand->erasesize - 1);
		block_len = nand->erasesize - block_off;

		if (!nand_block_isbad(nand, block_start))
			len_excl_bad += block_len;
		else
			ret = 1;

		offset += block_len;
		*used += block_len;
	}

	/* If the length is not a multiple of block_len, adjust. */
	if (len_excl_bad > length)
		*used -= (len_excl_bad - length);

	return ret;
}

#ifdef CONFIG_CMD_NAND_TRIMFFS
static size_t drop_ffs(const nand_info_t *nand, const u_char *buf,
			const size_t *len)
{
	size_t l = *len;
	ssize_t i;

	for (i = l - 1; i >= 0; i--)
		if (buf[i] != 0xFF)
			break;

	/* The resulting length must be aligned to the minimum flash I/O size */
	l = i + 1;
	l = (l + nand->writesize - 1) / nand->writesize;
	l *=  nand->writesize;

	/*
	 * since the input length may be unaligned, prevent access past the end
	 * of the buffer
	 */
	return min(l, *len);
}
#endif

/**
 * nand_verify_page_oob:
 *
 * Verify a page of NAND flash, including the OOB.
 * Reads page of NAND and verifies the contents and OOB against the
 * values in ops.
 *
 * @param nand		NAND device
 * @param ops		MTD operations, including data to verify
 * @param ofs		offset in flash
 * @return		0 in case of success
 */
int nand_verify_page_oob(nand_info_t *nand, struct mtd_oob_ops *ops, loff_t ofs)
{
	int rval;
	struct mtd_oob_ops vops;
	size_t verlen = nand->writesize + nand->oobsize;

	memcpy(&vops, ops, sizeof(vops));

	vops.datbuf = memalign(ARCH_DMA_MINALIGN, verlen);

	if (!vops.datbuf)
		return -ENOMEM;

	vops.oobbuf = vops.datbuf + nand->writesize;

	rval = mtd_read_oob(nand, ofs, &vops);
	if (!rval)
		rval = memcmp(ops->datbuf, vops.datbuf, vops.len);
	if (!rval)
		rval = memcmp(ops->oobbuf, vops.oobbuf, vops.ooblen);

	free(vops.datbuf);

	return rval ? -EIO : 0;
}

/**
 * nand_verify:
 *
 * Verify a region of NAND flash.
 * Reads NAND in page-sized chunks and verifies the contents against
 * the contents of a buffer.  The offset into the NAND must be
 * page-aligned, and the function doesn't handle skipping bad blocks.
 *
 * @param nand		NAND device
 * @param ofs		offset in flash
 * @param len		buffer length
 * @param buf		buffer to read from
 * @return		0 in case of success
 */
int nand_verify(nand_info_t *nand, loff_t ofs, size_t len, u_char *buf)
{
	int rval = 0;
	size_t verofs;
	size_t verlen = nand->writesize;
	uint8_t *verbuf = memalign(ARCH_DMA_MINALIGN, verlen);

	if (!verbuf)
		return -ENOMEM;

	/* Read the NAND back in page-size groups to limit malloc size */
	for (verofs = ofs; verofs < ofs + len;
	     verofs += verlen, buf += verlen) {
		verlen = min(nand->writesize, (uint32_t)(ofs + len - verofs));
		rval = nand_read(nand, verofs, &verlen, verbuf);
		if (!rval || (rval == -EUCLEAN))
			rval = memcmp(buf, verbuf, verlen);

		if (rval)
			break;
	}

	free(verbuf);

	return rval ? -EIO : 0;
}



/**
 * nand_write_skip_bad:
 *
 * Write image to NAND flash.
 * Blocks that are marked bad are skipped and the is written to the next
 * block instead as long as the image is short enough to fit even after
 * skipping the bad blocks.  Due to bad blocks we may not be able to
 * perform the requested write.  In the case where the write would
 * extend beyond the end of the NAND device, both length and actual (if
 * not NULL) are set to 0.  In the case where the write would extend
 * beyond the limit we are passed, length is set to 0 and actual is set
 * to the required length.
 *
 * @param nand  	NAND device
 * @param offset	offset in flash
 * @param length	buffer length
 * @param actual	set to size required to write length worth of
 *			buffer or 0 on error, if not NULL
 * @param lim		maximum size that actual may be in order to not
 *			exceed the buffer
 * @param buffer        buffer to read from
 * @param flags		flags modifying the behaviour of the write to NAND
 * @return		0 in case of success
 */
int nand_write_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
		size_t *actual, loff_t lim, u_char *buffer, int flags)
{
	int rval = 0, blocksize;
	size_t left_to_write = *length;
	size_t used_for_write = 0;
	u_char *p_buffer = buffer;
	int need_skip;

	if (actual)
		*actual = 0;

	blocksize = nand->erasesize;

	/*
	 * nand_write() handles unaligned, partial page writes.
	 *
	 * We allow length to be unaligned, for convenience in
	 * using the $filesize variable.
	 *
	 * However, starting at an unaligned offset makes the
	 * semantics of bad block skipping ambiguous (really,
	 * you should only start a block skipping access at a
	 * partition boundary).  So don't try to handle that.
	 */
	if ((offset & (nand->writesize - 1)) != 0) {
		printf("Attempt to write non page-aligned data\n");
		*length = 0;
		return -EINVAL;
	}

	need_skip = check_skip_len(nand, offset, *length, &used_for_write);

	if (actual)
		*actual = used_for_write;

	if (need_skip < 0) {
		printf("Attempt to write outside the flash area\n");
		*length = 0;
		return -EINVAL;
	}

	if (used_for_write > lim) {
		puts("Size of write exceeds partition or device limit\n");
		*length = 0;
		return -EFBIG;
	}

	if (!need_skip && !(flags & WITH_DROP_FFS)) {
		rval = nand_write(nand, offset, length, buffer);

		if ((flags & WITH_WR_VERIFY) && !rval)
			rval = nand_verify(nand, offset, *length, buffer);

		if (rval == 0)
			return 0;

		*length = 0;
		printf("NAND write to offset %llx failed %d\n",
			offset, rval);
		return rval;
	}

	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size, truncated_write_size;

		WATCHDOG_RESET();

		if (nand_block_isbad(nand, offset & ~(nand->erasesize - 1))) {
			printf("Skip bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_write < (blocksize - block_offset))
			write_size = left_to_write;
		else
			write_size = blocksize - block_offset;

		truncated_write_size = write_size;
#ifdef CONFIG_CMD_NAND_TRIMFFS
		if (flags & WITH_DROP_FFS)
			truncated_write_size = drop_ffs(nand, p_buffer,
					&write_size);
#endif

		rval = nand_write(nand, offset, &truncated_write_size,
				p_buffer);

		if ((flags & WITH_WR_VERIFY) && !rval)
			rval = nand_verify(nand, offset,
				truncated_write_size, p_buffer);

		offset += write_size;
		p_buffer += write_size;

		if (rval != 0) {
			printf("NAND write to offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_write;
			return rval;
		}

		left_to_write -= write_size;
	}

	return 0;
}

/**
 * nand_read_skip_bad:
 *
 * Read image from NAND flash.
 * Blocks that are marked bad are skipped and the next block is read
 * instead as long as the image is short enough to fit even after
 * skipping the bad blocks.  Due to bad blocks we may not be able to
 * perform the requested read.  In the case where the read would extend
 * beyond the end of the NAND device, both length and actual (if not
 * NULL) are set to 0.  In the case where the read would extend beyond
 * the limit we are passed, length is set to 0 and actual is set to the
 * required length.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length buffer length, on return holds number of read bytes
 * @param actual set to size required to read length worth of buffer or 0
 * on error, if not NULL
 * @param lim maximum size that actual may be in order to not exceed the
 * buffer
 * @param buffer buffer to write to
 * @return 0 in case of success
 */
int nand_read_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
		size_t *actual, loff_t lim, u_char *buffer)
{
	int rval;
	size_t left_to_read = *length;
	size_t used_for_read = 0;
	u_char *p_buffer = buffer;
	int need_skip;

	if ((offset & (nand->writesize - 1)) != 0) {
		printf("Attempt to read non page-aligned data\n");
		*length = 0;
		if (actual)
			*actual = 0;
		return -EINVAL;
	}

	need_skip = check_skip_len(nand, offset, *length, &used_for_read);

	if (actual)
		*actual = used_for_read;

	if (need_skip < 0) {
		printf("Attempt to read outside the flash area\n");
		*length = 0;
		return -EINVAL;
	}

	if (used_for_read > lim) {
		puts("Size of read exceeds partition or device limit\n");
		*length = 0;
		return -EFBIG;
	}

	if (!need_skip) {
		rval = nand_read(nand, offset, length, buffer);
		if (!rval || rval == -EUCLEAN)
			return 0;

		*length = 0;
		printf("NAND read from offset %llx failed %d\n",
			offset, rval);
		return rval;
	}

	while (left_to_read > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t read_length;

		WATCHDOG_RESET();

		if (nand_block_isbad(nand, offset & ~(nand->erasesize - 1))) {
			printf("Skipping bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_read < (nand->erasesize - block_offset))
			read_length = left_to_read;
		else
			read_length = nand->erasesize - block_offset;

		rval = nand_read(nand, offset, &read_length, p_buffer);
		if (rval && rval != -EUCLEAN) {
			printf("NAND read from offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_read;
			return rval;
		}

		left_to_read -= read_length;
		offset       += read_length;
		p_buffer     += read_length;
	}

	return 0;
}

#ifdef CONFIG_CMD_NAND_TORTURE

/**
 * check_pattern:
 *
 * Check if buffer contains only a certain byte pattern.
 *
 * @param buf buffer to check
 * @param patt the pattern to check
 * @param size buffer size in bytes
 * @return 1 if there are only patt bytes in buf
 *         0 if something else was found
 */
static int check_pattern(const u_char *buf, u_char patt, int size)
{
	int i;

	for (i = 0; i < size; i++)
		if (buf[i] != patt)
			return 0;
	return 1;
}

/**
 * nand_torture:
 *
 * Torture a block of NAND flash.
 * This is useful to determine if a block that caused a write error is still
 * good or should be marked as bad.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @return 0 if the block is still good
 */
int nand_torture(nand_info_t *nand, loff_t offset)
{
	u_char patterns[] = {0xa5, 0x5a, 0x00};
	struct erase_info instr = {
		.mtd = nand,
		.addr = offset,
		.len = nand->erasesize,
	};
	size_t retlen;
	int err, ret = -1, i, patt_count;
	u_char *buf;

	if ((offset & (nand->erasesize - 1)) != 0) {
		puts("Attempt to torture a block at a non block-aligned offset\n");
		return -EINVAL;
	}

	if (offset + nand->erasesize > nand->size) {
		puts("Attempt to torture a block outside the flash area\n");
		return -EINVAL;
	}

	patt_count = ARRAY_SIZE(patterns);

	buf = malloc_cache_aligned(nand->erasesize);
	if (buf == NULL) {
		puts("Out of memory for erase block buffer\n");
		return -ENOMEM;
	}

	for (i = 0; i < patt_count; i++) {
		err = nand->erase(nand, &instr);
		if (err) {
			printf("%s: erase() failed for block at 0x%llx: %d\n",
				nand->name, instr.addr, err);
			goto out;
		}

		/* Make sure the block contains only 0xff bytes */
		err = nand->read(nand, offset, nand->erasesize, &retlen, buf);
		if ((err && err != -EUCLEAN) || retlen != nand->erasesize) {
			printf("%s: read() failed for block at 0x%llx: %d\n",
				nand->name, instr.addr, err);
			goto out;
		}

		err = check_pattern(buf, 0xff, nand->erasesize);
		if (!err) {
			printf("Erased block at 0x%llx, but a non-0xff byte was found\n",
				offset);
			ret = -EIO;
			goto out;
		}

		/* Write a pattern and check it */
		memset(buf, patterns[i], nand->erasesize);
		err = nand->write(nand, offset, nand->erasesize, &retlen, buf);
		if (err || retlen != nand->erasesize) {
			printf("%s: write() failed for block at 0x%llx: %d\n",
				nand->name, instr.addr, err);
			goto out;
		}

		err = nand->read(nand, offset, nand->erasesize, &retlen, buf);
		if ((err && err != -EUCLEAN) || retlen != nand->erasesize) {
			printf("%s: read() failed for block at 0x%llx: %d\n",
				nand->name, instr.addr, err);
			goto out;
		}

		err = check_pattern(buf, patterns[i], nand->erasesize);
		if (!err) {
			printf("Pattern 0x%.2x checking failed for block at "
					"0x%llx\n", patterns[i], offset);
			ret = -EIO;
			goto out;
		}
	}

	ret = 0;

out:
	free(buf);
	return ret;
}

#endif

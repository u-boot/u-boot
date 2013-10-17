/*
 * Core registration and callback routines for MTD
 * drivers and users.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/compat.h>
#include <ubi_uboot.h>

struct mtd_info *mtd_table[MAX_MTD_DEVICES];

int add_mtd_device(struct mtd_info *mtd)
{
	int i;

	BUG_ON(mtd->writesize == 0);

	for (i = 0; i < MAX_MTD_DEVICES; i++)
		if (!mtd_table[i]) {
			mtd_table[i] = mtd;
			mtd->index = i;
			mtd->usecount = 0;

			/* default value if not set by driver */
			if (mtd->bitflip_threshold == 0)
				mtd->bitflip_threshold = mtd->ecc_strength;


			/* No need to get a refcount on the module containing
			   the notifier, since we hold the mtd_table_mutex */

			/* We _know_ we aren't being removed, because
			   our caller is still holding us here. So none
			   of this try_ nonsense, and no bitching about it
			   either. :) */
			return 0;
		}

	return 1;
}

/**
 *      del_mtd_device - unregister an MTD device
 *      @mtd: pointer to MTD device info structure
 *
 *      Remove a device from the list of MTD devices present in the system,
 *      and notify each currently active MTD 'user' of its departure.
 *      Returns zero on success or 1 on failure, which currently will happen
 *      if the requested device does not appear to be present in the list.
 */
int del_mtd_device(struct mtd_info *mtd)
{
	int ret;

	if (mtd_table[mtd->index] != mtd) {
		ret = -ENODEV;
	} else if (mtd->usecount) {
		printk(KERN_NOTICE "Removing MTD device #%d (%s)"
				" with use count %d\n",
				mtd->index, mtd->name, mtd->usecount);
		ret = -EBUSY;
	} else {
		/* No need to get a refcount on the module containing
		 * the notifier, since we hold the mtd_table_mutex */
		mtd_table[mtd->index] = NULL;

		ret = 0;
	}

	return ret;
}

/**
 *	get_mtd_device - obtain a validated handle for an MTD device
 *	@mtd: last known address of the required MTD device
 *	@num: internal device number of the required MTD device
 *
 *	Given a number and NULL address, return the num'th entry in the device
 *      table, if any.  Given an address and num == -1, search the device table
 *      for a device with that address and return if it's still present. Given
 *      both, return the num'th driver only if its address matches. Return
 *      error code if not.
 */
struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num)
{
	struct mtd_info *ret = NULL;
	int i, err = -ENODEV;

	if (num == -1) {
		for (i = 0; i < MAX_MTD_DEVICES; i++)
			if (mtd_table[i] == mtd)
				ret = mtd_table[i];
	} else if (num < MAX_MTD_DEVICES) {
		ret = mtd_table[num];
		if (mtd && mtd != ret)
			ret = NULL;
	}

	if (!ret)
		goto out_unlock;

	ret->usecount++;
	return ret;

out_unlock:
	return ERR_PTR(err);
}

/**
 *      get_mtd_device_nm - obtain a validated handle for an MTD device by
 *      device name
 *      @name: MTD device name to open
 *
 *      This function returns MTD device description structure in case of
 *      success and an error code in case of failure.
 */
struct mtd_info *get_mtd_device_nm(const char *name)
{
	int i, err = -ENODEV;
	struct mtd_info *mtd = NULL;

	for (i = 0; i < MAX_MTD_DEVICES; i++) {
		if (mtd_table[i] && !strcmp(name, mtd_table[i]->name)) {
			mtd = mtd_table[i];
			break;
		}
	}

	if (!mtd)
		goto out_unlock;

	mtd->usecount++;
	return mtd;

out_unlock:
	return ERR_PTR(err);
}

void put_mtd_device(struct mtd_info *mtd)
{
	int c;

	c = --mtd->usecount;
	BUG_ON(c < 0);
}

#if defined(CONFIG_CMD_MTDPARTS_SPREAD)
/**
 * mtd_get_len_incl_bad
 *
 * Check if length including bad blocks fits into device.
 *
 * @param mtd an MTD device
 * @param offset offset in flash
 * @param length image length
 * @return image length including bad blocks in *len_incl_bad and whether or not
 *         the length returned was truncated in *truncated
 */
void mtd_get_len_incl_bad(struct mtd_info *mtd, uint64_t offset,
			  const uint64_t length, uint64_t *len_incl_bad,
			  int *truncated)
{
	*truncated = 0;
	*len_incl_bad = 0;

	if (!mtd->block_isbad) {
		*len_incl_bad = length;
		return;
	}

	uint64_t len_excl_bad = 0;
	uint64_t block_len;

	while (len_excl_bad < length) {
		if (offset >= mtd->size) {
			*truncated = 1;
			return;
		}

		block_len = mtd->erasesize - (offset & (mtd->erasesize - 1));

		if (!mtd->block_isbad(mtd, offset & ~(mtd->erasesize - 1)))
			len_excl_bad += block_len;

		*len_incl_bad += block_len;
		offset       += block_len;
	}
}
#endif /* defined(CONFIG_CMD_MTDPARTS_SPREAD) */

 /*
 * Erase is an asynchronous operation.  Device drivers are supposed
 * to call instr->callback() whenever the operation completes, even
 * if it completes with a failure.
 * Callers are supposed to pass a callback function and wait for it
 * to be called before writing to the block.
 */
int mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	if (instr->addr > mtd->size || instr->len > mtd->size - instr->addr)
		return -EINVAL;
	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;
	if (!instr->len) {
		instr->state = MTD_ERASE_DONE;
		mtd_erase_callback(instr);
		return 0;
	}
	return mtd->_erase(mtd, instr);
}

int mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,
	     u_char *buf)
{
	int ret_code;
	if (from < 0 || from > mtd->size || len > mtd->size - from)
		return -EINVAL;
	if (!len)
		return 0;

	/*
	 * In the absence of an error, drivers return a non-negative integer
	 * representing the maximum number of bitflips that were corrected on
	 * any one ecc region (if applicable; zero otherwise).
	 */
	ret_code = mtd->_read(mtd, from, len, retlen, buf);
	if (unlikely(ret_code < 0))
		return ret_code;
	if (mtd->ecc_strength == 0)
		return 0;	/* device lacks ecc */
	return ret_code >= mtd->bitflip_threshold ? -EUCLEAN : 0;
}

int mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen,
	      const u_char *buf)
{
	*retlen = 0;
	if (to < 0 || to > mtd->size || len > mtd->size - to)
		return -EINVAL;
	if (!mtd->_write || !(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	if (!len)
		return 0;
	return mtd->_write(mtd, to, len, retlen, buf);
}

/*
 * In blackbox flight recorder like scenarios we want to make successful writes
 * in interrupt context. panic_write() is only intended to be called when its
 * known the kernel is about to panic and we need the write to succeed. Since
 * the kernel is not going to be running for much longer, this function can
 * break locks and delay to ensure the write succeeds (but not sleep).
 */
int mtd_panic_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen,
		    const u_char *buf)
{
	*retlen = 0;
	if (!mtd->_panic_write)
		return -EOPNOTSUPP;
	if (to < 0 || to > mtd->size || len > mtd->size - to)
		return -EINVAL;
	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	if (!len)
		return 0;
	return mtd->_panic_write(mtd, to, len, retlen, buf);
}

int mtd_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	ops->retlen = ops->oobretlen = 0;
	if (!mtd->_read_oob)
		return -EOPNOTSUPP;
	return mtd->_read_oob(mtd, from, ops);
}

/*
 * Method to access the protection register area, present in some flash
 * devices. The user data is one time programmable but the factory data is read
 * only.
 */
int mtd_get_fact_prot_info(struct mtd_info *mtd, struct otp_info *buf,
			   size_t len)
{
	if (!mtd->_get_fact_prot_info)
		return -EOPNOTSUPP;
	if (!len)
		return 0;
	return mtd->_get_fact_prot_info(mtd, buf, len);
}

int mtd_read_fact_prot_reg(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t *retlen, u_char *buf)
{
	*retlen = 0;
	if (!mtd->_read_fact_prot_reg)
		return -EOPNOTSUPP;
	if (!len)
		return 0;
	return mtd->_read_fact_prot_reg(mtd, from, len, retlen, buf);
}

int mtd_get_user_prot_info(struct mtd_info *mtd, struct otp_info *buf,
			   size_t len)
{
	if (!mtd->_get_user_prot_info)
		return -EOPNOTSUPP;
	if (!len)
		return 0;
	return mtd->_get_user_prot_info(mtd, buf, len);
}

int mtd_read_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t *retlen, u_char *buf)
{
	*retlen = 0;
	if (!mtd->_read_user_prot_reg)
		return -EOPNOTSUPP;
	if (!len)
		return 0;
	return mtd->_read_user_prot_reg(mtd, from, len, retlen, buf);
}

int mtd_write_user_prot_reg(struct mtd_info *mtd, loff_t to, size_t len,
			    size_t *retlen, u_char *buf)
{
	*retlen = 0;
	if (!mtd->_write_user_prot_reg)
		return -EOPNOTSUPP;
	if (!len)
		return 0;
	return mtd->_write_user_prot_reg(mtd, to, len, retlen, buf);
}

int mtd_lock_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len)
{
	if (!mtd->_lock_user_prot_reg)
		return -EOPNOTSUPP;
	if (!len)
		return 0;
	return mtd->_lock_user_prot_reg(mtd, from, len);
}

/* Chip-supported device locking */
int mtd_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	if (!mtd->_lock)
		return -EOPNOTSUPP;
	if (ofs < 0 || ofs > mtd->size || len > mtd->size - ofs)
		return -EINVAL;
	if (!len)
		return 0;
	return mtd->_lock(mtd, ofs, len);
}

int mtd_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	if (!mtd->_unlock)
		return -EOPNOTSUPP;
	if (ofs < 0 || ofs > mtd->size || len > mtd->size - ofs)
		return -EINVAL;
	if (!len)
		return 0;
	return mtd->_unlock(mtd, ofs, len);
}

int mtd_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	if (!mtd->_block_isbad)
		return 0;
	if (ofs < 0 || ofs > mtd->size)
		return -EINVAL;
	return mtd->_block_isbad(mtd, ofs);
}

int mtd_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	if (!mtd->_block_markbad)
		return -EOPNOTSUPP;
	if (ofs < 0 || ofs > mtd->size)
		return -EINVAL;
	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	return mtd->_block_markbad(mtd, ofs);
}

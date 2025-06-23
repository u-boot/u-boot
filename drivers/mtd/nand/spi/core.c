// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2017 Micron Technology, Inc.
 *
 * Authors:
 *	Peter Pan <peterpandong@micron.com>
 *	Boris Brezillon <boris.brezillon@bootlin.com>
 */

#define pr_fmt(fmt)	"spi-nand: " fmt

#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/spinand.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-mem.h>
#else
#include <errno.h>
#include <watchdog.h>
#include <spi.h>
#include <spi-mem.h>
#include <ubi_uboot.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/mtd/spinand.h>
#include <linux/printk.h>
#include <linux/delay.h>
#endif

struct spinand_plat {
	struct mtd_info *mtd;
};

/* SPI NAND index visible in MTD names */
static int spi_nand_idx;

static int spinand_read_reg_op(struct spinand_device *spinand, u8 reg, u8 *val)
{
	struct spi_mem_op op = SPINAND_GET_FEATURE_OP(reg,
						      spinand->scratchbuf);
	int ret;

	ret = spi_mem_exec_op(spinand->slave, &op);
	if (ret)
		return ret;

	*val = *spinand->scratchbuf;
	return 0;
}

static int spinand_write_reg_op(struct spinand_device *spinand, u8 reg, u8 val)
{
	struct spi_mem_op op = SPINAND_SET_FEATURE_OP(reg,
						      spinand->scratchbuf);

	*spinand->scratchbuf = val;
	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinand_read_status(struct spinand_device *spinand, u8 *status)
{
	return spinand_read_reg_op(spinand, REG_STATUS, status);
}

static int spinand_get_cfg(struct spinand_device *spinand, u8 *cfg)
{
	struct nand_device *nand = spinand_to_nand(spinand);

	if (WARN_ON(spinand->cur_target < 0 ||
		    spinand->cur_target >= nand->memorg.ntargets))
		return -EINVAL;

	*cfg = spinand->cfg_cache[spinand->cur_target];
	return 0;
}

static int spinand_set_cfg(struct spinand_device *spinand, u8 cfg)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	int ret;

	if (WARN_ON(spinand->cur_target < 0 ||
		    spinand->cur_target >= nand->memorg.ntargets))
		return -EINVAL;

	if (spinand->cfg_cache[spinand->cur_target] == cfg)
		return 0;

	ret = spinand_write_reg_op(spinand, REG_CFG, cfg);
	if (ret)
		return ret;

	spinand->cfg_cache[spinand->cur_target] = cfg;
	return 0;
}

/**
 * spinand_upd_cfg() - Update the configuration register
 * @spinand: the spinand device
 * @mask: the mask encoding the bits to update in the config reg
 * @val: the new value to apply
 *
 * Update the configuration register.
 *
 * Return: 0 on success, a negative error code otherwise.
 */
int spinand_upd_cfg(struct spinand_device *spinand, u8 mask, u8 val)
{
	int ret;
	u8 cfg;

	ret = spinand_get_cfg(spinand, &cfg);
	if (ret)
		return ret;

	cfg &= ~mask;
	cfg |= val;

	return spinand_set_cfg(spinand, cfg);
}

/**
 * spinand_select_target() - Select a specific NAND target/die
 * @spinand: the spinand device
 * @target: the target/die to select
 *
 * Select a new target/die. If chip only has one die, this function is a NOOP.
 *
 * Return: 0 on success, a negative error code otherwise.
 */
int spinand_select_target(struct spinand_device *spinand, unsigned int target)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	int ret;

	if (WARN_ON(target >= nand->memorg.ntargets))
		return -EINVAL;

	if (spinand->cur_target == target)
		return 0;

	if (nand->memorg.ntargets == 1) {
		spinand->cur_target = target;
		return 0;
	}

	ret = spinand->select_target(spinand, target);
	if (ret)
		return ret;

	spinand->cur_target = target;
	return 0;
}

static int spinand_read_cfg(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int target;
	int ret;

	for (target = 0; target < nand->memorg.ntargets; target++) {
		ret = spinand_select_target(spinand, target);
		if (ret)
			return ret;

		/*
		 * We use spinand_read_reg_op() instead of spinand_get_cfg()
		 * here to bypass the config cache.
		 */
		ret = spinand_read_reg_op(spinand, REG_CFG,
					  &spinand->cfg_cache[target]);
		if (ret)
			return ret;
	}

	return 0;
}

static int spinand_init_cfg_cache(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	struct udevice *dev = spinand->slave->dev;

	spinand->cfg_cache = devm_kcalloc(dev,
					  nand->memorg.ntargets,
					  sizeof(*spinand->cfg_cache),
					  GFP_KERNEL);
	if (!spinand->cfg_cache)
		return -ENOMEM;

	return 0;
}

static int spinand_init_quad_enable(struct spinand_device *spinand)
{
	bool enable = false;

	if (!(spinand->flags & SPINAND_HAS_QE_BIT))
		return 0;

	if (spinand->op_templates.read_cache->data.buswidth == 4 ||
	    spinand->op_templates.write_cache->data.buswidth == 4 ||
	    spinand->op_templates.update_cache->data.buswidth == 4)
		enable = true;

	return spinand_upd_cfg(spinand, CFG_QUAD_ENABLE,
			       enable ? CFG_QUAD_ENABLE : 0);
}

static int spinand_ecc_enable(struct spinand_device *spinand,
			      bool enable)
{
	return spinand_upd_cfg(spinand, CFG_ECC_ENABLE,
			       enable ? CFG_ECC_ENABLE : 0);
}

static int spinand_check_ecc_status(struct spinand_device *spinand, u8 status)
{
	struct nand_device *nand = spinand_to_nand(spinand);

	if (spinand->eccinfo.get_status)
		return spinand->eccinfo.get_status(spinand, status);

	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_HAS_BITFLIPS:
		/*
		 * We have no way to know exactly how many bitflips have been
		 * fixed, so let's return the maximum possible value so that
		 * wear-leveling layers move the data immediately.
		 */
		return nanddev_get_ecc_conf(nand)->strength;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	default:
		break;
	}

	return -EINVAL;
}

static int spinand_noecc_ooblayout_ecc(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	return -ERANGE;
}

static int spinand_noecc_ooblayout_free(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 2 bytes for the BBM. */
	region->offset = 2;
	region->length = 62;

	return 0;
}

static const struct mtd_ooblayout_ops spinand_noecc_ooblayout = {
	.ecc = spinand_noecc_ooblayout_ecc,
	.rfree = spinand_noecc_ooblayout_free,
};

static int spinand_ondie_ecc_init_ctx(struct nand_device *nand)
{
	struct spinand_device *spinand = nand_to_spinand(nand);
	struct mtd_info *mtd = nanddev_to_mtd(nand);
	struct spinand_ondie_ecc_conf *engine_conf;

	nand->ecc.ctx.conf.engine_type = NAND_ECC_ENGINE_TYPE_ON_DIE;
	nand->ecc.ctx.conf.step_size = nand->ecc.requirements.step_size;
	nand->ecc.ctx.conf.strength = nand->ecc.requirements.strength;

	engine_conf = kzalloc(sizeof(*engine_conf), GFP_KERNEL);
	if (!engine_conf)
		return -ENOMEM;

	nand->ecc.ctx.priv = engine_conf;

	if (spinand->eccinfo.ooblayout)
		mtd_set_ooblayout(mtd, spinand->eccinfo.ooblayout);
	else
		mtd_set_ooblayout(mtd, &spinand_noecc_ooblayout);

	return 0;
}

static void spinand_ondie_ecc_cleanup_ctx(struct nand_device *nand)
{
	kfree(nand->ecc.ctx.priv);
}

static int spinand_ondie_ecc_prepare_io_req(struct nand_device *nand,
					    struct nand_page_io_req *req)
{
	struct spinand_device *spinand = nand_to_spinand(nand);
	bool enable = (req->mode != MTD_OPS_RAW);

	memset(spinand->oobbuf, 0xff, nanddev_per_page_oobsize(nand));

	/* Only enable or disable the engine */
	return spinand_ecc_enable(spinand, enable);
}

static int spinand_ondie_ecc_finish_io_req(struct nand_device *nand,
					   struct nand_page_io_req *req)
{
	struct spinand_ondie_ecc_conf *engine_conf = nand->ecc.ctx.priv;
	struct spinand_device *spinand = nand_to_spinand(nand);
	struct mtd_info *mtd = spinand_to_mtd(spinand);
	int ret;

	if (req->mode == MTD_OPS_RAW)
		return 0;

	/* Nothing to do when finishing a page write */
	if (req->type == NAND_PAGE_WRITE)
		return 0;

	/* Finish a page read: check the status, report errors/bitflips */
	ret = spinand_check_ecc_status(spinand, engine_conf->status);
	if (ret == -EBADMSG)
		mtd->ecc_stats.failed++;
	else if (ret > 0)
		mtd->ecc_stats.corrected += ret;

	return ret;
}

static struct nand_ecc_engine_ops spinand_ondie_ecc_engine_ops = {
	.init_ctx = spinand_ondie_ecc_init_ctx,
	.cleanup_ctx = spinand_ondie_ecc_cleanup_ctx,
	.prepare_io_req = spinand_ondie_ecc_prepare_io_req,
	.finish_io_req = spinand_ondie_ecc_finish_io_req,
};

static struct nand_ecc_engine spinand_ondie_ecc_engine = {
	.ops = &spinand_ondie_ecc_engine_ops,
};

static void spinand_ondie_ecc_save_status(struct nand_device *nand, u8 status)
{
	struct spinand_ondie_ecc_conf *engine_conf = nand->ecc.ctx.priv;

	if (nand->ecc.ctx.conf.engine_type == NAND_ECC_ENGINE_TYPE_ON_DIE &&
	    engine_conf)
		engine_conf->status = status;
}

static int spinand_write_enable_op(struct spinand_device *spinand)
{
	struct spi_mem_op op = SPINAND_WR_EN_DIS_OP(true);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinand_load_page_op(struct spinand_device *spinand,
				const struct nand_page_io_req *req)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int row = nanddev_pos_to_row(nand, &req->pos);
	struct spi_mem_op op = SPINAND_PAGE_READ_OP(row);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinand_read_from_cache_op(struct spinand_device *spinand,
				      const struct nand_page_io_req *req)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	struct mtd_info *mtd = spinand_to_mtd(spinand);
	struct spi_mem_dirmap_desc *rdesc;
	unsigned int nbytes = 0;
	void *buf = NULL;
	u16 column = 0;
	ssize_t ret;

	if (req->datalen) {
		buf = spinand->databuf;
		nbytes = nanddev_page_size(nand);
		column = 0;
	}

	if (req->ooblen) {
		nbytes += nanddev_per_page_oobsize(nand);
		if (!buf) {
			buf = spinand->oobbuf;
			column = nanddev_page_size(nand);
		}
	}

	if (req->mode == MTD_OPS_RAW)
		rdesc = spinand->dirmaps[req->pos.plane].rdesc;
	else
		rdesc = spinand->dirmaps[req->pos.plane].rdesc_ecc;

	while (nbytes) {
		ret = spi_mem_dirmap_read(rdesc, column, nbytes, buf);
		if (ret < 0)
			return ret;

		if (!ret || ret > nbytes)
			return -EIO;

		nbytes -= ret;
		column += ret;
		buf += ret;
	}

	if (req->datalen)
		memcpy(req->databuf.in, spinand->databuf + req->dataoffs,
		       req->datalen);

	if (req->ooblen) {
		if (req->mode == MTD_OPS_AUTO_OOB)
			mtd_ooblayout_get_databytes(mtd, req->oobbuf.in,
						    spinand->oobbuf,
						    req->ooboffs,
						    req->ooblen);
		else
			memcpy(req->oobbuf.in, spinand->oobbuf + req->ooboffs,
			       req->ooblen);
	}

	return 0;
}

static int spinand_write_to_cache_op(struct spinand_device *spinand,
				     const struct nand_page_io_req *req)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	struct mtd_info *mtd = spinand_to_mtd(spinand);
	struct spi_mem_dirmap_desc *wdesc;
	unsigned int nbytes, column = 0;
	void *buf = spinand->databuf;
	ssize_t ret;

	/*
	 * Looks like PROGRAM LOAD (AKA write cache) does not necessarily reset
	 * the cache content to 0xFF (depends on vendor implementation), so we
	 * must fill the page cache entirely even if we only want to program
	 * the data portion of the page, otherwise we might corrupt the BBM or
	 * user data previously programmed in OOB area.
	 *
	 * Only reset the data buffer manually, the OOB buffer is prepared by
	 * ECC engines ->prepare_io_req() callback.
	 */
	nbytes = nanddev_page_size(nand) + nanddev_per_page_oobsize(nand);
	memset(spinand->databuf, 0xff, nanddev_page_size(nand));

	if (req->datalen)
		memcpy(spinand->databuf + req->dataoffs, req->databuf.out,
		       req->datalen);

	if (req->ooblen) {
		if (req->mode == MTD_OPS_AUTO_OOB)
			mtd_ooblayout_set_databytes(mtd, req->oobbuf.out,
						    spinand->oobbuf,
						    req->ooboffs,
						    req->ooblen);
		else
			memcpy(spinand->oobbuf + req->ooboffs, req->oobbuf.out,
			       req->ooblen);
	}

	if (req->mode == MTD_OPS_RAW)
		wdesc = spinand->dirmaps[req->pos.plane].wdesc;
	else
		wdesc = spinand->dirmaps[req->pos.plane].wdesc_ecc;

	while (nbytes) {
		ret = spi_mem_dirmap_write(wdesc, column, nbytes, buf);
		if (ret < 0)
			return ret;

		if (!ret || ret > nbytes)
			return -EIO;

		nbytes -= ret;
		column += ret;
		buf += ret;
	}

	return 0;
}

static int spinand_program_op(struct spinand_device *spinand,
			      const struct nand_page_io_req *req)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int row = nanddev_pos_to_row(nand, &req->pos);
	struct spi_mem_op op = SPINAND_PROG_EXEC_OP(row);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinand_erase_op(struct spinand_device *spinand,
			    const struct nand_pos *pos)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int row = nanddev_pos_to_row(nand, pos);
	struct spi_mem_op op = SPINAND_BLK_ERASE_OP(row);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinand_wait(struct spinand_device *spinand,
			unsigned long initial_delay_us,
			unsigned long poll_delay_us,
			u8 *s)
{
	unsigned long start, stop;
	u8 status;
	int ret;

	udelay(initial_delay_us);
	start = get_timer(0);
	stop = SPINAND_WAITRDY_TIMEOUT_MS;
	do {
		schedule();

		ret = spinand_read_status(spinand, &status);
		if (ret)
			return ret;

		if (!(status & STATUS_BUSY))
			goto out;

		udelay(poll_delay_us);
	} while (get_timer(start) < stop);

	/*
	 * Extra read, just in case the STATUS_READY bit has changed
	 * since our last check
	 */
	ret = spinand_read_status(spinand, &status);
	if (ret)
		return ret;

out:
	if (s)
		*s = status;

	return status & STATUS_BUSY ? -ETIMEDOUT : 0;
}

static int spinand_read_id_op(struct spinand_device *spinand, u8 naddr,
			      u8 ndummy, u8 *buf)
{
	struct spi_mem_op op = SPINAND_READID_OP(
		naddr, ndummy, spinand->scratchbuf, SPINAND_MAX_ID_LEN);
	int ret;

	ret = spi_mem_exec_op(spinand->slave, &op);
	if (!ret)
		memcpy(buf, spinand->scratchbuf, SPINAND_MAX_ID_LEN);

	return ret;
}

static int spinand_reset_op(struct spinand_device *spinand)
{
	struct spi_mem_op op = SPINAND_RESET_OP;
	int ret;

	ret = spi_mem_exec_op(spinand->slave, &op);
	if (ret)
		return ret;

	return spinand_wait(spinand,
			    SPINAND_RESET_INITIAL_DELAY_US,
			    SPINAND_RESET_POLL_DELAY_US,
			    NULL);
}

static int spinand_lock_block(struct spinand_device *spinand, u8 lock)
{
	return spinand_write_reg_op(spinand, REG_BLOCK_LOCK, lock);
}

static int spinand_read_page(struct spinand_device *spinand,
			     const struct nand_page_io_req *req)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	u8 status;
	int ret;

	ret = nand_ecc_prepare_io_req(nand, (struct nand_page_io_req *)req);
	if (ret)
		return ret;

	ret = spinand_load_page_op(spinand, req);
	if (ret)
		return ret;

	ret = spinand_wait(spinand,
			   SPINAND_READ_INITIAL_DELAY_US,
			   SPINAND_READ_POLL_DELAY_US,
			   &status);
	if (ret < 0)
		return ret;

	spinand_ondie_ecc_save_status(nand, status);

	ret = spinand_read_from_cache_op(spinand, req);
	if (ret)
		return ret;

	return nand_ecc_finish_io_req(nand, (struct nand_page_io_req *)req);
}

static int spinand_write_page(struct spinand_device *spinand,
			      const struct nand_page_io_req *req)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	u8 status;
	int ret;

	ret = nand_ecc_prepare_io_req(nand, (struct nand_page_io_req *)req);
	if (ret)
		return ret;

	ret = spinand_write_enable_op(spinand);
	if (ret)
		return ret;

	ret = spinand_write_to_cache_op(spinand, req);
	if (ret)
		return ret;

	ret = spinand_program_op(spinand, req);
	if (ret)
		return ret;

	ret = spinand_wait(spinand,
			   SPINAND_WRITE_INITIAL_DELAY_US,
			   SPINAND_WRITE_POLL_DELAY_US,
			   &status);
	if (!ret && (status & STATUS_PROG_FAILED))
		return -EIO;

	return nand_ecc_finish_io_req(nand, (struct nand_page_io_req *)req);
}

static int spinand_mtd_read(struct mtd_info *mtd, loff_t from,
			    struct mtd_oob_ops *ops)
{
	struct spinand_device *spinand = mtd_to_spinand(mtd);
	struct nand_device *nand = mtd_to_nanddev(mtd);
	unsigned int max_bitflips = 0;
	struct nand_io_iter iter;
	bool disable_ecc = false;
	bool ecc_failed = false;
	int ret = 0;

	if (ops->mode == MTD_OPS_RAW || !spinand->eccinfo.ooblayout)
		disable_ecc = true;

#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif

	nanddev_io_for_each_page(nand, NAND_PAGE_READ, from, ops, &iter) {
		schedule();
		if (disable_ecc)
			iter.req.mode = MTD_OPS_RAW;

		ret = spinand_select_target(spinand, iter.req.pos.target);
		if (ret)
			break;

		ret = spinand_read_page(spinand, &iter.req);
		if (ret < 0 && ret != -EBADMSG)
			break;

		if (ret == -EBADMSG)
			ecc_failed = true;
		else
			max_bitflips = max_t(unsigned int, max_bitflips, ret);

		ret = 0;
		ops->retlen += iter.req.datalen;
		ops->oobretlen += iter.req.ooblen;
	}

#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif
	if (ecc_failed && !ret)
		ret = -EBADMSG;

	return ret ? ret : max_bitflips;
}

static int spinand_mtd_write(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops)
{
	struct spinand_device *spinand = mtd_to_spinand(mtd);
	struct nand_device *nand = mtd_to_nanddev(mtd);
	struct nand_io_iter iter;
	bool disable_ecc = false;
	int ret = 0;

	if (ops->mode == MTD_OPS_RAW || !mtd->ooblayout)
		disable_ecc = true;

#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif

	nanddev_io_for_each_page(nand, NAND_PAGE_WRITE, to, ops, &iter) {
		schedule();
		if (disable_ecc)
			iter.req.mode = MTD_OPS_RAW;

		ret = spinand_select_target(spinand, iter.req.pos.target);
		if (ret)
			break;

		ret = spinand_write_page(spinand, &iter.req);
		if (ret)
			break;

		ops->retlen += iter.req.datalen;
		ops->oobretlen += iter.req.ooblen;
	}

#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif

	return ret;
}

static bool spinand_isbad(struct nand_device *nand, const struct nand_pos *pos)
{
	struct spinand_device *spinand = nand_to_spinand(nand);
	u8 marker[2] = { };
	struct nand_page_io_req req = {
		.pos = *pos,
		.ooblen = sizeof(marker),
		.ooboffs = 0,
		.oobbuf.in = marker,
		.mode = MTD_OPS_RAW,
	};

	spinand_select_target(spinand, pos->target);
	spinand_read_page(spinand, &req);
	if (marker[0] != 0xff || marker[1] != 0xff)
		return true;

	return false;
}

static int spinand_mtd_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	struct nand_device *nand = mtd_to_nanddev(mtd);
#ifndef __UBOOT__
	struct spinand_device *spinand = nand_to_spinand(nand);
#endif
	struct nand_pos pos;
	int ret;

	nanddev_offs_to_pos(nand, offs, &pos);
#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif
	ret = nanddev_isbad(nand, &pos);
#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif
	return ret;
}

static int spinand_markbad(struct nand_device *nand, const struct nand_pos *pos)
{
	struct spinand_device *spinand = nand_to_spinand(nand);
	u8 marker[2] = { };
	struct nand_page_io_req req = {
		.pos = *pos,
		.ooboffs = 0,
		.ooblen = sizeof(marker),
		.oobbuf.out = marker,
		.mode = MTD_OPS_RAW,
	};
	int ret;

	ret = spinand_select_target(spinand, pos->target);
	if (ret)
		return ret;

	ret = spinand_write_enable_op(spinand);
	if (ret)
		return ret;

	return spinand_write_page(spinand, &req);
}

static int spinand_mtd_block_markbad(struct mtd_info *mtd, loff_t offs)
{
	struct nand_device *nand = mtd_to_nanddev(mtd);
#ifndef __UBOOT__
	struct spinand_device *spinand = nand_to_spinand(nand);
#endif
	struct nand_pos pos;
	int ret;

	nanddev_offs_to_pos(nand, offs, &pos);
#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif
	ret = nanddev_markbad(nand, &pos);
#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif
	return ret;
}

static int spinand_erase(struct nand_device *nand, const struct nand_pos *pos)
{
	struct spinand_device *spinand = nand_to_spinand(nand);
	u8 status;
	int ret;

	ret = spinand_select_target(spinand, pos->target);
	if (ret)
		return ret;

	ret = spinand_write_enable_op(spinand);
	if (ret)
		return ret;

	ret = spinand_erase_op(spinand, pos);
	if (ret)
		return ret;

	ret = spinand_wait(spinand,
			   SPINAND_ERASE_INITIAL_DELAY_US,
			   SPINAND_ERASE_POLL_DELAY_US,
			   &status);
	if (!ret && (status & STATUS_ERASE_FAILED))
		ret = -EIO;

	return ret;
}

static int spinand_mtd_erase(struct mtd_info *mtd,
			     struct erase_info *einfo)
{
#ifndef __UBOOT__
	struct spinand_device *spinand = mtd_to_spinand(mtd);
#endif
	int ret;

#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif
	ret = nanddev_mtd_erase(mtd, einfo);
#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif

	return ret;
}

static int spinand_mtd_block_isreserved(struct mtd_info *mtd, loff_t offs)
{
#ifndef __UBOOT__
	struct spinand_device *spinand = mtd_to_spinand(mtd);
#endif
	struct nand_device *nand = mtd_to_nanddev(mtd);
	struct nand_pos pos;
	int ret;

	nanddev_offs_to_pos(nand, offs, &pos);
#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif
	ret = nanddev_isreserved(nand, &pos);
#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif

	return ret;
}

static int spinand_create_dirmap(struct spinand_device *spinand,
				 unsigned int plane)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	struct spi_mem_dirmap_info info = {
		.length = nanddev_page_size(nand) +
			  nanddev_per_page_oobsize(nand),
	};
	struct spi_mem_dirmap_desc *desc;

	/* The plane number is passed in MSB just above the column address */
	info.offset = plane << fls(nand->memorg.pagesize);

	info.op_tmpl = *spinand->op_templates.update_cache;
	desc = spi_mem_dirmap_create(spinand->slave, &info);
	if (IS_ERR(desc))
		return PTR_ERR(desc);

	spinand->dirmaps[plane].wdesc = desc;

	info.op_tmpl = *spinand->op_templates.read_cache;
	desc = spi_mem_dirmap_create(spinand->slave, &info);
	if (IS_ERR(desc)) {
		spi_mem_dirmap_destroy(spinand->dirmaps[plane].wdesc);
		return PTR_ERR(desc);
	}

	spinand->dirmaps[plane].rdesc = desc;

	if (nand->ecc.engine->integration != NAND_ECC_ENGINE_INTEGRATION_PIPELINED) {
		spinand->dirmaps[plane].wdesc_ecc = spinand->dirmaps[plane].wdesc;
		spinand->dirmaps[plane].rdesc_ecc = spinand->dirmaps[plane].rdesc;

		return 0;
	}

	info.op_tmpl = *spinand->op_templates.update_cache;
	info.op_tmpl.data.ecc = true;
	desc = spi_mem_dirmap_create(spinand->slave, &info);
	if (IS_ERR(desc)) {
		spi_mem_dirmap_destroy(spinand->dirmaps[plane].wdesc);
		spi_mem_dirmap_destroy(spinand->dirmaps[plane].rdesc);
		return PTR_ERR(desc);
	}

	spinand->dirmaps[plane].wdesc_ecc = desc;

	info.op_tmpl = *spinand->op_templates.read_cache;
	info.op_tmpl.data.ecc = true;
	desc = spi_mem_dirmap_create(spinand->slave, &info);
	if (IS_ERR(desc)) {
		spi_mem_dirmap_destroy(spinand->dirmaps[plane].wdesc);
		spi_mem_dirmap_destroy(spinand->dirmaps[plane].rdesc);
		spi_mem_dirmap_destroy(spinand->dirmaps[plane].wdesc_ecc);
		return PTR_ERR(desc);
	}

	spinand->dirmaps[plane].rdesc_ecc = desc;

	return 0;
}

static int spinand_create_dirmaps(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	int i, ret;

	spinand->dirmaps = devm_kzalloc(spinand->slave->dev,
					sizeof(*spinand->dirmaps) *
					nand->memorg.planes_per_lun,
					GFP_KERNEL);
	if (!spinand->dirmaps)
		return -ENOMEM;

	for (i = 0; i < nand->memorg.planes_per_lun; i++) {
		ret = spinand_create_dirmap(spinand, i);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct nand_ops spinand_ops = {
	.erase = spinand_erase,
	.markbad = spinand_markbad,
	.isbad = spinand_isbad,
};

static const struct spinand_manufacturer *spinand_manufacturers[] = {
	&alliancememory_spinand_manufacturer,
	&ato_spinand_manufacturer,
	&esmt_c8_spinand_manufacturer,
	&foresee_spinand_manufacturer,
	&gigadevice_spinand_manufacturer,
	&macronix_spinand_manufacturer,
	&micron_spinand_manufacturer,
	&paragon_spinand_manufacturer,
	&toshiba_spinand_manufacturer,
	&winbond_spinand_manufacturer,
	&xtx_spinand_manufacturer,
};

static int spinand_manufacturer_match(struct spinand_device *spinand,
				      enum spinand_readid_method rdid_method)
{
	u8 *id = spinand->id.data;
	unsigned int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(spinand_manufacturers); i++) {
		const struct spinand_manufacturer *manufacturer =
			spinand_manufacturers[i];

		if (id[0] != manufacturer->id)
			continue;

		ret = spinand_match_and_init(spinand,
					     manufacturer->chips,
					     manufacturer->nchips,
					     rdid_method);
		if (ret < 0)
			continue;

		spinand->manufacturer = manufacturer;
		return 0;
	}
	return -ENOTSUPP;
}

static int spinand_id_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;

	ret = spinand_read_id_op(spinand, 0, 0, id);
	if (ret)
		return ret;
	ret = spinand_manufacturer_match(spinand, SPINAND_READID_METHOD_OPCODE);
	if (!ret)
		return 0;

	ret = spinand_read_id_op(spinand, 1, 0, id);
	if (ret)
		return ret;
	ret = spinand_manufacturer_match(spinand,
					 SPINAND_READID_METHOD_OPCODE_ADDR);
	if (!ret)
		return 0;

	ret = spinand_read_id_op(spinand, 0, 1, id);
	if (ret)
		return ret;
	ret = spinand_manufacturer_match(spinand,
					 SPINAND_READID_METHOD_OPCODE_DUMMY);

	return ret;
}

static int spinand_manufacturer_init(struct spinand_device *spinand)
{
	if (spinand->manufacturer->ops->init)
		return spinand->manufacturer->ops->init(spinand);

	return 0;
}

static void spinand_manufacturer_cleanup(struct spinand_device *spinand)
{
	/* Release manufacturer private data */
	if (spinand->manufacturer->ops->cleanup)
		return spinand->manufacturer->ops->cleanup(spinand);
}

static const struct spi_mem_op *
spinand_select_op_variant(struct spinand_device *spinand,
			  const struct spinand_op_variants *variants)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int i;

	for (i = 0; i < variants->nops; i++) {
		struct spi_mem_op op = variants->ops[i];
		unsigned int nbytes;
		int ret;

		nbytes = nanddev_per_page_oobsize(nand) +
			 nanddev_page_size(nand);

		while (nbytes) {
			op.data.nbytes = nbytes;
			ret = spi_mem_adjust_op_size(spinand->slave, &op);
			if (ret)
				break;

			if (!spi_mem_supports_op(spinand->slave, &op))
				break;

			nbytes -= op.data.nbytes;
		}

		if (!nbytes)
			return &variants->ops[i];
	}

	return NULL;
}

static int spinand_setup_slave(struct spinand_device *spinand,
			       const struct spinand_info *spinand_info)
{
	struct spi_slave *slave = spinand->slave;
	struct udevice *bus = slave->dev->parent;
	struct dm_spi_ops *ops = spi_get_ops(bus);

	if (!ops->setup_for_spinand)
		return 0;

	return ops->setup_for_spinand(slave, spinand_info);
}

/**
 * spinand_match_and_init() - Try to find a match between a device ID and an
 *			      entry in a spinand_info table
 * @spinand: SPI NAND object
 * @table: SPI NAND device description table
 * @table_size: size of the device description table
 * @rdid_method: read id method to match
 *
 * Match between a device ID retrieved through the READ_ID command and an
 * entry in the SPI NAND description table. If a match is found, the spinand
 * object will be initialized with information provided by the matching
 * spinand_info entry.
 *
 * Return: 0 on success, a negative error code otherwise.
 */
int spinand_match_and_init(struct spinand_device *spinand,
			   const struct spinand_info *table,
			   unsigned int table_size,
			   enum spinand_readid_method rdid_method)
{
	u8 *id = spinand->id.data;
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int i;
	int ret;

	for (i = 0; i < table_size; i++) {
		const struct spinand_info *info = &table[i];
		const struct spi_mem_op *op;

		if (rdid_method != info->devid.method)
			continue;

		if (memcmp(id + 1, info->devid.id, info->devid.len))
			continue;

		ret = spinand_setup_slave(spinand, info);
		if (ret)
			return ret;

		nand->memorg = table[i].memorg;
		nanddev_set_ecc_requirements(nand, &table[i].eccreq);
		spinand->eccinfo = table[i].eccinfo;
		spinand->flags = table[i].flags;
		spinand->id.len = 1 + table[i].devid.len;
		spinand->select_target = table[i].select_target;

		op = spinand_select_op_variant(spinand,
					       info->op_variants.read_cache);
		if (!op)
			return -ENOTSUPP;

		spinand->op_templates.read_cache = op;

		op = spinand_select_op_variant(spinand,
					       info->op_variants.write_cache);
		if (!op)
			return -ENOTSUPP;

		spinand->op_templates.write_cache = op;

		op = spinand_select_op_variant(spinand,
					       info->op_variants.update_cache);
		spinand->op_templates.update_cache = op;

		return 0;
	}

	return -ENOTSUPP;
}

static int spinand_detect(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	int ret;

	ret = spinand_reset_op(spinand);
	if (ret)
		return ret;

	ret = spinand_id_detect(spinand);
	if (ret) {
		dev_err(spinand->slave->dev, "unknown raw ID %02x %02x %02x %02x\n",
			spinand->id.data[0], spinand->id.data[1],
			spinand->id.data[2], spinand->id.data[3]);
		return ret;
	}

	if (nand->memorg.ntargets > 1 && !spinand->select_target) {
		dev_err(spinand->slave->dev,
			"SPI NANDs with more than one die must implement ->select_target()\n");
		return -EINVAL;
	}

	dev_info(spinand->slave->dev,
		 "%s SPI NAND was found.\n", spinand->manufacturer->name);
	dev_info(spinand->slave->dev,
		 "%llu MiB, block size: %zu KiB, page size: %zu, OOB size: %u\n",
		 nanddev_size(nand) >> 20, nanddev_eraseblock_size(nand) >> 10,
		 nanddev_page_size(nand), nanddev_per_page_oobsize(nand));

	return 0;
}

static int spinand_init_flash(struct spinand_device *spinand)
{
	struct udevice *dev = spinand->slave->dev;
	struct nand_device *nand = spinand_to_nand(spinand);
	int ret, i;

	ret = spinand_read_cfg(spinand);
	if (ret)
		return ret;

	ret = spinand_init_quad_enable(spinand);
	if (ret)
		return ret;

	ret = spinand_upd_cfg(spinand, CFG_OTP_ENABLE, 0);
	if (ret)
		return ret;

	ret = spinand_manufacturer_init(spinand);
	if (ret) {
		dev_err(dev,
			"Failed to initialize the SPI NAND chip (err = %d)\n",
			ret);
		return ret;
	}

	/* After power up, all blocks are locked, so unlock them here. */
	for (i = 0; i < nand->memorg.ntargets; i++) {
		ret = spinand_select_target(spinand, i);
		if (ret)
			break;

		ret = spinand_lock_block(spinand, BL_ALL_UNLOCKED);
		if (ret)
			break;
	}

	if (ret)
		spinand_manufacturer_cleanup(spinand);

	return ret;
}

static int spinand_init(struct spinand_device *spinand)
{
	struct udevice *dev = spinand->slave->dev;
	struct mtd_info *mtd = spinand_to_mtd(spinand);
	struct nand_device *nand = mtd_to_nanddev(mtd);
	int ret;

	/*
	 * We need a scratch buffer because the spi_mem interface requires that
	 * buf passed in spi_mem_op->data.buf be DMA-able.
	 */
	spinand->scratchbuf = kzalloc(SPINAND_MAX_ID_LEN, GFP_KERNEL);
	if (!spinand->scratchbuf)
		return -ENOMEM;

	ret = spinand_detect(spinand);
	if (ret)
		goto err_free_bufs;

	/*
	 * Use kzalloc() instead of devm_kzalloc() here, because some drivers
	 * may use this buffer for DMA access.
	 * Memory allocated by devm_ does not guarantee DMA-safe alignment.
	 */
	spinand->databuf = kzalloc(nanddev_page_size(nand) +
			       nanddev_per_page_oobsize(nand),
			       GFP_KERNEL);
	if (!spinand->databuf) {
		ret = -ENOMEM;
		goto err_free_bufs;
	}

	spinand->oobbuf = spinand->databuf + nanddev_page_size(nand);

	ret = spinand_init_cfg_cache(spinand);
	if (ret)
		goto err_free_bufs;

	ret = spinand_init_flash(spinand);
	if (ret)
		goto err_free_bufs;

	ret = nanddev_init(nand, &spinand_ops, THIS_MODULE);
	if (ret)
		goto err_manuf_cleanup;

	/* SPI-NAND default ECC engine is on-die */
	nand->ecc.defaults.engine_type = NAND_ECC_ENGINE_TYPE_ON_DIE;
	nand->ecc.ondie_engine = &spinand_ondie_ecc_engine;

	spinand_ecc_enable(spinand, false);
	ret = nanddev_ecc_engine_init(nand);
	if (ret)
		goto err_cleanup_nanddev;

	mtd->_read_oob = spinand_mtd_read;
	mtd->_write_oob = spinand_mtd_write;
	mtd->_block_isbad = spinand_mtd_block_isbad;
	mtd->_block_markbad = spinand_mtd_block_markbad;
	mtd->_block_isreserved = spinand_mtd_block_isreserved;
	mtd->_erase = spinand_mtd_erase;

	if (nand->ecc.engine) {
		ret = mtd_ooblayout_count_freebytes(mtd);
		if (ret < 0)
			goto err_cleanup_ecc_engine;
	}

	mtd->oobavail = ret;

	/* Propagate ECC information to mtd_info */
	mtd->ecc_strength = nanddev_get_ecc_conf(nand)->strength;
	mtd->ecc_step_size = nanddev_get_ecc_conf(nand)->step_size;

	ret = spinand_create_dirmaps(spinand);
	if (ret) {
		dev_err(dev,
			"Failed to create direct mappings for read/write operations (err = %d)\n",
			ret);
		goto err_cleanup_ecc_engine;
	}

	return 0;

err_cleanup_ecc_engine:
	nanddev_ecc_engine_cleanup(nand);

err_cleanup_nanddev:
	nanddev_cleanup(nand);

err_manuf_cleanup:
	spinand_manufacturer_cleanup(spinand);

err_free_bufs:
	kfree(spinand->databuf);
	kfree(spinand->scratchbuf);
	return ret;
}

static void spinand_cleanup(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);

	nanddev_cleanup(nand);
	spinand_manufacturer_cleanup(spinand);
	kfree(spinand->databuf);
	kfree(spinand->scratchbuf);
}

static int spinand_bind(struct udevice *dev)
{
	if (blk_enabled()) {
		struct spinand_plat *plat = dev_get_plat(dev);
		int ret;

		if (CONFIG_IS_ENABLED(MTD_BLOCK)) {
			ret = mtd_bind(dev, &plat->mtd);
			if (ret)
				return ret;
		}

		if (CONFIG_IS_ENABLED(UBI_BLOCK))
			return ubi_bind(dev);
	}

	return 0;
}

static int spinand_probe(struct udevice *dev)
{
	struct spinand_device *spinand = dev_get_priv(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct mtd_info *mtd = dev_get_uclass_priv(dev);
	struct nand_device *nand = spinand_to_nand(spinand);
	struct spinand_plat *plat = dev_get_plat(dev);
	int ret;

#ifndef __UBOOT__
	spinand = devm_kzalloc(&mem->spi->dev, sizeof(*spinand),
			       GFP_KERNEL);
	if (!spinand)
		return -ENOMEM;

	spinand->spimem = mem;
	spi_mem_set_drvdata(mem, spinand);
	spinand_set_of_node(spinand, mem->spi->dev.of_node);
	mutex_init(&spinand->lock);

	mtd = spinand_to_mtd(spinand);
	mtd->dev.parent = &mem->spi->dev;
#else
	nand->mtd = mtd;
	mtd->priv = nand;
	mtd->dev = dev;
	mtd->name = malloc(20);
	if (!mtd->name)
		return -ENOMEM;
	sprintf(mtd->name, "spi-nand%d", spi_nand_idx++);
	spinand->slave = slave;
	spinand_set_ofnode(spinand, dev_ofnode(dev));
#endif

	ret = spinand_init(spinand);
	if (ret)
		return ret;

#ifndef __UBOOT__
	ret = mtd_device_register(mtd, NULL, 0);
#else
	ret = add_mtd_device(mtd);
#endif
	if (ret)
		goto err_spinand_cleanup;

	plat->mtd = mtd;

	return 0;

err_spinand_cleanup:
	spinand_cleanup(spinand);

	return ret;
}

#ifndef __UBOOT__
static int spinand_remove(struct udevice *slave)
{
	struct spinand_device *spinand;
	struct mtd_info *mtd;
	int ret;

	spinand = spi_mem_get_drvdata(slave);
	mtd = spinand_to_mtd(spinand);
	free(mtd->name);

	ret = mtd_device_unregister(mtd);
	if (ret)
		return ret;

	spinand_cleanup(spinand);

	return 0;
}

static const struct spi_device_id spinand_ids[] = {
	{ .name = "spi-nand" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(spi, spinand_ids);

#ifdef CONFIG_OF
static const struct of_device_id spinand_of_ids[] = {
	{ .compatible = "spi-nand" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, spinand_of_ids);
#endif

static struct spi_mem_driver spinand_drv = {
	.spidrv = {
		.id_table = spinand_ids,
		.driver = {
			.name = "spi-nand",
			.of_match_table = of_match_ptr(spinand_of_ids),
		},
	},
	.probe = spinand_probe,
	.remove = spinand_remove,
};
module_spi_mem_driver(spinand_drv);

MODULE_DESCRIPTION("SPI NAND framework");
MODULE_AUTHOR("Peter Pan<peterpandong@micron.com>");
MODULE_LICENSE("GPL v2");
#endif /* __UBOOT__ */

static const struct udevice_id spinand_ids[] = {
	{ .compatible = "spi-nand" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(spinand) = {
	.name = "spi_nand",
	.id = UCLASS_MTD,
	.of_match = spinand_ids,
	.priv_auto	= sizeof(struct spinand_device),
	.probe = spinand_probe,
	.bind = spinand_bind,
	.plat_auto = sizeof(struct spinand_plat),
};

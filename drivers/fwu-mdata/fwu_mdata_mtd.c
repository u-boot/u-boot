// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <efi_loader.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <malloc.h>
#include <memalign.h>
#include <spi.h>
#include <spi_flash.h>
#include <flash.h>

#include <linux/errno.h>
#include <linux/types.h>
#include <u-boot/crc.h>

struct fwu_mdata_mtd_priv {
	struct mtd_info *mtd;
	u32 pri_offset;
	u32 sec_offset;
};

enum fwu_mtd_op {
	FWU_MTD_READ,
	FWU_MTD_WRITE,
};

static bool mtd_is_aligned_with_block_size(struct mtd_info *mtd, u64 size)
{
	return !do_div(size, mtd->erasesize);
}

static int mtd_io_data(struct mtd_info *mtd, u32 offs, u32 size, void *data,
		       enum fwu_mtd_op op)
{
	struct mtd_oob_ops io_op ={};
	u64 lock_offs, lock_len;
	size_t len;
	void *buf;
	int ret;

	if (!mtd_is_aligned_with_block_size(mtd, offs))
		return -EINVAL;
	lock_offs = offs;
	lock_len = round_up(size, mtd->erasesize);

	ret = mtd_unlock(mtd, lock_offs, lock_len);
	if (ret && ret != -EOPNOTSUPP)
		return ret;

	if (op == FWU_MTD_WRITE) {
		struct erase_info erase_op = {};

		/* This will expand erase size to align with the block size */
		erase_op.mtd = mtd;
		erase_op.addr = lock_offs;
		erase_op.len = lock_len;
		erase_op.scrub = 0;

		ret = mtd_erase(mtd, &erase_op);
		if (ret)
			goto lock_out;
	}

	/* Also, expand the write size to align with the write size */
	len = round_up(size, mtd->writesize);

	buf = memalign(ARCH_DMA_MINALIGN, len);
	if (!buf) {
		ret = -ENOMEM;
		goto lock_out;
	}
	io_op.mode = MTD_OPS_AUTO_OOB;
	io_op.len = len;
	io_op.ooblen = 0;
	io_op.datbuf = buf;
	io_op.oobbuf = NULL;

	if (op == FWU_MTD_WRITE) {
		memcpy(buf, data, size);
		ret = mtd_write_oob(mtd, offs, &io_op);
	} else {
		ret = mtd_read_oob(mtd, offs, &io_op);
		if (!ret)
			memcpy(data, buf, size);
	}
	free(buf);

lock_out:
	mtd_lock(mtd, lock_offs, lock_len);

	return ret;
}

static int fwu_mtd_load_mdata(struct mtd_info *mtd, struct fwu_mdata **mdata,
			      u32 offs, bool primary)
{
	size_t size = sizeof(struct fwu_mdata);
	int ret;

	*mdata = malloc(size);
	if (!*mdata)
		return -ENOMEM;

	ret = mtd_io_data(mtd, offs, size, (void *)*mdata, FWU_MTD_READ);
	if (ret >= 0) {
		ret = fwu_verify_mdata(*mdata, primary);
		if (ret < 0) {
			free(*mdata);
			*mdata = NULL;
		}
	}

	return ret;
}

static int fwu_mtd_load_primary_mdata(struct fwu_mdata_mtd_priv *mtd_priv,
				     struct fwu_mdata **mdata)
{
	return fwu_mtd_load_mdata(mtd_priv->mtd, mdata, mtd_priv->pri_offset, true);
}

static int fwu_mtd_load_secondary_mdata(struct fwu_mdata_mtd_priv *mtd_priv,
				       struct fwu_mdata **mdata)
{
	return fwu_mtd_load_mdata(mtd_priv->mtd, mdata, mtd_priv->sec_offset, false);
}

static int fwu_mtd_save_primary_mdata(struct fwu_mdata_mtd_priv *mtd_priv,
				     struct fwu_mdata *mdata)
{
	return mtd_io_data(mtd_priv->mtd, mtd_priv->pri_offset,
			   sizeof(struct fwu_mdata), mdata, FWU_MTD_WRITE);
}

static int fwu_mtd_save_secondary_mdata(struct fwu_mdata_mtd_priv *mtd_priv,
				       struct fwu_mdata *mdata)
{
	return mtd_io_data(mtd_priv->mtd, mtd_priv->sec_offset,
			   sizeof(struct fwu_mdata), mdata, FWU_MTD_WRITE);
}

static int fwu_mtd_get_valid_mdata(struct fwu_mdata_mtd_priv *mtd_priv,
				  struct fwu_mdata **mdata)
{
	if (fwu_mtd_load_primary_mdata(mtd_priv, mdata) == 0)
		return 0;

	log_err("Failed to load/verify primary mdata. Try secondary.\n");

	if (fwu_mtd_load_secondary_mdata(mtd_priv, mdata) == 0)
		return 0;

	log_err("Failed to load/verify secondary mdata.\n");

	return -1;
}

static int fwu_mtd_update_mdata(struct udevice *dev, struct fwu_mdata *mdata)
{
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(dev);
	int ret;

	/* Update mdata crc32 field */
	mdata->crc32 = crc32(0, (void *)&mdata->version,
			     sizeof(*mdata) - sizeof(u32));

	/* First write the primary mdata */
	ret = fwu_mtd_save_primary_mdata(mtd_priv, mdata);
	if (ret < 0) {
		log_err("Failed to update the primary mdata.\n");
		return ret;
	}

	/* And now the replica */
	ret = fwu_mtd_save_secondary_mdata(mtd_priv, mdata);
	if (ret < 0) {
		log_err("Failed to update the secondary mdata.\n");
		return ret;
	}

	return 0;
}

static int fwu_mtd_mdata_check(struct udevice *dev)
{
	struct fwu_mdata *primary = NULL, *secondary = NULL;
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(dev);
	int ret;

	ret = fwu_mtd_load_primary_mdata(mtd_priv, &primary);
	if (ret < 0)
		log_err("Failed to read the primary mdata: %d\n", ret);

	ret = fwu_mtd_load_secondary_mdata(mtd_priv, &secondary);
	if (ret < 0)
		log_err("Failed to read the secondary mdata: %d\n", ret);

	if (primary && secondary) {
		if (memcmp(primary, secondary, sizeof(struct fwu_mdata))) {
			log_err("The primary and the secondary mdata are different\n");
			ret = -1;
		}
	} else if (primary) {
		ret = fwu_mtd_save_secondary_mdata(mtd_priv, primary);
		if (ret < 0)
			log_err("Restoring secondary mdata partition failed\n");
	} else if (secondary) {
		ret = fwu_mtd_save_primary_mdata(mtd_priv, secondary);
		if (ret < 0)
			log_err("Restoring primary mdata partition failed\n");
	}

	free(primary);
	free(secondary);
	return ret;
}

static int fwu_mtd_get_mdata(struct udevice *dev, struct fwu_mdata **mdata)
{
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(dev);

	return fwu_mtd_get_valid_mdata(mtd_priv, mdata);
}

/**
 * fwu_mdata_mtd_of_to_plat() - Translate from DT to fwu mdata device
 */
static int fwu_mdata_mtd_of_to_plat(struct udevice *dev)
{
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(dev);
	const fdt32_t *phandle_p = NULL;
	struct udevice *mtd_dev;
	struct mtd_info *mtd;
	int ret, size;
	u32 phandle;

	/* Find the FWU mdata storage device */
	phandle_p = ofnode_get_property(dev_ofnode(dev),
					"fwu-mdata-store", &size);
	if (!phandle_p) {
		log_err("fwu-mdata-store property not found\n");
		return -ENOENT;
	}

	phandle = fdt32_to_cpu(*phandle_p);

	ret = device_get_global_by_ofnode(
		ofnode_get_by_phandle(phandle),
		&mtd_dev);
	if (ret)
		return ret;

	mtd_probe_devices();

	mtd_for_each_device(mtd) {
		if (mtd->dev == mtd_dev) {
			mtd_priv->mtd = mtd;
			log_debug("Found the FWU mdata mtd device %s\n", mtd->name);
			break;
		}
	}
	if (!mtd_priv->mtd) {
		log_err("Failed to find mtd device by fwu-mdata-store\n");
		return -ENOENT;
	}

	/* Get the offset of primary and seconday mdata */
	ret = ofnode_read_u32_index(dev_ofnode(dev), "mdata-offsets", 0,
				    &mtd_priv->pri_offset);
	if (ret)
		return ret;
	ret = ofnode_read_u32_index(dev_ofnode(dev), "mdata-offsets", 1,
				    &mtd_priv->sec_offset);
	if (ret)
		return ret;

	return 0;
}

static int fwu_mdata_mtd_probe(struct udevice *dev)
{
	/* Ensure the metadata can be read. */
	return fwu_mtd_mdata_check(dev);
}

static struct fwu_mdata_ops fwu_mtd_ops = {
	.mdata_check = fwu_mtd_mdata_check,
	.get_mdata = fwu_mtd_get_mdata,
	.update_mdata = fwu_mtd_update_mdata,
};

static const struct udevice_id fwu_mdata_ids[] = {
	{ .compatible = "u-boot,fwu-mdata-mtd" },
	{ }
};

U_BOOT_DRIVER(fwu_mdata_mtd) = {
	.name		= "fwu-mdata-mtd",
	.id		= UCLASS_FWU_MDATA,
	.of_match	= fwu_mdata_ids,
	.ops		= &fwu_mtd_ops,
	.probe		= fwu_mdata_mtd_probe,
	.of_to_plat	= fwu_mdata_mtd_of_to_plat,
	.priv_auto	= sizeof(struct fwu_mdata_mtd_priv),
};

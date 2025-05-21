// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_FWU_MDATA

#include <fwu.h>
#include <fwu_mdata.h>
#include <memalign.h>
#include <mtd.h>

#include <linux/errno.h>
#include <linux/types.h>

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
	struct mtd_oob_ops io_op = {};
	u64 lock_len;
	size_t len;
	void *buf;
	int ret;

	if (!mtd_is_aligned_with_block_size(mtd, offs)) {
		log_err("Offset unaligned with a block (0x%x)\n", mtd->erasesize);
		return -EINVAL;
	}

	/* This will expand erase size to align with the block size */
	lock_len = round_up(size, mtd->erasesize);

	ret = mtd_unlock(mtd, offs, lock_len);
	if (ret && ret != -EOPNOTSUPP)
		return ret;

	if (op == FWU_MTD_WRITE) {
		struct erase_info erase_op = {};

		erase_op.mtd = mtd;
		erase_op.addr = offs;
		erase_op.len = lock_len;
		erase_op.scrub = 0;

		ret = mtd_erase(mtd, &erase_op);
		if (ret)
			goto lock;
	}

	/* Also, expand the write size to align with the write size */
	len = round_up(size, mtd->writesize);

	buf = memalign(ARCH_DMA_MINALIGN, len);
	if (!buf) {
		ret = -ENOMEM;
		goto lock;
	}
	memset(buf, 0xff, len);

	io_op.mode = MTD_OPS_AUTO_OOB;
	io_op.len = len;
	io_op.datbuf = buf;

	if (op == FWU_MTD_WRITE) {
		memcpy(buf, data, size);
		ret = mtd_write_oob(mtd, offs, &io_op);
	} else {
		ret = mtd_read_oob(mtd, offs, &io_op);
		if (!ret)
			memcpy(data, buf, size);
	}
	free(buf);

lock:
	mtd_lock(mtd, offs, lock_len);

	return ret;
}

static int fwu_mtd_read_mdata(struct udevice *dev, struct fwu_mdata *mdata,
			      bool primary, u32 size)
{
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(dev);
	struct mtd_info *mtd = mtd_priv->mtd;
	u32 offs = primary ? mtd_priv->pri_offset : mtd_priv->sec_offset;

	return mtd_io_data(mtd, offs, size, mdata, FWU_MTD_READ);
}

static int fwu_mtd_write_mdata(struct udevice *dev, struct fwu_mdata *mdata,
			       bool primary, u32 size)
{
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(dev);
	struct mtd_info *mtd = mtd_priv->mtd;
	u32 offs = primary ? mtd_priv->pri_offset : mtd_priv->sec_offset;

	return mtd_io_data(mtd, offs, size, mdata, FWU_MTD_WRITE);
}

static int flash_partition_offset(struct udevice *dev, const char *part_name, fdt_addr_t *offset)
{
	ofnode node, parts_node;
	fdt_addr_t size = 0;

	parts_node = ofnode_by_compatible(dev_ofnode(dev), "fixed-partitions");
	node = ofnode_by_prop_value(parts_node, "label", part_name, strlen(part_name) + 1);
	if (!ofnode_valid(node)) {
		log_err("Warning: Failed to find partition by label <%s>\n", part_name);
		return -ENOENT;
	}

	*offset = ofnode_get_addr_size_index_notrans(node, 0, &size);

	return (int)size;
}

static int get_fwu_mdata_dev(struct udevice *dev)
{
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(dev);
	const fdt32_t *phandle_p = NULL;
	struct udevice *mtd_dev;
	struct mtd_info *mtd;
	const char *label;
	fdt_addr_t offset;
	int ret, size;
	u32 phandle;

	/* Find the FWU mdata storage device */
	phandle_p = ofnode_get_property(dev_ofnode(dev),
					"fwu-mdata-store", &size);
	if (!phandle_p) {
		log_err("FWU meta data store not defined in device-tree\n");
		return -ENOENT;
	}

	phandle = fdt32_to_cpu(*phandle_p);

	ret = device_get_global_by_ofnode(ofnode_get_by_phandle(phandle),
					  &mtd_dev);
	if (ret) {
		log_err("FWU: failed to get mtd device\n");
		return ret;
	}

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
		return -ENODEV;
	}

	/* Get the offset of primary and secondary mdata */
	ret = ofnode_read_string_index(dev_ofnode(dev), "mdata-parts", 0, &label);
	if (ret)
		return ret;
	strncpy(mtd_priv->pri_label, label, 50);

	ret = flash_partition_offset(mtd_dev, mtd_priv->pri_label, &offset);
	if (ret <= 0)
		return ret;
	mtd_priv->pri_offset = offset;

	ret = ofnode_read_string_index(dev_ofnode(dev), "mdata-parts", 1, &label);
	if (ret)
		return ret;
	strncpy(mtd_priv->sec_label, label, 50);

	ret = flash_partition_offset(mtd_dev, mtd_priv->sec_label, &offset);
	if (ret <= 0)
		return ret;
	mtd_priv->sec_offset = offset;

	return 0;
}

static int fwu_mtd_image_info_populate(struct udevice *dev, u8 nbanks,
				       u16 nimages)
{
	struct fwu_mtd_image_info *mtd_images;
	struct fwu_mdata_mtd_priv *mtd_priv = dev_get_priv(dev);
	struct udevice *mtd_dev = mtd_priv->mtd->dev;
	fdt_addr_t offset;
	ofnode bank;
	int off_img;
	u32 total_images;

	total_images = nbanks * nimages;
	mtd_priv->fwu_mtd_images = malloc(sizeof(struct fwu_mtd_image_info) *
					  total_images);
	if (!mtd_priv->fwu_mtd_images)
		return -ENOMEM;

	off_img = 0;
	mtd_images = mtd_priv->fwu_mtd_images;
	ofnode_for_each_subnode(bank, dev_ofnode(dev)) {
		int bank_num, bank_offset, bank_size;
		const char *bank_name;
		ofnode image;

		ofnode_read_u32(bank, "id", &bank_num);
		bank_name = ofnode_read_string(bank, "label");
		bank_size = flash_partition_offset(mtd_dev, bank_name, &offset);
		if (bank_size <= 0)
			return bank_size;
		bank_offset = offset;
		log_debug("Bank%d: %s [0x%x - 0x%x]\n",
			  bank_num, bank_name, bank_offset, bank_offset + bank_size);

		ofnode_for_each_subnode(image, bank) {
			int image_num, image_offset, image_size;
			const char *uuid;

			if (off_img == total_images) {
				log_err("DT provides more images than configured!\n");
				break;
			}

			uuid = ofnode_read_string(image, "uuid");
			ofnode_read_u32(image, "id", &image_num);
			ofnode_read_u32(image, "offset", &image_offset);
			ofnode_read_u32(image, "size", &image_size);

			mtd_images[off_img].start = bank_offset + image_offset;
			mtd_images[off_img].size = image_size;
			mtd_images[off_img].bank_num = bank_num;
			mtd_images[off_img].image_num = image_num;
			strcpy(mtd_images[off_img].uuidbuf, uuid);
			log_debug("\tImage%d: %s @0x%x\n\n",
				  image_num, uuid, bank_offset + image_offset);
			off_img++;
		}
	}

	return 0;
}

static int fwu_mdata_mtd_probe(struct udevice *dev)
{
	u8 nbanks;
	u16 nimages;
	int ret;

	ret = get_fwu_mdata_dev(dev);
	if (ret)
		return ret;

	nbanks = CONFIG_FWU_NUM_BANKS;
	nimages = CONFIG_FWU_NUM_IMAGES_PER_BANK;
	ret = fwu_mtd_image_info_populate(dev, nbanks, nimages);
	if (ret)
		return ret;

	return 0;
}

static struct fwu_mdata_ops fwu_mtd_ops = {
	.read_mdata = fwu_mtd_read_mdata,
	.write_mdata = fwu_mtd_write_mdata,
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
	.priv_auto	= sizeof(struct fwu_mdata_mtd_priv),
};

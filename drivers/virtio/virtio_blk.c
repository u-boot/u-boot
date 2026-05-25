// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#define LOG_CATEGORY UCLASS_VIRTIO

#include <blk.h>
#include <dm.h>
#include <part.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <linux/log2.h>
#include <linux/err.h>
#include "virtio_blk.h"
#include <malloc.h>

/**
 * struct virtio_blk_priv - private data for virtio block device
 */
struct virtio_blk_priv {
	/** @virtqueue - virtqueue to process */
	struct virtqueue *vq;
	/** @blksz_shift - log2 of block size divided by 512 */
	u32 blksz_shift;
	/** @size_max - maximum segment size */
	u32 size_max;
	/** @seg_max - maximum segment count */
	u32 seg_max;
};

static const u32 feature[] = {
	VIRTIO_BLK_F_BLK_SIZE,
	VIRTIO_BLK_F_SIZE_MAX,
	VIRTIO_BLK_F_SEG_MAX,
	VIRTIO_BLK_F_WRITE_ZEROES
};

static void virtio_blk_init_header_sg(struct udevice *dev, u64 sector, u32 type,
				      struct virtio_blk_outhdr *out_hdr, struct virtio_sg *sg)
{
	const bool sector_is_needed = type == VIRTIO_BLK_T_IN ||
				      type == VIRTIO_BLK_T_OUT;

	out_hdr->type = cpu_to_virtio32(dev, type);
	out_hdr->sector = cpu_to_virtio64(dev, sector_is_needed ? sector : 0);

	sg->addr = out_hdr;
	sg->length = sizeof(*out_hdr);
}

static void virtio_blk_init_write_zeroes_sg(struct udevice *dev, u64 sector, lbaint_t blkcnt,
					    struct virtio_blk_discard_write_zeroes *wz,
					    struct virtio_sg *sg)
{
	wz->sector = cpu_to_virtio64(dev, sector);
	wz->num_sectors = cpu_to_virtio32(dev, blkcnt);
	wz->flags = cpu_to_virtio32(dev, 0);

	sg->addr = wz;
	sg->length = sizeof(*wz);
}

static void virtio_blk_init_status_sg(u8 *status, struct virtio_sg *sg)
{
	sg->addr = status;
	sg->length = sizeof(*status);
}

static void virtio_blk_init_data_sg(void *buffer, lbaint_t blkcnt, struct virtio_sg *sg)
{
	sg->addr = buffer;
	sg->length = blkcnt * 512;
}

/*
 * Create, execute and wait for one single virtio request. On success the
 * transferred block count is returned and in the error case -EIO.
 */
static ulong virtio_blk_do_single_req(struct udevice *dev, u64 sector,
				      lbaint_t blkcnt, char *buffer, u32 type)
{
	struct virtio_blk_priv *priv = dev_get_priv(dev);
	/*
	* The virtio device may have constrains on the maximum segment size.
	* Calculate how many segments we need.
	*/
	u32 seg_cnt = (blkcnt * 512) / priv->size_max + 1;
	lbaint_t seg_sec_cnt = priv->size_max / 512;
	struct virtio_blk_outhdr out_hdr;
	struct virtio_blk_discard_write_zeroes wz_hdr;
	unsigned int num_out = 0, num_in = 0;
	struct virtio_sg **sgs;
	u8 status = VIRTIO_BLK_S_IOERR;
	int ret;
	u32 i;

	/*
	* +2 is header and status descriptor; seg_cnt is the number of data segments
	* required. Needs to be dynamically allocated.
	*/
	sgs = calloc(seg_cnt + 2, sizeof(struct virtio_sg *));
	if (!sgs)
		return -ENOMEM;

	for (i = 0; i < seg_cnt + 2; ++i) {
		sgs[i] = malloc(sizeof(struct virtio_sg));
		if (!sgs[i])
			goto err_free;
	}

	virtio_blk_init_header_sg(dev, sector, type, &out_hdr, sgs[num_out++]);

	switch (type) {
	case VIRTIO_BLK_T_IN:
	case VIRTIO_BLK_T_OUT: {
		i = 0;
		while (i < blkcnt) {
			u32 blk_per_seg = min(blkcnt - i, seg_sec_cnt);

			if (type & VIRTIO_BLK_T_OUT)
				virtio_blk_init_data_sg(buffer + i * 512, blk_per_seg,
							sgs[num_out++]);
			else
				virtio_blk_init_data_sg(buffer + i * 512, blk_per_seg,
							sgs[num_out + num_in++]);
			i += blk_per_seg;
		}
		break;
	}
	case VIRTIO_BLK_T_WRITE_ZEROES:
		virtio_blk_init_write_zeroes_sg(dev, sector, blkcnt, &wz_hdr,
						sgs[num_out++]);
		break;

	default:
		goto err_free;
	}

	virtio_blk_init_status_sg(&status, sgs[num_out + num_in++]);
	log_debug("dev=%s, active=%d, priv=%p, priv->vq=%p\n", dev->name,
		  device_active(dev), priv, priv->vq);

	ret = virtqueue_add(priv->vq, sgs, num_out, num_in);
	if (ret)
		goto err_free;

	virtqueue_kick(priv->vq);

	log_debug("wait...");
	while (!virtqueue_get_buf(priv->vq, NULL))
		;
	log_debug("done\n");

err_free:
	for (i = 0; i < seg_cnt + 2; ++i)
		free(sgs[i]);
	free(sgs);

	return status == VIRTIO_BLK_S_OK ? blkcnt : -EIO;
}

static ulong virtio_blk_do_req(struct udevice *dev, u64 sector,
			       lbaint_t blkcnt, char *buffer, u32 type)
{
	struct virtio_blk_priv *priv = dev_get_priv(dev);
	lbaint_t seg_sec_cnt = priv->size_max / 512;
	u32 i = 0;
	ulong ret;

	sector <<= priv->blksz_shift;
	blkcnt <<= priv->blksz_shift;

	/*
	* The virtio device may have constrains on the maximum segment count. So
	* send multiple virtio requests one after each other, if so.
	*/
	while (i < blkcnt) {
		u32 blk_per_sg = min(blkcnt - i, seg_sec_cnt * priv->seg_max);

		ret = virtio_blk_do_single_req(dev, sector + i, blk_per_sg,
					       buffer + i * 512, type);
		if (IS_ERR_VALUE(ret))
			return ret;
		i += blk_per_sg;
	}

	return blkcnt >> priv->blksz_shift;
}

static ulong virtio_blk_read(struct udevice *dev, lbaint_t start,
			     lbaint_t blkcnt, void *buffer)
{
	log_debug("read %s\n", dev->name);
	return virtio_blk_do_req(dev, start, blkcnt, buffer,
				 VIRTIO_BLK_T_IN);
}

static ulong virtio_blk_write(struct udevice *dev, lbaint_t start,
			      lbaint_t blkcnt, const void *buffer)
{
	return virtio_blk_do_req(dev, start, blkcnt, (void *)buffer,
				 VIRTIO_BLK_T_OUT);
}

static ulong virtio_blk_erase(struct udevice *dev, lbaint_t start,
			      lbaint_t blkcnt)
{
	if (!virtio_has_feature(dev, VIRTIO_BLK_F_WRITE_ZEROES))
		return -EOPNOTSUPP;

	return virtio_blk_do_req(dev, start, blkcnt, NULL, VIRTIO_BLK_T_WRITE_ZEROES);
}

static int virtio_blk_bind(struct udevice *dev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(dev->parent);
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	int devnum;

	desc->uclass_id = UCLASS_VIRTIO;
	/*
	 * Initialize the devnum to -ENODEV. This is to make sure that
	 * blk_next_free_devnum() works as expected, since the default
	 * value 0 is a valid devnum.
	 */
	desc->devnum = -ENODEV;
	devnum = blk_next_free_devnum(UCLASS_VIRTIO);
	if (devnum < 0)
		return devnum;
	desc->devnum = devnum;
	desc->part_type = PART_TYPE_UNKNOWN;

	if (uc_priv->vendor == VIRTIO_VENDOR_QEMU)
		strcpy(desc->vendor, "QEMU");
	else
		sprintf(desc->vendor, "%08x", uc_priv->vendor);
	desc->bdev = dev;

	/* Indicate what driver features we support */
	virtio_driver_features_init(uc_priv, feature, ARRAY_SIZE(feature),
				    NULL, 0);

	return 0;
}

static int virtio_blk_probe(struct udevice *dev)
{
	struct virtio_blk_priv *priv = dev_get_priv(dev);
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	u64 cap;
	int ret;
	u32 blk_size;

	ret = virtio_find_vqs(dev, 1, &priv->vq);
	if (ret)
		return ret;

	virtio_cread(dev, struct virtio_blk_config, capacity, &cap);
	desc->lba = cap;
	if (virtio_has_feature(dev, VIRTIO_BLK_F_BLK_SIZE)) {
		virtio_cread(dev, struct virtio_blk_config, blk_size, &blk_size);
		desc->blksz = blk_size;
		if (!is_power_of_2(blk_size) || desc->blksz < 512)
			return -EIO;
	} else {
		desc->blksz = 512;
	}
	desc->log2blksz = LOG2(desc->blksz);
	priv->blksz_shift = desc->log2blksz - 9;
	desc->lba >>= priv->blksz_shift;

	if (virtio_has_feature(dev, VIRTIO_BLK_F_SIZE_MAX))
		virtio_cread(dev, struct virtio_blk_config, size_max, &priv->size_max);
	else
		priv->size_max = -1U;
	if (virtio_has_feature(dev, VIRTIO_BLK_F_SEG_MAX))
		virtio_cread(dev, struct virtio_blk_config, seg_max, &priv->seg_max);
	else
		priv->seg_max = -1U;

	return 0;
}

static const struct blk_ops virtio_blk_ops = {
	.read	= virtio_blk_read,
	.write	= virtio_blk_write,
	.erase	= virtio_blk_erase,
};

U_BOOT_DRIVER(virtio_blk) = {
	.name	= VIRTIO_BLK_DRV_NAME,
	.id	= UCLASS_BLK,
	.ops	= &virtio_blk_ops,
	.bind	= virtio_blk_bind,
	.probe	= virtio_blk_probe,
	.remove	= virtio_reset,
	.priv_auto	= sizeof(struct virtio_blk_priv),
	.flags	= DM_FLAG_ACTIVE_DMA,
};

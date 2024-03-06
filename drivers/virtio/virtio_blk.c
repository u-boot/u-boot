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
#include "virtio_blk.h"

struct virtio_blk_priv {
	struct virtqueue *vq;
};

static const u32 feature[] = {
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

static ulong virtio_blk_do_req(struct udevice *dev, u64 sector,
			       lbaint_t blkcnt, void *buffer, u32 type)
{
	struct virtio_blk_priv *priv = dev_get_priv(dev);
	struct virtio_blk_outhdr out_hdr;
	struct virtio_blk_discard_write_zeroes wz_hdr;
	unsigned int num_out = 0, num_in = 0;
	struct virtio_sg hdr_sg, wz_sg, data_sg, status_sg;
	struct virtio_sg *sgs[3];
	u8 status;
	int ret;

	virtio_blk_init_header_sg(dev, sector, type, &out_hdr, &hdr_sg);
	sgs[num_out++] = &hdr_sg;

	switch (type) {
	case VIRTIO_BLK_T_IN:
	case VIRTIO_BLK_T_OUT:
		virtio_blk_init_data_sg(buffer, blkcnt, &data_sg);
		if (type & VIRTIO_BLK_T_OUT)
			sgs[num_out++] = &data_sg;
		else
			sgs[num_out + num_in++] = &data_sg;
		break;

	case VIRTIO_BLK_T_WRITE_ZEROES:
		virtio_blk_init_write_zeroes_sg(dev, sector, blkcnt, &wz_hdr, &wz_sg);
		sgs[num_out++] = &wz_sg;
		break;

	default:
		return -EINVAL;
	}

	virtio_blk_init_status_sg(&status, &status_sg);
	sgs[num_out + num_in++] = &status_sg;
	log_debug("dev=%s, active=%d, priv=%p, priv->vq=%p\n", dev->name,
		  device_active(dev), priv, priv->vq);

	ret = virtqueue_add(priv->vq, sgs, num_out, num_in);
	if (ret)
		return ret;

	virtqueue_kick(priv->vq);

	log_debug("wait...");
	while (!virtqueue_get_buf(priv->vq, NULL))
		;
	log_debug("done\n");

	return status == VIRTIO_BLK_S_OK ? blkcnt : -EIO;
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
	/*
	 * virtio mmio transport supplies string identification for us,
	 * while pci trnasport uses a 2-byte subvendor value.
	 */
	if (uc_priv->vendor >> 16)
		sprintf(desc->vendor, "%s", (char *)&uc_priv->vendor);
	else
		sprintf(desc->vendor, "%04x", uc_priv->vendor);
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

	ret = virtio_find_vqs(dev, 1, &priv->vq);
	if (ret)
		return ret;

	desc->blksz = 512;
	desc->log2blksz = 9;
	virtio_cread(dev, struct virtio_blk_config, capacity, &cap);
	desc->lba = cap;

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

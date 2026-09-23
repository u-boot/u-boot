// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#define LOG_CATEGORY UCLASS_VIRTIO

#include <9p.h>
#include <dm.h>
#include <time.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <virtio_types.h>
#include <linux/errno.h>

#define VIRTIO_9P_TIMEOUT_MS 5000

struct virtio_9p_priv {
	struct virtqueue *vq;
	char mount_tag[32];
};

static const u32 feature[] = {
	VIRTIO_9P_MOUNT_TAG,
};

static int virtio_9p_request(struct udevice *dev, void *tx, int tx_len, void *rx, int rx_len)
{
	struct virtio_9p_priv *priv = dev_get_priv(dev);
	struct virtio_sg sg[2];
	struct virtio_sg *sgs[2];
	int len, ret;
	ulong start;

	sg[0].addr = tx;
	sg[0].length = tx_len;
	sgs[0] = &sg[0];

	sg[1].addr = rx;
	sg[1].length = rx_len;
	sgs[1] = &sg[1];

	ret = virtqueue_add(priv->vq, sgs, 1, 1);
	if (ret)
		return ret;

	virtqueue_kick(priv->vq);

	start = get_timer(0);
	while (!virtqueue_get_buf(priv->vq, &len)) {
		if (get_timer(start) > VIRTIO_9P_TIMEOUT_MS)
			return -ETIMEDOUT;
	}

	if (len < (int)sizeof(struct p9_header))
		return -EIO;

	return len;
}

static const char *virtio_9p_get_mount_tag(struct udevice *dev)
{
	struct virtio_9p_priv *priv = dev_get_priv(dev);

	return priv->mount_tag;
}

static const struct dm_p9_ops virtio_9p_ops = {
	.request = virtio_9p_request,
	.get_mount_tag = virtio_9p_get_mount_tag,
};

static int virtio_9p_bind(struct udevice *dev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(dev->parent);

	virtio_driver_features_init(uc_priv, feature, ARRAY_SIZE(feature), NULL, 0);
	return 0;
}

static int virtio_9p_probe(struct udevice *dev)
{
	struct virtio_9p_priv *priv = dev_get_priv(dev);
	int ret;

	ret = virtio_find_vqs(dev, 1, &priv->vq);
	if (ret)
		return ret;

	if (virtio_has_feature(dev, VIRTIO_9P_MOUNT_TAG)) {
		u16 tag_len;

		virtio_cread(dev, struct virtio_9p_config, tag_len, &tag_len);
		if (tag_len > 0 && tag_len < sizeof(priv->mount_tag)) {
			virtio_cread_bytes(dev, offsetof(struct virtio_9p_config, tag),
					   priv->mount_tag, tag_len);
			priv->mount_tag[tag_len] = '\0';
			log_debug("virtio-9p: mount_tag=%s\n", priv->mount_tag);
		}
	}

	return 0;
}

static int virtio_9p_remove(struct udevice *dev)
{
	virtio_del_vqs(dev);
	return virtio_reset(dev);
}

U_BOOT_DRIVER(virtio_9p) = {
	.name       = VIRTIO_9P_DRV_NAME,
	.id         = UCLASS_9P,
	.ops        = &virtio_9p_ops,
	.bind       = virtio_9p_bind,
	.probe      = virtio_9p_probe,
	.remove     = virtio_9p_remove,
	.priv_auto  = sizeof(struct virtio_9p_priv),
	.flags      = DM_FLAG_ACTIVE_DMA,
};

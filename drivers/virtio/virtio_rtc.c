// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026, Hung-Chun Tseng <alan.tseng.cs@gmail.com>
 *
 * Driver for the virtio RTC device (virtio device ID 17). The device
 * exposes one or more read-only clocks through a request virtqueue; this
 * driver reads a UTC clock, preferring one which smears leap seconds, and
 * reports it through the RTC uclass.
 */

#include <div64.h>
#include <dm.h>
#include <log.h>
#include <rtc.h>
#include <time.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <linux/types.h>

#define VIRTIO_RTC_TIMEOUT_MS		1000

#define VIRTIO_RTC_REQ_READ		0x0001
#define VIRTIO_RTC_REQ_CFG		0x1000
#define VIRTIO_RTC_REQ_CLOCK_CAP	0x1001

#define VIRTIO_RTC_S_OK			0

#define VIRTIO_RTC_CLOCK_UTC			0
#define VIRTIO_RTC_CLOCK_UTC_SMEARED		3
#define VIRTIO_RTC_CLOCK_UTC_MAYBE_SMEARED	4

struct virtio_rtc_req_head {
	__le16 msg_type;
	u8 reserved[6];
};

struct virtio_rtc_resp_head {
	u8 status;
	u8 reserved[7];
};

/* Request for VIRTIO_RTC_REQ_READ and VIRTIO_RTC_REQ_CLOCK_CAP */
struct virtio_rtc_req_clock {
	struct virtio_rtc_req_head head;
	__le16 clock_id;
	u8 reserved[6];
};

struct virtio_rtc_resp_read {
	struct virtio_rtc_resp_head head;
	__le64 clock_reading;
};

struct virtio_rtc_resp_cfg {
	struct virtio_rtc_resp_head head;
	__le16 num_clocks;
	u8 reserved[6];
};

struct virtio_rtc_resp_clock_cap {
	struct virtio_rtc_resp_head head;
	u8 type;
	u8 leap_second_smearing;
	u8 flags;
	u8 reserved[5];
};

struct virtio_rtc_priv {
	struct virtqueue *vq;
	u16 clock_id;
	bool clock_found;
	bool no_utc_clock;
};

static int virtio_rtc_xfer(struct udevice *dev, void *req, size_t req_len,
			   void *resp, size_t resp_len)
{
	struct virtio_rtc_priv *priv = dev_get_priv(dev);
	struct virtio_rtc_resp_head *head = resp;
	struct virtio_sg req_sg = { req, req_len };
	struct virtio_sg resp_sg = { resp, resp_len };
	struct virtio_sg *sgs[] = { &req_sg, &resp_sg };
	unsigned int len;
	ulong start;
	int ret;

	ret = virtqueue_add(priv->vq, sgs, 1, 1);
	if (ret)
		return ret;

	virtqueue_kick(priv->vq);

	start = get_timer(0);
	while (!virtqueue_get_buf(priv->vq, &len)) {
		if (get_timer(start) > VIRTIO_RTC_TIMEOUT_MS)
			return -ETIMEDOUT;
	}

	if (len < sizeof(*head) || head->status != VIRTIO_RTC_S_OK)
		return -EIO;

	if (len < resp_len)
		return -EPROTO;

	return 0;
}

static bool virtio_rtc_is_utc(u8 type)
{
	return type == VIRTIO_RTC_CLOCK_UTC ||
	       type == VIRTIO_RTC_CLOCK_UTC_SMEARED ||
	       type == VIRTIO_RTC_CLOCK_UTC_MAYBE_SMEARED;
}

static int virtio_rtc_find_utc_clock(struct udevice *dev)
{
	struct virtio_rtc_priv *priv = dev_get_priv(dev);
	struct virtio_rtc_req_head cfg_req = {
		.msg_type = cpu_to_le16(VIRTIO_RTC_REQ_CFG),
	};
	struct virtio_rtc_resp_cfg cfg = {};
	u16 num_clocks, id, utc_id = 0;
	bool utc_found = false;
	int ret;

	ret = virtio_rtc_xfer(dev, &cfg_req, sizeof(cfg_req), &cfg, sizeof(cfg));
	if (ret)
		return ret;

	num_clocks = le16_to_cpu(cfg.num_clocks);
	for (id = 0; id < num_clocks; id++) {
		struct virtio_rtc_req_clock req = {
			.head.msg_type = cpu_to_le16(VIRTIO_RTC_REQ_CLOCK_CAP),
			.clock_id = cpu_to_le16(id),
		};
		struct virtio_rtc_resp_clock_cap cap = {};

		ret = virtio_rtc_xfer(dev, &req, sizeof(req), &cap, sizeof(cap));
		if (ret)
			return ret;

		/*
		 * Like Linux, prefer a clock which smears leap seconds, but fall
		 * back to another UTC clock, which is all that QEMU v11.1 has.
		 */
		if (cap.type == VIRTIO_RTC_CLOCK_UTC_SMEARED) {
			utc_id = id;
			utc_found = true;
			break;
		}

		if (!utc_found && virtio_rtc_is_utc(cap.type)) {
			utc_id = id;
			utc_found = true;
		}
	}

	/* The set of clocks is fixed, so remember that none of them is UTC */
	if (!utc_found) {
		priv->no_utc_clock = true;
		return -ENODEV;
	}

	priv->clock_id = utc_id;
	priv->clock_found = true;

	return 0;
}

static int virtio_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	struct virtio_rtc_priv *priv = dev_get_priv(dev);
	struct virtio_rtc_req_clock req = {
		.head.msg_type = cpu_to_le16(VIRTIO_RTC_REQ_READ),
	};
	struct virtio_rtc_resp_read resp = {};
	u64 now;
	int ret;

	/*
	 * The virtqueue must not be used before DRIVER_OK, which the virtio
	 * uclass sets only after probe(), so look for the clock on first use.
	 */
	if (priv->no_utc_clock)
		return -ENODEV;

	if (!priv->clock_found) {
		ret = virtio_rtc_find_utc_clock(dev);
		if (ret)
			return ret;
	}

	req.clock_id = cpu_to_le16(priv->clock_id);

	ret = virtio_rtc_xfer(dev, &req, sizeof(req), &resp, sizeof(resp));
	if (ret)
		return ret;

	now = le64_to_cpu(resp.clock_reading);
	do_div(now, 1000000000U);
	rtc_to_tm(now, time);

	return 0;
}

static int virtio_rtc_bind(struct udevice *dev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(dev->parent);

	/* The alarm feature is not supported */
	virtio_driver_features_init(uc_priv, NULL, 0, NULL, 0);

	return 0;
}

static int virtio_rtc_probe(struct udevice *dev)
{
	struct virtio_rtc_priv *priv = dev_get_priv(dev);
	int ret;

	ret = virtio_find_vqs(dev, 1, &priv->vq);
	if (ret < 0) {
		debug("%s: virtio_find_vqs failed\n", __func__);
		return ret;
	}

	return 0;
}

static const struct rtc_ops virtio_rtc_ops = {
	.get	= virtio_rtc_get,
};

U_BOOT_DRIVER(virtio_rtc) = {
	.name	= VIRTIO_RTC_DRV_NAME,
	.id	= UCLASS_RTC,
	.bind	= virtio_rtc_bind,
	.probe	= virtio_rtc_probe,
	.remove	= virtio_reset,
	.ops	= &virtio_rtc_ops,
	.priv_auto	= sizeof(struct virtio_rtc_priv),
	.flags	= DM_FLAG_ACTIVE_DMA,
};

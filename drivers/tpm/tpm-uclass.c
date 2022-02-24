// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_TPM

#include <common.h>
#include <dm.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/unaligned/be_byteshift.h>
#include <tpm_api.h>
#include <tpm-v1.h>
#include <tpm-v2.h>
#include "tpm_internal.h"

#include <dm/lists.h>

#define TPM_RNG1_DRV_NAME	"tpm1-rng"
#define TPM_RNG2_DRV_NAME	"tpm2-rng"

bool tpm_is_v1(struct udevice *dev)
{
	return IS_ENABLED(CONFIG_TPM_V1) && tpm_get_version(dev) == TPM_V1;
}

bool tpm_is_v2(struct udevice *dev)
{
	return IS_ENABLED(CONFIG_TPM_V2) && tpm_get_version(dev) == TPM_V2;
}

int tpm_open(struct udevice *dev)
{
	struct tpm_ops *ops = tpm_get_ops(dev);

	if (!ops->open)
		return -ENOSYS;

	return ops->open(dev);
}

int tpm_close(struct udevice *dev)
{
	struct tpm_ops *ops = tpm_get_ops(dev);

	if (!ops->close)
		return -ENOSYS;

	return ops->close(dev);
}

int tpm_get_desc(struct udevice *dev, char *buf, int size)
{
	struct tpm_ops *ops = tpm_get_ops(dev);

	if (!ops->get_desc)
		return -ENOSYS;

	return ops->get_desc(dev, buf, size);
}

/* Returns max number of milliseconds to wait */
static ulong tpm_tis_i2c_calc_ordinal_duration(struct tpm_chip_priv *priv,
					       u32 ordinal)
{
	int duration_idx = TPM_UNDEFINED;
	int duration = 0;

	if (ordinal < TPM_MAX_ORDINAL) {
		duration_idx = tpm_ordinal_duration[ordinal];
	} else if ((ordinal & TPM_PROTECTED_ORDINAL_MASK) <
			TPM_MAX_PROTECTED_ORDINAL) {
		duration_idx = tpm_protected_ordinal_duration[
				ordinal & TPM_PROTECTED_ORDINAL_MASK];
	}

	if (duration_idx != TPM_UNDEFINED)
		duration = priv->duration_ms[duration_idx];

	if (duration <= 0)
		return 2 * 60 * 1000; /* Two minutes timeout */
	else
		return duration;
}

int tpm_xfer(struct udevice *dev, const uint8_t *sendbuf, size_t send_size,
	uint8_t *recvbuf, size_t *recv_size)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	struct tpm_ops *ops = tpm_get_ops(dev);
	ulong start, stop;
	uint count, ordinal;
	int ret, ret2 = 0;

	if (ops->xfer)
		return ops->xfer(dev, sendbuf, send_size, recvbuf, recv_size);

	if (!ops->send || !ops->recv)
		return -ENOSYS;

	/* switch endianess: big->little */
	count = get_unaligned_be32(sendbuf + TPM_CMD_COUNT_BYTE);
	ordinal = get_unaligned_be32(sendbuf + TPM_CMD_ORDINAL_BYTE);

	if (count == 0) {
		log_debug("no data\n");
		return -ENODATA;
	}
	if (count > send_size) {
		log_debug("invalid count value %x %zx\n", count, send_size);
		return -E2BIG;
	}

	log_debug("%s: Calling send\n", __func__);
	ret = ops->send(dev, sendbuf, send_size);
	if (ret < 0)
		return ret;

	start = get_timer(0);
	stop = tpm_tis_i2c_calc_ordinal_duration(priv, ordinal);
	do {
		ret = ops->recv(dev, priv->buf, sizeof(priv->buf));
		if (ret >= 0) {
			if (ret > *recv_size)
				return -ENOSPC;
			memcpy(recvbuf, priv->buf, ret);
			*recv_size = ret;
			ret = 0;
			break;
		} else if (ret != -EAGAIN) {
			return ret;
		}

		mdelay(priv->retry_time_ms);
		if (get_timer(start) > stop) {
			ret = -ETIMEDOUT;
			break;
		}
	} while (ret);

	if (ret) {
		if (ops->cleanup) {
			ret2 = ops->cleanup(dev);
			if (ret2)
				return log_msg_ret("cleanup", ret2);
		}
		return log_msg_ret("xfer", ret);
	}

	return 0;
}

#if IS_ENABLED(CONFIG_TPM)
static int tpm_uclass_post_probe(struct udevice *dev)
{
	int ret;
	const char *drv = tpm_is_v1(dev) ?
		TPM_RNG1_DRV_NAME : TPM_RNG2_DRV_NAME;
	struct udevice *child;

	ret = device_bind_driver(dev, drv, "tpm-rng0", &child);
	if (ret == -ENOENT) {
		log_err("No driver configured for tpm-rng device\n");
		return 0;
	}

	if (ret) {
		log_err("Unable to bind rng driver with the tpm-rng device\n");
		return ret;
	}

	return 0;
}

static int tpm_uclass_child_pre_probe(struct udevice *dev)
{
	int ret;

	ret = tpm_open(dev->parent);
	if (ret == -EBUSY) {
		log_info("TPM device already opened\n");
	} else if (ret) {
		log_err("Unable to open TPM device\n");
		return ret;
	}

	ret = tpm_startup(dev->parent, TPM_ST_CLEAR);
	if (ret)
		log_err("Unable to start TPM device\n");

	return ret;
}
#endif /* CONFIG_TPM */

UCLASS_DRIVER(tpm) = {
	.id			= UCLASS_TPM,
	.name			= "tpm",
	.flags			= DM_UC_FLAG_SEQ_ALIAS,
#if CONFIG_IS_ENABLED(OF_REAL)
	.post_bind		= dm_scan_fdt_dev,
#endif
#if IS_ENABLED(CONFIG_TPM)
	.post_probe		= tpm_uclass_post_probe,
	.child_pre_probe	= tpm_uclass_child_pre_probe,
#endif
	.per_device_auto	= sizeof(struct tpm_chip_priv),
};

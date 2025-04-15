// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (C) 2019-2022 Linaro Limited.
 */

#define LOG_CATEGORY UCLASS_SCMI_AGENT

#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <scmi_agent.h>
#include <asm/cache.h>
#include <asm/system.h>
#include <dm/ofnode.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/ioport.h>

#include "smt.h"

static void scmi_smt_enable_intr(struct scmi_smt *smt, bool enable)
{
	struct scmi_smt_header *hdr = (void *)smt->buf;

	if (enable)
		hdr->flags |= SCMI_SHMEM_FLAG_INTR_ENABLED;
	else
		hdr->flags &= ~SCMI_SHMEM_FLAG_INTR_ENABLED;
}

/**
 * Get shared memory configuration defined by the referred DT phandle
 * Return with a errno compliant value.
 */
int scmi_dt_get_smt_buffer(struct udevice *dev, struct scmi_smt *smt)
{
	int ret;
	struct ofnode_phandle_args args;
	struct resource resource;

	ret = dev_read_phandle_with_args(dev, "shmem", NULL, 0, 0, &args);
	if (ret)
		return ret;

	ret = ofnode_read_resource(args.node, 0, &resource);
	if (ret)
		return ret;

	smt->size = resource_size(&resource);
	if (smt->size < sizeof(struct scmi_smt_header)) {
		dev_err(dev, "Shared memory buffer too small\n");
		return -EINVAL;
	}

	smt->buf = devm_ioremap(dev, resource.start, smt->size);
	if (!smt->buf)
		return -ENOMEM;

	if (device_is_compatible(dev, "arm,scmi") && ofnode_has_property(dev_ofnode(dev), "mboxes"))
		scmi_smt_enable_intr(smt, true);

#ifdef CONFIG_ARM
	if (dcache_status())
		mmu_set_region_dcache_behaviour(ALIGN_DOWN((uintptr_t)smt->buf, MMU_SECTION_SIZE),
						ALIGN(smt->size, MMU_SECTION_SIZE),
						DCACHE_OFF);

#endif

	return 0;
}

/**
 * Write SCMI message @msg into a SMT shared buffer @smt.
 * Return 0 on success and with a negative errno in case of error.
 */
int scmi_write_msg_to_smt(struct udevice *dev, struct scmi_smt *smt,
			  struct scmi_msg *msg)
{
	struct scmi_smt_header *hdr = (void *)smt->buf;

	if ((!msg->in_msg && msg->in_msg_sz) ||
	    (!msg->out_msg && msg->out_msg_sz))
		return -EINVAL;

	if (!(hdr->channel_status & SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE)) {
		dev_dbg(dev, "Channel busy\n");
		return -EBUSY;
	}

	if (smt->size < (sizeof(*hdr) + msg->in_msg_sz) ||
	    smt->size < (sizeof(*hdr) + msg->out_msg_sz)) {
		dev_dbg(dev, "Buffer too small\n");
		return -ETOOSMALL;
	}

	/* Load message in shared memory */
	hdr->channel_status &= ~SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE;
	hdr->length = msg->in_msg_sz + sizeof(hdr->msg_header);
	hdr->msg_header = SMT_HEADER_TOKEN(0) |
			  SMT_HEADER_MESSAGE_TYPE(0) |
			  SMT_HEADER_PROTOCOL_ID(msg->protocol_id) |
			  SMT_HEADER_MESSAGE_ID(msg->message_id);

	memcpy_toio(hdr->msg_payload, msg->in_msg, msg->in_msg_sz);

	return 0;
}

/**
 * Read SCMI message from a SMT shared buffer @smt and copy it into @msg.
 * Return 0 on success and with a negative errno in case of error.
 */
int scmi_read_resp_from_smt(struct udevice *dev, struct scmi_smt *smt,
			    struct scmi_msg *msg)
{
	struct scmi_smt_header *hdr = (void *)smt->buf;

	if (!(hdr->channel_status & SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE)) {
		dev_err(dev, "Channel unexpectedly busy\n");
		return -EBUSY;
	}

	if (hdr->channel_status & SCMI_SHMEM_CHAN_STAT_CHANNEL_ERROR) {
		dev_err(dev, "Channel error reported, reset channel\n");
		return -ECOMM;
	}

	if (hdr->length > msg->out_msg_sz + sizeof(hdr->msg_header)) {
		dev_err(dev, "Buffer to small\n");
		return -ETOOSMALL;
	}

	/* Get the data */
	msg->out_msg_sz = hdr->length - sizeof(hdr->msg_header);
	memcpy_fromio(msg->out_msg, hdr->msg_payload, msg->out_msg_sz);

	return 0;
}

/**
 * Clear SMT flags in shared buffer to allow further message exchange
 */
void scmi_clear_smt_channel(struct scmi_smt *smt)
{
	struct scmi_smt_header *hdr = (void *)smt->buf;

	hdr->channel_status &= ~SCMI_SHMEM_CHAN_STAT_CHANNEL_ERROR;
}

/**
 * Write SCMI message @msg into a SMT_MSG shared buffer @smt.
 * Return 0 on success and with a negative errno in case of error.
 */
int scmi_msg_to_smt_msg(struct udevice *dev, struct scmi_smt *smt,
			struct scmi_msg *msg, size_t *buf_size)
{
	struct scmi_smt_msg_header *hdr = (void *)smt->buf;

	if ((!msg->in_msg && msg->in_msg_sz) ||
	    (!msg->out_msg && msg->out_msg_sz))
		return -EINVAL;

	if (smt->size < (sizeof(*hdr) + msg->in_msg_sz) ||
	    smt->size < (sizeof(*hdr) + msg->out_msg_sz)) {
		dev_dbg(dev, "Buffer too small\n");
		return -ETOOSMALL;
	}

	*buf_size = msg->in_msg_sz + sizeof(hdr->msg_header);

	hdr->msg_header = SMT_HEADER_TOKEN(0) |
			  SMT_HEADER_MESSAGE_TYPE(0) |
			  SMT_HEADER_PROTOCOL_ID(msg->protocol_id) |
			  SMT_HEADER_MESSAGE_ID(msg->message_id);

	memcpy(hdr->msg_payload, msg->in_msg, msg->in_msg_sz);

	return 0;
}

/**
 * Read SCMI message from a SMT shared buffer @smt and copy it into @msg.
 * Return 0 on success and with a negative errno in case of error.
 */
int scmi_msg_from_smt_msg(struct udevice *dev, struct scmi_smt *smt,
			  struct scmi_msg *msg, size_t buf_size)
{
	struct scmi_smt_msg_header *hdr = (void *)smt->buf;

	if (buf_size > msg->out_msg_sz + sizeof(hdr->msg_header)) {
		dev_err(dev, "Buffer to small\n");
		return -ETOOSMALL;
	}

	msg->out_msg_sz = buf_size - sizeof(hdr->msg_header);
	memcpy(msg->out_msg, hdr->msg_payload, msg->out_msg_sz);

	return 0;
}

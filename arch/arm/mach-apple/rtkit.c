// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Mark Kettenis <kettenis@openbsd.org>
 * (C) Copyright 2021 Copyright The Asahi Linux Contributors
 */

#include <common.h>
#include <mailbox.h>
#include <malloc.h>

#include <asm/arch/rtkit.h>
#include <linux/apple-mailbox.h>
#include <linux/bitfield.h>

#define APPLE_RTKIT_EP_MGMT 0
#define APPLE_RTKIT_EP_CRASHLOG	1
#define APPLE_RTKIT_EP_SYSLOG 2
#define APPLE_RTKIT_EP_DEBUG 3
#define APPLE_RTKIT_EP_IOREPORT 4

/* Messages for management endpoint. */
#define APPLE_RTKIT_MGMT_TYPE GENMASK(59, 52)

#define APPLE_RTKIT_MGMT_PWR_STATE GENMASK(15, 0)

#define APPLE_RTKIT_MGMT_HELLO 1
#define APPLE_RTKIT_MGMT_HELLO_REPLY 2
#define APPLE_RTKIT_MGMT_HELLO_MINVER GENMASK(15, 0)
#define APPLE_RTKIT_MGMT_HELLO_MAXVER GENMASK(31, 16)

#define APPLE_RTKIT_MGMT_STARTEP 5
#define APPLE_RTKIT_MGMT_STARTEP_EP GENMASK(39, 32)
#define APPLE_RTKIT_MGMT_STARTEP_FLAG BIT(1)

#define APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE 6
#define APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE_ACK 7

#define APPLE_RTKIT_MGMT_EPMAP 8
#define APPLE_RTKIT_MGMT_EPMAP_LAST BIT(51)
#define APPLE_RTKIT_MGMT_EPMAP_BASE GENMASK(34, 32)
#define APPLE_RTKIT_MGMT_EPMAP_BITMAP GENMASK(31, 0)

#define APPLE_RTKIT_MGMT_EPMAP_REPLY 8
#define APPLE_RTKIT_MGMT_EPMAP_REPLY_MORE BIT(0)

#define APPLE_RTKIT_MIN_SUPPORTED_VERSION 11
#define APPLE_RTKIT_MAX_SUPPORTED_VERSION 12

/* Messages for internal endpoints. */
#define APPLE_RTKIT_BUFFER_REQUEST 1
#define APPLE_RTKIT_BUFFER_REQUEST_SIZE GENMASK(51, 44)
#define APPLE_RTKIT_BUFFER_REQUEST_IOVA GENMASK(41, 0)

int apple_rtkit_init(struct mbox_chan *chan)
{
	struct apple_mbox_msg msg;
	int endpoints[256];
	int nendpoints = 0;
	int endpoint;
	int min_ver, max_ver, want_ver;
	int msgtype, pwrstate;
	u64 reply;
	u32 bitmap, base;
	int i, ret;

	/* Wakup the IOP. */
	msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE) |
		FIELD_PREP(APPLE_RTKIT_MGMT_PWR_STATE, APPLE_RTKIT_PWR_STATE_ON);
	msg.msg1 = APPLE_RTKIT_EP_MGMT;
	ret = mbox_send(chan, &msg);
	if (ret < 0)
		return ret;

	/* Wait for protocol version negotiation message. */
	ret = mbox_recv(chan, &msg, 10000);
	if (ret < 0)
		return ret;

	endpoint = msg.msg1;
	msgtype = FIELD_GET(APPLE_RTKIT_MGMT_TYPE, msg.msg0);
	if (endpoint != APPLE_RTKIT_EP_MGMT) {
		printf("%s: unexpected endpoint %d\n", __func__, endpoint);
		return -EINVAL;
	}
	if (msgtype != APPLE_RTKIT_MGMT_HELLO) {
		printf("%s: unexpected message type %d\n", __func__, msgtype);
		return -EINVAL;
	}

	min_ver = FIELD_GET(APPLE_RTKIT_MGMT_HELLO_MINVER, msg.msg0);
	max_ver = FIELD_GET(APPLE_RTKIT_MGMT_HELLO_MAXVER, msg.msg0);
	want_ver = min(APPLE_RTKIT_MAX_SUPPORTED_VERSION, max_ver);

	if (min_ver > APPLE_RTKIT_MAX_SUPPORTED_VERSION) {
		printf("%s: firmware min version %d is too new\n",
		       __func__, min_ver);
		return -ENOTSUPP;
	}

	if (max_ver < APPLE_RTKIT_MIN_SUPPORTED_VERSION) {
		printf("%s: firmware max version %d is too old\n",
		       __func__, max_ver);
		return -ENOTSUPP;
	}

	/* Ack version. */
	msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_HELLO_REPLY) |
		FIELD_PREP(APPLE_RTKIT_MGMT_HELLO_MINVER, want_ver) |
		FIELD_PREP(APPLE_RTKIT_MGMT_HELLO_MAXVER, want_ver);
	msg.msg1 = APPLE_RTKIT_EP_MGMT;
	ret = mbox_send(chan, &msg);
	if (ret < 0)
		return ret;

wait_epmap:
	/* Wait for endpoint map message. */
	ret = mbox_recv(chan, &msg, 10000);
	if (ret < 0)
		return ret;

	endpoint = msg.msg1;
	msgtype = FIELD_GET(APPLE_RTKIT_MGMT_TYPE, msg.msg0);
	if (endpoint != APPLE_RTKIT_EP_MGMT) {
		printf("%s: unexpected endpoint %d\n", __func__, endpoint);
		return -EINVAL;
	}
	if (msgtype != APPLE_RTKIT_MGMT_EPMAP) {
		printf("%s: unexpected message type %d\n", __func__, msgtype);
		return -EINVAL;
	}

	bitmap = FIELD_GET(APPLE_RTKIT_MGMT_EPMAP_BITMAP, msg.msg0);
	base = FIELD_GET(APPLE_RTKIT_MGMT_EPMAP_BASE, msg.msg0);
	for (i = 0; i < 32; i++) {
		if (bitmap & (1U << i))
			endpoints[nendpoints++] = base * 32 + i;
	}

	/* Ack endpoint map. */
	reply = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_EPMAP_REPLY) |
		FIELD_PREP(APPLE_RTKIT_MGMT_EPMAP_BASE, base);
	if (msg.msg0 & APPLE_RTKIT_MGMT_EPMAP_LAST)
		reply |= APPLE_RTKIT_MGMT_EPMAP_LAST;
	else
		reply |= APPLE_RTKIT_MGMT_EPMAP_REPLY_MORE;
	msg.msg0 = reply;
	msg.msg1 = APPLE_RTKIT_EP_MGMT;
	ret = mbox_send(chan, &msg);
	if (ret < 0)
		return ret;

	if (reply & APPLE_RTKIT_MGMT_EPMAP_REPLY_MORE)
		goto wait_epmap;

	for (i = 0; i < nendpoints; i++) {
		/* Don't start the syslog endpoint since we can't
		   easily handle its messages in U-Boot. */
		if (endpoints[i] == APPLE_RTKIT_EP_SYSLOG)
			continue;

		/* Request endpoint. */
		msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_STARTEP) |
			FIELD_PREP(APPLE_RTKIT_MGMT_STARTEP_EP, endpoints[i]) |
			APPLE_RTKIT_MGMT_STARTEP_FLAG;
		msg.msg1 = APPLE_RTKIT_EP_MGMT;
		ret = mbox_send(chan, &msg);
		if (ret < 0)
			return ret;
	}

	pwrstate = APPLE_RTKIT_PWR_STATE_SLEEP;
	while (pwrstate != APPLE_RTKIT_PWR_STATE_ON) {
		ret = mbox_recv(chan, &msg, 1000000);
		if (ret < 0)
			return ret;

		endpoint = msg.msg1;
		msgtype = FIELD_GET(APPLE_RTKIT_MGMT_TYPE, msg.msg0);

		if (endpoint == APPLE_RTKIT_EP_CRASHLOG ||
		    endpoint == APPLE_RTKIT_EP_SYSLOG ||
		    endpoint == APPLE_RTKIT_EP_IOREPORT) {
			u64 addr = FIELD_GET(APPLE_RTKIT_BUFFER_REQUEST_IOVA, msg.msg0);
			u64 size = FIELD_GET(APPLE_RTKIT_BUFFER_REQUEST_SIZE, msg.msg0);

			if (msgtype == APPLE_RTKIT_BUFFER_REQUEST && addr != 0)
				continue;

			msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_BUFFER_REQUEST) |
				FIELD_PREP(APPLE_RTKIT_BUFFER_REQUEST_SIZE, size) |
				FIELD_PREP(APPLE_RTKIT_BUFFER_REQUEST_IOVA, addr);
			msg.msg1 = endpoint;
			ret = mbox_send(chan, &msg);
			if (ret < 0)
				return ret;
			continue;
		}

		if (endpoint != APPLE_RTKIT_EP_MGMT) {
			printf("%s: unexpected endpoint %d\n", __func__, endpoint);
			return -EINVAL;
		}
		if (msgtype != APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE_ACK) {
			printf("%s: unexpected message type %d\n", __func__, msgtype);
			return -EINVAL;
		}

		pwrstate = FIELD_GET(APPLE_RTKIT_MGMT_PWR_STATE, msg.msg0);
	}

	return 0;
}

int apple_rtkit_shutdown(struct mbox_chan *chan, int pwrstate)
{
	struct apple_mbox_msg msg;
	int ret;

	msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE) |
		FIELD_PREP(APPLE_RTKIT_MGMT_PWR_STATE, pwrstate);
	msg.msg1 = APPLE_RTKIT_EP_MGMT;
	ret = mbox_send(chan, &msg);
	if (ret < 0)
		return ret;

	ret = mbox_recv(chan, &msg, 100000);
	if (ret < 0)
		return ret;

	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026, Hung-Chun Tseng <alan.tseng.cs@gmail.com>
 *
 * Tests for the virtio RTC driver, with the device emulated through the
 * sandbox virtio transport
 */

#include <dm.h>
#include <rtc.h>
#include <time.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <asm/test.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Message types and clock types from the virtio specification */
#define REQ_READ		0x0001
#define REQ_CFG			0x1000
#define REQ_CLOCK_CAP		0x1001

#define S_OK			0
#define S_EIO			4

#define CLOCK_UTC		0
#define CLOCK_TAI		1
#define CLOCK_MONOTONIC		2
#define CLOCK_UTC_SMEARED	3
#define CLOCK_UTC_MAYBE_SMEARED	4

/* Request: le16 msg_type, 6 reserved bytes, then le16 clock_id */
struct emul_req {
	__le16 msg_type;
	u8 reserved[6];
	__le16 clock_id;
};

/* Response: u8 status, 7 reserved bytes, then the message payload */
struct emul_resp {
	u8 status;
	u8 reserved[7];
	union {
		__le16 num_clocks;
		u8 type;
		__le64 clock_reading;
	};
};

/* 2023-11-14 22:13:20 UTC, plus some nanoseconds */
#define EMUL_TIME_S		1700000000ULL
#define EMUL_TIME_NS		(EMUL_TIME_S * 1000000000ULL + 123456789)

#define EMUL_MAX_CLOCKS		4

/**
 * struct virtio_rtc_emul - State of the emulated device
 *
 * @num_clocks: number of clocks the device reports
 * @types: type of each clock
 * @cap_status: status to return for VIRTIO_RTC_REQ_CLOCK_CAP
 * @no_reply: do not answer requests, but move time past the timeout
 * @cfg_count: number of VIRTIO_RTC_REQ_CFG requests seen
 * @cap_count: number of VIRTIO_RTC_REQ_CLOCK_CAP requests seen
 * @read_count: number of VIRTIO_RTC_REQ_READ requests seen
 * @read_clock: clock ID of the last VIRTIO_RTC_REQ_READ
 * @last_avail: next available ring entry to process
 */
struct virtio_rtc_emul {
	u16 num_clocks;
	u8 types[EMUL_MAX_CLOCKS];
	u8 cap_status;
	bool no_reply;
	int cfg_count;
	int cap_count;
	int read_count;
	u16 read_clock;
	u16 last_avail;
};

static struct virtio_rtc_emul emul;

static u32 emul_handle(const struct emul_req *req, struct emul_resp *resp)
{
	u16 clock_id = le16_to_cpu(req->clock_id);

	memset(resp, '\0', sizeof(*resp));
	resp->status = S_OK;

	switch (le16_to_cpu(req->msg_type)) {
	case REQ_CFG:
		emul.cfg_count++;
		resp->num_clocks = cpu_to_le16(emul.num_clocks);
		return 16;
	case REQ_CLOCK_CAP:
		emul.cap_count++;
		resp->status = emul.cap_status;
		if (clock_id >= emul.num_clocks)
			resp->status = S_EIO;
		else
			resp->type = emul.types[clock_id];
		return 16;
	case REQ_READ:
		emul.read_count++;
		emul.read_clock = clock_id;
		resp->clock_reading = cpu_to_le64(EMUL_TIME_NS);
		return 16;
	default:
		resp->status = S_EIO;
		return 8;
	}
}

static int emul_notify(struct udevice *dev, struct virtqueue *vq)
{
	struct udevice *vdev = vq->vdev;
	struct vring *vr = &vq->vring;

	if (emul.no_reply) {
		timer_test_add_offset(2000);
		return 0;
	}

	while (emul.last_avail != virtio16_to_cpu(vdev, vr->avail->idx)) {
		u16 head = virtio16_to_cpu(vdev, vr->avail->ring[emul.last_avail %
								 vr->num]);
		struct vring_desc *out = &vr->desc[head];
		struct vring_desc *in = &vr->desc[virtio16_to_cpu(vdev,
								   out->next)];
		void *req = (void *)(uintptr_t)virtio64_to_cpu(vdev, out->addr);
		void *resp = (void *)(uintptr_t)virtio64_to_cpu(vdev, in->addr);
		u16 used = virtio16_to_cpu(vdev, vr->used->idx);
		u32 len;

		len = emul_handle(req, resp);
		vr->used->ring[used % vr->num].id = cpu_to_virtio32(vdev, head);
		vr->used->ring[used % vr->num].len = cpu_to_virtio32(vdev, len);
		vr->used->idx = cpu_to_virtio16(vdev, used + 1);
		emul.last_avail++;
	}

	return 0;
}

/* Set up the emulated device and return the virtio RTC device */
static int setup(struct unit_test_state *uts, u16 num_clocks, const u8 *types,
		 struct udevice **devp)
{
	struct udevice *bus;

	memset(&emul, '\0', sizeof(emul));
	emul.num_clocks = num_clocks;
	memcpy(emul.types, types, num_clocks);

	ut_assertok(uclass_get_device_by_name(UCLASS_VIRTIO,
					      "sandbox-virtio-rtc", &bus));
	sandbox_virtio_set_notify(bus, emul_notify);
	ut_assertok(device_find_first_child(bus, devp));
	ut_asserteq_str(VIRTIO_RTC_DRV_NAME, (*devp)->driver->name);
	ut_assertok(device_probe(*devp));

	return 0;
}

/* Test reading the time from the only UTC clock */
static int dm_test_virtio_rtc_get(struct unit_test_state *uts)
{
	static const u8 types[] = { CLOCK_MONOTONIC, CLOCK_UTC, CLOCK_TAI };
	struct udevice *dev;
	struct rtc_time tm;

	ut_assertok(setup(uts, ARRAY_SIZE(types), types, &dev));

	/* probe() must not use the virtqueue */
	ut_asserteq(0, emul.cfg_count);

	ut_assertok(dm_rtc_get(dev, &tm));
	ut_asserteq(1, emul.cfg_count);
	ut_asserteq(3, emul.cap_count);
	ut_asserteq(1, emul.read_count);
	ut_asserteq(1, emul.read_clock);

	ut_asserteq(2023, tm.tm_year);
	ut_asserteq(11, tm.tm_mon);
	ut_asserteq(14, tm.tm_mday);
	ut_asserteq(22, tm.tm_hour);
	ut_asserteq(13, tm.tm_min);
	ut_asserteq(20, tm.tm_sec);
	ut_asserteq(2, tm.tm_wday);

	/* The clock is looked up only once */
	ut_assertok(dm_rtc_get(dev, &tm));
	ut_asserteq(1, emul.cfg_count);
	ut_asserteq(3, emul.cap_count);
	ut_asserteq(2, emul.read_count);

	/* The clocks are read-only */
	ut_asserteq(-ENOSYS, dm_rtc_set(dev, &tm));

	return 0;
}
DM_TEST(dm_test_virtio_rtc_get, UTF_SCAN_FDT);

/* Test that a clock which smears leap seconds is preferred */
static int dm_test_virtio_rtc_smeared(struct unit_test_state *uts)
{
	static const u8 types[] = { CLOCK_UTC, CLOCK_UTC_MAYBE_SMEARED,
				    CLOCK_UTC_SMEARED, CLOCK_TAI };
	struct udevice *dev;
	struct rtc_time tm;

	ut_assertok(setup(uts, ARRAY_SIZE(types), types, &dev));

	/* The search stops at the smeared clock */
	ut_assertok(dm_rtc_get(dev, &tm));
	ut_asserteq(3, emul.cap_count);
	ut_asserteq(2, emul.read_clock);

	return 0;
}
DM_TEST(dm_test_virtio_rtc_smeared, UTF_SCAN_FDT);

/* Test that the first UTC clock is used if none smears leap seconds */
static int dm_test_virtio_rtc_fallback(struct unit_test_state *uts)
{
	static const u8 types[] = { CLOCK_TAI, CLOCK_UTC_MAYBE_SMEARED,
				    CLOCK_UTC };
	struct udevice *dev;
	struct rtc_time tm;

	ut_assertok(setup(uts, ARRAY_SIZE(types), types, &dev));

	ut_assertok(dm_rtc_get(dev, &tm));
	ut_asserteq(3, emul.cap_count);
	ut_asserteq(1, emul.read_clock);

	return 0;
}
DM_TEST(dm_test_virtio_rtc_fallback, UTF_SCAN_FDT);

/* Test that a device without a UTC clock is only queried once */
static int dm_test_virtio_rtc_no_utc(struct unit_test_state *uts)
{
	static const u8 types[] = { CLOCK_TAI, CLOCK_MONOTONIC };
	struct udevice *dev;
	struct rtc_time tm;

	ut_assertok(setup(uts, ARRAY_SIZE(types), types, &dev));

	ut_asserteq(-ENODEV, dm_rtc_get(dev, &tm));
	ut_asserteq(1, emul.cfg_count);
	ut_asserteq(2, emul.cap_count);

	ut_asserteq(-ENODEV, dm_rtc_get(dev, &tm));
	ut_asserteq(1, emul.cfg_count);
	ut_asserteq(2, emul.cap_count);
	ut_asserteq(0, emul.read_count);

	return 0;
}
DM_TEST(dm_test_virtio_rtc_no_utc, UTF_SCAN_FDT);

/* Test that a failed request stops the search and is not remembered */
static int dm_test_virtio_rtc_error(struct unit_test_state *uts)
{
	static const u8 types[] = { CLOCK_TAI, CLOCK_UTC };
	struct udevice *dev;
	struct rtc_time tm;

	ut_assertok(setup(uts, ARRAY_SIZE(types), types, &dev));

	emul.cap_status = S_EIO;
	ut_asserteq(-EIO, dm_rtc_get(dev, &tm));
	ut_asserteq(1, emul.cap_count);

	/* The next read looks for the clock again */
	emul.cap_status = S_OK;
	ut_assertok(dm_rtc_get(dev, &tm));
	ut_asserteq(2, emul.cfg_count);
	ut_asserteq(3, emul.cap_count);
	ut_asserteq(1, emul.read_count);
	ut_asserteq(1, emul.read_clock);

	return 0;
}
DM_TEST(dm_test_virtio_rtc_error, UTF_SCAN_FDT);

/* Test that the driver gives up if the device does not answer */
static int dm_test_virtio_rtc_timeout(struct unit_test_state *uts)
{
	static const u8 types[] = { CLOCK_UTC };
	struct udevice *dev;
	struct rtc_time tm;

	ut_assertok(setup(uts, ARRAY_SIZE(types), types, &dev));

	emul.no_reply = true;
	ut_asserteq(-ETIMEDOUT, dm_rtc_get(dev, &tm));
	ut_asserteq(0, emul.cfg_count);

	return 0;
}
DM_TEST(dm_test_virtio_rtc_timeout, UTF_SCAN_FDT);

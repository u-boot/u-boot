// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx Zynq MPSoC Firmware driver
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.
 */

#include <common.h>
#include <dm.h>

#if defined(CONFIG_ZYNQMP_IPI)
#include <mailbox.h>
#include <zynqmp_firmware.h>
#include <asm/arch/sys_proto.h>

#define PMUFW_PAYLOAD_ARG_CNT	8

struct zynqmp_power {
	struct mbox_chan tx_chan;
	struct mbox_chan rx_chan;
} zynqmp_power;

static int ipi_req(const u32 *req, size_t req_len, u32 *res, size_t res_maxlen)
{
	struct zynqmp_ipi_msg msg;
	int ret;

	if (req_len > PMUFW_PAYLOAD_ARG_CNT ||
	    res_maxlen > PMUFW_PAYLOAD_ARG_CNT)
		return -EINVAL;

	if (!(zynqmp_power.tx_chan.dev) || !(&zynqmp_power.rx_chan.dev))
		return -EINVAL;

	msg.buf = (u32 *)req;
	msg.len = req_len;
	ret = mbox_send(&zynqmp_power.tx_chan, &msg);
	if (ret) {
		debug("%s: Sending message failed\n", __func__);
		return ret;
	}

	msg.buf = res;
	msg.len = res_maxlen;
	ret = mbox_recv(&zynqmp_power.rx_chan, &msg, 100);
	if (ret)
		debug("%s: Receiving message failed\n", __func__);

	return ret;
}

static int send_req(const u32 *req, size_t req_len, u32 *res, size_t res_maxlen)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		return ipi_req(req, req_len, res, res_maxlen);

	return invoke_smc(req[0] + PM_SIP_SVC, 0, 0, 0, 0, res);
}

unsigned int zynqmp_firmware_version(void)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];
	static u32 pm_api_version = ZYNQMP_PM_VERSION_INVALID;

	/*
	 * Get PMU version only once and later
	 * just return stored values instead of
	 * asking PMUFW again.
	 **/
	if (pm_api_version == ZYNQMP_PM_VERSION_INVALID) {
		const u32 request[] = { PM_GET_API_VERSION };

		ret = send_req(request, ARRAY_SIZE(request), ret_payload, 2);
		if (ret)
			panic("PMUFW is not found - Please load it!\n");

		pm_api_version = ret_payload[1];
		if (pm_api_version < ZYNQMP_PM_VERSION)
			panic("PMUFW version error. Expected: v%d.%d\n",
			      ZYNQMP_PM_VERSION_MAJOR, ZYNQMP_PM_VERSION_MINOR);
	}

	return pm_api_version;
};

/**
 * Send a configuration object to the PMU firmware.
 *
 * @cfg_obj: Pointer to the configuration object
 * @size:    Size of @cfg_obj in bytes
 */
void zynqmp_pmufw_load_config_object(const void *cfg_obj, size_t size)
{
	const u32 request[] = {
		PM_SET_CONFIGURATION,
		(u32)((u64)cfg_obj)
	};
	u32 response;
	int err;

	printf("Loading new PMUFW cfg obj (%ld bytes)\n", size);

	err = send_req(request, ARRAY_SIZE(request), &response, 1);
	if (err)
		panic("Cannot load PMUFW configuration object (%d)\n", err);
	if (response != 0)
		panic("PMUFW returned 0x%08x status!\n", response);
}

static int zynqmp_power_probe(struct udevice *dev)
{
	int ret = 0;

	debug("%s, (dev=%p)\n", __func__, dev);

	ret = mbox_get_by_name(dev, "tx", &zynqmp_power.tx_chan);
	if (ret) {
		debug("%s, cannot tx mailbox\n", __func__);
		return ret;
	}

	ret = mbox_get_by_name(dev, "rx", &zynqmp_power.rx_chan);
	if (ret) {
		debug("%s, cannot rx mailbox\n", __func__);
		return ret;
	}

	ret = zynqmp_firmware_version();
	printf("PMUFW:\tv%d.%d\n",
	       ret >> ZYNQMP_PM_VERSION_MAJOR_SHIFT,
	       ret & ZYNQMP_PM_VERSION_MINOR_MASK);

	return 0;
};

static const struct udevice_id zynqmp_power_ids[] = {
	{ .compatible = "xlnx,zynqmp-power" },
	{ }
};

U_BOOT_DRIVER(zynqmp_power) = {
	.name = "zynqmp_power",
	.id = UCLASS_FIRMWARE,
	.of_match = zynqmp_power_ids,
	.probe = zynqmp_power_probe,
};
#endif

static const struct udevice_id zynqmp_firmware_ids[] = {
	{ .compatible = "xlnx,zynqmp-firmware" },
	{ .compatible = "xlnx,versal-firmware"},
	{ }
};

U_BOOT_DRIVER(zynqmp_firmware) = {
	.id = UCLASS_FIRMWARE,
	.name = "zynqmp-firmware",
	.probe = dm_scan_fdt_dev,
	.of_match = zynqmp_firmware_ids,
};

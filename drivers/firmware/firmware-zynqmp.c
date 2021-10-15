// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx Zynq MPSoC Firmware driver
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <zynqmp_firmware.h>
#include <asm/cache.h>
#include <asm/ptrace.h>

#if defined(CONFIG_ZYNQMP_IPI)
#include <mailbox.h>
#include <asm/arch/sys_proto.h>

#define PMUFW_PAYLOAD_ARG_CNT	8

#define XST_PM_NO_ACCESS	2002L

struct zynqmp_power {
	struct mbox_chan tx_chan;
	struct mbox_chan rx_chan;
} zynqmp_power;

static int ipi_req(const u32 *req, size_t req_len, u32 *res, size_t res_maxlen)
{
	struct zynqmp_ipi_msg msg;
	int ret;
	u32 buffer[PAYLOAD_ARG_CNT];

	if (!res)
		res = buffer;

	if (req_len > PMUFW_PAYLOAD_ARG_CNT ||
	    res_maxlen > PMUFW_PAYLOAD_ARG_CNT)
		return -EINVAL;

	if (!(zynqmp_power.tx_chan.dev) || !(&zynqmp_power.rx_chan.dev))
		return -EINVAL;

	debug("%s, Sending IPI message with ID: 0x%0x\n", __func__, req[0]);
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

		ret = xilinx_pm_request(PM_GET_API_VERSION, 0, 0, 0, 0,
					ret_payload);
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
	int err;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	printf("Loading new PMUFW cfg obj (%ld bytes)\n", size);

	err = xilinx_pm_request(PM_SET_CONFIGURATION, (u32)(u64)cfg_obj, 0, 0,
				0, ret_payload);
	if (err == XST_PM_NO_ACCESS) {
		printf("PMUFW no permission to change config object\n");
		return;
	}

	if (err)
		printf("Cannot load PMUFW configuration object (%d)\n", err);

	if (ret_payload[0])
		printf("PMUFW returned 0x%08x status!\n", ret_payload[0]);

	if ((err || ret_payload[0]) && IS_ENABLED(CONFIG_SPL_BUILD))
		panic("PMUFW config object loading failed in EL3\n");
}

static int zynqmp_power_probe(struct udevice *dev)
{
	int ret;

	debug("%s, (dev=%p)\n", __func__, dev);

	ret = mbox_get_by_name(dev, "tx", &zynqmp_power.tx_chan);
	if (ret) {
		debug("%s: Cannot find tx mailbox\n", __func__);
		return ret;
	}

	ret = mbox_get_by_name(dev, "rx", &zynqmp_power.rx_chan);
	if (ret) {
		debug("%s: Cannot find rx mailbox\n", __func__);
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

int __maybe_unused xilinx_pm_request(u32 api_id, u32 arg0, u32 arg1, u32 arg2,
				     u32 arg3, u32 *ret_payload)
{
	debug("%s at EL%d, API ID: 0x%0x\n", __func__, current_el(), api_id);

	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
#if defined(CONFIG_ZYNQMP_IPI)
		/*
		 * Use fixed payload and arg size as the EL2 call. The firmware
		 * is capable to handle PMUFW_PAYLOAD_ARG_CNT bytes but the
		 * firmware API is limited by the SMC call size
		 */
		u32 regs[] = {api_id, arg0, arg1, arg2, arg3};
		int ret;

		if (api_id == PM_FPGA_LOAD) {
			/* Swap addr_hi/low because of incompatibility */
			u32 temp = regs[1];

			regs[1] = regs[2];
			regs[2] = temp;
		}

		ret = ipi_req(regs, PAYLOAD_ARG_CNT, ret_payload,
			      PAYLOAD_ARG_CNT);
		if (ret)
			return ret;
#else
		return -EPERM;
#endif
	} else {
		/*
		 * Added SIP service call Function Identifier
		 * Make sure to stay in x0 register
		 */
		struct pt_regs regs;

		regs.regs[0] = PM_SIP_SVC | api_id;
		regs.regs[1] = ((u64)arg1 << 32) | arg0;
		regs.regs[2] = ((u64)arg3 << 32) | arg2;

		smc_call(&regs);

		if (ret_payload) {
			ret_payload[0] = (u32)regs.regs[0];
			ret_payload[1] = upper_32_bits(regs.regs[0]);
			ret_payload[2] = (u32)regs.regs[1];
			ret_payload[3] = upper_32_bits(regs.regs[1]);
			ret_payload[4] = (u32)regs.regs[2];
		}

	}
	return (ret_payload) ? ret_payload[0] : 0;
}

static const struct udevice_id zynqmp_firmware_ids[] = {
	{ .compatible = "xlnx,zynqmp-firmware" },
	{ .compatible = "xlnx,versal-firmware"},
	{ }
};

U_BOOT_DRIVER(zynqmp_firmware) = {
	.id = UCLASS_FIRMWARE,
	.name = "zynqmp_firmware",
	.of_match = zynqmp_firmware_ids,
};

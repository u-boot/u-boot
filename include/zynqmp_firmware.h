/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Xilinx Zynq MPSoC Firmware driver
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.
 */

#ifndef _ZYNQMP_FIRMWARE_H_
#define _ZYNQMP_FIRMWARE_H_

enum pm_api_id {
	PM_GET_API_VERSION = 1,
	PM_SET_CONFIGURATION = 2,
	PM_GET_NODE_STATUS = 3,
	PM_GET_OPERATING_CHARACTERISTIC = 4,
	PM_REGISTER_NOTIFIER = 5,
	/* API for suspending */
	PM_REQUEST_SUSPEND = 6,
	PM_SELF_SUSPEND = 7,
	PM_FORCE_POWERDOWN = 8,
	PM_ABORT_SUSPEND = 9,
	PM_REQUEST_WAKEUP = 10,
	PM_SET_WAKEUP_SOURCE = 11,
	PM_SYSTEM_SHUTDOWN = 12,
	PM_REQUEST_NODE = 13,
	PM_RELEASE_NODE = 14,
	PM_SET_REQUIREMENT = 15,
	PM_SET_MAX_LATENCY = 16,
	/* Direct control API functions: */
	PM_RESET_ASSERT = 17,
	PM_RESET_GET_STATUS = 18,
	PM_MMIO_WRITE = 19,
	PM_MMIO_READ = 20,
	PM_PM_INIT_FINALIZE = 21,
	PM_FPGA_LOAD = 22,
	PM_FPGA_GET_STATUS = 23,
	PM_GET_CHIPID = 24,
	/* ID 25 is been used by U-boot to process secure boot images */
	/* Secure library generic API functions */
	PM_SECURE_SHA = 26,
	PM_SECURE_RSA = 27,
	PM_PINCTRL_REQUEST = 28,
	PM_PINCTRL_RELEASE = 29,
	PM_PINCTRL_GET_FUNCTION = 30,
	PM_PINCTRL_SET_FUNCTION = 31,
	PM_PINCTRL_CONFIG_PARAM_GET = 32,
	PM_PINCTRL_CONFIG_PARAM_SET = 33,
	PM_IOCTL = 34,
	PM_QUERY_DATA = 35,
	PM_CLOCK_ENABLE = 36,
	PM_CLOCK_DISABLE = 37,
	PM_CLOCK_GETSTATE = 38,
	PM_CLOCK_SETDIVIDER = 39,
	PM_CLOCK_GETDIVIDER = 40,
	PM_CLOCK_SETRATE = 41,
	PM_CLOCK_GETRATE = 42,
	PM_CLOCK_SETPARENT = 43,
	PM_CLOCK_GETPARENT = 44,
	PM_SECURE_IMAGE = 45,
	PM_FPGA_READ = 46,
	PM_SECURE_AES = 47,
	PM_CLOCK_PLL_GETPARAM = 49,
	/* PM_REGISTER_ACCESS API */
	PM_REGISTER_ACCESS = 52,
	PM_EFUSE_ACCESS = 53,
	PM_FEATURE_CHECK = 63,
	PM_API_MAX,
};

enum pm_query_id {
	PM_QID_INVALID = 0,
	PM_QID_CLOCK_GET_NAME = 1,
	PM_QID_CLOCK_GET_TOPOLOGY = 2,
	PM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS = 3,
	PM_QID_CLOCK_GET_PARENTS = 4,
	PM_QID_CLOCK_GET_ATTRIBUTES = 5,
	PM_QID_PINCTRL_GET_NUM_PINS = 6,
	PM_QID_PINCTRL_GET_NUM_FUNCTIONS = 7,
	PM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS = 8,
	PM_QID_PINCTRL_GET_FUNCTION_NAME = 9,
	PM_QID_PINCTRL_GET_FUNCTION_GROUPS = 10,
	PM_QID_PINCTRL_GET_PIN_GROUPS = 11,
	PM_QID_CLOCK_GET_NUM_CLOCKS = 12,
	PM_QID_CLOCK_GET_MAX_DIVISOR = 13,
};

#define PM_SIP_SVC      0xc2000000

#define ZYNQMP_PM_VERSION_MAJOR         1
#define ZYNQMP_PM_VERSION_MINOR         0
#define ZYNQMP_PM_VERSION_MAJOR_SHIFT   16
#define ZYNQMP_PM_VERSION_MINOR_MASK    0xFFFF

#define ZYNQMP_PM_VERSION       \
	((ZYNQMP_PM_VERSION_MAJOR << ZYNQMP_PM_VERSION_MAJOR_SHIFT) | \
	 ZYNQMP_PM_VERSION_MINOR)

#define ZYNQMP_PM_VERSION_INVALID       ~0

#define PMUFW_V1_0      ((1 << ZYNQMP_PM_VERSION_MAJOR_SHIFT) | 0)

/*
 * Return payload size
 * Not every firmware call expects the same amount of return bytes, however the
 * firmware driver always copies 5 bytes from RX buffer to the ret_payload
 * buffer. Therefore allocating with this defined value is recommended to avoid
 * overflows.
 */
#define PAYLOAD_ARG_CNT	5U

unsigned int zynqmp_firmware_version(void);
void zynqmp_pmufw_load_config_object(const void *cfg_obj, size_t size);
int xilinx_pm_request(u32 api_id, u32 arg0, u32 arg1, u32 arg2,
		      u32 arg3, u32 *ret_payload);

#endif /* _ZYNQMP_FIRMWARE_H_ */

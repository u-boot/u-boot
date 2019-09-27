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
	PM_SET_CONFIGURATION,
	PM_SECURE_IMAGE = 45,
};

#define PM_SIP_SVC      0xc2000000
#define ZYNQMP_SIP_SVC_PM_SECURE_IMG_LOAD       \
	(PM_SIP_SVC + PM_SECURE_IMAGE)

#define ZYNQMP_PM_VERSION_MAJOR         1
#define ZYNQMP_PM_VERSION_MINOR         0
#define ZYNQMP_PM_VERSION_MAJOR_SHIFT   16
#define ZYNQMP_PM_VERSION_MINOR_MASK    0xFFFF

#define ZYNQMP_PM_VERSION       \
	((ZYNQMP_PM_VERSION_MAJOR << ZYNQMP_PM_VERSION_MAJOR_SHIFT) | \
	 ZYNQMP_PM_VERSION_MINOR)

#define ZYNQMP_PM_VERSION_INVALID       ~0

#define PMUFW_V1_0      ((1 << ZYNQMP_PM_VERSION_MAJOR_SHIFT) | 0)

unsigned int zynqmp_firmware_version(void);
void zynqmp_pmufw_load_config_object(const void *cfg_obj, size_t size);

#endif /* _ZYNQMP_FIRMWARE_H_ */

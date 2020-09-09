/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (C) 2019-2020, Linaro Limited
 */
#ifndef _SCMI_PROTOCOLS_H
#define _SCMI_PROTOCOLS_H

#include <linux/bitops.h>

/*
 * Subset the SCMI protocols definition
 * based on SCMI specification v2.0 (DEN0056B)
 * https://developer.arm.com/docs/den0056/b
 */

enum scmi_std_protocol {
	SCMI_PROTOCOL_ID_BASE = 0x10,
	SCMI_PROTOCOL_ID_POWER_DOMAIN = 0x11,
	SCMI_PROTOCOL_ID_SYSTEM = 0x12,
	SCMI_PROTOCOL_ID_PERF = 0x13,
	SCMI_PROTOCOL_ID_CLOCK = 0x14,
	SCMI_PROTOCOL_ID_SENSOR = 0x15,
	SCMI_PROTOCOL_ID_RESET_DOMAIN = 0x16,
};

enum scmi_status_code {
	SCMI_SUCCESS =  0,
	SCMI_NOT_SUPPORTED = -1,
	SCMI_INVALID_PARAMETERS = -2,
	SCMI_DENIED = -3,
	SCMI_NOT_FOUND = -4,
	SCMI_OUT_OF_RANGE = -5,
	SCMI_BUSY = -6,
	SCMI_COMMS_ERROR = -7,
	SCMI_GENERIC_ERROR = -8,
	SCMI_HARDWARE_ERROR = -9,
	SCMI_PROTOCOL_ERROR = -10,
};

#endif /* _SCMI_PROTOCOLS_H */

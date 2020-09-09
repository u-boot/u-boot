/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (C) 2019-2020, Linaro Limited
 */
#ifndef _SCMI_PROTOCOLS_H
#define _SCMI_PROTOCOLS_H

#include <linux/bitops.h>
#include <asm/types.h>

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

/*
 * SCMI Clock Protocol
 */

enum scmi_clock_message_id {
	SCMI_CLOCK_RATE_SET = 0x5,
	SCMI_CLOCK_RATE_GET = 0x6,
	SCMI_CLOCK_CONFIG_SET = 0x7,
};

#define SCMI_CLK_RATE_ASYNC_NOTIFY	BIT(0)
#define SCMI_CLK_RATE_ASYNC_NORESP	(BIT(0) | BIT(1))
#define SCMI_CLK_RATE_ROUND_DOWN	0
#define SCMI_CLK_RATE_ROUND_UP		BIT(2)
#define SCMI_CLK_RATE_ROUND_CLOSEST	BIT(3)

/**
 * struct scmi_clk_state_in - Message payload for CLOCK_CONFIG_SET command
 * @clock_id:	SCMI clock ID
 * @attributes:	Attributes of the targets clock state
 */
struct scmi_clk_state_in {
	u32 clock_id;
	u32 attributes;
};

/**
 * struct scmi_clk_state_out - Response payload for CLOCK_CONFIG_SET command
 * @status:	SCMI command status
 */
struct scmi_clk_state_out {
	s32 status;
};

/**
 * struct scmi_clk_state_in - Message payload for CLOCK_RATE_GET command
 * @clock_id:	SCMI clock ID
 * @attributes:	Attributes of the targets clock state
 */
struct scmi_clk_rate_get_in {
	u32 clock_id;
};

/**
 * struct scmi_clk_rate_get_out - Response payload for CLOCK_RATE_GET command
 * @status:	SCMI command status
 * @rate_lsb:	32bit LSB of the clock rate in Hertz
 * @rate_msb:	32bit MSB of the clock rate in Hertz
 */
struct scmi_clk_rate_get_out {
	s32 status;
	u32 rate_lsb;
	u32 rate_msb;
};

/**
 * struct scmi_clk_state_in - Message payload for CLOCK_RATE_SET command
 * @clock_id:	SCMI clock ID
 * @flags:	Flags for the clock rate set request
 * @rate_lsb:	32bit LSB of the clock rate in Hertz
 * @rate_msb:	32bit MSB of the clock rate in Hertz
 */
struct scmi_clk_rate_set_in {
	u32 clock_id;
	u32 flags;
	u32 rate_lsb;
	u32 rate_msb;
};

/**
 * struct scmi_clk_rate_set_out - Response payload for CLOCK_RATE_SET command
 * @status:	SCMI command status
 */
struct scmi_clk_rate_set_out {
	s32 status;
};

#endif /* _SCMI_PROTOCOLS_H */

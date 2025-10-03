/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright 2025 NXP
 */

#ifndef _SCMI_NXP_PROTOCOLS_H
#define _SCMI_NXP_PROTOCOLS_H

#include <asm/types.h>
#include <linux/bitops.h>

enum scmi_imx_protocol {
	SCMI_IMX_PROTOCOL_ID_MISC = 0x84,
};

#define SCMI_PAYLOAD_LEN	100

#define SCMI_ARRAY(X, Y)	((SCMI_PAYLOAD_LEN - (X)) / sizeof(Y))

#define SCMI_IMX_MISC_RESET_REASON	0xA

struct scmi_imx_misc_reset_reason_in {
#define MISC_REASON_FLAG_SYSTEM		BIT(0)
	u32 flags;
};

struct scmi_imx_misc_reset_reason_out {
	s32 status;
	/* Boot reason flags */
#define MISC_BOOT_FLAG_VLD	BIT(31)
#define MISC_BOOT_FLAG_ORG_VLD	BIT(28)
#define MISC_BOOT_FLAG_ORIGIN	GENMASK(27, 24)
#define MISC_BOOT_FLAG_O_SHIFT	24
#define MISC_BOOT_FLAG_ERR_VLD	BIT(23)
#define MISC_BOOT_FLAG_ERR_ID	GENMASK(22, 8)
#define MISC_BOOT_FLAG_E_SHIFT	8
#define MISC_BOOT_FLAG_REASON	GENMASK(7, 0)
	u32 bootflags;
	/* Shutdown reason flags */
#define MISC_SHUTDOWN_FLAG_VLD		BIT(31)
#define MISC_SHUTDOWN_FLAG_EXT_LEN	GENMASK(30, 29)
#define MISC_SHUTDOWN_FLAG_ORG_VLD	BIT(28)
#define MISC_SHUTDOWN_FLAG_ORIGIN	GENMASK(27, 24)
#define MISC_SHUTDOWN_FLAG_O_SHIFT	24
#define MISC_SHUTDOWN_FLAG_ERR_VLD	BIT(23)
#define MISC_SHUTDOWN_FLAG_ERR_ID	GENMASK(22, 8)
#define MISC_SHUTDOWN_FLAG_E_SHIFT	8
#define MISC_SHUTDOWN_FLAG_REASON	GENMASK(7, 0)
	u32 shutdownflags;
	/* Array of extended info words */
#define MISC_MAX_EXTINFO	SCMI_ARRAY(16, u32)
	u32 extInfo[MISC_MAX_EXTINFO];
};

#endif

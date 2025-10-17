/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright 2025 NXP
 */

#ifndef _SCMI_NXP_PROTOCOLS_H
#define _SCMI_NXP_PROTOCOLS_H

#include <asm/types.h>
#include <linux/bitops.h>

#define SCMI_PROTOCOL_ID_IMX_LMM	0x80
#define SCMI_PROTOCOL_ID_IMX_CPU	0x82
#define SCMI_PROTOCOL_ID_IMX_MISC	0x84

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

#define LMM_ID_DISCOVER	0xFFFFFFFFU
#define	LMM_MAX_NAME	16

enum scmi_imx_lmm_state {
	LMM_STATE_LM_OFF,
	LMM_STATE_LM_ON,
	LMM_STATE_LM_SUSPEND,
	LMM_STATE_LM_POWERED,
};

struct scmi_imx_lmm_info {
	u32 lmid;
	enum scmi_imx_lmm_state state;
	u32 errstatus;
	u8 name[LMM_MAX_NAME];
};

#if IS_ENABLED(CONFIG_IMX_SM_LMM)
int scmi_imx_lmm_info(struct udevice *dev, u32 lmid, struct scmi_imx_lmm_info *info);
int scmi_imx_lmm_power_boot(struct udevice *dev, u32 lmid, bool boot);
int scmi_imx_lmm_reset_vector_set(struct udevice *dev, u32 lmid, u32 cpuid, u32 flags, u64 vector);
int scmi_imx_lmm_shutdown(struct udevice *dev, u32 lmid, bool flags);
#else
static inline int scmi_imx_lmm_info(struct udevice *dev, u32 lmid, struct scmi_imx_lmm_info *info)
{
	return -EOPNOTSUPP;
}

static inline int scmi_imx_lmm_power_boot(struct udevice *dev, u32 lmid, bool boot)
{
	return -EOPNOTSUPP;
}

static inline int
scmi_imx_lmm_reset_vector_set(struct udevice *dev, u32 lmid, u32 cpuid, u32 flags, u64 vector)
{
	return -EOPNOTSUPP;
}

static inline int scmi_imx_lmm_shutdown(struct udevice *dev, u32 lmid, bool flags)
{
	return -EOPNOTSUPP;
}
#endif

#if IS_ENABLED(CONFIG_IMX_SM_CPU)
int scmi_imx_cpu_started(struct udevice *dev, u32 cpuid, bool *started);
int scmi_imx_cpu_reset_vector_set(struct udevice *dev, u32 cpuid, u32 flags, u64 vector,
				  bool start, bool boot, bool resume);
int scmi_imx_cpu_start(struct udevice *dev, u32 cpuid, bool start);
#else
static inline int scmi_imx_cpu_started(struct udevice *dev, u32 cpuid, bool *started)
{
	return -EOPNOTSUPP;
}

static inline int scmi_imx_cpu_reset_vector_set(struct udevice *dev, u32 cpuid, u32 flags,
						u64 vector, bool start, bool boot, bool resume)
{
	return -EOPNOTSUPP;
}

static inline int scmi_imx_cpu_start(struct udevice *dev, u32 cpuid, bool start)
{
	return -EOPNOTSUPP;
}
#endif
#endif

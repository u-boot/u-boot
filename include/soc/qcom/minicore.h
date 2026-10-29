/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
 */
#ifndef _SOC_QCOM_MINI_CORE
#define _SOC_QCOM_MINI_CORE

struct qup_mini_core_info {
	u16 serial_protocol;
	u16 fw_version;
	u16 cfg_version;
	u16 cfg_count;
	u32 *cfg_val;
	u8 *cfg_idx;
	u32 *cfg_ram;
	u32 cfg_ram_count;
};

#if IS_ENABLED(CONFIG_QCOM_GENI_MINICORE)
extern struct qup_mini_core_info qup_mini_cores[];
#else
#define qup_mini_cores		NULL
#endif

#endif /* _SOC_QCOM_MINI_CORE */

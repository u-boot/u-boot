/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc.
 * Copyright (c) 2025, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Slimmed down header from Linux: drivers/ufs/host/ufs-mediatek-sip.h
 */

#ifndef _UFS_MEDIATEK_SIP_H
#define _UFS_MEDIATEK_SIP_H

#include <linux/arm-smccc.h>

/*
 * SiP (Slicon Partner) commands
 */
#define MTK_SIP_UFS_CONTROL			ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
								   ARM_SMCCC_SMC_64, \
								   ARM_SMCCC_OWNER_SIP, 0x276)
#define UFS_MTK_SIP_DEVICE_RESET		BIT(1)
#define UFS_MTK_SIP_REF_CLK_NOTIFICATION	BIT(3)

/*
 * SMC call wrapper function
 */
struct ufs_mtk_smc_arg {
	unsigned long cmd;
	struct arm_smccc_res *res;
	unsigned long v1;
	unsigned long v2;
	unsigned long v3;
	unsigned long v4;
	unsigned long v5;
	unsigned long v6;
	unsigned long v7;
};

static inline void _ufs_mtk_smc(struct ufs_mtk_smc_arg s)
{
	arm_smccc_smc(MTK_SIP_UFS_CONTROL,
		      s.cmd,
		      s.v1, s.v2, s.v3, s.v4, s.v5, s.v6, s.res);
}

#define ufs_mtk_smc(...) \
	_ufs_mtk_smc((struct ufs_mtk_smc_arg) {__VA_ARGS__})

/* SIP interface */

#define ufs_mtk_ref_clk_notify(on, stage, res) \
	ufs_mtk_smc(UFS_MTK_SIP_REF_CLK_NOTIFICATION, &(res), on, stage)

#define ufs_mtk_device_reset_ctrl(high, res) \
	ufs_mtk_smc(UFS_MTK_SIP_DEVICE_RESET, &(res), high)

#endif /* !_UFS_MEDIATEK_SIP_H */

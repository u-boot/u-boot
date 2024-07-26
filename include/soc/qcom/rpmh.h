/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 */

#ifndef __SOC_QCOM_RPMH_H__
#define __SOC_QCOM_RPMH_H__

#include <dm/device-internal.h>
#include <soc/qcom/tcs.h>


#if IS_ENABLED(CONFIG_QCOM_RPMH)
int rpmh_write(const struct udevice *dev, enum rpmh_state state,
	       const struct tcs_cmd *cmd, u32 n);

#else

static inline int rpmh_write(const struct device *dev, enum rpmh_state state,
			     const struct tcs_cmd *cmd, u32 n)
{ return -ENODEV; }

#endif /* CONFIG_QCOM_RPMH */

/* u-boot: no multithreading */
#define rpmh_write_async(dev, state, cmd, n) rpmh_write(dev, state, cmd, n)

#endif /* __SOC_QCOM_RPMH_H__ */

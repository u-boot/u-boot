// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2026, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <log.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <linux/err.h>

/* SYSCFG register */
#define SYSCFG_DEVICEID_OFFSET		0x6400
#define SYSCFG_DEVICEID_DEV_ID_MASK	GENMASK(11, 0)
#define SYSCFG_DEVICEID_DEV_ID_SHIFT	0

/* Revision ID = OTP102[5:0] 6 bits : 3 for Major / 3 for Minor*/
#define REVID_SHIFT	0
#define REVID_MASK	GENMASK(5, 0)

/* Device Part Number (RPN) = OTP9 */
#define RPN_SHIFT	0
#define RPN_MASK	GENMASK(31, 0)

#define PKG_SHIFT	0
#define PKG_MASK	GENMASK(2, 0)

static u32 read_deviceid(void)
{
	void *syscfg = syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	if (IS_ERR(syscfg)) {
		pr_err("Error, can't get SYSCON range (%ld)\n", PTR_ERR(syscfg));

		return PTR_ERR(syscfg);
	}

	return readl(syscfg + SYSCFG_DEVICEID_OFFSET);
}

u32 get_cpu_dev(void)
{
	return (read_deviceid() & SYSCFG_DEVICEID_DEV_ID_MASK) >> SYSCFG_DEVICEID_DEV_ID_SHIFT;
}

u32 get_cpu_rev(void)
{
	return get_otp(BSEC_OTP_REVID, REVID_SHIFT, REVID_MASK);
}

/* Get Device Part Number (RPN) from OTP */
u32 get_cpu_type(void)
{
	return get_otp(BSEC_OTP_RPN, RPN_SHIFT, RPN_MASK);
}

/* Get Package options from OTP */
u32 get_cpu_package(void)
{
	return get_otp(BSEC_OTP_PKG, PKG_SHIFT, PKG_MASK);
}

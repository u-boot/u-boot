// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <dm.h>
#include <stdbool.h>
#include <sysreset.h>
#include <wait_bit.h>
#include <linux/io.h>
#include <linux/errno.h>

#define REG_RTC_BASE             (void *)0x05026000
#define REG_RTC_CTRL_BASE        (void *)0x05025000
#define REG_RTC_EN_SHDN_REQ      (REG_RTC_BASE + 0xc0)
#define REG_RTC_EN_PWR_CYC_REQ   (REG_RTC_BASE + 0xc8)
#define REG_RTC_EN_WARM_RST_REQ  (REG_RTC_BASE + 0xcc)
#define REG_RTC_CTRL_UNLOCKKEY   (REG_RTC_CTRL_BASE + 0x4)
#define REG_RTC_CTRL             (REG_RTC_CTRL_BASE + 0x8)

#define CTRL_UNLOCKKEY_MAGIC     0xAB18

/* REG_RTC_CTRL */
#define BIT_REQ_SHDN       BIT(0)
#define BIT_REQ_PWR_CYC    BIT(3)
#define BIT_REQ_WARM_RST   BIT(4)

static struct {
	void *pre_req_reg;
	u32 req_bit;
} reset_info[SYSRESET_COUNT] = {
	[SYSRESET_WARM]      = { REG_RTC_EN_WARM_RST_REQ, BIT_REQ_WARM_RST },
	[SYSRESET_COLD]      = { REG_RTC_EN_WARM_RST_REQ, BIT_REQ_WARM_RST },
	[SYSRESET_POWER]     = { REG_RTC_EN_PWR_CYC_REQ, BIT_REQ_PWR_CYC },
	[SYSRESET_POWER_OFF] = { REG_RTC_EN_SHDN_REQ, BIT_REQ_SHDN },
};

static int cv1800b_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	u32 reg;

	writel(1, reset_info[type].pre_req_reg);
	writel(CTRL_UNLOCKKEY_MAGIC, REG_RTC_CTRL_UNLOCKKEY);
	reg = readl(REG_RTC_CTRL);
	writel(0xFFFF0800 | reset_info[type].req_bit, REG_RTC_CTRL);

	return -EINPROGRESS;
}

static struct sysreset_ops cv1800b_sysreset = {
	.request = cv1800b_sysreset_request,
};

static const struct udevice_id cv1800b_sysreset_ids[] = {
	{ .compatible = "sophgo,cv1800b-sysreset", },
	{},
};

U_BOOT_DRIVER(sysreset_cv1800b) = {
	.name = "cv1800b_sysreset",
	.id	  = UCLASS_SYSRESET,
	.ops  = &cv1800b_sysreset,
	.of_match = cv1800b_sysreset_ids
};

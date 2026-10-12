// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 * Copyright (c) 2026, Zhuxu Ran <zhuxu_ran@zohomail.com>
 */

#include <dm.h>
#include <regmap.h>
#include <sysreset.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>

#define REG_RTC_CTRL_UNLOCKKEY  0x0004
#define REG_RTC_CTRL            0x0008
#define REG_RTC_EN_SHDN_REQ     0x10C0
#define REG_RTC_EN_PWR_CYC_REQ  0x10C8
#define REG_RTC_EN_WARM_RST_REQ 0x10CC

#define CTRL_UNLOCKKEY_MAGIC    0xAB18

#define BIT_REQ_SHDN            BIT(0)
#define BIT_REQ_PWR_CYC         BIT(3)
#define BIT_REQ_WARM_RST        BIT(4)

struct cv1800b_sysreset_priv {
	struct regmap *regmap;
};

static const struct {
	u32 pre_req_reg;
	u32 req_bit;
} reset_info[SYSRESET_COUNT] = {
	[SYSRESET_WARM]      = { REG_RTC_EN_WARM_RST_REQ, BIT_REQ_WARM_RST },
	[SYSRESET_COLD]      = { REG_RTC_EN_WARM_RST_REQ, BIT_REQ_WARM_RST },
	[SYSRESET_POWER]     = { REG_RTC_EN_PWR_CYC_REQ, BIT_REQ_PWR_CYC },
	[SYSRESET_POWER_OFF] = { REG_RTC_EN_SHDN_REQ, BIT_REQ_SHDN },
};

static int cv1800b_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct cv1800b_sysreset_priv *priv = dev_get_priv(dev);

	if (type >= SYSRESET_COUNT)
		return -EINVAL;

	regmap_write(priv->regmap, reset_info[type].pre_req_reg, 1);
	regmap_write(priv->regmap, REG_RTC_CTRL_UNLOCKKEY, CTRL_UNLOCKKEY_MAGIC);
	regmap_write(priv->regmap, REG_RTC_CTRL,
		     0xFFFF0800 | reset_info[type].req_bit);

	return -EINPROGRESS;
}

static int cv1800b_sysreset_probe(struct udevice *dev)
{
	struct cv1800b_sysreset_priv *priv = dev_get_priv(dev);

	priv->regmap = devm_regmap_init(dev, NULL, NULL, NULL);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	return 0;
}

static struct sysreset_ops cv1800b_sysreset = {
	.request = cv1800b_sysreset_request,
};

U_BOOT_DRIVER(sysreset_cv1800b) = {
	.name = "cv1800b_sysreset",
	.id = UCLASS_SYSRESET,
	.probe = cv1800b_sysreset_probe,
	.ops = &cv1800b_sysreset,
	.priv_auto = sizeof(struct cv1800b_sysreset_priv),
};

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/bitops.h>
#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <pwrap/pwrap.h>
#include <power/pmic.h>
#include <power/mt6328_pmic.h>

#define MT6328_TOPSTATUS                        0x0220
#define PWRKEY_ON_INT_BIT                     1
#define HOMEKEY_ON_INT_BIT                      2


static int mtk_pwrkey_get_function(struct udevice *dev, unsigned offset)
{
	return GPIOF_INPUT;
}

static int mtk_pwrkey_get_value(struct udevice *dev, unsigned offset)
{
	u32 val;
	int reg = pwrap_read(dev->parent, MT6328_TOPSTATUS, &val);
	if (reg < 0)
		return 0;

	switch (offset) {
	case 0: /* Power button */
		return (reg & BIT(PWRKEY_ON_INT_BIT)) != 0;
		break;
	case 1: /* Reset button */
	default:
		return (reg & BIT(HOMEKEY_ON_INT_BIT)) != 0;
		break;
	}
}


static const struct dm_gpio_ops mtk_pwrkey_ops = {
	.get_value		= mtk_pwrkey_get_value,
	.get_function		= mtk_pwrkey_get_function,
};

static int mtk_pwrkey_probe(struct udevice *dev)
{
	int ret, val;

    pwrap_write(dev->parent, MT6328_TOP_RST_MISC, (0 << 10)); /* Enable Reset timer */

    pwrap_write(dev->parent, MT6328_TOP_RST_MISC, 0x3000); /* Reset timer set for 5s Long press reset */

    ret = pwrap_read(dev->parent, MT6328_TOP_RST_MISC, &val);

    if (ret < 0)
        return ret;
    if (val & 0x3000) {
        debug("Reset enabled for long-press 5s");
    } else {
        return -EINVAL;
    }
    pwrap_write(dev->parent, MT6328_TOP_RST_MISC, 0x200);  /* Reset enable for PWRKEY*/

    pwrap_write(dev->parent, MT6328_TOP_RST_MISC, 0x100); /* Reset disable for HOMEKEY */

    ret = pwrap_read(dev->parent, MT6328_TOP_RST_MISC, &val);
    if (ret < 0)
        return ret;

    if (val & 0x200) {
        debug("Reset enabled for PWRKEY");
    } if (val & 0x100) {
        debug("Reset enabled for HOMEKEY");
    } else {
        return -EINVAL;
    }

    return 0;

}


static int mtk_pwrkey_of_to_plat(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = 2;
	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");
	if (uc_priv->bank_name == NULL)
		uc_priv->bank_name = "mtk_key";

	return 0;
}

static const struct udevice_id mtk_pwrkey_ids[] = {
	{ .compatible = "mtk,mtk-pwrkey" },
	{ }
};

U_BOOT_DRIVER(mtk_pwrkey) = {
	.name	= "mt6328_keys",
	.id	= UCLASS_GPIO,
	.of_match = mtk_pwrkey_ids,
	.of_to_plat = mtk_pwrkey_of_to_plat,
	.probe	= mtk_pwrkey_probe,
	.ops	= &mtk_pwrkey_ops,
};

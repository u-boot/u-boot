// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Pali Rohár <pali@kernel.org>
 * Copyright (C) 2024 Marek Behún <kabel@kernel.org>
 */

#include <console.h>
#include <dm.h>
#include <dm/lists.h>
#include <i2c.h>
#include <rng.h>
#include <sysreset.h>
#include <turris-omnia-mcu-interface.h>
#include <asm/byteorder.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/log2.h>

#define CMD_TRNG_MAX_ENTROPY_LEN	64

struct turris_omnia_mcu_info {
	u32 features;
};

static int omnia_gpio_get_function(struct udevice *dev, uint offset)
{
	struct turris_omnia_mcu_info *info = dev_get_priv(dev->parent);

	switch (offset) {
	/* bank 0 */
	case 0 ... 15:
		switch (offset) {
		case ilog2(STS_USB30_PWRON):
		case ilog2(STS_USB31_PWRON):
		case ilog2(STS_ENABLE_4V5):
		case ilog2(STS_BUTTON_MODE):
			return GPIOF_OUTPUT;
		default:
			return GPIOF_INPUT;
		}

	/* bank 1 - supported only when FEAT_EXT_CMDS is set */
	case (16 + 0) ... (16 + 31):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		return GPIOF_INPUT;

	/* bank 2 - supported only when FEAT_EXT_CMDS and FEAT_PERIPH_MCU is set */
	case (16 + 32 + 0) ... (16 + 32 + 15):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		if (!(info->features & FEAT_PERIPH_MCU))
			return -EINVAL;
		return GPIOF_OUTPUT;

	default:
		return -EINVAL;
	}
}

static int omnia_gpio_get_value(struct udevice *dev, uint offset)
{
	struct turris_omnia_mcu_info *info = dev_get_priv(dev->parent);
	u32 val32;
	u16 val16;
	int ret;

	switch (offset) {
	/* bank 0 */
	case 0 ... 15:
		ret = dm_i2c_read(dev->parent, CMD_GET_STATUS_WORD,
				  (void *)&val16, sizeof(val16));
		if (ret)
			return ret;

		return !!(le16_to_cpu(val16) & BIT(offset));

	/* bank 1 - supported only when FEAT_EXT_CMDS is set */
	case (16 + 0) ... (16 + 31):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;

		ret = dm_i2c_read(dev->parent, CMD_GET_EXT_STATUS_DWORD,
				  (void *)&val32, sizeof(val32));
		if (ret)
			return ret;

		return !!(le32_to_cpu(val32) & BIT(offset - 16));

	/* bank 2 - supported only when FEAT_EXT_CMDS and FEAT_PERIPH_MCU is set */
	case (16 + 32 + 0) ... (16 + 32 + 15):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		if (!(info->features & FEAT_PERIPH_MCU))
			return -EINVAL;

		ret = dm_i2c_read(dev->parent, CMD_GET_EXT_CONTROL_STATUS,
				  (void *)&val16, sizeof(val16));
		if (ret)
			return ret;

		return !!(le16_to_cpu(val16) & BIT(offset - 16 - 32));

	default:
		return -EINVAL;
	}
}

static int omnia_gpio_set_value(struct udevice *dev, uint offset, int value)
{
	struct turris_omnia_mcu_info *info = dev_get_priv(dev->parent);
	u16 valmask16[2];
	u8 valmask8[2];

	switch (offset) {
	/* bank 0 */
	case 0 ... 15:
		switch (offset) {
		case ilog2(STS_USB30_PWRON):
			valmask8[1] = CTL_USB30_PWRON;
			break;
		case ilog2(STS_USB31_PWRON):
			valmask8[1] = CTL_USB31_PWRON;
			break;
		case ilog2(STS_ENABLE_4V5):
			valmask8[1] = CTL_ENABLE_4V5;
			break;
		case ilog2(STS_BUTTON_MODE):
			valmask8[1] = CTL_BUTTON_MODE;
			break;
		default:
			return -EINVAL;
		}

		valmask8[0] = value ? valmask8[1] : 0;

		return dm_i2c_write(dev->parent, CMD_GENERAL_CONTROL, valmask8,
				    sizeof(valmask8));

	/* bank 2 - supported only when FEAT_EXT_CMDS and FEAT_PERIPH_MCU is set */
	case (16 + 32 + 0) ... (16 + 32 + 15):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		if (!(info->features & FEAT_PERIPH_MCU))
			return -EINVAL;

		valmask16[1] = cpu_to_le16(BIT(offset - 16 - 32));
		valmask16[0] = value ? valmask16[1] : 0;

		return dm_i2c_write(dev->parent, CMD_EXT_CONTROL,
				    (void *)valmask16, sizeof(valmask16));

	default:
		return -EINVAL;
	}
}

static int omnia_gpio_direction_input(struct udevice *dev, uint offset)
{
	int ret;

	ret = omnia_gpio_get_function(dev, offset);
	if (ret < 0)
		return ret;
	else if (ret != GPIOF_INPUT)
		return -EOPNOTSUPP;

	return 0;
}

static int omnia_gpio_direction_output(struct udevice *dev, uint offset, int value)
{
	int ret;

	ret = omnia_gpio_get_function(dev, offset);
	if (ret < 0)
		return ret;
	else if (ret != GPIOF_OUTPUT)
		return -EOPNOTSUPP;

	return omnia_gpio_set_value(dev, offset, value);
}

static int omnia_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
				  struct ofnode_phandle_args *args)
{
	uint bank, gpio, flags, offset;
	int ret;

	if (args->args_count != 3)
		return -EINVAL;

	bank = args->args[0];
	gpio = args->args[1];
	flags = args->args[2];

	switch (bank) {
	case 0:
		if (gpio >= 16)
			return -EINVAL;
		offset = gpio;
		break;
	case 1:
		if (gpio >= 32)
			return -EINVAL;
		offset = 16 + gpio;
		break;
	case 2:
		if (gpio >= 16)
			return -EINVAL;
		offset = 16 + 32 + gpio;
		break;
	default:
		return -EINVAL;
	}

	ret = omnia_gpio_get_function(dev, offset);
	if (ret < 0)
		return ret;

	desc->offset = offset;
	desc->flags = gpio_flags_xlate(flags);

	return 0;
}

static const struct dm_gpio_ops omnia_gpio_ops = {
	.direction_input	= omnia_gpio_direction_input,
	.direction_output	= omnia_gpio_direction_output,
	.get_value		= omnia_gpio_get_value,
	.set_value		= omnia_gpio_set_value,
	.get_function		= omnia_gpio_get_function,
	.xlate			= omnia_gpio_xlate,
};

static int omnia_gpio_probe(struct udevice *dev)
{
	struct turris_omnia_mcu_info *info = dev_get_priv(dev->parent);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->bank_name = "mcu_";

	if ((info->features & FEAT_EXT_CMDS) && (info->features & FEAT_PERIPH_MCU))
		uc_priv->gpio_count = 16 + 32 + 16;
	else if (info->features & FEAT_EXT_CMDS)
		uc_priv->gpio_count = 16 + 32;
	else
		uc_priv->gpio_count = 16;

	return 0;
}

U_BOOT_DRIVER(turris_omnia_mcu_gpio) = {
	.name		= "turris-omnia-mcu-gpio",
	.id		= UCLASS_GPIO,
	.ops		= &omnia_gpio_ops,
	.probe		= omnia_gpio_probe,
};

static int omnia_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct {
		u16 magic;
		u16 arg;
		u32 csum;
	} __packed args;

	if (type != SYSRESET_POWER_OFF)
		return -EPROTONOSUPPORT;

	args.magic = CMD_POWER_OFF_MAGIC;
	args.arg = CMD_POWER_OFF_POWERON_BUTTON;
	args.csum = 0xba3b7212;

	return dm_i2c_write(dev->parent, CMD_POWER_OFF, (void *)&args,
			    sizeof(args));
}

static const struct sysreset_ops omnia_sysreset_ops = {
	.request	= omnia_sysreset_request,
};

U_BOOT_DRIVER(turris_omnia_mcu_sysreset) = {
	.name		= "turris-omnia-mcu-sysreset",
	.id		= UCLASS_SYSRESET,
	.ops		= &omnia_sysreset_ops,
};

static int omnia_rng_read(struct udevice *dev, void *data, size_t count)
{
	u8 buf[1 + CMD_TRNG_MAX_ENTROPY_LEN];
	size_t len;
	int ret;

	while (count) {
		ret = dm_i2c_read(dev->parent, CMD_TRNG_COLLECT_ENTROPY, buf,
				  sizeof(buf));
		if (ret)
			return ret;

		len = min_t(size_t, buf[0],
			    min_t(size_t, CMD_TRNG_MAX_ENTROPY_LEN, count));

		if (!len) {
			/* wait 500ms (fail if interrupted), then try again */
			for (int i = 0; i < 5; ++i) {
				mdelay(100);
				if (ctrlc())
					return -EINTR;
			}
			continue;
		}

		memcpy(data, &buf[1], len);
		data += len;
		count -= len;
	}

	return 0;
}

static const struct dm_rng_ops omnia_rng_ops = {
	.read		= omnia_rng_read,
};

U_BOOT_DRIVER(turris_omnia_mcu_trng) = {
	.name		= "turris-omnia-mcu-trng",
	.id		= UCLASS_RNG,
	.ops		= &omnia_rng_ops,
};

static int turris_omnia_mcu_bind(struct udevice *dev)
{
	/* bind MCU GPIOs as a child device */
	return device_bind_driver_to_node(dev, "turris-omnia-mcu-gpio",
					  "turris-omnia-mcu-gpio",
					  dev_ofnode(dev), NULL);
}

static int turris_omnia_mcu_probe(struct udevice *dev)
{
	struct turris_omnia_mcu_info *info = dev_get_priv(dev);
	u32 dword;
	u16 word;
	int ret;

	ret = dm_i2c_read(dev, CMD_GET_STATUS_WORD, (void *)&word, sizeof(word));
	if (ret < 0) {
		printf("Error: turris_omnia_mcu CMD_GET_STATUS_WORD failed: %d\n",
		       ret);
		return ret;
	}

	if (le16_to_cpu(word) & STS_FEATURES_SUPPORTED) {
		/* try read 32-bit features */
		ret = dm_i2c_read(dev, CMD_GET_FEATURES, (void *)&dword,
				  sizeof(dword));
		if (ret < 0) {
			/* try read 16-bit features */
			ret = dm_i2c_read(dev, CMD_GET_FEATURES, (void *)&word,
					  sizeof(word));
			if (ret < 0) {
				printf("Error: turris_omnia_mcu CMD_GET_FEATURES failed: %d\n",
				       ret);
				return ret;
			}

			info->features = le16_to_cpu(word);
		} else {
			info->features = le32_to_cpu(dword);
			if (info->features & FEAT_FROM_BIT_16_INVALID)
				info->features &= GENMASK(15, 0);
		}
	}

	/* bind sysreset if poweroff is supported */
	if (info->features & FEAT_POWEROFF_WAKEUP) {
		ret = device_bind_driver_to_node(dev,
						 "turris-omnia-mcu-sysreset",
						 "turris-omnia-mcu-sysreset",
						 dev_ofnode(dev), NULL);
		if (ret < 0)
			return ret;
	}

	/* bind rng if trng is supported */
	if (info->features & FEAT_TRNG) {
		ret = device_bind_driver_to_node(dev, "turris-omnia-mcu-trng",
						 "turris-omnia-mcu-trng",
						 dev_ofnode(dev), NULL);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static const struct udevice_id turris_omnia_mcu_ids[] = {
	{ .compatible = "cznic,turris-omnia-mcu" },
	{ }
};

U_BOOT_DRIVER(turris_omnia_mcu) = {
	.name		= "turris-omnia-mcu",
	.id		= UCLASS_MISC,
	.bind		= turris_omnia_mcu_bind,
	.probe		= turris_omnia_mcu_probe,
	.priv_auto	= sizeof(struct turris_omnia_mcu_info),
	.of_match	= turris_omnia_mcu_ids,
};

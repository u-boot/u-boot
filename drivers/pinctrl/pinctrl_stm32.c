#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAX_PINS_ONE_IP			70
#define MODE_BITS_MASK			3
#define OSPEED_MASK			3
#define PUPD_MASK			3
#define OTYPE_MSK			1
#define AFR_MASK			0xF

static int stm32_gpio_config(struct gpio_desc *desc,
			     const struct stm32_gpio_ctl *ctl)
{
	struct stm32_gpio_priv *priv = dev_get_priv(desc->dev);
	struct stm32_gpio_regs *regs = priv->regs;
	u32 index;

	if (!ctl || ctl->af > 15 || ctl->mode > 3 || ctl->otype > 1 ||
	    ctl->pupd > 2 || ctl->speed > 3)
		return -EINVAL;

	index = (desc->offset & 0x07) * 4;
	clrsetbits_le32(&regs->afr[desc->offset >> 3], AFR_MASK << index,
			ctl->af << index);

	index = desc->offset * 2;
	clrsetbits_le32(&regs->moder, MODE_BITS_MASK << index,
			ctl->mode << index);
	clrsetbits_le32(&regs->ospeedr, OSPEED_MASK << index,
			ctl->speed << index);
	clrsetbits_le32(&regs->pupdr, PUPD_MASK << index, ctl->pupd << index);

	index = desc->offset;
	clrsetbits_le32(&regs->otyper, OTYPE_MSK << index, ctl->otype << index);

	return 0;
}
static int prep_gpio_dsc(struct stm32_gpio_dsc *gpio_dsc, u32 port_pin)
{
	gpio_dsc->port = (port_pin & 0xF000) >> 12;
	gpio_dsc->pin = (port_pin & 0x0F00) >> 8;
	debug("%s: GPIO:port= %d, pin= %d\n", __func__, gpio_dsc->port,
	      gpio_dsc->pin);

	return 0;
}

static int prep_gpio_ctl(struct stm32_gpio_ctl *gpio_ctl, u32 gpio_fn, int node)
{
	gpio_fn &= 0x00FF;
	gpio_ctl->af = 0;

	switch (gpio_fn) {
	case 0:
		gpio_ctl->mode = STM32_GPIO_MODE_IN;
		break;
	case 1 ... 16:
		gpio_ctl->mode = STM32_GPIO_MODE_AF;
		gpio_ctl->af = gpio_fn - 1;
		break;
	case 17:
		gpio_ctl->mode = STM32_GPIO_MODE_AN;
		break;
	default:
		gpio_ctl->mode = STM32_GPIO_MODE_OUT;
		break;
	}

	gpio_ctl->speed = fdtdec_get_int(gd->fdt_blob, node, "slew-rate", 0);

	if (fdtdec_get_bool(gd->fdt_blob, node, "drive-open-drain"))
		gpio_ctl->otype = STM32_GPIO_OTYPE_OD;
	else
		gpio_ctl->otype = STM32_GPIO_OTYPE_PP;

	if (fdtdec_get_bool(gd->fdt_blob, node, "bias-pull-up"))
		gpio_ctl->pupd = STM32_GPIO_PUPD_UP;
	else if (fdtdec_get_bool(gd->fdt_blob, node, "bias-pull-down"))
		gpio_ctl->pupd = STM32_GPIO_PUPD_DOWN;
	else
		gpio_ctl->pupd = STM32_GPIO_PUPD_NO;

	debug("%s: gpio fn= %d, slew-rate= %x, op type= %x, pull-upd is = %x\n",
	      __func__,  gpio_fn, gpio_ctl->speed, gpio_ctl->otype,
	     gpio_ctl->pupd);

	return 0;
}

static int stm32_pinctrl_config(int offset)
{
	u32 pin_mux[MAX_PINS_ONE_IP];
	int rv, len;

	/*
	 * check for "pinmux" property in each subnode (e.g. pins1 and pins2 for
	 * usart1) of pin controller phandle "pinctrl-0"
	 * */
	fdt_for_each_subnode(offset, gd->fdt_blob, offset) {
		struct stm32_gpio_dsc gpio_dsc;
		struct stm32_gpio_ctl gpio_ctl;
		int i;

		len = fdtdec_get_int_array_count(gd->fdt_blob, offset,
						 "pinmux", pin_mux,
						 ARRAY_SIZE(pin_mux));
		debug("%s: no of pinmux entries= %d\n", __func__, len);
		if (len < 0)
			return -EINVAL;
		for (i = 0; i < len; i++) {
			struct gpio_desc desc;
			debug("%s: pinmux = %x\n", __func__, *(pin_mux + i));
			prep_gpio_dsc(&gpio_dsc, *(pin_mux + i));
			prep_gpio_ctl(&gpio_ctl, *(pin_mux + i), offset);
			rv = uclass_get_device_by_seq(UCLASS_GPIO,
						      gpio_dsc.port, &desc.dev);
			if (rv)
				return rv;
			desc.offset = gpio_dsc.pin;
			rv = stm32_gpio_config(&desc, &gpio_ctl);
			debug("%s: rv = %d\n\n", __func__, rv);
			if (rv)
				return rv;
		}
	}

	return 0;
}

#if CONFIG_IS_ENABLED(PINCTRL_FULL)
static int stm32_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	return stm32_pinctrl_config(dev_of_offset(config));
}
#else /* PINCTRL_FULL */
static int stm32_pinctrl_set_state_simple(struct udevice *dev,
					  struct udevice *periph)
{
	const void *fdt = gd->fdt_blob;
	const fdt32_t *list;
	uint32_t phandle;
	int config_node;
	int size, i, ret;

	list = fdt_getprop(fdt, dev_of_offset(periph), "pinctrl-0", &size);
	if (!list)
		return -EINVAL;

	debug("%s: periph->name = %s\n", __func__, periph->name);

	size /= sizeof(*list);
	for (i = 0; i < size; i++) {
		phandle = fdt32_to_cpu(*list++);

		config_node = fdt_node_offset_by_phandle(fdt, phandle);
		if (config_node < 0) {
			pr_err("prop pinctrl-0 index %d invalid phandle\n", i);
			return -EINVAL;
		}

		ret = stm32_pinctrl_config(config_node);
		if (ret)
			return ret;
	}

	return 0;
}
#endif /* PINCTRL_FULL */

static struct pinctrl_ops stm32_pinctrl_ops = {
#if CONFIG_IS_ENABLED(PINCTRL_FULL)
	.set_state		= stm32_pinctrl_set_state,
#else /* PINCTRL_FULL */
	.set_state_simple	= stm32_pinctrl_set_state_simple,
#endif /* PINCTRL_FULL */
};

static const struct udevice_id stm32_pinctrl_ids[] = {
	{ .compatible = "st,stm32f429-pinctrl" },
	{ .compatible = "st,stm32f469-pinctrl" },
	{ .compatible = "st,stm32f746-pinctrl" },
	{ .compatible = "st,stm32h743-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_stm32) = {
	.name		= "pinctrl_stm32",
	.id		= UCLASS_PINCTRL,
	.of_match	= stm32_pinctrl_ids,
	.ops		= &stm32_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
};

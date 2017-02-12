#include <common.h>
#include <asm/arch/gpio.h>
#include <dm.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

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

static int stm32_pinctrl_set_state_simple(struct udevice *dev,
					  struct udevice *periph)
{
	u32 pin_mux[50];
	struct fdtdec_phandle_args args;
	int rv, len;

	/* Get node pinctrl-0 */
	rv = fdtdec_parse_phandle_with_args(gd->fdt_blob, periph->of_offset,
					   "pinctrl-0", 0, 0, 0, &args);
	if (rv)
		return rv;
	/*
	 * check for "pinmux" property in each subnode (e.g. pins1 and pins2 for
	 * usart1) of pin controller phandle "pinctrl-0"
	 * */
	fdt_for_each_subnode(args.node, gd->fdt_blob, args.node) {
		struct stm32_gpio_dsc gpio_dsc;
		struct stm32_gpio_ctl gpio_ctl;
		int i;

		len = fdtdec_get_int_array_count(gd->fdt_blob, args.node,
						 "pinmux", pin_mux,
						 ARRAY_SIZE(pin_mux));
		debug("%s: periph->name = %s, no of pinmux entries= %d\n",
		      __func__, periph->name, len);
		if (len < 0)
			return -EINVAL;
		for (i = 0; i < len; i++) {
			debug("%s: pinmux = %x\n", __func__, *(pin_mux + i));
			prep_gpio_dsc(&gpio_dsc, *(pin_mux + i));
			prep_gpio_ctl(&gpio_ctl, *(pin_mux + i), args.node);

			rv = stm32_gpio_config(&gpio_dsc, &gpio_ctl);
			debug("%s: rv = %d\n\n", __func__, rv);
			if (rv)
				return rv;
		}
	}

	return 0;
}

static struct pinctrl_ops stm32_pinctrl_ops = {
	.set_state_simple	= stm32_pinctrl_set_state_simple,
};

static const struct udevice_id stm32_pinctrl_ids[] = {
	{ .compatible = "st,stm32f746-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_stm32) = {
	.name		= "pinctrl_stm32",
	.id		= UCLASS_PINCTRL,
	.of_match	= stm32_pinctrl_ids,
	.ops		= &stm32_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 - 2018 Xilinx, Michal Simek
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <linux/list.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm.h>

static LIST_HEAD(gpio_list);

enum gpio_direction {
	GPIO_DIRECTION_OUT = 0,
	GPIO_DIRECTION_IN = 1,
};

/* Gpio simple map */
struct gpio_regs {
	u32 gpiodata;
	u32 gpiodir;
};

#if !defined(CONFIG_DM_GPIO)

#define GPIO_NAME_SIZE	10

struct gpio_names {
	char name[GPIO_NAME_SIZE];
};

/* Initialized, rxbd_current, rx_first_buf must be 0 after init */
struct xilinx_gpio_priv {
	struct gpio_regs *regs;
	u32 gpio_min;
	u32 gpio_max;
	u32 gpiodata_store;
	char name[GPIO_NAME_SIZE];
	struct list_head list;
	struct gpio_names *gpio_name;
};

/* Store number of allocated gpio pins */
static u32 xilinx_gpio_max;

/* Get associated gpio controller */
static struct xilinx_gpio_priv *gpio_get_controller(unsigned gpio)
{
	struct list_head *entry;
	struct xilinx_gpio_priv *priv = NULL;

	list_for_each(entry, &gpio_list) {
		priv = list_entry(entry, struct xilinx_gpio_priv, list);
		if (gpio >= priv->gpio_min && gpio <= priv->gpio_max) {
			debug("%s: reg: %x, min-max: %d-%d\n", __func__,
			      (u32)priv->regs, priv->gpio_min, priv->gpio_max);
			return priv;
		}
	}
	puts("!!!Can't get gpio controller!!!\n");
	return NULL;
}

/* Get gpio pin name if used/setup */
static char *get_name(unsigned gpio)
{
	u32 gpio_priv;
	struct xilinx_gpio_priv *priv;

	debug("%s\n", __func__);

	priv = gpio_get_controller(gpio);
	if (priv) {
		gpio_priv = gpio - priv->gpio_min;

		return *priv->gpio_name[gpio_priv].name ?
			priv->gpio_name[gpio_priv].name : "UNKNOWN";
	}
	return "UNKNOWN";
}

/* Get output value */
static int gpio_get_output_value(unsigned gpio)
{
	u32 val, gpio_priv;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		gpio_priv = gpio - priv->gpio_min;
		val = !!(priv->gpiodata_store & (1 << gpio_priv));
		debug("%s: reg: %x, gpio_no: %d, dir: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);

		return val;
	}
	return -1;
}

/* Get input value */
static int gpio_get_input_value(unsigned gpio)
{
	u32 val, gpio_priv;
	struct gpio_regs *regs;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		regs = priv->regs;
		gpio_priv = gpio - priv->gpio_min;
		val = readl(&regs->gpiodata);
		val = !!(val & (1 << gpio_priv));
		debug("%s: reg: %x, gpio_no: %d, dir: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);

		return val;
	}
	return -1;
}

/* Set gpio direction */
static int gpio_set_direction(unsigned gpio, enum gpio_direction direction)
{
	u32 val, gpio_priv;
	struct gpio_regs *regs;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		regs = priv->regs;
		val = readl(&regs->gpiodir);

		gpio_priv = gpio - priv->gpio_min;
		if (direction == GPIO_DIRECTION_OUT)
			val &= ~(1 << gpio_priv);
		else
			val |= 1 << gpio_priv;

		writel(val, &regs->gpiodir);
		debug("%s: reg: %x, gpio_no: %d, dir: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);

		return 0;
	}

	return -1;
}

/* Get gpio direction */
static int gpio_get_direction(unsigned gpio)
{
	u32 val, gpio_priv;
	struct gpio_regs *regs;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		regs = priv->regs;
		gpio_priv = gpio - priv->gpio_min;
		val = readl(&regs->gpiodir);
		val = !!(val & (1 << gpio_priv));
		debug("%s: reg: %x, gpio_no: %d, dir: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);

		return val;
	}

	return -1;
}

/*
 * Get input value
 * for example gpio setup to output only can't get input value
 * which is breaking gpio toggle command
 */
int gpio_get_value(unsigned gpio)
{
	u32 val;

	if (gpio_get_direction(gpio) == GPIO_DIRECTION_OUT)
		val = gpio_get_output_value(gpio);
	else
		val = gpio_get_input_value(gpio);

	return val;
}

/* Set output value */
static int gpio_set_output_value(unsigned gpio, int value)
{
	u32 val, gpio_priv;
	struct gpio_regs *regs;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		regs = priv->regs;
		gpio_priv = gpio - priv->gpio_min;
		val = priv->gpiodata_store;
		if (value)
			val |= 1 << gpio_priv;
		else
			val &= ~(1 << gpio_priv);

		writel(val, &regs->gpiodata);
		debug("%s: reg: %x, gpio_no: %d, output_val: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);
		priv->gpiodata_store = val;

		return 0;
	}

	return -1;
}

int gpio_set_value(unsigned gpio, int value)
{
	if (gpio_get_direction(gpio) == GPIO_DIRECTION_OUT)
		return gpio_set_output_value(gpio, value);

	return -1;
}

/* Set GPIO as input */
int gpio_direction_input(unsigned gpio)
{
	debug("%s\n", __func__);
	return gpio_set_direction(gpio, GPIO_DIRECTION_IN);
}

/* Setup GPIO as output and set output value */
int gpio_direction_output(unsigned gpio, int value)
{
	int ret = gpio_set_direction(gpio, GPIO_DIRECTION_OUT);

	debug("%s\n", __func__);

	if (ret < 0)
		return ret;

	return gpio_set_output_value(gpio, value);
}

/* Show gpio status */
void gpio_info(void)
{
	unsigned gpio;

	struct list_head *entry;
	struct xilinx_gpio_priv *priv = NULL;

	list_for_each(entry, &gpio_list) {
		priv = list_entry(entry, struct xilinx_gpio_priv, list);
		printf("\n%s: %s/%x (%d-%d)\n", __func__, priv->name,
		       (u32)priv->regs, priv->gpio_min, priv->gpio_max);

		for (gpio = priv->gpio_min; gpio <= priv->gpio_max; gpio++) {
			printf("GPIO_%d:\t%s is an ", gpio, get_name(gpio));
			if (gpio_get_direction(gpio) == GPIO_DIRECTION_OUT)
				printf("OUTPUT value = %d\n",
				       gpio_get_output_value(gpio));
			else
				printf("INPUT value = %d\n",
				       gpio_get_input_value(gpio));
		}
	}
}

int gpio_request(unsigned gpio, const char *label)
{
	u32 gpio_priv;
	struct xilinx_gpio_priv *priv;

	if (gpio >= xilinx_gpio_max)
		return -EINVAL;

	priv = gpio_get_controller(gpio);
	if (priv) {
		gpio_priv = gpio - priv->gpio_min;

		if (label != NULL) {
			strncpy(priv->gpio_name[gpio_priv].name, label,
				GPIO_NAME_SIZE);
			priv->gpio_name[gpio_priv].name[GPIO_NAME_SIZE - 1] =
					'\0';
		}
		return 0;
	}

	return -1;
}

int gpio_free(unsigned gpio)
{
	u32 gpio_priv;
	struct xilinx_gpio_priv *priv;

	if (gpio >= xilinx_gpio_max)
		return -EINVAL;

	priv = gpio_get_controller(gpio);
	if (priv) {
		gpio_priv = gpio - priv->gpio_min;
		priv->gpio_name[gpio_priv].name[0] = '\0';

		/* Do nothing here */
		return 0;
	}

	return -1;
}

int gpio_alloc(u32 baseaddr, const char *name, u32 gpio_no)
{
	struct xilinx_gpio_priv *priv;

	priv = calloc(1, sizeof(struct xilinx_gpio_priv));

	/* Setup gpio name */
	if (name != NULL) {
		strncpy(priv->name, name, GPIO_NAME_SIZE);
		priv->name[GPIO_NAME_SIZE - 1] = '\0';
	}
	priv->regs = (struct gpio_regs *)baseaddr;

	priv->gpio_min = xilinx_gpio_max;
	xilinx_gpio_max = priv->gpio_min + gpio_no;
	priv->gpio_max = xilinx_gpio_max - 1;

	priv->gpio_name = calloc(gpio_no, sizeof(struct gpio_names));

	INIT_LIST_HEAD(&priv->list);
	list_add_tail(&priv->list, &gpio_list);

	printf("%s: Add %s (%d-%d)\n", __func__, name,
	       priv->gpio_min, priv->gpio_max);

	/* Return the first gpio allocated for this device */
	return priv->gpio_min;
}

/* Dual channel gpio is one IP with two independent channels */
int gpio_alloc_dual(u32 baseaddr, const char *name, u32 gpio_no0, u32 gpio_no1)
{
	int ret;

	ret = gpio_alloc(baseaddr, name, gpio_no0);
	gpio_alloc(baseaddr + 8, strcat((char *)name, "_1"), gpio_no1);

	/* Return the first gpio allocated for this device */
	return ret;
}
#else
#include <dt-bindings/gpio/gpio.h>

#define XILINX_GPIO_MAX_BANK	2

struct xilinx_gpio_platdata {
	struct gpio_regs *regs;
	int bank_max[XILINX_GPIO_MAX_BANK];
	int bank_input[XILINX_GPIO_MAX_BANK];
	int bank_output[XILINX_GPIO_MAX_BANK];
};

static int xilinx_gpio_get_bank_pin(unsigned offset, u32 *bank_num,
				    u32 *bank_pin_num, struct udevice *dev)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	u32 bank, max_pins;
	/* the first gpio is 0 not 1 */
	u32 pin_num = offset;

	for (bank = 0; bank < XILINX_GPIO_MAX_BANK; bank++) {
		max_pins = platdata->bank_max[bank];
		if (pin_num < max_pins) {
			debug("%s: found at bank 0x%x pin 0x%x\n", __func__,
			      bank, pin_num);
			*bank_num = bank;
			*bank_pin_num = pin_num;
			return 0;
		}
		pin_num -= max_pins;
	}

	return -EINVAL;
}

static int xilinx_gpio_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	debug("%s: regs: %lx, value: %x, gpio: %x, bank %x, pin %x\n",
	      __func__, (ulong)platdata->regs, value, offset, bank, pin);

	if (value) {
		val = readl(&platdata->regs->gpiodata + bank * 2);
		val = val | (1 << pin);
		writel(val, &platdata->regs->gpiodata + bank * 2);
	} else {
		val = readl(&platdata->regs->gpiodata + bank * 2);
		val = val & ~(1 << pin);
		writel(val, &platdata->regs->gpiodata + bank * 2);
	}

	return val;
};

static int xilinx_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	debug("%s: regs: %lx, gpio: %x, bank %x, pin %x\n", __func__,
	      (ulong)platdata->regs, offset, bank, pin);

	val = readl(&platdata->regs->gpiodata + bank * 2);
	val = !!(val & (1 << pin));

	return val;
};

static int xilinx_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int val, ret;
	u32 bank, pin;

	/* Check if all pins are inputs */
	if (platdata->bank_input[bank])
		return GPIOF_INPUT;

	/* Check if all pins are outputs */
	if (platdata->bank_output[bank])
		return GPIOF_OUTPUT;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	/* FIXME test on dual */
	val = readl(&platdata->regs->gpiodir + bank * 2);
	val = !(val & (1 << pin));

	/* input is 1 in reg but GPIOF_INPUT is 0 */
	/* output is 0 in reg but GPIOF_OUTPUT is 1 */

	return val;
}

static int xilinx_gpio_direction_output(struct udevice *dev, unsigned offset,
					int value)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	/* can't change it if all is input by default */
	if (platdata->bank_input[bank])
		return -EINVAL;

	if (!platdata->bank_output[bank]) {
		val = readl(&platdata->regs->gpiodir + bank * 2);
		val = val & ~(1 << pin);
		writel(val, &platdata->regs->gpiodir + bank * 2);
	}

	xilinx_gpio_set_value(dev, offset, value);

	return 0;
}

static int xilinx_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	/* Already input */
	if (platdata->bank_input[bank])
		return 0;

	/* can't change it if all is output by default */
	if (platdata->bank_output[bank])
		return -EINVAL;

	val = readl(&platdata->regs->gpiodir + bank * 2);
	val = val | (1 << pin);
	writel(val, &platdata->regs->gpiodir + bank * 2);

	return 0;
}

static int xilinx_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			     struct ofnode_phandle_args *args)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);

	desc->offset = args->args[0];

	debug("%s: argc: %x, [0]: %x, [1]: %x, [2]: %x\n", __func__,
	      args->args_count, args->args[0], args->args[1], args->args[2]);

	/*
	 * The second cell is channel offset:
	 *  0 is first channel, 8 is second channel
	 *
	 * U-Boot driver just combine channels together that's why simply
	 * add amount of pins in second channel if present.
	 */
	if (args->args[1]) {
		if (!platdata->bank_max[1]) {
			printf("%s: %s has no second channel\n",
			       __func__, dev->name);
			return -EINVAL;
		}

		desc->offset += platdata->bank_max[0];
	}

	/* The third cell is optional */
	if (args->args_count > 2)
		desc->flags = (args->args[2] &
			       GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0);

	debug("%s: offset %x, flags %lx\n",
	      __func__, desc->offset, desc->flags);
	return 0;
}

static const struct dm_gpio_ops xilinx_gpio_ops = {
	.direction_input = xilinx_gpio_direction_input,
	.direction_output = xilinx_gpio_direction_output,
	.get_value = xilinx_gpio_get_value,
	.set_value = xilinx_gpio_set_value,
	.get_function = xilinx_gpio_get_function,
	.xlate = xilinx_gpio_xlate,
};

static int xilinx_gpio_probe(struct udevice *dev)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->bank_name = dev->name;

	uc_priv->gpio_count = platdata->bank_max[0] + platdata->bank_max[1];

	return 0;
}

static int xilinx_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int is_dual;

	platdata->regs = (struct gpio_regs *)dev_read_addr(dev);

	platdata->bank_max[0] = dev_read_u32_default(dev,
						     "xlnx,gpio-width", 0);
	platdata->bank_input[0] = dev_read_u32_default(dev,
						       "xlnx,all-inputs", 0);
	platdata->bank_output[0] = dev_read_u32_default(dev,
							"xlnx,all-outputs", 0);

	is_dual = dev_read_u32_default(dev, "xlnx,is-dual", 0);
	if (is_dual) {
		platdata->bank_max[1] = dev_read_u32_default(dev,
						"xlnx,gpio2-width", 0);
		platdata->bank_input[1] = dev_read_u32_default(dev,
						"xlnx,all-inputs-2", 0);
		platdata->bank_output[1] = dev_read_u32_default(dev,
						"xlnx,all-outputs-2", 0);
	}

	return 0;
}

static const struct udevice_id xilinx_gpio_ids[] = {
	{ .compatible = "xlnx,xps-gpio-1.00.a",},
	{ }
};

U_BOOT_DRIVER(xilinx_gpio) = {
	.name = "xlnx_gpio",
	.id = UCLASS_GPIO,
	.ops = &xilinx_gpio_ops,
	.of_match = xilinx_gpio_ids,
	.ofdata_to_platdata = xilinx_gpio_ofdata_to_platdata,
	.probe = xilinx_gpio_probe,
	.platdata_auto_alloc_size = sizeof(struct xilinx_gpio_platdata),
};
#endif

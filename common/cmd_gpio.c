/*
 * Control GPIO pins on the fly
 *
 * Copyright (c) 2008-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <asm/gpio.h>

int __weak name_to_gpio(const char *name)
{
	return simple_strtoul(name, NULL, 10);
}

enum gpio_cmd {
	GPIO_INPUT,
	GPIO_SET,
	GPIO_CLEAR,
	GPIO_TOGGLE,
};

#if defined(CONFIG_DM_GPIO) && !defined(gpio_status)
static const char * const gpio_function[] = {
	"input",
	"output",
	"unknown",
};

static void show_gpio(struct udevice *dev, const char *bank_name, int offset)
{
	struct dm_gpio_ops *ops = gpio_get_ops(dev);
	char buf[80];
	int ret;

	*buf = '\0';
	if (ops->get_state) {
		ret = ops->get_state(dev, offset, buf, sizeof(buf));
		if (ret) {
			puts("<unknown>");
			return;
		}
	} else {
		int func =  GPIOF_UNKNOWN;
		int ret;

		if (ops->get_function) {
			ret = ops->get_function(dev, offset);
			if (ret >= 0 && ret < ARRAY_SIZE(gpio_function))
				func = ret;
		}
		sprintf(buf, "%s%u: %8s %d", bank_name, offset,
			gpio_function[func], ops->get_value(dev, offset));
	}

	puts(buf);
	puts("\n");
}

static int do_gpio_status(const char *gpio_name)
{
	struct udevice *dev;
	int newline = 0;
	int ret;

	if (gpio_name && !*gpio_name)
		gpio_name = NULL;
	for (ret = uclass_first_device(UCLASS_GPIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		const char *bank_name;
		int num_bits;

		bank_name = gpio_get_bank_info(dev, &num_bits);

		if (!gpio_name || !bank_name ||
		    !strncmp(gpio_name, bank_name, strlen(bank_name))) {
			const char *p = NULL;
			int offset;

			if (bank_name) {
				if (newline)
					putc('\n');
				printf("Bank %s:\n", bank_name);
			}
			newline = 1;
			if (gpio_name && bank_name) {
				p = gpio_name + strlen(bank_name);
				offset = simple_strtoul(p, NULL, 10);
				show_gpio(dev, bank_name, offset);
			} else {
				for (offset = 0; offset < num_bits; offset++)
					show_gpio(dev, bank_name, offset);
			}
		}
	}

	return ret;
}
#endif

static int do_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int gpio;
	enum gpio_cmd sub_cmd;
	ulong value;
	const char *str_cmd, *str_gpio = NULL;
#ifdef CONFIG_DM_GPIO
	int ret;
#endif

	if (argc < 2)
 show_usage:
		return CMD_RET_USAGE;
	str_cmd = argv[1];
	if (argc > 2)
		str_gpio = argv[2];
	if (!strcmp(str_cmd, "status")) {
		/* Support deprecated gpio_status() */
#ifdef gpio_status
		gpio_status();
		return 0;
#elif defined(CONFIG_DM_GPIO)
		return cmd_process_error(cmdtp, do_gpio_status(str_gpio));
#else
		goto show_usage;
#endif
	}

	if (!str_gpio)
		goto show_usage;

	/* parse the behavior */
	switch (*str_cmd) {
		case 'i': sub_cmd = GPIO_INPUT;  break;
		case 's': sub_cmd = GPIO_SET;    break;
		case 'c': sub_cmd = GPIO_CLEAR;  break;
		case 't': sub_cmd = GPIO_TOGGLE; break;
		default:  goto show_usage;
	}

#if defined(CONFIG_DM_GPIO)
	/*
	 * TODO(sjg@chromium.org): For now we must fit into the existing GPIO
	 * framework, so we look up the name here and convert it to a GPIO number.
	 * Once all GPIO drivers are converted to driver model, we can change the
	 * code here to use the GPIO uclass interface instead of the numbered
	 * GPIO compatibility layer.
	 */
	ret = gpio_lookup_name(str_gpio, NULL, NULL, &gpio);
	if (ret)
		return cmd_process_error(cmdtp, ret);
#else
	/* turn the gpio name into a gpio number */
	gpio = name_to_gpio(str_gpio);
	if (gpio < 0)
		goto show_usage;
#endif
	/* grab the pin before we tweak it */
	if (gpio_request(gpio, "cmd_gpio")) {
		printf("gpio: requesting pin %u failed\n", gpio);
		return -1;
	}

	/* finally, let's do it: set direction and exec command */
	if (sub_cmd == GPIO_INPUT) {
		gpio_direction_input(gpio);
		value = gpio_get_value(gpio);
	} else {
		switch (sub_cmd) {
			case GPIO_SET:    value = 1; break;
			case GPIO_CLEAR:  value = 0; break;
			case GPIO_TOGGLE: value = !gpio_get_value(gpio); break;
			default:          goto show_usage;
		}
		gpio_direction_output(gpio, value);
	}
	printf("gpio: pin %s (gpio %i) value is %lu\n",
		str_gpio, gpio, value);

	gpio_free(gpio);

	return value;
}

U_BOOT_CMD(gpio, 3, 0, do_gpio,
	"query and control gpio pins",
	"<input|set|clear|toggle> <pin>\n"
	"    - input/set/clear/toggle the specified pin\n"
	"gpio status [<bank> | <pin>]");
